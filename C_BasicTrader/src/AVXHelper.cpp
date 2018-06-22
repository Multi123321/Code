#include "AVXHelper.h"
#include <x86intrin.h>

AVXHelper::AVXHelper()
{
}

__m256d avcLogDouble for (int i = 0; i < 4; i++)
{
    (double *)tmp[i] = log((double *)tmp[i]);
}