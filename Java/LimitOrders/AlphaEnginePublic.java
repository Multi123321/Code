
public class AlphaEnginePublic {

    public static void main(String[] args) {
        AlphaEngine alphaEngine = new AlphaEngine();
        Price[] priceFeed = CSVReader.readExchangeFromFile("../../EUR_USD.csv");

        for (Price price : priceFeed){ // please use your own price feed
            alphaEngine.run(price);
        }

        alphaEngine.printAllProfits();
    }

}