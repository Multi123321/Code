#ifndef LIMIT_ORDER_H
#define LIMIT_ORDER_H

#include "Price.h"

class LimitOrder
{
private:
    int type;
    Price priceOpened;
    double level;
    float volume;
    double delta;
    int dcORos;



public:
    int i;
    list<LimitOrder> compensatedOrders;

    static double CumNorm(double x);
};

#endif