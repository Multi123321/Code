#ifndef COAST_LINE_TRADER_H
#define COAST_LINE_TRADER_H

#include <string>
#include "LocalLiquidity.h"
#include "Runner.h"
#include <deque>
#include <x86intrin.h>

using namespace std;

typedef __mmask8 mask;

class CoastlineTrader
{
private:
  deque<double> prices[4];
  deque<double> sizes[4];

  __m256d deltaUp, deltaDown, deltaOriginal;
  __m256d shrinkFlong, shrinkFshort;

  int longShort;

  bool initalized = false;
  Runner runner;
  //TODO
  Runner runnerG[2][2];

  __m256d increaseLong, increaseShort;

  string fxRate;

  LocalLiquidity liquidity;

public:
  __m256d tP; /* -- Total position -- */
  __m256d profitTarget;
  __m256d cashLimit;
  __m256d pnl, tempPnl;
  __m256d pnlPerc;
  double lastPrice;

  CoastlineTrader();
  CoastlineTrader(__m256d dOriginal, __m256d dUp, __m256d dDown, __m256d profitT, string FxRate, int lS);

  __m256d computePnl(PriceFeedData::Price price);
  __m256d computePnlLastPrice();
  __m256d getPercPnl(PriceFeedData::Price price);
  mask tryToClose(PriceFeedData::Price price);
  void assignCashTarget(mask mask);
  bool runPriceAsymm(PriceFeedData::Price price, __m256d oppositeInv);
};

#endif
