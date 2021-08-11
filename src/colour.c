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


#define _G(x)  (((x) <= 0.04045) ? ((x) / 12.92) : (pow((x + 0.055) / 1.055, 2.4)))

#define COL(R,G,B)      {.r=_G(R), .g=_G(G), .b=_G(B), .a=1.0}
#define ACOL(R,G,B,A)   {.r=_G(R), .g=_G(G), .b=_G(B), .a=A}


/*
 * default colours
 */
/* grey */
const glColour cWhite      = COL( 1.00, 1.00, 1.00 ); /**< White */
const glColour cGrey90     = COL( 0.90, 0.90, 0.90 ); /**< Grey 90% */
const glColour cGrey80     = COL( 0.80, 0.80, 0.80 ); /**< Grey 80% */
const glColour cGrey70     = COL( 0.70, 0.70, 0.70 ); /**< Grey 70% */
const glColour cGrey60     = COL( 0.60, 0.60, 0.60 ); /**< Grey 60% */
const glColour cGrey50     = COL( 0.50, 0.50, 0.50 ); /**< Grey 50% */
const glColour cGrey45     = COL( 0.45, 0.45, 0.45 ); /**< Grey 45% */
const glColour cGrey40     = COL( 0.40, 0.40, 0.40 ); /**< Grey 40% */
const glColour cGrey35     = COL( 0.35, 0.35, 0.35 ); /**< Grey 35% */
const glColour cGrey30     = COL( 0.30, 0.30, 0.30 ); /**< Grey 30% */
const glColour cGrey25     = COL( 0.25, 0.25, 0.25 ); /**< Grey 25% */
const glColour cGrey20     = COL( 0.20, 0.20, 0.20 ); /**< Grey 20% */
const glColour cGrey15     = COL( 0.15, 0.15, 0.15 ); /**< Grey 15% */
const glColour cGrey10     = COL( 0.10, 0.10, 0.10 ); /**< Grey 10% */
const glColour cGrey5      = COL( 0.05, 0.05, 0.05 ); /**< Grey 5% */
const glColour cBlack      = COL( 0.00, 0.00, 0.00 ); /**< Black */

/* Greens. */
const glColour cDarkGreen  = COL( 0.10, 0.50, 0.10 ); /**< Dark Green */
const glColour cGreen      = COL( 0.20, 0.80, 0.20 ); /**< Green */
const glColour cPrimeGreen = COL( 0.00, 1.00, 0.00 ); /**< Primary Green */
/* Reds. */
const glColour cDarkRed    = COL( 0.60, 0.10, 0.10 ); /**< Dark Red */
const glColour cRed        = COL( 0.80, 0.20, 0.20 ); /**< Red */
const glColour cPrimeRed   = COL( 1.00, 0.00, 0.00 ); /**< Primary Red */
const glColour cBrightRed  = COL( 1.00, 0.60, 0.60 ); /**< Bright Red */
/* Oranges. */
const glColour cOrange     = COL( 0.90, 0.70, 0.10 ); /**< Orange */
/* Yellows. */
const glColour cGold       = COL( 1.00, 0.84, 0.00 ); /**< Gold */
const glColour cYellow     = COL( 0.80, 0.80, 0.00 ); /**< Yellow */
/* Blues. */
const glColour cMidnightBlue=COL( 0.10, 0.10, 0.4 ); /**< Midnight Blue. */
const glColour cDarkBlue   = COL( 0.10, 0.10, 0.60 ); /**< Dark Blue */
const glColour cBlue       = COL( 0.20, 0.20, 0.80 ); /**< Blue */
const glColour cLightBlue  = COL( 0.40, 0.40, 1.00 ); /**< Light Blue */
const glColour cPrimeBlue  = COL( 0.00, 0.00, 1.00 ); /**< Primary Blue */
const glColour cCyan       = COL( 0.00, 1.00, 1.00 ); /**< Cyan. */
/* Purples. */
const glColour cPurple     = COL( 0.90, 0.10, 0.90 ); /**< Purple */
const glColour cDarkPurple = COL( 0.68, 0.18, 0.64 ); /**< Dark Purple */
/* Browns. */
const glColour cBrown      = COL( 0.59, 0.28, 0.00 ); /**< Brown */
/* Misc. */
const glColour cSilver     = COL( 0.75, 0.75, 0.75 ); /**< Silver */
const glColour cAqua       = COL( 0.00, 0.75, 1.00 ); /**< Aqua */


/*
 * game specific
 */
const glColour cBlackHilight  = ACOL( 0.0, 0.0, 0.0, 0.4 ); /**< Hilight colour over black background. */
/* toolkit */
const glColour cHilight       = ACOL( 0.1, 0.9, 0.1, 0.6 ); /**< Hilight colour */
/* outfit slot colours.
 * Taken from https://cran.r-project.org/web/packages/khroma/vignettes/tol.html#muted
 */
