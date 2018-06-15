import java.util.LinkedList;
import java.util.List;

public class NbDcCounter{
	List<Long> eventList;
	double delta;
	long timeWindow;
	Runner runner;
	
	NbDcCounter(){};
	NbDcCounter(double d, long tW){
		eventList = new LinkedList<Long>();
		delta = d;
		runner = new Runner(delta, delta, "events", delta, delta);
		timeWindow = tW;
	}
	@SuppressWarnings("deprecation")
	boolean run(PriceFeedData.Price price){
		if( Math.abs(runner.run(price)) == 1 ){
			eventList.add(new Long(price.time));
		}
		
		if( eventList.size() == 0 )
			return true;
		
		while( eventList.get(0).longValue() < price.time - timeWindow )
			eventList.remove(0);
		
		return true;
	}
};