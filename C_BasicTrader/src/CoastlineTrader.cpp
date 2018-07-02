#include "CoastlineTrader.h"
#include <string>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <assert.h>
#include <x86intrin.h>
#include <algorithm>
#include <string.h>

#include "helper/Macros.h"
#include "AVXHelper.h"

typedef unsigned int uint;

CoastlineTrader::CoastlineTrader()
{
}

CoastlineTrader::CoastlineTrader(__m256d dOriginal, __m256d dUp, __m256d dDown, __m256d profitT, string FxRate, int lS) : deltaUp(dUp), deltaDown(dDown), deltaOriginal(dOriginal),
                                                                                                                          longShort(lS), profitTarget(profitT), cashLimit(profitT)
{
    tP = AVXHelper::avxZero; /* -- Total position -- */

    pnl = AVXHelper::avxZero;
    tempPnl = AVXHelper::avxZero;
    pnlPerc = AVXHelper::avxZero;

    shrinkFlong = _mm256_set1_pd(1.0);
    shrinkFshort = _mm256_set1_pd(1.0);

    increaseLong = AVXHelper::avxZero;
    increaseShort = AVXHelper::avxZero;

    fxRate = FxRate;
}

#if 0
deque<__m256d> convertListArrayToAvx(deque<double> list[])
{
    deque<__m256d> result;
    uint maxSize = max(max(list[0].size(), list[1].size()), max(list[2].size(), list[3].size()));
    for (uint i = 0; i < maxSize; i++)
    {
        double elements[4];
        elements[0] = (i < list[0].size()) ? list[0].at(i) : 0.0;
        elements[1] = (i < list[1].size()) ? list[1].at(i) : 0.0;
        elements[2] = (i < list[2].size()) ? list[2].at(i) : 0.0;
        elements[3] = (i < list[3].size()) ? list[3].at(i) : 0.0;
        __m256d newElement = *elements;
        result.push_back(newElement);
    }
    return result;
}
#endif

__m256d CoastlineTrader::computePnl(PriceFeedData::Price price)
{
    __m256d result;
#if 0
    __m256d maskTp = _mm256_cmp_pd(tp, AVXHelper::avxZero, _CMP_GT_OS);

    __m256d priceE = AVXHelper::setValues(price.ask, price.bid, maskTp);

#else
#ifdef AVX_LIST
    __m256d profitLoss = AVXHelper::avxZero;
    mask TpGreaterZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
    __m256d priceE = AVXHelper::setValues(_mm256_set1_pd(price.ask), price.bid, TpGreaterZero);

    for (uint i = 0; i < sizes.size(); i++)
    {
        profitLoss = _mm256_add_pd(profitLoss, _mm256_mul_pd(sizes[i], _mm256_sub_pd(priceE, prices[i])));
    }
    result = profitLoss;
#else
    SERIAL_AVX(avx)
    {
        double profitLoss = 0.0;
        double pricE = (((double *)&tP)[avx] > 0.0 ? price.bid : price.ask);

        //cout << std::dec << "Line: " << __LINE__ << " " << sizes[avx].size();
        for (uint i = 0; i < sizes[avx].size(); i++)
        {
            profitLoss += sizes[avx][i] * (pricE - prices[avx][i]);
        }
        //cout << profitLoss << endl;

        ((double *)&result)[avx] = profitLoss;
    }
#endif
#endif

    return result;
}

__m256d CoastlineTrader::computePnlLastPrice()
{
    __m256d result;
#ifdef AVX_LIST
    __m256d profitLoss = AVXHelper::avxZero;

    for (uint i = 0; i < sizes.size(); i++)
    {
        profitLoss = _mm256_add_pd(profitLoss, _mm256_mul_pd(sizes[i], _mm256_sub_pd(_mm256_set1_pd(lastPrice), prices[i])));
    }
    result = profitLoss;
#else
    SERIAL_AVX(avx)
    {
        double profitLoss = 0.0;
        //cout << std::dec << "Line: " << __LINE__ << " " << sizes[avx].size();
        for (uint i = 0; i < sizes[avx].size(); i++)
        {
            profitLoss += sizes[avx][i] * (lastPrice - prices[avx][i]);
        }
        //cout << profitLoss << endl;
        ((double *)&result)[avx] = profitLoss;
    }
#endif
    return result;
}

