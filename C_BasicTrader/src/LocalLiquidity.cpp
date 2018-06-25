#include "helper/Macros.h"

#include "LocalLiquidity.h"
#include "PriceFeedData.h"
#include "AVXHelper.h"

#include <x86intrin.h>
#include <math.h>
#include <stdlib.h>

using namespace std;

LocalLiquidity::LocalLiquidity()
{
}

LocalLiquidity::LocalLiquidity(__m256d deltaArg, __m256d deltaUpArg, __m256d deltaDownArg, __m256d dStarArg, double alphaArg) : deltaUp(deltaUpArg), deltaDown(deltaDownArg), delta(deltaArg), dStar(dStarArg), alpha(alphaArg)
{
    type = _mm256_set1_pd(-1);
    initalized = false;
    alphaWeight = exp(-2.0 / (alpha + 1.0));
    computeH1H2exp(dStar);
    liq = _mm256_set1_pd(0.0);
    upLiq = _mm256_set1_pd(0.0);
    downLiq = _mm256_set1_pd(0.0);
    surp = _mm256_set1_pd(0.0);
    downSurp = _mm256_set1_pd(0.0);
    upSurp = _mm256_set1_pd(0.0);
}

LocalLiquidity::LocalLiquidity(__m256d deltaArg, __m256d deltaUpArg, __m256d deltaDownArg, PriceFeedData::Price priceArg, __m256d dStarArg, double alphaArg) : deltaUp(deltaUpArg), deltaDown(deltaDownArg), delta(deltaArg), dStar(dStarArg), alpha(alphaArg)
{
    type = _mm256_set1_pd(-1);
    extreme = reference = _mm256_set1_pd(priceArg.mid);
    initalized = true;
    alphaWeight = exp(-2.0 / (alpha + 1.0));
    computeH1H2exp(dStar);
    liq = _mm256_set1_pd(0.0);
    upLiq = _mm256_set1_pd(0.0);
    downLiq = _mm256_set1_pd(0.0);
    surp = _mm256_set1_pd(0.0);
    downSurp = _mm256_set1_pd(0.0);
    upSurp = _mm256_set1_pd(0.0);
}

bool LocalLiquidity::computeH1H2exp(__m256d dS)
{
    UNUSED(dS);
    // exp(-dStar / delta)
    __m256d expDStarDivDelta = AVXHelper::avxExpDouble(_mm256_div_pd(_mm256_mul_pd(dStar, AVXHelper::avxNegOne), delta));
    // (1.0 - exp(-dStar / delta))
    __m256d oneMinusExpDStarDivDelta = _mm256_sub_pd(_mm256_set1_pd(1.0), expDStarDivDelta);
    // H1 = -1 * exp(-dStar / delta) * log(exp(-dStar / delta)) - (1.0 - exp(-dStar / delta)) * log(1.0 - exp(-dStar / delta));
    H1 = _mm256_mul_pd(_mm256_mul_pd(expDStarDivDelta, AVXHelper::avxNegOne), AVXHelper::avxLogDouble(expDStarDivDelta));
    H1 = _mm256_add_pd(H1, _mm256_mul_pd(oneMinusExpDStarDivDelta, AVXHelper::avxLogDouble(oneMinusExpDStarDivDelta)));
    // H2 = exp(-dStar / delta) * pow(log(exp(-dStar / delta)), 2.0) - (1.0 - exp(-dStar / delta)) * pow(log(1.0 - exp(-dStar / delta)), 2.0) - H1 * H1;
    H2 = _mm256_mul_pd(expDStarDivDelta, AVXHelper::avxPowDouble(AVXHelper::avxLogDouble(expDStarDivDelta), 2.0));
    H2 = _mm256_sub_pd(H2, _mm256_mul_pd(oneMinusExpDStarDivDelta, AVXHelper::avxPowDouble(AVXHelper::avxLogDouble(oneMinusExpDStarDivDelta), 2.0)));
    H2 = _mm256_sub_pd(H2, _mm256_mul_pd(H1, H1));
    return true;
}

