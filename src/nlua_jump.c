/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file nlua_jump.c
 *
 * @brief Lua jump module.
 */


#include "nlua_jump.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_vec2.h"
#include "nlua_system.h"
#include "land_outfits.h"
#include "log.h"


static JumpPoint* luaL_validjumpSystem( lua_State *L, int ind, int *offset, StarSystem **sys );


/* Jump metatable methods */
static int jumpL_get( lua_State *L );
static int jumpL_eq( lua_State *L );
static int jumpL_position( lua_State *L );
static int jumpL_angle( lua_State *L );
static int jumpL_hidden( lua_State *L );
static int jumpL_exitonly( lua_State *L );
static int jumpL_system( lua_State *L );
static int jumpL_dest( lua_State *L );
static int jumpL_isKnown( lua_State *L );
static int jumpL_setKnown( lua_State *L );
static const luaL_reg jump_methods[] = {
   { "get", jumpL_get },
   { "__eq", jumpL_eq },
   { "pos", jumpL_position },
   { "angle", jumpL_angle },
   { "hidden", jumpL_hidden },
   { "exitonly", jumpL_exitonly },
   { "system", jumpL_system },
   { "dest", jumpL_dest },
   { "known", jumpL_isKnown },
   { "setKnown", jumpL_setKnown },
   {0,0}
}; /**< Jump metatable methods. */
static const luaL_reg jump_cond_methods[] = {
   { "get", jumpL_get },
   { "__eq", jumpL_eq },
   { "pos", jumpL_position },
   { "angle", jumpL_angle },
   { "hidden", jumpL_hidden },
   { "exitonly", jumpL_exitonly },
   { "system", jumpL_system },
   { "dest", jumpL_dest },
   { "known", jumpL_isKnown },
   {0,0}
}; /**< Read only jump metatable methods. */


/**
 * @brief Loads the jump library.
 *
 *    @param L State to load jump library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadJump( lua_State *L, int readonly )
{
   /* Create the metatable */
   luaL_newmetatable(L, JUMP_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, jump_cond_methods);
   else
      luaL_register(L, NULL, jump_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, JUMP_METATABLE);

   return 0; /* No error */
}


/**
 * @brief This module allows you to handle the jumps from Lua.
 *
 * Generally you do something like:
 *
 * @code
 * j = jump.get("Gamma Polaris", "Apez") -- Get the jump from Gamma Polaris to Apez
 * if j:known() then -- The jump is known
 *    v = j:pos() -- Get the position
 *    -- Do other stuff
 * end
 * @endcode
 *
 * @luamod jump
 */
/**
 * @brief Gets jump at index.
 *
 *    @param L Lua state to get jump from.
 *    @param ind Index position to find the jump.
 *    @return Jump found at the index in the state.
 */
LuaJump* lua_tojump( lua_State *L, int ind )
{
   return (LuaJump*) lua_touserdata(L,ind);
}
/**
 * @brief Gets jump at index raising an error if isn't a jump.
 *
 *    @param L Lua state to get jump from.
 *    @param a Index to check.
 *    @return Jump found at the index in the state.
 */
LuaJump* luaL_checkjump( lua_State *L, int ind )
{
   if (lua_isjump(L,ind))
      return lua_tojump(L,ind);
   luaL_typerror(L, ind, JUMP_METATABLE);
   return NULL;
}


/**
 * @brief Back-end for luaL_validjump.
 *
 *    @param L Lua state to get jump from.
 *    @param ind Index to check.
 *    @param[out] offset How many Lua arguments were passed.
 *    @param[out] sys System the jump exists in.
 *    @return Jump found at the index in the state.
 *
 * @sa luaL_validjump
 */
