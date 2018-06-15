#include <cstddef>
#include "LocalLiquidity.h"
#include "Tools.h"
#include "Price.h"
#include <cmath>

LocalLiquidity::LocalLiquidity(double d, double deltaStar, double alpha):
deltaUp(d), deltaDown(d), deltaStar(deltaStar), delta(d), alpha(alpha)
{
    type = -1;
    initalized = false;
    alphaWeight = exp(-2.0/(alpha + 1.0));

    computeH1H2exp();
}

bool LocalLiquidity::computeH1H2exp()
{
    H1 = -exp(-deltaStar/delta)*log(exp(-deltaStar/delta)) - (1.0 - exp(-deltaStar/delta))*log(1.0 - exp(-deltaStar/delta));
    H2 = exp(-deltaStar/delta)*pow(log(exp(-deltaStar/delta)), 2.0) - (1.0 - exp(-deltaStar/delta))*pow(log(1.0 - exp(-deltaStar/delta)), 2.0) - H1*H1;
    return true;    
}

/**
 * This method should be called with every new price
 * @param price new price
 * @return boolean
 */
double LocalLiquidity::computation(Price price)
{
    int event = run(price);
    if( event != 0 ){
        surp = alphaWeight * (std::abs(event) == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight) * surp;
        liq = 1.0 - Tools::CumNorm(sqrt(alpha) * (surp - H1) / sqrt(H2));
    }
    return liq;
}

/**
 * This is the local runner of the class. It can be delegated to an external class
 * @param price is just a new price
 * @return 1 and -1 if DC IE, 2 or -2 if OS IE, 0 otherwise.
 */
int LocalLiquidity::run(Price price)
{
    if(&price == NULL)
    {
        return 0;
    }

    if(!initalized)
    {
        type = -1; initalized = true;
        extreme = reference = price.getMid();
        return 0;
    }

    if(type == -1)
    {
        if(log(price.getBid()/extreme) >= deltaUp)
        {
            type = 1;
            extreme = price.getBid();
            reference = price.getBid();
            return 1;
        }
        if(price.getAsk() < extreme)
        {
            extreme = price.getAsk();
        }
        if(log(reference/extreme) >= deltaStar)
        {
            reference = extreme;
            return -2;
        }
    }
    else if(type == 1)
    {
        if(log(price.getAsk()/extreme) <= -deltaDown)
        {
            type = -1;
            extreme = price.getAsk();
            reference = price.getAsk();
            return -1;
        }
        if(price.getBid() > extreme )
        {
            extreme = price.getBid();
        }
        if(log(reference/extreme) <= -deltaStar)
        {
            reference = extreme;
            return 2;
        }
    }
    return 0;
}