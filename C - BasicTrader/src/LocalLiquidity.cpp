#include "LocalLiquidity.h"
#include "PriceFeedData.h"
#include <math.h>
#include <stdlib.h>

using namespace std;

LocalLiquidity::LocalLiquidity()
{}

LocalLiquidity::LocalLiquidity(double delta, double deltaUp, double deltaDown, double dStar, double alpha) : 
    delta(delta), deltaUp(deltaUp), deltaDown(deltaDown), dStar(dStar), alpha(alpha)
{
    type = -1; 
    initalized = false;
    alphaWeight = exp(-2.0/(alpha + 1.0));
    computeH1H2exp(dStar);
} 

LocalLiquidity::LocalLiquidity(double delta, double deltaUp, double deltaDown, PriceFeedData price, double dStar, double alpha) : 
    delta(delta), deltaUp(deltaUp), deltaDown(deltaDown), dStar(dStar), alpha(alpha)
{
    type = -1;
    extreme = reference = price.elems.mid;
    initalized = true;
    alphaWeight = exp(-2.0/(alpha + 1.0));
    computeH1H2exp(dStar);
} 

bool LocalLiquidity::computeH1H2exp(double dS)
{
    H1 = -1 * exp(-dStar/delta)*log(exp(-dStar/delta)) - (1.0 - exp(-dStar/delta))*log(1.0 - exp(-dStar/delta));
    H2 = exp(-dStar/delta)*pow(log(exp(-dStar/delta)), 2.0) - (1.0 - exp(-dStar/delta))*pow(log(1.0 - exp(-dStar/delta)), 2.0) - H1*H1;
    return true; 
}
 
// Another implementation of the CNDF for a standard normal: N(0,1)
double LocalLiquidity::CumNorm(double x)
{
    // protect against overflow
    if (x > 6.0)
        return 1.0;
    if (x < -6.0)
        return 0.0;
            
    double b1 = 0.31938153;
    double b2 = -0.356563782;
    double b3 = 1.781477937;
    double b4 = -1.821255978;
    double b5 = 1.330274429;
    double p = 0.2316419;
    double c2 = 0.3989423;
            
    double a = abs(x);
    double t = 1.0 / (1.0 + a * p);
    double b = c2*exp((-x)*(x/2.0));
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
    n = 1.0-b*n;
            
    if ( x < 0.0 )
        n = 1.0 - n;

    return n;
}

int LocalLiquidity::run(PriceFeedData price){
    if( &price == NULL )
        return 0;
    
    if( !initalized ){
        type = -1; initalized = true;
        extreme = reference = price.elems.mid;
        return 0;
    }
    
    if( type == -1 ){
        if( log(price.elems.bid/extreme) >= deltaUp ){
            type = 1;
            extreme = price.elems.ask;
            reference = price.elems.ask;
            return 1;
        }
        if( price.elems.ask < extreme ){
            extreme = price.elems.ask;
        }
        if( log(reference/extreme) >= dStar ){
            reference = extreme;
            return 2;
        }
    }
    else if( type == 1 ){
        if( log(price.elems.ask/extreme) <= -deltaDown ){
            type = -1;
            extreme = price.elems.bid; 
            reference = price.elems.bid;
            return -1;
        }
        if( price.elems.bid > extreme ){
            extreme = price.elems.bid; 
        }
        if( log(reference/extreme) <= -dStar ){
            reference = extreme;
            return -2;
        }
    }
    return 0;
}

bool LocalLiquidity::computation(PriceFeedData price){
    if( &price == NULL )
        return false;
    
    int event = run(price);
    if( event != 0 ){
        surp = alphaWeight*(abs(event) == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*surp;
        
        if( event > 0 ){ // down moves
            downSurp = alphaWeight*(event == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*downSurp;
        }else if( event < 0 ){ // up moves
            upSurp = alphaWeight*(event == -1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*upSurp;
        }
        
        liq = 1.0 - CumNorm(sqrt(alpha)*(surp - H1)/sqrt(H2)); 
        upLiq = 1.0 - CumNorm(sqrt(alpha)*(upSurp - H1)/sqrt(H2)); 
        downLiq = 1.0 - CumNorm(sqrt(alpha)*(downSurp - H1)/sqrt(H2)); 
    }
    return true;
}