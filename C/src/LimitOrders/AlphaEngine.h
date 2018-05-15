#ifndef ALPHA_ENGINE_H
#define ALPHA_ENGINE_H

class Price;
class CoastLineTrader;

class AlphaEngine
{
private:
    CoastLineTrader* longCoastlineTraders[];
    CoastLineTrader* shortCoastlineTraders[];

    void initiateTraders();

public:
    AlphaEngine();

    void run(Price price);
};

#endif