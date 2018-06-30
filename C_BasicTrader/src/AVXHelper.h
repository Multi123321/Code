#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <x86intrin.h>
#include <math.h>
#include <iostream>
#include <string.h>

typedef __mmask8 mask;

class AVXHelper
{
  private:
    AVXHelper(){};
    static constexpr mask zeroMask = 0x00;
    static constexpr mask oneMask = 0xFF;

  public:
    static constexpr __m256d avxOne = {1.0, 1.0, 1.0, 1.0};
    static constexpr __m256d avxNegOne = {-1.0, -1.0, -1.0, -1.0};
    static constexpr __m256d avxZero = {0.0, 0.0, 0.0, 0.0};
    /*
    static inline __mmask8 cmpDouble(__m256d vec1, __m256d vec2, const int imm8);
    static inline __mmask8 cmpDoubleMasked(__m256d vec1, __m256d vec2, const int imm8, __mmask8 maskArg);
    */
    static inline __m256d setValues(double input, double value, mask maskArg);
    static inline __m256d setValues(__m256d input, double value, mask maskArg);
    static inline __m256i setValues(__m256i input, long value, mask maskArg);
    static inline __m256d setValues(__m256d original, __m256d newValues, mask maskArg);
    static inline __m256i setValues(__m256i original, __m256i newValues, mask maskArg);
    static inline __m256d addMasked(__m256d summand1, double summand2, mask maskArg);
    static inline __m256d addMasked(__m256d summand1, __m256d summand2, mask maskArg);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third);
    static inline __m256d multiply(__m256d first, __m256d second, __m256d third, __m256d forth);
    static inline mask multMasks(mask mask1, mask mask2);
    static inline mask addMasks(mask mask1, mask mask2);
    static inline mask invert(mask value);
    static inline __m256d avxLogDouble(__m256d input);
    static inline __m256d avxExpDouble(__m256d input);
    static inline __m256d avxPowDouble(__m256d input, double power);
    static inline bool isMaskZero(mask value);
    static inline double verticalSum(__m256d value);
    static inline void printMask(mask mask);
    static inline void printMaskOld(__m256d maskArg);
};
/*
inline __mmask8 AVXHelper::cmpDouble(__m256d vec1, __m256d vec2, const int imm8)
{
    return _mm256_cmp_pd_mask(vec1, vec2, imm8);
}

inline __mmask8 AVXHelper::cmpDoubleMasked(__m256d vec1, __m256d vec2, const int imm8, __mmask8 maskArg)
{
    return _mm256_mask_cmp_pd_mask(maskArg, vec1, vec2, imm8);
}
*/
inline __m256d AVXHelper::addMasked(__m256d summand1, double summand2, mask maskArg)
{
    return addMasked(summand1, _mm256_set1_pd(summand2), maskArg);
}

inline __m256d AVXHelper::addMasked(__m256d summand1, __m256d summand2, mask maskArg)
{
    return _mm256_mask_add_pd(summand1, maskArg, summand1, summand2);
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, third));
}

inline __m256d AVXHelper::multiply(__m256d first, __m256d second, __m256d third, __m256d forth)
{
    return _mm256_mul_pd(first, _mm256_mul_pd(second, _mm256_mul_pd(third, forth)));
}

inline mask AVXHelper::multMasks(mask mask1, mask mask2)
{
    return (mask1 & mask2);
}

inline mask AVXHelper::addMasks(mask mask1, mask mask2)
{
    return (mask1 | mask2);
}

inline __m256d AVXHelper::setValues(double input, double value, mask maskArg)
{
    return setValues(_mm256_set1_pd(input), _mm256_set1_pd(value), maskArg);
}

inline __m256d AVXHelper::setValues(__m256d input, double value, mask maskArg)
{
    return setValues(input, _mm256_set1_pd(value), maskArg);
}

inline __m256i AVXHelper::setValues(__m256i input, long value, mask maskArg)
{
    return setValues(input, _mm256_set1_epi64x(value), maskArg);
}

inline __m256d AVXHelper::setValues(__m256d original, __m256d newValues, mask maskArg)
{
    return _mm256_mask_blend_pd(maskArg, original, newValues);
}

inline __m256i AVXHelper::setValues(__m256i original, __m256i newValues, mask maskArg)
{
    return _mm256_castpd_si256(_mm256_mask_blend_pd(maskArg, _mm256_castsi256_pd(original), _mm256_castsi256_pd(newValues)));
}

inline __m256d AVXHelper::avxLogDouble(__m256d vector)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        ((double *)&output)[i] = log(((double *)&vector)[i]);
    }
    return output;
}

inline __m256d AVXHelper::avxExpDouble(__m256d vector)
{
    __m256d output;
    for (int i = 0; i < 4; i++)
    {
        ((double *)&output)[i] = exp(((double *)&vector)[i]);
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

inline mask AVXHelper::invert(mask value)
{
    //return _mm256_xor_pd(value, _mm256_castsi256_pd(oneMask));
    return ~value;
}

inline bool AVXHelper::isMaskZero(mask value)
{
    //bool result = _mm256_testc_pd(cmpDouble(value, _mm256_castsi256_pd(zeroMask), _CMP_EQ_OS), _mm256_castsi256_pd(oneMask));

    //return result;
    return (value == zeroMask);
}

inline void AVXHelper::printMask(mask maskArg)
{
    //uint64_t u0, u1, u2, u3;
    //memcpy(&u0, &(((double *)&maskArg)[0]), sizeof(((double *)&maskArg)[0]));
    //memcpy(&u1, &(((double *)&maskArg)[1]), sizeof(((double *)&maskArg)[1]));
    //memcpy(&u2, &(((double *)&maskArg)[2]), sizeof(((double *)&maskArg)[2]));
    //memcpy(&u3, &(((double *)&maskArg)[3]), sizeof(((double *)&maskArg)[3]));
    //std::cout << std::hex << u0 << " " << u1 << " " << u2 << " " << u3;
    std::cout << std::hex << (int)(unsigned char)maskArg << std::endl;
}

inline void AVXHelper::printMaskOld(__m256d maskArg)
{
    uint64_t u0, u1, u2, u3;
    memcpy(&u0, &(((double *)&maskArg)[0]), sizeof(((double *)&maskArg)[0]));
    memcpy(&u1, &(((double *)&maskArg)[1]), sizeof(((double *)&maskArg)[1]));
    memcpy(&u2, &(((double *)&maskArg)[2]), sizeof(((double *)&maskArg)[2]));
    memcpy(&u3, &(((double *)&maskArg)[3]), sizeof(((double *)&maskArg)[3]));
    std::cout << std::hex << u0 << " " << u1 << " " << u2 << " " << u3 << std::endl;
}

#endif
