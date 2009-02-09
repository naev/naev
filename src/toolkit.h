/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "SDL.h"

#include "tk/widget.h"

#include "opengl.h"
#include "font.h"


extern int toolkit;


/*
 * creation
 */
unsigned int window_create( const char* name,
      const int x, const int y, /* position */
      const int w, const int h ); /* dimensions */
void window_addInput( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int max, const int oneline );
void window_addImageArray( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int iw, const int ih, /* name and image sizes */
      glTexture** tex, char** caption, int nelem, /* elements */
      void (*call) (unsigned int,char*) );


/*
 * modification
 */
/* window */
void window_setAccept( const unsigned int wid, void (*fptr)(unsigned int,char*) );
void window_setCancel( const unsigned int wid, void (*cancel)(unsigned int,char*) );


/*
 * get
 */
/* generic */
int window_exists( const char* wdwname );
int widget_exists( const unsigned int wid, const char* wgtname );
unsigned int window_get( const char* wdwname );
char* window_getInput( const unsigned int wid, char* name );
void window_posWidget( const unsigned int wid,
      char* name, int *x, int *y );
void window_moveWidget( const unsigned int wid,
      char* name, int x, int y );
/* specific */
glTexture* window_getImage( const unsigned int wid, char* name );

/*
 * destruction
 */
void window_close( unsigned int wid, char *str );
void window_destroy( const unsigned int wid );
void window_destroyWidget( unsigned int wid, const char* wgtname );


/*
 * render
 */
void toolkit_render (void);


/*
 * input
 */
int toolkit_input( SDL_Event* event );
void toolkit_update (void);

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);


#endif /* TOOLKIT_H */

