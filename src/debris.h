/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DEBRIS_H
#  define DEBRIS_H


#include "pilot.h"


void debris_cleanup (void);
void debris_add( double mass, double rad, double px, double py,
      double vx, double vy );


#endif /* DEBRIS_H */

