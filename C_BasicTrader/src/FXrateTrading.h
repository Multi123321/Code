#ifndef FX_RATE_TRADING_H
#define FX_RATE_TRADING_H

#include "CoastlineTrader.h"
#include <string>
#include <vector>

class FXrateTrading
{
private:
    vector<CoastlineTrader*> coastTraderLong;
    vector<CoastlineTrader*> coastTraderShort;
    
    string FXrate;
    double currentTime, oneDay;

    bool printDataHeader();
public:
    
    FXrateTrading();
    FXrateTrading(string rate, int nbOfCoastTraders, double deltas[]);

    bool runTradingAsymm(PriceFeedData::Price price);
    bool printDataAsymm(double time);
};

#endif
