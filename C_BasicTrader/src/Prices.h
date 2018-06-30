#ifndef PRICES_H
#define PRICES_H

class Prices
{
  private:
    double bid;
    double ask;

  public:
    Prices();
    Prices(double bid, double ask);

    Prices(const Prices &p);

    double getAsk();
    double getBid();
    void setAsk(double ask);
    void setBid(double bid);
    double getSpread();
};

#endif