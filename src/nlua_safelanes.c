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
#include "collision.h"
#include "nlua_faction.h"
#include "nlua_jump.h"
#include "nlua_planet.h"
#include "nlua_system.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "safelanes.h"


/* diffs */
static int safelanesL_get( lua_State *L );
static int safelanesL_intersect( lua_State *L );
static const luaL_Reg safelanesL_methods[] = {
   { "get", safelanesL_get },
   { "intersect", safelanesL_intersect },
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
 *    @luatparam[opt] string standing What type of lanes to get. Either nil for specific faction only, "friendly", "neutral", "hostile", "non-friendly", or "non-hostile".
 *    @luatparam[opt=system.cur()] System s The system whose lanes we want.
 *    @luatreturn table The list of matching safe lanes, each of which is a table where:
 *                lane[1] and lane[2] are the endpoints (type vec2),
 *                and lane.faction is the owner's Faction.
 * @luafunc get
 */
static int safelanesL_get( lua_State *L )
{
   int i, j, faction, standing;
   StarSystem *sys;
   SafeLane *lanes;
   Planet *pnt;
   JumpPoint *jmp;
   const char *std;

   if (!lua_isnoneornil(L,1))
      faction = luaL_validfaction( L, 1 );
   else
      faction = -1;
   std = luaL_optstring(L, 2, NULL);
   if (!lua_isnoneornil(L,3))
      sys = luaL_validsystem( L, 3 );
   else
      sys = cur_system;

   /* Translate standing into number. */
   standing = 0;
   if ((faction >= 0) && (std != NULL)) {
      if (strcmp(std,"friendly")==0)
         standing |= SAFELANES_FRIENDLY;
      else if (strcmp(std,"neutral")==0)
         standing |= SAFELANES_NEUTRAL;
      else if (strcmp(std,"hostile")==0)
         standing |= SAFELANES_HOSTILE;
      else if (strcmp(std,"non-friendly")==0)
         standing |= SAFELANES_NEUTRAL & SAFELANES_HOSTILE;
      else if (strcmp(std,"non-hostile")==0)
         standing |= SAFELANES_NEUTRAL & SAFELANES_FRIENDLY;
      else
         NLUA_ERROR(L,_("Unknown standing type '%s'!"), std);
   }

   lanes = safelanes_get( faction, standing, sys );
   lua_newtable( L );
   for (i=0; i<array_size(lanes); i++) {
      lua_newtable( L );
      for (j=0; j<2; j++) {
         switch (lanes[i].point_type[j]) {
            case SAFELANE_LOC_PLANET:
               pnt = planet_getIndex( lanes[i].point_id[j] );
               //lua_pushplanet( L, lanes[i].point_id[j] );
#ifdef DEBUGGING
               if (pnt==NULL)
                  NLUA_ERROR(L, _("Planet is invalid"));
#endif /* DEBUGGING */
               lua_pushvector( L, pnt->pos );
               break;
            case SAFELANE_LOC_DEST_SYS:
               //jump.srcid = system_index( sys );
               //jump.destid = lanes[i].point_id[j];
               //lua_pushjump( L, jump );
               jmp = jump_getTarget( system_getIndex(lanes[i].point_id[j]), sys );
#ifdef DEBUGGING
               if (jmp==NULL)
                  NLUA_ERROR(L, _("Jump is invalid"));
#endif /* DEBUGGING */
               lua_pushvector( L, jmp->pos );
               break;
            default:
               NLUA_ERROR( L, _("Safe-lane vertex type is invalid.") );
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


/**
 * @brief Computes the intersection of a line segment and a circle.
 *
 *    @luatparam Vector center Center of the circle.
 *    @luatparam number radius Radius of the circle.
 *    @luatparam Vector p1 First point of the line segment.
 *    @luatparam Vector p2 Second point of the line segment.
 *    @luatreturn Vector|nil First point of collision or nil if no collision.
 *    @luatreturn Vector|nil Second point of collision or nil if single-point collision.
 * @luafunc intersect
 */
static int safelanesL_intersect( lua_State *L )
{
   Vector2d *center, *p1, *p2, crash[2];
   double radius;

   center = luaL_checkvector( L, 1 );
   radius = luaL_checknumber( L, 2 );
   p1     = luaL_checkvector( L, 3 );
   p2     = luaL_checkvector( L, 4 );

   int cnt = CollideLineCircle( p1, p2, center, radius, crash );
   if (cnt>0)
      lua_pushvector( L, crash[0] );
   if (cnt>1)
      lua_pushvector( L, crash[1] );

   return cnt;
}



