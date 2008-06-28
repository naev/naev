/*
 * See Licensing and Copmt_yright notice in naev.h
 */



#include "rng.h"

#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#ifdef LINUX
#include <sys/time.h>
#include <fcntl.h>
#endif
#include "SDL.h"

#include "naev.h"
#include "log.h"



/*
 * mersenne twister state
 */
static uint32_t MT[624];
static uint32_t mt_y;
static int mt_pos = 0; /* current number */


/*
 * prototypes
 */
static uint32_t rng_timeEntropy (void);
/* mersenne twister */
static void mt_initArray( uint32_t seed );
static void mt_genArray (void);
static uint32_t mt_getInt (void);


void rng_init (void)
{
   uint32_t i;
   int need_init;

   need_init = 1; /* initialize by default */
#ifdef LINUX
   int fd;
   fd = open("/dev/urandom", O_RDONLY); /* /dev/urandom is better then time seed */
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
#else
   i = rng_timeEntropy();
#endif

   if (need_init)
      mt_initArray( i );
   for (i=0; i<10; i++) /* generate numbers to get away from poor initial values */
      mt_genArray();
}


/*
 * uses the time as a source of entropy
 */
static uint32_t rng_timeEntropy (void)
{
   int i;
#ifdef WIN32
   struct _timeb tb;
   _ftime( &tb );
   i = tb.time * 1000 + tb.millitm;
#else
   struct timeval tv;
   gettimeofday( &tv, NULL );
   i = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
   return i;
}


/*
 * generates the initial mersenne twister based on seed
 */
static void mt_initArray( uint32_t seed )
{
   int i;

   MT[0] = seed;
   for (i=1; i<624; i++)
      MT[i] = 1812433253 * (MT[i-1] ^ (((MT[i-1])) + i) >> 30);
   mt_pos = 0;
}


/*
 * generates an array of numbers
 */
static void mt_genArray (void)
{
   int i;

   for (i=0; i<624; i++ ) {
      mt_y = (MT[i] & 0x80000000) + ((MT[i] % 624) & 0x7FFFFFFF);
      if (mt_y % 2) /* odd */
         MT[i] = (MT[(i+397) % 624] ^ (mt_y >> 1)) ^ 2567483615;
      else /* even */
         MT[i] = MT[(i+397) % 624] ^ (mt_y >> 1);
   }
   mt_pos = 0;
}


/*
 * gets the next int
 */
static uint32_t mt_getInt (void)
{
   if (mt_pos >= 624) mt_genArray();

   mt_y = MT[mt_pos++];
   mt_y ^= mt_y >> 11;
   mt_y ^= (mt_y << 7) & 2636928640;
   mt_y ^= (mt_y << 15) & 4022730752;
   mt_y ^= mt_y >> 18;

   return mt_y;
}


/*
 * returns a random int
 */
unsigned int randint (void)
{
   return mt_getInt();
}


/*
 * returns a random double ( 0 <= randfp <= 1. )
 */
static double m_div = (double)(0xFFFFFFFF);
double randfp (void)
{
   double m = (double)mt_getInt();
   return m / m_div;
}


/*
 * Calculates N(x) where N is the normal distribution.
 *
 * Approximates to a power series:
 *
 *  N(x) =  1 - n(x)*(b1*t + b2*t^2 + b3*t^3 + b4*t^4 + b5*t^5) + Err
 *  where t = 1 / (1 + 0.2316419*x)
 *  
 * Maximum absolute error is 7.5e^-8.
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


/*
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
};
static const double b[] =
{
   -5.447609879822406e+01,
   1.615858368580409e+02,
   -1.556989798598866e+02,
   6.680131188771972e+01,
   -1.328068155288572e+01
};
static const double c[] =
{
   -7.784894002430293e-03,
   -3.223964580411365e-01,
   -2.400758277161838e+00,
   -2.549732539343734e+00,
   4.374664141464968e+00,
   2.938163982698783e+00
};
static const double d[] =
{
   7.784695709041462e-03,
   3.224671290700398e-01,
   2.445134137142996e+00,
   3.754408661907416e+00
};
#define LOW 0.02425
#define HIGH 0.97575
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
   /* Use different aproximations for different parts */
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
   x =  x - u/(1 + x*u/2);

   return x;
}



