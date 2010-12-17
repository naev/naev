/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_EW_H
#  define PILOT_EW_H


#include "pilot.h"


void pilot_ewUpdateStatic( Pilot *p );
void pilot_ewUpdateDynamic( Pilot *p );

double pilot_ewMovement( double vmod );
double pilot_ewMass( double mass );


#endif /* PILOT_EW_H */
