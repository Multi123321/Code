#include "PriceFeedData.h"

PriceFeedData::PriceFeedData()
{}

    
void PriceFeedData::addPrice(double ask, double bid, long time)
{
    priceFeed.push_back(new Price(ask, bid, time));
}


PriceFeedData::Price::Price(double ask, double bid, long time) :
    ask(ask), bid(bid), time(time)
{
    mid = (ask + bid) / 2;
}
            
double PriceFeedData::Price::getAsk()
{
    return ask;
}

double PriceFeedData::Price::getBid()
{
    return bid;
}

double PriceFeedData::Price::getMid()
{
    return mid;
}

long PriceFeedData::Price::getTime()
{
    return time;
}

