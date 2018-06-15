import java.util.*;
/* -- Price feed -- */
	// Simple
	public class PriceFeedData{
		LinkedList<Price> priceFeed = null;

		PriceFeedData(){
			this.priceFeed = new LinkedList<>();
		};

		public void addPrice(double ask, double bid, long time) {
			this.priceFeed.add(new Price(ask, bid, time));
		}
		
		public class Price{
			long   time;
			double ask;
			double bid;
			double mid;

			Price(double ask, double bid, long time) {
				this.ask = ask;
				this.bid = bid;
				this.time = time;
				this.mid = (ask+bid)/2.0;
			}

			public double getAsk() {
				return this.ask;
			}

			public double getBid() {
				return this.bid;
			}
			public double getMid() {
				return this.mid;
			}
		}
	};