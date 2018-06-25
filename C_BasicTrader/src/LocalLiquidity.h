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
    
    double surp, upSurp, downSurp;
    double liq, upLiq, downLiq;
    double alpha, alphaWeight;
    double H1, H2;
    
    LocalLiquidity();
    LocalLiquidity(double delta, double deltaUp, double deltaDown, double dStar, double alpha);
    LocalLiquidity(double delta, double deltaUp, double deltaDown, PriceFeedData::Price price, double dStar, double alpha);

    bool computeH1H2exp(double dStar);
    double CumNorm(double x);

    int run(PriceFeedData::Price price);

    bool computation(PriceFeedData::Price price);
};

#endif