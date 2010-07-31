/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file colour.c
 *
 * @brief Predefined colours for use with NAEV.
 */


#include "colour.h"

#include <math.h>
#include <string.h>

#include "nmath.h"
#include "naev.h"
#include "log.h"
#include "ncompat.h"
#include "nstd.h"


/*
 * default colours
 */
/* grey */
glColour cWhite      = { .r=1.00, .g=1.00, .b=1.00, .a=1. }; /**< White */
glColour cGrey90     = { .r=0.90, .g=0.90, .b=0.90, .a=1. }; /**< Grey 90% */
glColour cGrey80     = { .r=0.80, .g=0.80, .b=0.80, .a=1. }; /**< Grey 80% */
glColour cGrey70     = { .r=0.70, .g=0.70, .b=0.70, .a=1. }; /**< Grey 70% */
glColour cGrey60     = { .r=0.60, .g=0.60, .b=0.60, .a=1. }; /**< Grey 60% */
glColour cGrey50     = { .r=0.50, .g=0.50, .b=0.50, .a=1. }; /**< Grey 50% */
glColour cGrey40     = { .r=0.40, .g=0.40, .b=0.40, .a=1. }; /**< Grey 40% */
glColour cGrey30     = { .r=0.30, .g=0.30, .b=0.30, .a=1. }; /**< Grey 30% */
glColour cGrey20     = { .r=0.20, .g=0.20, .b=0.20, .a=1. }; /**< Grey 20% */
glColour cGrey10     = { .r=0.10, .g=0.10, .b=0.10, .a=1. }; /**< Grey 10% */
glColour cBlack      = { .r=0.00, .g=0.00, .b=0.00, .a=1. }; /**< Black */

/* Greens. */
glColour cGreen      = { .r=0.20, .g=0.80, .b=0.20, .a=1. }; /**< Green */
/* Reds. */
glColour cDarkRed    = { .r=0.60, .g=0.10, .b=0.10, .a=1. }; /**< Dark Red */
glColour cRed        = { .r=0.80, .g=0.20, .b=0.20, .a=1. }; /**< Red */
/* Oranges. */
glColour cOrange     = { .r=0.90, .g=0.70, .b=0.10, .a=1. }; /**< Orange */
/* Yellows. */
glColour cGold       = { .r=1.00, .g=0.84, .b=0.00, .a=1. }; /**< Gold */
glColour cYellow     = { .r=0.80, .g=0.80, .b=0.00, .a=1. }; /**< Yellow */
/* Blues. */
glColour cMidnightBlue = { .r=0.40, .g=0.4, .b=0.60, .a=1. }; /**< Midnight Blue. */
glColour cDarkBlue   = { .r=0.10, .g=0.10, .b=0.60, .a=1. }; /**< Dark Blue */
glColour cBlue       = { .r=0.20, .g=0.20, .b=0.80, .a=1. }; /**< Blue */
glColour cLightBlue  = { .r=0.40, .g=0.40, .b=1.00, .a=1. }; /**< Light Blue */
glColour cCyan       = { .r=0.00, .g=1.00, .b=1.00, .a=1. }; /* Cyan. */
/* Purples. */
glColour cPurple     = { .r=0.90, .g=0.10, .b=0.90, .a=1. }; /**< Purple */
glColour cDarkPurple = { .r=0.68, .g=0.18, .b=0.64, .a=1. }; /**< Dark Purple */
/* Browns. */
glColour cBrown      = { .r=0.59, .g=0.28, .b=0.00, .a=1. }; /**< Brown */
/* Misc. */
glColour cSilver     = { .r=0.75, .g=0.75, .b=0.75, .a=1. }; /**< Silver */
glColour cAqua       = { .r=0.00, .g=0.75, .b=1.00, .a=1. }; /**< Aqua */


/*
 * game specific
 */
glColour cConsole       =  { .r = 0.1, .g = 0.9, .b = 0.1, .a = 1.  }; /**< Console colour */
glColour cDConsole      =  { .r = 0.0, .g = 0.7, .b = 0.0, .a = 1.  }; /**< Dark Console colour */
/* toolkit */
glColour cHilight       =  { .r = 0.1, .g = 0.9, .b = 0.1, .a = 0.3 }; /**< Hilight colour */
/* objects */
glColour cInert         =  { .r = 0.6, .g = 0.6, .b = 0.6, .a = 1.  }; /**< Inert object colour */
glColour cNeutral       =  { .r = 0.9, .g = 1.0, .b = 0.3, .a = 1.  }; /**< Neutral object colour */
glColour cMapNeutral    =  { .r = 0.3, .g = 0.3, .b = 0.3, .a = 1.  }; /**< Neutral object map screen text colour */
glColour cFriend        =  { .r = 0.0, .g = 0.8, .b = 0.0, .a = 1.  }; /**< Friend object colour */
glColour cHostile       =  { .r = 0.9, .g = 0.2, .b = 0.2, .a = 1.  }; /**< Hostile object colour */
/* radar */
glColour cRadar_player  =  { .r = 0.4, .g = 0.8, .b = 0.4, .a = 1.  }; /**< Player colour on radar. */
glColour cRadar_tPilot  =  { .r = 0.8, .g = 0.5, .b = 0.0, .a = 1.  }; /**< Targetted object colour on radar. */
glColour cRadar_tPlanet =  { .r = 0.7, .g = 0.0, .b = 0.9, .a = 1.  }; /**< Targetted planet colour. */
glColour cRadar_weap    =  { .r = 0.8, .g = 0.2, .b = 0.2, .a = 1.  }; /**< Weapon colour on radar. */
/* health */
glColour cShield        =  { .r = 0.2, .g = 0.2, .b = 0.8, .a = 1.  }; /**< Shield bar colour. */
glColour cArmour        =  { .r = 0.5, .g = 0.5, .b = 0.5, .a = 1.  }; /**< Armour bar colour. */
glColour cEnergy        =  { .r = 0.2, .g = 0.8, .b = 0.2, .a = 1.  }; /**< Energy bar colour. */
glColour cFuel          =  { .r = 0.9, .g = 0.1, .b = 0.4, .a = 1.  }; /**< Fuel bar colour. */

