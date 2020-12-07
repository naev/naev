/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef COLOUR_H
#  define COLOUR_H


#include "glad.h"


/**
 * @brief represents a colour via its RGBA values.
 */
typedef struct glColour_ {
   GLfloat r; /**< Red value of the colour (0 to 1). */
   GLfloat g; /**< Green value of the colour (0 to 1). */
   GLfloat b; /**< Blue value of the colour (0 to 1). */
   GLfloat a; /**< Alpha value of the colour (0 to 1). */
} __attribute__((packed)) glColour;

/*
 * default colours
 */
/* greyscale */
extern const glColour cWhite;
#define cGrey  cGrey70
extern const glColour cBlack;
/* greys */
extern const glColour cGrey90;
extern const glColour cGrey80;
extern const glColour cGrey70;
extern const glColour cGrey60;
extern const glColour cGrey50;
extern const glColour cGrey40;
extern const glColour cGrey30;
extern const glColour cGrey25;
extern const glColour cGrey20;
extern const glColour cGrey15;
extern const glColour cGrey10;

extern const glColour cDarkGreen;
extern const glColour cGreen;
extern const glColour cPrimeGreen;
extern const glColour cDarkRed;
extern const glColour cRed;
extern const glColour cPrimeRed;
extern const glColour cBrightRed;
extern const glColour cOrange;
extern const glColour cYellow;
extern const glColour cMidnightBlue;
extern const glColour cDarkBlue;
extern const glColour cBlue;
extern const glColour cLightBlue;
extern const glColour cPrimeBlue;
extern const glColour cCyan;
extern const glColour cPurple;
extern const glColour cDarkPurple;
extern const glColour cBrown;
extern const glColour cGold;
extern const glColour cSilver;
extern const glColour cAqua;

/*
 * game specific
 */
extern const glColour cBlackHilight;
/* toolkit */
extern const glColour cHilight;
/* objects */
extern const glColour cInert;
extern const glColour cNeutral;
extern const glColour cFriend;
extern const glColour cHostile;
extern const glColour cRestricted;
extern const glColour cDRestricted;
/* radar */
extern const glColour cRadar_player;
extern const glColour cRadar_tPilot;
extern const glColour cRadar_tPlanet;
extern const glColour cRadar_weap;
extern const glColour cRadar_hilight;
/* health */
extern const glColour cShield;
extern const glColour cArmour;
extern const glColour cEnergy;
extern const glColour cFuel;
/* Deiz's Super Font Palette */
extern const glColour cFontRed;
extern const glColour cFontGreen;
extern const glColour cFontBlue;
extern const glColour cFontOrange;
extern const glColour cFontYellow;
extern const glColour cFontWhite;
extern const glColour cFontGrey;
extern const glColour cFontPurple;


/*
 * Misc functions.
 */
const glColour* col_fromName( const char* name );

/*
 * Colour space conversion routines.
 */
void col_hsv2rgb( double *r, double *g, double *b, double h, double s, double v );
void col_rgb2hsv( double *h, double *s, double *v, double r, double g, double b );
void col_blend( glColour *blend, const glColour *fg, const glColour *bg, double alpha );



#endif /* COLOUR_H */

