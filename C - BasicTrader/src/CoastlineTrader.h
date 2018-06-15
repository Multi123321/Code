#ifndef COAST_LINE_TRADER_H
#define COAST_LINE_TRADER_H

#include <string>
#include "LocalLiquidity.h"
#include "Runner.h"
#include <deque>

using namespace std;

class CoastlineTrader
{
private:
    deque<double> prices;
    deque<double> sizes;
    
    double deltaUp, deltaDown, deltaLiq, deltaOriginal;
    double shrinkFlong, shrinkFshort;
    
    
    int longShort;
    
    bool initalized;
    Runner* runner;
    //TODO
    Runner* runnerG[2][2];
    
    double increaseLong, increaseShort;
    
    
    string fxRate;
    
    LocalLiquidity* liquidity;
public:    
    double tP; /* -- Total position -- */
    double profitTarget;
    double cashLimit;
    double pnl, tempPnl;
    double pnlPerc;
    double lastPrice;

    CoastlineTrader();
    CoastlineTrader(double dOriginal, double dUp, double dDown, double profitT, string FxRate, int lS);

    double computePnl(PriceFeedData price);
    double computePnlLastPrice();
    double getPercPnl(PriceFeedData price);
    bool tryToClose(PriceFeedData price);
    bool assignCashTarget();
    bool runPriceAsymm(PriceFeedData price, double oppositeInv);
};

#endif
