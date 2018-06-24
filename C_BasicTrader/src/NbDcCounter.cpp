#include "NbDcCounter.h"
#include <stdlib.h>
#include "Runner.h"

NbDcCounter::NbDcCounter()
{
}

NbDcCounter::NbDcCounter(double deltaArg, long timeWindowArg) : delta(deltaArg), timeWindow(timeWindowArg)
{
    //runner = Runner(delta, delta, "events", delta, delta);
}

bool NbDcCounter::run(PriceFeedData::Price price)
{
    // if (abs(runner.run(price)) == 1)
    // {
    //     eventList.push_front(price.time);
    // }

    // if (eventList.size() == 0)
    //     return true;

    // eventList.sort();

    // while (eventList.front() < price.time - timeWindow)
    //     eventList.pop_front();

    // return true;
}