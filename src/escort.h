/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ESCORT_H
#  define ESCORT_H


#include "physics.h"
#include "pilot.h"


/* Creation. */
int escort_addList( Pilot *p, char *ship, EscortType_t type, unsigned int id,
		    int persist );
int escort_rmList( Pilot *p, unsigned int id );
unsigned int escort_create( Pilot *p, char *ship,
      Vector2d *pos, Vector2d *vel, double dir,
      EscortType_t type, int add );

/* Keybind commands. */
int escorts_attack( Pilot *parent );
int escorts_hold( Pilot *parent );
int escorts_return( Pilot *parent );
int escorts_clear( Pilot *parent );
int escorts_jump( Pilot *parent );


#endif /* ESCORT_H */