__m256d CoastlineTrader::getPercPnl(PriceFeedData::Price price)
{
    __m256d result;
#ifdef AVX_LIST
    __m256d percentage = AVXHelper::avxZero;
    mask TpGreaterZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
    __m256d priceE = AVXHelper::setValues(_mm256_set1_pd(price.ask), price.bid, TpGreaterZero);

    for (uint i = 0; i < sizes.size(); i++)
    {
        __m256d absProfitLoss = _mm256_sub_pd(priceE, prices[i]);
        percentage = _mm256_add_pd(percentage, _mm256_mul_pd(_mm256_div_pd(absProfitLoss, prices[i]), sizes[i]));
    }
    result = percentage;
#else
    SERIAL_AVX(avx)
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
#endif

    return result;
}

mask CoastlineTrader::tryToClose(PriceFeedData::Price price)
{
    mask result;

    __m256d computedPnl = computePnl(price);
#ifdef AVX_LIST
    mask toCloseMask = _mm256_cmp_pd(_mm256_div_pd(_mm256_add_pd(tempPnl, computedPnl), cashLimit), AVXHelper::avxOne, _CMP_GE_OS);
    if (!AVXHelper::isMaskZero(toCloseMask))
    {
        mask TpGreaterZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
        __m256d priceE = AVXHelper::setValues(_mm256_set1_pd(price.ask), price.bid, TpGreaterZero);
        __m256d addPnl = AVXHelper::avxZero;
        uint len = prices.size();
        int removed = 0;
        for (uint i = 0; i < len; i++)
        {
            int idx = i - removed;
            addPnl = _mm256_mul_pd(_mm256_sub_pd(priceE, prices[idx]), sizes[idx]);
            tempPnl = AVXHelper::addMasked(tempPnl, addPnl, toCloseMask);
            tP = AVXHelper::subMasked(tP, sizes[idx], toCloseMask);
            sizes[idx] = AVXHelper::setValues(sizes[idx], AVXHelper::avxZero, toCloseMask);
            mask allClosedMask = _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_NEQ_OS);
            if (AVXHelper::isMaskZero(allClosedMask))
            {
                sizes.erase(sizes.begin() + idx);
                prices.erase(prices.begin() + idx);
                removed++;
            }
            mask decreaseLongMask = AVXHelper::multMasks(_mm256_cmp_pd(increaseLong, AVXHelper::avxZero, _CMP_GT_OS), TpGreaterZero);
            increaseLong = AVXHelper::addMasked(increaseLong, AVXHelper::avxNegOne, AVXHelper::multMasks(toCloseMask, decreaseLongMask));
            mask decreaseShortMask = AVXHelper::invert(decreaseLongMask);
            increaseShort = AVXHelper::addMasked(increaseShort, AVXHelper::avxNegOne, AVXHelper::multMasks(toCloseMask, decreaseShortMask));
        }
        pnl = AVXHelper::addMasked(pnl, tempPnl, toCloseMask);
        pnlPerc = AVXHelper::addMasked(pnlPerc, _mm256_mul_pd(_mm256_div_pd(tempPnl, cashLimit), profitTarget), toCloseMask);
        tempPnl = AVXHelper::setValues(tempPnl, AVXHelper::avxZero, toCloseMask);
        tP = AVXHelper::setValues(tP, AVXHelper::avxZero, toCloseMask);
    }
    result = toCloseMask;
#else
    SERIAL_AVX(avx)
    {
        //cout << dec << "ls: " << longShort << "; tp: " << ((double *)&tP)[avx] << "; tempPnl: " << ((double *)&tempPnl)[avx] << "; computedPnl: " << ((double *)&computedPnl)[avx] << "; cashLimit: " << ((double *)&cashLimit)[avx] << endl;

        if ((((double *)&tempPnl)[avx] + ((double *)&computedPnl)[avx]) / ((double *)&cashLimit)[avx] >= 1)
        {
            double pricE = (((double *)&tP)[avx] > 0.0 ? price.bid : price.ask);
            double addPnl = 0;
            uint len = prices[avx].size();
            for (uint i = 0; i < len; i++)
            {
                addPnl = (pricE - prices[avx].front()) * sizes[avx].front();
                ((double *)&tempPnl)[avx] += addPnl;
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
            ((double *)&tP)[avx] = 0;
            ((long *)&result)[avx] = 0xFFFFFFFFFFFFFFFF;
        }
        else
        {
            ((long *)&result)[avx] = 0x0000000000000000;
        }
    }
#endif
    return result;
}

void CoastlineTrader::assignCashTarget(__m256d maskArg)
{
    __m256d temp = _mm256_mul_pd(_mm256_set1_pd(lastPrice), profitTarget);
    cashLimit = AVXHelper::setValues(cashLimit, temp, maskArg);
}

