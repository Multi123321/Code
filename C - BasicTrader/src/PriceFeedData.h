#ifndef PRICE_FEED_H
#define PRICE_FEED_H

#include <chrono>


class PriceFeedData
{
public:
    class Elems
    {
        public:
            double mid = 1.1;
            double ask = 1.1;
            double bid = 1.0;
            long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch()).count();
    };

    double ask;
    Elems elems;
    
    PriceFeedData();    
};



#endif