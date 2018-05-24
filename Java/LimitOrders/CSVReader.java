import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;

public class CSVReader {

    public static Price[] readExchangeFromFile(String filepath) 
    {
        String csvFile = filepath;
        String line = "";
        String cvsSplitBy = ";";
        LinkedList<Price> priceList = new LinkedList<Price>();

        try (BufferedReader br = new BufferedReader(new FileReader(csvFile))) {
            
            br.readLine();

            while ((line = br.readLine()) != null) {

                // use comma as separator
                String[] splitLine = line.split(cvsSplitBy);

                Price newPrice = new Price(Float.valueOf(splitLine[2]), Float.valueOf(splitLine[2]), Long.valueOf(splitLine[1]));
                priceList.add(newPrice);

                //System.out.println("Timestamp: " + newPrice.getTime() + ", Kurs: " + newPrice.getMid());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
        return priceList.toArray(new Price[0]);
    }
}