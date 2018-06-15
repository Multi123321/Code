#include "Runner.h"

#include "PriceFeedData.h"
#include <math.h>
#include <string>

Runner::Runner()
{}

Runner::Runner(double threshUp, double threshDown, PriceFeedData::Price price, std::string file, double dStarUp, double dStarDown) :
    deltaUp(threshUp), deltaDown(threshDown), deltaStarUp(dStarUp), deltaStarDown(dStarDown)
{
    prevExtreme = price.mid; 
    prevExtremeTime = price.time;

    prevDC = price.mid; 
    prevDCTime = price.time;

    extreme = price.mid; 
    extremeTime = price.time;

    reference = price.mid;
    
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

int Runner::run(PriceFeedData::Price price)
{
    //TODO: search for better way...
    // if( &price == NULL )
    //     return 0;

    if( !initalized ){
        type = -1; osL = 0.0; initalized = true;
        prevExtreme = price.mid; prevExtremeTime = price.time;
        prevDC = price.mid; prevDCTime = price.time;
        extreme = price.mid; extremeTime = price.time;
        reference = price.mid;
        
        return 0;
    }

    if( type == -1 ){
        if( log(price.bid/extreme) >= deltaUp ){
            prevExtreme = extreme;
            prevExtremeTime = extremeTime;
            type = 1;
            extreme = price.ask; extremeTime = price.time;
            prevDC = price.ask; prevDCTime = price.time;
            reference = price.ask;		
            return 1;
        }
        if( price.ask < extreme ){
            extreme = price.ask;
            extremeTime = price.time;
            osL = -1 * log(extreme/prevDC)/deltaDown;
            
            if( log(extreme/reference) <= -deltaStarUp ){
                reference = extreme;
                return -2;
            }
            return 0;
        }
    }
    else if( type == 1 ){
        if( log(price.ask/extreme) <= -deltaDown ){
            prevExtreme = extreme; 
            prevExtremeTime = extremeTime;
            type = -1;
            extreme = price.bid; extremeTime = price.time;
            prevDC = price.bid; prevDCTime = price.time;
            reference = price.bid;
            return -1;
        }
        if( price.bid > extreme ){
            extreme = price.bid; 
            extremeTime = price.time;
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