#include "FXrateTrading.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace config;

FXrateTrading::FXrateTrading()
{}

FXrateTrading::FXrateTrading(string rate, int nbOfCoastTraders, double deltas[])
{
    currentTime = 1136073600000.0;
    oneDay = 24.0*60.0*60.0*1000.0;
    FXrate = string(rate);

    coastTraderLong.resize(nbOfCoastTraders);
    coastTraderShort.resize(nbOfCoastTraders);
    
    for( int i = 0; i < coastTraderLong.size(); ++i )
    {
        coastTraderLong[i] = new CoastlineTrader(deltas[i], deltas[i], deltas[i], deltas[i], rate, 1);
        coastTraderShort[i] =  new CoastlineTrader(deltas[i], deltas[i], deltas[i], deltas[i], rate, -1);
    }
}

bool FXrateTrading::runTradingAsymm(PriceFeedData price)
{
    for( int i = 0; i < coastTraderLong.size(); ++i )
    {
        coastTraderLong[i]->runPriceAsymm(price, coastTraderShort[i]->tP);
        coastTraderShort[i]->runPriceAsymm(price, coastTraderLong[i]->tP);
    }
    
    if( price.elems.time >= currentTime + oneDay )
    {
        while( currentTime <= price.elems.time )
            currentTime += oneDay;
        
        printDataAsymm(currentTime);
    }
    return true;
}

bool FXrateTrading::printDataAsymm(double time)
{
    ofstream outputFile = functions::openOutputFile(configValues["outputDir"],  FXrate + "DataAsymmLiq.dat", ofstream::app);
    
    double totalPos = 0.0, totalShort = 0.0, totalLong = 0.0; 
    double totalPnl = 0.0; 
    double totalPnlPerc = 0.0;  
    double price = -1.0;
    for( int i = 0; i < coastTraderLong.size(); ++i )
    {
        if( i == 0 )
        {
            price = coastTraderLong[i]->lastPrice;
        }
        totalLong += coastTraderLong[i]->tP;
        totalShort += coastTraderShort[i]->tP;
        totalPos += (coastTraderLong[i]->tP + coastTraderShort[i]->tP);
        totalPnl += (coastTraderLong[i]->pnl + coastTraderLong[i]->tempPnl + coastTraderLong[i]->computePnlLastPrice()
                + coastTraderShort[i]->pnl + coastTraderShort[i]->tempPnl + coastTraderShort[i]->computePnlLastPrice());
        totalPnlPerc += (coastTraderLong[i]->pnlPerc + (coastTraderLong[i]->tempPnl + coastTraderLong[i]->computePnlLastPrice())/coastTraderLong[i]->cashLimit*coastTraderLong[i]->profitTarget
                + coastTraderShort[i]->pnlPerc + (coastTraderShort[i]->tempPnl + coastTraderShort[i]->computePnlLastPrice())/coastTraderShort[i]->cashLimit*coastTraderShort[i]->profitTarget);
    }
    outputFile << (long)time << "," << totalPnl << "," << totalPnlPerc << "," << totalPos << "," << totalLong << "," << totalShort << "," << price << endl;
    
    outputFile.close();
    
    return true;
}