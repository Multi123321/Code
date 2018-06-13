public class LocalLiquidity{
		double deltaUp, deltaDown;
		double delta;
		double extreme, dStar, reference;
		int type;
		boolean initalized;
		
		double surp, upSurp, downSurp;
		double liq, upLiq, downLiq;
		double alpha, alphaWeight;
		double H1, H2;
		
		LocalLiquidity(){};
		LocalLiquidity(double d, double dUp, double dDown, double dS, double a){
			type = -1; deltaUp = dUp; deltaDown = dDown; dStar = dS; delta = d;
			initalized = false;
			alpha = a;
			alphaWeight = Math.exp(-2.0/(a + 1.0));
			computeH1H2exp(dS);
		}		
		LocalLiquidity(double d,double dUp, double dDown, PriceFeedData price, double dS, double a){
			deltaUp = dUp; deltaDown = dDown; delta = d;
			type = -1;
			extreme = reference = price.elems.mid;
			dStar = dS;
			initalized = true;
			alpha = a;
			alphaWeight = Math.exp(-2.0/(a + 1.0));
			computeH1H2exp(dS);
		}
		boolean computeH1H2exp(double dS){
			H1 = -Math.exp(-dStar/delta)*Math.log(Math.exp(-dStar/delta)) - (1.0 - Math.exp(-dStar/delta))*Math.log(1.0 - Math.exp(-dStar/delta));
			H2 = Math.exp(-dStar/delta)*Math.pow(Math.log(Math.exp(-dStar/delta)), 2.0) - (1.0 - Math.exp(-dStar/delta))*Math.pow(Math.log(1.0 - Math.exp(-dStar/delta)), 2.0) - H1*H1;
			return true;
		}
		// Another implementation of the CNDF for a standard normal: N(0,1)
		double CumNorm(double x){
			// protect against overflow
			if (x > 6.0)
				return 1.0;
			if (x < -6.0)
				return 0.0;
				 
			double b1 = 0.31938153;
			double b2 = -0.356563782;
			double b3 = 1.781477937;
			double b4 = -1.821255978;
			double b5 = 1.330274429;
			double p = 0.2316419;
			double c2 = 0.3989423;
				 
			double a = Math.abs(x);
			double t = 1.0 / (1.0 + a * p);
			double b = c2*Math.exp((-x)*(x/2.0));
			double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
			n = 1.0-b*n;
					
			if ( x < 0.0 )
				n = 1.0 - n;

			return n;
		}
		
		public int run(PriceFeedData price){
			if( price == null )
				return 0;
			
			if( !initalized ){
				type = -1; initalized = true;
				extreme = reference = price.elems.mid;
				return 0;
			}
			
			if( type == -1 ){
				if( Math.log(price.elems.bid/extreme) >= deltaUp ){
					type = 1;
					extreme = price.elems.ask;
					reference = price.elems.ask;
					return 1;
				}
				if( price.elems.ask < extreme ){
					extreme = price.elems.ask;
				}
				if( Math.log(reference/extreme) >= dStar ){
					reference = extreme;
					return 2;
				}
			}
			else if( type == 1 ){
				if( Math.log(price.elems.ask/extreme) <= -deltaDown ){
					type = -1;
					extreme = price.elems.bid; 
					reference = price.elems.bid;
					return -1;
				}
				if( price.elems.bid > extreme ){
					extreme = price.elems.bid; 
				}
				if( Math.log(reference/extreme) <= -dStar ){
					reference = extreme;
					return -2;
				}
			}
			return 0;
		}
		
		public boolean computation(PriceFeedData price){
			if( price == null )
				return false;
			
			int event = run(price);
			if( event != 0 ){
				surp = alphaWeight*(Math.abs(event) == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*surp;
				
				if( event > 0 ){ // down moves
					downSurp = alphaWeight*(event == 1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*downSurp;
				}else if( event < 0 ){ // up moves
					upSurp = alphaWeight*(event == -1 ? 0.08338161 : 2.525729) + (1.0 - alphaWeight)*upSurp;
				}
				
				liq = 1.0 - CumNorm(Math.sqrt(alpha)*(surp - H1)/Math.sqrt(H2)); 
				upLiq = 1.0 - CumNorm(Math.sqrt(alpha)*(upSurp - H1)/Math.sqrt(H2)); 
				downLiq = 1.0 - CumNorm(Math.sqrt(alpha)*(downSurp - H1)/Math.sqrt(H2)); 
			}
			return true;
		}
	};