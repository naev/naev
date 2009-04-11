/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file console.c
 *
 * @brief Handles the Lua console.
 */

#include "console.h"

#include "naev.h"

#include <string.h>

#define lua_c
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "log.h"
#include "nlua.h"
#include "nlua_cli.h"
#include "font.h"
#include "toolkit.h"


#define CONSOLE_FONT_SIZE  10 /**< Size of the console font. */


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
static int cli_history     = 0; /**< Position in history. */
static int cli_width       = 0; /**< Console width. */
static int cli_height      = 0; /**< Console height. */


/*
 * Input handling.
 */
static int cli_firstline   = 1; /**< Is this the first line? */


/*
 * CLI stuff.
 */
static int cli_print( lua_State *L );
static int cli_script( lua_State *L );
static const luaL_Reg cli_methods[] = {
   { "print", cli_print },
   { "script", cli_script },
   {NULL, NULL}
}; /**< Console only functions. */



/*
 * Prototypes.
 */
static int cli_keyhandler( unsigned int wid, SDLKey key, SDLMod mod );
static void cli_addMessage( const char *msg );
static void cli_render( double bx, double by, double w, double h );


/**
 * @brief Replacement for the internal Lua print to print to console instead of terminal.
 */
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
      p += snprintf( &buf[p], LINE_LENGTH-p, "%s%s", (i>1) ? "   " : "", s );
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
 * @brief Would be like "dofile" from the base Lua lib.
 */
static int cli_script( lua_State *L )
{
   const char *fname = luaL_optstring(L, 1, NULL);
   int n = lua_gettop(L);
   if (luaL_loadfile(L, fname) != 0) lua_error(L);
   lua_call(L, 0, LUA_MULTRET);
   return lua_gettop(L) - n;
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
   cli_history = cli_cursor; /* History matches cursor. */

   /* Move viewport if needed. */
   n = (cli_cursor - cli_viewport) % BUF_LINES;
   if ((n+1)*(cli_font->h+5) > cli_height-80-BUTTON_HEIGHT)
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
 * @brief Key handler for the console window.
 */
static int cli_keyhandler( unsigned int wid, SDLKey key, SDLMod mod )
{
   (void) mod;
   int i;

   switch (key) {

      /* Go up in history. */
      case SDLK_UP:
         i = cli_history-1;
         while (cli_buffer[i][0] != '\0') {
            if (cli_buffer[i][0] == '>') {
               window_setInput( wid, "inpInput", &cli_buffer[i][3] );
               cli_history = i;
               return 1;
            }
            i--;
         }
         return 1;

      /* Go down in history. */
      case SDLK_DOWN:
         /* Clears buffer. */
         if (cli_history >= cli_cursor) {
            window_setInput( wid, "inpInput", NULL );
            return 1;
         }

         /* Find next buffer. */
         i = cli_history+1;
         while (cli_buffer[i][0] != '\0') {
            if (cli_buffer[i][0] == '>') {
               window_setInput( wid, "inpInput", &cli_buffer[i][3] );
               cli_history = i;
               return 1;
            }
            i++;
         }
         window_setInput( wid, "inpInput", NULL );
         return 1;

      default:
         break;
   }

   return 0;
}


/**
 * @brief Initializes the CLI environment.
 */
int cli_init (void)
{
   /* Already loaded. */
   if (cli_state != NULL)
      return 0;

   /* Calculate size. */
   cli_width  = SCREEN_W - 100;
   cli_height = SCREEN_H - 100;

   /* Create the state. */
   cli_state = nlua_newState();
   nlua_loadBasic( cli_state );
   nlua_loadStandard( cli_state, 0 );
   nlua_loadCLI( cli_state );
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

   /* Must not be already open. */
   if (window_exists( "Lua Console" ))
      return;

   /* Create the window. */
   wid = window_create( "Lua Console", -1, -1, cli_width, cli_height );

   /* Window settings. */
   window_setAccept( wid, cli_input );
   window_setCancel( wid, window_close );
   window_handleKeys( wid, cli_keyhandler );

   /* Input box. */
   window_addInput( wid, 20, 20,
         cli_width-60-BUTTON_WIDTH, BUTTON_HEIGHT,
         "inpInput", LINE_LENGTH, 1 );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Custom console widget. */
   window_addCust( wid, 20, -40,
         cli_width-40, cli_height-80-BUTTON_HEIGHT,
         "cstConsole", 0, cli_render, NULL );
}


