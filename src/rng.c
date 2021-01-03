/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file rng.c
 *
 * @brief Handles all the random number logic.
 *
 * Random numbers are currently generated using the mersenne twister.
 */


/** @cond */
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "SDL.h"

#include "naev.h"

#if HAS_POSIX
#include <fcntl.h>
#include <sys/time.h>
#endif /* HAS_POSIX */
#if HAS_WIN32
#include <sys/timeb.h>
#include <sys/types.h>
#endif /* HAS_WIN32 */
/** @endcond */

#include "rng.h"

#include "log.h"


/*
 * mersenne twister state
 */
static uint32_t MT[624]; /**< Mersenne twister state. */
static uint32_t mt_y; /**< Internal mersenne twister variable. */
static int mt_pos = 0; /**< Current number being used. */


/*
 * prototypes
 */
static uint32_t rng_timeEntropy (void);
/* mersenne twister */
static void mt_initArray( uint32_t seed );
static void mt_genArray (void);
static uint32_t mt_getInt (void);


/**
 * @fn void rng_init (void)
 *
 * @brief Initializes the random subsystem.
 */
void rng_init (void)
{
   uint32_t i;
   int need_init;

   need_init = 1; /* initialize by default */
#if HAS_LINUX
   int fd;
   fd = open("/dev/urandom", O_RDONLY); /* /dev/urandom is better than time seed */
   if (fd != -1) {
      i = sizeof(uint32_t)*624;
      if (read( fd, &MT, i ) == (ssize_t)i)
         need_init = 0;
      else
         i = rng_timeEntropy();
      close(fd);
   }
   else
      i = rng_timeEntropy();
#else /* HAS_LINUX */
   i = rng_timeEntropy();
#endif /* HAS_LINUX */

   if (need_init)
      mt_initArray( i );
   for (i=0; i<10; i++) /* generate numbers to get away from poor initial values */
      mt_genArray();
}


/**
 * @fn static uint32_t rng_timeEntropy (void)
 *
 * @brief Uses time as a source of entropy.
 *
 *    @return A 4 byte entropy seed.
 */
static uint32_t rng_timeEntropy (void)
{
   int i;
#if HAS_POSIX
   struct timeval tv;
   gettimeofday( &tv, NULL );
   i = tv.tv_sec * 1000000 + tv.tv_usec;
#elif HAS_WIN32
   struct _timeb tb;
   _ftime( &tb );
   i = tb.time * 1000 + tb.millitm;
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
   return i;
}


/**
 * @fn static void mt_initArray( uint32_t seed )
 *
 * @brief Generates the initial mersenne twister based on seed.
 */
static void mt_initArray( uint32_t seed )
{
   int i;

   MT[0] = seed;
   for (i=1; i<624; i++)
      MT[i] = 1812433253 * (MT[i-1] ^ (((MT[i-1])) + i) >> 30);
   mt_pos = 0;
}


/**
 * @fn static void mt_genArray (void)
 *
 * @brief Generates a new set of random numbers for the mersenne twister.
 */
static void mt_genArray (void)
{
   int i;

   for (i=0; i<624; i++ ) {
      mt_y = (MT[i] & 0x80000000) + ((MT[i] % 624) & 0x7FFFFFFF);
      if (mt_y % 2) /* odd */
         MT[i] = (MT[(i+397) % 624] ^ (mt_y >> 1)) ^ 2567483615U;
      else /* even */
         MT[i] = MT[(i+397) % 624] ^ (mt_y >> 1);
   }
   mt_pos = 0;
}


/**
 * @fn static uint32_t mt_getInt (void)
 *
 * @brief Gets the next int.
 *
 *    @return A random 4 byte number.
 */
static uint32_t mt_getInt (void)
{
   if (mt_pos >= 624) mt_genArray();

   mt_y = MT[mt_pos++];
   mt_y ^= mt_y >> 11;
   mt_y ^= (mt_y << 7) & 2636928640U;
   mt_y ^= (mt_y << 15) & 4022730752U;
   mt_y ^= mt_y >> 18;

   return mt_y;
}


/**
 * @fn unsigned int randint (void)
 *
 * @brief Gets a random integer.
 *
 *    @return A random integer.
 */
unsigned int randint (void)
{
   return mt_getInt();
}


/**
 * @fn double randfp (void)
 *
 * @brief Gets a random float between 0 and 1 (inclusive).
 *
 *    @return A random float between 0 and 1 (inclusive).
 */
