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
#include "debris.h"
#include "array.h"
#include "nlua_audio.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "sound.h"
#include "opengl.h"
#include "nopenal.h"
#include "player.h"

#define SPFX_GLOBAL     (1<<1) /**< Spfx sound ignores pitch changes. */
#define SPFX_RELATIVE   (1<<2) /**< Spfx is relative. */
#define SPFX_MOVING     (1<<3) /**< Spfx is moving. */
#define SPFX_AUDIO      (1<<4) /**< Spfx has audio. */
#define SPFX_CLEANUP    (1<<5) /**< Spfx has to be cleaned up. */

/**
 * @brief Handles the special effects Lua-side.
 *
 * It has the advantage of automatically updating the position and audio, while Lua gives flexibility elsewhere.
 */
typedef struct LuaSpfxData_s {
   int id;        /**< Unique ID. */
   unsigned int flags; /**< Flags. */
   double ttl;    /**< Time to live. */
   vec2 pos;      /**< Position. */
   vec2 vel;      /**< Velocity. */
   double radius; /**< Used for rendering. */
   int data;      /**< Reference to table of data. */
   int render_bg; /**< Reference to background render function. */
   int render_mg; /**< Reference to middle render function. */
   int render_fg; /**< Reference to foreground render function. */
   int update;    /**< Reference to update function. */
   int remove;    /**< Reference to remove function. */
   LuaAudio_t sfx;/**< Sound effect. */
} LuaSpfxData_t;

/**
 * @brief List of special effects being handled.
 */
static LuaSpfxData_t *lua_spfx = NULL;
static LuaSpfxData_t *lua_spfx_queue = NULL;
static int lua_spfx_idgen = 0;
static int lua_spfx_lock = 0;

/* Spfx methods. */
static int spfxL_gc( lua_State *L );
static int spfxL_eq( lua_State *L );
static int spfxL_getAll( lua_State *L );
static int spfxL_new( lua_State *L );
static int spfxL_rm( lua_State *L );
static int spfxL_pos( lua_State *L );
static int spfxL_vel( lua_State *L );
static int spfxL_setPos( lua_State *L );
static int spfxL_setVel( lua_State *L );
static int spfxL_sfx( lua_State *L );
static int spfxL_data( lua_State *L );
static int spfxL_debris( lua_State *L );
static const luaL_Reg spfxL_methods[] = {
   { "__gc", spfxL_gc },
   { "__eq", spfxL_eq },
   { "getAll", spfxL_getAll },
   { "new", spfxL_new },
   { "rm", spfxL_rm },
   { "pos", spfxL_pos },
   { "vel", spfxL_vel },
   { "setPos", spfxL_setPos },
   { "setVel", spfxL_setVel },
   { "sfx", spfxL_sfx },
   { "data", spfxL_data },
   { "debris", spfxL_debris },
   {0,0}
}; /**< SpfxLua methods. */

