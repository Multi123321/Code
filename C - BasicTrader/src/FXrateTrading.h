#ifndef FX_RATE_TRADING_H
#define FX_RATE_TRADING_H

#include "CoastlineTrader.h"
#include <string>
#include <vector>
#include "Liquidity.h"

class FXrateTrading
{
private:
    vector<CoastlineTrader*> coastTraderLong;
    vector<CoastlineTrader*> coastTraderShort;
    
    string FXrate;
    Liquidity liquidity;
    double currentTime, oneDay;
public:
    
    FXrateTrading();
    FXrateTrading(string rate, int nbOfCoastTraders, double deltas[]);

    bool runTradingAsymm(PriceFeedData price);
    bool printDataAsymm(double time);
};

#endif
