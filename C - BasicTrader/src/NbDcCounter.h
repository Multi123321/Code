#ifndef NB_DC_COUNTER
#define NB_DC_COUNTER

#include <list>
#include "PriceFeedData.h"
#include "Runner.h"

class NbDcCounter
{
private:
	std::list<long> eventList;
	double delta;
	long timeWindow;
	Runner runner;
public:
    
    NbDcCounter();
    NbDcCounter(double delta, long timeWindow);
    
    bool run(PriceFeedData price);
};

#endif