static int spfx_cmp( const void *p1, const void *p2 )
{
   const LuaSpfxData_t *s1, *s2;
   s1 = (const LuaSpfxData_t*) p1;
   s2 = (const LuaSpfxData_t*) p2;
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
   return (LuaSpfx_t*) lua_touserdata(L,ind);
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
static LuaSpfxData_t* luaL_checkspfxdataNoWarn( lua_State *L, int ind )
{
   LuaSpfx_t *ls = luaL_checkspfx( L , ind );
   const LuaSpfxData_t key = { .id = *ls };
   LuaSpfxData_t *f = bsearch( &key, lua_spfx, array_size(lua_spfx), sizeof(LuaSpfxData_t), spfx_cmp );
   if (f == NULL) {
      f = bsearch( &key, lua_spfx_queue, array_size(lua_spfx_queue), sizeof(LuaSpfxData_t), spfx_cmp );
   }
   return f;
}
static LuaSpfxData_t* luaL_checkspfxdata( lua_State *L, int ind )
{
   LuaSpfxData_t *f = luaL_checkspfxdataNoWarn( L, ind );
   if (f == NULL)
      NLUA_ERROR( L, _("Spfx does not exist.") );
   return f;
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
static void spfx_cleanup( LuaSpfxData_t *ls )
{
   /* Unreference stuff so it can get gc'd. */
   nlua_unref( naevL, ls->data );
   nlua_unref( naevL, ls->render_bg );
   nlua_unref( naevL, ls->render_mg );
   nlua_unref( naevL, ls->render_fg );
   nlua_unref( naevL, ls->update );
   nlua_unref( naevL, ls->remove );

   /* Make sure stuff doesn't get run. */
   ls->data       = LUA_NOREF;
   ls->render_bg  = LUA_NOREF;
   ls->render_mg  = LUA_NOREF;
   ls->render_fg  = LUA_NOREF;
   ls->update     = LUA_NOREF;
   ls->remove     = LUA_NOREF;

   /* Clean up audio. */
   ls->sfx.nocleanup = 0; /* Have to disable so it gets cleaned. */
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
 * @brief Gets all the active spfx.
 *
 *    @luatreturn table A table containing all the spfx.
 * @luafunc getAll
 */
static int spfxL_getAll( lua_State *L )
{
   int n=1;
   lua_newtable(L);
   for (int i=0; i<array_size(lua_spfx); i++) {
      LuaSpfxData_t *ls = &lua_spfx[i];

      if (ls->flags & (SPFX_GLOBAL | SPFX_CLEANUP))
         continue;

      lua_pushspfx( L, ls->id );
      lua_rawseti( L, -2, n++ );
   }
   return 1;
}

/**
 * @brief Creates a new special effect.
 *
 * @usage spfx.new( 5, update, nil, nil, render, player.pos(), player.pilot():vel(), sfx ) -- Play effect with update and render functions at player position/velocity
 * @usage spfx.new( 10, nil, nil, nil, nil, true, nil, sfx ) -- Play an effect locally (affected by time compression and autonav stuff)
 * @usage spfx.new( 10, nil, nil, nil, nil, nil, nil, sfx ) -- Play a global effect (not affected by time stuff )
 *
 *    @luatparam Number ttl Time to live of the effect.
 *    @luatparam[opt] Function|nil update Update function to use if applicable.
 *    @luatparam[opt] Function|nil render_bg Background render function to use if applicable (behind ships).
 *    @luatparam[opt] Function|nil render_mg Middle render function to use if applicable (infront of NPC ships, behind player).
 *    @luatparam[opt] Function|nil render_fg Foregroundrender function to use if applicable (infront of player).
 *    @luatparam[opt] vec2|boolean pos Position of the effect, or a boolean to indicate whether or not the effect is local.
 *    @luatparam[opt] vec2 vel Velocity of the effect.
 *    @luatparam[opt] audio sfx Sound effect associated with the spfx.
 *    @luatparam[opt] number radius Radius to use to determine if should render.
 *    @luatparam[opt] Function|nil remove Function to run when removing the outfit.
 *    @luatreturn spfx New spfx corresponding to the data.
 * @luafunc new
 */
static int spfxL_new( lua_State *L )
{
   LuaSpfxData_t ls;

   memset( &ls, 0, sizeof(LuaSpfxData_t) );

   ls.id       = ++lua_spfx_idgen;
   ls.ttl      = luaL_checknumber(L,1);
   ls.update   = LUA_NOREF;
   ls.render_bg = LUA_NOREF;
   ls.render_mg = LUA_NOREF;
   ls.render_fg = LUA_NOREF;
   ls.remove   = LUA_NOREF;

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
   if (!lua_isnoneornil(L,6)) {
      if (lua_isboolean( L, 6 )) {
         ls.flags |= SPFX_RELATIVE;
         if (!lua_toboolean( L, 6 ))
            ls.flags |= SPFX_GLOBAL;
      }
      else
         ls.pos = *luaL_checkvector( L, 6 );
   }
   else
      ls.flags |= SPFX_GLOBAL | SPFX_RELATIVE;
   if (!lua_isnoneornil(L,7)) {
      ls.vel = *luaL_checkvector( L, 7 );
      ls.flags |= SPFX_MOVING;
   }

   /* Special effect. */
   if (!lua_isnoneornil(L,8)) {
      LuaAudio_t *la = luaL_checkaudio( L, 8 );

      if (!sound_disabled) {
         ls.flags |= SPFX_AUDIO;
         audio_clone( &ls.sfx, la );
         ls.sfx.nocleanup = 1;

         /* Set up parameters. */
         soundLock();
         alSourcei( ls.sfx.source, AL_LOOPING, AL_FALSE );
         alSourcef( ls.sfx.source, AL_REFERENCE_DISTANCE, SOUND_REFERENCE_DISTANCE );
         alSourcef( ls.sfx.source, AL_MAX_DISTANCE, SOUND_MAX_DISTANCE );
         if (ls.flags & SPFX_RELATIVE) {
            alSourcei( ls.sfx.source, AL_SOURCE_RELATIVE, AL_TRUE );
            if (ls.flags & SPFX_GLOBAL)
               alSourcef( ls.sfx.source, AL_PITCH, 1. );
            else
               alSourcef( ls.sfx.source, AL_PITCH, player_dt_default() * player.speed );
         }
         else {
            ALfloat alf[3];
            alSourcei( ls.sfx.source, AL_SOURCE_RELATIVE, AL_FALSE );
            alSourcef( ls.sfx.source, AL_PITCH, player_dt_default() * player.speed );
            alf[0] = ls.pos.x;
            alf[1] = ls.pos.y;
            alf[2] = 0.;
            alSourcefv( ls.sfx.source, AL_POSITION, alf );
            alf[0] = ls.vel.x;
            alf[1] = ls.vel.y;
            alf[2] = 0.;
            alSourcefv( ls.sfx.source, AL_VELOCITY, alf );

            /* Set the global filter. */
            if (al_info.efx == AL_TRUE)
               alSource3i( ls.sfx.source, AL_AUXILIARY_SEND_FILTER, sound_efx_directSlot, 0, AL_FILTER_NULL );
         }
         alSourcePlay( ls.sfx.source );
         al_checkErr();
         soundUnlock();
      }
   }

   /* Store radius. */
   ls.radius = luaL_optnumber(L,9,-1.);

   /* Finally remove function if applicable. */
   if (!lua_isnoneornil(L,10))
      ls.remove = nlua_ref( L, 10 );

   /* Set up new data. */
   lua_newtable(L);
   ls.data = luaL_ref( L, LUA_REGISTRYINDEX ); /* Pops result. */

   /* Add to Lua and stack, depending on if locked or not. */
   if (lua_spfx_lock) {
      if (lua_spfx_queue == NULL)
         lua_spfx_queue = array_create( LuaSpfxData_t );
      array_push_back( &lua_spfx_queue, ls );
   }
   else {
      if (lua_spfx == NULL)
         lua_spfx = array_create( LuaSpfxData_t );
      array_push_back( &lua_spfx, ls );
   }

   lua_pushspfx( L, ls.id );
   return 1;
}

/**
 * @brief Removes a special effect.
 *
 *    @luatparam spfx s Spfx to remove.
 * @luafunc rm
 */
static int spfxL_rm( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdataNoWarn(L,1);
   if (ls != NULL) {
      if (ls->remove != LUA_NOREF) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->remove );
         lua_pushspfx( naevL, ls->id );
         if (lua_pcall( naevL, 1, 0, 0) != 0) {
            WARN(_("Spfx failed to run 'remove':\n%s"), lua_tostring( naevL, -1 ));
            lua_pop( naevL, 1 );
         }
      }

      ls->flags &= SPFX_CLEANUP;
      ls->ttl = -1.;
   }
   return 0;
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
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   lua_pushvector( L, ls->pos );
   return 1;
}

/**
 * @brief Gets the velocity of a spfx.
 *
 *    @luatparam spfx s Spfx to get velocity of.
 *    @luatreturn vec2 Velocity of the spfx.
 * @luafunc vel
 */
static int spfxL_vel( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   lua_pushvector( L, ls->vel );
   return 1;
}

/**
 * @brief Sets the position of a spfx.
 *
 *    @luatparam spfx s Spfx to set the position of.
 *    @luatparam vec2 p Position to set to.
 * @luafunc setPos
 */
static int spfxL_setPos( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   vec2 *v = luaL_checkvector(L,2);
   ls->pos = *v;
   return 0;
}

/**
 * @brief Sets the velocity of a spfx.
 *
 *    @luatparam spfx s Spfx to set the velocity of.
 *    @luatparam vec2 v Velocity to set to.
 * @luafunc setVel
 */
static int spfxL_setVel( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   vec2 *v = luaL_checkvector(L,2);
   ls->vel = *v;
   return 0;
}

/**
 * @brief Gets the sound effect of a spfx.
 *
 *    @luatparam spfx s Spfx to get sound effect of.
 *    @luatreturn audio Sound effect of the spfx.
 * @luafunc vel
 */
static int spfxL_sfx( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   lua_pushaudio( L, ls->sfx );
   return 1;
}

/**
 * @brief Gets the data table of a spfx.
 *
 * This table is unique to each instance.
 *
 *    @luatparam spfx s Spfx to get data table of.
 *    @luatreturn table Data table of the spfx.
 * @luafunc data
 */
static int spfxL_data( lua_State *L )
{
   LuaSpfxData_t *ls = luaL_checkspfxdata(L,1);
   lua_rawgeti( L, LUA_REGISTRYINDEX, ls->data );
   return 1;
}

/**
 * @brief Sets the speed of the playing spfx sounds.
 */
void spfxL_setSpeed( double s )
{
   if (sound_disabled)
      return;

   soundLock();
   for (int i=0; i<array_size(lua_spfx); i++) {
      LuaSpfxData_t *ls = &lua_spfx[i];

      if (!(ls->flags & SPFX_AUDIO))
         continue;

      if (ls->flags & (SPFX_GLOBAL | SPFX_CLEANUP))
         continue;

      alSourcef( ls->sfx.source, AL_PITCH, s );
   }
   al_checkErr();
   soundUnlock();
}

/**
 * @brief Sets the speed volume due to autonav and the likes.
 *
 *    @param v Speed volume to use.
 */
void spfxL_setSpeedVolume( double v )
{
   if (sound_disabled)
      return;

   soundLock();
   for (int i=0; i<array_size(lua_spfx); i++) {
      LuaSpfxData_t *ls = &lua_spfx[i];

      if (!(ls->flags & SPFX_AUDIO))
         continue;

      if (ls->flags & (SPFX_GLOBAL | SPFX_CLEANUP))
         continue;

      alSourcef( ls->sfx.source, AL_GAIN, ls->sfx.volume * v );
   }
   al_checkErr();
   soundUnlock();
}

static void spfx_lock (void)
{
   lua_spfx_lock = 1;
}

static void spfx_unlock (void)
{
   lua_spfx_lock = 0;

   if (lua_spfx_queue==NULL)
      return;

   for (int i=0; i<array_size(lua_spfx_queue); i++)
      array_push_back( &lua_spfx, lua_spfx_queue[i] );
   array_erase( &lua_spfx_queue, array_begin(lua_spfx_queue), array_end(lua_spfx_queue) );
}

/**
 * @brief Clears the Lua spfx.
 */
void spfxL_clear (void)
{
   for (int i=0; i<array_size(lua_spfx); i++)
      spfx_cleanup( &lua_spfx[i] );
   array_erase( &lua_spfx, array_begin(lua_spfx), array_end(lua_spfx) );
   for (int i=0; i<array_size(lua_spfx_queue); i++)
      spfx_cleanup( &lua_spfx_queue[i] );
   array_erase( &lua_spfx_queue, array_begin(lua_spfx_queue), array_end(lua_spfx_queue) );
}

void spfxL_exit (void)
{
   spfxL_clear();
   array_free( lua_spfx );
   lua_spfx = NULL;
   array_free( lua_spfx_queue );
   lua_spfx_queue = NULL;
}

/**
 * @brief Updates the spfx.
 *
 *    @luatparam dt Delta tick to use for the update.
 */
void spfxL_update( double dt )
{
   spfx_lock();
   for (int i=array_size(lua_spfx)-1; i>=0; i--) {
      LuaSpfxData_t *ls = &lua_spfx[i];

      /* Count down. */
      ls->ttl -= dt;
      if ((ls->ttl <= 0.) || (ls->flags & SPFX_CLEANUP)) {
         spfx_cleanup( ls );
         array_erase( &lua_spfx, &lua_spfx[i], &lua_spfx[i+1] );
         continue;
      }

      /* Normal update. */
      if (ls->flags & SPFX_MOVING) {
         ls->pos.x += ls->vel.x * dt;
         ls->pos.y += ls->vel.y * dt;

         /* Check sound. */
         if ((ls->flags & SPFX_AUDIO) && !(ls->flags & SPFX_RELATIVE)) {
            soundLock();
            ALfloat alf[3];
            alf[0] = ls->pos.x;
            alf[1] = ls->pos.y;
            alf[2] = 0.;
            alSourcefv( ls->sfx.source, AL_POSITION, alf );
            al_checkErr();
            soundUnlock();
         }
      }

      /* Update if necessary. */
      if (ls->update == LUA_NOREF)
         continue;

      /* Run update. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, ls->update );
      lua_pushspfx( naevL, ls->id );
      lua_pushnumber( naevL, dt );
      if (lua_pcall( naevL, 2, 0, 0) != 0) {
         WARN(_("Spfx failed to run 'update':\n%s"), lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
   spfx_unlock();
}

static void spfxL_renderLayer( int func, const char *funcname, double dt )
{
   double z = cam_getZoom();
   spfx_lock();
   for (int i=0; i<array_size(lua_spfx); i++) {
      vec2 pos;
      LuaSpfxData_t *ls = &lua_spfx[i];
      double r = ls->radius;
      int funcref;

      switch (func) {
         case 0:
            funcref = ls->render_bg;
            break;
         case 1:
            funcref = ls->render_mg;
            break;
         case 2:
            funcref = ls->render_fg;
            break;
      }

      /* Skip no rendering. */
      if ((funcref == LUA_NOREF) || (ls->flags & SPFX_CLEANUP))
         continue;

      /* Convert coordinates. */
      gl_gameToScreenCoords( &pos.x, &pos.y, ls->pos.x, ls->pos.y );

      /* If radius is defined see if in screen. */
      if ((r > 0.) && ((pos.x < -r) || (pos.y < -r) ||
            (pos.x > SCREEN_W+r) || (pos.y > SCREEN_H+r)))
         continue;

      /* Invert y axis. */
      pos.y = SCREEN_H-pos.y;

      /* Render. */
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, funcref );
      lua_pushspfx( naevL, ls->id );
      lua_pushnumber( naevL, pos.x );
      lua_pushnumber( naevL, pos.y );
      lua_pushnumber( naevL, z );
      lua_pushnumber( naevL, dt );
      if (lua_pcall( naevL, 5, 0, 0) != 0) {
         WARN(_("Spfx failed to run '%s':\n%s"), funcname, lua_tostring( naevL, -1 ));
         lua_pop( naevL, 1 );
      }
   }
   spfx_unlock();
}

/**
 * @brief Renders the Lua SPFX on the background.
 */
void spfxL_renderbg( double dt )
{
   spfxL_renderLayer( 0, "renderbg", dt );
}

/**
 * @brief Renders the Lua SPFX in the midground.
 */
void spfxL_rendermg( double dt )
{
   spfxL_renderLayer( 1, "rendermg", dt );
}

/**
 * @brief Renders the Lua SPFX in the foreground.
 */
void spfxL_renderfg( double dt )
{
   spfxL_renderLayer( 2, "rendermg", dt );
}

/**
 * @brief Creates a cloud of debris.
 *
 *    @luatparam number mass Mass of the cloud.
 *    @luatparam number radius Radius of the cloud.
 *    @luatparam Vec2 pos Position of the cloud.
 *    @luatparam Vec2 vel Velocity of the cloud.
 * @luafunc debris
 */
static int spfxL_debris( lua_State *L )
{
   double mass = luaL_checknumber( L, 1 );
   double radius = luaL_checknumber( L, 2 );
   vec2 *p = luaL_checkvector( L, 3 );
   vec2 *v = luaL_checkvector( L, 4 );
   debris_add( mass, radius, p->x, p->y, v->x, v->y );
   return 0;
}
