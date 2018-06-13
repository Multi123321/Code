	public class Liquidity{
		public class Runner{
			public double prevDC;
			public double extreme;
			
			public double deltaUp;
			public double deltaDown;
			public int type;
			public boolean initalized;
			
			public String fileName;
			
			public Runner(double threshUp, double threshDown, PriceFeedData price, String file){
				prevDC = price.elems.mid; 
				extreme = price.elems.mid; 
				
				type = -1; deltaUp = threshUp; deltaDown = threshDown;initalized = true;
				fileName = new String(file);
			}
			
			public Runner(double threshUp, double threshDown, double price, String file){
				prevDC = price; extreme = price; 
				
				type = -1; deltaUp = threshUp; deltaDown = threshDown;  initalized = true;
				fileName = new String(file);
			}
			
			public Runner(double threshUp, double threshDown, String file){
				deltaUp = threshUp; deltaDown = threshDown;
				initalized = false;
				fileName = new String(file);
			}
			
			public int run(PriceFeedData price){
				if( price == null )
					return 0;
				
				if( !initalized ){
					type = -1; initalized = true;
					prevDC = price.elems.mid;
					extreme = price.elems.mid;
					return 0;
				}
				
				if( type == -1 ){
					if( Math.log(price.elems.bid/extreme) >= deltaUp ){
						type = 1;
						extreme = price.elems.ask; 
						prevDC = price.elems.ask;	
						return 1;
					}
					if( price.elems.ask < extreme ){
						extreme = price.elems.ask;
						return 0;
					}
				}
				else if( type == 1 ){
					if( Math.log(price.elems.ask/extreme) <= -deltaDown ){
						type = -1;
						extreme = price.elems.bid;
						prevDC = price.elems.bid; 
						return -1;
					}
					if( price.elems.bid > extreme ){
						extreme = price.elems.bid; 
						return 0;
					}
				}
				return 0;
			}
			
			public int run(double price){
				if( !initalized ){
					type = -1; initalized = true;
					prevDC = price;
					extreme = price; 
					return 0;
				}
				
				if( type == -1 ){
					if( price - extreme >= deltaUp ){
						type = 1;
						extreme = price; 
						prevDC = price;
						return 1;
					}
					if( price < extreme ){
						extreme = price;
						return 0;
					}
				}
				else if( type == 1 ){
					if( price - extreme <= -deltaDown ){
						type = -1;
						extreme = price; 
						prevDC = price; ;
						return 1;
					}
					if( price > extreme ){
						extreme = price;
						return 0;
					}
				}
				return 0;
			}
		}
		
		public Runner[] runner;
		double[] prevState;
		double surp = 0.0, dSurp = 0.0, uSurp = 0.0;
		double liquidity, liquidityUp, liquidityDown; 
		double liqEMA;
		double upLiq, downLiq, diffLiq, diffRaw;
		double H1 = 0.0, H2 = 0.0;
		double d1 = 0.0, d2 = 0.0;
		double alpha, alphaWeight;
		List<Double> mySurprise, downSurprise, upSurprise;
		
		public Liquidity(){};
		@SuppressWarnings("deprecation")
		public Liquidity(PriceFeedData price, double delta1, double delta2, int lgt){
			double prob = Math.exp(-1.0);
			H1 = -(prob*Math.log(prob) + (1.0 - prob)*Math.log(1.0 - prob));
			H2 = prob*Math.pow(Math.log(prob), 2.0) + (1.0 - prob)*Math.pow(Math.log(1.0 - prob), 2.0) - H1*H1;
			runner = new Runner[lgt];
			prevState = new double[lgt];
			d1 = delta1; d2 = delta2;
	
			getH1nH2(); //skip computation and assign!
			
			runner = new Runner[lgt];
			prevState = new double[lgt];
			
			for( int i = 0; i < runner.length; ++i ){
				runner[i] = new Runner(0.025/100.0 + 0.05/100.0*(double)i, 0.025/100.0 + 0.05/100.0*(double)i, price, "JustFake");
				runner[i].type = (i%2 == 0 ? 1 : -1);
				prevState[i] = (runner[i].type == 1 ? 1 : 0);
			}
			surp = H1; dSurp = H1; uSurp = H1;
			liquidity = 0.5; 
			liqEMA = 0.5;
			
			mySurprise = new LinkedList<Double>();
			downSurprise = new LinkedList<Double>();
			upSurprise = new LinkedList<Double>();
			for( int i = 0; i < 100; ++i ){
				mySurprise.add(new Double(H1));
				downSurprise.add(new Double(H1));
				upSurprise.add(new Double(H1));
			}
			
			//computeLiquidity();
			
			downLiq = 0.5; 
			upLiq = 0.5; 
			diffLiq = 0.5; 
			diffRaw = 0.0;
			alpha = 2.0/(100.0 + 1.0); 
			alphaWeight = Math.exp(-alpha); 
		}
		
		public void getH1nH2(){
			double price = 0.0; 
			alpha = 2.0/(100.0 + 1.0);
			alphaWeight = Math.exp(-alpha);
			runner = new Runner[runner.length];
			for( int i = 0; i < runner.length; ++i ){
				runner[i] = new Runner(0.025/100.0 + 0.05/100.0*(double)i, 0.025/100.0 + 0.05/100.0*(double)i, price, "JustFake");
				runner[i].type = (i%2 == 0 ? 1 : -1);
				prevState[i] = (runner[i].type == 1 ? 1 : 0);
			}
			
			double total1 = 0.0, total2 = 0.0;
			Random rand = new Random(1);
			double dt = 1.0/Math.sqrt(1000000.0);
			double sigma = 0.25; // 25%
			for( int i = 0; i < 100000000; ++i ){
				price += sigma*dt*rand.nextGaussian();
				for( int j= 0; j < runner.length; ++j ){
					if( Math.abs(runner[j].run(price)) == 1 ){ // this is OK for simulated prices
						double myProbs = getProbs(j);
						total1 = total1*alphaWeight + (1.0 - alphaWeight)*(-Math.log(myProbs));
						total2 = total2*alphaWeight + (1.0 - alphaWeight)*Math.pow(Math.log(myProbs), 2.0);
					}
				}
			}
			H1 = total1;
			H2 = total2 - H1*H1;
			System.out.println("H1:" + H1 + " H2:" + H2);
		}
		
		@SuppressWarnings("deprecation")
		public boolean Trigger(PriceFeedData price){
			// -- update values -- 
			boolean doComp = false;
			for( int i = 0; i < runner.length; ++i ){
				int value = runner[i].run(price);
				if( Math.abs(value) == 1 ){
					//double alpha = 2.0/(100.0 + 1.0);
					double myProbs = getProbs(i);
					surp = surp*alphaWeight + (1.0 - alphaWeight)*(-Math.log(myProbs));
					mySurprise.remove(0); mySurprise.add(new Double(-Math.log(myProbs)));
					if( runner[i].type == -1 ){
						dSurp = dSurp*alphaWeight + (1.0 - alphaWeight)*(-Math.log(myProbs));
						downSurprise.remove(0); downSurprise.add(new Double(-Math.log(myProbs)));
					}else if( runner[i].type == 1 ){
						uSurp = uSurp*alphaWeight + (1.0 - alphaWeight)*(-Math.log(myProbs));
						upSurprise.remove(0); upSurprise.add(new Double(-Math.log(myProbs)));
					}
					doComp = true;
				}
			}
			if( doComp ){
				liqEMA = (1.0 - CumNorm(Math.sqrt(100.0)*(surp - H1)/Math.sqrt(H2)));
				upLiq = (1.0 - CumNorm(Math.sqrt(100.0)*(uSurp - H1)/Math.sqrt(H2)));
				downLiq =  (1.0 - CumNorm(Math.sqrt(100.0)*(dSurp - H1)/Math.sqrt(H2)));
				diffLiq = CumNorm(Math.sqrt(100.0)*(uSurp - dSurp)/Math.sqrt(H2));
				diffRaw = Math.sqrt(100.0)*(uSurp-dSurp)/Math.sqrt(H2);
			}
			return doComp;
		}
		
		public double getProbs(int i){
			int where = -1;
			for( int j = 1; j < prevState.length; ++j ){
				if( prevState[j] != prevState[0] ){
					where = j;
					break;
				}
			}
			if( i > 0 && where != i ){
				System.out.println("This should not happen! " + where);
			}
			prevState[i] = (prevState[i] == 1 ? 0 : 1);
			
			if( where == 1 ){
				if( i > 0 ){
					return Math.exp(-(runner[1].deltaDown - runner[0].deltaDown)/runner[0].deltaDown);
				}else{
					return (1.0 - Math.exp(-(runner[1].deltaDown - runner[0].deltaDown)/runner[0].deltaDown));
				}
			}else if( where > 1 ){
				double numerator = 0.0;
				for( int k = 1; k <= where; ++k ){
					numerator -= (runner[k].deltaDown - runner[k-1].deltaDown)/runner[k-1].deltaDown;
				}
				numerator = Math.exp(numerator);
				double denominator = 0.0;
				for( int k = 1; k <= where - 1; ++k ){
					double secVal = 0.0;
					for( int j  = k+1; j <= where; ++j ){
						secVal -=  (runner[j].deltaDown - runner[j-1].deltaDown)/runner[j-1].deltaDown;
					}
					denominator += (1.0 - Math.exp(-(runner[k].deltaDown - runner[k-1].deltaDown)/runner[k-1].deltaDown))*Math.exp(secVal);
				}
				if( i > 0 ){
					return numerator/(1.0 - denominator);
				}else{
					return (1.0 - numerator/(1.0 - denominator));
				}
			}
			else{
				return 1.0;
			}
		}
		
		// Another implementation of the CNDF for a standard normal: N(0,1)
		double CumNorm(double x){
			// Protect against overflow
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
		
		public boolean computeLiquidity(long deltaT){
			double surpT = 0.0;
			double downSurp = 0.0, upSurp = 0.0;
			
			for( int i = 0; i < mySurprise.size(); ++i ){
				surpT += mySurprise.get(i).doubleValue();
				downSurp += downSurprise.get(i).doubleValue();
				upSurp += upSurprise.get(i).doubleValue();
			}
			
			liquidity = 1.0 - CumNorm((surpT - H1*mySurprise.size())/Math.sqrt(H2*mySurprise.size()));
			liquidityDown = 1.0 - CumNorm((downSurp - H1*downSurprise.size())/Math.sqrt(H2*downSurprise.size()));
			liquidityUp = 1.0 - CumNorm((upSurp - H1*upSurprise.size())/Math.sqrt(H2*upSurprise.size()));
			
			return true;
		}
	};