bool CoastlineTrader::runPriceAsymm(PriceFeedData::Price price, __m256d oppositeInv)
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

    __m256d event = AVXHelper::avxZero;

    __m256d fraction = _mm256_set1_pd(1.0);

    mask liqSmallerZeroPoint5 = _mm256_cmp_pd(liquidity.liq, _mm256_set1_pd(0.5), _CMP_LT_OS);
    mask liqSmallerZeroPoint1 = _mm256_cmp_pd(liquidity.liq, _mm256_set1_pd(0.1), _CMP_LT_OS);
    __m256d size = _mm256_set1_pd(1.0);
    size = AVXHelper::setValues(size, 0.5, liqSmallerZeroPoint5);
    size = AVXHelper::setValues(size, 0.1, liqSmallerZeroPoint1);

    // call runner and set event
    if (longShort == 1)
    {
        __m256d eventR = runner.run(price);
        __m256d eventG00 = runnerG[0][0].run(price);
        __m256d eventG01 = runnerG[0][1].run(price);

        SERIAL_AVX(i)
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

        SERIAL_AVX(i)
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

    mask tryToCloseMask = tryToClose(price);

    IFDEBUG(
        SERIAL_AVX(i) {
            if (((long *)&tryToCloseMask)[i] != 0)
            { /* -- Try to close position -- */
                cout << "Close" << endl;
            }
        });
    mask tryToCloseElse = AVXHelper::invert(tryToCloseMask);

    if (AVXHelper::isMaskZero(tryToCloseElse))
    {
        IFDEBUG(
            __m256d unrealized = computePnlLastPrice();
            SERIAL_AVX(i) {
                cout << "longShort: " << longShort << "; tP: " << ((double *)&tP)[i] << "; pnl: "
                     << ((double *)&pnl)[i] << "; pnlPerc: " << ((double *)&pnlPerc)[i] << "; tempPnl: "
                     << ((double *)&tempPnl)[i] << "; unrealized: " << ((double *)&unrealized)[i] << "; cashLimit: "
                     << ((double *)&cashLimit)[i] << "; price: " << lastPrice << "; runner.type: " << ((double *)&runner.type)[i] << std::endl;
            });
        return true;
    }

    if (longShort == 1)
    { // Long positions only
        mask maskEventSmallerZero = _mm256_cmp_pd(event, AVXHelper::avxZero, _CMP_LT_OS);
        maskEventSmallerZero = AVXHelper::multMasks(tryToCloseElse, maskEventSmallerZero);
        //if (event < 0) -> use maskEventSmallerZero
        if (!AVXHelper::isMaskZero(maskEventSmallerZero))
        {
            __m256d sign = _mm256_mul_pd(runner.type, AVXHelper::avxNegOne);

            __m256d maskTpEqualsZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_EQ_OS);
            maskTpEqualsZero = AVXHelper::multMasks(maskTpEqualsZero, maskEventSmallerZero);
            //if (tP == 0.0) -> use maskTpEqualsZero
            if (!AVXHelper::isMaskZero(maskTpEqualsZero))
            { // Open long position
                __m256d maskOppositeInvAbsGreaterThan15 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(15.0), _CMP_GT_OS);
                maskOppositeInvAbsGreaterThan15 = _mm256_or_pd(maskOppositeInvAbsGreaterThan15, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-15.0), _CMP_LT_OS));
                maskOppositeInvAbsGreaterThan15 = AVXHelper::multMasks(maskTpEqualsZero, maskOppositeInvAbsGreaterThan15);
                //if (std::abs(oppositeInv) > 15.0) -> use maskOppositeInvAbsGreaterThan15
                {
                    size = AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan15);

                    __m256d maskOppositeInvAbsGreaterThan30 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(30.0), _CMP_GT_OS);
                    maskOppositeInvAbsGreaterThan30 = _mm256_or_pd(maskOppositeInvAbsGreaterThan30, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-30.0), _CMP_LT_OS));
                    maskOppositeInvAbsGreaterThan30 = AVXHelper::multMasks(maskOppositeInvAbsGreaterThan30, maskOppositeInvAbsGreaterThan15);
                    //if (std::abs(oppositeInv) > 30.0) -> use maskOppositeInvAbsGreaterThan30
                    {
                        size = AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan30);
                    }
                }
                __m256d sizeToAdd = AVXHelper::setValues(AVXHelper::avxZero, _mm256_mul_pd(sign, size), maskTpEqualsZero);
                tP = AVXHelper::addMasked(tP, sizeToAdd, maskTpEqualsZero);
