/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef OSD_H
#  define OSD_H


/*
 * OSD usage.
 */
unsigned int osd_create( const char *title, int nitems, const char **items );
int osd_destroy( unsigned int osd );
int osd_active( unsigned int osd, int msg );
int osd_getActive( unsigned int osd );
char *osd_getTitle( unsigned int osd );
char **osd_getItems( unsigned int osd, int *nitems );


/*
 * Subsystem usage.
 */
int osd_setup( int x, int y, int w, int h );
void osd_exit (void);
void osd_render (void);


#endif /* OSD_H */
