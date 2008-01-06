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
		char* name, glTexture* image, int border ); /* label and image itself */
void window_addList( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		char* name, char **items, int nitems, int defitem,
		void (*call) (char*) );
void window_addRect( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		char* name, glColour* colour, int border ); /* propertiers */
void window_addCust( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		char* name, const int border,
		void (*render) (double x, double y, double w, double h),
		void (*mouse) (SDL_Event* event, double x, double y) );
void window_addInput( const unsigned int wid,
		const int x, const int y, /* position */
		const int w, const int h, /* size */
		char* name, const int max, const int oneline );



/*
 * popups and alerts
 */
void toolkit_alert( const char *fmt, ... );
int toolkit_YesNo( char *caption, const char *fmt, ... );


/*
 * modification
 */
void window_setFptr( const unsigned int wid, void (*fptr)( char* ) );
void window_modifyText( const unsigned int wid,
		char* name, char* newstring );
void window_modifyImage( const unsigned int wid,
		char* name, glTexture* image );


/*
 * get
 */
int window_exists( const char* wdwname );
unsigned int window_get( const char* wdwname );
char* toolkit_getList( const unsigned int wid, char* name );
glTexture* window_getImage( const unsigned int wid, char* name );
char* window_getInput( const unsigned int wid, char* name );


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
int toolkit_input( SDL_Event* event );
void toolkit_update (void);

/*
 * init/exit
 */
int toolkit_init (void);
void toolkit_exit (void);


#endif

