

#ifndef SPACE_H
#  define SPACE_H


#define MIN_HYPERSPACE_DIST	1500

/*
 * loading/exiting
 */
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * render
 */
void space_render( double dt );
void planets_render (void);


#endif /* SPACE_H */
