#include "Liquidity.h"

#include "PriceFeedData.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <numeric> 
#include <chrono>
#include <random>
#include <list>

Liquidity::Liquidity()
{}

// Another implementation of the CNDF for a standard normal: N(0,1)
double Liquidity::CumNorm(double x)
{
    // Protect against overflow
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
    
    double a = std::abs(x);
    double t = 1.0 / (1.0 + a * p);
    double b = c2*exp((-x)*(x/2.0));
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
    n = 1.0-b*n;
    
    if ( x < 0.0 )
        n = 1.0 - n;

    return n;
}

Liquidity::Liquidity(PriceFeedData price, double delta1, double delta2, int lgt)
{
    double prob = exp(-1.0);
    H1 = -(prob*log(prob) + (1.0 - prob)*log(1.0 - prob));
    H2 = prob*pow(log(prob), 2.0) + (1.0 - prob)*pow(log(1.0 - prob), 2.0) - H1*H1;
    runner.resize(lgt);
    prevState.resize(lgt);
    d1 = delta1; d2 = delta2;

    getH1nH2(); //skip computation and assign!
    
    runner.resize(lgt);
    prevState.resize(lgt);
    
    for( int i = 0; i < runner.size(); ++i )
    {
        // TODO maybe a new is necessary
        runner[i] = Runner(0.025/100.0 + 0.05/100.0*(double)i, 0.025/100.0 + 0.05/100.0*(double)i, price, "JustFake");
        runner[i].type = (i%2 == 0 ? 1 : -1);
        prevState[i] = (runner[i].type == 1 ? 1 : 0);
    }
    surp = H1; dSurp = H1; uSurp = H1;
    liquidity = 0.5; 
    liqEMA = 0.5;
    
    for( int i = 0; i < 100; ++i ){
        mySurprise.push_front(H1);
        downSurprise.push_front(H1);
        upSurprise.push_front(H1);
    }
    
    //computeLiquidity();
    
    downLiq = 0.5; 
    upLiq = 0.5; 
    diffLiq = 0.5; 
    diffRaw = 0.0;
    alpha = 2.0/(100.0 + 1.0); 
    alphaWeight = exp(-alpha); 
}

void Liquidity::getH1nH2()
{
    double price = 0.0; 
    alpha = 2.0/(100.0 + 1.0);
    alphaWeight = exp(-alpha);
    runner = vector<Runner>(runner.size());
    for( int i = 0; i < runner.size(); ++i )
    {
        // TODO maybe a new is necessary here
        runner[i] = Runner(0.025/100.0 + 0.05/100.0 * (double)i, 0.025/100.0 + 0.05/100.0*(double)i, price, "JustFake");
        runner[i].type = (i%2 == 0 ? 1 : -1);
        prevState[i] = (runner[i].type == 1 ? 1 : 0);
    }
    
    double total1 = 0.0, total2 = 0.0;
    //TODO: set random seed here (e.g. 1)
    default_random_engine generator (1);
    normal_distribution<double> distribution (0.0, 1.0);

    double dt = 1.0/sqrt(1000000.0);
    double sigma = 0.25; // 25%
    for( int i = 0; i < 100000000; ++i )
    {
        price += sigma * dt * distribution(generator);
        for( int j= 0; j < runner.size(); ++j )
        {
            if( abs(runner[j].run(price)) == 1 ) // this is OK for simulated prices
            { 
                double myProbs = getProbs(j);
                total1 = total1*alphaWeight + (1.0 - alphaWeight)*(-log(myProbs));
                total2 = total2*alphaWeight + (1.0 - alphaWeight)*pow(log(myProbs), 2.0);
            }
        }
    }
    H1 = total1;
    H2 = total2 - H1*H1;
    cout << "H1:" << H1 << " H2:" << H2;
}

bool Liquidity::Trigger(PriceFeedData price)
{
    // -- update values -- 
    bool doComp = false;
    for( int i = 0; i < runner.size(); ++i )
    {
        int value = runner[i].run(price);
        if( abs(value) == 1 )
        {
            //double alpha = 2.0/(100.0 + 1.0);
            double myProbs = getProbs(i);
            surp = surp*alphaWeight + (1.0 - alphaWeight)*(-log(myProbs));
            mySurprise.remove(0); mySurprise.push_front(-log(myProbs));
            if( runner[i].type == -1 )
            {
                dSurp = dSurp*alphaWeight + (1.0 - alphaWeight)*(-log(myProbs));
                downSurprise.remove(0); downSurprise.push_front(-log(myProbs));
            }
            else if( runner[i].type == 1 )
            {
                uSurp = uSurp*alphaWeight + (1.0 - alphaWeight)*(-log(myProbs));
                upSurprise.remove(0); upSurprise.push_front(-log(myProbs));
            }
            doComp = true;
        }
    }
    if( doComp ){
        liqEMA = (1.0 - CumNorm(sqrt(100.0)*(surp - H1)/sqrt(H2)));
        upLiq = (1.0 - CumNorm(sqrt(100.0)*(uSurp - H1)/sqrt(H2)));
        downLiq =  (1.0 - CumNorm(sqrt(100.0)*(dSurp - H1)/sqrt(H2)));
        diffLiq = CumNorm(sqrt(100.0)*(uSurp - dSurp)/sqrt(H2));
        diffRaw = sqrt(100.0)*(uSurp-dSurp)/sqrt(H2);
    }
    return doComp;
}


