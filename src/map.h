/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAP_H
#  define MAP_H


#include "space.h"


/* open the map window */
void map_open (void);

/* misc */
void map_clear (void);
void map_jump (void);

/* manipulate universe stuff */
StarSystem** map_getJumpPath( int* njumps, char* sysstart, char* sysend );


#endif /* MAP_H */