/* Deiz's Super Font Palette */

glColour cFontRed       =  { .r = 0.8, .g = 0.2, .b = 0.2, .a = 1. };
glColour cFontGreen     =  { .r = 0.4, .g = 0.8, .b = 0.2, .a = 1. };
glColour cFontBlue      =  { .r = 0.2, .g = 0.4, .b = 0.8, .a = 1. };
glColour cFontYellow    =  { .r = 0.9, .g = 0.8, .b = 0.0, .a = 1. };
glColour cFontWhite     =  { .r = 0.8, .g = 0.8, .b = 0.8, .a = 1.  };
glColour cFontPurple    =  { .r = 0.7, .g = 0.3, .b = 0.7, .a = 1.  };
glColour cFontFriendly  =  { .r = 0.3, .g = 0.9, .b = 0.3, .a = 1. };
glColour cFontHostile   =  { .r = 0.9, .g = 0.2, .b = 0.2, .a = 1. };
glColour cFontNeutral   =  { .r = 1.0, .g = 0.9, .b = 0.0, .a = 1.  };


/**
 * @brief Changes colourspace from HSV to RGB.
 *
 * All values go from 0 to 1, except H which is 0-360.
 *
 *    @param[out] r Stores R.
 *    @param[out] g Stores G.
 *    @param[out] b Stores B.
 *    @param h Hue to convert.
 *    @param s Saturation to convert.
 *    @param v Value to convert.
 */
void col_hsv2rgb( double *r, double *g, double *b, double h, double s, double v )
{
   double var_h, var_i, var_1, var_2, var_3;

   if (v > 1)
      v = 1;

   if (s == 0) {
      *r = v;
      *g = v;
      *b = v;
   }
   else {
      var_h = h * 6 / 360.;
      var_i = floor(var_h);
      var_1 = v * (1 - s);
      var_2 = v * (1 - s * (var_h - var_i));
      var_3 = v * (1 - s * (1 - (var_h - var_i)));

      if      (var_i == 0) { *r = v     ; *g = var_3 ; *b = var_1; }
      else if (var_i == 1) { *r = var_2 ; *g = v     ; *b = var_1; }
      else if (var_i == 2) { *r = var_1 ; *g = v     ; *b = var_3; }
      else if (var_i == 3) { *r = var_1 ; *g = var_2 ; *b = v;     }
      else if (var_i == 4) { *r = var_3 ; *g = var_1 ; *b = v;     }
      else                 { *r = v     ; *g = var_1 ; *b = var_2; }
   }
}


/**
 * @brief Changes colourspace from RGB to HSV.
 *
 * All values go from 0 to 1, except H which is 0-360.
 *
 * Taken from (GIFT) GNU Image Finding Tool.
 *
 *    @param[out] h Stores Hue.
 *    @param[out] s Stores Saturation.
 *    @param[out] v Stores Value.
 *    @param r Red to convert.
 *    @param g Green to convert.
 *    @param b Blue to convert.
 */
void col_rgb2hsv( double *H, double *S, double *V, double R, double G, double B )
{
   double H1, S1, V1;
#ifdef HSV_TRAVIS
   double R1, G1, B1;
#endif /* HSV_TRAVIS */
   double max, min, diff;

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
void col_blend( glColour *blend, glColour fg, glColour bg, double alpha )
{
   glColour temp;

   temp.r = (1 - alpha) * bg.r + alpha * fg.r;
   temp.g = (1 - alpha) * bg.g + alpha * fg.g;
   temp.b = (1 - alpha) * bg.b + alpha * fg.b;
   temp.a = (1 - alpha) * bg.a + alpha * fg.a;

   *blend = temp;
}


/**
 * @brief Returns a colour from it's name
 *
 *    @param name Colour's name
 *    @return the colour
 */
#define CHECK_COLOUR(colour) \
      if (STRCASECMP(name, #colour) == 0) return &c##colour
glColour* col_fromName(const char* name) {
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


   WARN("Unknown colour %s", name);
   return NULL;
}
#undef CHECK_COLOUR