static JumpPoint* luaL_validjumpSystem( lua_State *L, int ind, int *offset, StarSystem **outsys )
{
   LuaJump *lj;
   JumpPoint *jp;
   StarSystem *a, *b;

   /* Defaults. */
   jp = NULL;
   a = NULL;
   b = NULL;

   if (lua_isjump(L, ind)) {
      lj = luaL_checkjump(L, ind);
      a = system_getIndex( lj->srcid );
      b = system_getIndex( lj->destid );
      if (offset != NULL)
         *offset = 1;
   }
   else if (lua_gettop(L) > 1) {
      if (lua_isstring(L, ind))
         a = system_get( lua_tostring( L, ind ));
      else if (lua_issystem(L, ind))
         a = system_getIndex( lua_tosystem(L, ind)->id );

      if (lua_isstring(L, ind+1))
         b = system_get( lua_tostring( L, ind+1 ));
      else if (lua_issystem(L, ind+1))
         b = system_getIndex( lua_tosystem(L, ind+1)->id );

      if (offset != NULL)
         *offset = 2;
   }
   else {
      luaL_typerror(L, ind, JUMP_METATABLE);
      return NULL;
   }

   if (b != NULL && a != NULL)
         jp = jump_getTarget( b, a );

   if (jp == NULL)
      NLUA_ERROR(L, "Jump is invalid");

   if (outsys != NULL)
      *outsys = a;
   return jp;
}


/**
 * @brief Gets a jump directly.
 *
 *    @param L Lua state to get jump from.
 *    @param ind Index to check.
 *    @param[out] sys System the jump exists in.
 *    @return Jump found at the index in the state.
 */
JumpPoint* luaL_validjump( lua_State *L, int ind )
{
   return luaL_validjumpSystem(L, ind, NULL, NULL);
}


/**
 * @brief Pushes a jump on the stack.
 *
 *    @param L Lua state to push jump into.
 *    @param jump Jump to push.
 *    @return Newly pushed jump.
 */
LuaJump* lua_pushjump( lua_State *L, LuaJump jump )
{
   LuaJump *j;
   j = (LuaJump*) lua_newuserdata(L, sizeof(LuaJump));
   *j = jump;
   luaL_getmetatable(L, JUMP_METATABLE);
   lua_setmetatable(L, -2);
   return j;
}
/**
 * @brief Checks to see if ind is a jump.
 *
 *    @param L Lua state to check.
 *    @param ind Index to check.
 *    @return 1 if ind is a jump.
 */
int lua_isjump( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, JUMP_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Gets a jump.
 *
 * Possible values of params: <br/>
 *    - string : Gets the jump by system name. <br/>
 *    - system : Gets the jump by system. <br/>
 *
 * @usage j,r  = jump.get( "Ogat", "Goddard" ) -- Returns the Ogat to Goddard and Goddard to Ogat jumps.
 *    @luaparam param See description.
 *    @luareturn Returns the jump and the inverse (where it exits).
 * @luafunc get( param )
 */
static int jumpL_get( lua_State *L )
{
   LuaJump lj;
   StarSystem *a, *b;

   /* Defaults. */
   a = NULL;
   b = NULL;

   if (lua_gettop(L) > 1) {
      if (lua_isstring(L, 1))
         a = system_get( lua_tostring(L, 1));
      else if (lua_issystem(L, 1))
         a = system_getIndex( lua_tosystem(L, 1)->id );

      if (lua_isstring(L, 2))
         b = system_get( lua_tostring(L, 2));
      else if (lua_issystem(L, 2))
         b = system_getIndex( lua_tosystem(L, 2)->id );

      if ((a == NULL) || (b == NULL)) {
         NLUA_ERROR(L, "No matching jump points found.");
         return 0;
      }

      if (jump_getTarget(b, a) != NULL) {
         lj.srcid  = a->id;
         lj.destid = b->id;
         lua_pushjump(L, lj);

         /* The inverse. If it doesn't exist, there are bigger problems. */
         lj.srcid  = b->id;
         lj.destid = a->id;
         lua_pushjump(L, lj);
         return 2;
      }
   }
   else
      NLUA_INVALID_PARAMETER(L);

   return 0;
}


/**
 * @brief You can use the '=' operator within Lua to compare jumps with this.
 *
 * @usage if j:__eq( jump.get( "Rhu", "Ruttwi" ) ) then -- Do something
 *    @luaparam j Jump comparing.
 *    @luaparam comp jump to compare against.
 *    @luareturn true if both jumps are the same.
 * @luafunc __eq( j, comp )
 */
static int jumpL_eq( lua_State *L )
{
   LuaJump *a, *b;
   a = luaL_checkjump(L,1);
   b = luaL_checkjump(L,2);
   lua_pushboolean(L,((a->srcid == b->srcid) && (a->destid == b->destid)));
   return 1;
}


/**
 * @brief Gets the position of the jump in the system.
 *
 * @usage v = j:pos()
 *    @luaparam j Jump to get the position of.
 *    @luareturn The position of the jump in the system as a vec2.
 * @luafunc pos( j )
 */
static int jumpL_position( lua_State *L )
{
   JumpPoint *jp;
   LuaVector v;
   jp = luaL_validjump(L,1);
   vectcpy(&v.vec, &jp->pos);
   lua_pushvector(L, v);
   return 1;
}


/**
 * @brief Gets the angle of a jump in degrees.
 *
 * @usage v = j:angle()
 *    @luaparam j Jump to get the angle of.
 *    @luareturn The angle.
 * @luafunc angle( j )
 */
static int jumpL_angle( lua_State *L )
{
   JumpPoint *jp;

   jp = luaL_validjump(L,1);
   lua_pushnumber(L, jp->angle * 180. / M_PI);
   return 1;
}


/**
 * @brief Checks whether a jump is hidden.
 *
 * @usage if not j:hidden() then -- Exclude hidden jumps.
 *    @luaparam j Jump to get the hidden status of.
 *    @luareturn Whether the jump is hidden.
 * @luafunc hidden( j )
 */
static int jumpL_hidden( lua_State *L )
{
   JumpPoint *jp;
   jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isFlag(jp, JP_HIDDEN) );
   return 1;
}


