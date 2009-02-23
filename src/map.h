/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAP_H
#  define MAP_H


#include "space.h"


/* open the map window */
void map_open (void);
void map_close (void);
int map_isOpen (void);

/* misc */
void map_select( StarSystem *sys );
void map_clear (void);
void map_jump (void);

/* manipulate universe stuff */
StarSystem** map_getJumpPath( int* njumps, char* sysstart, char* sysend, int ignore_known );
int map_map( char* targ_sys, int r );
int map_isMapped( char* targ_sys, int r );

/* shows a map at x, y (relative to wid) with size w,h  */
void map_show( int wid, int x, int y, int w, int h );



#endif /* MAP_H */
