#ifndef RUNNER_H
#define RUNNER_H

class Price;

class Runner
{
private:
    double deltaUp;
    double deltaDown;
    double dStarUp;
    double dStarDown;
    int mode;
    bool initialized;
    double extreme;
    double reference;
    double expectedDcLevel;
    double expectedOsLevel;

    void findExpectedDClevel();
    void findExpectedOSlevel();

public:
    Runner(double deltaUp, double deltaDown, double dStarUp, double dStarDown);
    int run(Price price);
    double getExpectedDcLevel();
    double getExpectedOsLevel();
    double getExpectedUpperIE();
    double getExpectedLowerIE();
    int getMode();
    double getDeltaUp();
    double getDeltaDown();
    double getdStarUp();
    double getdStarDown();
    int getUpperIEtype();
    int getLowerIEtype();
};

#endif