/**
 * @brief Checks whether a jump is exit-only.
 *
 * @usage if jump.exitonly("Eneguoz", "Zied") then -- The jump point in Eneguoz cannot be entered.
 *    @luaparam j Jump to get the exit-only status of.
 *    @luareturn Whether the jump is exit-only.
 * @luafunc exitonly( j )
 */
static int jumpL_exitonly( lua_State *L )
{
   JumpPoint *jp;
   jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isFlag(jp, JP_EXITONLY) );
   return 1;
}


/**
 * @brief Gets the system that a jump point exists in.
 *
 * @usage s = j:system()
 *    @luaparam j Jump to get the system of.
 *    @luareturn The jump's system.
 * @luafunc system( j )
 */
static int jumpL_system( lua_State *L )
{
   StarSystem *sys;
   LuaSystem ls;

   luaL_validjumpSystem(L, 1, NULL, &sys);
   ls.id = sys->id;
   lua_pushsystem(L,ls);
   return 1;
}


/**
 * @brief Gets the system that a jump point exits into.
 *
 * @usage v = j:dest()
 *    @luaparam j Jump to get the destination of.
 *    @luareturn The jump's destination system.
 * @luafunc dest( j )
 */
static int jumpL_dest( lua_State *L )
{
   JumpPoint *jp;
   LuaSystem ls;

   jp = luaL_validjump(L,1);
   ls.id = jp->targetid;
   lua_pushsystem(L,ls);
   return 1;
}


/**
 * @brief Checks to see if a jump is known by the player.
 *
 * @usage b = j:known()
 *
 *    @luaparam s Jump to check if the player knows.
 *    @luareturn true if the player knows the jump.
 * @luafunc known( j )
 */
static int jumpL_isKnown( lua_State *L )
{
   JumpPoint *jp;

   jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isKnown(jp));
   return 1;
}

/**
 * @brief Sets a jump's known state.
 *
 * @usage j:setKnown( false ) -- Makes jump unknown.
 *    @luaparam j Jump to set known.
 *    @luaparam b Whether or not to set as known (defaults to true).
 * @luafunc setKnown( j, b )
 */
static int jumpL_setKnown( lua_State *L )
{
   int b, offset, changed;
   JumpPoint *jp;

   jp = luaL_validjumpSystem(L, 1, &offset, NULL);

   /* True if boolean isn't supplied. */
   if (lua_gettop(L) > offset)
      b  = lua_toboolean(L, 1 + offset);
   else
      b = 1;

   changed = (b != (int)jp_isKnown(jp));

   if (b)
      jp_setFlag( jp, JP_KNOWN );
   else
      jp_rmFlag( jp, JP_KNOWN );

   /* Update outfits image array. */
   if (changed)
      outfits_updateEquipmentOutfits();

   return 0;
}
