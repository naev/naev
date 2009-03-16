/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua.c
 *
 * @brief Handles creating and setting up basic Lua environments.
 */

#include "nlua.h"

#include "lauxlib.h"

#include "nluadef.h"
#include "log.h"
#include "naev.h"
#include "dialogue.h"
#include "ndata.h"


/*
 * prototypes
 */
static int nlua_packfileLoader( lua_State* L );


/*
 * libraries
 */
/* toolkit */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static const luaL_reg tk_methods[] = {
   { "msg", tk_msg },
   { "yesno", tk_yesno },
   { "input", tk_input },
   {0,0}
}; /**< Toolkit Lua methods. */



/**
 * @brief Wrapper around luaL_newstate.
 *
 *    @return A newly created lua_State.
 */
lua_State *nlua_newState (void)
{
   lua_State *L;

   /* try to create the new state */
   L = luaL_newstate();
   if (L == NULL) {
      WARN("Failed to create new lua state.");
      return NULL;
   }

   return L;
}


/**
 * @brief Opens a lua library.
 *
 *    @param L Lua state to load the library into.
 *    @param f CFunction to load.
 */
int nlua_load( lua_State* L, lua_CFunction f )
{
   lua_pushcfunction(L, f);
   if (lua_pcall(L, 0, 0, 0))
      WARN("nlua include error: %s",lua_tostring(L,1));

   return 0;
}


/**
 * @brief Loads specially modified basic stuff.
 *
 *    @param L Lua State to load the basic stuff into.
 *    @return 0 on success.
 */
int nlua_loadBasic( lua_State* L )
{
   int i;
   const char *override[] = { /* unsafe functions */
         "collectgarbage",
         "dofile",
         "getfenv",
         "getmetatable",
         "load",
         "loadfile",
         "loadstring",
         "rawequal",
         "rawget",
         "rawset",
         "setfenv",
         "setmetatable",
         "END"
   };


   nlua_load(L,luaopen_base); /* open base. */

   /* replace non-safe functions */
   for (i=0; strcmp(override[i],"END")!=0; i++) {
      lua_pushnil(L);
      lua_setglobal(L, override[i]);
   }

   nlua_load(L,luaopen_math); /* open math. */
   nlua_load(L,luaopen_table); /* open table. */

   /* add our own */
   lua_register(L, "include", nlua_packfileLoader);

   return 0;
}


/**
 * @brief include( string module )
 *
 * Loads a module into the current Lua state from inside the data file.
 *
 *    @param module Name of the module to load.
 *    @return An error string on error.
 */
static int nlua_packfileLoader( lua_State* L )
{
   char *buf, *filename;
   uint32_t bufsize;

   NLUA_MIN_ARGS(1);

   if (!lua_isstring(L,1)) {
      NLUA_INVALID_PARAMETER();
      return 0;
   }

   filename = (char*) lua_tostring(L,1);

   /* try to locate the data */
   buf = ndata_read( filename, &bufsize );
   if (buf == NULL) {
      lua_pushfstring(L, "%s not found in ndata.", filename);
      return 1;
   }
   
   /* run the buffer */
   if (luaL_dobuffer(L, buf, bufsize, filename) != 0) {
      /* will push the current error from the dobuffer */
      lua_error(L);
      return 1;
   }

   /* cleanup, success */
   free(buf);
   return 0;
}



/*
 * individual library loading
 */
/**
 * @brief Loads the Toolkit Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int lua_loadTk( lua_State *L )
{
   luaL_register(L, "tk", tk_methods);
   return 0;
}


/**
 * @defgroup TOOLKIT Toolkit Lua Bindings
 *
 * @brief Bindings for interacting with the Toolkit.
 *
 * Functions should be called like:
 *
 * @code
 * tk.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @brief msg( string title, string message )
 *
 * Creates a window with an ok button.
 *
 *    @param title Title of the window.
 *    @param message Message to display in the window.
 */
static int tk_msg( lua_State *L )
{  
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) str = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();
   
   dialogue_msgRaw( title, str );
   return 0;
}
/**
 * @brief bool yesno( string title, string message )
 *
 * Displays a window with Yes and No buttons.
 *
 *    @param title Title of the window.
 *    @param message Message to display in the window.
 *    @return true if yes was clicked, false if no was clicked.
 */
static int tk_yesno( lua_State *L )
{  
   int ret;
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) str = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();
   
   ret = dialogue_YesNoRaw( title, str );
   lua_pushboolean(L,ret);
   return 1;
}
/**
 * @brief string input( string title, number min, number max, string str )
 *
 * Creates a window that allows player to write text input.
 *
 *    @param title Title of the window.
 *    @param min Minimum characters to accept (must be greater then 0).
 *    @param max Maximum characters to accept.
 *    @param str Text to display in the window.
 *    @return nil if input was canceled or a string with the text written.
 */
static int tk_input( lua_State *L )
{  
   char *title, *str, *ret;
   int min, max;
   NLUA_MIN_ARGS(4);

   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2)) min = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,3)) max = (int) lua_tonumber(L,3);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,4)) str = (char*) lua_tostring(L,4);
   else NLUA_INVALID_PARAMETER();
   
   ret = dialogue_inputRaw( title, min, max, str );
   if (ret != NULL) {
      lua_pushstring(L, ret);
      free(ret);
   }
   else
      lua_pushnil(L);
   return 1;
}
/**
 * @}
 */