#ifdef AVX_LIST
                sizes.push_back(sizeToAdd);
                mask signEqualsOne = _mm256_cmp_pd(sign, AVXHelper::avxOne, _CMP_EQ_OS);
                prices.push_back(AVXHelper::setValues(price.bid, price.ask, signEqualsOne));
                IFDEBUG(
                    SERIAL_AVX(i) {
                        if (((double *)&maskTpEqualsZero)[i] != 0)
                        {
                            cout << "Open long" << endl;
                        }
                    });
#else
                SERIAL_AVX(i)
                {
                    if (((double *)&maskTpEqualsZero)[i] != 0)
                    {
                        sizes[i].push_back(((double *)&sizeToAdd)[i]);
                        prices[i].push_back(((double *)&sign)[i] == 1.0 ? price.ask : price.bid);
                        IFDEBUG(cout << "Open long" << endl);
                    }
                }
#endif

                assignCashTarget(maskTpEqualsZero);
            }
            __m256d maskTpGreaterThanZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
            maskTpGreaterThanZero = AVXHelper::multMasks(maskTpGreaterThanZero, AVXHelper::invert(maskTpEqualsZero));
            maskTpGreaterThanZero = AVXHelper::multMasks(maskTpGreaterThanZero, maskEventSmallerZero);
            //else if (tP > 0.0) -> use maskTpGreaterThanZero
            if (!AVXHelper::isMaskZero(maskTpGreaterThanZero))
            { // Increase long position (buy)
                __m256d sizeToAdd = AVXHelper::setValues(AVXHelper::avxZero, AVXHelper::multiply(sign, size, fraction, shrinkFlong), maskTpGreaterThanZero);

                IFDEBUG(
                    SERIAL_AVX(i) {if (((double*)&sizeToAdd)[i] < 0.0 && AVX_DOUBLE(maskTpGreaterThanZero, i) != 0) {
                        cout << "Runner.type: " << AVX_DOUBLE(runner.type, i) << " Sign: " << AVX_DOUBLE(sign, i) << " size: " << AVX_DOUBLE(size, i) << " fraction: " << AVX_DOUBLE(fraction, i) << " shrinkFshort: " << AVX_DOUBLE(shrinkFshort, i) << endl;
                        cout << "How did this happen! increase position but neg size: " << AVX_DOUBLE(sizeToAdd, i) << endl;
                        exit(EXIT_FAILURE);
                    } });

                increaseLong = AVXHelper::addMasked(increaseLong, 1.0, maskTpGreaterThanZero);
                tP = AVXHelper::addMasked(tP, sizeToAdd, maskTpGreaterThanZero);
#ifdef AVX_LIST
                sizes.push_back(sizeToAdd);
                mask signEqualsOne = _mm256_cmp_pd(sign, AVXHelper::avxOne, _CMP_EQ_OS);
                prices.push_back(AVXHelper::setValues(price.bid, price.ask, signEqualsOne));
                IFDEBUG(
                    SERIAL_AVX(i) {
                        if (((double *)&maskTpGreaterThanZero)[i] != 0)
                        {
                            cout << "Cascade" << endl;
                        }
                    });
#else
                SERIAL_AVX(i)
                {
                    if (((double *)&maskTpGreaterThanZero)[i] != 0)
                    {
                        sizes[i].push_back(((double *)&sizeToAdd)[i]);
                        prices[i].push_back(((double *)&sign)[i] == 1.0 ? price.ask : price.bid);
                        IFDEBUG(cout << "Cascade" << endl);
                    }
                }
#endif
            }
        }
        mask maskEventGreaterZeroAndTpGreaterZero = _mm256_cmp_pd(event, AVXHelper::avxZero, _CMP_GT_OS);
        maskEventGreaterZeroAndTpGreaterZero = AVXHelper::multMasks(maskEventGreaterZeroAndTpGreaterZero, _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS));
        maskEventGreaterZeroAndTpGreaterZero = AVXHelper::multMasks(maskEventGreaterZeroAndTpGreaterZero, AVXHelper::invert(maskEventSmallerZero));
        maskEventGreaterZeroAndTpGreaterZero = AVXHelper::multMasks(maskEventGreaterZeroAndTpGreaterZero, tryToCloseElse);
        //else if (event > 0 && tP > 0.0)
        if (!AVXHelper::isMaskZero(maskEventGreaterZeroAndTpGreaterZero))
        { // Possibility to decrease long position only at intrinsic events
            mask tpGreaterZeroMask = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
            __m256d pricE = AVXHelper::setValues(_mm256_set1_pd(price.ask), price.bid, tpGreaterZeroMask);
#ifdef AVX_LIST
            uint len = prices.size();
            int removed = 0;
            __m256d firstElementFound = AVXHelper::avxZero;
            for (uint i = 0; i < len; i++)
            {
                int idx = i - removed;
                mask elementNotZero = _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_NEQ_OS);
                __m256d tempP = AVXHelper::setValues(AVXHelper::avxLogDoubleMasked(_mm256_div_pd(prices[idx], pricE), elementNotZero), AVXHelper::avxLogDoubleMasked(_mm256_div_pd(pricE, prices[idx]), elementNotZero), tpGreaterZeroMask);
                __m256d delta = AVXHelper::setValues(deltaDown, deltaUp, tpGreaterZeroMask);
                mask tempPGreaterDelta = _mm256_cmp_pd(tempP, delta, _CMP_GE_OS);
                tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, maskEventGreaterZeroAndTpGreaterZero);
                tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, elementNotZero);
                if (!AVXHelper::isMaskZero(AVXHelper::invert(firstElementFound)))
                {
                    mask newFirstElement = _mm256_or_pd(firstElementFound, elementNotZero);
                    tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, firstElementFound);
                    firstElementFound = _mm256_or_pd(firstElementFound, newFirstElement);
                }
                if (!AVXHelper::isMaskZero(tempPGreaterDelta))
                {
                    __m256d addPnl = _mm256_mul_pd(_mm256_sub_pd(pricE, prices[idx]), sizes[idx]);
                    IFDEBUG(
                        mask addPnlSmallerThanZero = _mm256_cmp_pd(addPnl, AVXHelper::avxZero, _CMP_LT_OS);
                        addPnlSmallerThanZero = AVXHelper::multMasks(addPnlSmallerThanZero, tempPGreaterDelta);
                        SERIAL_AVX(avx) {
                            if (((double *)&addPnlSmallerThanZero)[avx] != 0)
                            {
                                cout << "Descascade with a loss: " << AVX_DOUBLE(addPnl, avx) << endl;
                            }
                        });
                    tempPnl = AVXHelper::addMasked(tempPnl, addPnl, tempPGreaterDelta);
                    tP = AVXHelper::subMasked(tP, sizes[idx], tempPGreaterDelta);
                    sizes[idx] = AVXHelper::setValues(sizes[idx], AVXHelper::avxZero, tempPGreaterDelta);
                    mask allClosedMask = _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_NEQ_OS);
                    if (AVXHelper::isMaskZero(allClosedMask))
                    {
                        sizes.erase(sizes.begin() + idx);
                        prices.erase(prices.begin() + idx);
                        removed++;
                    }
                    mask decreaseLongMask = _mm256_cmp_pd(increaseLong, AVXHelper::avxZero, _CMP_GT_OS);
                    increaseLong = AVXHelper::addMasked(increaseLong, AVXHelper::avxNegOne, AVXHelper::multMasks(tempPGreaterDelta, decreaseLongMask));
                    IFDEBUG(
                        SERIAL_AVX(avx) {
                            if (((double *)&tempPGreaterDelta)[avx] != 0)
                            {
                                cout << "Decascade" << endl;
                            }
                        });
                }
            }
