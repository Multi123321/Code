#ifndef CSV_READER_H
#define CSV_READER_H

#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"

#include <vector>
#include <string>
#include <deque>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "PriceFeedData.h"

class CSVReader
{
public:
  static int readExchangeFromFile(PriceFeedData &prices, std::string filepath);
};

#endif
