/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_ship.c
 *
 * @brief Handles the Lua ship bindings.
 */

#include "nlua_ship.h"
#include "slots.h"

#include "naev.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "nlua_outfit.h"
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
static int shipL_getSlots( lua_State *L );
static int shipL_CPU( lua_State *L );
static int shipL_gfxTarget( lua_State *L );
static int shipL_gfx( lua_State *L );
static int shipL_price( lua_State *L );
static const luaL_reg shipL_methods[] = {
   { "__tostring", shipL_name },
   { "__eq", shipL_eq },
   { "get", shipL_get },
   { "name", shipL_name },
   { "baseType", shipL_baseType },
   { "class", shipL_class },
   { "slots", shipL_slots },
   { "getSlots", shipL_getSlots },
   { "cpu", shipL_CPU },
   { "price", shipL_price },
   { "gfxTarget", shipL_gfxTarget },
   { "gfx", shipL_gfx },
   {0,0}
}; /**< Ship metatable methods. */




/**
 * @brief Loads the ship library.
 *
 *    @param env Environment to load ship library into.
 *    @return 0 on success.
 */
int nlua_loadShip( nlua_env env )
{
   nlua_register(env, SHIP_METATABLE, shipL_methods, 1);
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
Ship* lua_toship( lua_State *L, int ind )
{
   return *((Ship**) lua_touserdata(L,ind));
}
/**
 * @brief Gets ship at index or raises error if there is no ship at index.
 *
 *    @param L Lua state to get ship from.
 *    @param ind Index position to find ship.
 *    @return Ship found at the index in the state.
 */
Ship* luaL_checkship( lua_State *L, int ind )
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
   Ship *s;

   if (lua_isship(L, ind))
      s = luaL_checkship(L,ind);
   else if (lua_isstring(L, ind))
      s = ship_get( lua_tostring(L, ind) );
   else {
      luaL_typerror(L, ind, SHIP_METATABLE);
      return NULL;
   }

   if (s == NULL)
      NLUA_ERROR(L, "Ship is invalid.");

   return s;
}
/**
 * @brief Pushes a ship on the stack.
 *
 *    @param L Lua state to push ship into.
 *    @param ship Ship to push.
 *    @return Newly pushed ship.
 */