#else
            SERIAL_AVX(avx)
            {
                if (AVX_DOUBLE(maskEventGreaterZeroAndTpGreaterZero, avx) == 0)
                {
                    continue;
                }
                uint len = prices[avx].size();
                int removed = 0;
                for (uint i = 1; i < len; ++i)
                {
                    int idx = i - removed;
                    double tempP = (AVX_DOUBLE(tP, avx) > 0.0 ? log(AVX_DOUBLE(pricE, avx) / prices[avx].at(idx)) : log(prices[avx].at(idx) / AVX_DOUBLE(pricE, avx)));
                    if (tempP >= (AVX_DOUBLE(tP, avx) > 0.0 ? AVX_DOUBLE(deltaUp, avx) : AVX_DOUBLE(deltaDown, avx)))
                    {
                        double addPnl = (AVX_DOUBLE(pricE, avx) - prices[avx].at(idx)) * sizes[avx].at(idx);
                        if (addPnl < 0.0)
                        {
                            IFDEBUG(cout << "Descascade with a loss: " << addPnl << endl);
                        }
                        AVX_DOUBLE(tempPnl, avx) += addPnl;
                        AVX_DOUBLE(tP, avx) -= sizes[avx].at(idx);
                        sizes[avx].erase(sizes[avx].begin() + idx);
                        prices[avx].erase(prices[avx].begin() + idx);
                        removed++;
                        AVX_DOUBLE(increaseLong, avx) += -1.0;
                        IFDEBUG(cout << "Decascade" << endl);
                    }
                }
            }
