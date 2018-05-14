#ifndef LIMIT_ORDER_H
#define LIMIT_ORDER_H

#include "Price.h"
#include <list>

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
    std::list<LimitOrder> compensatedOrders;

    LimitOrder(int type, Price priceOpened, double level, float volume, int dcORos, double delta);

    LimitOrder clone();

    void setLevel(double level);
    int getType();
    double getLevel();
    float getVolume();
    int getDcORos();
    double getDelta();
    void addCompensatedOrder(LimitOrder compensatedOrders);
    void cleanCompensatedList();
    void setCompensatedOrders(std::list<LimitOrder> compensatedOrders);
    float computeCompensatedVolume();
    double getRelativePnL();
};

#endif