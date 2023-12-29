/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_asteroid.c
 *
 * @brief Bindings for asteroids from Lua.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_asteroid.h"

#include "array.h"
#include "space.h"
#include "nluadef.h"
#include "nlua_vec2.h"
#include "nlua_commodity.h"
#include "nlua_pilot.h"

/* Asteroid methods. */
static int asteroidL_eq( lua_State *L );
static int asteroidL_getAll( lua_State *L );
static int asteroidL_get( lua_State *L );
static int asteroidL_exists( lua_State *L );
static int asteroidL_state( lua_State *L );
static int asteroidL_setState( lua_State *L );
static int asteroidL_field( lua_State *L );
static int asteroidL_pos( lua_State *L );
static int asteroidL_vel( lua_State *L );
static int asteroidL_setPos( lua_State *L );
static int asteroidL_setVel( lua_State *L );
static int asteroidL_scanned( lua_State *L );
static int asteroidL_timer( lua_State *L );
static int asteroidL_setTimer( lua_State *L );
static int asteroidL_armour( lua_State *L );
static int asteroidL_setArmour( lua_State *L );
static int asteroidL_alertRange( lua_State *L );
static int asteroidL_materials( lua_State *L );
static const luaL_Reg asteroidL_methods[] = {
   { "__eq", asteroidL_eq },
   { "getAll", asteroidL_getAll },
   { "get", asteroidL_get },
   { "exists", asteroidL_exists },
   { "state", asteroidL_state },
   { "setState", asteroidL_setState },
   { "field", asteroidL_field },
   { "pos", asteroidL_pos },
   { "vel", asteroidL_vel },
   { "setPos", asteroidL_setPos },
   { "setVel", asteroidL_setVel },
   { "scanned", asteroidL_scanned },
   { "timer", asteroidL_timer },
   { "setTimer", asteroidL_setTimer },
   { "armour", asteroidL_armour },
   { "setArmour", asteroidL_setArmour },
   { "alertRange", asteroidL_alertRange },
   { "materials", asteroidL_materials },
   {0,0}
}; /**< AsteroidLua methods. */

/**
 * @brief Loads the asteroid library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadAsteroid( nlua_env env )
{
   nlua_register(env, ASTEROID_METATABLE, asteroidL_methods, 1);
   return 0;
}

/**
 * @brief Gets asteroid at index.
 *
 *    @param L Lua state to get asteroid from.
 *    @param ind Index position to find the asteroid.
 *    @return Asteroid found at the index in the state.
 */
