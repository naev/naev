/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_munition.c
 *
 * @brief Handles the Lua munition bindings.
 *
 * These bindings control the spobs and systems.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua_munition.h"

#include "array.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_outfit.h"
#include "nlua_vec2.h"
#include "nlua_pilot.h"

/* Prototypes. */
static Weapon *munition_get( LuaMunition *lm );

/* Munition metatable methods. */
static int munitionL_eq( lua_State *L );
static int munitionL_tostring( lua_State *L );
static int munitionL_exists( lua_State *L );
static int munitionL_clear( lua_State *L );
static int munitionL_getAll( lua_State *L );
static int munitionL_getInrange( lua_State *L );
static int munitionL_pos( lua_State *L );
static int munitionL_vel( lua_State *L );
static int munitionL_faction( lua_State *L );
static int munitionL_parent( lua_State *L );
static int munitionL_target( lua_State *L );
static int munitionL_outfit( lua_State *L );
static int munitionL_strength( lua_State *L );
static int munitionL_strengthSet( lua_State *L );
static const luaL_Reg munitionL_methods[] = {
   /* General. */
   { "__eq", munitionL_eq },
   { "__tostring", munitionL_tostring },
   { "exists", munitionL_exists },
   { "clear", munitionL_clear },
   { "getAll", munitionL_getAll },
   { "getInrange", munitionL_getInrange },
   /* Get properties. */
   { "pos", munitionL_pos },
   { "vel", munitionL_vel },
   { "faction", munitionL_faction },
   { "parent", munitionL_parent },
   { "target", munitionL_target },
   { "outfit", munitionL_outfit },
   { "strength", munitionL_strength },
   /* Set properties. */
   { "strengthSet", munitionL_strengthSet },
   /* End sentinal. */
   {0,0},
}; /**< Munition metatable methods. */

/**
 * @brief Loads the munition library.
 *
 *    @param env Environment to load library into.
 *    @return 0 on success.
 */
int nlua_loadMunition( nlua_env env )
{
   nlua_register(env, MUNITION_METATABLE, munitionL_methods, 1);

   /* Munition always loads ship and asteroid. */
   nlua_loadOutfit(env);

   return 0;
}

/**
 * @brief Lua bindings to interact with munitions.
 *
 * This will allow you to create and manipulate munitions in-game.
 *
 * @luamod munition
 */
/**
 * @brief Gets munition at index.
 *
 *    @param L Lua state to get munition from.
 *    @param ind Index position to find the munition.
 *    @return Munition found at the index in the state.
 */
LuaMunition* lua_tomunition( lua_State *L, int ind )
{
   return ((LuaMunition*) lua_touserdata(L,ind));
}
/**
 * @brief Gets munition at index or raises error if there is no munition at index.
 *
 *    @param L Lua state to get munition from.
 *    @param ind Index position to find munition.
 *    @return Munition found at the index in the state.
 */
LuaMunition* luaL_checkmunition( lua_State *L, int ind )
{
   if (lua_ismunition(L,ind))
      return lua_tomunition(L,ind);
   luaL_typerror(L, ind, MUNITION_METATABLE);
   return lua_tomunition(L,ind); /* Just to shut compiler up, shouldn't be reached. */
}
/**
 * @brief Makes sure the munition is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the munition to validate.
 *    @return The munition (doesn't return if fails - raises Lua error ).
 */
Weapon* luaL_validmunition( lua_State *L, int ind )
{
   Weapon *w = munition_get( luaL_checkmunition(L,ind) );
   if (w==NULL) {
      NLUA_ERROR(L,_("Munition is invalid."));
      return NULL;
   }
   return w;
}
/**
 * @brief Pushes a weapon as a munition on the stack.
 *
 *    @param L Lua state to push munition into.
 *    @param w Weapon to push as munition.
 *    @return Newly pushed munition.
 */
LuaMunition* lua_pushmunition( lua_State *L, const Weapon *w )
{
   Weapon *weapon_stack = weapon_getStack();
   LuaMunition *lm = (LuaMunition*) lua_newuserdata(L, sizeof(LuaMunition));
   lm->id   = w->id;
   lm->idx  = w-weapon_stack;
   luaL_getmetatable(L, MUNITION_METATABLE);
   lua_setmetatable(L, -2);
   return lm;
}
/**
 * @brief Checks to see if ind is a munition.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a munition.
 */
