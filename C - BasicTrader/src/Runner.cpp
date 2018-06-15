#include "Runner.h"

#include "PriceFeedData.h"
#include <math.h>
#include <string>

Runner::Runner()
{}

Runner::Runner(double threshUp, double threshDown, PriceFeedData price, std::string file, double dStarUp, double dStarDown) :
    deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    prevExtreme = price.elems.mid; 
    prevExtremeTime = price.elems.time;

    prevDC = price.elems.mid; 
    prevDCTime = price.elems.time;

    extreme = price.elems.mid; 
    extremeTime = price.elems.time;

    reference = price.elems.mid;
    
    type = -1; 
    osL = 0.0; 
    initalized = true;
    fileName = std::string(file);
}

Runner::Runner(double threshUp, double threshDown, double price, std::string file, double dStarUp, double dStarDown):
    deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    prevExtreme = price; 
    prevExtremeTime = 0;

    prevDC = price; 
    prevDCTime = 0;

    extreme = price; 
    extremeTime = 0;

    reference = price; 
    
    type = -1; 
    osL = 0.0; 
    initalized = true;
    fileName = std::string(file);
}

Runner::Runner(double threshUp, double threshDown, std::string file, double dStarUp, double dStarDown) :
    deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    initalized = false;
    fileName = std::string(file);
}

int Runner::run(PriceFeedData price)
{
    if( &price == NULL )
        return 0;

    if( !initalized ){
        type = -1; osL = 0.0; initalized = true;
        prevExtreme = price.elems.mid; prevExtremeTime = price.elems.time;
        prevDC = price.elems.mid; prevDCTime = price.elems.time;
        extreme = price.elems.mid; extremeTime = price.elems.time;
        reference = price.elems.mid;
        
        return 0;
    }

    if( type == -1 ){
        if( log(price.elems.bid/extreme) >= deltaUp ){
            prevExtreme = extreme;
            prevExtremeTime = extremeTime;
            type = 1;
            extreme = price.elems.ask; extremeTime = price.elems.time;
            prevDC = price.elems.ask; prevDCTime = price.elems.time;
            reference = price.elems.ask;		
            return 1;
        }
        if( price.elems.ask < extreme ){
            extreme = price.elems.ask;
            extremeTime = price.elems.time;
            osL = -1 * log(extreme/prevDC)/deltaDown;
            
            if( log(extreme/reference) <= -deltaStarUp ){
                reference = extreme;
                return -2;
            }
            return 0;
        }
    }
    else if( type == 1 ){
        if( log(price.elems.ask/extreme) <= -deltaDown ){
            prevExtreme = extreme; 
            prevExtremeTime = extremeTime;
            type = -1;
            extreme = price.elems.bid; extremeTime = price.elems.time;
            prevDC = price.elems.bid; prevDCTime = price.elems.time;
            reference = price.elems.bid;
            return -1;
        }
        if( price.elems.bid > extreme ){
            extreme = price.elems.bid; 
            extremeTime = price.elems.time;
            osL = log(extreme/prevDC)/deltaUp;
            
            if( log(extreme/reference) >= deltaStarDown ){
                reference = extreme;
                return 2;
            }
            return 0;
        }
    }
    return 0;
}

int Runner::run(double price)
{
    if( !initalized ){
        type = -1; osL = 0.0; initalized = true;
        prevExtreme = price; prevExtremeTime = 0;
        prevDC = price; prevDCTime = 0;
        extreme = price; extremeTime = 0;
        reference = price;
        return 0;
    }
    
    if( type == -1 ){
        if( price - extreme >= deltaUp ){
            prevExtreme = extreme;
            prevExtremeTime = extremeTime;
            type = 1;
            extreme = price; extremeTime = 0;
            prevDC = price; prevDCTime = 0;
            reference = price;
            osL = 0.0;
            
            return 1;
        }
        if( price < extreme ){
            extreme = price;
            extremeTime = 0;
            osL = -(extreme - prevDC);
            if( extreme - reference <= -deltaStarUp ){
                reference = extreme;
                return -2;
            }
            return 0;
        }
    }
    else if( type == 1 ){
        if( price - extreme <= -deltaDown ){
            prevExtreme = extreme; prevExtremeTime = extremeTime;
            type = -1;
            extreme = price; extremeTime = 0;
            prevDC = price; prevDCTime = 0;
            reference = price;
            osL = 0.0;
            
            return 1;
        }
        if( price > extreme ){
            extreme = price; extremeTime = 0;
            osL = (extreme -prevDC);
            if( extreme - reference >= deltaStarDown ){
                reference = extreme;
                return 2;
            }
            return 0;
        }
    }
    return 0;
}