// Another implementation of the CNDF for a standard normal: N(0,1)
__m256d LocalLiquidity::CumNorm(__m256d x)
{
    __m256d returnValues = AVXHelper::avxZero;

    // protect against overflow
    // if (x >  6.0)
    __m256d maskOverflow = _mm256_cmp_pd(x, _mm256_set1_pd(6.0), _CMP_GT_OS);
    // if (x < -6.0)
    __m256d maskUnderflow = _mm256_cmp_pd(x, _mm256_set1_pd(-6.0), _CMP_LT_OS);

    // if (-6.0 <= x <= 6.0)
    __m256d maskOverAndUnderflow = _mm256_or_pd(maskOverflow, maskUnderflow);

    __m256d restX = AVXHelper::setValues(x, 0.0, maskOverAndUnderflow);

    __m256d b1 = _mm256_set1_pd(0.31938153);
    __m256d b2 = _mm256_set1_pd(-0.356563782);
    __m256d b3 = _mm256_set1_pd(1.781477937);
    __m256d b4 = _mm256_set1_pd(-1.821255978);
    __m256d b5 = _mm256_set1_pd(1.330274429);
    __m256d p = _mm256_set1_pd(0.2316419);
    __m256d c2 = _mm256_set1_pd(0.3989423);

    // double a = abs(x);
    __m256d maskSmallerThan0 = _mm256_cmp_pd(restX, _mm256_set1_pd(0.0), _CMP_LT_OS);
    __m256d negatedX = _mm256_mul_pd(restX, _mm256_set1_pd(-1));
    __m256d a = AVXHelper::setValues(restX, negatedX, maskSmallerThan0);

    // double t = 1.0 / (1.0 + a * p);
    __m256d t = _mm256_div_pd(AVXHelper::avxOne, _mm256_add_pd(AVXHelper::avxOne, _mm256_mul_pd(a, p)));

    // double b = c2 * exp((-x) * (x / 2.0));
    __m256d b = _mm256_mul_pd(c2, AVXHelper::avxExpDouble(_mm256_mul_pd(negatedX, _mm256_div_pd(restX, _mm256_set1_pd(2.0)))));

    // double n = ((((b5 * t + b4) * t + b3) * t + b2) * t + b1) * t;
    __m256d n = _mm256_mul_pd(_mm256_add_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_mul_pd(b5, t), b4), t), b3), t), b2), t), b1), t);

    // n = 1.0 - b * n;
    n = _mm256_sub_pd(AVXHelper::avxOne, _mm256_mul_pd(b, n));

    // if (x < 0.0)
    //     n = 1.0 - n;
    n = AVXHelper::setValues(n, _mm256_sub_pd(AVXHelper::avxOne, n), maskSmallerThan0);

    returnValues = n;
    returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxOne, maskOverflow);
    returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxZero, maskUnderflow);

    return returnValues;
}