#endif
        }
    }
    else if (longShort == -1)
    { // Short positions only
        mask maskEventsGreaterZero = _mm256_cmp_pd(event, AVXHelper::avxZero, _CMP_GT_OS);
        maskEventsGreaterZero = AVXHelper::multMasks(tryToCloseElse, maskEventsGreaterZero);
        // if (event > 0) -> use maskEventsGreaterZero
        if (!AVXHelper::isMaskZero(maskEventsGreaterZero))
        {
            __m256d sign = _mm256_mul_pd(runner.type, _mm256_set1_pd(-1.0));

            __m256d maskTpEqualsZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_EQ_OS);
            maskTpEqualsZero = AVXHelper::multMasks(maskTpEqualsZero, maskEventsGreaterZero);
            // if (tP == 0.0) -> use maskTpEqualsZero
            if (!AVXHelper::isMaskZero(maskTpEqualsZero))
            { // Open short position

                __m256d maskOppositeInvAbsGreaterThan15 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(15.0), _CMP_GT_OS);
                maskOppositeInvAbsGreaterThan15 = _mm256_or_pd(maskOppositeInvAbsGreaterThan15, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-15.0), _CMP_LT_OS));
                maskOppositeInvAbsGreaterThan15 = AVXHelper::multMasks(maskTpEqualsZero, maskOppositeInvAbsGreaterThan15);
                //if (std::abs(oppositeInv) > 15.0) -> use maskOppositeInvAbsGreaterThan15
                {
                    size = AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan15);

                    __m256d maskOppositeInvAbsGreaterThan30 = _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(30.0), _CMP_GT_OS);
                    maskOppositeInvAbsGreaterThan30 = _mm256_or_pd(maskOppositeInvAbsGreaterThan30, _mm256_cmp_pd(oppositeInv, _mm256_set1_pd(-30.0), _CMP_LT_OS));
                    maskOppositeInvAbsGreaterThan30 = AVXHelper::multMasks(maskOppositeInvAbsGreaterThan30, maskOppositeInvAbsGreaterThan15);
                    //if (std::abs(oppositeInv) > 30.0) -> use maskOppositeInvAbsGreaterThan30
                    {
                        size = AVXHelper::setValues(size, 1.0, maskOppositeInvAbsGreaterThan30);
                    }
                }

                __m256d sizeToAdd = AVXHelper::setValues(AVXHelper::avxZero, _mm256_mul_pd(sign, size), maskTpEqualsZero);
                tP = _mm256_add_pd(tP, sizeToAdd);

#ifdef AVX_LIST
                sizes.push_back(sizeToAdd);
                mask signEqualsOne = _mm256_cmp_pd(sign, AVXHelper::avxOne, _CMP_EQ_OS);
                prices.push_back(AVXHelper::setValues(price.ask, price.bid, signEqualsOne));
                IFDEBUG(
                    SERIAL_AVX(i) {
                        if (((double *)&maskTpEqualsZero)[i] != 0)
                        {
                            cout << "Open short" << endl;
                        }
                    });
#else
                SERIAL_AVX(i)
                {
                    if (((double *)&maskTpEqualsZero)[i] != 0)
                    {
                        sizes[i].push_back(((double *)&sizeToAdd)[i]);
                        prices[i].push_back(((double *)&sign)[i] == 1.0 ? price.bid : price.ask);
                        IFDEBUG(cout << "Open short" << endl);
                    }
                }
