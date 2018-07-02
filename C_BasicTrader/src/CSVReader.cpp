#include "CSVReader.h"

#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"

#include <vector>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "PriceFeedData.h"

using namespace std;

int CSVReader::readExchangeFromFile(PriceFeedData &prices, string filepath)
{
    string csvFile = filepath;
    string line;
    string thisVal;
    char cvsSplitBy = ',';
    ifstream inputFile;
    functions::openInputFile(inputFile, config::configValues["exchangeInputDir"], filepath);

    std::getline(inputFile, line, '\n');

    while (std::getline(inputFile, line, '\n'))
    {
        stringstream lineStream(line);
        vector<string> splitLine;
        while (getline(lineStream, thisVal, cvsSplitBy))
        {
            splitLine.emplace_back(thisVal);
        }
        prices.addPrice(atof(splitLine[1].c_str()) + 0.0001, atof(splitLine[1].c_str()) - 0.0001, atof(splitLine[0].c_str()) * 1000);
    }
    inputFile.close();
    return true;
}