#ifndef LOCAL_LIQUIDITY_H
#define LOCAL_LIQUIDITY_H

class Price;

class LocalLiquidity
{
private:
    int run(Price price);

public:
    double deltaUp, deltaDown;
    double delta;
    double extreme, deltaStar, reference;
    int type;
    bool initalized;

    double surp;
    double liq;
    double alpha, alphaWeight;
    double H1, H2;

    LocalLiquidity(double d, double deltaStar, double alpha);

    bool computeH1H2exp();
    double computation(Price price);
};

#endif