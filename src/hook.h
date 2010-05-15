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
 *    - "takeoff" - When taking off
 *    - "jumpin" - When player jumps (after changing system)
 *    - "jumpout" - When player jumps (before changing system)
 *    - "time" - When time is increment drastically (hyperspace and taking off)
 *    - "hail" - When any pilot is hailed
 *    - "board" - WHen any pilot is boarded
 *  - Landing
 *    - "land" - When landed
 *    - "outfits" - When visited outfitter
 *    - "shipyard" - When visited shipyard
 *    - "bar" - When visited bar
 *    - "mission" - When visited mission computer
 *    - "commodity" - When visited commodity exchange
 *    - "equipment" - When visiting equipment place < br/>
 */
int hooks_runParam( const char* stack, unsigned int pilot );
int hooks_run( const char* stack );
int hook_runIDparam( unsigned int id, unsigned int pilot );
int hook_runID( unsigned int id ); /* runs hook of specific id */

/* destroys hooks */
void hook_cleanup (void);

/* Timer hooks. */
void hooks_update( double dt );
unsigned int hook_addTimerMisn( unsigned int parent, const char *func, double ms );
unsigned int hook_addTimerEvt( unsigned int parent, const char *func, double ms );


#endif /* HOOK_H */

