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
int escorts_attack( Pilot *parent );
int escorts_hold( Pilot *parent );
int escorts_return( Pilot *parent );
int escorts_clear( Pilot *parent );


#endif /* ESCORT_H */