static double m_div = (double)(0xFFFFFFFF); /**< Number to divide by. */
double randfp (void)
{
   double m = (double)mt_getInt();
   return m / m_div;
}


/**
 * @fn double Normal( double x )
 *
 * @brief Calculates the Normal distribution.
 *
 * Calculates N(x) where N is the normal distribution.
 *
 * Approximates to a power series:
 *
 *  N(x) =  1 - n(x)*(b1*t + b2*t^2 + b3*t^3 + b4*t^4 + b5*t^5) + Err
 *  where t = 1 / (1 + 0.2316419*x)
 *
 * Maximum absolute error is 7.5e^-8.
 *
 *    @param x Value to calculate the normal of.
 *    @return The value of the Normal.
 */
double Normal( double x )
{
   double t;
   double series;
   const double b1 =  0.319381530;
   const double b2 = -0.356563782;
   const double b3 =  1.781477937;
   const double b4 = -1.821255978;
   const double b5 =  1.330274429;
   const double p  =  0.2316419;
   const double c  =  0.39894228;

   t = 1. / ( 1. + p * FABS(x) );
   series = (1. - c * exp( -x * x / 2. ) * t *
         ( t *( t * ( t * ( t * b5 + b4 ) + b3 ) + b2 ) + b1 ));
   return (x > 0.) ? 1. - series : series;
}


/**
 * @fn double NormalInverse( double p )
 *
 * @brief Calculates the inverse of the normal.
 *
 * Lower tail quantile for standard normal distribution function.
 *
 * This function returns an approximation of the inverse cumulative
 * standard normal distribution function.  I.e., given P, it returns
 * an approximation to the X satisfying P = Pr{Z <= X} where Z is a
 * random variable from the standard normal distribution.
 *
 * The algorithm uses a minimax approximation by rational functions
 * and the result has a relative error whose absolute value is less
 * than 1.15e-9.
 *
 * Author:      Peter J. Acklam
 * Time-stamp:  2002-06-09 18:45:44 +0200
 * E-mail:      jacklam@math.uio.no
 * WWW URL:     http://www.math.uio.no/~jacklam
 *
 * C implementation adapted from Peter's Perl version.
 *
 * Original algorithm from http://home.online.no/~pjacklam/notes/invnorm/ .
 */
/* Coefficients in rational approximations. */
static const double a[] =
{
   -3.969683028665376e+01,
    2.209460984245205e+02,
   -2.759285104469687e+02,
    1.383577518672690e+02,
   -3.066479806614716e+01,
    2.506628277459239e+00
}; /**< Inverse normal coefficients. */
static const double b[] =
{
   -5.447609879822406e+01,
    1.615858368580409e+02,
   -1.556989798598866e+02,
    6.680131188771972e+01,
   -1.328068155288572e+01
}; /**< Inverse normal coefficients. */
static const double c[] =
{
   -7.784894002430293e-03,
   -3.223964580411365e-01,
   -2.400758277161838e+00,
   -2.549732539343734e+00,
    4.374664141464968e+00,
    2.938163982698783e+00
}; /**< Inverse normal coefficients. */
static const double d[] =
{
    7.784695709041462e-03,
    3.224671290700398e-01,
    2.445134137142996e+00,
    3.754408661907416e+00
}; /**< Inverse normal coefficients. */
#define LOW 0.02425 /**< Low area threshold. */
#define HIGH 0.97575 /**< High area threshold. */
double NormalInverse( double p )
{
   double x, e, u, q, r;

   /* Check for errors */
   errno = 0;
   if ((p < 0) || (p > 1)) {
      errno = EDOM;
      return 0.;
   }
   else if (p == 0.) {
      errno = ERANGE;
      return -HUGE_VAL /* minus "infinity" */;
   }
   else if (p == 1.) {
      errno = ERANGE;
      return HUGE_VAL /* "infinity" */;
   }
   /* Use different approximations for different parts */
   else if (p < LOW) {
      /* Rational approximation for lower region */
      q = sqrt(-2*log(p));
      x = (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
           ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
   }
   else if (p > HIGH) {
      /* Rational approximation for upper region */
      q  = sqrt(-2*log(1-p));
      x = -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
   }
   else {
      /* Rational approximation for central region */
      q = p - 0.5;
      r = q*q;
      x = (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
          (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
   }

   /* Full machine precision */
   e = 0.5 * erfc(-x / M_SQRT2) - p;
   u = e * 2.5066282746310002 /* sqrt(2*pi) */ * exp((x*x)/2);
   x = x - u/(1 + x*u/2);

   return x;
}



