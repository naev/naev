/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_H
#  define LAND_H


#include "space.h"


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


#endif /* LAND_H */
