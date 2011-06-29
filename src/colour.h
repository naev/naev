/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef COLOUR_H
#  define COLOUR_H


#include "SDL_opengl.h"


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
extern glColour cWhite;
#define cGrey  cGrey70
extern glColour cBlack;
/* greys */
extern glColour cGrey90;
extern glColour cGrey80;
extern glColour cGrey70;
extern glColour cGrey60;
extern glColour cGrey50;
extern glColour cGrey40;
extern glColour cGrey30;
extern glColour cGrey20;
extern glColour cGrey10;

extern glColour cGreen;
extern glColour cDarkRed;
extern glColour cRed;
extern glColour cOrange;
extern glColour cYellow;
extern glColour cMidnightBlue;
extern glColour cDarkBlue;
extern glColour cBlue;
extern glColour cLightBlue;
extern glColour cPurple;
extern glColour cDarkPurple;
extern glColour cBrown;
extern glColour cGold;
extern glColour cSilver;
extern glColour cAqua;

/*
 * game specific
 */
/*
 * game specific
 */
extern glColour cBlackHilight;
extern glColour cConsole;
extern glColour cDConsole;
/* toolkit */
extern glColour cHilight;
/* objects */
extern glColour cInert;
extern glColour cNeutral;
extern glColour cMapNeutral;
extern glColour cFriend;
extern glColour cHostile;
extern glColour cRestricted;
extern glColour cDRestricted;
/* radar */
extern glColour cRadar_player;
extern glColour cRadar_tPilot;
extern glColour cRadar_tPlanet;
extern glColour cRadar_weap;
extern glColour cRadar_hilight;
/* health */
extern glColour cShield;
extern glColour cArmour;
extern glColour cEnergy;
extern glColour cFuel;
/* Deiz's Super Font Palette */
extern glColour cFontRed;
extern glColour cFontGreen;
extern glColour cFontBlue;
extern glColour cFontYellow;
extern glColour cFontWhite;
extern glColour cFontPurple;
extern glColour cFontFriendly;
extern glColour cFontHostile;
extern glColour cFontNeutral;

glColour* col_fromName( const char* name );


/*
 * Colour space conversion routines.
 */
void col_hsv2rgb( double *r, double *g, double *b, double h, double s, double v );
void col_rgb2hsv( double *h, double *s, double *v, double r, double g, double b );
void col_blend( glColour *out, glColour fg, glColour bg, double alpha );



#endif /* COLOUR_H */