#endif

                assignCashTarget(maskTpEqualsZero);
            }
            __m256d maskTpLessThanZero = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_LT_OS);
            maskTpLessThanZero = AVXHelper::multMasks(maskTpLessThanZero, AVXHelper::invert(maskTpEqualsZero));
            maskTpLessThanZero = AVXHelper::multMasks(maskTpLessThanZero, maskEventsGreaterZero);
            // else if (tP < 0.0) -> use maskTpLessThanZero
            if (!AVXHelper::isMaskZero(maskTpLessThanZero))
            {
                __m256d sizeToAdd = AVXHelper::setValues(AVXHelper::avxZero, AVXHelper::multiply(sign, size, fraction, shrinkFshort), maskTpLessThanZero);
                IFDEBUG(
                    SERIAL_AVX(i) {if (((double*)&sizeToAdd)[i] > 0.0 && AVX_DOUBLE(maskTpLessThanZero, i) != 0) {
                        cout << "Runner.type: " << AVX_DOUBLE(runner.type, i) << " Sign: " << AVX_DOUBLE(sign, i) << " size: " << AVX_DOUBLE(size, i) << " fraction: " << AVX_DOUBLE(fraction, i) << " shrinkFshort: " << AVX_DOUBLE(shrinkFshort, i) << endl;
                        cout << "How did this happen! increase position but pos size: " << AVX_DOUBLE(sizeToAdd, i) << endl;
                        exit(EXIT_FAILURE);
                    } });

                increaseShort = AVXHelper::addMasked(increaseShort, 1.0, maskTpLessThanZero);
                tP = AVXHelper::addMasked(tP, sizeToAdd, maskTpLessThanZero);

#ifdef AVX_LIST
                sizes.push_back(sizeToAdd);
                mask signEqualsOne = _mm256_cmp_pd(sign, AVXHelper::avxOne, _CMP_EQ_OS);
                prices.push_back(AVXHelper::setValues(price.ask, price.bid, signEqualsOne));
                IFDEBUG(
                    SERIAL_AVX(i) {
                        if (((double *)&maskTpLessThanZero)[i] != 0)
                        {
                            cout << "Cascade" << endl;
                        }
                    });
#else
                SERIAL_AVX(i)
                {
                    if (((double *)&maskTpLessThanZero)[i] != 0)
                    {
                        sizes[i].push_back(((double *)&sizeToAdd)[i]);
                        prices[i].push_back(((double *)&sign)[i] == 1.0 ? price.bid : price.ask);
                        IFDEBUG(cout << "Cascade" << endl);
                    }
                }
#endif
            }
        }
        mask maskEventLessZeroAndTpLessZero = _mm256_cmp_pd(event, AVXHelper::avxZero, _CMP_LT_OS);
        maskEventLessZeroAndTpLessZero = AVXHelper::multMasks(maskEventLessZeroAndTpLessZero, _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_LT_OS));
        maskEventLessZeroAndTpLessZero = AVXHelper::multMasks(maskEventLessZeroAndTpLessZero, AVXHelper::invert(maskEventsGreaterZero));
        maskEventLessZeroAndTpLessZero = AVXHelper::multMasks(maskEventLessZeroAndTpLessZero, tryToCloseElse);
        // else if (event < 0.0 && tP < 0.0) -> use maskEventLessZeroAndTpLessZero
        if (!AVXHelper::isMaskZero(maskEventLessZeroAndTpLessZero))
        {
            mask tpGreaterZeroMask = _mm256_cmp_pd(tP, AVXHelper::avxZero, _CMP_GT_OS);
            __m256d pricE = AVXHelper::setValues(_mm256_set1_pd(price.ask), price.bid, tpGreaterZeroMask);
#ifdef AVX_LIST
            uint len = prices.size();
            int removed = 0;
            __m256d firstElementFound = AVXHelper::avxZero;
            for (uint i = 0; i < len; i++)
            {
                int idx = i - removed;
                mask elementNotZero = _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_NEQ_OS);
                __m256d tempP = AVXHelper::setValues(AVXHelper::avxLogDoubleMasked(_mm256_div_pd(prices[idx], pricE), elementNotZero), AVXHelper::avxLogDoubleMasked(_mm256_div_pd(pricE, prices[idx]), elementNotZero), tpGreaterZeroMask);
                __m256d delta = AVXHelper::setValues(deltaDown, deltaUp, tpGreaterZeroMask);
                mask tempPGreaterDelta = _mm256_cmp_pd(tempP, delta, _CMP_GE_OS);
                tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, maskEventLessZeroAndTpLessZero);
                tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, elementNotZero);
                if (!AVXHelper::isMaskZero(AVXHelper::invert(firstElementFound)))
                {
                    mask newFirstElement = _mm256_or_pd(firstElementFound, elementNotZero);
                    tempPGreaterDelta = AVXHelper::multMasks(tempPGreaterDelta, firstElementFound);
                    firstElementFound = _mm256_or_pd(firstElementFound, newFirstElement);
                }
                if (!AVXHelper::isMaskZero(tempPGreaterDelta))
                {
                    __m256d addPnl = _mm256_mul_pd(_mm256_sub_pd(pricE, prices[idx]), sizes[idx]);
                    IFDEBUG(
                        mask addPnlSmallerThanZero = _mm256_cmp_pd(addPnl, AVXHelper::avxZero, _CMP_LT_OS);
                        addPnlSmallerThanZero = AVXHelper::multMasks(addPnlSmallerThanZero, tempPGreaterDelta);
                        addPnlSmallerThanZero = AVXHelper::multMasks(addPnlSmallerThanZero, _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_LT_OS));
                        SERIAL_AVX(avx) {
                            if (((double *)&addPnlSmallerThanZero)[avx] != 0)
                            {
                                cout << "Descascade with a loss: " << AVX_DOUBLE(addPnl, avx) << endl;
                            }
                        });
                    tempPnl = AVXHelper::addMasked(tempPnl, addPnl, tempPGreaterDelta);
                    tP = AVXHelper::subMasked(tP, sizes[idx], tempPGreaterDelta);
                    sizes[idx] = AVXHelper::setValues(sizes[idx], AVXHelper::avxZero, tempPGreaterDelta);
                    mask allClosedMask = _mm256_cmp_pd(sizes[idx], AVXHelper::avxZero, _CMP_NEQ_OS);
                    if (AVXHelper::isMaskZero(allClosedMask))
                    {
                        sizes.erase(sizes.begin() + idx);
                        prices.erase(prices.begin() + idx);
                        removed++;
                    }
                    mask decreaseShortMask = _mm256_cmp_pd(increaseShort, AVXHelper::avxZero, _CMP_GT_OS);
                    increaseShort = AVXHelper::addMasked(increaseShort, AVXHelper::avxNegOne, AVXHelper::multMasks(tempPGreaterDelta, decreaseShortMask));
                    IFDEBUG(
                        SERIAL_AVX(avx) {
                            if (((double *)&tempPGreaterDelta)[avx] != 0)
                            {
                                cout << "Decascade" << endl;
                            }
                        });
                }
            }