Ship** lua_pushship( lua_State *L, Ship *ship )
{
   Ship **p;
   p = (Ship**) lua_newuserdata(L, sizeof(Ship*));
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
 *    @luatparam Ship s1 First ship to compare.
 *    @luatparam Ship s2 Second ship to compare.
 *    @luatreturn boolean true if both ships are the same.
 * @luafunc __eq( s1, s2 )
 */
static int shipL_eq( lua_State *L )
{
   Ship *a, *b;
   a = luaL_checkship(L,1);
   b = luaL_checkship(L,2);
   if (a == b)
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
 *    @luatparam string s Name of the ship to get.
 *    @luatreturn Ship The ship matching name or nil if error.
 * @luafunc get( s )
 */
static int shipL_get( lua_State *L )
{
   const char *name;
   Ship *ship;

   /* Handle parameters. */
   name = luaL_checkstring(L,1);

   /* Get ship. */
   ship = ship_get( name );
   if (ship == NULL) {
      NLUA_ERROR(L,"Ship '%s' not found!", name);
      return 0;
   }

   /* Push. */
   lua_pushship(L, ship);
   return 1;
}
/**
 * @brief Gets the name of the ship.
 *
 * @usage shipname = s:name()
 *
 *    @luatparam Ship s Ship to get ship name.
 *    @luatreturn string The name of the ship.
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
 *    @luatparam Ship s Ship to get the ship base type.
 *    @luatreturn string The name of the ship base type.
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
 * @brief Gets the name of the ship's class.
 *
 * @usage shipclass = s:class()
 *
 *    @luatparam Ship s Ship to get ship class name.
 *    @luatreturn string The name of the ship's class.
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
 * @brief Gets the amount of the ship's slots.
 *
 * @usage slots_weapon, slots_utility, slots_structure = p:slots()
 *
 *    @luatparam Ship s Ship to get ship slots of.
 *    @luatreturn number Number of weapon slots.
 *    @luatreturn number Number of utility slots.
 *    @luatreturn number Number of structure slots.
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
 * @brief Get a table of slots of a ship, where a slot is a table with a string size, type, and property
 *
 * @usage for _,v in ipairs( ship.getSlots( ship.get("Llama") ) ) do print(v["type"]) end
 *
 *    @luaparam s Ship to get slots of
 *    @luareturn A table of tables with slot properties string "size", string "type", and string "property"
 * @luafunc getSlots( s )
 */
static int shipL_getSlots( lua_State *L )
{
   int i, j, k, outfit_type;
   Ship *s;
   OutfitSlot *slot;
   ShipOutfitSlot *sslot;
   char *outfit_types[] = {"structure", "utility", "weapon"};

   s = luaL_validship(L,1);

   lua_newtable(L);
   k=1;
   for (i=0; i < s->outfit_nstructure + s->outfit_nutility + s->outfit_nweapon ; i++){

         /* get the slot */
      if (i < s->outfit_nstructure){
         j     = i;
         slot  = &s->outfit_structure[j].slot;
         sslot = &s->outfit_structure[j];
         outfit_type = 0;
      }
      else if (i < s->outfit_nstructure + s->outfit_nutility){
         j     = i - s->outfit_nstructure;
         slot  = &s->outfit_utility[j].slot;
         sslot = &s->outfit_utility[j];
         outfit_type = 1;
      }
      else {
         j     = i - (s->outfit_nstructure + s->outfit_nutility);
         slot  = &s->outfit_weapon[j].slot;
         sslot = &s->outfit_weapon[j];
         outfit_type = 2;
      }

      /* make the slot table and put it in */
      lua_pushnumber(L, k++); /* slot table key */
      lua_newtable(L);

      lua_pushstring(L, "type"); /* key */
      lua_pushstring(L, outfit_types[outfit_type]); /* value */
      lua_rawset(L, -3); /* table[key = value ]*/

      lua_pushstring(L, "size"); /* key */
      lua_pushstring(L, slotSize( slot->size) );
      lua_rawset(L, -3); /* table[key] = value */

      lua_pushstring(L, "property"); /* key */
      lua_pushstring( L, sp_display(slot->spid)); /* value */
      lua_rawset(L, -3); /* table[key] = value */

      if (sslot->data != NULL) {
         lua_pushstring(L, "outfit"); /* key */
         lua_pushoutfit(L, sslot->data); /* value*/
         lua_rawset(L, -3); /* table[key] = value */
      }

      lua_rawset(L, -3);   /* put the slot table in */
   }

   return 1;

}


/**
 * @brief Gets the ship available CPU.
 *
 * @usage cpu_left = s:cpu()
 *
 *    @luatparam Ship s Ship to get available CPU of.
 *    @luatreturn number The CPU available on the ship.
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
 * @brief Gets the ship's price, with and without default outfits.
 *
 * @usage price, base = s:price()
 *
 *    @luatparam Ship s Ship to get the price of.
 *    @luatreturn number The ship's final purchase price.
 *    @luatreturn number The ship's base price.
 * @luafunc price( s )
 */
static int shipL_price( lua_State *L )
{
   Ship *s;

   s = luaL_validship(L,1);
   lua_pushnumber(L, ship_buyPrice(s));
   lua_pushnumber(L, ship_basePrice(s));
   return 2;
}


/**
 * @brief Gets the ship's target graphics.
 *
 * Will not work without access to the Tex module.
 *
 * @usage gfx = s:gfxTarget()
 *
 *    @luatparam Ship s Ship to get target graphics of.
 *    @luatreturn Tex The target graphics of the ship.
 * @luafunc gfxTarget( s )
 */
static int shipL_gfxTarget( lua_State *L )
{
   Ship *s;
   glTexture *tex;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Push graphic. */
   tex = gl_dupTexture( s->gfx_target );
   if (tex == NULL) {
      WARN("Unable to get ship target graphic for '%s'.", s->name);
      return 0;
   }
   lua_pushtex( L, tex );
   return 1;
}


/**
 * @brief Gets the ship's graphics.
 *
 * Will not work without access to the Tex module. These are nearly always a sprite sheet.
 *
 * @usage gfx = s:gfx()
 *
 *    @luatparam Ship s Ship to get graphics of.
 *    @luatreturn Tex The graphics of the ship.
 * @luafunc gfx( s )
 */
static int shipL_gfx( lua_State *L )
{
   Ship *s;
   glTexture *tex;

   /* Get the ship. */
   s  = luaL_validship(L,1);

   /* Push graphic. */
   tex = gl_dupTexture( s->gfx_space );
   if (tex == NULL) {
      WARN("Unable to get ship graphic for '%s'.", s->name);
      return 0;
   }
   lua_pushtex( L, tex );
   return 1;
}

