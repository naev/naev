/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef HOOK_H
#  define HOOK_H


#include "mission.h"


/* add/run hooks */
int hook_add( Mission* misn, char *func, char *stack );
void hook_rm( int id );
void hook_rmParent( unsigned int parent );

/* run hooks */
int hooks_run( char* stack );

/* destroys hooks */
void hook_cleanup (void);


#endif /* HOOK_H */

