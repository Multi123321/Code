#include "Price.h"

Price::Price()
{}

Price::Price(float bid, float ask, long time):
    bid(bid), ask(ask), time(time)
{}
    
Price Price::clone()
{
    return Price(bid, ask, time);
}

float Price::getAsk()
{
    return ask;    
}

float Price::getBid() 
{
    return bid;
}

long Price::getTime() 
{
    return time;
}

float Price::getMid()
{
    return (bid + ask) / 2.0f;
}

void Price::setAsk(int ask)
{
    this->ask = ask;
}

void Price::setBid(int bid)
{
    this->bid = bid;
}

void Price::setTime(long time)
{
    this->time = time;
}

float Price::getSpread()
{
    return ask - bid;
}
