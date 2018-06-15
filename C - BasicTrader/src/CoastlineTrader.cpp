#include "CoastlineTrader.h"
#include <string>
#include <deque>
#include <iostream>
#include <cmath>
#include <stdlib.h>

CoastlineTrader::CoastlineTrader()
{}

CoastlineTrader::CoastlineTrader(double dOriginal, double dUp, double dDown, double profitT, string FxRate, int lS) :
    deltaOriginal(dOriginal), deltaUp(dUp), deltaDown(dDown), 
    longShort(lS), profitTarget(profitT), cashLimit(profitT)
{
    tP = 0.0; /* -- Total position -- */
    
    pnl = tempPnl = pnlPerc = 0.0;
    shrinkFlong = shrinkFshort = 1.0;
    increaseLong = increaseShort = 0.0;
    
    fxRate = FxRate;
}

double CoastlineTrader::computePnl(PriceFeedData price)
{
    double profitLoss = 0;
    for (int i = 0; i  < sizes.size(); i++) 
    {
        profitLoss += sizes[i]*(price.elems.bid-prices[i]);
    }
    return profitLoss;
}

double CoastlineTrader::computePnlLastPrice()
{
    double lastPrice = *(prices.end());
    double profitLoss = 0;
    for (int i = 0; i  < sizes.size(); i++) 
    {
        profitLoss += sizes[i]*(lastPrice-prices[i]);
    }
    return profitLoss;
}

double CoastlineTrader::getPercPnl(PriceFeedData price)
{
    // TODO:
    // Percentage PnL
    return 0.0;
}

bool CoastlineTrader::tryToClose(PriceFeedData price)
{
    return ((computePnl(price)+tempPnl) >= profitTarget);
}

bool CoastlineTrader::assignCashTarget()
{
    // TODO:
    // Compute cash value corresponding to percentage PnL 
    return true;
}

