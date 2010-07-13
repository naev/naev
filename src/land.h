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
 * Landed at.
 */
extern int landed;
extern Planet* land_planet;


/*
 * Main interface.
 */
void land ( Planet* p );
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
