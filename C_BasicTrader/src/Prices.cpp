#include "Prices.h"

Prices::Prices()
{
}

Prices::Prices(double bidArg, double askArg) : bid(bidArg), ask(askArg)
{
}

Prices::Prices(const Prices &p) : bid(p.bid), ask(p.ask)
{
}

double Prices::getAsk()
{
    return ask;
}

double Prices::getBid()
{
    return bid;
}

void Prices::setAsk(double askArg)
{
    this->ask = askArg;
}

void Prices::setBid(double bidArg)
{
    this->bid = bidArg;
}

double Prices::getSpread()
{
    return ask - bid;
}