int lua_ismunition( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, MUNITION_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Checks to see if munition and p are the same.
 *
 * @usage if p == p2 then -- Munition 'p' and 'p2' match.
 *
 *    @luatparam Munition p Munition to compare.
 *    @luatparam Munition comp Munition to compare against.
 *    @luatreturn boolean true if they are the same.
 * @luafunc __eq
 */
static int munitionL_eq( lua_State *L )
{
   LuaMunition *lm1 = luaL_checkmunition(L,1);
   LuaMunition *lm2 = luaL_checkmunition(L,2);
   lua_pushboolean(L, lm1->id==lm2->id);
   return 1;
}

static Weapon *munition_get( LuaMunition *lm )
{
   Weapon *weapon_stack = weapon_getStack();
   Weapon *w;

   if ((lm->idx < (size_t)array_size(weapon_stack)) && (weapon_stack[lm->idx].id==lm->id))
      return &weapon_stack[lm->idx];

   w = weapon_getID( lm->id );
   if (w==NULL)
      return NULL;
   lm->idx = w-weapon_stack; /* For next look ups. */
   return w;
}

/**
 * @brief Gets the munition's current (translated) name or notes it is inexistent.
 *
 * @usage tostring(p)
 *
 *    @luatparam Munition p Munition to convert to string.
 *    @luatreturn string The current name of the munition or "(inexistent munition)" if not existent.
 * @luafunc __tostring
 */
static int munitionL_tostring( lua_State *L )
{
   LuaMunition *lm = luaL_checkmunition( L, 1 );
   Weapon *w = munition_get(lm);
   if (w!=NULL)
      lua_pushstring(L,_(w->outfit->name));
   else
      lua_pushstring(L,"(inexistent munition)");
   return 1;
}

/**
 * @brief Checks to see if a munition still exists.
 *
 *    @luatparam Munition m Munition to see if still exists.
 *    @luatreturn boolean true if the munition still exists.
 * @luafunc exists
 */
static int munitionL_exists( lua_State *L )
{
   LuaMunition *lm = luaL_checkmunition( L, 1 );
   lua_pushboolean(L, munition_get(lm)!=NULL);
   return 1;
}

/**
 * @brief Clears all the munitions in the system.
 *
 * @luafunc clear
 */
static int munitionL_clear( lua_State *L )
{
   (void) L;
   weapon_clear();
   return 0;
}

/**
 * @brief Gets all the munitions in the system.
 *
 *    @luatparam boolean onlyhittable Whether or not to only get hittable munitions, or all of them.
 *    @luatreturn table A table containing all the munitions in the system.
 * @luafunc getAll
 */
static int munitionL_getAll( lua_State *L )
{
   const Weapon *weapon_stack = weapon_getStack();
   int onlyhittable = lua_toboolean(L,1);
   int n = 1;
   lua_newtable(L);
   for (int i=0; i<array_size(weapon_stack); i++) {
      const Weapon *w = &weapon_stack[i];
      if (weapon_isFlag(w,WEAPON_FLAG_DESTROYED))
         continue;
      if (onlyhittable && !weapon_isFlag(w,WEAPON_FLAG_HITTABLE))
         continue;
      lua_pushmunition( L, w );
      lua_rawseti( L, -2, n++ );
   }
   return 1;
}

static int weapon_isHostile( const Weapon *w, const Pilot *p )
{
   if (p->id == w->parent)
      return 0;

   if ((w->target.type==TARGET_PILOT) && (w->target.u.id==p->id))
      return 1;

   /* Let hostiles hit player. */
   if (p->faction == FACTION_PLAYER) {
      const Pilot *parent = pilot_get(w->parent);
      if (parent != NULL) {
         if (pilot_isHostile(parent))
            return 1;
      }
   }

   /* Hit non-allies. */
   if (areEnemies(w->faction, p->faction))
      return 1;

   return 0;
}

/**
 * @brief Get munitions in range. Note that this can only get hittable munitions.
 *
 *    @luatparam Vec2 pos Position from which to get munitions.
 *    @luatparam number range Range to get munitions from.
 *    @luatparam[opt=nil] Pilot|nil Pilot to check hostility to.
 *    @luatreturn table A table containing all the munitions found in range.
 * @luafunc getInrange
 */
static int munitionL_getInrange( lua_State *L )
{
   const Weapon *weapon_stack = weapon_getStack();
   const IntList *qt;
   int n = 1;
   const vec2 *pos = luaL_checkvector(L,1);
   double range = luaL_checknumber(L,2);
   const Pilot *p = luaL_optpilot(L,3,NULL);
   double r2 = pow2(range);
   int x, y, r;

   x = round( pos->x );
   y = round( pos->y );
   r = ceil( range );

   lua_newtable(L);
   qt = weapon_collideQuery( x-r, y-r, x+r, y+r );
   for (int i=0; i<il_size(qt); i++) {
      const Weapon *w = &weapon_stack[ il_get(qt, i, 0) ];
      if (weapon_isFlag(w,WEAPON_FLAG_DESTROYED))
         continue;
      if ((p!=NULL) && !weapon_isHostile(w,p))
         continue;
      if (vec2_dist2( &w->solid.pos, pos ) > r2 )
         continue;
      lua_pushmunition( L, w );
      lua_rawseti( L, -2, n++ );
   }
   return 1;
}

/**
 * @brief Gets the position of the munition.
 *
 *    @luatparam Munition m Munition to get property of.
 *    @luatreturn Vec2 Position of the munition.
 * @luafunc pos
 */
static int munitionL_pos( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushvector( L, w->solid.pos );
   return 1;
}

/**
 * @brief Gets the velocity of the munition.
 *
 *    @luatparam Munition m Munition to get property of.
 *    @luatreturn Vec2 Velocity of the munition.
 * @luafunc vel
 */
static int munitionL_vel( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushvector( L, w->solid.vel );
   return 1;
}

/**
 * @brief Gets the faction of the munition.
 *
 *    @luatparam Munition m Munition to get property of.
 *    @luatreturn Faction Faction of the munition.
 * @luafunc faction
 */
static int munitionL_faction( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushfaction( L, w->faction );
   return 1;
}

/**
 * @brief Gets the parent of the munition.
 *
 *    @luatparam Munition m Munition to get property of.
 *    @luatreturn Pilot Parent of the munition.
 * @luafunc parent
 */
static int munitionL_parent( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushpilot( L, w->parent );
   return 1;
}

/**
 * @brief Gets the target of the munition.
 *
 *    @luatparam Munition m Munition to get property of.
 *    @luatreturn Pilot Target of the munition.
 * @luafunc target
 */
static int munitionL_target( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushpilot( L, w->parent );
   return 1;
}

/**
 * @brief Gets the outfit corresponding to the munition.
 *
 *    @luatparam Munition m Munition to get corresponding outfit of.
 *    @luatreturn Outfit The outfit corresponding to the munition.
 * @luafunc outfit
 */
static int munitionL_outfit( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushoutfit( L, w->outfit );
   return 1;
}

/**
 * @brief Gets the strength of a munition.
 *
 * Defaults to 1. and only changed when past falloff range or modified by a Lua script.
 *
 *    @luatparam Munition m Munition to get strength of.
 *    @luatreturn number The corresponding strength value where 1 indicates normal strength.
 * @luafunc strength
 * @see strengthSet
 */
static int munitionL_strength( lua_State *L )
{
   const Weapon *w = luaL_validmunition( L, 1 );
   lua_pushnumber( L, w->strength );
   return 1;
}

/**
 * @brief Sets the strength of a munition.
 *
 *    @luatparam Munition m Munition to get strength of.
 *    @luatparam number str Strength to set to. A value of 1 indicates normal strength.
 * @luafunc strengthSet
 * @see strength
 */
static int munitionL_strengthSet( lua_State *L )
{
   Weapon *w = luaL_validmunition( L, 1 );
   double sb = w->strength_base;
   w->strength_base = luaL_checknumber( L, 2 );
   w->strength *= w->strength_base / sb;
   return 1;
}
