#ifndef CROSS_CMATH_H
#define CROSS_CMATH_H

#if (defined(_WIN32) || defined(__WIN32__)) && defined(_MSC_VER)

#include <math.h>
#include <float.h>
#define isnan _isnan

#else

#include <math.h>

#endif

#endif // !CROSS_CMATH_H
