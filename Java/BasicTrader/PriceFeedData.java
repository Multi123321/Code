	/* -- Price feed -- */
	// Simple
	public class PriceFeedData{
		// TODO Implement your feed
		Elems elems;
		double ask;
		PriceFeedData(){
			elems = new Elems();
		};
		
		public class Elems{
			double mid = 1.1;
			double ask = 1.1;
			double bid = 1.0;
			long time = System.currentTimeMillis();
		}
	};