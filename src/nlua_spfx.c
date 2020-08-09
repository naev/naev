/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_spfx.c
 *
 * @brief Bindings for Special effects functionality from Lua.
 */

#include "nlua_spfx.h"

#include "naev.h"

#include "nstring.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "sound.h"
#include "nlua_vec2.h"


/* Camera methods. */
static int spfxL_soundPlay( lua_State *L );
static const luaL_Reg spfxL_methods[] = {
   { "soundPlay", spfxL_soundPlay },
   {0,0}
}; /**< Spfx Lua methods. */




/**
 * @brief Loads the spfx library.
 *
 *    @param L State to load camera library into.
 *    @return 0 on success.
 */
int nlua_loadSpfx( nlua_env env )
{
   nlua_register(env, "spfx", spfxL_methods, 0);
   return 0;
}


/**
 * @brief Lua bindings to interact with the Special effects.
 *
 * An example would be:
 * @code
 * @endcode
 *
 * @luamod spfx
 */


/**
 * @brief Plays a sound.
 *
 * by default, the sound is played at player's current position
 *
 * @usage spfx.soundPlay( "hail" ) -- Plays the hail sound
 * @usage spfx.soundPlay( "hail", pos ) -- Plays the hail sound at position pos
 * @usage spfx.soundPlay( "hail", pos, vel ) -- Plays the hail sound at position pos with velocity vel
 *
 *    @luatparam string s Name of the sound to play
 *    @luatparam [opt] Vec2 pos Position of the source
 *    @luatparam [opt] Vec2 vel Velocity of the source
 * @luafunc soundPlay( s )
 */
static int spfxL_soundPlay( lua_State *L )
{
   const char *name;
   char buf[PATH_MAX];
   Vector2d *pos, *vel, vel0;
   int dopos;

   /* Flag wether to use sound_playPos or sound_play. */
   dopos = 0;

   /* Handle parameters. */
   name = luaL_checkstring(L,1);
   if (lua_gettop(L) > 1) {
      dopos = 1;
      pos = luaL_checkvector(L,2);
      if (lua_gettop(L) > 2) {
         vel = luaL_checkvector(L,3);
      }
      else {
         vectnull( &vel0 );
         vel = &vel0;
      }
   }

   nsnprintf(buf, PATH_MAX, "%s", name);

   if (dopos)
      sound_playPos( sound_get(buf), pos->x, pos->y, vel->x, vel->y );
   else
      sound_play( sound_get(buf) );

   return 0;
}

