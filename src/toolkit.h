/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
/** @endcond */

#include "font.h"
#include "opengl.h"
#include "tk/widget.h"

int toolkit_isOpen (void);

/*
 * creation
 */
unsigned int window_create( const char* name, const char *displayname,
      const int x, const int y, /* position */
      const int w, const int h ); /* dimensions */
unsigned int window_createFlags( const char* name, const char *displayname,
      const int x, const int y, /* position */
      const int w, const int h, unsigned int flags ); /* dimensions */

/*
 * modification
 */
/* window */
void window_setAccept( unsigned int wid, void (*fptr)(unsigned int,const char*) );
void window_setCancel( unsigned int wid, void (*cancel)(unsigned int,const char*) );
void window_setBorder( unsigned int wid, int enable );
void window_setFocus( unsigned int wid, const char* wgtname );
void window_handleKeys( unsigned int wid,
      int (*keyhandler)(unsigned int,SDL_Keycode,SDL_Keymod,int) );
void window_handleEvents( unsigned int wid,
      int (*eventhandler)(unsigned int,SDL_Event*) );
void window_move( unsigned int wid, int x, int y );
void window_resize( unsigned int wid, int w, int h );
void window_raise( unsigned int wid );
void window_lower( unsigned int wid );
int window_setDisplayname( unsigned int wid, const char *displayname );
void window_setDynamic( unsigned int wid, int dynamic );

/*
 * get
 */
/* generic */
int window_exists( const char* wdwname );
int window_existsID( unsigned int wid );
int widget_exists( unsigned int wid, const char* wgtname );
unsigned int window_get( const char* wdwname );
void window_dimWindow( unsigned int wid, int *w, int *h );
void window_posWindow( unsigned int wid, int *x, int *y );
void window_dimWidget( unsigned int wid, const char *name, int *w, int *h );
char* window_getFocus( unsigned int wid );
void window_posWidget( unsigned int wid, const char* name, int *x, int *y );
void window_moveWidget( unsigned int wid, const char* name, int x, int y );
void window_resizeWidget( unsigned int wid, const char* name, int w, int h );
void window_canFocusWidget( unsigned int wid, const char* name, int canfocus );
int window_isTop( unsigned int wid );
int widget_isCovered( unsigned int wid, const char *name, int x, int y );

/*
 * destruction
 */
void toolkit_closeAll (void);
void window_close( unsigned int wid, const char* str );
void window_destroy( unsigned int wid );
void window_destroyWidget( unsigned int wid, const char* wgtname );
void window_setParent( unsigned int wid, unsigned int parent );
unsigned int window_getParent( unsigned int wid );
void window_onClose( unsigned int wid, void (*fptr)(unsigned int,const char*) );
void window_onCleanup( unsigned int wid, void (*fptr)(unsigned int,const char*) );

/*
 * data
 */
void window_setData( unsigned int wid, void *data );
void* window_getData( unsigned int wid );

/*
 * render
 */
void toolkit_rerender (void);
void toolkit_render( double dt );

/*
 * input
 */
int toolkit_input( SDL_Event* event );
void toolkit_update (void);
void toolkit_clearKey (void);

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);

/*
 * hacks
 */
void toolkit_resize (void);
void toolkit_delay (void);
