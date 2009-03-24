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

#define lua_c
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "naev.h"
#include "log.h"
#include "nlua.h"
#include "font.h"
#include "toolkit.h"


#define CONSOLE_FONT_SIZE  10


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
static int cli_viewport    = 0; /**< Current viewport. */


/*
 * Input handling.
 */
static int cli_firstline   = 1; /**< Is this the first line? */


/*
 * CLI stuff.
 */
static int cli_print( lua_State *L );
static const luaL_Reg cli_methods[] = {
   { "print", cli_print },
   {NULL, NULL}
};



/*
 * Prototypes.
 */
static void cli_addMessage( const char *msg );
static void cli_render( double bx, double by, double w, double h );


static int cli_print( lua_State *L ) {
   int n = lua_gettop(L);  /* number of arguments */
   int i;
   char buf[LINE_LENGTH];
   int p;
   p = 0;
   lua_getglobal(L, "tostring");
   for (i=1; i<=n; i++) {
      const char *s;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tostring(L, -1);  /* get result */
      if (s == NULL)                                                         
         return luaL_error(L, LUA_QL("tostring") " must return a string to "
               LUA_QL("print"));

      /* Add to console. */
      p += snprintf( buf, LINE_LENGTH-p, "%s%s", (i>1) ? "   " : "", s );
      if (p >= LINE_LENGTH) {
         cli_addMessage(buf);
         p = 0;
      }
      lua_pop(L, 1);  /* pop result */
   }

   /* Add last line if needed. */
   cli_addMessage(buf);

   return 0;
}


/**
 * @brief Adds a message to the buffer.
 *
 *    @param msg Message to add.
 */
static void cli_addMessage( const char *msg )
{
   int n;

   if (msg != NULL)
      strncpy( cli_buffer[cli_cursor], msg, LINE_LENGTH );
   else
      cli_buffer[cli_cursor][0] = '\0';

   cli_cursor = (cli_cursor+1) % BUF_LINES;

   /* Move viewport if needed. */
   n = (cli_cursor - cli_viewport) % BUF_LINES;
   if ((n+1)*(cli_font->h+5) > CONSOLE_HEIGHT-80-BUTTON_HEIGHT)
      cli_viewport = (cli_viewport+1) % BUF_LINES;
}


/**
 * @brief Render function for the custom widget.
 */
static void cli_render( double bx, double by, double w, double h )
{
   int i, y;
   glColour *c;

   /* Draw the text. */
   i = cli_viewport;
   for (y=h-cli_font->h-5; y>0; y -= cli_font->h + 5) {
      if (cli_buffer[i][0] == '>')
         c = &cDConsole;
      else if (strncmp(cli_buffer[i], "cli:", 4)==0)
         c = &cRed;
      else
         c = &cBlack;
      gl_printMaxRaw( cli_font, w,
            bx + SCREEN_W/2., by + y + SCREEN_H/2., 
            c, cli_buffer[i] );
      i = (i + 1)  % BUF_LINES;
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
   luaL_register( cli_state, "_G", cli_methods );
   lua_settop( cli_state, 0 );

   /* Set the font. */
   cli_font = malloc( sizeof(glFont) );
   gl_fontInit( cli_font, "dat/mono.ttf", CONSOLE_FONT_SIZE );

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
   if (cli_font != NULL) {
      gl_freeFont( cli_font );
      free( cli_font );
      cli_font = NULL;
   }
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
   int status;
   char *str;
   lua_State *L;
   char buf[LINE_LENGTH];

   /* Get the input. */
   str = window_getInput( wid, "inpInput" );

   /* Ignore useless stuff. */
   if ((str == NULL) || (str[0] == '\0'))
      return;

   /* Put the message in the console. */
   snprintf( buf, LINE_LENGTH, "%s %s",
         cli_firstline ? "> " : ">>", str );
   cli_addMessage( buf );

   /* Set up state. */
   L = cli_state;
   /* Load the string. */
   lua_pushstring( L, str );
   /* Concat string if needed. */
   if (!cli_firstline) {
      lua_pushliteral(L, "\n");  /* add a new line... */
      lua_insert(L, -2);  /* ...between the two lines */
      lua_concat(L, 3);  /* join them */
   }
   status = luaL_loadbuffer( L, lua_tostring(L,-1), lua_strlen(L,-1), "=cli" );
   /* String isn't proper Lua yet. */
   if (status == LUA_ERRSYNTAX) {
      size_t lmsg;
      const char *msg = lua_tolstring(L, -1, &lmsg);
      const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
      if (strstr(msg, LUA_QL("<eof>")) == tp) {
         /* Pop the loaded buffer. */
         lua_pop(L, 1);
         cli_firstline = 0;
      }
      else {
         /* Real error, spew message and break. */
         cli_addMessage( lua_tostring(L, -1) );
         lua_settop(L, 0);
         cli_firstline = 1;
      }
   }
   /* Print results - all went well. */
   else if (status == 0) {
      if (lua_pcall(L, 0, LUA_MULTRET, 0))
         cli_addMessage( lua_tostring(L, -1) );
      /* Clear stack. */
      lua_settop(L, 0);
      cli_firstline = 1;
   }

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


