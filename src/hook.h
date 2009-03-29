/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef HOOK_H
#  define HOOK_H


#include "mission.h"


/* add/run hooks */
unsigned int hook_add( unsigned int parent, const char *func, const char *stack );
int hook_rm( unsigned int id );
void hook_rmParent( unsigned int parent );

/* 
 * run hooks
 *
 * Currently used:
 *    "land" - when landed
 *    "takeoff" - when taking off
 *    "jump" - when changing system
 *    "time" - when time is increment drastically (hyperspace and taking off)
 */
int hooks_run( const char* stack );
void hook_runID( unsigned int id ); /* runs hook of specific id */

/* destroys hooks */
void hook_cleanup (void);


#endif /* HOOK_H */

