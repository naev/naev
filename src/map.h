/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAP_H
#  define MAP_H


#include "space.h"


/* init/exit */
int map_init (void);
void map_exit (void);

/* open the map window */
void map_open (void);
void map_close (void);
int map_isOpen (void);

/* misc */
void map_setZoom( double zoom );
void map_select( StarSystem *sys, char shifted );
void map_cleanup (void);
void map_clear (void);
void map_jump (void);

/* manipulate universe stuff */
StarSystem** map_getJumpPath( int* njumps, const char* sysstart,
     const char* sysend, int ignore_known, StarSystem** old_data );
int map_map( const char* targ_sys, int r );
int map_isMapped( const char* targ_sys, int r );

/* shows a map at x, y (relative to wid) with size w,h  */
void map_show( int wid, int x, int y, int w, int h, double zoom );
int map_center( const char *sys );

/* Internal rendering sort of stuff. */
void map_renderParams( double bx, double by, double xpos, double ypos,
      double w, double h, double zoom, double *x, double *y, double *r );
void map_renderSystems( double bx, double by, double x, double y, double w, double h, double r, int editor );
void map_renderNames( double x, double y, int editor );


#endif /* MAP_H */

