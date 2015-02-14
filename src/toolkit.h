/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "SDL.h"

#include "tk/widget.h"

#include "opengl.h"
#include "font.h"


int toolkit_isOpen (void);


/*
 * creation
 */
unsigned int window_create( const char* name,
      const int x, const int y, /* position */
      const int w, const int h ); /* dimensions */

/*
 * modification
 */
/* window */
void window_setAccept( const unsigned int wid, void (*fptr)(unsigned int,char*) );
void window_setCancel( const unsigned int wid, void (*cancel)(unsigned int,char*) );
void window_setBorder( unsigned int wid, int enable );
void window_setFocus( const unsigned int wid, const char* wgtname );
void window_handleKeys( const unsigned int wid,
      int (*keyhandler)(unsigned int,SDLKey,SDLMod) );
void window_handleEvents( const unsigned int wid,
      int (*eventhandler)(unsigned int,SDL_Event*) );


/*
 * get
 */
/* generic */
int window_exists( const char* wdwname );
int window_existsID( const unsigned int wid );
int widget_exists( const unsigned int wid, const char* wgtname );
unsigned int window_get( const char* wdwname );
void window_dimWindow( const unsigned int wid, int *w, int *h );
char* window_getFocus( const unsigned int wid );
void window_posWidget( const unsigned int wid,
      char* name, int *x, int *y );
void window_moveWidget( const unsigned int wid,
      char* name, int x, int y );

/*
 * destruction
 */
void window_close( unsigned int wid, char *str );
void window_destroy( const unsigned int wid );
void window_destroyWidget( unsigned int wid, const char* wgtname );
void window_setParent( unsigned int wid, unsigned int parent );
unsigned int window_getParent( unsigned int wid );
void window_onClose( unsigned int wid, void (*fptr)(unsigned int,char*) );


/*
 * data
 */
void window_setData( unsigned int wid, void *data );
void* window_getData( unsigned int wid );


/*
 * render
 */
void toolkit_render (void);


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
void toolkit_delay (void);


#endif /* TOOLKIT_H */

