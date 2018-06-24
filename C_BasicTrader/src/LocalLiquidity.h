#ifndef LOCAL_LIQUIDITY_H
#define LOCAL_LIQUIDITY_H

#include "PriceFeedData.h"
#include "AVXHelper.h"

class LocalLiquidity
{
  public:
    __m256d deltaUp, deltaDown;
    __m256d delta;
    __m256d extreme, dStar, reference;
    __m256d type;
    bool initalized = false;

    __m256d surp, upSurp, downSurp = 0;
    __m256d liq, upLiq, downLiq = 0;
    double alpha, alphaWeight = 0;
    __m256d H1, H2 = 0;

    LocalLiquidity();
    LocalLiquidity(__m256d delta, __m256d deltaUp, __m256d deltaDown, __m256d dStar, double alpha);
    LocalLiquidity(__m256d delta, __m256d deltaUp, __m256d deltaDown, PriceFeedData::Price price, __m256d dStar, double alpha);

    bool computeH1H2exp(double __m256d);
    __m256d CumNorm(__m256d x);

    __m256d run(PriceFeedData::Price price);

    bool computation(PriceFeedData::Price price);
};

#endif