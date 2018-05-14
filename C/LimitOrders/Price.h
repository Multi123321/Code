#ifndef PRICE_H
#define PRICE_H

class Price
{
private:
    float bid;
    float ask;
    long time;

public:
    Price();
    Price(float bid, float ask, long time);
    
    Price clone();

    float getAsk();
    float getBid();
    long getTime();
    float getMid();
    void setAsk(int ask);
    void setBid(int bid);
    void setTime(long time);
    float getSpread();

};

#endif