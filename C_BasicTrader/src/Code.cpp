#include "helper/Macros.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "helper/Timer.h"

#include "PriceFeedData.h"
#include "FXrateTrading.h"
#include "CSVReader.h"

using namespace std;

int main(int argc, const char *argv[]) 
{
    functions::init(argc, argv);
    config::loadConfig();

    // TODO Currency configuration
    const int numberOfCurrencies = 1;
    string ccyList[numberOfCurrencies] = {"EUR_USD"};/* , "AUD_JPY", "AUD_NZD", "AUD_USD", "CAD_JPY", "CHF_JPY", "EUR_AUD", "EUR_CAD", "EUR_CHF",
            "EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_USD", "GBP_AUD", "GBP_CAD", "GBP_CHF", "GBP_JPY", "GBP_USD", "NZD_CAD",
            "NZD_JPY", "NZD_USD", "USD_CAD", "USD_CHF", "USD_JPY"}; */
    
    FXrateTrading trading[numberOfCurrencies];
    
    // TODO Threshold configuration  (see below)
    const int numberOfThresholds = 4;
    double deltaS[numberOfThresholds] = {0.25/100.0, 0.5/100.0, 1.0/100.0, 1.5/100.0};
    for( int i = 0; i < numberOfCurrencies; ++i ){
        trading[i] = FXrateTrading(ccyList[i], numberOfThresholds, deltaS);
    }
    
    double time;

    // Run
    for( int i = 0; i < numberOfCurrencies; ++i )
    {
        PriceFeedData prices;
        CSVReader::readExchangeFromFile(prices, "EUR_USD.csv");
        Timer timer;
        timer.reset();
        for (PriceFeedData::Price price : prices.priceFeed)
        {
            trading[i].runTradingAsymm(price);
        }
        time = timer.elapsed();
        cout << time << " Seconds elapsed\n";
    }
}