LuaAsteroid_t* lua_toasteroid( lua_State *L, int ind )
{
   return (LuaAsteroid_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets asteroid at index or raises error if there is no asteroid at index.
 *
 *    @param L Lua state to get asteroid from.
 *    @param ind Index position to find asteroid.
 *    @return Asteroid found at the index in the state.
 */
LuaAsteroid_t* luaL_checkasteroid( lua_State *L, int ind )
{
   if (lua_isasteroid(L,ind))
      return lua_toasteroid(L,ind);
   luaL_typerror(L, ind, ASTEROID_METATABLE);
   return NULL;
}
/**
 * @brief Gets asteroid at index raising an error if type doesn't match.
 *
 *    @param L Lua state to get system from.
 *    @param ind Index position of system.
 *    @return The Asteroid at ind.
 */
Asteroid* luaL_validasteroid( lua_State *L, int ind )
{
   Asteroid *a;
   if (lua_isasteroid(L, ind)) {
      const LuaAsteroid_t *la = luaL_checkasteroid(L, ind);
#if DEBUGGING
      if ((la->parent < 0) || (la->parent >= array_size(cur_system->asteroids))) {
         NLUA_ERROR(L,_("Asteroid field '%d' is out of range!"),la->parent);
         return NULL;
      }
#endif /* DEBUGGING */
      AsteroidAnchor *field = &cur_system->asteroids[ la->parent ];
#if DEBUGGING
      if ((la->id < 0) || (la->id >= array_size(field->asteroids))) {
         NLUA_ERROR(L,_("Asteroid '%d' in field '%d' is out of range!"),la->id,la->parent);
         return NULL;
      }
#endif /* DEBUGGING */
      a = &field->asteroids[ la->id ];
   }
   else {
      luaL_typerror(L, ind, ASTEROID_METATABLE);
      return NULL;
   }

   if (a == NULL) {
      NLUA_ERROR(L, _("Asteroid is invalid"));
      return NULL;
   }

   return a;
}
/**
 * @brief Pushes a asteroid on the stack.
 *
 *    @param L Lua state to push asteroid into.
 *    @param asteroid Asteroid to push.
 *    @return Newly pushed asteroid.
 */
LuaAsteroid_t* lua_pushasteroid( lua_State *L, LuaAsteroid_t asteroid )
{
   LuaAsteroid_t *la = (LuaAsteroid_t*) lua_newuserdata(L, sizeof(LuaAsteroid_t));
   *la = asteroid;
   luaL_getmetatable(L, ASTEROID_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a asteroid.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a asteroid.
 */
int lua_isasteroid( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, ASTEROID_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Lua bindings to interact with asteroid.
 *
 *
 * @luamod asteroid
 */
/**
 * @brief Compares two asteroids to see if they are the same.
 *
 *    @luatparam Asteroid a1 Asteroid 1 to compare.
 *    @luatparam Asteroid a2 Asteroid 2 to compare.
 *    @luatreturn boolean true if both asteroids are the same.
 * @luafunc __eq
 */
static int asteroidL_eq( lua_State *L )
{
   LuaAsteroid_t *a1, *a2;
   a1 = luaL_checkasteroid(L,1);
   a2 = luaL_checkasteroid(L,2);
   lua_pushboolean( L, (memcmp( a1, a2, sizeof(LuaAsteroid_t) )==0) );
   return 1;
}

/**
 * @brief Gets all the asteroids in the system.
 *
 *    @luatreturn table t A list of all asteroids in the system.
 * @luafunc getAll
 */
static int asteroidL_getAll( lua_State *L )
{
   int n = 1;
   lua_newtable(L);
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      const AsteroidAnchor *ast = &cur_system->asteroids[i];
      for (int j=0; j<array_size(ast->asteroids); j++) {
         const Asteroid *a = &ast->asteroids[j];
         LuaAsteroid_t la = {
            .parent = a->parent,
            .id = a->id,
         };

         lua_pushasteroid( L, la );
         lua_rawseti( L, -2, n++ );
      }
   }

   return 1;
}

/**
 * @brief Gets an asteroid in the system.
 *
 *    @luatparam nil|number|Vector|Pilot If not supplied, gets a random asteroid, if not it tries to get the asteroid closest to the Vector or Pilot. In the case of a number, it tries to get a random asteroid from the asteroid field with the number as an id.
 *    @luatreturn Asteroid The closest asteroid or nil if not found.
 * @luafunc get
 */
static int asteroidL_get( lua_State *L )
{
   LuaAsteroid_t la;
   int field = -1;
   const vec2 *pos = NULL;

   /* No asteroids, so everything just returns nil. */
   if (array_size(cur_system->asteroids) <= 0)
      return 0;

   if (lua_isvector(L,1))
      pos = lua_tovector(L,1);

   else if (lua_ispilot(L,1))
      pos = &luaL_validpilot(L,1)->solid.pos;

   else if (lua_isnoneornil(L,1)) {
      int max_field = array_size(cur_system->asteroids) - 1;
      field = (max_field < 0 ? max_field : RNG(0, max_field));
   }
   else if (lua_isnumber(L,1)) {
      field = luaL_checkinteger(L,1)-1;
      if ((field < 0) || (field >= array_size(cur_system->asteroids)))
         NLUA_INVALID_PARAMETER(L,1);
   }
   else
      NLUA_INVALID_PARAMETER(L,1);

   /* Get random asteroid. */
   if ((pos==NULL) && (field >= 0)) {
      /* Random asteroid. */
      int ast   = RNG(0,array_size(cur_system->asteroids[field].asteroids)-1);
      int bad_asteroid = 0;
      Asteroid *a = &cur_system->asteroids[field].asteroids[ast];

      if (a->state != ASTEROID_FG) {
         int n = array_size( cur_system->asteroids[field].asteroids );
         /* Switch to next index until we find a valid one, or until we come full-circle. */
         bad_asteroid = 1;
         for (int i=0; i<n; i++) {
            ast = (ast+1) % n;
            a = &cur_system->asteroids[field].asteroids[ast];
            if (a->state == ASTEROID_FG) {
               bad_asteroid = 0;
               break;
            }
         }
      }

      if (bad_asteroid)
         return 0;

      la.parent = field;
      la.id = ast;
      lua_pushasteroid(L,la);
      return 1;
   }

   /* Try to find nearest asteroid. */
   Asteroid *a_closest = NULL;
   double dist2 = HUGE_VAL;
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      for (int j=0; j<array_size(ast->asteroids); j++) {
         double d2;
         Asteroid *a = &ast->asteroids[j];

         if (a->state != ASTEROID_FG)
            continue;

         d2 = vec2_dist2( pos, &a->sol.pos );
         if (d2 > dist2)
            continue;

         a_closest = a;
         dist2 = d2;
      }
   }
   if (a_closest==NULL)
      return 0;

   /* Got an asteroid, ship it. */
   la.parent = a_closest->parent;
   la.id     = a_closest->id;
   lua_pushasteroid(L,la);
   return 1;
}

/**
 * @brief Checks to see if an asteroid exists.
 *
 *    @luatparam Asteroid a Asteroid to check to see if exists.
 *    @luatreturn boolean true if te asteroid exists, false otherwise.
 * @luafunc exists
 */
static int asteroidL_exists( lua_State *L )
{
   if (!lua_isasteroid(L, 1)) {
      lua_pushboolean(L, 0);
      return 1;
   }

   const LuaAsteroid_t *la = luaL_checkasteroid(L, 1);
   if ((la->parent < 0) || (la->parent >= array_size(cur_system->asteroids))) {
      lua_pushboolean(L, 0);
      return 1;
   }

   AsteroidAnchor *field = &cur_system->asteroids[ la->parent ];
   if ((la->id < 0) || (la->id >= array_size(field->asteroids))) {
      lua_pushboolean(L, 0);
      return 1;
   }

   const Asteroid *a = &field->asteroids[ la->id ];
   lua_pushboolean(L, (a->state==ASTEROID_FG));
   return 1;
}

/**
 * @brief Gets the state of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to check state of.
 *    @luatreturn string State of the asteroid. Can be one of "FG", "XB", "BX", "XX_TO_BG", "FG_TO_BG", "BG_TO_FG", "BG_TO_XX", or "XX".
 * @luafunc state
 */
static int asteroidL_state( lua_State *L )
{
   const Asteroid *ast = luaL_validasteroid(L,1);
   const char *state = NULL;
   switch (ast->state) {
      case ASTEROID_FG:
         state = "FG";
         break;
      case ASTEROID_XB:
         state = "XB";
         break;
      case ASTEROID_BX:
         state = "BX";
         break;
      case ASTEROID_XX_TO_BG:
         state = "XX_TO_BG";
         break;
      case ASTEROID_FG_TO_BG:
         state = "FG_TO_BG";
         break;
      case ASTEROID_BG_TO_FG:
         state = "BG_TO_FG";
         break;
      case ASTEROID_BG_TO_XX:
         state = "BG_TO_XX";
         break;
      case ASTEROID_XX:
         state = "XX";
         break;
   }
   lua_pushstring( L, state );
   return 1;
}

/**
 * @brief Sets the state of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set state of.
 *    @luatparam string s State to set. Has to be one of "FG", "XB", "BX", "XX_TO_BG", "FG_TO_BG", "BG_TO_FG", "BG_TO_XX", or "XX".
 * @luafunc state
 */
static int asteroidL_setState( lua_State *L )
{
   Asteroid *ast = luaL_validasteroid(L,1);
   const char *state = luaL_checkstring(L,2);
   if (strcmp(state,"FG")==0)
      ast->state = ASTEROID_FG;
   else if (strcmp(state,"XB")==0)
      ast->state = ASTEROID_XB;
   else if (strcmp(state,"BX")==0)
      ast->state = ASTEROID_BX;
   else if (strcmp(state,"XX_TO_BG")==0)
      ast->state = ASTEROID_XX_TO_BG;
   else if (strcmp(state,"FG_TO_BG")==0)
      ast->state = ASTEROID_FG_TO_BG;
   else if (strcmp(state,"BG_TO_FG")==0)
      ast->state = ASTEROID_BG_TO_FG;
   else if (strcmp(state,"BG_TO_XX")==0)
      ast->state = ASTEROID_BG_TO_XX;
   else if (strcmp(state,"XX")==0)
      ast->state = ASTEROID_XX;
   else
      NLUA_INVALID_PARAMETER(L,2);
   return 0;
}

/**
 * @brief Gets the field the asteroid belongs to (useful for getting more asteroids from the same field).
 *
 *    @luatparam Asteroid a Asteroid to get what field it belongs to.
 *    @luatreturn number ID of the field the asteroid belongs to.
 * @luafunc field
 */
static int asteroidL_field( lua_State *L )
{
   const LuaAsteroid_t *la = luaL_checkasteroid(L,1);
   lua_pushinteger(L,la->parent+1);
   return 1;
}

/**
 * @brief Gets the position of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get position of.
 *    @luatreturn Vector Position of the asteroid.
 * @luafunc pos
 */
static int asteroidL_pos( lua_State *L )
{
   const Asteroid *a = luaL_validasteroid(L,1);
   lua_pushvector(L,a->sol.pos);
   return 1;
}

/**
 * @brief Gets the velocity of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get velocity of.
 *    @luatreturn Vector Position of the asteroid.
 * @luafunc vel
 */
static int asteroidL_vel( lua_State *L )
{
   const Asteroid *a = luaL_validasteroid(L,1);
   lua_pushvector(L,a->sol.vel);
   return 1;
}

/**
 * @brief Sets the position of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set position of.
 *    @luatparam vec2 v Position to set to.
 * @luafunc setPos
 */
static int asteroidL_setPos( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   const vec2 *v = luaL_checkvector(L,2);
   a->sol.pos = *v;
   return 0;
}

/**
 * @brief Sets the velocity of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set velocity of.
 *    @luatparam vec2 v Velocity to set to.
 * @luafunc setVel
 */
static int asteroidL_setVel( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   const vec2 *v = luaL_checkvector(L,2);
   a->sol.vel = *v;
   return 0;
}

/**
 * @brief Gets whether or not an asteroid got scanned.
 *
 *    @luatparam Asteroid a Asteroid to get scanned state of.
 *    @luatreturn boolean Whether or not the asteroid was scanned.
 * @luafunc scanned
 */
static int asteroidL_scanned( lua_State *L )
{
   const Asteroid *a = luaL_validasteroid(L,1);
   lua_pushboolean(L,a->scanned);
   return 1;
}

/**
 * @brief Gets the time left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get time left on.
 *    @luatreturn number Seconds left before the asteroid is unavailable.
 *    @luatreturn number Total number of seconds the asteroid had to live.
 * @luafunc timer
 */
static int asteroidL_timer( lua_State *L )
{
   const Asteroid *a = luaL_validasteroid(L,1);
   lua_pushnumber(L,a->timer);
   lua_pushnumber(L,a->timer_max);
   return 2;
}

/**
 * @brief Sets the time left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set time left of.
 *    @luatparam number Time left to set to in seconds.
 * @luafunc setTimer
 */
static int asteroidL_setTimer( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   a->timer = luaL_checknumber(L,2);
   a->timer_max = MAX( a->timer_max, a->timer );
   return 0;
}

/**
 * @brief Gets the armour (health) left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get armour of.
 *    @luatreturn number Armour left before the asteroid is destroyed.
 * @luafunc armour
 */
static int asteroidL_armour( lua_State *L )
{
   const Asteroid *a = luaL_validasteroid(L,1);
   lua_pushnumber(L,a->armour);
   return 1;
}

/**
 * @brief Sets the armour of the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set armour of.
 *    @luatparam number Amount to set the armour to (negative values will explode it).
 * @luafunc setTimer
 */
static int asteroidL_setArmour( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   a->armour = luaL_checknumber(L,2);
   if (a->armour <= 0.)
      asteroid_explode( a, -1, 0. );
   return 0;
}

/**
 * @brief Gets the alert range of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get alert range of.
 *    @luatreturn number Alert range of the asteroid.
 * @luafunc alertRange
 */
static int asteroidL_alertRange( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushnumber(L,a->type->alert_range);
   return 1;
}

/**
 * @brief Gets the materials the asteroid can potentially drop.
 *
 *    @luatparam Asteroid a Asteroid to get materials of.
 *    @luatreturn table Table containing the materials.
 * @luafunc materials
 */
static int asteroidL_materials( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   const AsteroidType *at = a->type;

   lua_newtable(L);
   for (int i=0; i<array_size(at->material); i++) {
      const AsteroidReward *mat = &at->material[i];
      lua_newtable(L);

      lua_pushcommodity( L, mat->material );
      lua_setfield(L,-2,"commodity");

      lua_pushnumber( L, mat->quantity );
      lua_setfield(L,-2,"quantity");

      lua_pushnumber( L, mat->rarity );
      lua_setfield(L,-2,"rarity");

      lua_rawseti( L, -2, i+1 );
   }

   return 1;
}
