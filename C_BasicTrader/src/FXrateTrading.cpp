#include "FXrateTrading.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "helper/Macros.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>

typedef unsigned int uint;

FXrateTrading::FXrateTrading()
{
    initialized = false;
}

FXrateTrading::FXrateTrading(string rate, int nbOfCoastTraders, double deltas[])
{
    currentTime = 0;
    oneDay = 24.0 * 60.0 * 60.0 * 1000.0;
    FXrate = string(rate);

    printDataHeader();

    coastTraderLong.resize(nbOfCoastTraders);
    coastTraderShort.resize(nbOfCoastTraders);

    for (uint i = 0; i < coastTraderLong.size(); ++i)
    {
        coastTraderLong[i] = CoastlineTrader(deltas[i], deltas[i], deltas[i], deltas[i], rate, 1);
        coastTraderShort[i] = CoastlineTrader(deltas[i], deltas[i], deltas[i], deltas[i], rate, -1);
    }
    initialized = true;
}

bool FXrateTrading::runTradingAsymm(PriceFeedData::Price price)
{
    for (uint i = 0; i < coastTraderLong.size(); ++i)
    {
        coastTraderLong[i].runPriceAsymm(price, coastTraderShort[i].tP);
    }
    for (uint i = 0; i < coastTraderShort.size(); ++i)
    {
        coastTraderShort[i].runPriceAsymm(price, coastTraderLong[i].tP);
    }

    if (price.time >= currentTime + oneDay)
    {
        while (currentTime <= price.time)
            currentTime += oneDay;

        IFDEBUG(printDataAsymm(currentTime));
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
    for (uint i = 0; i < coastTraderLong.size(); ++i)
    {
        if (i == 0)
        {
            price = coastTraderLong[i].lastPrice;
        }
        totalLong += coastTraderLong[i].tP;
        totalShort += coastTraderShort[i].tP;
        totalPos += (coastTraderLong[i].tP + coastTraderShort[i].tP);
        totalPnl += (coastTraderLong[i].pnl + coastTraderLong[i].tempPnl + coastTraderLong[i].computePnlLastPrice() + coastTraderShort[i].pnl + coastTraderShort[i].tempPnl + coastTraderShort[i].computePnlLastPrice());
        totalPnlPerc += (coastTraderLong[i].pnlPerc + (coastTraderLong[i].tempPnl + coastTraderLong[i].computePnlLastPrice()) / coastTraderLong[i].cashLimit * coastTraderLong[i].profitTarget + coastTraderShort[i].pnlPerc + (coastTraderShort[i].tempPnl + coastTraderShort[i].computePnlLastPrice()) / coastTraderShort[i].cashLimit * coastTraderShort[i].profitTarget);
    }
    outputFile << ((((long)time / 3600000) / 24) + 25569) << "," << totalPnl << "," << totalPnlPerc << "," << totalPos << "," << totalLong << "," << totalShort << "," << price << endl;

    outputFile.close();

    return true;
}