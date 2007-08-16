

#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "opengl.h"


extern int toolkit;


/*
 * creation
 */
unsigned int window_create( int x, int y, int w, int h, gl_texture* t );

/*
 * destruction
 */
void window_destroy( unsigned int wid );

/*
 * render
 */
void toolkit_render (void);

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);


#endif