bool CoastlineTrader::runPriceAsymm(PriceFeedData price, double oppositeInv)
{
    if( !initalized ){
        runner = new Runner(deltaUp, deltaDown, price, fxRate, deltaUp, deltaDown);
                
        runnerG[0][0] = new Runner(0.75*deltaUp, 1.50*deltaDown, price, fxRate, 0.75*deltaUp, 0.75*deltaUp);
        runnerG[0][1] = new Runner(0.50*deltaUp, 2.00*deltaDown, price, fxRate, 0.50*deltaUp, 0.50*deltaUp);
        
        runnerG[1][0] = new Runner(1.50*deltaUp, 0.75*deltaDown, price, fxRate, 0.75*deltaDown, 0.75*deltaDown);
        runnerG[1][1] = new Runner(2.00*deltaUp, 0.50*deltaDown, price, fxRate, 0.50*deltaDown, 0.50*deltaDown);
        
        liquidity = new LocalLiquidity(deltaOriginal, deltaUp, deltaDown, price, deltaOriginal*2.525729, 50.0);
        initalized = true;
    }
    
    if( !liquidity->computation(price) ){
        cout << "Didn't compute liquidity!";
    }
    
    if( tryToClose(price) ){ /* -- Try to close position -- */
        cout << "Close";
        return true;
    }
    
    int event = 0;
    
    double fraction = 1.0;
    double size = (liquidity->liq < 0.5 ? 0.5 : 1.0);
    size = (liquidity->liq < 0.1 ? 0.1 : size);
    
    if( longShort == 1 ){ // Long positions only
        event = runner->run(price);
        
        if( 15.0 <= tP && tP < 30.0 ){
            event = runnerG[0][0]->run(price);
            runnerG[0][1]->run(price);
            fraction = 0.5;
        }else if( tP >= 30.0 ){
            event = runnerG[0][1]->run(price);
            runnerG[0][0]->run(price);
            fraction = 0.25;
        }else{
            runnerG[0][0]->run(price); runnerG[0][1]->run(price);
        }
        
        if( event < 0 ){
            if( tP == 0.0 ){ // Open long position
                int sign = -runner->type;
                if( std::abs(oppositeInv) > 15.0 ){
                    size = 1.0;
                    if( std::abs(oppositeInv) > 30.0 ){
                        size = 1.0;
                    }
                }
                double sizeToAdd = sign*size; 
                tP += sizeToAdd;
                sizes.push_front(sizeToAdd);
                
                prices.push_front(sign == 1 ? price.elems.ask : price.elems.bid);
                assignCashTarget();
                cout << "Open long";
                
            }
            else if( tP > 0.0 ){ // Increase long position (buy)
                int sign = -runner->type;
                double sizeToAdd = sign*size*fraction*shrinkFlong;
                if( sizeToAdd < 0.0 ){
                    cout << "How did this happen! increase position but neg size: " << sizeToAdd;
                    sizeToAdd = -sizeToAdd; 
                }
                increaseLong += 1.0;
                tP += sizeToAdd;						
                sizes.push_front(sizeToAdd);
                
                prices.push_front(sign == 1 ? price.elems.ask : price.elems.bid);
                cout << "Cascade";
            }
        }
        else if( event > 0 &&  tP > 0.0 ){ // Possibility to decrease long position only at intrinsic events
            double pricE = (tP > 0.0 ? price.elems.bid : price.elems.ask);
            
            for( int i = 1; i < prices.size(); ++i ){
                double tempP = (tP > 0.0 ? log(pricE/prices[i]) : log(prices[i]/pricE));
                if( tempP >= (tP > 0.0 ? deltaUp : deltaDown) ){
                    double addPnl = (pricE - prices[i])*sizes[i];
                    if( addPnl < 0.0 ){
                        cout << "Descascade with a loss: " << addPnl;
                    }
                    tempPnl += addPnl;
                    tP -= sizes[i];
                    sizes.erase(sizes.begin() + i); prices.erase(sizes.begin() + i);
                    increaseLong += -1.0;
                    cout << "Decascade";
                }
            }
        }
    }
    else if( longShort == -1 ){ // Short positions only
        event = runner->run(price);
        if( -30.0 < tP && tP < -15.0 ){
            event = runnerG[1][0]->run(price);
            runnerG[1][1]->run(price);
            fraction = 0.5;
        }else if( tP <= -30.0 ){
            event = runnerG[1][1]->run(price);
            runnerG[1][0]->run(price);
            fraction = 0.25;
        }else{
            runnerG[1][0]->run(price); runnerG[1][1]->run(price);
        }
        
        if( event > 0 ){
            if( tP == 0.0 ){ // Open short position
                int sign = -runner->type;
                if( abs(oppositeInv) > 15.0 ){
                    size = 1.0;
                    if( abs(oppositeInv) > 30.0 ){
                        size = 1.0;
                    }
                }
                double sizeToAdd = sign*size;
                if( sizeToAdd > 0.0 ){
                    cout << "How did this happen! increase position but pos size: " << sizeToAdd;
                    sizeToAdd = -sizeToAdd;
                }
                tP += sizeToAdd;
                sizes.push_front(sizeToAdd);
                
                prices.push_front(sign == 1 ? price.elems.bid : price.elems.ask);
                cout << "Open short";
                assignCashTarget();
            }else if( tP < 0.0 ){
                int sign = -runner->type;
                double sizeToAdd = sign*size*fraction*shrinkFshort;
                if( sizeToAdd > 0.0 ){
                    cout << "How did this happen! increase position but pos size: " << sizeToAdd;
                    sizeToAdd = -sizeToAdd;
                }
                
                tP += sizeToAdd;
                sizes.push_front(sizeToAdd);
                increaseShort += 1.0;
                prices.push_front(sign == 1 ? price.elems.bid : price.elems.ask);
                cout << "Cascade";
            }
        }
        else if( event < 0.0 && tP < 0.0 ){
            double pricE = (tP > 0.0 ? price.elems.bid : price.elems.ask);
            
            for( int i = 1; i < prices.size(); ++i ){
                double tempP = (tP > 0.0 ? log(pricE/prices[i]) : log(prices[i]/pricE));
                if( tempP >= (tP > 0.0 ? deltaUp : deltaDown) ){
                    double addPnl = (pricE - prices[i])*sizes[i];
                    if( addPnl < 0.0 ){
                        cout << "Descascade with a loss: " << addPnl;
                    }
                    tempPnl += (pricE - prices[i])*sizes[i];
                    tP -= sizes[i];
                    sizes.erase(sizes.begin() + i); prices.erase(sizes.begin() + i);
                    increaseShort += -1.0;
                    cout << "Decascade";
                }
            }
        }
    }
    else{
        cout << "Should never happen! " << longShort;
    }
    return true;
}