#else
            SERIAL_AVX(avx)
            {
                if (AVX_DOUBLE(maskEventLessZeroAndTpLessZero, avx) == 0)
                {
                    continue;
                }
                int removed = 0;
                uint len = prices[avx].size();
                for (uint i = 1; i < len; ++i)
                {
                    int idx = i - removed;
                    double tempP = (AVX_DOUBLE(tP, avx) > 0.0 ? log(AVX_DOUBLE(pricE, avx) / prices[avx].at(idx)) : log(prices[avx].at(idx) / AVX_DOUBLE(pricE, avx)));
                    if (tempP >= (AVX_DOUBLE(tP, avx) > 0.0 ? AVX_DOUBLE(deltaUp, avx) : AVX_DOUBLE(deltaDown, avx)))
                    {
                        double addPnl = (AVX_DOUBLE(pricE, avx) - prices[avx].at(idx)) * sizes[avx].at(idx);
                        if (addPnl < 0.0)
                        {
                            IFDEBUG(cout << "Descascade with a loss: " << addPnl << endl);
                        }
                        AVX_DOUBLE(tempPnl, avx) += (AVX_DOUBLE(pricE, avx) - prices[avx].at(idx)) * sizes[avx].at(idx);
                        AVX_DOUBLE(tP, avx) -= sizes[avx].at(idx);
                        sizes[avx].erase(sizes[avx].begin() + idx);
                        prices[avx].erase(prices[avx].begin() + idx);
                        removed++;
                        AVX_DOUBLE(increaseShort, avx) += -1.0;
                        IFDEBUG(cout << "Decascade" << endl);
                    }
                }
            }
#endif
        }
    }
    else
    {
        cout << "Should never happen! " << longShort << endl;
    }
    //some prints
    IFDEBUG(
        __m256d unrealized = computePnlLastPrice();
        SERIAL_AVX(i) {
            cout << "longShort: " << longShort << "; tP: " << ((double *)&tP)[i] << "; pnl: "
                 << ((double *)&pnl)[i] << "; pnlPerc: " << ((double *)&pnlPerc)[i] << "; tempPnl: "
                 << ((double *)&tempPnl)[i] << "; unrealized: " << ((double *)&unrealized)[i] << "; cashLimit: "
                 << ((double *)&cashLimit)[i] << "; price: " << lastPrice << "; runner.type: " << ((double *)&runner.type)[i] << std::endl;
        });
    return true;
}