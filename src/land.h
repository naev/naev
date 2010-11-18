/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_H
#  define LAND_H


#include "space.h"


/*
 * The window interfaces.
 */
#define LAND_NUMWINDOWS          7 /**< Number of land windows. */
#define LAND_WINDOW_MAIN         0 /**< Main window. */
#define LAND_WINDOW_BAR          1 /**< Bar window. */
#define LAND_WINDOW_MISSION      2 /**< Mission computer window. */
#define LAND_WINDOW_OUTFITS      3 /**< Outfits window. */
#define LAND_WINDOW_SHIPYARD     4 /**< Shipyard window. */
#define LAND_WINDOW_EQUIPMENT    5 /**< Equipment window. */
#define LAND_WINDOW_COMMODITY    6 /**< Commodity window. */


/*
 * Default button sizes.
 */
#define LAND_BUTTON_WIDTH 200 /**< Default button width. */
#define LAND_BUTTON_HEIGHT 40 /**< Default button height. */


/*
 * Landed at.
 */
extern int landed;
extern Planet* land_planet;


/*
 * Main interface.
 */
int land_doneLoading (void);
void land( Planet* p, int load );
void land_genWindows( int load, int changetab );
void takeoff( int delay );
void land_cleanup (void);
void land_exit (void);


/*
 * Internal usage.
 */
void land_checkAddRefuel (void);
void land_buttonTakeoff( unsigned int wid, char *unused );
unsigned int land_getWid( int window );

/*
 * Error dialogue generation and associated sanity checks.
 */
int can_swap( char* shipname );
int can_swapEquipment( char* shipname );
int can_sell( char* shipname );
int land_errDialogue( char* shipname, char* type );
void land_errDialogueBuild( const char *fmt, ... );


#endif /* LAND_H */
