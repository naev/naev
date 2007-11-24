/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef TOOLKIT_H
#  define TOOLKIT_H


#include "SDL.h"

#include "opengl.h"
#include "font.h"


extern int toolkit;


/*
 * creation
 */
unsigned int window_create( char* name,
		const int x, const int y, /* position */
		const int w, const int h ); /* dimensions */
void window_addButton( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		char* name, char* display, /* label name, display name */
		void (*call) (char*) ); /* function to call when clicked */
void window_addText( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		const int centered, char* name, /* text is centered? label name */
		glFont* font, glColour* colour, char* string ); /* font, colour and actual text */
void window_addImage( const unsigned int wid,
		const int x, const int y, /* position */
		char* name, glTexture* image ); /* label and image itself */

/*
 * modification
 */
void window_modifyText( const unsigned int wid,
		char* name, char* newstring );


/*
 * get
 */
int window_exists( const char* wdwname );
unsigned int window_get( const char* wdwname );

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
void toolkit_input( SDL_Event* event );

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);


#endif

