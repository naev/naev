/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef COLOUR_H
#  define COLOUR_H


/*
 * Colours
 */
typedef struct glColour_ {
	double r, g, b, a;
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

/*
 * game specific
 */
/*
 * game specific
 */
extern glColour cConsole;
extern glColour cDConsole;
/* objects */
extern glColour cInert;
extern glColour cNeutral;
extern glColour cFriend;
extern glColour cHostile;
/* radar */
extern glColour cRadar_player;
extern glColour cRadar_targ;
extern glColour cRadar_weap;
/* health */
extern glColour cShield;
extern glColour cArmour;
extern glColour cEnergy;



#endif /* COLOUR_H */