double Liquidity::getProbs(int i)
{
    	int where = -1;
		for( int j = 1; j < prevState.size(); ++j )
        {
			if( prevState[j] != prevState[0] )
            {
				where = j;
				break;
			}
		}
		if( i > 0 && where != i )
        {
			cout << "This should not happen! " << where;
		}
		prevState[i] = (prevState[i] == 1 ? 0 : 1);
		
		if( where == 1 )
        {
			if( i > 0 )
            {
				return exp(-(runner[1].deltaDown - runner[0].deltaDown)/runner[0].deltaDown);
			}
            else
            {
				return (1.0 - exp(-(runner[1].deltaDown - runner[0].deltaDown)/runner[0].deltaDown));
			}
		}
        else if( where > 1 )
        {
			double numerator = 0.0;
			for( int k = 1; k <= where; ++k )
            {
				numerator -= (runner[k].deltaDown - runner[k-1].deltaDown)/runner[k-1].deltaDown;
			}
			numerator = exp(numerator);
			double denominator = 0.0;
			for( int k = 1; k <= where - 1; ++k )
            {
				double secVal = 0.0;
				for( int j  = k+1; j <= where; ++j )
                {
					secVal -=  (runner[j].deltaDown - runner[j-1].deltaDown)/runner[j-1].deltaDown;
				}
				denominator += (1.0 - exp(-(runner[k].deltaDown - runner[k-1].deltaDown)/runner[k-1].deltaDown))*exp(secVal);
			}
			if( i > 0 )
            {
				return numerator/(1.0 - denominator);
			}
            else
            {
				return (1.0 - numerator/(1.0 - denominator));
			}
		}
		else
        {
			return 1.0;
		}
}


bool Liquidity::computeLiquidity(long deltaT)
{
    double surpT = 0.0;
    double downSurp = 0.0, upSurp = 0.0;
    
    surpT = accumulate(begin(mySurprise), end(mySurprise), 0.0);
    downSurp = accumulate(begin(downSurprise), end(downSurprise), 0.0);
    upSurp = accumulate(begin(upSurprise), end(upSurprise), 0.0);
    
    liquidity = 1.0 - CumNorm((surpT - H1*mySurprise.size())/sqrt(H2*mySurprise.size()));
    liquidityDown = 1.0 - CumNorm((downSurp - H1*downSurprise.size())/sqrt(H2*downSurprise.size()));
    liquidityUp = 1.0 - CumNorm((upSurp - H1*upSurprise.size())/sqrt(H2*upSurprise.size()));
    
    return true;
}

Liquidity::Runner::Runner()
{}

Liquidity::Runner::Runner(double threshUp, double threshDown, PriceFeedData price, string file) :
    fileName(file), deltaUp(threshUp), deltaDown(threshDown), prevDC(price.elems.mid), extreme(price.elems.mid)
{    
    type = -1; 
    initalized = true;
}

Liquidity::Runner::Runner(double threshUp, double threshDown, double price, string file) :
    prevDC(price), extreme(price), deltaUp(threshUp), deltaDown(threshDown), fileName(file)
{
    type = -1; 
    initalized = true;
}

Liquidity::Runner::Runner(double threshUp, double threshDown, string file) :
    deltaUp(threshUp), deltaDown(threshDown), fileName(file)
{
    initalized = false;
}

int Liquidity::Runner::run(PriceFeedData price)
{
    // if( &price == NULL )
    //     return 0;
    
    if( !initalized ){
        type = -1; initalized = true;
        prevDC = price.elems.mid;
        extreme = price.elems.mid;
        return 0;
    }
    
    if( type == -1 )
    {
        if( log(price.elems.bid/extreme) >= deltaUp )
        {
            type = 1;
            extreme = price.elems.ask; 
            prevDC = price.elems.ask;	
            return 1;
        }
        if( price.elems.ask < extreme ){
            extreme = price.elems.ask;
            return 0;
        }
    }
    else if( type == 1 ){
        if( log(price.elems.ask/extreme) <= -deltaDown )
        {
            type = -1;
            extreme = price.elems.bid;
            prevDC = price.elems.bid; 
            return -1;
        }
        if( price.elems.bid > extreme )
        {
            extreme = price.elems.bid; 
            return 0;
        }
    }
    return 0;
}

int Liquidity::Runner::run(double price)
{
    if( !initalized )
    {
        type = -1; initalized = true;
        prevDC = price;
        extreme = price; 
        return 0;
    }
    
    if( type == -1 )
    {
        if( price - extreme >= deltaUp )
        {
            type = 1;
            extreme = price; 
            prevDC = price;
            return 1;
        }
        if( price < extreme )
        {
            extreme = price;
            return 0;
        }
    }
    else if( type == 1 )
    {
        if( price - extreme <= -deltaDown )
        {
            type = -1;
            extreme = price; 
            prevDC = price; ;
            return 1;
        }
        if( price > extreme )
        {
            extreme = price;
            return 0;
        }
    }
    return 0;
}