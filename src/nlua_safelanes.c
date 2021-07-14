/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_safelanes.c
 *
 * @brief Handles factions' safe lanes through systems.
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua_safelanes.h"

#include "array.h"
#include "nlua_faction.h"
#include "nlua_jump.h"
#include "nlua_planet.h"
#include "nlua_system.h"
#include "nluadef.h"
#include "safelanes.h"


/* diffs */
static int safelanesL_get( lua_State *L );
static const luaL_Reg safelanesL_methods[] = {
   { "get", safelanesL_get },
   {0,0}
}; /**< Safelane Lua methods. */


/**
 * @brief Loads the safelanes Lua library.
 *    @param env Lua enviornment.
 *    @return 0 on success.
 */
int nlua_loadSafelanes( nlua_env env )
{
   nlua_register(env, "safelanes", safelanesL_methods, 0);
   return 0;
}


/**
 * @brief Lua accessor functions to safe lane information.
 *
 * A "safe lane" is an intra-system route patrolled by a faction.
 *
 * This module is in development; its interface should not be considered final.
 *
 * @luamod safelanes
 */
/**
 * @brief Return a table of matching lanes (format described below).
 *
 * @usage safelanes.get() -- Everyone's in current system
 * @usage safelanes.get( "Empire" ) -- Empire's lanes in current system
 * @usage safelanes.get( faction.get("Empire"), system.get("Gamma Polaris") ) -- Empire's lanes through Gamma Polaris.
 *    @luatparam[opt] Faction f If present, only return this faction's lanes.
 *    @luatparam[opt=system.cur()] System s The system whose lanes we want.
 *    @luatreturn table The list of matching safe lanes, each of which is a table where:
 *                lane[1] and lane[2] are the endpoints (type Planet|Jump),
 *                and lane.faction is the owner's Faction.
 * @luafunc get
 */
static int safelanesL_get( lua_State *L )
{
   int i, j, faction;
   StarSystem *sys;
   SafeLane *lanes;
   LuaJump jump;

   if (lua_gettop( L ) >= 1 && !lua_isnil( L, 1 ))
      faction = luaL_validfaction( L, 1 );
   else
      faction = -1;

   if (lua_gettop( L ) >= 2 && !lua_isnil( L, 2 ))
      sys = luaL_validsystem( L, 2 );
   else
      sys = cur_system;

   lanes = safelanes_get( faction, sys );
   lua_newtable( L );
   for (i=0; i<array_size(lanes); i++) {
      lua_newtable( L );
      for (j=0; j<2; j++) {
         switch (lanes[i].point_type[j]) {
            case SAFELANE_LOC_PLANET:
               lua_pushplanet( L, lanes[i].point_id[j] );
               break;
            case SAFELANE_LOC_DEST_SYS:
               jump.srcid = system_index( sys );
               jump.destid = lanes[i].point_id[j];
               lua_pushjump( L, jump );
               break;
            default:
               NLUA_ERROR( L, _("What the?") );
         }
         lua_rawseti( L, -2, j+1 );
      }
      lua_pushstring( L, "faction" ); /* key */
      lua_pushfaction( L, lanes[i].faction ); /* value */
      lua_rawset( L, -3 );
      lua_rawseti( L, -2, i+1 );
   }
   array_free( lanes );

   return 1;
}
