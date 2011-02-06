/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_ship.c
 *
 * @brief Handles the Lua ship bindings.
 */

#include "nlua_ship.h"

#include "naev.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "log.h"
#include "rng.h"


/* Ship metatable methods. */
static int shipL_eq( lua_State *L );
static int shipL_get( lua_State *L );
static int shipL_name( lua_State *L );
static int shipL_baseType( lua_State *L );
static int shipL_class( lua_State *L );
static int shipL_slots( lua_State *L );
static int shipL_CPU( lua_State *L );
static int shipL_outfitCPU( lua_State *L );
static int shipL_gfxTarget( lua_State *L );
static int shipL_gfx( lua_State *L );
static const luaL_reg shipL_methods[] = {
   { "__tostring", shipL_name },
   { "__eq", shipL_eq },
   { "get", shipL_get },
   { "name", shipL_name },
   { "baseType", shipL_baseType },
   { "class", shipL_class },
   { "slots", shipL_slots },
   { "cpu", shipL_CPU },
   { "outfitCPU", shipL_outfitCPU },
   { "gfxTarget", shipL_gfxTarget },
   { "gfx", shipL_gfx },
   {0,0}
}; /**< Ship metatable methods. */




/**
 * @brief Loads the ship library.
 *
 *    @param L State to load ship library into.
 *    @return 0 on success.
 */
int nlua_loadShip( lua_State *L, int readonly )
{
   (void) readonly; /* Everything is readonly. */

   /* Create the metatable */
   luaL_newmetatable(L, SHIP_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, shipL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, SHIP_METATABLE);

   return 0;
}


/**
 * @brief Lua bindings to interact with ships.
 *
 * This will allow you to create and manipulate ships in-game.
 *
 * An example would be:
 * @code
 * s = ship.get( "Empire Lancelot" ) -- Gets the ship
 * cpu_free = s:cpu() -- Gets the CPU
 * @endcode
 *
 * @luamod ship
 */
/**
 * @brief Gets ship at index.
 *
 *    @param L Lua state to get ship from.
 *    @param ind Index position to find the ship.
 *    @return Ship found at the index in the state.
 */
LuaShip* lua_toship( lua_State *L, int ind )
{
   return (LuaShip*) lua_touserdata(L,ind);
}
/**
 * @brief Gets ship at index or raises error if there is no ship at index.
 *
 *    @param L Lua state to get ship from.
 *    @param ind Index position to find ship.
 *    @return Ship found at the index in the state.
 */
LuaShip* luaL_checkship( lua_State *L, int ind )
{
   if (lua_isship(L,ind))
      return lua_toship(L,ind);
   luaL_typerror(L, ind, SHIP_METATABLE);
   return NULL;
}
/**
 * @brief Makes sure the ship is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the ship to validate.
 *    @return The ship (doesn't return if fails - raises Lua error ).
 */
Ship* luaL_validship( lua_State *L, int ind )
{
   LuaShip *ls;
   Ship *s;

   /* Get the ship. */
   ls = luaL_checkship(L,ind);
   s  = ls->ship;
   if (s==NULL) {
      NLUA_ERROR(L,"Ship is invalid.");
      return NULL;
   }

   return s;
}
/**
 * @brief Pushes a ship on the stack.
 *
 *    @param L Lua state to push ship into.
 *    @param ship Ship to push.
 *    @return Newly pushed ship.
 */
