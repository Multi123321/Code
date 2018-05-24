
public class AlphaEngine {

    private CoastlineTrader[] longCoastlineTraders;
    private CoastlineTrader[] shortCoastlineTraders;

    public AlphaEngine(){
        initiateTraders();
    }


    public void run(Price price){
        for (CoastlineTrader coastlineTrader : longCoastlineTraders){
            coastlineTrader.run(price);
        }
        for (CoastlineTrader coastlineTrader: shortCoastlineTraders){
            coastlineTrader.run(price);
        }
    }

    public void printAllProfits()
    {
        for (CoastlineTrader coastlineTrader : longCoastlineTraders){
            System.out.println("Profit: " + coastlineTrader.getRealizedProfit());
        }
        for (CoastlineTrader coastlineTrader: shortCoastlineTraders){
            System.out.println("Profit: " + coastlineTrader.getRealizedProfit());
        }

    }


    private void initiateTraders(){
        longCoastlineTraders = new CoastlineTrader[4];
        longCoastlineTraders[0] = new CoastlineTrader(0.0025, 1);
        longCoastlineTraders[1] = new CoastlineTrader(0.005, 1);
        longCoastlineTraders[2] = new CoastlineTrader(0.01, 1);
        longCoastlineTraders[3] = new CoastlineTrader(0.015, 1);
        shortCoastlineTraders = new CoastlineTrader[4];
        shortCoastlineTraders[0] = new CoastlineTrader(0.0025, -1);
        shortCoastlineTraders[1] = new CoastlineTrader(0.005, -1);
        shortCoastlineTraders[2] = new CoastlineTrader(0.01, -1);
        shortCoastlineTraders[3] = new CoastlineTrader(0.015, -1);
    }

}
