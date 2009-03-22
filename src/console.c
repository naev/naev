/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file console.c
 *
 * @brief Handles the Lua console.
 */

#include "console.h"

#include <string.h>

#include "lauxlib.h"

#include "naev.h"
#include "log.h"
#include "nlua.h"
#include "font.h"
#include "toolkit.h"


#define CONSOLE_WIDTH   500 /**< Console window width. */
#define CONSOLE_HEIGHT  400 /**< Console window height. */

#define BUTTON_WIDTH    50 /**< Button width. */
#define BUTTON_HEIGHT   20 /**< Button height. */


/*
 * Global stuff.
 */
static lua_State *cli_state = NULL; /**< Lua CLI state. */
static glFont *cli_font     = NULL; /**< CLI font to use. */

/*
 * Buffers.
 */
#define BUF_LINES          256 /**< Number of lines in the buffer. */
#define LINE_LENGTH        80 /**< Length of lines in the buffer. */
static int cli_cursor      = 0; /**< Current cursor position. */
static char cli_buffer[BUF_LINES][LINE_LENGTH]; /**< CLI buffer. */


/*
 * Prototypes.
 */
static void cli_addMessage( const char *msg );
static void cli_render( double bx, double by, double w, double h );


/**
 * @brief Adds a message to the buffer.
 *
 *    @param msg Message to add.
 */
static void cli_addMessage( const char *msg )
{
   strncpy( cli_buffer[cli_cursor], msg, LINE_LENGTH );
   cli_cursor = (cli_cursor+1) % BUF_LINES;
}


/**
 * @brief Render function for the custom widget.
 */
static void cli_render( double bx, double by, double w, double h )
{
   int i, y;

   /* Draw the text. */
   i = 0;
   for (y=h-cli_font->h-5; y>0; y -= cli_font->h + 5) {
      gl_printMaxRaw( cli_font, w,
            bx + SCREEN_W/2., by + y + SCREEN_H/2., 
            &cBlack, cli_buffer[i] );
      i++;
   }
}


/**
 * @brief Initializes the CLI environment.
 */
int cli_init (void)
{
   /* Already loaded. */
   if (cli_state != NULL)
      return 0;

   /* Create the state. */
   cli_state = nlua_newState();
   nlua_loadBasic( cli_state );
   nlua_loadStandard( cli_state, 0 );

   /* Set the font. */
   cli_font = &gl_smallFont;

   /* Clear the buffer. */
   memset( cli_buffer, 0, sizeof(cli_buffer) );

   /* Put a friendly message at first. */
   cli_addMessage( "Welcome to the Lua console!" );
   cli_addMessage( "" );

   return 0;
}


/**
 * @brief Destroys the CLI environment.
 */
void cli_exit (void)
{
   /* Destroy the state. */
   if (cli_state != NULL) {
      lua_close( cli_state );
      cli_state = NULL;
   }

   /* Free the font. */
   /*if (cli_font != NULL) {
      gl_freeFont( cli_font );
      cli_font = NULL;
   }*/
}


/**
 * @brief Handles the CLI input.
 *
 *    @param wid Window recieving the input.
 *    @param unused Unused.
 */
static void cli_input( unsigned int wid, char *unused )
{
   (void) unused;
   char *str;

   /* Get the input. */
   str = window_getInput( wid, "inpInput" );

   /* Clear the box now. */
   window_setInput( wid, "inpInput", NULL );
}


/**
 * @brief Opens the console.
 */
void cli_open (void)
{
   unsigned int wid;

   /* Lazy loading. */
   if (cli_state == NULL)
      if (cli_init())
         return;

   /* Create the window. */
   wid = window_create( "Lua Console", -1, -1,
         CONSOLE_WIDTH, CONSOLE_HEIGHT );

   /* Window settings. */
   window_setAccept( wid, cli_input );
   window_setCancel( wid, window_close );

   /* Input box. */
   window_addInput( wid, 20, 20,
         CONSOLE_WIDTH-60-BUTTON_WIDTH, BUTTON_HEIGHT,
         "inpInput", LINE_LENGTH, 1 );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Custom console widget. */
   window_addCust( wid, 20, -40,
         CONSOLE_WIDTH-40, CONSOLE_HEIGHT-80-BUTTON_HEIGHT,
         "cstConsole", 0, cli_render, NULL );
}


