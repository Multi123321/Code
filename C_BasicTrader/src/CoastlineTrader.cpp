#include "CoastlineTrader.h"
#include <string>
#include <deque>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <assert.h>
#include "helper/Macros.h"
#include "AVXHelper.h"
#include <x86intrin.h>

typedef unsigned int uint;

CoastlineTrader::CoastlineTrader()
{
}

CoastlineTrader::CoastlineTrader(__m256d dOriginal, __m256d dUp, __m256d dDown, __m256d profitT, string FxRate, int lS) : deltaUp(dUp), deltaDown(dDown), deltaOriginal(dOriginal),
                                                                                                                          longShort(lS), profitTarget(profitT), cashLimit(profitT)
{
    tP = _mm256_setzero_pd(); /* -- Total position -- */

    pnl = _mm256_setzero_pd();
    tempPnl = _mm256_setzero_pd();
    pnlPerc = _mm256_setzero_pd();

    shrinkFlong = _mm256_set1_pd(1.0);
    shrinkFshort = _mm256_set1_pd(1.0);

    increaseLong = _mm256_setzero_pd();
    increaseShort = _mm256_setzero_pd();

    fxRate = FxRate;
}

__m256d CoastlineTrader::computePnl(PriceFeedData::Price price)
{
    __m256d result;

    for (int avx = 0; avx < 4; avx++)
    {
        double profitLoss = 0.0;
        double pricE = (tP > 0.0 ? price.bid : price.ask);
        for (uint i = 0; i < sizes[avx].size(); i++)
        {
            profitLoss += sizes[avx][i] * (pricE - prices[avx][i]);
        }

        ((double *)&result)[avx] = profitLoss;
    }

    return result;
}

__m256d CoastlineTrader::computePnlLastPrice()
{
    __m256d result;

    for (int avx = 0; avx < 4; avx++)
    {
        double profitLoss = 0.0;

        for (uint i = 0; i < sizes[avx].size(); i++)
        {
            profitLoss += sizes[avx][i] * (lastPrice - prices[avx][i]);
        }

        ((double *)&result)[avx] = profitLoss;
    }

    return result;
}

__m256d CoastlineTrader::getPercPnl(PriceFeedData::Price price)
{
    __m256d result;

    for (int avx = 0; avx < 4; avx++)
    {
        double pricE = (((double *)&tP)[avx] > 0.0 ? price.bid : price.ask);
        double percentage = 0.0;

        for (uint i = 0; i < sizes[avx].size(); i++)
        {
            double absProfitLoss = pricE - prices[avx][i];
            percentage += (absProfitLoss / prices[avx][i]) * sizes[avx][i];
        }

        ((double *)&result)[avx] = percentage;
    }

    return result;
}

mask CoastlineTrader::tryToClose(PriceFeedData::Price price)
{
    mask result;

    __m256d computedPnl = computePnl(price);

    for (int avx = 0; avx < 4; avx++)
    {
        if ((((double *)&tempPnl)[avx] + ((double *)&computedPnl)[avx]) / ((double *)&cashLimit)[avx] >= 1)
        {
            double pricE = (((double *)&tP)[avx] > 0.0 ? price.bid : price.ask);
            double addPnl = 0;
            uint len = prices[avx].size();
            for (uint i = 0; i < len; i++)
            {
                addPnl = (pricE - prices[avx].front()) * sizes[avx].front();
                ((double *)&tempPnl)[avx] += ((double *)&addPnl)[avx];
                ((double *)&tP)[avx] -= sizes[avx].front();
                sizes[avx].erase(sizes[avx].begin());
                prices[avx].erase(prices[avx].begin());
                if (i > 0)
                    ((double *)&increaseLong)[avx] += -1.0;
            }
            ((double *)&pnl)[avx] += ((double *)&tempPnl)[avx];
            ((double *)&pnlPerc)[avx] += (((double *)&tempPnl)[avx]) / ((double *)&cashLimit)[avx] * ((double *)&profitTarget)[avx];
            ((double *)&tempPnl)[avx] = 0;
            assert(sizes[avx].empty());
            assert(prices[avx].empty());
            ((long *)&result)[avx] = 0xFFFFFFFFFFFFFFFF;
        }
        else
        {
            ((long *)&result)[avx] = 0x0000000000000000;
        }
    }
}

void CoastlineTrader::assignCashTarget()
{
    cashLimit = _mm256_mul_pd(lastPrice, profitTarget);
}

