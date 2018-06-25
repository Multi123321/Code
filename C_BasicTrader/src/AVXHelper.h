#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>
#include <math.h>

class AVXHelper
{
  private:
    AVXHelper(){};
    static constexpr __m256i zeroMask = {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000};
    static constexpr __m256i oneMask = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};

  public:
    static constexpr __m256d avxOne = {1.0, 1.0, 1.0, 1.0};
    static constexpr __m256d avxNegOne = {-1.0, -1.0, -1.0, -1.0};
    static constexpr __m256d avxZero = {0.0, 0.0, 0.0, 0.0};
    static inline __m256d setValues(double input, double value, __m256d maskArg);
    static inline __m256d setValues(__m256d input, double value, __m256d maskArg);
    static inline __m256i setValues(__m256i input, long value, __m256d maskArg);
    static inline __m256d setValues(__m256d original, __m256d newValues, __m256d maskArg);
    static inline __m256i setValues(__m256i original, __m256i newValues, __m256d maskArg);
    static inline __m256d addMasked(__m256d summand1, double summand2, __m256d maskArg);
    static inline __m256d addMasked(__m256d summand1, __m256d summand2, __m256d maskArg);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third, __m256d forth);
    static inline __m256d applyMask(__m256d value, __m256d maskArg);
    static inline __m256d invert(__m256d value);
    static inline __m256d avxLogDouble(__m256d input);
    static inline __m256d avxExpDouble(__m256d input);
    static inline __m256d avxPowDouble(__m256d input, double power);
    static inline bool isMaskZero(__m256d value);
    static inline double verticalSum(__m256d value);
};

inline __m256d AVXHelper::addMasked(__m256d summand1, double summand2, __m256d maskArg)
{
    return addMasked(summand1, _mm256_set1_pd(summand2), maskArg);
}

inline __m256d AVXHelper::addMasked(__m256d summand1, __m256d summand2, __m256d maskArg)
{
    return _mm256_add_pd(summand1, applyMask(summand2, maskArg));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, third));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third, __m256d forth)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, _mm256_mul_pd(third, forth)));
}

inline __m256d AVXHelper::applyMask(__m256d value, __m256d maskArg)
{
    return _mm256_and_pd(value, maskArg);
}

inline __m256d AVXHelper::setValues(double input, double value, __m256d maskArg)
{
    return setValues(_mm256_set1_pd(input), _mm256_set1_pd(value), maskArg);
}

inline __m256d AVXHelper::setValues(__m256d input, double value, __m256d maskArg)
{
    return setValues(input, _mm256_set1_pd(value), maskArg);
}

inline __m256i AVXHelper::setValues(__m256i input, long value, __m256d maskArg)
{
    return setValues(input, _mm256_set1_epi64x(value), maskArg);
}

inline __m256d AVXHelper::setValues(__m256d original, __m256d newValues, __m256d maskArg)
{
    return _mm256_blendv_pd(original, newValues, maskArg);
}

inline __m256i AVXHelper::setValues(__m256i original, __m256i newValues, __m256d maskArg)
{
    return _mm256_castpd_si256(_mm256_blendv_pd(_mm256_castsi256_pd(original), _mm256_castsi256_pd(newValues), maskArg));
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

inline __m256d AVXHelper::avxPowDouble(__m256d input, double power)
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
    }
    return output;
}

inline double AVXHelper::verticalSum(__m256d value)
{
    __m256d tmp = _mm256_hadd_pd(value, value);
    return ((double *)&tmp)[0] + ((double *)&tmp)[2];
}

inline __m256d AVXHelper::invert(__m256d value)
{
    return _mm256_xor_pd(value, _mm256_castsi256_pd(oneMask));
}

inline bool AVXHelper::isMaskZero(__m256d value)
{
    return _mm256_testc_pd(_mm256_cmp_pd(value, _mm256_castsi256_pd(zeroMask), _CMP_EQ_OS), _mm256_castsi256_pd(oneMask));
}

#endif
