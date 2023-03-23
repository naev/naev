/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/*
 * OSD usage.
 */
unsigned int osd_create( const char *title,
      int nitems, const char **items, int priority );
int osd_destroy( unsigned int osd );
int osd_active( unsigned int osd, int msg );
int osd_getActive( unsigned int osd );
char *osd_getTitle( unsigned int osd );
char **osd_getItems( unsigned int osd );
int osd_setHide( unsigned int osd, int state );
int osd_getHide( unsigned int osd );

/*
 * Subsystem usage.
 */
int osd_setup( int x, int y, int w, int h );
void osd_exit (void);
void osd_render (void);