const glColour cOutfitHeavy   = COL( 0.8, 0.4, 0.46 ); /**< Heavy outfit colour (reddish). */
const glColour cOutfitMedium  = COL( 0.2, 0.73, 0.93 ); /**< Medium outfit colour (blueish). */
//const glColour cOutfitMedium = COL( 0.55, 0.8, 0.93 ); /**< Medium outfit colour (blueish). Technically color safe but doesn't work with our colorblind filter. */
const glColour cOutfitLight   = COL( 0.86, 0.8, 0.46 ); /**< Light outfit colour (yellowish). */
/* objects */
const glColour cInert         = COL( 221./255., 221./255., 221./255. ); /**< Inert object colour */
const glColour cNeutral       = COL( 221./255., 204./255., 119./255. ); /**< Neutral object colour */
const glColour cFriend        = COL(  68./255., 170./255., 153./255. ); /**< Friend object colour */
const glColour cHostile       = COL( 170./255.,  68./255., 153./255. ); /**< Hostile object colour */
const glColour cRestricted    = COL( 153./255., 153./255.,  51./255. ); /**< Restricted object colour. */
/* mission markers */
const glColour cMarkerNew     = COL( 154./255., 112./255., 158./255. ); /**< New mission marker colour. */
const glColour cMarkerComputer= COL( 208./255., 231./255., 202./255. ); /**< Computer mission marker colour. */
const glColour cMarkerLow     = COL( 234./255., 240./255., 181./255. ); /**< Low priority mission marker colour. */
const glColour cMarkerHigh    = COL( 252./255., 247./255., 213./255. ); /**< High priority mission marker colour. */
const glColour cMarkerPlot    = COL( 255./255., 255./255., 255./255. ); /**< Plot mission marker colour. */
/* radar */
const glColour cRadar_player  = COL( 0.9, 0.1, 0.9 ); /**< Player colour on radar. */
const glColour cRadar_tPilot  = COL( 1.0, 1.0, 1.0 ); /**< Targeted object colour on radar. */
const glColour cRadar_tPlanet = COL( 1.0, 1.0, 1.0 ); /**< Targeted planet colour. */
const glColour cRadar_weap    = COL( 0.8, 0.2, 0.2 ); /**< Weapon colour on radar. */
const glColour cRadar_hilight = COL( 0.6, 1.0, 1.0 ); /**< Radar hilighted object. */
/* health */
const glColour cShield        = COL( 0.2, 0.2, 0.8 ); /**< Shield bar colour. */
const glColour cArmour        = COL( 0.5, 0.5, 0.5 ); /**< Armour bar colour. */
const glColour cEnergy        = COL( 0.2, 0.8, 0.2 ); /**< Energy bar colour. */
const glColour cFuel          = COL( 0.9, 0.1, 0.4 ); /**< Fuel bar colour. */

/* Deiz's Super Font Palette */

const glColour cFontRed       = COL( 1.0, 0.4, 0.4 ); /**< Red font colour. */
const glColour cFontGreen     = COL( 0.6, 1.0, 0.4 ); /**< Green font colour. */
const glColour cFontBlue      = COL( 0.4, 0.6, 1.0 ); /**< Blue font colour. */
const glColour cFontYellow    = COL( 1.0, 1.0, 0.5 ); /**< Yellow font colour. */
const glColour cFontWhite     = COL( 0.95, 0.95, 0.95 ); /**< White font colour. */
const glColour cFontGrey      = COL( 0.7, 0.7, 0.7 ); /**< Grey font colour. */
const glColour cFontPurple    = COL( 1.0, 0.3, 1.0 ); /**< Purple font colour. */
const glColour cFontOrange    = COL( 1.0, 0.7, 0.3 ); /**< Orange font colour. */

/*
 * http://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
 */
__attribute__((const)) double linearToGamma( double x )
{
   if (x <= 0.0031308)
      return x * 12.92;
   return 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}
/*
 * http://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
 */
