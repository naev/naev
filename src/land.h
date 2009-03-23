/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_H
#  define LAND_H


#include "space.h"


extern int landed;
extern Planet* land_planet;


void land ( Planet* p );
void takeoff( int delay );
void land_cleanup (void);


#endif /* LAND_H */
