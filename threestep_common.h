#ifndef THREESTEP_COMMON_H
#define THREESTEP_COMMON_H 1

double median(double *x, int length);
double median_nocopy(double *x, int length);
double median_nocopy_hasNA(double *x, int length,int num_na);
double quartiles(double *x, int length, double *LQ, double *UQ);




/* Everything below is for compilation on Windows using VC++ */

#ifndef HAVE_STRCASECMP
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define HAVE_STRCASECMP
#endif
#endif

#ifndef NAN
#ifdef _MSC_VER
static const unsigned int nan[2] = {0xffffffff, 0x7fffffff};
#define NAN (*(const double *) nan)
#endif
#endif

#ifdef _MSC_VER
#include <float.h>
#define finite _finite
#define isnan _isnan
#endif

#ifndef FUNCTIONS_FOR_VC
#ifdef _MSC_VER
#define FUNCTIONS_FOR_VC

/*static inline double round( double d)
{
  return floor( d + 0.5 );
}*/


/*static inline double trunc(double d)
{
  return((d < 0? floor(-d): floor(d)));
} */

/*static double log1p(double x)
{
    double u;

    u = 1.0 + x;
    if (u == 1.0)
      return (x);
    else
      return (log(u) * (x / (u - 1.0)));
}*/
#endif
#endif

#ifndef INFINITY
#ifdef _MSC_VER
#include <limits>
static const double inf = std::numeric_limits<double>::infinity();
#define INFINITY inf
#endif
#endif


#endif
