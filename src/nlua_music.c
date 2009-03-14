/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_music.c
 *
 * @brief Lua music playing module.
 */


#include "nlua_music.h"

#include "SDL.h"

#include "nlua.h"
#include "nluadef.h"
#include "music.h"
#include "naev.h"
#include "log.h"
#include "ndata.h"


/* functions */
static int musicL_load( lua_State* L );
static int musicL_play( lua_State* L );
static int musicL_stop( lua_State* L );
static int musicL_isPlaying( lua_State* L );
static int musicL_current( lua_State* L );
static const luaL_reg music_methods[] = {
   { "load", musicL_load },
   { "play", musicL_play },
   { "stop", musicL_stop },
   { "isPlaying", musicL_isPlaying },
   { "current", musicL_current },
   {0,0}
}; /**< Music specific methods. */


/**
 * @brief Loads the music functions into a lua_State.
 *
 *    @param L Lua State to load the music functions into.
 *    @param read_only Load the write functions?
 *    @return 0 on success.
 */
int lua_loadMusic( lua_State *L, int read_only )
{
   (void)read_only; /* future proof */
   luaL_register(L, "music", music_methods);
   return 0;
}


/*
 * the music lua functions
 */
static int musicL_load( lua_State *L )
{
   char* str;

   /* check parameters */
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L,1)) str = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   music_load( str );
   return 0;
}
static int musicL_play( lua_State *L )
{
   (void)L;
   music_play();
   return 0;
}
static int musicL_stop( lua_State *L )
{
   (void)L;
   music_stop();
   return 0;
}
static int musicL_isPlaying( lua_State* L )
{
   lua_pushboolean(L, music_isPlaying());
   return 1;
}
static int musicL_current( lua_State* L )
{
   const char *music_name;
   
   music_name = music_playingName();

   lua_pushstring(L, (music_name != NULL) ? music_name : "none" );
   return 1;
}



