// #include "CSVReader.h"

// #include "helper/UsefullFunctions.h"
// #include "helper/ConfigManager.h"

// #include <vector>
// #include <string>
// #include <deque>
// #include <stdlib.h>
// #include <iostream>
// #include "PriceFeedData.h"

// using namespace std;
// using namespace config;

// static PriceFeedData* CSVReader::readExchangeFromFile(string filepath) 
// {
//     string csvFile = filepath;
//     string line;
//     string thisVal;
//     string cvsSplitBy = ";";
//     PriceFeedData* prices = new PriceFeedData();
//     ifstream inputFile = functions::openInputFile(configValues["exchangeInputDir"],  filepath);

//     std::getline(inputFile, line);

//     while (std::getline(inputFile, line, '\n'))
//     {
//         stringstream lineStream(line);
//         vector<string> splitLine;
//         while(getline(lineStream, thisVal,',')) 
//         {
//             splitLine.emplace_back(thisVal);
//         }
//         prices->addPrice(atof(splitLine[2].c_str())+0.0001, atof(splitLine[2].c_str())-0.0001, atof(splitLine[1].c_str())*1000); 
//     }
//     inputFile.close();
//     return prices;
// }