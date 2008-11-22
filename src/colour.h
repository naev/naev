/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef COLOUR_H
#  define COLOUR_H


/**
 * @struct glColour
 *
 * @brief reperesents a colour via it's RGBA values.
 */
typedef struct glColour_ {
   double r; /**< Red value of the colour (0 to 1). */
   double g; /**< Green value of the colour (0 to 1). */
   double b; /**< Blue value of the colour (0 to 1). */
   double a; /**< Alpha value of the colour (0 to 1). */
} glColour;

/*
 * default colors
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
extern glColour cYellow;
extern glColour cDarkBlue;
extern glColour cBlue;
extern glColour cPurple;

/*
 * game specific
 */
/*
 * game specific
 */
extern glColour cConsole;
extern glColour cDConsole;
/* toolkit */
extern glColour cHilight;
/* objects */
extern glColour cInert;
extern glColour cNeutral;
extern glColour cFriend;
extern glColour cHostile;
/* radar */
extern glColour cRadar_player;
extern glColour cRadar_tPilot;
extern glColour cRadar_tPlanet;
extern glColour cRadar_weap;
/* health */
extern glColour cShield;
extern glColour cArmour;
extern glColour cEnergy;
extern glColour cFuel;



#endif /* COLOUR_H */

