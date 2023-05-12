/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_jump.c
 *
 * @brief Lua jump module.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_jump.h"

#include "nluadef.h"
#include "nlua_vec2.h"
#include "nlua_system.h"
#include "land_outfits.h"
#include "map_overlay.h"
#include "log.h"

RETURNS_NONNULL static JumpPoint *luaL_validjumpSystem( lua_State *L, int ind, int *offset );

/* Jump metatable methods */
static int jumpL_get( lua_State *L );
static int jumpL_eq( lua_State *L );
static int jumpL_tostring( lua_State *L );
static int jumpL_radius( lua_State *L );
static int jumpL_position( lua_State *L );
static int jumpL_angle( lua_State *L );
static int jumpL_hidden( lua_State *L );
static int jumpL_exitonly( lua_State *L );
static int jumpL_system( lua_State *L );
static int jumpL_dest( lua_State *L );
static int jumpL_isKnown( lua_State *L );
static int jumpL_setKnown( lua_State *L );
static const luaL_Reg jump_methods[] = {
   { "get", jumpL_get },
   { "__eq", jumpL_eq },
   { "__tostring", jumpL_tostring },
   { "radius", jumpL_radius },
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

/**
 * @brief Loads the jump library.
 *
 *    @param env Environment to load jump library into.
 *    @return 0 on success.
 */
int nlua_loadJump( nlua_env env )
{
   nlua_register(env, JUMP_METATABLE, jump_methods, 1);
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
 *    @param ind Index to check.
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
 *    @return Jump found at the index in the state.
 *
 * @sa luaL_validjump
 */
static JumpPoint *luaL_validjumpSystem( lua_State *L, int ind, int *offset )
{
   JumpPoint *jp;
   StarSystem *a, *b;

   /* Defaults. */
   jp = NULL;
   a = NULL;
   b = NULL;

   if (lua_isjump(L, ind)) {
      LuaJump *lj = luaL_checkjump(L, ind);
      a = system_getIndex( lj->srcid );
      b = system_getIndex( lj->destid );
      if (offset != NULL)
         *offset = 1;
   }
   else if (lua_gettop(L) > 1) {
      if (lua_isstring(L, ind))
         a = system_get( lua_tostring( L, ind ));
      else if (lua_issystem(L, ind))
         a = system_getIndex( lua_tosystem(L, ind) );

      if (lua_isstring(L, ind+1))
         b = system_get( lua_tostring( L, ind+1 ));
      else if (lua_issystem(L, ind+1))
         b = system_getIndex( lua_tosystem(L, ind+1) );

      if (offset != NULL)
         *offset = 2;
   }
   else {
      luaL_typerror(L, ind, JUMP_METATABLE);
      // noreturn
   }

   if ((b != NULL) && (a != NULL))
      jp = jump_getTarget( b, a );

   if (jp == NULL)
      NLUA_ERROR(L, _("Jump is invalid"));

   return jp;
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
   return luaL_validjumpSystem( L, ind, NULL );
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
   LuaJump *j = (LuaJump*) lua_newuserdata(L, sizeof(LuaJump));
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
 *    @luatparam string|System src See description.
 *    @luatparam string|System dest See description.
 *    @luatreturn Jump Returns the jump.
 *    @luatreturn Jump Returns the inverse.
 * @luafunc get
 */
static int jumpL_get( lua_State *L )
{
   LuaJump lj;
   StarSystem *a, *b;

   a = luaL_validsystem(L,1);
   b = luaL_validsystem(L,2);

   if ((a == NULL) || (b == NULL)) {
      NLUA_ERROR(L, _("No matching jump points found."));
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

   return 0;
}

/**
 * @brief You can use the '==' operator within Lua to compare jumps with this.
 *
 * @usage if j:__eq( jump.get( "Rhu", "Ruttwi" ) ) then -- Do something
 *    @luatparam Jump j Jump comparing.
 *    @luatparam Jump comp jump to compare against.
 *    @luatreturn boolean true if both jumps are the same.
 * @luafunc __eq
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
 * @brief Converts a jump to readable form. Mainly meant to be used for printing.
 *
 *    @luatparam Jump j Jump to print.
 * @luafunc __tostring
 */
static int jumpL_tostring( lua_State *L )
{
   char buf[STRMAX_SHORT];
   LuaJump *lj = luaL_checkjump(L,1);
   StarSystem *src = system_getIndex( lj->srcid );
   StarSystem *dst = system_getIndex( lj->destid );
   snprintf( buf, sizeof(buf), _("Jump( %s -> %s )"), _(src->name), _(dst->name) );
   lua_pushstring( L, buf );
   return 1;
}

/**
 * @brief Gets the jump's radius.
 *
 * @usage radius = j:radius()
 *    @luatparam Jump j Jump to get the radius of.
 *    @luatreturn number The jump's radius.
 * @luafunc radius
 */
static int jumpL_radius( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushnumber(L,jp->radius);
   return 1;
}

/**
 * @brief Gets the position of the jump in the system.
 *
 * @usage v = j:pos()
 *    @luatparam Jump j Jump to get the position of.
 *    @luatreturn Vec2 The position of the jump in the system.
 * @luafunc pos
 */
static int jumpL_position( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushvector(L, jp->pos);
   return 1;
}

/**
 * @brief Gets the angle of a jump in radians.
 *
 * @usage v = j:angle()
 *    @luatparam Jump j Jump to get the angle of.
 *    @luatreturn number The angle.
 * @luafunc angle
 */
static int jumpL_angle( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushnumber(L, jp->angle);
   return 1;
}

/**
 * @brief Checks whether a jump is hidden.
 *
 * @usage if not j:hidden() then -- Exclude hidden jumps.
 *    @luatparam Jump j Jump to get the hidden status of.
 *    @luatreturn boolean Whether the jump is hidden.
 * @luafunc hidden
 */
static int jumpL_hidden( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isFlag(jp, JP_HIDDEN) );
   return 1;
}

/**
 * @brief Checks whether a jump is exit-only.
 *
 * @usage if jump.exitonly("Eneguoz", "Zied") then -- The jump point in Eneguoz cannot be entered.
 *    @luatparam Jump j Jump to get the exit-only status of.
 *    @luatreturn boolean Whether the jump is exit-only.
 * @luafunc exitonly
 */
static int jumpL_exitonly( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isFlag(jp, JP_EXITONLY) );
   return 1;
}

/**
 * @brief Gets the system that a jump point exists in.
 *
 * @usage s = j:system()
 *    @luatparam Jump j Jump to get the system of.
 *    @luatreturn System The jump's system.
 * @luafunc system
 */
static int jumpL_system( lua_State *L )
{
   JumpPoint *jp = luaL_validjumpSystem( L, 1, NULL );
   lua_pushsystem( L, jp->from->id );
   return 1;
}

/**
 * @brief Gets the system that a jump point exits into.
 *
 * @usage v = j:dest()
 *    @luatparam Jump j Jump to get the destination of.
 *    @luatreturn System The jump's destination system.
 * @luafunc dest
 */
static int jumpL_dest( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushsystem(L,jp->targetid);
   return 1;
}

/**
 * @brief Checks to see if a jump is known by the player.
 *
 * @usage b = j:known()
 *
 *    @luatparam Jump j Jump to check if the player knows.
 *    @luatreturn boolean true if the player knows the jump.
 * @luafunc known
 */
static int jumpL_isKnown( lua_State *L )
{
   JumpPoint *jp = luaL_validjump(L,1);
   lua_pushboolean(L, jp_isKnown(jp));
   return 1;
}

/**
 * @brief Sets a jump's known state.
 *
 * @usage j:setKnown( false ) -- Makes jump unknown.
 *    @luatparam Jump j Jump to set known.
 *    @luatparam[opt=true] boolean value Whether or not to set as known.
 * @luafunc setKnown
 */
static int jumpL_setKnown( lua_State *L )
{
   int b, offset, changed;
   JumpPoint *jp;

   offset = 0;
   jp     = luaL_validjumpSystem( L, 1, &offset );

   /* True if boolean isn't supplied. */
   if (lua_gettop(L) > offset)
      b = lua_toboolean(L, 1 + offset);
   else
      b = 1;

   changed = (b != (int)jp_isKnown(jp));

   if (b)
      jp_setFlag( jp, JP_KNOWN );
   else
      jp_rmFlag( jp, JP_KNOWN );

   if (changed) {
      /* Update overlay. */
      ovr_refresh();
      /* Update outfits image array - in the case it changes map owned status. */
      outfits_updateEquipmentOutfits();
   }

   return 0;
}
