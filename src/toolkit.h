/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "SDL.h"

#include "tk/widget/button.h"
#include "tk/widget/text.h"

#include "opengl.h"
#include "font.h"


extern int toolkit;


/*
 * creation
 */
unsigned int window_create( const char* name,
      const int x, const int y, /* position */
      const int w, const int h ); /* dimensions */
void window_addText( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      const int centered, char* name, /* text is centered? label name */
      glFont* font, glColour* colour, const char* string ); /* font, colour and actual text */
void window_addImage( const unsigned int wid,
      const int x, const int y, /* position */
      char* name, glTexture* image, int border ); /* label and image itself */
void window_addList( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, char **items, int nitems, int defitem,
      void (*call) (unsigned int,char*) );
void window_addRect( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, glColour* colour, int border ); /* properties */
void window_addCust( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int border,
      void (*render) (double x, double y, double w, double h),
      void (*mouse) (unsigned int wid, SDL_Event* event, double x, double y) );
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
void window_addFader( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size, if w > h fader is horizontal, else vertical */
      char* name, const double min, const double max, /* name, minimum & maximum values */
      const double def, /* default pos. */
      void (*call) (unsigned int,char*) ); /* function to called on value change */


/*
 * modification
 */
/* window */
void window_setAccept( const unsigned int wid, void (*fptr)(unsigned int,char*) );
void window_setCancel( const unsigned int wid, void (*cancel)(unsigned int,char*) );
/* image */
void window_modifyImage( const unsigned int wid,
      char* name, glTexture* image );
void window_imgColour( const unsigned int wid,
      char* name, glColour* colour );
/* fader */
void window_faderValue( const unsigned int wid,
      char* name, double value );
void window_faderBounds( const unsigned int wid,
      char* name, double min, double max );


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
char* toolkit_getList( const unsigned int wid, char* name );
int toolkit_getListPos( const unsigned int wid, char* name );
glTexture* window_getImage( const unsigned int wid, char* name );
double window_getFaderValue(const unsigned int wid, char* name);

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

