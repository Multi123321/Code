#include "helper/Macros.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "helper/Timer.h"

#include <omp.h>
#include <x86intrin.h>

#ifdef LIKWID_PERFMON
extern "C" {
#include <likwid.h>
}
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

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
    const int numberOfCurrencies = 16;
    string ccyList[numberOfCurrencies] = {
        "kurs1.csv",
        "kurs2.csv",
        "kurs3.csv",
        "kurs4.csv",
        "kurs5.csv",
        "kurs6.csv",
        "kurs7.csv",
        "kurs8.csv",
		"kurs9.csv",
        "kurs10.csv",
        "kurs11.csv",
        "kurs12.csv",
        "kurs13.csv",
        "kurs14.csv",
        "kurs15.csv",
        "kurs16.csv",
    };
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
	LIKWID_MARKER_INIT;
#pragma omp parallel for
    for (int i = 0; i < numberOfCurrencies; ++i)
    {
        Timer timer;
        timer.reset();
		LIKWID_MARKER_START( "code" );
        for (PriceFeedData::Price price : prices[i].priceFeed)
        {
            trading[i].runTradingAsymm(price);
        }
		LIKWID_MARKER_STOP( "code" );
        time = timer.elapsed();
        cout << ccyList[i] << " took: " << time << " Seconds" << std::endl;
    }
	LIKWID_MARKER_CLOSE;
	
    cout << "Total time: " << totalTimer.elapsed() << std::endl;
}
