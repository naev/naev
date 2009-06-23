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
char *osd_getTitle( unsigned int osd );
char **osd_getItems( unsigned int osd, int *nitems );


/*
 * Subsystem usage.
 */
void osd_exit (void);
void osd_render( double x, double y, double w, double h );


#endif /* OSD_H */
