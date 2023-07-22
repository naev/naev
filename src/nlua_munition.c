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
#include "nlua_outfit.h"

/* Prototypes. */
static Weapon *munition_get( LuaMunition *lm );

/* Munition metatable methods. */
static int munitionL_eq( lua_State *L );
static int munitionL_tostring( lua_State *L );
static int munitionL_clear( lua_State *L );
static int munitionL_getAll( lua_State *L );
static int munitionL_outfit( lua_State *L );
static const luaL_Reg munitionL_methods[] = {
   /* General. */
   { "__eq", munitionL_eq },
   { "__tostring", munitionL_tostring },
   { "clear", munitionL_clear },
   { "getAll", munitionL_getAll },
   { "outfit", munitionL_outfit },
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
 *    @luatreturn table A table containing all the munitions in the system.
 * @luafunc getAll
 */
static int munitionL_getAll( lua_State *L )
{
   const Weapon *weapon_stack = weapon_getStack();
   int n = 1;
   lua_newtable(L);
   for (int i=0; i<array_size(weapon_stack); i++) {
      const Weapon *w = &weapon_stack[i];
      if (weapon_isFlag(w,WEAPON_FLAG_DESTROYED))
         continue;

      lua_pushmunition( L, w );
      lua_rawseti( L, -2, n++ );
   }
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
   Weapon *w = luaL_validmunition( L, 1 );
   lua_pushoutfit( L, w->outfit );
   return 1;
}
