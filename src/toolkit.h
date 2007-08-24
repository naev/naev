

#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "opengl.h"
#include "SDL.h"


extern int toolkit;


/*
 * creation
 */
unsigned int window_create( const int x, const int y, const int w, const int h );
void window_addButton( const unsigned int wid,
		const int x, const int y,
		const int w, const int h,
		char* name, char* display,
		void (*call) (char*) );
void window_addText( const unsigned int wid,
		const int x, const int y,
		const int w, const int h,
		char* name, glFont* font, glColour* colour );

/*
 * destruction
 */
void window_destroy( const unsigned int wid );

/*
 * render
 */
void toolkit_render (void);

/*
 * input
 */
void toolkit_mouseEvent( SDL_Event* event );

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);


#endif

