

#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "SDL.h"

#include "opengl.h"
#include "font.h"


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
		const int w, const int centered, char* name,
		glFont* font, glColour* colour, char* string);
void window_addImage( const unsigned int wid,
		const int x, const int y,
		char* name, glTexture* image );


/*
 * destruction
 */
void window_destroy( const unsigned int wid );
void window_destroyWidget( unsigned int wid, const char* wgtname );

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