__attribute__((const)) double gammaToLinear( double x )
{
   if (x <= 0.04045)
      return x / 12.92;
   return pow((x + 0.055) / 1.055, 2.4);
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

   if (v > 1)
      v = 1;

   if (s == 0) {
      c->r = v;
      c->g = v;
      c->b = v;
   }
   else {
      var_h = h * 6 / 360.;
      var_i = floor(var_h);
      var_1 = v * (1 - s);
      var_2 = v * (1 - s * (var_h - var_i));
      var_3 = v * (1 - s * (1 - (var_h - var_i)));

      if      (var_i == 0) { c->r = v     ; c->g = var_3 ; c->b = var_1; }
      else if (var_i == 1) { c->r = var_2 ; c->g = v     ; c->b = var_1; }
      else if (var_i == 2) { c->r = var_1 ; c->g = v     ; c->b = var_3; }
      else if (var_i == 3) { c->r = var_1 ; c->g = var_2 ; c->b = v;     }
      else if (var_i == 4) { c->r = var_3 ; c->g = var_1 ; c->b = v;     }
      else                 { c->r = v     ; c->g = var_1 ; c->b = var_2; }
   }
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

   max = max3( R, G, B );
   min = min3( R, G, B );
   diff = max - min;

   if (max == 0)
      H1 = S1 = V1 = 0;
   else {
      V1 = max;
      S1 = diff/max;
      if (S1 == 0)
         /* H1 is undefined, but give it a value anyway */
         H1 = 0;
      else {
#ifdef HSV_TRAVIS
         R1 = (max - R)/diff;
         G1 = (max - G)/diff;
         B1 = (max - B)/diff;

         if ((R == max) && (G == min))
            H1 = 5 + B1;
         else {
            if ((R == max) && (G != min))
               H1 = 1 - G1;
            else {
               if ((G == max) && (B == min))
                  H1 = 1 + R1;
               else {
                  if ((G == max) && (B != min))
                     H1 = 3 - B1;
                  else {
                     if (R == max)
                        H1 = 3 + G1;
                     else
                        H1 = 5 - R1;
                  }
               }
            }
         }

         H1 *= 60; /* convert to range [0, 360] degrees */
#else /* HSV_TRAVIS */
         H1 = 0.; /* Shuts up Clang. */
         /* assume Foley & VanDam HSV */
         if (R == max)
            H1 = (G - B)/diff;
         if (G == max)
            H1 = 2 + (B - R)/diff;
         if (B == max)
            H1 = 4 + (R - G)/diff;

         H1 *= 60; /* convert to range [0, 360] degrees */
         if (H1 < 0)
            H1 += 360;
#endif /* HSV_TRAVIS */
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
void col_blend( glColour *blend, const glColour *fg, const glColour *bg, float alpha )
{
   blend->r = (1. - alpha) * bg->r + alpha * fg->r;
   blend->g = (1. - alpha) * bg->g + alpha * fg->g;
   blend->b = (1. - alpha) * bg->b + alpha * fg->b;
   blend->a = (1. - alpha) * bg->a + alpha * fg->a;
}


#define CHECK_COLOUR(colour) \
      if (strcasecmp(name, #colour) == 0) return &c##colour /**< Checks the colour. */
/**
 * @brief Returns a colour from its name
 *
 *    @param name Colour's name
 *    @return the colour
 */
const glColour* col_fromName( const char* name )
{
   if (name[0] == 'a' || name[0] == 'A') {
      CHECK_COLOUR(Aqua);
   }

   if (name[0] == 'b' || name[0] == 'B') {
      CHECK_COLOUR(Blue);
      CHECK_COLOUR(Black);
      CHECK_COLOUR(Brown);
   }

   if (name[0] == 'c' || name[0] == 'C') {
      CHECK_COLOUR(Cyan);
   }

   if (name[0] == 'd' || name[0] == 'D') {
      CHECK_COLOUR(DarkRed);
      CHECK_COLOUR(DarkBlue);
      CHECK_COLOUR(DarkPurple);
   }

   if (name[0] == 'g' || name[0] == 'G') {
      CHECK_COLOUR(Gold);
      CHECK_COLOUR(Green);
      CHECK_COLOUR(Grey90);
      CHECK_COLOUR(Grey80);
      CHECK_COLOUR(Grey70);
      CHECK_COLOUR(Grey60);
      CHECK_COLOUR(Grey50);
      CHECK_COLOUR(Grey40);
      CHECK_COLOUR(Grey30);
      CHECK_COLOUR(Grey20);
      CHECK_COLOUR(Grey10);
   }

   if (name[0] == 'l' || name[0] == 'L') {
      CHECK_COLOUR(LightBlue);
   }

   if (name[0] == 'o' || name[0] == 'O') {
      CHECK_COLOUR(Orange);
   }

   if (name[0] == 'p' || name[0] == 'P') {
      CHECK_COLOUR(Purple);
   }

   if (name[0] == 'r' || name[0] == 'R') {
      CHECK_COLOUR(Red);
   }

   if (name[0] == 's' || name[0] == 'S') {
      CHECK_COLOUR(Silver);
   }

   if (name[0] == 'w' || name[0] == 'W') {
      CHECK_COLOUR(White);
   }

   if (name[0] == 'y' || name[0] == 'Y') {
      CHECK_COLOUR(Yellow);
   }

   if (name[0] == 'm' || name[0] == 'M') {
      CHECK_COLOUR(MidnightBlue);
   }

   WARN(_("Unknown colour %s"), name);
   return NULL;
}
#undef CHECK_COLOUR

