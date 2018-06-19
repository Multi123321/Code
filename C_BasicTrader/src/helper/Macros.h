#ifndef MACROS_H
#define MACROS_H

#define UNUSED(x) (void)(x)
#define OUT(x) x

#define ITERATE_ALL_POINTS(nX, nY, border, currentPoint) for(currentPoint.y = border; currentPoint.y < nY - border; currentPoint.y++)\
                                                          for(currentPoint.x = border; currentPoint.x < nX - border; currentPoint.x++)

#define SQUARE(x) x * x

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE() cout << __PRETTY_FUNCTION__ << " Line: " << __LINE__ << " in file: " << __FILENAME__ << std::endl;

#endif