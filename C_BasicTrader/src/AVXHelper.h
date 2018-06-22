#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>

class AVXHelper
{
  private:
    AVXHelper();

  public:
    static inline __m256d setValues(__m256d input, double value, __m256d mask);
    static inline __m256i setValues(__m256i input, long value, __m256d mask);
    static inline __m256d setValues(__m256d original, __m256d newValues, __m256d mask);
    static inline __m256i setValues(__m256i original, __m256i newValues, __m256d mask);
    static inline __m256d applyMask(__m256d value, __m256d mask);
    static inline __m256d invert(__m256d value);
    static inline void avxLogDouble(__m256d &input);
};

inline __m256d AVXHelper::applyMask(__m256d value, __m256d mask)
{
    return _mm256_and_pd(value, mask);
}

inline __m256d AVXHelper::setValues(__m256d input, double value, __m256d mask)
{
    return setValues(input, _mm256_set1_pd(value), mask);
    // __m256d result;

    // __m256d maskInverted = _mm256_and_pd(mask, _mm256_setzero_pd());

    // __m256d removedPositionsWithNewValue = _mm256_and_pd(maskInverted, input);

    // double newValueArray[4] = {value, value, value, value};
    // __m256d newValueM256 = _mm256_maskload_pd(newValueArray, mask);

    // result = _m256_add_pd(removedPositionsWithNewValue, newValueM256);

    // return result;
}

inline __m256i setValues(__m256i input, long value, __m256d mask)
{
    return setValues(input, _mm256_set1_epi64x(value), mask);
}

inline __m256d AVXHelper::setValues(__m256d original, __m256d newValues, __m256d mask)
{
    return _mm256_blendv_pd(original, newValues, mask);
}

inline __m256i AVXHelper::setValues(__m256i original, __m256i newValues, __m256d mask)
{
    return _mm256_castpd_si256(_mm256_blendv_pd(_mm256_castsi256_pd(original), _mm256_castsi256_pd(newValues), mask));
}

inline void AVXHelper::avxLogDouble(__m256d &input)
{
    for (int i = 0; i < 4; i++)
    {
        (double *)&input[i] = log((double *)&input[i]);
    }
}

inline __m256d AVXHelper::invert(__m256d value)
{
    return _mm256_and_pd(value, _mm256_setzero_pd());
}

#endif
