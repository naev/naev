/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef HOOK_H
#  define HOOK_H


#include "mission.h"


/* add/run hooks */
unsigned int hook_addMisn( unsigned int parent, const char *func, const char *stack );
unsigned int hook_addEvent( unsigned int parent, const char *func, const char *stack );
unsigned int hook_addFunc( int (*func)(void*), void* data, const char *stack );
int hook_rm( unsigned int id );
void hook_rmMisnParent( unsigned int parent );
void hook_rmEventParent( unsigned int parent );

/* 
 * run hooks
 *
 * Currently used:
 *  - General
 *    - "takeoff" - when taking off
 *    - "jump" - when changing system
 *    - "time" - when time is increment drastically (hyperspace and taking off)
 *  - Landing
 *    - "land" - when landed
 *    - "outfits" - when visited outfitter
 *    - "shipyard" - when visited shipyard
 *    - "bar" - when visited bar
 *    - "mission" - when visited mission computer
 *    - "commodity" - when visited commodity exchange
 *    - "equipment" - when visiting equipment place
 */
int hooks_run( const char* stack );
int hook_runID( unsigned int id ); /* runs hook of specific id */

/* destroys hooks */
void hook_cleanup (void);


#endif /* HOOK_H */

