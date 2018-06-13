import java.util.LinkedList;
import java.util.List;

public class CoastlineTrader{
		double tP; /* -- Total position -- */
		List<Double> prices;
		List<Double> sizes;
		
		double profitTarget;
		double pnl, tempPnl;
		double deltaUp, deltaDown, deltaLiq, deltaOriginal;
		double shrinkFlong, shrinkFshort;
		
		double pnlPerc;
		
		int longShort;
		
		boolean initalized;
		Runner runner;
		Runner[][] runnerG;
		
		double increaseLong, increaseShort;
		
		double lastPrice;
		
		double cashLimit;
		String fxRate;
		
		LocalLiquidity liquidity;
		
		CoastlineTrader(){};
		CoastlineTrader(double dOriginal, double dUp, double dDown, double profitT, String FxRate, int lS){
			prices = new LinkedList<Double>();
			sizes = new LinkedList<Double>();
			tP = 0.0; /* -- Total position -- */
			
			profitTarget = cashLimit = profitT;
			pnl = tempPnl = pnlPerc = 0.0;
			deltaOriginal = dOriginal;
			deltaUp = dUp; deltaDown = dDown;
			longShort = lS; // 1 for only longs, -1 for only shorts
			shrinkFlong = shrinkFshort = 1.0;
			increaseLong = increaseShort = 0.0;
			
			fxRate = new String(FxRate);
		}
		
		double computePnl(PriceFeedData price){
			// TODO:
			// Compute PnL with current price
			return 0.0;
		}
		
		double computePnlLastPrice(){
			// TODO:
			// Compute PnL with last available price
			return 0.0;
		}
		double getPercPnl(PriceFeedData price){
			// TODO:
			// Percentage PnL
			return 0.0;
		}
		
		boolean tryToClose(PriceFeedData price){
			// TODO:
			// Check if PnL target hit implementation
			return false;
		}
		
		boolean assignCashTarget(){
			// TODO:
			// Compute cash value corresponding to percentage PnL 
			return true;
		}
		
