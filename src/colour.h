

#ifndef COLOUR_H
#  define COLOUR_H


/*
 * Colours
 */
typedef struct {
	double r, g, b, a;
} glColour;

/*
 * default colors
 */
/* greyscale */
extern glColour cWhite;
#define cGrey  cGrey70
extern glColour cBlack;

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
extern glColour cRed;


#endif /* COLOUR_H */

