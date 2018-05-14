#ifndef ALPHA_ENGINE_H
#define ALPHA_ENGINE_H

class Price;
class CoastLineTrader;

class AlphaEngine
{
private:
    CoastLineTrader* longCoastlineTraders[];
    CoastLineTrader* shortCoastlineTraders[];


public:
    AlphaEngine();

    void run(Price price);

    void initiateTraders();
};

#endif