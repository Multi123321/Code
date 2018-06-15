#ifndef RUNNER_H
#define RUNNER_H

#include "PriceFeedData.h"
#include <string>

class Runner
{
private:

public:
    double prevExtreme;
    long prevExtremeTime;

    double prevDC;
    long prevDCTime;

    double extreme;
    long extremeTime;

    double deltaUp;
    double deltaDown;
    double deltaStarUp, deltaStarDown;
    double osL;
    int type;
    bool initalized;
    double reference;

    std::string fileName;

    Runner();
    Runner(double threshUp, double threshDown, PriceFeedData::Price price, std::string file, double dStarUp, double dStarDown);
    Runner(double threshUp, double threshDown, double price, std::string file, double dStarUp, double dStarDown);
    Runner(double threshUp, double threshDown, std::string file, double dStarUp, double dStarDown);

    int run(PriceFeedData::Price price);
    int run(double price);


};

#endif
