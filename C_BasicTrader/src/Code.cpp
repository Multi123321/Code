#include "helper/Macros.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "helper/Timer.h"

#include <omp.h>
#include <x86intrin.h>

#include "AVXHelper.h"
#include "PriceFeedData.h"
#include "FXrateTrading.h"
#include "CSVReader.h"

using namespace std;

int main(int argc, const char *argv[])
{
    functions::init(argc, argv);
    config::loadConfig();

    // TODO Currency configuration
    const int numberOfCurrencies = 4;
    string ccyList[numberOfCurrencies] = {"kurs1.csv", "kurs2.csv", "kurs3.csv", "kurs4.csv"};
    //const int numberOfCurrencies = 1;
    //string ccyList[numberOfCurrencies] = {"EUR_USD.csv"};

    FXrateTrading trading[numberOfCurrencies];

    // TODO Threshold configuration  (see below)
    const int numberOfThresholds = 4;
    __m256d deltaS = _mm256_set_pd(1.5 / 100.0, 1.0 / 100.0, 0.5 / 100.0, 0.25 / 100.0);

    PriceFeedData prices[numberOfCurrencies];

#pragma omp parallel for
    for (int i = 0; i < numberOfCurrencies; ++i)
    {
        trading[i] = FXrateTrading(ccyList[i], numberOfThresholds, deltaS);

        CSVReader::readExchangeFromFile(prices[i], ccyList[i]);
    }

    double time;

    Timer totalTimer;
    totalTimer.reset();
// Run
#pragma omp parallel for
    for (int i = 0; i < numberOfCurrencies; ++i)
    {
        Timer timer;
        timer.reset();
        for (PriceFeedData::Price price : prices[i].priceFeed)
        {
            trading[i].runTradingAsymm(price);
        }
        time = timer.elapsed();
        cout << ccyList[i] << " took: " << time << " Seconds" << std::endl;
    }

    cout << "Total time: " << totalTimer.elapsed() << std::endl;
}
