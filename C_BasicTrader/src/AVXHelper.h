#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>

class AVXHelper
{
  private:
    AVXHelper();

  public:
    static inline __m256d setValues(__m256d input, double value, __m256d mask);
    static inline __m256d applyMask(__m256d value, __m256d mask);
    static inline __m256d invert(__m256d value);
};

inline __m256d AVXHelper::applyMask(__m256d value, __m256d mask)
{
    return _mm256_and_pd(value, mask);
}

inline __m256d AVXHelper::setValues(__m256d input, double value, __m256d mask)
{
    __m256d result;

    __m256d maskInverted = invert(mask);

    __m256d removedPositionsWithNewValue = _mm256_and_pd(maskInverted, input);

    double newValueArray[4] = {value, value, value, value};
    __m256d newValueM256 = _mm256_maskload_pd(newValueArray, mask);

    result = _m256_add_pd(removedPositionsWithNewValue, newValueM256);

    return result;
}

inline __m256d AVXHelper::invert(__m256d value)
{
    return _mm256_and_pd(value, _mm256_setzero_pd());
}

#endif
