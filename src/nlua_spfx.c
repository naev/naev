/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_spfx.c
 *
 * @brief Bindings for Special effects functionality from Lua.
 */
/** @cond */
#include <lauxlib.h>
#include "physfsrwops.h"

#include "naev.h"
/** @endcond */

#include "nlua_spfx.h"

#include "conf.h"
#include "camera.h"
#include "array.h"
#include "nlua_audio.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "sound.h"
#include "sound_openal.h"
#include "opengl.h"
#include "nopenal.h"
#include "player.h"

#define SPFX_GLOBAL     (1<<1) /**< Spfx is not localized. */
#define SPFX_MOVING     (1<<2) /**< Spfx is moving. */
#define SPFX_AUDIO      (1<<3) /**< Spfx has audio. */

/**
 * @brief Handles the special effects Lua-side.
 *
 * It has the advantage of automatically updating the position and audio, while Lua gives flexibility elsewhere.
 */
typedef struct LuaSpfx_s {
   int id;        /**< Unique ID. */
   unsigned int flags; /**< Flags. */
   double ttl;    /**< Time to live. */
   vec2 pos;      /**< Position. */
   vec2 vel;      /**< Velocity. */
   int data;      /**< Reference to table of data. */
   int render_bg; /**< Reference to background render function. */
   int render_mg; /**< Reference to middle render function. */
   int render_fg; /**< Reference to foreground render function. */
   int update;    /**< Reference to update function. */
   LuaAudio_t sfx;/**< Sound effect. */
} LuaSpfx_t;

/**
 * @brief List of special effects being handled.
 */
static LuaSpfx_t **lua_spfx = NULL;
static int lua_spfx_idgen = 0;

/* Spfx methods. */
static int spfxL_gc( lua_State *L );
static int spfxL_eq( lua_State *L );
static int spfxL_new( lua_State *L );
static int spfxL_pos( lua_State *L );
static int spfxL_vel( lua_State *L );
static int spfxL_data( lua_State *L );
static const luaL_Reg spfxL_methods[] = {
   { "__gc", spfxL_gc },
   { "__eq", spfxL_eq },
   { "new", spfxL_new },
   { "pos", spfxL_pos },
   { "vel", spfxL_vel },
   { "data", spfxL_data },
   {0,0}
}; /**< SpfxLua methods. */

static int spfx_cmp( const void *p1, const void *p2 )
{
   const LuaSpfx_t *s1, *s2;
   s1 = *((const LuaSpfx_t**) p1);
   s2 = *((const LuaSpfx_t**) p2);
   return s1->id - s2->id;
}

/**
 * @brief Loads the spfx library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadSpfx( nlua_env env )
{
   nlua_register(env, SPFX_METATABLE, spfxL_methods, 1);
   return 0;
}

/**
 * @brief Gets spfx at index.
 *
 *    @param L Lua state to get spfx from.
 *    @param ind Index position to find the spfx.
 *    @return Spfx found at the index in the state.
 */
LuaSpfx_t* lua_tospfx( lua_State *L, int ind )
{
   LuaSpfx_t *ls = (LuaSpfx_t*) lua_touserdata(L,ind);
   /* TODO is this bsearch necessary? */
   LuaSpfx_t **f = bsearch( &ls, lua_spfx, array_size(lua_spfx), sizeof(LuaSpfx_t*), spfx_cmp );
   if (f == NULL)
      NLUA_ERROR( L, _("Spfx does not exist.") );
   return *f;
}
/**
 * @brief Gets spfx at index or raises error if there is no spfx at index.
 *
 *    @param L Lua state to get spfx from.
 *    @param ind Index position to find spfx.
 *    @return Spfx found at the index in the state.
 */
