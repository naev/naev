/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "physics.h"
#include "pilot.h"
#include "space.h"

/* Creation. */
int escort_addList( Pilot *p, const Ship *ship,
      EscortType_t type, unsigned int id, int persist );
void escort_freeList( Pilot *p );
void escort_rmList( Pilot *p, unsigned int id );
void escort_rmListIndex( Pilot *p, int i );
unsigned int escort_create( Pilot *p, const Ship *ship,
      const vec2 *pos, const vec2 *vel, double dir,
      EscortType_t type, int add, int dockslot );
unsigned int escort_createRef( Pilot *p, Pilot *ref,
      const vec2 *pos, const vec2 *vel, double dir,
      EscortType_t type, int add, int dockslot );
int escort_clearDeployed( Pilot *p );

/* Keybind commands. */
int escorts_attack( Pilot *parent );
int escorts_hold( Pilot *parent );
int escorts_return( Pilot *parent );
int escorts_clear( Pilot *parent );
int escorts_jump( Pilot *parent, const JumpPoint *jp );
int escort_playerCommand( Pilot *e );
