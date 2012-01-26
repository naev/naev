/*
 * See Licensing and Copyright notice in naev.h
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
#include "log.h"


/* Jump metatable methods */
static int jumpL_get( lua_State *L );
static int jumpL_eq( lua_State *L );
static int jumpL_position( lua_State *L );
static int jumpL_isKnown( lua_State *L );
static int jumpL_setKnown( lua_State *L );
static const luaL_reg jump_methods[] = {
   { "get", jumpL_get },
   { "__eq", jumpL_eq },
   { "pos", jumpL_position },
   { "isKnown", jumpL_isKnown },
   { "setKnown", jumpL_setKnown },
   {0,0}
}; /**< Jump metatable methods. */


/**
 * @brief Loads the jump library.
 *
 *    @param L State to load jump library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadJump( lua_State *L, int readonly )
{
   (void) readonly;
   /* Create the metatable */
   luaL_newmetatable(L, JUMP_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
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
 * if j:isKnown() then -- The jump is known
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
 * @brief Gets a jump directly.
 *
 *    @param L Lua state to get jump from.
 *    @param ind Index to check.
 *    @return Jump found at the index in the state.
 */
JumpPoint* luaL_validjump( lua_State *L, int ind )
{
   LuaJump *lj;
   JumpPoint *jp;
   StarSystem *a, *b;

   if (lua_isjump(L, ind)) {
      lj = luaL_checkjump(L, ind);
      a = system_getIndex( lj->sysid );
      jp = &a->jumps[lj->id];
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

      if (b != NULL && a != NULL)
         jp = jump_get( b->name, a );
   }
   else {
      luaL_typerror(L, ind, JUMP_METATABLE);
      return NULL;
   }

   if (jp == NULL)
      NLUA_ERROR(L, "Jump is invalid");

   return jp;
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
 * @usage j  = jump.get( "Ogat", "Goddard" ) -- Returns the Ogat to Goddard jump.
 *    @luaparam param See description.
 *    @luareturn Returns the jump and the it belongs to.
 * @luafunc get( param )
 */
static int jumpL_get( lua_State *L )
{
   LuaJump lj;
   int i;
   StarSystem *a, *b;

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
      
      lj.sysid = a->id;
      for (i=0; i<a->njumps; i++) {
         if (a->jumps[i].targetid == b->id) {
            lj.id = i;
            lua_pushjump(L, lj);
            return 1;
         }
      }
   }
   else
      NLUA_INVALID_PARAMETER(L);

   return 0;
}


/**
 * @brief You can use the '=' operator within Lua to compare jumps with this.
 *
 * @usage if j.__eq( jump.get( "Rhu", "Ruttwi" ) ) then -- Do something
 *    @luaparam j Jump comparing.
 *    @luaparam comp jump to compare against.
 *    @luareturn true if both jumps are the same.
 * @luafunc __eq( p, comp )
 */
static int jumpL_eq( lua_State *L )
{
   LuaJump *a, *b;
   a = luaL_checkjump(L,1);
   b = luaL_checkjump(L,2);
   lua_pushboolean(L,(a->id == b->id));
   return 1;
}


/**
 * @brief Gets the position of the jump in the system.
 *
 * @usage v = p:pos()
 *    @luaparam p Jump to get the position of.
 *    @luareturn The position of the jump in the system as a vec2.
 * @luafunc pos( p )
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
 * @brief Checks to see if a jump is known by the player.
 *
 * @usage b = p:isKnown()
 *
 *    @luaparam s Jump to check if the player knows.
 *    @luareturn true if the player knows the jump.
 * @luafunc isKnown( p )
 */
static int jumpL_isKnown( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isKnown(jp));
   return 1;
}

/**
 * @brief Sets a jumps's known state.
 *
 * @usage p:setKnown( false ) -- Makes jump unknown.
 *    @luaparam p Jump to set known.
 *    @luaparam b Whether or not to set as known (defaults to false).
 * @luafunc setKnown( p, b )
 */
static int jumpL_setKnown( lua_State *L )
{
   int b;
   JumpPoint *jp;

   p = luaL_validjump(L,1);
   b = lua_toboolean(L, 2);

   if (b)
      jp_setFlag( jp, JP_KNOWN );
   else
      jp_rmFlag( jp, JP_KNOWN );
   return 0;
}