LuaSpfx_t* luaL_checkspfx( lua_State *L, int ind )
{
   if (lua_isspfx(L,ind))
      return lua_tospfx(L,ind);
   luaL_typerror(L, ind, SPFX_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a spfx on the stack.
 *
 *    @param L Lua state to push spfx into.
 *    @param spfx Spfx to push.
 *    @return Newly pushed spfx.
 */
LuaSpfx_t* lua_pushspfx( lua_State *L, LuaSpfx_t spfx )
{
   LuaSpfx_t *la = (LuaSpfx_t*) lua_newuserdata(L, sizeof(LuaSpfx_t));
   *la = spfx;
   luaL_getmetatable(L, SPFX_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a spfx.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a spfx.
 */
int lua_isspfx( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SPFX_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Cleans up a special effect.
 *
 *    @param ls Special effect to clean up.
 */
static void spfx_cleanup( LuaSpfx_t *ls )
{
   /* Unreference stuff so it can get gc'd. */
   nlua_unref( naevL, ls->data );
   nlua_unref( naevL, ls->render_bg );
   nlua_unref( naevL, ls->render_mg );
   nlua_unref( naevL, ls->render_fg );
   nlua_unref( naevL, ls->update );

   /* Make sure stuff doesn't get run. */
   ls->data       = LUA_NOREF;
   ls->render_bg  = LUA_NOREF;
   ls->render_mg  = LUA_NOREF;
   ls->render_fg  = LUA_NOREF;
   ls->update     = LUA_NOREF;

   /* Clean up audio. */
   audio_cleanup( &ls->sfx );

   /* Set as cleaned up. */
   ls->id = -1;
}

/**
 * @brief Lua bindings to interact with spfx.
 *
 *
 * @luamod spfx
 */
/**
 * @brief Frees a spfx.
 *
 *    @luatparam Spfx spfx Spfx to free.
 * @luafunc __gc
 */
static int spfxL_gc( lua_State *L )
{
   LuaSpfx_t *ls = luaL_checkspfx(L,1);
   (void) ls;
   return 0;
}

/**
 * @brief Compares two spfxs to see if they are the same.
 *
 *    @luatparam Spfx s1 Spfx 1 to compare.
 *    @luatparam Spfx s2 Spfx 2 to compare.
 *    @luatreturn boolean true if both spfxs are the same.
 * @luafunc __eq
 */
static int spfxL_eq( lua_State *L )
{
   LuaSpfx_t *s1, *s2;
   s1 = luaL_checkspfx(L,1);
   s2 = luaL_checkspfx(L,2);
   lua_pushboolean( L, (memcmp( s1, s2, sizeof(LuaSpfx_t) )==0) );
   return 1;
}

/**
 * @brief Creates a new special effect.
 *
 *    @luatparam Number ttl Time to live of the effect.
 *    @luatparam Function|nil update Update function to use if applicable.
 *    @luatparam Function|nil render_bg Background render function to use if applicable (behind ships).
 *    @luatparam Function|nil render_mg Middle render function to use if applicable (infront of NPC ships, behind player).
 *    @luatparam Function|nil render_fg Foregroundrender function to use if applicable (infront of player).
 *    @luatparam vec2 pos Position of the effect.
 *    @luatparam vec2 vel Velocity of the effect.
 *    @luatparam audio sfx Sound effect associated with the spfx.
 *    @luatreturn spfx New spfx corresponding to the data.
 * @luafunc new
 */
static int spfxL_new( lua_State *L )
{
   LuaSpfx_t ls;

   memset( &ls, 0, sizeof(LuaSpfx_t) );

   ls.id       = ++lua_spfx_idgen;
   ls.ttl      = luaL_checknumber(L,1);
   ls.update   = LUA_NOREF;
   ls.render_bg = LUA_NOREF;
   ls.render_mg = LUA_NOREF;
   ls.render_fg = LUA_NOREF;

   /* Functions. */
   if (!lua_isnoneornil(L,2))
      ls.update = nlua_ref( L, 2 );
   if (!lua_isnoneornil(L,3))
      ls.render_bg = nlua_ref( L, 3 );
   if (!lua_isnoneornil(L,4))
      ls.render_mg = nlua_ref( L, 4 );
   if (!lua_isnoneornil(L,5))
      ls.render_fg = nlua_ref( L, 5 );

   /* Position information. */
   if (!lua_isnoneornil(L,6))
      ls.pos = *luaL_checkvector( L, 6 );
   else
      ls.flags |= SPFX_GLOBAL;
   if (!lua_isnoneornil(L,7)) {
      ls.vel = *luaL_checkvector( L, 7 );
      ls.flags |= SPFX_MOVING;
   }

   /* Special effect. */
   if (!lua_isnoneornil(L,8)) {
      LuaAudio_t *la = luaL_checkaudio( L, 8 );
      ls.flags |= SPFX_AUDIO;
      audio_clone( &ls.sfx, la );

      /* Set up parameters. */
      soundLock();
      alSourcei( ls.sfx.source, AL_LOOPING, AL_FALSE );
      if (ls.flags & SPFX_GLOBAL) {
         alSourcei( ls.sfx.source, AL_SOURCE_RELATIVE, AL_TRUE );
         alSourcef( ls.sfx.source, AL_PITCH, 1. );
      }
      else {
         ALfloat alf[3];
         alSourcei( ls.sfx.source, AL_SOURCE_RELATIVE, AL_FALSE );
         alSourcef( ls.sfx.source, AL_REFERENCE_DISTANCE, SOUND_REFERENCE_DISTANCE );
         alSourcef( ls.sfx.source, AL_MAX_DISTANCE, SOUND_MAX_DISTANCE );
         alSourcef( ls.sfx.source, AL_PITCH, player_dt_default() * player.speed );
         alf[0] = ls.pos.x;
         alf[1] = ls.pos.y;
         alf[2] = 0.;
         alSourcefv( ls.sfx.source, AL_POSITION, alf );
         alf[0] = ls.vel.x;
         alf[1] = ls.vel.y;
         alf[2] = 0.;
         alSourcefv( ls.sfx.source, AL_VELOCITY, alf );
      }
      al_checkErr();
      soundUnlock();
   }

   /* Set up new data. */
   lua_newtable(L);
   ls.data = luaL_ref( L, LUA_REGISTRYINDEX ); /* Pops result. */

   /* Add to Lua and stack. */
   if (lua_spfx == NULL)
      lua_spfx = array_create( LuaSpfx_t* );
   array_push_back( &lua_spfx, lua_pushspfx(L, ls) );

   return 1;
}

/**
 * @brief Gets the position of a spfx.
 *
 *    @luatparam spfx s Spfx to get position of.
 *    @luatreturn vec2 Position of the spfx.
 * @luafunc pos( s )
 */
static int spfxL_pos( lua_State *L )
{
   LuaSpfx_t *ls = luaL_checkspfx(L,1);
   lua_pushvector( L, ls->pos );
   return 1;
}

/**
 * @brief Gets the velocity of a spfx.
 *
 *    @luatparam spfx s Spfx to get velocity of.
 *    @luatreturn vec2 Velocity of the spfx.
 * @luafunc vel( s )
 */
static int spfxL_vel( lua_State *L )
{
   LuaSpfx_t *ls = luaL_checkspfx(L,1);
   lua_pushvector( L, ls->pos );
   return 1;
}

/**
 * @brief Gets the data table of a spfx.
 *
 * This table is unique to each instance.
 *
 *    @luatparam spfx s Spfx to get data table of.
 *    @luatreturn table Data table of the spfx.
 * @luafunc data( s )
 */
static int spfxL_data( lua_State *L )
{
   LuaSpfx_t *ls = luaL_checkspfx(L,1);
   lua_rawgeti( L, LUA_REGISTRYINDEX, ls->data );
   return 1;
}

/**
 * @brief Sets the speed of the playing spfx sounds.
 */
void spfxL_setSpeed( double s )
{
   soundLock();
   for (int i=0; i<array_size(lua_spfx); i++) {
      LuaSpfx_t *ls = lua_spfx[i];

      if (!(ls->flags & SPFX_AUDIO))
         continue;

      if (ls->flags & SPFX_GLOBAL)
         continue;

      alSourcef( ls->sfx.source, AL_PITCH, s );
   }
   soundUnlock();
}

/**
 * @brief Updates the spfx.
 *
 *    @luatparam dt Delta tick to use for the update.
 */
void spfxL_update( double dt )
{
   for (int i=array_size(lua_spfx)-1; i>=0; i--) {
      LuaSpfx_t *ls = lua_spfx[i];

      /* Count down. */
      ls->ttl -= dt;
      if (ls->ttl <= 0.) {
         spfx_cleanup( ls );
         array_erase( &lua_spfx, &lua_spfx[i], &lua_spfx[i+1] );
         continue;
      }

      /* Normal update. */
      if (ls->flags & SPFX_MOVING) {
         ls->pos.x += ls->vel.x * dt;
         ls->pos.y += ls->vel.y * dt;

         /* Check sound. */
         if ((ls->flags & SPFX_AUDIO) && !(ls->flags & SPFX_GLOBAL)) {
            soundLock();
            ALfloat alf[3];
            alf[0] = ls->pos.x;
            alf[1] = ls->pos.y;
            alf[2] = 0.;
            alSourcefv( ls->sfx.source, AL_POSITION, alf );
            soundUnlock();
         }
      }

      /* Update if necessary. */
      if (ls->update == LUA_NOREF)
         continue;

      /* Run update. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->update );
      lua_pushspfx( naevL, *ls );
      lua_pushnumber( naevL, dt );
      if (lua_pcall( naevL, 2, 0, 0) != 0) {
         WARN(_("Spfx failed to run 'update':\n%s"), lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
}

/**
 * @brief Renders the Lua SPFX on the background.
 */
void spfxL_renderbg (void)
{
   double z = cam_getZoom();
   for (int i=0; i<array_size(lua_spfx); i++) {
      vec2 pos;
      LuaSpfx_t *ls = lua_spfx[i];

      /* Skip no rendering. */
      if (ls->render_bg == LUA_NOREF)
         continue;

      /* Convert coordinates. */
      gl_gameToScreenCoords( &pos.x, &pos.y, ls->pos.x, ls->pos.y );
      pos.y = SCREEN_H-pos.y;

      /* Render. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->render_bg );
      lua_pushspfx( naevL, *ls );
      lua_pushnumber( naevL, pos.x );
      lua_pushnumber( naevL, pos.y );
      lua_pushnumber( naevL, z );
      if (lua_pcall( naevL, 4, 0, 0) != 0) {
         WARN(_("Spfx failed to run 'renderbg':\n%s"), lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
}

/**
 * @brief Renders the Lua SPFX in the midground.
 */
void spfxL_rendermg (void)
{
   double z = cam_getZoom();
   for (int i=0; i<array_size(lua_spfx); i++) {
      vec2 pos;
      LuaSpfx_t *ls = lua_spfx[i];

      /* Skip no rendering. */
      if (ls->render_mg == LUA_NOREF)
         continue;

      /* Convert coordinates. */
      gl_gameToScreenCoords( &pos.x, &pos.y, ls->pos.x, ls->pos.y );
      pos.y = SCREEN_H-pos.y;

      /* Render. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->render_mg );
      lua_pushspfx( naevL, *ls );
      lua_pushnumber( naevL, pos.x );
      lua_pushnumber( naevL, pos.y );
      lua_pushnumber( naevL, z );
      if (lua_pcall( naevL, 4, 0, 0) != 0) {
         WARN(_("Spfx failed to run 'rendermg':\n%s"), lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
}

/**
 * @brief Renders the Lua SPFX in the foreground.
 */
void spfxL_renderfg (void)
{
   double z = cam_getZoom();
   for (int i=0; i<array_size(lua_spfx); i++) {
      vec2 pos;
      LuaSpfx_t *ls = lua_spfx[i];

      /* Skip no rendering. */
      if (ls->render_fg == LUA_NOREF)
         continue;

      /* Convert coordinates. */
      gl_gameToScreenCoords( &pos.x, &pos.y, ls->pos.x, ls->pos.y );
      pos.y = SCREEN_H-pos.y;

      /* Render. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->render_fg );
      lua_pushspfx( naevL, *ls );
      lua_pushnumber( naevL, pos.x );
      lua_pushnumber( naevL, pos.y );
      lua_pushnumber( naevL, z );
      if (lua_pcall( naevL, 4, 0, 0) != 0) {
         WARN(_("Spfx failed to run 'renderfg':\n%s"), lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
}
