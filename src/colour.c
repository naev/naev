/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file colour.c
 *
 * @brief Predefined colours for use with Naev.
 */

/** @cond */
#include <math.h>

#include "naev.h"
/** @endcond */

#include "colour.h"

#include "log.h"
#include "nmath.h"
#include "nstring.h"

/*
 * http://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
 */
__attribute__( ( const ) ) double linearToGamma( double x )
{
   if ( x <= 0.0031308 )
      return x * 12.92;
   return 1.055 * pow( x, 1.0 / 2.4 ) - 0.055;
}
/*
 * http://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
 */
__attribute__( ( const ) ) double gammaToLinear( double x )
{
   if ( x <= 0.04045 )
      return x / 12.92;
   return pow( ( x + 0.055 ) / 1.055, 2.4 );
}

void col_linearToGamma( glColour *c )
{
   c->r = linearToGamma( c->r );
   c->g = linearToGamma( c->g );
   c->b = linearToGamma( c->b );
}

void col_gammaToLinear( glColour *c )
{
   c->r = gammaToLinear( c->r );
   c->g = gammaToLinear( c->g );
   c->b = gammaToLinear( c->b );
}

/**
 * @brief Changes colour space from HSV to RGB.
 *
 * All values go from 0 to 1, except H which is 0-360.
 *
 *    @param[out] c Colour to be converted to from hsv.
 *    @param h Hue to convert.
 *    @param s Saturation to convert.
 *    @param v Value to convert.
 */
void col_hsv2rgb( glColour *c, float h, float s, float v )
{
   float var_h, var_i, var_1, var_2, var_3;

   if ( v > 1 )
      v = 1;

   if ( s == 0 ) {
      c->r = v;
      c->g = v;
      c->b = v;
   } else {
      var_h = h * 6 / 360.;
      var_i = floor( var_h );
      var_1 = v * ( 1 - s );
      var_2 = v * ( 1 - s * ( var_h - var_i ) );
      var_3 = v * ( 1 - s * ( 1 - ( var_h - var_i ) ) );

      if ( var_i == 0 ) {
         c->r = v;
         c->g = var_3;
         c->b = var_1;
      } else if ( var_i == 1 ) {
         c->r = var_2;
         c->g = v;
         c->b = var_1;
      } else if ( var_i == 2 ) {
         c->r = var_1;
         c->g = v;
         c->b = var_3;
      } else if ( var_i == 3 ) {
         c->r = var_1;
         c->g = var_2;
         c->b = v;
      } else if ( var_i == 4 ) {
         c->r = var_3;
         c->g = var_1;
         c->b = v;
      } else {
         c->r = v;
         c->g = var_1;
         c->b = var_2;
      }
   }

   c->r = gammaToLinear( c->r );
   c->g = gammaToLinear( c->g );
   c->b = gammaToLinear( c->b );
}

/**
 * @brief Changes colour space from RGB to HSV.
 *
 * All values go from 0 to 1, except H which is 0-360.
 *
 * Taken from (GIFT) GNU Image Finding Tool.
 *
 *    @param[out] H Stores Hue.
 *    @param[out] S Stores Saturation.
 *    @param[out] V Stores Value.
 *    @param R Red to convert.
 *    @param G Green to convert.
 *    @param B Blue to convert.
 */
void col_rgb2hsv( float *H, float *S, float *V, float R, float G, float B )
{
   float H1, S1, V1;
#ifdef HSV_TRAVIS
   float R1, G1, B1;
#endif /* HSV_TRAVIS */
   float max, min, diff;

   R = gammaToLinear( R );
   G = gammaToLinear( G );
   B = gammaToLinear( B );

   max  = max3( R, G, B );
   min  = min3( R, G, B );
   diff = max - min;

   if ( max == 0 )
      H1 = S1 = V1 = 0;
   else {
      V1 = max;
      S1 = diff / max;
      if ( S1 == 0 )
         /* H1 is undefined, but give it a value anyway */
         H1 = 0;
      else {
#ifdef HSV_TRAVIS
         R1 = ( max - R ) / diff;
         G1 = ( max - G ) / diff;
         B1 = ( max - B ) / diff;

         if ( ( R == max ) && ( G == min ) )
            H1 = 5 + B1;
         else {
            if ( ( R == max ) && ( G != min ) )
               H1 = 1 - G1;
            else {
               if ( ( G == max ) && ( B == min ) )
                  H1 = 1 + R1;
               else {
                  if ( ( G == max ) && ( B != min ) )
                     H1 = 3 - B1;
                  else {
                     if ( R == max )
                        H1 = 3 + G1;
                     else
                        H1 = 5 - R1;
                  }
               }
            }
         }

         H1 *= 60; /* convert to range [0, 360] degrees */
#else              /* HSV_TRAVIS */
         H1 = 0.; /* Shuts up Clang. */
         /* assume Foley & VanDam HSV */
         if ( R == max )
            H1 = ( G - B ) / diff;
         if ( G == max )
            H1 = 2 + ( B - R ) / diff;
         if ( B == max )
            H1 = 4 + ( R - G ) / diff;

         H1 *= 60; /* convert to range [0, 360] degrees */
         if ( H1 < 0 )
            H1 += 360;
#endif             /* HSV_TRAVIS */
      }
   }
   *H = H1;
   *S = S1;
   *V = V1;
}

/**
 * @brief Blends two colours.
 *
 *    @param[out] blend Stores blended output colour.
 *    @param fg Foreground colour.
 *    @param bg Background colour.
 *    @param alpha Alpha value to use (0 to 1).
 */
void col_blend( glColour *blend, const glColour *fg, const glColour *bg,
                float alpha )
{
   blend->r = ( 1. - alpha ) * bg->r + alpha * fg->r;
   blend->g = ( 1. - alpha ) * bg->g + alpha * fg->g;
   blend->b = ( 1. - alpha ) * bg->b + alpha * fg->b;
   blend->a = ( 1. - alpha ) * bg->a + alpha * fg->a;
}
