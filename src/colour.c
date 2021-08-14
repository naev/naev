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

#define ACOL(R,G,B,A)   {.r=_G(R), .g=_G(G), .b=_G(B), .a=A}
#define COL(R,G,B)      ACOL(R,G,B,1.0)


/*
 * default colours
 */
/* grey */
const glColour cWhite      = {.r=0xff.fffffffffff8p-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< White */
const glColour cGrey90     = {.r=0xc9.93da0f6c33b8p-8, .g=0xc9.93da0f6c33b8p-8, .b=0xc9.93da0f6c33b8p-8, .a=1.0}; /**< Grey 90% */
const glColour cGrey80     = {.r=0x9a.946db0d07b98p-8, .g=0x9a.946db0d07b98p-8, .b=0x9a.946db0d07b98p-8, .a=1.0}; /**< Grey 80% */
const glColour cGrey70     = {.r=0x72.af5e5c6cc2d8p-8, .g=0x72.af5e5c6cc2d8p-8, .b=0x72.af5e5c6cc2d8p-8, .a=1.0}; /**< Grey 70% */
const glColour cGrey60     = {.r=0x51.8c481a4b1ebcp-8, .g=0x51.8c481a4b1ebcp-8, .b=0x51.8c481a4b1ebcp-8, .a=1.0}; /**< Grey 60% */
const glColour cGrey50     = {.r=0x36.cb66725ea6a8p-8, .g=0x36.cb66725ea6a8p-8, .b=0x36.cb66725ea6a8p-8, .a=1.0}; /**< Grey 50% */
const glColour cGrey45     = {.r=0x2b.af62f28e4a14p-8, .g=0x2b.af62f28e4a14p-8, .b=0x2b.af62f28e4a14p-8, .a=1.0}; /**< Grey 45% */
const glColour cGrey40     = {.r=0x22.03a887bf80dep-8, .g=0x22.03a887bf80dep-8, .b=0x22.03a887bf80dep-8, .a=1.0}; /**< Grey 40% */
const glColour cGrey35     = {.r=0x19.b927ff090438p-8, .g=0x19.b927ff090438p-8, .b=0x19.b927ff090438p-8, .a=1.0}; /**< Grey 35% */
const glColour cGrey30     = {.r=0x12.bfc9c84a7aeep-8, .g=0x12.bfc9c84a7aeep-8, .b=0x12.bfc9c84a7aeep-8, .a=1.0}; /**< Grey 30% */
const glColour cGrey25     = {.r=0x0d.06371ed8623ap-8, .g=0x0d.06371ed8623ap-8, .b=0x0d.06371ed8623ap-8, .a=1.0}; /**< Grey 25% */
const glColour cGrey20     = {.r=0x08.798dd1c37f93p-8, .g=0x08.798dd1c37f93p-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Grey 20% */
const glColour cGrey15     = {.r=0x05.04f0fa29522cp-8, .g=0x05.04f0fa29522cp-8, .b=0x05.04f0fa29522cp-8, .a=1.0}; /**< Grey 15% */
const glColour cGrey10     = {.r=0x02.90db1c0ebd3bp-8, .g=0x02.90db1c0ebd3bp-8, .b=0x02.90db1c0ebd3bp-8, .a=1.0}; /**< Grey 10% */
const glColour cGrey5      = {.r=0x01.01f21b72f7fep-8, .g=0x01.01f21b72f7fep-8, .b=0x01.01f21b72f7fep-8, .a=1.0}; /**< Grey 5% */
const glColour cBlack      = {.r=0x00.000000000000p-8, .g=0x00.000000000000p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Black */

/* Greens. */
const glColour cDarkGreen  = {.r=0x02.90db1c0ebd3bp-8, .g=0x36.cb66725ea6a8p-8, .b=0x02.90db1c0ebd3bp-8, .a=1.0}; /**< Dark Green */
const glColour cGreen      = {.r=0x08.798dd1c37f93p-8, .g=0x9a.946db0d07b98p-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Green */
const glColour cPrimeGreen = {.r=0x00.000000000000p-8, .g=0xff.fffffffffff8p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Primary Green */
/* Reds. */
const glColour cDarkRed    = {.r=0x51.8c481a4b1ebcp-8, .g=0x02.90db1c0ebd3bp-8, .b=0x02.90db1c0ebd3bp-8, .a=1.0}; /**< Dark Red */
const glColour cRed        = {.r=0x9a.946db0d07b98p-8, .g=0x08.798dd1c37f93p-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Red */
const glColour cPrimeRed   = {.r=0xff.fffffffffff8p-8, .g=0x00.000000000000p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Primary Red */
const glColour cBrightRed  = {.r=0xff.fffffffffff8p-8, .g=0x51.8c481a4b1ebcp-8, .b=0x51.8c481a4b1ebcp-8, .a=1.0}; /**< Bright Red */
/* Oranges. */
const glColour cOrange     = {.r=0xc9.93da0f6c33b8p-8, .g=0x72.af5e5c6cc2d8p-8, .b=0x02.90db1c0ebd3bp-8, .a=1.0}; /**< Orange */
/* Yellows. */
const glColour cGold       = {.r=0xff.fffffffffff8p-8, .g=0xac.820f118ca4a8p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Gold */
const glColour cYellow     = {.r=0x9a.946db0d07b98p-8, .g=0x9a.946db0d07b98p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Yellow */
/* Blues. */
const glColour cMidnightBlue={.r=0x02.90db1c0ebd3bp-8, .g=0x02.90db1c0ebd3bp-8, .b=0x22.03a887bf80dep-8, .a=1.0}; /**< Midnight Blue. */
const glColour cDarkBlue   = {.r=0x02.90db1c0ebd3bp-8, .g=0x02.90db1c0ebd3bp-8, .b=0x51.8c481a4b1ebcp-8, .a=1.0}; /**< Dark Blue */
const glColour cBlue       = {.r=0x08.798dd1c37f93p-8, .g=0x08.798dd1c37f93p-8, .b=0x9a.946db0d07b98p-8, .a=1.0}; /**< Blue */
const glColour cLightBlue  = {.r=0x22.03a887bf80dep-8, .g=0x22.03a887bf80dep-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Light Blue */
const glColour cPrimeBlue  = {.r=0x00.000000000000p-8, .g=0x00.000000000000p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Primary Blue */
const glColour cCyan       = {.r=0x00.000000000000p-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Cyan. */
/* Purples. */
const glColour cPurple     = {.r=0xc9.93da0f6c33b8p-8, .g=0x02.90db1c0ebd3bp-8, .b=0xc9.93da0f6c33b8p-8, .a=1.0}; /**< Purple */
const glColour cDarkPurple = {.r=0x6b.874d362149c8p-8, .g=0x06.f759ed40898ap-8, .b=0x5e.03dd5288c3d0p-8, .a=1.0}; /**< Dark Purple */
/* Browns. */
const glColour cBrown      = {.r=0x4e.9781e2b01decp-8, .g=0x10.5033e0b9c184p-8, .b=0x00.000000000000p-8, .a=1.0}; /**< Brown */
/* Misc. */
const glColour cSilver     = {.r=0x85.c3f8f9bfd4f0p-8, .g=0x85.c3f8f9bfd4f0p-8, .b=0x85.c3f8f9bfd4f0p-8, .a=1.0}; /**< Silver */
const glColour cAqua       = {.r=0x00.000000000000p-8, .g=0x85.c3f8f9bfd4f0p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Aqua */


/*
 * game specific
 */
const glColour cBlackHilight  = {.r=0x00.000000000000p-8, .g=0x00.000000000000p-8, .b=0x00.000000000000p-8, .a=0.4}; /**< Hilight colour over black background. */
/* outfit slot colours.
 * Taken from https://cran.r-project.org/web/packages/khroma/vignettes/tol.html#muted
 */
const glColour cOutfitHeavy   = {.r=0x9a.946db0d07b98p-8, .g=0x22.03a887bf80dep-8, .b=0x2d.ca42bfce5faap-8, .a=1.0}; /**< Heavy outfit colour (reddish). */
const glColour cOutfitMedium  = {.r=0x08.798dd1c37f93p-8, .g=0x7d.ed7d1f37a810p-8, .b=0xd9.1c4fe20fe318p-8, .a=1.0}; /**< Medium outfit colour (blueish). */
//const glColour cOutfitMedium = {.r=0x43.65e2d739ba08p-8, .g=0x9a.946db0d07b98p-8, .b=0xd9.1c4fe20fe318p-8, .a=1.0}; /**< Medium outfit colour (blueish). Technically color safe but doesn't work with our colorblind filter. */
const glColour cOutfitLight   = {.r=0xb5.e7af7d25bbc8p-8, .g=0x9a.946db0d07b98p-8, .b=0x2d.ca42bfce5faap-8, .a=1.0}; /**< Light outfit colour (yellowish). */
/* objects */
const glColour cInert         = {.r=0xb9.1a2413ecc7b0p-8, .g=0xb9.1a2413ecc7b0p-8, .b=0xb9.1a2413ecc7b0p-8, .a=1.0}; /**< Inert object colour */
const glColour cNeutral       = {.r=0xb9.1a2413ecc7b0p-8, .g=0x9a.946db0d07b98p-8, .b=0x2f.39c0d44f2606p-8, .a=1.0}; /**< Neutral object colour */
const glColour cFriend        = {.r=0x0e.cc563033ac1ap-8, .g=0x66.e8040a191484p-8, .b=0x51.8c481a4b1ebcp-8, .a=1.0}; /**< Friend object colour */
const glColour cHostile       = {.r=0x66.e8040a191484p-8, .g=0x0e.cc563033ac1ap-8, .b=0x51.8c481a4b1ebcp-8, .a=1.0}; /**< Hostile object colour */
const glColour cRestricted    = {.r=0x51.8c481a4b1ebcp-8, .g=0x51.8c481a4b1ebcp-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Restricted object colour. */
/* mission markers */
const glColour cMarkerNew     = {.r=0x52.b9836b108a74p-8, .g=0x29.7ac1d55c7ce6p-8, .b=0x57.87b429036aacp-8, .a=1.0}; /**< New mission marker colour. */
const glColour cMarkerComputer= {.r=0xa1.794cb85347c0p-8, .g=0xcc.91ff3dea7b38p-8, .b=0x97.32cbde266500p-8, .a=1.0}; /**< Computer mission marker colour. */
const glColour cMarkerLow     = {.r=0xd2.a21652bf2970p-8, .g=0xdf.11ea5fc4c0d8p-8, .b=0x76.4aada1bfde08p-8, .a=1.0}; /**< Low priority mission marker colour. */
const glColour cMarkerHigh    = {.r=0xf9.33b5e6b4d6f0p-8, .g=0xee.1bbec6607ea8p-8, .b=0xaa.56d26d4bd0d8p-8, .a=1.0}; /**< High priority mission marker colour. */
const glColour cMarkerPlot    = {.r=0xff.fffffffffff8p-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Plot mission marker colour. */
/* radar */
const glColour cRadar_player  = {.r=0xc9.93da0f6c33b8p-8, .g=0x02.90db1c0ebd3bp-8, .b=0xc9.93da0f6c33b8p-8, .a=1.0}; /**< Player colour on radar. */
const glColour cRadar_tPilot  = {.r=0xff.fffffffffff8p-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Targeted object colour on radar. */
const glColour cRadar_tPlanet = {.r=0xff.fffffffffff8p-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Targeted planet colour. */
const glColour cRadar_weap    = {.r=0x9a.946db0d07b98p-8, .g=0x08.798dd1c37f93p-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Weapon colour on radar. */
const glColour cRadar_hilight = {.r=0x51.8c481a4b1ebcp-8, .g=0xff.fffffffffff8p-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Radar hilighted object. */
/* health */
const glColour cShield        = {.r=0x08.798dd1c37f93p-8, .g=0x08.798dd1c37f93p-8, .b=0x9a.946db0d07b98p-8, .a=1.0}; /**< Shield bar colour. */
const glColour cArmour        = {.r=0x36.cb66725ea6a8p-8, .g=0x36.cb66725ea6a8p-8, .b=0x36.cb66725ea6a8p-8, .a=1.0}; /**< Armour bar colour. */
const glColour cEnergy        = {.r=0x08.798dd1c37f93p-8, .g=0x9a.946db0d07b98p-8, .b=0x08.798dd1c37f93p-8, .a=1.0}; /**< Energy bar colour. */
const glColour cFuel          = {.r=0xc9.93da0f6c33b8p-8, .g=0x02.90db1c0ebd3bp-8, .b=0x22.03a887bf80dep-8, .a=1.0}; /**< Fuel bar colour. */

/* Deiz's Super Font Palette */

const glColour cFontRed       = {.r=0xff.fffffffffff8p-8, .g=0x22.03a887bf80dep-8, .b=0x22.03a887bf80dep-8, .a=1.0}; /**< Red font colour. */
const glColour cFontGreen     = {.r=0x51.8c481a4b1ebcp-8, .g=0xff.fffffffffff8p-8, .b=0x22.03a887bf80dep-8, .a=1.0}; /**< Green font colour. */
const glColour cFontBlue      = {.r=0x22.03a887bf80dep-8, .g=0x51.8c481a4b1ebcp-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Blue font colour. */
const glColour cFontYellow    = {.r=0xff.fffffffffff8p-8, .g=0xff.fffffffffff8p-8, .b=0x36.cb66725ea6a8p-8, .a=1.0}; /**< Yellow font colour. */
const glColour cFontWhite     = {.r=0xe3.d764f44ce578p-8, .g=0xe3.d764f44ce578p-8, .b=0xe3.d764f44ce578p-8, .a=1.0}; /**< White font colour. */
const glColour cFontGrey      = {.r=0x72.af5e5c6cc2d8p-8, .g=0x72.af5e5c6cc2d8p-8, .b=0x72.af5e5c6cc2d8p-8, .a=1.0}; /**< Grey font colour. */
const glColour cFontPurple    = {.r=0xff.fffffffffff8p-8, .g=0x12.bfc9c84a7aeep-8, .b=0xff.fffffffffff8p-8, .a=1.0}; /**< Purple font colour. */
const glColour cFontOrange    = {.r=0xff.fffffffffff8p-8, .g=0x72.af5e5c6cc2d8p-8, .b=0x12.bfc9c84a7aeep-8, .a=1.0}; /**< Orange font colour. */

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

