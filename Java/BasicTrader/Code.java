/* -- NB: 
 * See the folder
 * 	Code/tree/master/LimitOrders
 * for eight Java classes implementing the trading model algorithm for limit orders.
 *  
 * The following code is intended to give the reader an overview of how the
 * trading model algorithm functions. However, the focus lies not on the framework
 * supporting the trading model. As a result, users are required to implement some
 * of their own code in order to get the trading model running. Specifically, this
 * concerns your price feed data and the take profit target. The only configuration
 * is the number and sizes of the thresholds on which the Coastline Traders live.
 * See all the TODO in the code. — */

import java.io.FileWriter;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

public class Code {
	
	// Main method
	public static void main(String[] args) {
		new Code();
	}
	
	// Run algo
	public Code(){
		// TODO Currency configuration
		String[] ccyList = {"AUD_CAD"};/* , "AUD_JPY", "AUD_NZD", "AUD_USD", "CAD_JPY", "CHF_JPY", "EUR_AUD", "EUR_CAD", "EUR_CHF",
				"EUR_GBP", "EUR_JPY", "EUR_NZD", "EUR_USD", "GBP_AUD", "GBP_CAD", "GBP_CHF", "GBP_JPY", "GBP_USD", "NZD_CAD",
				"NZD_JPY", "NZD_USD", "USD_CAD", "USD_CHF", "USD_JPY"}; */
		int length = ccyList.length;
		
		FXrateTrading[] trading = new FXrateTrading[length];
		
		// TODO Threshold configuration  (see below)
		double[] deltaS = {0.25/100.0, 0.5/100.0, 1.0/100.0, 1.5/100.0};
		for( int i = 0; i < length; ++i ){
			trading[i] = new FXrateTrading(ccyList[i], deltaS.length, deltaS);
		}
		
		// Run
		String sep = new String(System.getProperty("file.separator"));
		for( int i = 0; i < length; ++i ){
			PriceFeedData priceFeed = CSVReader.readExchangeFromFile(".."+sep+".."+sep+"EUR_USD.csv");
			for (PriceFeedData.Price price : priceFeed.priceFeed){
				trading[i].runTradingAsymm(price);
			}
		}
	}
	

	/* -- Configuration
	// TODO
	public static class Configuration{
		public double startThreshold;
		public int numberOf;
		public double endThreshold;
		
		public Configuration() {
			startThreshold = 0.001;
			numberOf = 10;
			endThreshold = 0.05;
		}
	}
	*/
}