/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ESCORT_H
#  define ESCORT_H


#include "physics.h"
#include "pilot.h"


int escort_create( unsigned int parent, char *ship,
      Vector2d *pos, Vector2d *vel, int carried );

/* Keybind commands. */
void escorts_attack( Pilot *parent );
void escorts_hold( Pilot *parent );
void escorts_return( Pilot *parent );
void escorts_clear( Pilot *parent );


#endif /* ESCORT_H */
