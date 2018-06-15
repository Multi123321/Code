#include <string>
#include <deque>
#include <iostream>
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"
#include "PriceFeedData.h"

using namespace std;

static PriceFeedData readExchangeFromFile(string file) {
    string csvFile = file;
    string line;
    string thisVal;
    string cvsSplitBy = ";";
    PriceFeedData prices = new PriceFeedData();
    ifstream inputFile = functions::openInputFile(configValues["exchangeInputDir"],  file);

    istream::getline(inputFile, line);

    while (inputFile.getline(line, 1000, '\n') {
        vector<string> splitLine;
        while(line.getline(thisVal, 1000,',')) {
            values.emplace_back(thisVal);
        }
        prices.addPrice(splitLine[2]+0.0001, splitLine[2]-0.0001, splitLine[1]*1000); 
    }
    inputFile.close()
    return prices;
}