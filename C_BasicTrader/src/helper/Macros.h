#ifndef MACROS_H
#define MACROS_H

#define UNUSED(x) (void)(x)
#define OUT(x) x

#define ITERATE_ALL_POINTS(nX, nY, border, currentPoint)                          \
    for (currentPoint.y = border; currentPoint.y < nY - border; currentPoint.y++) \
        for (currentPoint.x = border; currentPoint.x < nX - border; currentPoint.x++)

#define SQUARE(x) x *x

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE() cout << __PRETTY_FUNCTION__ << " Line: " << __LINE__ << " in file: " << __FILENAME__ << std::endl;

#ifdef NDEBUG
#define IFDEBUG(x) \
    do             \
    {              \
    } while (0)
#else
#define IFDEBUG(x) x;
#endif

#ifdef PRINTRES
#define IFPRINT(x) x;
#else
#define IFPRINT(x) \
    do             \
    {              \
    } while (0)
#endif

#ifdef LIKWID
#define IFLIKWID(x) x
#else
#define IFLIKWID(x) \
    do              \
    {               \
    } while (0)
#endif

#define SERIAL_AVX(x) for (int x = 0; x < 4; x++)
#define AVX_DOUBLE(x, i) ((double *)&x)[i]
#define AVX_TO_STRING(x) AVX_DOUBLE(x, 0) << " " << AVX_DOUBLE(x, 1) << " " << AVX_DOUBLE(x, 2) << " " << AVX_DOUBLE(x, 3)

#endif