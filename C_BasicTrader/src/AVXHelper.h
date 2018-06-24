#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>
#include <avxintrin.h>
#include <immintrin.h>
#include <math.h>

namespace AVXHelper
{
extern __m256d avx256dOne = _mm256_set1_pd(1.0);
extern __m256d avx256dNegOne = _mm256_set1_pd(-1.0);
extern __m256d avx256dZero = _mm256_setzero_pd();

static inline __m256d setValues(__m256d input, double value, __m256d mask);
static inline __m256i setValues(__m256i input, long value, __m256d mask);
static inline __m256d setValues(__m256d original, __m256d newValues, __m256d mask);
static inline __m256i setValues(__m256i original, __m256i newValues, __m256d mask);
static inline __m256d addMasked(__m256d summand1, double summand2, __m256d mask);
static inline __m256d addMasked(__m256d summand1, __m256d summand2, __m256d mask);
static inline __m256d multiply(__m256d first, __m256d second, __m256d third);
static inline __m256d multiply(__m256d first, __m256d second, __m256d third, __m256d forth);
static inline __m256d applyMask(__m256d value, __m256d mask);
static inline __m256d invert(__m256d value);
static inline __m256d avxLogDouble(__m256d input);
static inline __m256d avxExpDouble(__m256d input);
static inline __m256d avxPowDouble(__m256d input, double power);
static inline double verticalSum(__m256d value);
}; // namespace AVXHelper

inline __m256d AVXHelper::addMasked(__m256d summand1, double summand2, __m256d mask)
{
    return addMasked(summand1, _mm256_set1_pd(summand2), mask);
}

inline __m256d AVXHelper::addMasked(__m256d summand1, __m256d summand2, __m256d mask)
{
    return _mm256_add_pd(summand1, applyMask(summand2, mask));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, third));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third, __m256d forth)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, _mm256_mul_pd(third, forth)));
}

inline __m256d AVXHelper::applyMask(__m256d value, __m256d mask)
{
    return _mm256_and_pd(value, mask);
}

inline __m256d AVXHelper::setValues(__m256d input, double value, __m256d mask)
{
    return setValues(input, _mm256_set1_pd(value), mask);
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

inline __m256d AVXHelper::avxLogDouble(__m256d input)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        ((double *)&output)[i] = log(((double *)&input)[i]);
    }
    return output;
}

inline __m256d AVXHelper::avxExpDouble(__m256d input)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        ((double *)&output)[i] = exp(((double *)&input)[i]);
    }
    return output;
}

inline __m256d avxPowDouble(__m256d input, double power)
{
    __m256d output;
    if (power == 2.0)
    {
        output = _mm256_mul_pd(input, input);
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            ((double *)&output)[i] = pow(((double *)&input)[i], power);
        }
        return output;
    }
}

inline double AVXHelper::verticalSum(__m256d value)
{
    __m256d tmp = _mm256_hadd_pd(value, value);
    return ((double *)&tmp)[0] + ((double *)&tmp)[2];
}

inline __m256d AVXHelper::invert(__m256d value)
{
    return _mm256_and_pd(value, _mm256_setzero_pd());
}

#endif
