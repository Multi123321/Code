	public class Runner{
		public double prevExtreme;
		public long prevExtremeTime;
		
		public double prevDC;
		public long prevDCTime;
		
		public double extreme;
		public long extremeTime;
		
		public double deltaUp;
		public double deltaDown;
		public double deltaStarUp, deltaStarDown;
		public double osL;
		public int type;
		public boolean initalized;
		public double reference;
		
		public String fileName;
		
		public Runner(double threshUp, double threshDown, PriceFeedData.Price price, String file, double dStarUp, double dStarDown){
			prevExtreme = price.mid; prevExtremeTime = price.time;
			prevDC = price.mid; prevDCTime = price.time;
			extreme = price.mid; extremeTime = price.time;
			reference = price.mid;
			
			type = -1; deltaUp = threshUp; deltaDown = threshDown; osL = 0.0; initalized = true;
			fileName = new String(file);
			deltaStarUp = dStarUp; deltaStarDown = dStarDown;
		}
		
		public Runner(double threshUp, double threshDown, double price, String file, double dStarUp, double dStarDown){
			prevExtreme = price; prevExtremeTime = 0;
			prevDC = price; prevDCTime = 0;
			extreme = price; extremeTime = 0;
			reference = price; 
			deltaStarUp = dStarUp; deltaStarDown = dStarDown;
			
			type = -1; deltaUp = threshUp; deltaDown = threshDown; osL = 0.0; initalized = true;
			fileName = new String(file);
		}
		
		public Runner(double threshUp, double threshDown, String file, double dStarUp, double dStarDown){
			deltaUp = threshUp; deltaDown = threshDown;
			initalized = false;
			fileName = new String(file);
			deltaStarUp = dStarUp; deltaStarDown = dStarDown;
		}
		
		public int run(PriceFeedData.Price price){
			if( price == null )
				return 0;
			
			if( !initalized ){
				type = -1; osL = 0.0; initalized = true;
				prevExtreme = price.mid; prevExtremeTime = price.time;
				prevDC = price.mid; prevDCTime = price.time;
				extreme = price.mid; extremeTime = price.time;
				reference = price.mid;
				
				return 0;
			}
			
			if( type == -1 ){
				if( Math.log(price.bid/extreme) >= deltaUp ){
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
					osL = -Math.log(extreme/prevDC)/deltaDown;
					
					if( Math.log(extreme/reference) <= -deltaStarUp ){
						reference = extreme;
						return -2;
					}
					return 0;
				}
			}
			else if( type == 1 ){
				if( Math.log(price.ask/extreme) <= -deltaDown ){
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
					osL = Math.log(extreme/prevDC)/deltaUp;
					
					if( Math.log(extreme/reference) >= deltaStarDown ){
						reference = extreme;
						return 2;
					}
					return 0;
				}
			}
			return 0;
		}
		
		public int run(double price){
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
	}