		@SuppressWarnings("deprecation")
		boolean runPriceAsymm(PriceFeedData price, double oppositeInv){
			if( !initalized ){
				runner = new Runner(deltaUp, deltaDown, price, fxRate, deltaUp, deltaDown);
				
				runnerG = new Runner[2][2];
				
				runnerG[0][0] = new Runner(0.75*deltaUp, 1.50*deltaDown, price, fxRate, 0.75*deltaUp, 0.75*deltaUp);
				runnerG[0][1] = new Runner(0.50*deltaUp, 2.00*deltaDown, price, fxRate, 0.50*deltaUp, 0.50*deltaUp);
				
				runnerG[1][0] = new Runner(1.50*deltaUp, 0.75*deltaDown, price, fxRate, 0.75*deltaDown, 0.75*deltaDown);
				runnerG[1][1] = new Runner(2.00*deltaUp, 0.50*deltaDown, price, fxRate, 0.50*deltaDown, 0.50*deltaDown);
				
				liquidity = new LocalLiquidity(deltaOriginal, deltaUp, deltaDown, price, deltaOriginal*2.525729, 50.0);
				initalized = true;
			}
			
			if( !liquidity.computation(price) ){
				System.out.println("Didn't compute liquidity!");
			}
			
			if( tryToClose(price) ){ /* -- Try to close position -- */
				System.out.println("Close");
				return true;
			}
			
			int event = 0;
			
			double fraction = 1.0;
			double size = (liquidity.liq < 0.5 ? 0.5 : 1.0);
			size = (liquidity.liq < 0.1 ? 0.1 : size);
			
			if( longShort == 1 ){ // Long positions only
				event = runner.run(price);
				
				if( 15.0 <= tP && tP < 30.0 ){
					event = runnerG[0][0].run(price);
					runnerG[0][1].run(price);
					fraction = 0.5;
				}else if( tP >= 30.0 ){
					event = runnerG[0][1].run(price);
					runnerG[0][0].run(price);
					fraction = 0.25;
				}else{
					runnerG[0][0].run(price); runnerG[0][1].run(price);
				}
				
				if( event < 0 ){
					if( tP == 0.0 ){ // Open long position
						int sign = -runner.type;
						if( Math.abs(oppositeInv) > 15.0 ){
							size = 1.0;
							if( Math.abs(oppositeInv) > 30.0 ){
								size = 1.0;
							}
						}
						double sizeToAdd = sign*size; 
						tP += sizeToAdd;
						sizes.add(new Double(sizeToAdd));
						
						prices.add(new Double(sign == 1 ? price.elems.ask : price.elems.bid));
						assignCashTarget();
						System.out.println("Open long");
						
					}
					else if( tP > 0.0 ){ // Increase long position (buy)
						int sign = -runner.type;
						double sizeToAdd = sign*size*fraction*shrinkFlong;
						if( sizeToAdd < 0.0 ){
							System.out.println("How did this happen! increase position but neg size: " + sizeToAdd);
							sizeToAdd = -sizeToAdd; 
						}
						increaseLong += 1.0;
						tP += sizeToAdd;						
						sizes.add(new Double(sizeToAdd));
						
						prices.add(new Double(sign == 1 ? price.elems.ask : price.elems.bid));
						System.out.println("Cascade");
					}
				}
				else if( event > 0 &&  tP > 0.0 ){ // Possibility to decrease long position only at intrinsic events
					double pricE = (tP > 0.0 ? price.elems.bid : price.elems.ask);
					
					for( int i = 1; i < prices.size(); ++i ){
						double tempP = (tP > 0.0 ? Math.log(pricE/prices.get(i).doubleValue()) : Math.log(prices.get(i).doubleValue()/pricE));
						if( tempP >= (tP > 0.0 ? deltaUp : deltaDown) ){
							double addPnl = (pricE - prices.get(i).doubleValue())*sizes.get(i).doubleValue();
							if( addPnl < 0.0 ){
								System.out.println("Descascade with a loss: " + addPnl);
							}
							tempPnl += addPnl;
							tP -= sizes.get(i).doubleValue();
							sizes.remove(i); prices.remove(i);
							increaseLong += -1.0;
							System.out.println("Decascade");
						}
					}
				}
			}
			else if( longShort == -1 ){ // Short positions only
				event = runner.run(price);
				if( -30.0 < tP && tP < -15.0 ){
					event = runnerG[1][0].run(price);
					runnerG[1][1].run(price);
					fraction = 0.5;
				}else if( tP <= -30.0 ){
					event = runnerG[1][1].run(price);
					runnerG[1][0].run(price);
					fraction = 0.25;
				}else{
					runnerG[1][0].run(price); runnerG[1][1].run(price);
				}
				
				if( event > 0 ){
					if( tP == 0.0 ){ // Open short position
						int sign = -runner.type;
						if( Math.abs(oppositeInv) > 15.0 ){
							size = 1.0;
							if( Math.abs(oppositeInv) > 30.0 ){
								size = 1.0;
							}
						}
						double sizeToAdd = sign*size;
						if( sizeToAdd > 0.0 ){
							System.out.println("How did this happen! increase position but pos size: " + sizeToAdd);
							sizeToAdd = -sizeToAdd;
						}
						tP += sizeToAdd;
						sizes.add(new Double(sizeToAdd));
						
						prices.add(new Double(sign == 1 ? price.elems.bid : price.elems.ask));
						System.out.println("Open short");
						assignCashTarget();
					}else if( tP < 0.0 ){
						int sign = -runner.type;
						double sizeToAdd = sign*size*fraction*shrinkFshort;
						if( sizeToAdd > 0.0 ){
							System.out.println("How did this happen! increase position but pos size: " + sizeToAdd);
							sizeToAdd = -sizeToAdd;
						}
						
						tP += sizeToAdd;
						sizes.add(new Double(sizeToAdd));
						increaseShort += 1.0;
						prices.add(new Double(sign == 1 ? price.elems.bid : price.elems.ask));
						System.out.println("Cascade");
					}
				}
				else if( event < 0.0 && tP < 0.0 ){
					double pricE = (tP > 0.0 ? price.elems.bid : price.elems.ask);
					
					for( int i = 1; i < prices.size(); ++i ){
						double tempP = (tP > 0.0 ? Math.log(pricE/prices.get(i).doubleValue()) : Math.log(prices.get(i).doubleValue()/pricE));
						if( tempP >= (tP > 0.0 ? deltaUp : deltaDown) ){
							double addPnl = (pricE - prices.get(i).doubleValue())*sizes.get(i).doubleValue();
							if( addPnl < 0.0 ){
								System.out.println("Descascade with a loss: " + addPnl);
							}
							tempPnl += (pricE - prices.get(i).doubleValue())*sizes.get(i).doubleValue();
							tP -= sizes.get(i).doubleValue();
							sizes.remove(i); prices.remove(i);
							increaseShort += -1.0;
							System.out.println("Decascade");
						}
					}
				}
			}
			else{
				System.out.println("Should never happen! " + longShort);
			}
			return true;
		}
	};