__m256d LocalLiquidity::run(PriceFeedData::Price price)
{
    __m256d returnValues = AVXHelper::avxZero;

    if (!initalized)
    {
        type = AVXHelper::avxNegOne;
        initalized = true;
        extreme = reference = _mm256_set1_pd(price.mid);
        return returnValues;
    }

    __m256d mask1 = _mm256_cmp_pd(type, AVXHelper::avxNegOne, _CMP_EQ_OS);
    /* if (type == -1) */
    if (!AVXHelper::isMaskZero(mask1))
    {
        __m256d tmp = _mm256_div_pd(_mm256_set1_pd(price.bid), extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        __m256d mask11 = _mm256_cmp_pd(tmp, deltaUp, _CMP_GE_OS);
        mask11 = AVXHelper::multMasks(mask11, mask1);
        /*     if (log(price.bid / extreme) >= deltaUp) */
        if (!AVXHelper::isMaskZero(mask11))
        {
            type = AVXHelper::setValues(type, AVXHelper::avxOne, mask11);                 /*type = 1; */
            extreme = AVXHelper::setValues(extreme, price.ask, mask11);                   /* extreme = price.ask; */
            reference = AVXHelper::setValues(reference, price.ask, mask11);               /* reference = price.ask; */
            returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxOne, mask11); /* return 1; */
        }
        __m256d mask12 = _mm256_cmp_pd(_mm256_set1_pd(price.ask), extreme, _CMP_LT_OS);
        mask12 = AVXHelper::multMasks(mask12, AVXHelper::invert(mask11));
        mask12 = AVXHelper::multMasks(mask12, mask1);
        /*     if (price.ask < extreme) */
        if (!AVXHelper::isMaskZero(mask12))
        {
            extreme = AVXHelper::setValues(extreme, price.ask, mask12); /* extreme = price.ask; */
        }
        tmp = _mm256_div_pd(reference, extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        __m256d mask13 = _mm256_cmp_pd(tmp, dStar, _CMP_GE_OS);
        mask13 = AVXHelper::multMasks(mask13, AVXHelper::invert(mask11));
        mask13 = AVXHelper::multMasks(mask13, mask1);
        /*     if (log(reference / extreme) >= dStar) */
        if (!AVXHelper::isMaskZero(mask13))
        {
            reference = AVXHelper::setValues(reference, extreme, mask13);   /* reference = extreme; */
            returnValues = AVXHelper::setValues(returnValues, 2.0, mask13); /* return 2; */
        }
    }
    __m256d mask2 = AVXHelper::multMasks(_mm256_cmp_pd(type, AVXHelper::avxOne, _CMP_EQ_OS), AVXHelper::invert(mask1));
    /* else if (type == 1) */
    if (!AVXHelper::isMaskZero(mask2))
    {
        __m256d tmp = _mm256_div_pd(_mm256_set1_pd(price.ask), extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        __m256d mask21 = _mm256_cmp_pd(tmp, _mm256_mul_pd(deltaDown, AVXHelper::avxNegOne), _CMP_LE_OS);
        mask21 = AVXHelper::multMasks(mask21, mask2);
        /*     if (log(price.ask / extreme) <= -deltaDown) */
        if (!AVXHelper::isMaskZero(mask21))
        {
            type = AVXHelper::setValues(type, AVXHelper::avxNegOne, mask21);                 /* type = -1; */
            extreme = AVXHelper::setValues(extreme, price.bid, mask21);                      /* extreme = price.bid; */
            reference = AVXHelper::setValues(reference, price.bid, mask21);                  /* reference = price.bid; */
            returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxNegOne, mask21); /* return -1; */
        }
        __m256d mask22 = _mm256_cmp_pd(_mm256_set1_pd(price.bid), extreme, _CMP_GT_OS);
        mask22 = AVXHelper::multMasks(mask22, AVXHelper::invert(mask21));
        mask22 = AVXHelper::multMasks(mask22, mask2);
        /*     if (price.bid > extreme) */
        if (!AVXHelper::isMaskZero(mask22))
        {
            extreme = AVXHelper::setValues(extreme, price.bid, mask22); /* extreme = price.bid; */
        }
        tmp = _mm256_div_pd(reference, extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        __m256d mask23 = _mm256_cmp_pd(tmp, _mm256_mul_pd(dStar, AVXHelper::avxNegOne), _CMP_LE_OS);
        mask23 = AVXHelper::multMasks(mask23, AVXHelper::invert(mask21));
        mask23 = AVXHelper::multMasks(mask23, mask2);
        /*     if (log(reference / extreme) <= -dStar) */
        if (!AVXHelper::isMaskZero(mask23))
        {
            reference = AVXHelper::setValues(reference, extreme, mask23);    /* reference = extreme; */
            returnValues = AVXHelper::setValues(returnValues, -2.0, mask23); /* return -2; */
        }
    }
    return returnValues;
}

bool LocalLiquidity::computation(PriceFeedData::Price price)
{
    __m256d event = run(price);

    __m256d mask1 = _mm256_cmp_pd(event, _mm256_set1_pd(0.0), _CMP_NEQ_OS);
    //if (event != 0)
    if (!AVXHelper::isMaskZero(mask1))
    {
        // if (event == 1)
        __m256d maskOne = _mm256_cmp_pd(event, _mm256_set1_pd(1.0), _CMP_EQ_OS);

        // if (event == -1)
        __m256d maskNegOne = _mm256_cmp_pd(event, _mm256_set1_pd(-1.0), _CMP_EQ_OS);

        // if (abs(event) == 1)
        __m256d maskabsOne = _mm256_or_pd(maskOne, maskNegOne);

        // surp = alphaWeight * (abs(event) == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight) * surp;
        __m256d eventEqAbsOneMultiplier = AVXHelper::setValues(_mm256_set1_pd(2.525729), _mm256_set1_pd(0.08338161), maskabsOne);
        __m256d tmp = _mm256_mul_pd(_mm256_set1_pd(alphaWeight), eventEqAbsOneMultiplier);
        tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_set1_pd(1.0 - alphaWeight), surp));
        surp = AVXHelper::setValues(surp, tmp, mask1);

        __m256d mask11 = _mm256_cmp_pd(event, _mm256_set1_pd(0.0), _CMP_GT_OS);
        mask11 = _mm256_add_pd(mask11, mask1);
        //if (event > 0)
        if (!AVXHelper::isMaskZero(mask11))
        {
            // downSurp = alphaWeight * (event == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight) * downSurp;
            __m256d eventEqOneMultiplier = AVXHelper::setValues(_mm256_set1_pd(2.525729), _mm256_set1_pd(0.08338161), maskOne);
            tmp = _mm256_mul_pd(_mm256_set1_pd(alphaWeight), eventEqOneMultiplier);
            tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_set1_pd(1.0 - alphaWeight), downSurp));
            downSurp = AVXHelper::setValues(downSurp, tmp, mask11);
        }
        __m256d mask12 = _mm256_cmp_pd(event, _mm256_set1_pd(0.0), _CMP_LT_OS);
        mask12 = AVXHelper::multMasks(mask12, AVXHelper::invert(mask11));
        mask12 = _mm256_add_pd(mask12, mask1);
        //if (event < 0)
        if (!AVXHelper::isMaskZero(mask12))
        {
            // upSurp = alphaWeight * (event == -1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight) * upSurp;
            __m256d eventEqNegOneMultiplier = AVXHelper::setValues(_mm256_set1_pd(2.525729), _mm256_set1_pd(0.08338161), maskNegOne);
            tmp = _mm256_mul_pd(_mm256_set1_pd(alphaWeight), eventEqNegOneMultiplier);
            tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_set1_pd(1.0 - alphaWeight), upSurp));
            upSurp = AVXHelper::setValues(upSurp, tmp, mask12);
        }

        // 1.0 / sqrt(H2)
        __m256d oneDivSqrtH2 = _mm256_div_pd(AVXHelper::avxOne, _mm256_sqrt_pd(H2));

        // liq = 1.0 - CumNorm(sqrt(alpha) * (surp - H1) / sqrt(H2));
        __m256d surpArgument = _mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(sqrt(alpha)), _mm256_sub_pd(surp, H1)), oneDivSqrtH2);
        liq = AVXHelper::setValues(liq, _mm256_sub_pd(_mm256_set1_pd(1.0), CumNorm(surpArgument)), mask1);
        // upLiq = 1.0 - CumNorm(sqrt(alpha) * (upSurp - H1) / sqrt(H2));
        __m256d upSurpArgument = _mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(sqrt(alpha)), _mm256_sub_pd(upSurp, H1)), oneDivSqrtH2);
        upLiq = AVXHelper::setValues(upLiq, _mm256_sub_pd(_mm256_set1_pd(1.0), CumNorm(upSurpArgument)), mask1);
        // downLiq = 1.0 - CumNorm(sqrt(alpha) * (downSurp - H1) / sqrt(H2));
        __m256d downSurpArgument = _mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(sqrt(alpha)), _mm256_sub_pd(downSurp, H1)), oneDivSqrtH2);
        downLiq = AVXHelper::setValues(downLiq, _mm256_sub_pd(_mm256_set1_pd(1.0), CumNorm(downSurpArgument)), mask1);
    }

    return true;
}