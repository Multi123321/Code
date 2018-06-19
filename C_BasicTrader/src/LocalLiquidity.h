#ifndef LOCAL_LIQUIDITY_H
#define LOCAL_LIQUIDITY_H

#include "PriceFeedData.h"

class LocalLiquidity
{
public:
    double deltaUp, deltaDown;
    double delta;
    double extreme, dStar, reference;
    int type;
    bool initalized = false;
    
    double surp, upSurp, downSurp = 0;
    double liq, upLiq, downLiq = 0;
    double alpha, alphaWeight = 0;
    double H1, H2 = 0;
    
    LocalLiquidity();
    LocalLiquidity(double delta, double deltaUp, double deltaDown, double dStar, double alpha);
    LocalLiquidity(double delta, double deltaUp, double deltaDown, PriceFeedData::Price price, double dStar, double alpha);

    bool computeH1H2exp(double dStar);
    double CumNorm(double x);

    int run(PriceFeedData::Price price);

    bool computation(PriceFeedData::Price price);
};

#endif