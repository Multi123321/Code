#include "Prices.h"

Prices::Prices()
{}

Prices::Prices(double bid, double ask) : 
    bid(bid), ask(ask)
{}

Prices::Prices(const Prices& p) :
    bid(p.bid), ask(p.ask)
{}

double Prices::getAsk()
{
    return ask;
}

double Prices::getBid()
{
    return bid;
}

void Prices::setAsk(double ask)
{
    this->ask = ask;
}

void Prices::setBid(double bid)
{
    this->bid = bid;
}

double Prices::getSpread()
{
    return ask - bid;
}
