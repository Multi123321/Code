#include "NbDcCounter.h"
#include <stdlib.h>
#include "Runner.h"

NbDcCounter::NbDcCounter()
{}

NbDcCounter::NbDcCounter(double delta, long timeWindow) :
    delta(delta), timeWindow(timeWindow)
{
    runner = Runner(delta, delta, "events", delta, delta);
}

bool NbDcCounter::run(PriceFeedData price)
{
    if( abs(runner.run(price)) == 1 ){
        eventList.push_front(price.elems.time);
    }
    
    if( eventList.size() == 0 )
        return true;

    eventList.sort();
    
    while( eventList.front() < price.elems.time - timeWindow )
        eventList.pop_front();
    
    return true;
}