#ifndef RUNNER_H
#define RUNNER_H

#include "PriceFeedData.h"
#include <string>
#include <x86intrin.h>

class Runner
{
private:
public:
  __m256d prevExtreme;
  __m256i prevExtremeTime;

  __m256d prevDC;
  __m256i prevDCTime;

  __m256d extreme;
  __m256i extremeTime;

  __m256d deltaUp;
  __m256d deltaDown;
  __m256d deltaStarUp, deltaStarDown;
  __m256d osL;
  __m256d type;
  bool initalized;
  __m256d reference;

  std::string fileName;

  Runner();
  Runner(__m256d threshUp, __m256d threshDown, PriceFeedData::Price price, std::string file, __m256d dStarUp, __m256d dStarDown);
  Runner(__m256d threshUp, __m256d threshDown, double price, std::string file, __m256d dStarUp, __m256d dStarDown);
  Runner(__m256d threshUp, __m256d threshDown, std::string file, __m256d dStarUp, __m256d dStarDown);

  __m256d run(PriceFeedData::Price price);
  __m256d run(double price);
};

#endif