LuaShip* lua_pushship( lua_State *L, LuaShip ship )
{
   LuaShip *p;
   p = (LuaShip*) lua_newuserdata(L, sizeof(LuaShip));
   *p = ship;
   luaL_getmetatable(L, SHIP_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}
/**
 * @brief Checks to see if ind is a ship.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a ship.
 */
int lua_isship( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SHIP_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Checks to see if two ships are the same.
 *
 * @usage if s1 == s2 then -- Checks to see if ship s1 and s2 are the same
 *
 *    @luaparam s1 First ship to compare.
 *    @luaparam s2 Second ship to compare.
 *    @luareturn true if both ships are the same.
 * @luafunc __eq( s1, s2 )
 */
static int shipL_eq( lua_State *L )
{
   LuaShip *a, *b;
   a = luaL_checkship(L,1);
   b = luaL_checkship(L,2);
   if (a->ship == b->ship)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}


/**
 * @brief Gets a ship.
 *
 * @usage s = ship.get( "Hyena" ) -- Gets the hyena
 *
 *    @luaparam s Name of the ship to get.
 *    @luareturn The ship matching name or nil if error.
 * @luafunc get( s )
 */
static int shipL_get( lua_State *L )
{
   const char *name;
   LuaShip ls;

   /* Handle paremeters. */
   name = luaL_checkstring(L,1);

   /* Get ship. */
   ls.ship = ship_get( name );
   if (ls.ship == NULL) {
      NLUA_ERROR(L,"Ship '%s' not found!", name);
      return 0;
   }

   /* Push. */
   lua_pushship(L, ls);
   return 1;
}
/**
 * @brief Gets the name of the ship's ship.
 *
 * @usage shipname = s:name()
 *
 *    @luaparam s Ship to get ship name.
 *    @luareturn The name of the ship's ship.
 * @luafunc name( s )
 */
static int shipL_name( lua_State *L )
{
   Ship *s;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /** Return the ship name. */
   lua_pushstring(L, s->name);
   return 1;
}


/**
 * @brief Gets the ship's base type.
 *
 * For example "Empire Lancelot" and "Lancelot" are both of the base type "Lancelot".
 *
 * @usage type = s:baseType()
 *
 *    @luaparam s Ship to get the ship base type.
 *    @luareturn The name of the ship base type.
 * @luafunc baseType( s )
 */
static int shipL_baseType( lua_State *L )
{
   Ship *s;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   lua_pushstring(L, s->base_type);
   return 1;
}


/**
 * @brief Gets the name of the ship's ship class.
 *
 * @usage shipclass = s:class()
 *
 *    @luaparam s Ship to get ship class name.
 *    @luareturn The name of the ship's ship class.
 * @luafunc class( s )
 */
static int shipL_class( lua_State *L )
{
   Ship *s;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   lua_pushstring(L, ship_class(s));
   return 1;
}


/**
 * @brief Gets the amount of the ship's ship slots.
 *
 * @usage slots_weapon, slots_utility, slots_structure = p:slots()
 *
 *    @luaparam s Ship to get ship slots of.
 *    @luareturn Number of weapon, utility and structure slots.
 * @luafunc slots( s )
 */
static int shipL_slots( lua_State *L )
{
   Ship *s;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Push slot numbers. */
   lua_pushnumber(L, s->outfit_nweapon);
   lua_pushnumber(L, s->outfit_nutility);
   lua_pushnumber(L, s->outfit_nstructure);
   return 3;
}


/**
 * @brief Gets the ship available CPU.
 *
 * @usage cpu_left = s:cpu()
 *
 *    @luaparam s Ship to get available CPU of.
 *    @luareturn The CPU available on the ship's ship.
 * @luafunc cpu( s )
 */
static int shipL_CPU( lua_State *L )
{
   Ship *s;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Get CPU. */
   lua_pushnumber(L, s->cpu);
   return 1;
}


/**
 * @brief Gets the outfit CPU usage.
 *
 * @usage cpu_used += s.outfitCPU( "Heavy Ion Turret" ) -- Adds the used cpu by the outfit
 *
 *    @luaparam outfit Name of the outfit to get CPU usage of.
 *    @luareturn CPU the outfit uses.
 * @luafunc outfitCPU( outfit )
 */
static int shipL_outfitCPU( lua_State *L )
{
   const char *outfit;
   Outfit *o;

   /* Get parameters. */
   outfit = luaL_checkstring(L,1);

   /* Get the outfit. */
   o = outfit_get( outfit );
   if (o == NULL)
      return 0;

   /* Return parameter. */
   lua_pushnumber(L, outfit_cpu(o));
   return 1;
}


/**
 * @brief Gets the ship's target graphics.
 *
 * Will not work without access to the Tex module.
 *
 * @usage gfx = s:gfxTarget()
 *
 *    @luaparam s Ship to get target graphics of.
 *    @luareturn The target graphics of the ship.
 * @luafunc gfxTarget( s )
 */
static int shipL_gfxTarget( lua_State *L )
{
   Ship *s;
   LuaTex lt;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Push graphic. */
   lt.tex = gl_dupTexture( s->gfx_target );
   if (lt.tex == NULL) {
      WARN("Unable to get ship target graphic for '%s'.", s->name);
      return 0;
   }
   lua_pushtex( L, lt );
   return 1;
}


/**
 * @brief Gets the ship's graphics.
 *
 * Will not work without access to the Tex module. These are nearly always a spritesheet.
 *
 * @usage gfx = s:gfx()
 *
 *    @luaparam s Ship to get graphics of.
 *    @luareturn The graphics of the ship.
 * @luafunc gfx( s )
 */
static int shipL_gfx( lua_State *L )
{
   Ship *s;
   LuaTex lt;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Push graphic. */
   lt.tex = gl_dupTexture( s->gfx_space );
   if (lt.tex == NULL) {
      WARN("Unable to get ship graphic for '%s'.", s->name);
      return 0;
   }
   lua_pushtex( L, lt );
   return 1;
}

