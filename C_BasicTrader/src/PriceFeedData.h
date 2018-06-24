#ifndef PRICE_FEED_H
#define PRICE_FEED_H

#include <chrono>
#include <list>

class PriceFeedData
{
  public:
    class Price
    {
      public:
        double mid;
        double ask;
        double bid;
        long time;

        Price(double ask, double bid, long time);

        double getAsk();
        double getBid();
        double getMid();
        long getTime();
    };

    std::list<Price> priceFeed;

    void addPrice(double ask, double bid, long time);

    PriceFeedData();
};

#endif