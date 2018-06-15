#ifndef LIQUIDITY_H
#define LIQUIDITY_H

#include "PriceFeedData.h"
#include <string>
#include <list>
#include <vector>

using namespace std;

class Liquidity
{
private:
    vector<double> prevState = vector<double>(0);
	double surp = 0.0, dSurp = 0.0, uSurp = 0.0;
	double liquidity, liquidityUp, liquidityDown; 
	double liqEMA;
	double upLiq, downLiq, diffLiq, diffRaw;
	double H1 = 0.0, H2 = 0.0;
	double d1 = 0.0, d2 = 0.0;
	double alpha, alphaWeight;
	list<double> mySurprise, downSurprise, upSurprise;

    double CumNorm(double x);



public:
    class Runner
    {
    public:
        double prevDC;
        double extreme;

        double deltaUp;
        double deltaDown;
        int type;
        bool initalized;

        string fileName;

        Runner();
        Runner(double threshUp, double threshDown, PriceFeedData::Price price, string file);
        Runner(double threshUp, double threshDown, double price, string file);
        Runner(double threshUp, double threshDown, string file);

        int run(PriceFeedData::Price price);
        int run(double price);
    };

    vector<Runner> runner = vector<Runner>(0);

    Liquidity();
    Liquidity(PriceFeedData::Price price, double delta1, double delta2, int lgt);

    void getH1nH2();
    bool Trigger(PriceFeedData::Price price);
    double getProbs(int i);
    bool computeLiquidity(long deltaT);
};

#endif
