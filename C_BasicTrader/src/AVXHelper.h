#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>

class AVXHelper
{
private:
  AVXHelper();

public:
  static inline void AVXHelper::avxLogDouble(__m256d &input)
  {
    for (int i = 0; i < 4; i++)
    {
      (double *)&input[i] = log((double *)&input[i]);
    }
  }
  // TODO
};

#endif
