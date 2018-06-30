#include "Runner.h"

#include "PriceFeedData.h"
#include "AVXHelper.h"
#include <math.h>
#include <string>
#include <x86intrin.h>

Runner::Runner()
{
}

Runner::Runner(__m256d threshUp, __m256d threshDown, PriceFeedData::Price price, std::string file, __m256d dStarUp, __m256d dStarDown) : deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    prevExtreme = _mm256_set1_pd(price.mid);
    prevExtremeTime = _mm256_set1_epi64x(price.time);

    prevDC = _mm256_set1_pd(price.mid);
    prevDCTime = _mm256_set1_epi64x(price.time);

    extreme = _mm256_set1_pd(price.mid);
    extremeTime = _mm256_set1_epi64x(price.time);

    reference = _mm256_set1_pd(price.mid);

    type = _mm256_set1_pd(-1.0);
    osL = _mm256_setzero_pd();
    initalized = true;
    fileName = std::string(file);
}

Runner::Runner(__m256d threshUp, __m256d threshDown, double price, std::string file, __m256d dStarUp, __m256d dStarDown) : deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    prevExtreme = _mm256_set1_pd(price);
    prevExtremeTime = _mm256_set1_epi64x(0);

    prevDC = _mm256_set1_pd(price);
    prevDCTime = _mm256_set1_epi64x(0);

    extreme = _mm256_set1_pd(price);
    extremeTime = _mm256_set1_epi64x(0);

    reference = _mm256_set1_pd(price);

    type = _mm256_set1_pd(-1.0);
    osL = _mm256_setzero_pd();
    initalized = true;
    fileName = std::string(file);
}

Runner::Runner(__m256d threshUp, __m256d threshDown, std::string file, __m256d dStarUp, __m256d dStarDown) : deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    initalized = false;
    fileName = std::string(file);
}

