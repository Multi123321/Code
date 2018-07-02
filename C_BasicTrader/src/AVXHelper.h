#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>
#include <math.h>
#include <iostream>
#include <string.h>

class AVXHelper
{
  private:
    AVXHelper(){};
    static constexpr __m256i zeroMask = {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000};
    static constexpr __m256i oneMask = {(long long int)0xFFFFFFFFFFFFFFFF, (long long int)0xFFFFFFFFFFFFFFFF, (long long int)0xFFFFFFFFFFFFFFFF, (long long int)0xFFFFFFFFFFFFFFFF};

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
    static inline __m256d subMasked(__m256d minuend, double subtrahend, __m256d maskArg);
    static inline __m256d subMasked(__m256d minuend, __m256d subtrahend, __m256d maskArg);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third, __m256d forth);
    static inline __m256d multMasks(__m256d mask1, __m256d mask2);
    static inline __m256d invert(__m256d value);
    static inline __m256d avxLogDouble(__m256d input);
    static inline __m256d avxLogDoubleMasked(__m256d input, __m256d maskArg);
    static inline __m256d avxExpDouble(__m256d input);
    static inline __m256d avxExpDoubleMasked(__m256d input, __m256d maskArg);
    static inline __m256d avxPowDouble(__m256d input, double power);
    static inline __m256d avxPowDoubleMasked(__m256d input, double power, __m256d maskArg);
    static inline bool isMaskZero(__m256d value);
    static inline double verticalSum(__m256d value);
    static inline void printMask(__m256d mask);
};

inline __m256d AVXHelper::addMasked(__m256d summand1, double summand2, __m256d maskArg)
{
    return addMasked(summand1, _mm256_set1_pd(summand2), maskArg);
}

inline __m256d AVXHelper::addMasked(__m256d summand1, __m256d summand2, __m256d maskArg)
{
    return _mm256_add_pd(summand1, setValues(avxZero, summand2, maskArg));
}

inline __m256d AVXHelper::subMasked(__m256d minuend, double subtrahend, __m256d maskArg)
{
    return subMasked(minuend, _mm256_set1_pd(subtrahend), maskArg);
}

inline __m256d AVXHelper::subMasked(__m256d minuend, __m256d subtrahend, __m256d maskArg)
{
    return _mm256_sub_pd(minuend, setValues(avxZero, subtrahend, maskArg));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, third));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third, __m256d forth)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, _mm256_mul_pd(third, forth)));
}

inline __m256d AVXHelper::multMasks(__m256d mask1, __m256d mask2)
{
    return _mm256_and_pd(mask1, mask2);
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

inline __m256d AVXHelper::avxLogDoubleMasked(__m256d input, __m256d maskArg)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        if (((double *)&maskArg)[i] != 0.0)
        {
            ((double *)&output)[i] = log(((double *)&input)[i]);
        }
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

inline __m256d AVXHelper::avxExpDoubleMasked(__m256d input, __m256d maskArg)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        if (((double *)&maskArg)[i] != 0.0)
        {
            ((double *)&output)[i] = exp(((double *)&input)[i]);
        }
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

inline __m256d AVXHelper::avxPowDoubleMasked(__m256d input, double power, __m256d maskArg)
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
            if (((double *)&maskArg)[i] != 0.0)
            {
                ((double *)&output)[i] = pow(((double *)&input)[i], power);
            }
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
    bool result = _mm256_testc_pd(_mm256_cmp_pd(value, _mm256_castsi256_pd(zeroMask), _CMP_EQ_OS), _mm256_castsi256_pd(oneMask));

    // printMask(value);
    // if (result == true)
    // {
    //     std::cout << " Mask seems to be zero!" << std::endl;
    // }
    // else
    // {
    //     std::cout << " This is not zero..." << std::endl;
    // }

    return result; // _mm256_testc_pd(_mm256_cmp_pd(value, _mm256_castsi256_pd(zeroMask), _CMP_EQ_OS), _mm256_castsi256_pd(oneMask));
}

inline void AVXHelper::printMask(__m256d maskArg)
{
    uint64_t u0, u1, u2, u3;
    memcpy(&u0, &(((double *)&maskArg)[0]), sizeof(((double *)&maskArg)[0]));
    memcpy(&u1, &(((double *)&maskArg)[1]), sizeof(((double *)&maskArg)[1]));
    memcpy(&u2, &(((double *)&maskArg)[2]), sizeof(((double *)&maskArg)[2]));
    memcpy(&u3, &(((double *)&maskArg)[3]), sizeof(((double *)&maskArg)[3]));
    std::cout << std::hex << u0 << " " << u1 << " " << u2 << " " << u3;
}

#endif
