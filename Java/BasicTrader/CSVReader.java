import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;

public class CSVReader{

    public static PriceFeedData readExchangeFromFile(String filepath)
    {
        String csvFile = filepath;
        String line = "";
        String cvsSplitBy = ";";
        PriceFeedData prices = new PriceFeedData();

        try (BufferedReader br = new BufferedReader(new FileReader(csvFile))) {
            
            br.readLine();

            while ((line = br.readLine()) != null) {

                // use comma as separator
                String[] splitLine = line.split(cvsSplitBy);

                prices.addPrice(Float.valueOf(splitLine[2])+0.0001, Float.valueOf(splitLine[2])-0.0001, Long.valueOf(splitLine[1])*1000);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
        return prices;
    }
}