__m256d Runner::run(PriceFeedData::Price price)
{
    //TODO: search for better way...
    // if( &price == NULL )
    //     return 0;

    __m256d returnValues = _mm256_set1_pd(0.0);

    if (!initalized)
    {
        type = _mm256_set1_pd(-1.0);
        osL = _mm256_setzero_pd();
        initalized = true;
        prevExtreme = _mm256_set1_pd(price.mid);
        prevExtremeTime = _mm256_set1_epi64x(price.time);
        prevDC = _mm256_set1_pd(price.mid);
        prevDCTime = _mm256_set1_epi64x(price.time);
        extreme = _mm256_set1_pd(price.mid);
        extremeTime = _mm256_set1_epi64x(price.time);
        reference = _mm256_set1_pd(price.mid);

        return returnValues;
    }

    /* ########## Set Masks ########## */

    mask mask1 = _mm256_cmp_pd_mask(type, AVXHelper::avxNegOne, _CMP_EQ_OS);
    /* if (type == -1) */
    if (!AVXHelper::isMaskZero(mask1))
    {
        __m256d tmp = _mm256_div_pd(_mm256_set1_pd(price.bid), extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        mask mask11 = _mm256_cmp_pd_mask(tmp, deltaUp, _CMP_GE_OS);
        mask11 = AVXHelper::multMasks(mask11, mask1);
        /*     if (log(price.bid / extreme) >= deltaUp) */
        if (!AVXHelper::isMaskZero(mask11))
        {
            prevExtreme = AVXHelper::setValues(prevExtreme, extreme, mask11);             /* prevExtreme = extreme; */
            prevExtremeTime = AVXHelper::setValues(prevExtremeTime, extremeTime, mask11); /* prevExtremeTime = extremeTime; */
            type = AVXHelper::setValues(type, AVXHelper::avxOne, mask11);                 /* type = 1; */
            extreme = AVXHelper::setValues(extreme, price.ask, mask11);                   /* extreme = price.ask; */
            extremeTime = AVXHelper::setValues(extremeTime, price.time, mask11);          /* extremeTime = price.time; */
            prevDC = AVXHelper::setValues(prevDC, price.ask, mask11);                     /* prevDC = price.ask; */
            prevDCTime = AVXHelper::setValues(prevDCTime, price.time, mask11);            /* prevDCTime = price.time; */
            reference = AVXHelper::setValues(reference, price.ask, mask11);               /* reference = price.ask; */
            returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxOne, mask11); /* return 1; */
        }
        mask mask12 = _mm256_cmp_pd_mask(_mm256_set1_pd(price.ask), extreme, _CMP_LT_OS);
        mask12 = AVXHelper::multMasks(mask12, AVXHelper::invert(mask11));
        mask12 = AVXHelper::multMasks(mask12, mask1);
        /*     if (price.ask < extreme) */
        if (!AVXHelper::isMaskZero(mask12))
        {
            extreme = AVXHelper::setValues(extreme, price.ask, mask12);          /* extreme = price.ask; */
            extremeTime = AVXHelper::setValues(extremeTime, price.time, mask12); /* extremeTime = price.time; */
            tmp = _mm256_div_pd(extreme, prevDC);
            tmp = AVXHelper::avxLogDouble(tmp);
            tmp = _mm256_div_pd(tmp, deltaDown);
            tmp = _mm256_mul_pd(tmp, AVXHelper::avxNegOne);
            osL = AVXHelper::setValues(osL, tmp, mask12);                   /* osL = -1 * log(extreme / prevDC) / deltaDown; */
            returnValues = AVXHelper::setValues(returnValues, 0.0, mask12); /* return 0; */

            tmp = _mm256_div_pd(extreme, reference);
            tmp = AVXHelper::avxLogDouble(tmp);
            mask mask121 = _mm256_cmp_pd_mask(tmp, _mm256_mul_pd(deltaStarUp, AVXHelper::avxNegOne), _CMP_LE_OS);
            mask121 = AVXHelper::multMasks(mask121, mask12);
            /*         if (log(extreme / reference) <= -deltaStarUp) */
            if (!AVXHelper::isMaskZero(mask121))
            {
                reference = AVXHelper::setValues(reference, extreme, mask121);    /* reference = extreme; */
                returnValues = AVXHelper::setValues(returnValues, -2.0, mask121); /* return -2; */
            }
        }
    }
    mask mask2 = AVXHelper::multMasks(_mm256_cmp_pd_mask(type, AVXHelper::avxOne, _CMP_EQ_OS), AVXHelper::invert(mask1)); /* else if (type == 1) */
    /* else if (type == 1) */
    if (!AVXHelper::isMaskZero(mask2))
    {
        __m256d tmp = _mm256_div_pd(_mm256_set1_pd(price.ask), extreme);
        tmp = AVXHelper::avxLogDouble(tmp);
        mask mask21 = _mm256_cmp_pd_mask(tmp, _mm256_mul_pd(deltaDown, AVXHelper::avxNegOne), _CMP_LE_OS);
        mask21 = AVXHelper::multMasks(mask21, mask2);
        /*     if (log(price.ask / extreme) <= -deltaDown) */
        if (!AVXHelper::isMaskZero(mask21))
        {
            prevExtreme = AVXHelper::setValues(prevExtreme, extreme, mask21);                /* prevExtreme = extreme; */
            prevExtremeTime = AVXHelper::setValues(prevExtremeTime, extremeTime, mask21);    /* prevExtremeTime = extremeTime; */
            type = AVXHelper::setValues(type, AVXHelper::avxNegOne, mask21);                 /* type = -1; */
            extreme = AVXHelper::setValues(extreme, price.bid, mask21);                      /* extreme = price.bid; */
            extremeTime = AVXHelper::setValues(extremeTime, price.time, mask21);             /* extremeTime = price.time; */
            prevDC = AVXHelper::setValues(prevDC, price.bid, mask21);                        /* prevDC = price.bid; */
            prevDCTime = AVXHelper::setValues(prevDCTime, price.time, mask21);               /* prevDCTime = price.time; */
            reference = AVXHelper::setValues(reference, price.bid, mask21);                  /* reference = price.bid; */
            returnValues = AVXHelper::setValues(returnValues, AVXHelper::avxNegOne, mask21); /* return -1; */
        }
        mask mask22 = _mm256_cmp_pd_mask(_mm256_set1_pd(price.bid), extreme, _CMP_GT_OS);
        mask22 = AVXHelper::multMasks(mask22, AVXHelper::invert(mask21));
        mask22 = AVXHelper::multMasks(mask22, mask2);
        /*     if (price.bid > extreme) */
        if (!AVXHelper::isMaskZero(mask22))
        {
            extreme = AVXHelper::setValues(extreme, price.bid, mask22);          /* extreme = price.bid; */
            extremeTime = AVXHelper::setValues(extremeTime, price.time, mask22); /* extremeTime = price.time; */
            tmp = _mm256_div_pd(extreme, prevDC);
            tmp = AVXHelper::avxLogDouble(tmp);
            tmp = _mm256_div_pd(tmp, deltaUp);
            osL = AVXHelper::setValues(osL, tmp, mask22);                   /* osL = log(extreme / prevDC) / deltaUp; */
            returnValues = AVXHelper::setValues(returnValues, 0.0, mask22); /* return 0; */

            tmp = _mm256_div_pd(extreme, reference);
            tmp = AVXHelper::avxLogDouble(tmp);
            mask mask221 = _mm256_cmp_pd_mask(tmp, deltaStarDown, _CMP_GE_OS);
            mask221 = AVXHelper::multMasks(mask221, mask22);
            /*         if (log(extreme / reference) >= deltaStarDown) */
            if (!AVXHelper::isMaskZero(mask221))
            {
                reference = AVXHelper::setValues(reference, extreme, mask221);   /* reference = extreme; */
                returnValues = AVXHelper::setValues(returnValues, 2.0, mask221); /* return 2; */
            }
        }
    }
    /* else */
    //__m256d mask3 = _mm256_and_pd(mask1, mask2);

    /* else */
    //returnValues = AVXHelper::setValues(returnValues, 0.0, mask3); /* return 0; */

    return returnValues;
}

__m256d Runner::run(double price)
{
    return run(PriceFeedData::Price(price, price, 0));
}