mask CoastlineTrader::runPriceAsymm(PriceFeedData::Price price, __m256d oppositeInv)
{
    if (!initalized)
    {
        runner = Runner(deltaUp, deltaDown, price, fxRate, deltaUp, deltaDown);

        __m256d point75 = _mm256_set1_pd(0.75);
        __m256d onePoint5 = _mm256_set1_pd(1.5);
        __m256d twoPoint0 = _mm256_set1_pd(2.0);
        __m256d point50 = _mm256_set1_pd(0.50);

        runnerG[0][0] = Runner(_mm256_mul_pd(deltaUp, point75), _mm256_mul_pd(deltaDown, onePoint5), price, fxRate, _mm256_mul_pd(deltaUp, point75), _mm256_mul_pd(deltaUp, point75));
        runnerG[0][1] = Runner(_mm256_mul_pd(deltaUp, point50), _mm256_mul_pd(deltaDown, twoPoint0), price, fxRate, _mm256_mul_pd(deltaUp, point50), _mm256_mul_pd(deltaUp, point50));

        runnerG[1][0] = Runner(_mm256_mul_pd(deltaUp, onePoint5), _mm256_mul_pd(deltaDown, point75), price, fxRate, _mm256_mul_pd(deltaDown, point75), _mm256_mul_pd(deltaDown, point75));
        runnerG[1][1] = Runner(_mm256_mul_pd(deltaUp, twoPoint0), _mm256_mul_pd(deltaDown, point50), price, fxRate, _mm256_mul_pd(deltaDown, point50), _mm256_mul_pd(deltaDown, point50));

        liquidity = LocalLiquidity(deltaOriginal, deltaUp, deltaDown, price, deltaOriginal * 2.525729, 50.0);
        initalized = true;
    }

    lastPrice = price.getMid();

    if (!liquidity.computation(price))
    {
        cout << "Didn't compute liquidity!" << endl;
    }

    mask tryToCloseMask = tryToClose(price);

    IFDEBUG(for (int i = 0; i < 4; i++) {
        if (((long *)&tryToCloseMask)[i] != 0)
        { /* -- Try to close position -- */
            cout << "longShort: " << longShort << "; tP: " << ((double *)&tP)[i] << "; pnl: "
                 << ((double *)&pnl)[i] << "; pnlPerc: " << ((double *)&pnlPerc)[i] << "; tempPnl: "
                 << ((double *)&tempPnl)[i] << "; cashLimit: " << ((double *)&cashLimit)[i] << "; price: "
                 << ((double *)&lastPrice)[i] << std::endl;
            cout << "Close" << endl;
        }
    })
    mask tryToCloseElse = AVXHelper::invert(tryToCloseMask);

    __m256d event = _mm256_setzero_pd();

    __m256d fraction = _mm256_set1_pd(1.0);
    __m256d size = (liquidity.liq < 0.5 ? 0.5 : 1.0);

    // call runner and set event
    if (longShort == 1)
    {
        __m256d eventR = runner.run(price);
        __m256d eventG00 = runnerG[0][0].run(price);
        __m256d eventG01 = runnerG[0][1].run(price);

        for (int i = 0; i < 4; i++)
        {
            double currentTp = (((double *)&tP)[i]);
            if (15.0 <= currentTp && currentTp < 30.0)
            {
                ((double *)&event)[i] = ((double *)&eventG00)[i];
                ((double *)&fraction)[i] = 0.5;
            }
            else if (currentTp >= 30.0)
            {
                ((double *)&event)[i] = ((double *)&eventG01)[i];
                ((double *)&fraction)[i] = 0.25;
            }
            else
            {
                ((double *)&event)[i] = ((double *)&eventR)[i];
            }
        }
    }
    else if (longShort == -1)
    {
        __m256d eventR = runner.run(price);
        __m256d eventG10 = runnerG[1][0].run(price);
        __m256d eventG11 = runnerG[1][1].run(price);

        for (int i = 0; i < 4; i++)
        {
            double currentTp = (((double *)&tP)[i]);
            if (-30.0 < currentTp && currentTp < -15.0)
            {
                ((double *)&event)[i] = ((double *)&eventG10)[i];
                ((double *)&fraction)[i] = 0.5;
            }
            else if (currentTp <= -30.0)
            {
                ((double *)&event)[i] = ((double *)&eventG11)[i];
                ((double *)&fraction)[i] = 0.25;
            }
            else
            {
                ((double *)&event)[i] = ((double *)&eventR)[i];
            }
        }
    }

    if (longShort == 1)
    { // Long positions only

        mask maskEventSmallerZero = _mm256_cmp_pd(event, _mm256_setzero_pd(), _CMP_LT_OS);
        maskEventSmallerZero = AVXHelper::applyMask(tryToCloseElse, maskEventSmallerZero);
        //if (event < 0) -> use maskEventSmallerZero
        {
            __m256d sign = _mm256_mul_pd(runner.type, _mm_set1_pd(-1.0));

            __m256d maskTpEqualsZero = _mm256_cmp_pd(tP, _mm256_setzero_pd(), _CMP_EQ_OS);
            AVXHelper::applyMask(maskTpEqualsZero, maskEventSmallerZero);
            //if (tP == 0.0) -> use maskTpEqualsZero
            { // Open long position
                __m256d maskOppositeInvAbsGreaterThan15 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(15.0), _CMP_GT_OS);
                maskOppositeInvAbsGreaterThan15 = _mm256_or_pd(maskOppositeInvAbsGreaterThan15, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-15.0), _CMP_LT_OS));
                maskOppositeInvAbsGreaterThan15 = AVXHelper::applyMask(maskTpEqualsZero, maskOppositeInvAbsGreaterThan15);
                //if (std::abs(oppositeInv) > 15.0) -> use maskOppositeInvAbsGreaterThan15
                {
                    AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan15);

                    __m256d maskOppositeInvAbsGreaterThan30 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(30.0), _CMP_GT_OS);
                    maskOppositeInvAbsGreaterThan30 = _mm256_or_pd(maskOppositeInvAbsGreaterThan15, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-30.0), _CMP_LT_OS));
                    maskOppositeInvAbsGreaterThan30 = AVXHelper::applyMask(maskOppositeInvAbsGreaterThan30, maskOppositeInvAbsGreaterThan15);
                    //if (std::abs(oppositeInv) > 30.0) -> use maskOppositeInvAbsGreaterThan30
                    {
                        AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan30);
                    }
                }
                __m256d sizeToAdd = _mm256_mul_pd(sign, size);
                AVXHelper::applyMask(sizeToAdd, maskTpEqualsZero);
                tP = _mm256_add_pd(tP, sizeToAdd);

                for (int i = 0; i < 4; i++)
                {
                    if (((double *)&maskTpEqualsZero)[i] != 0)
                    {
                        sizes[i].push_front(((double *)&sizeToAdd)[i]);
                        prices[i].push_front(((double *)&sign)[i] == 1.0 ? price.ask : price.bid);
                    }
                }

                assignCashTarget();
                IFDEBUG(cout << "Open long" << endl);
            }
            __m256d maskTpLessThanZero = _mm256_cmp_pd(tP, _mm256_setzero_pd(), _CMP_GT_OS);
            //else if (tP > 0.0) -> use maskTpLessThanZero
            { // Increase long position (buy)
                __m256d sizeToAdd = AVXHelper::multiply(sign, size, fraction, shrinkFlong);

                IFDEBUG(
                    for (int i = 0; i < 4; i++) {if (((double*)&sizeToAdd)[i] < 0.0) {
                        cout << "How did this happen! increase position but neg size: " << sizeToAdd << endl;
                        exit(EXIT_FAILURE);
                    } });

                increaseLong = AVXHelper::addMasked(increaseLong, 1.0, maskTpLessThanZero);
                tP = AVXHelper::addMasked(tP, sizeToAdd, maskTpLessThanZero);

                for (int i = 0; i < 4; i++)
                {
                    if (((double *)&maskTpLessThanZero)[i] != 0)
                    {
                        sizes[i].push_back(((double *)&sizeToAdd)[i]);
                        prices[i].push_back(((double *)&sign)[i] == 1.0 ? price.ask : price.bid);
                    }
                }

                IFDEBUG(cout << "Cascade" << endl);
            }
        }
        mask maskEventGreaterZeroAndTpGreaterZero = _mm256_cmp_pd(event, _mm256_setzero_pd(), _CMP_GT_OS);
        maskEventGreaterZeroAndTpGreaterZero = AVXHelper::applyMask(maskEventGreaterZeroAndTpGreaterZero, _mm256_cmp_pd(tP, _mm256_setzero_pd(), _CMP_GT_OS));
        maskEventGreaterZeroAndTpGreaterZero = AVXHelper::applyMask(maskEventGreaterZeroAndTpGreaterZero, tryToCloseElse);
        //else if (event > 0 && tP > 0.0)
        { // Possibility to decrease long position only at intrinsic events
            mask TpGreaterZero = _mm256_cmp_pd(tP, _mm256_setzero_pd(), _CMP_GT_OS);
            __m256d pricE = AVXHelper::setValues(_mm256_set1_pd(price.bid), price.ask, TpGreaterZero);

            int removed = 0;
            for (uint i = 1; i < prices.size(); ++i)
            {
                int idx = i - removed;
                double tempP = (tP > 0.0 ? log(pricE / prices.at(idx)) : log(prices.at(idx) / pricE));
                if (tempP >= (tP > 0.0 ? deltaUp : deltaDown))
                {
                    double addPnl = (pricE - prices.at(idx)) * sizes.at(idx);
                    if (addPnl < 0.0)
                    {
                        IFDEBUG(cout << "Descascade with a loss: " << addPnl << endl);
                    }
                    tempPnl += addPnl;
                    tP -= sizes.at(idx);
                    sizes.erase(sizes.begin() + idx);
                    prices.erase(prices.begin() + idx);
                    removed++;
                    increaseLong += -1.0;
                    IFDEBUG(cout << "Decascade" << endl);
                }
            }
        }
    }
    else if (longShort == -1)
    { // Short positions only

        if (event > 0)
        {
            if (tP == 0.0)
            { // Open short position
                int sign = -runner.type;
                if (abs(oppositeInv) > 15.0)
                {
                    size = 1.0;
                    if (abs(oppositeInv) > 30.0)
                    {
                        size = 1.0;
                    }
                }
                double sizeToAdd = sign * size;
                if (sizeToAdd > 0.0)
                {
                    cout << "How did this happen! increase position but pos size: " << sizeToAdd << endl;
                    sizeToAdd = -sizeToAdd;
                }
                tP += sizeToAdd;
                sizes.push_front(sizeToAdd);

                prices.push_front(sign == 1 ? price.bid : price.ask);
                IFDEBUG(cout << "Open short" << endl);
                assignCashTarget();
            }
            else if (tP < 0.0)
            {
                int sign = -runner.type;
                double sizeToAdd = sign * size * fraction * shrinkFshort;
                if (sizeToAdd > 0.0)
                {
                    cout << "How did this happen! increase position but pos size: " << sizeToAdd << endl;
                    sizeToAdd = -sizeToAdd;
                }

                tP += sizeToAdd;
                sizes.push_back(sizeToAdd);
                increaseShort += 1.0;
                prices.push_back(sign == 1 ? price.bid : price.ask);
                IFDEBUG(cout << "Cascade" << endl);
            }
        }
        else if (event < 0.0 && tP < 0.0)
        {
            double pricE = (tP > 0.0 ? price.bid : price.ask);
            uint len = prices.size();
            int removed = 0;
            for (uint i = 1; i < len; ++i)
            {
                int idx = i - removed;
                double tempP = (tP > 0.0 ? log(pricE / prices.at(idx)) : log(prices.at(idx) / pricE));
                if (tempP >= (tP > 0.0 ? deltaUp : deltaDown))
                {
                    double addPnl = (pricE - prices.at(idx)) * sizes.at(idx);
                    if (addPnl < 0.0)
                    {
                        IFDEBUG(cout << "Descascade with a loss: " << addPnl << endl);
                    }
                    tempPnl += (pricE - prices.at(idx)) * sizes.at(idx);
                    tP -= sizes.at(idx);
                    sizes.erase(sizes.begin() + idx);
                    prices.erase(prices.begin() + idx);
                    removed++;
                    increaseShort += -1.0;
                    IFDEBUG(cout << "Decascade" << endl);
                }
            }
        }
    }
    else
    {
        cout << "Should never happen! " << longShort << endl;
    }
    //some prints
    IFDEBUG(cout << "longShort: " << longShort << "; tP: " << tP << "; pnl: " << pnl << "; pnlPerc: " << pnlPerc << "; tempPnl: " << tempPnl << "; unrealized: " << computePnlLastPrice() << "; cashLimit: " << cashLimit << "; price: " << lastPrice << std::endl);
    return true;
}