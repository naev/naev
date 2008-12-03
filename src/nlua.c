/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua.c
 *
 * @brief Contains some standard Lua binding libraries.
 *
 * Namely:
 *    - naev : General game stuff.
 *    - time : For manipulating time.
 *    - rnd : For access to the random number generator.
 *    - tk : Access to the toolkit.
 */

#include "nlua.h"

#include "lauxlib.h"

#include "log.h"
#include "naev.h"
#include "rng.h"
#include "ntime.h"
#include "dialogue.h"
#include "space.h"
#include "land.h"
#include "nluadef.h"
#include "map.h"
#include "ndata.h"


/*
 * prototypes
 */
static int nlua_packfileLoader( lua_State* L );


/*
 * libraries
 */
/* naev */
static int naev_lang( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
   {0,0}
}; /**< NAEV Lua methods. */
/* time */
static int time_get( lua_State *L );
static int time_str( lua_State *L );
static int time_units( lua_State *L );
static const luaL_reg time_methods[] = {
   { "get", time_get },
   { "str", time_str },
   { "units", time_units },
   {0,0}                                                                  
}; /**< Time Lua methods. */
/* rnd */
static int rnd_int( lua_State *L );
static const luaL_reg rnd_methods[] = {
   { "int", rnd_int },
   {0,0}
}; /**< Random Lua methods. */
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
 * @fn lua_State *nlua_newState (void)
 *
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
 * @fn int nlua_load( lua_State* L, lua_CFunction f )
 *
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
 * @fn int nlua_loadBasic( lua_State* L )
 *
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

   /* add our own */
   lua_register(L, "include", nlua_packfileLoader);

   return 0;
}


/**
 * @fn static int nlua_packfileLoader( lua_State* L )
 *
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
 * @brief Loads the NAEV Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int lua_loadNaev( lua_State *L )
{  
   luaL_register(L, "naev", naev_methods);
   return 0;
}
/**
 * @brief Loads the Time Lua library.
 *
 *    @param L Lua state.
 *    @param readonly Whether to open it as read only.
 *    @return 0 on success.
 */
int lua_loadTime( lua_State *L, int readonly )
{
   (void)readonly;
   luaL_register(L, "time", time_methods);
   return 0;
}
/**
 * @brief Loads the Random Number Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int lua_loadRnd( lua_State *L )
{
   luaL_register(L, "rnd", rnd_methods);
   return 0;
}
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
 * @defgroup NAEV NAEV Generic Lua Bindings
 *
 * @brief Bindings for interacting with general NAEV stuff.
 *
 * Functions should be called like:
 *
 * @code
 * naev.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int naev_lang( lua_State *L )
 *
 * @brief string lang( nil )
 *
 * Gets the language NAEV is currently using.
 *
 *    @return Two character identifier of the language.
 */
static int naev_lang( lua_State *L )
{  
   /** @todo multilanguage stuff */
   lua_pushstring(L,"en");
   return 1;
}
/**
 * @}
 */


/**
 * @defgroup TIME Time Lua Bindings
 *
 * @brief Bindings for interacting with the time.
 *
 * Functions should be called like:
 *
 * @code
 * time.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int time_get( lua_State *L )
 *
 * @brief number get( nil )
 *
 * Gets the current time in internal representation time.
 *
 *    @return Time in internal representation time.
 */
static int time_get( lua_State *L )
{
   lua_pushnumber( L, ntime_get() );
   return 1;
}
/**
 * @fn static int time_str( lua_State *L )
 *
 * @brief string str( [number t] )
 *
 * Converts the time to a pretty human readable format.
 *
 *    @param t Time to convert to pretty format.  If ommitted, current time is
 *              used.
 *    @return The time in human readable format.
 */
static int time_str( lua_State *L )
{
   char *nt;
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
      nt = ntime_pretty( (unsigned int) lua_tonumber(L,1) );
   else
      nt = ntime_pretty( ntime_get() );
   lua_pushstring(L, nt);
   free(nt);
   return 1;
}
/**
 * @fn static int time_units( lua_State *L )
 *
 * @brief number units( [number stu] ) 
 *
 * Converts stu to internal representation time.
 *
 *    @param stu Time in stu to convert to internal representation time.  If
 *                ommitted, 1 stu is used.
 *    @return The value of stu in internal representation time.
 */
static int time_units( lua_State *L )
{  
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
      lua_pushnumber( L, (unsigned int)lua_tonumber(L,1) * NTIME_UNIT_LENGTH );
   else
      lua_pushnumber( L, NTIME_UNIT_LENGTH );
   return 1;
}
/**
 * @}
 */



/**
 * @defgroup RND Random Number Lua Bindings
 *
 * @brief Bindings for interacting with the random number generator.
 *
 * Functions should be called like:
 *
 * @code
 * rnd.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int rnd_int( lua_State *L )
 *
 * @brief number int( [number x, number y] )
 *
 * Gets a random number.  With no parameters it returns a random float between
 *  0 and 1 (yes I know name is misleading).  With one parameter it returns a
 *  whole number between 0 and that number (both included).  With two
 *  parameters it returns a whole number between both parameters (both
 *  included).
 *
 *    @param x First parameter, read description for details.
 *    @param y Second parameter, read description for details.
 *    @return A randomly generated number, read description for details.
 */
static int rnd_int( lua_State *L )
{  
   int o;
   
   o = lua_gettop(L);
   
   if (o==0) lua_pushnumber(L, RNGF() ); /* random double 0 <= x <= 1 */
   else if (o==1) { /* random int 0 <= x <= parameter */
      if (lua_isnumber(L, 1))
         lua_pushnumber(L, RNG(0, (int)lua_tonumber(L, 1)));
      else return 0;
   }
   else if (o>=2) { /* random int paramater 1 <= x <= parameter 2 */
      if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
         lua_pushnumber(L,
               RNG((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2)));
      else return 0;
   }
   else NLUA_INVALID_PARAMETER();
   
   return 1; /* unless it's returned 0 already it'll always return a parameter */
}
/**
 * @}
 */



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
 * @fn static int tk_msg( lua_State *L )
 *
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
   
   dialogue_msg( title, str );
   return 0;
}
/**
 * @fn static int tk_yesno( lua_State *L )
 *
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
   
   ret = dialogue_YesNo( title, str );
   lua_pushboolean(L,ret);
   return 1;
}
/**
 * @fn static int tk_input( lua_State *L )
 *
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
   
   ret = dialogue_input( title, min, max, str );
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

