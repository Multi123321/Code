#include "FXrateTrading.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "helper/Macros.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <x86intrin.h>

typedef unsigned int uint;

FXrateTrading::FXrateTrading()
{
    initialized = false;
}

FXrateTrading::FXrateTrading(string rate, int nbOfCoastTraders, __m256d &deltas)
{
    currentTime = 0;
    oneDay = 24.0 * 60.0 * 60.0 * 1000.0;
    FXrate = string(rate);

    IFPRINT(printDataHeader());

    if (nbOfCoastTraders != 4)
    {
        cout << "Must be 4 traders because of AVX";
        exit(EXIT_FAILURE);
    }

    coastTraderLong = CoastlineTrader(deltas, deltas, deltas, deltas, rate, 1);
    coastTraderShort = CoastlineTrader(deltas, deltas, deltas, deltas, rate, -1);

    initialized = true;
}

bool FXrateTrading::runTradingAsymm(PriceFeedData::Price price)
{

    coastTraderLong.runPriceAsymm(price, coastTraderShort.tP);
    coastTraderShort.runPriceAsymm(price, coastTraderLong.tP);

    if (price.time >= currentTime + oneDay)
    {
        while (currentTime <= price.time)
            currentTime += oneDay;

        IFPRINT(printDataAsymm(currentTime));
    }
    return true;
}

bool FXrateTrading::printDataHeader()
{
    ofstream outputFile;
    functions::openOutputFile(outputFile, config::configValues["outputDir"], "DataAsymmLiq.csv");

    outputFile << "time, totalPnl, totalPnlPerc, totalPos, totalLong, totalShort, price" << endl;
    outputFile.close();

    return true;
}

bool FXrateTrading::printDataAsymm(double time)
{
    ofstream outputFile;
    functions::openOutputFile(outputFile, config::configValues["outputDir"], "DataAsymmLiq.csv", ofstream::app);

    double totalPos = 0.0, totalShort = 0.0, totalLong = 0.0;
    double totalPnl = 0.0;
    double totalPnlPerc = 0.0;
    double price = -1.0;

    price = coastTraderLong.lastPrice;
    totalLong = AVXHelper::verticalSum(coastTraderLong.tP);
    totalShort = AVXHelper::verticalSum(coastTraderShort.tP);
    totalPos = totalLong + totalShort;
    totalPnl = AVXHelper::verticalSum(coastTraderLong.pnl) + AVXHelper::verticalSum(coastTraderLong.tempPnl) + AVXHelper::verticalSum(coastTraderLong.computePnlLastPrice()) + AVXHelper::verticalSum(coastTraderShort.pnl) + AVXHelper::verticalSum(coastTraderShort.tempPnl) + AVXHelper::verticalSum(coastTraderShort.computePnlLastPrice());
    totalPnlPerc = AVXHelper::verticalSum(coastTraderLong.pnlPerc) + AVXHelper::verticalSum(coastTraderShort.pnlPerc);
    totalPnlPerc += AVXHelper::verticalSum(_mm256_div_pd(_mm256_add_pd(coastTraderLong.tempPnl, coastTraderLong.computePnlLastPrice()), _mm256_mul_pd(coastTraderLong.cashLimit, coastTraderLong.profitTarget)));
    totalPnlPerc += AVXHelper::verticalSum(_mm256_div_pd(_mm256_add_pd(coastTraderShort.tempPnl, coastTraderShort.computePnlLastPrice()), _mm256_mul_pd(coastTraderShort.cashLimit, coastTraderShort.profitTarget)));

    outputFile << ((((long)time / 3600000) / 24) + 25569) << "," << totalPnl << "," << totalPnlPerc << "," << totalPos << "," << totalLong << "," << totalShort << "," << price << endl;

    outputFile.close();

    return true;
}