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


/*
 * Subsystem usage.
 */
void osd_exit (void);
void osd_render( double x, double y, double w, double h );


#endif /* OSD_H */
