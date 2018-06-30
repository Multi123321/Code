#ifndef FX_RATE_TRADING_H
#define FX_RATE_TRADING_H

#include "CoastlineTrader.h"
#include "AVXHelper.h"
#include <string>
#include <vector>

class FXrateTrading
{
  private:
    CoastlineTrader coastTraderLong;
    CoastlineTrader coastTraderShort;

    bool initialized = false;

    string FXrate;
    double currentTime, oneDay;

    bool printDataHeader();

  public:
    FXrateTrading();
    FXrateTrading(string rate, int nbOfCoastTraders, __m256d &deltas);

    bool runTradingAsymm(PriceFeedData::Price price);
    bool printDataAsymm(double time);
};

#endif
