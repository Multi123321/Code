#include "helper/Macros.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"

#include "PriceFeedData.h"
#include "FXrateTrading.h"

using namespace std;
using namespace config;

string ingridPath, outgridPath;

int main(int argc, const char *argv[]) 
{
    // if(argc != 3)
    // {
    //     cout << "Aufrufen mit: " << argv[0] << " INGRID.vtk OUTGRID.vtk\n";
    //     exit(EXIT_FAILURE);
    // }

    functions::init(argc, argv);
    config::loadConfig();

    // TODO Currency configuration
    const int numberOfCurrencies = 1;
    string ccyList[numberOfCurrencies] = {"AUD_CAD"};/* , "AUD_JPY", "AUD_NZD", "AUD_USD", "CAD_JPY", "CHF_JPY", "EUR_AUD", "EUR_CAD", "EUR_CHF",
            "EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_USD", "GBP_AUD", "GBP_CAD", "GBP_CHF", "GBP_JPY", "GBP_USD", "NZD_CAD",
            "NZD_JPY", "NZD_USD", "USD_CAD", "USD_CHF", "USD_JPY"}; */
    
    FXrateTrading trading[numberOfCurrencies];
    
    // TODO Threshold configuration  (see below)
    const int numberOfThresholds = 4;
    double deltaS[numberOfThresholds] = {0.25/100.0, 0.5/100.0, 1.0/100.0, 1.5/100.0};
    for( int i = 0; i < numberOfCurrencies; ++i ){
        trading[i] = FXrateTrading(ccyList[i], numberOfThresholds, deltaS);
    }
    
    // Run
    PriceFeedData p = PriceFeedData();
    for( int i = 0; i < numberOfCurrencies; ++i )
    {
        for (PriceFeedData::Price* price : p.priceFeed)
        {
            trading[i].runTradingAsymm(*price);
        }
    }
}

