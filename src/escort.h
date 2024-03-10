/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "vec2.h"
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
int escort_clearDeployed( Pilot *p, int dockslot );

/* Keybind commands. */
int escorts_attack( Pilot *parent );
int escorts_hold( const Pilot *parent );
int escorts_return( const Pilot *parent );
int escorts_clear( const Pilot *parent );
int escorts_jump( const Pilot *parent, const JumpPoint *jp );
int escort_playerCommand( const Pilot *e );
