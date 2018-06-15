#ifndef COASTLINE_TRADER_H
#define COASTLINE_TRADER_H

#include <list>
#include "Runner.h"
#include "LimitOrder.h"
#include "LocalLiquidity.h"


class CoastlineTrader
{
private:
    Runner runners[];
    LimitOrder buyLimitOrder;
    LimitOrder sellLimitOrder;
    float originalUnitSize; // size of the initial traded volume
    float uniteSizeFromInventory; // inventory dictates the current unit size, this one
    bool initialized;
    LocalLiquidity localLiquidityIndicator;
    float inventory;
    int longShort; // +1 for only Long, -1 for only Short
    std::list<LimitOrder> disbalancedOrders; // list of all filled limit orders which have not been balanced by
    // an opposite order.
    double realizedProfit; // is the total profit of all closed positions
    double positionRealizedProfit; // is the total profit of all de-cascading orders
    double originalDelta;

    void initiateRunners(double originalDelta);
    bool checkBuyFilled(Price price);
    bool checkSellFilled(Price price);
    void putOrders(Price price);
    int findProperRunnerIndex();
    void makeBuyFilled(Price price);
    void makeSellFilled(Price price);
    void correctOrdersLevel(double expectedDcLevel);
    std::list<LimitOrder> findCompensatedOrdersList(double levelOrder, double delta, int buySell);
    bool correctBuyLimitOrder(double expectedDcLevel);
    bool correctSellLimitOrder(double expectedDcLevel);
    void cancelSellLimitOrder();
    void cancelBuyLimitOrder();
    void correctThresholdsAndVolumes(float inventory);
    double computeLiqUniteCoef(double liquidity);
    bool positionCrossedTargetPnL(Price price);
    double getPositionTotalPnL(Price price);
    double getPositionUnrealizedProfit(Price price);
    void closePosition(Price price);
    bool marketOrderToClosePosition(Price price);
    void computeTargetRelatPnL();


public:
    CoastlineTrader(double originalDelta, int longShort);
    void run(Price price);

    double getPositionProfit(Price price);
    double getRealizedProfit();
};

#endif