	public class Prices{
		double bid;
		double ask;
		Prices(){};
		Prices(Prices p){
			bid = p.bid;
			ask = p.ask;
		}
		Prices(double b, double a){
			bid = b;
			ask = a;
		}
	};