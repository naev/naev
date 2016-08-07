/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_space.c
 *
 * @brief Handles the Lua space bindings.
 *
 * These bindings control the planets and systems.
 */

#include "nlua_pilot.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_planet.h"
#include "nlua_outfit.h"
#include "log.h"
#include "rng.h"
#include "pilot.h"
#include "pilot_heat.h"
#include "player.h"
#include "space.h"
#include "ai.h"
#include "nlua_col.h"
#include "weapon.h"
#include "gui.h"
#include "camera.h"
#include "damagetype.h"
#include "land_outfits.h"


/*
 * From pilot.c
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * Prototypes.
 */
static Task *pilotL_newtask( lua_State *L, Pilot* p, const char *task );
static int pilotL_addFleetFrom( lua_State *L, int from_ship );
static int outfit_compareActive( const void *slot1, const void *slot2 );


/* Pilot metatable methods. */
static int pilotL_addFleetRaw( lua_State *L );
static int pilotL_addFleet( lua_State *L );
static int pilotL_remove( lua_State *L );
static int pilotL_clear( lua_State *L );
static int pilotL_toggleSpawn( lua_State *L );
static int pilotL_getPilots( lua_State *L );
static int pilotL_eq( lua_State *L );
static int pilotL_name( lua_State *L );
static int pilotL_id( lua_State *L );
static int pilotL_exists( lua_State *L );
static int pilotL_target( lua_State *L );
static int pilotL_inrange( lua_State *L );
static int pilotL_nav( lua_State *L );
static int pilotL_weapset( lua_State *L );
static int pilotL_weapsetHeat( lua_State *L );
static int pilotL_actives( lua_State *L );
static int pilotL_outfits( lua_State *L );
static int pilotL_rename( lua_State *L );
static int pilotL_position( lua_State *L );
static int pilotL_velocity( lua_State *L );
static int pilotL_dir( lua_State *L );
static int pilotL_ew( lua_State *L );
static int pilotL_temp( lua_State *L );
static int pilotL_faction( lua_State *L );
static int pilotL_spaceworthy( lua_State *L );
static int pilotL_setPosition( lua_State *L );
static int pilotL_setVelocity( lua_State *L );
static int pilotL_setDir( lua_State *L );
static int pilotL_broadcast( lua_State *L );
static int pilotL_comm( lua_State *L );
static int pilotL_setFaction( lua_State *L );
static int pilotL_setHostile( lua_State *L );
static int pilotL_setFriendly( lua_State *L );
static int pilotL_setInvincible( lua_State *L );
static int pilotL_setInvincPlayer( lua_State *L );
static int pilotL_setInvisible( lua_State *L );
static int pilotL_setVisplayer( lua_State *L );
static int pilotL_setVisible( lua_State *L );
static int pilotL_setHilight( lua_State *L );
static int pilotL_getColour( lua_State *L );
static int pilotL_getHostile( lua_State *L );
static int pilotL_flags( lua_State *L );
static int pilotL_setActiveBoard( lua_State *L );
static int pilotL_setNoDeath( lua_State *L );
static int pilotL_disable( lua_State *L );
static int pilotL_cooldown( lua_State *L );
static int pilotL_setCooldown( lua_State *L );
static int pilotL_setNoJump( lua_State *L );
static int pilotL_setNoLand( lua_State *L );
static int pilotL_addOutfit( lua_State *L );
static int pilotL_rmOutfit( lua_State *L );
static int pilotL_setFuel( lua_State *L );
static int pilotL_changeAI( lua_State *L );
static int pilotL_setTemp( lua_State *L );
static int pilotL_setHealth( lua_State *L );
static int pilotL_setEnergy( lua_State *L );
static int pilotL_setNoboard( lua_State *L );
static int pilotL_setNodisable( lua_State *L );
static int pilotL_setSpeedLimit( lua_State *L);
static int pilotL_getHealth( lua_State *L );
static int pilotL_getEnergy( lua_State *L );
static int pilotL_getLockon( lua_State *L );
static int pilotL_getStats( lua_State *L );
static int pilotL_cargoFree( lua_State *L );
static int pilotL_cargoHas( lua_State *L );
static int pilotL_cargoAdd( lua_State *L );
static int pilotL_cargoRm( lua_State *L );
static int pilotL_cargoList( lua_State *L );
static int pilotL_ship( lua_State *L );
static int pilotL_idle( lua_State *L );
static int pilotL_control( lua_State *L );
static int pilotL_memory( lua_State *L );
static int pilotL_memoryCheck( lua_State *L );
static int pilotL_taskclear( lua_State *L );
static int pilotL_goto( lua_State *L );
static int pilotL_face( lua_State *L );
static int pilotL_brake( lua_State *L );
static int pilotL_follow( lua_State *L );
static int pilotL_attack( lua_State *L );
static int pilotL_runaway( lua_State *L );
static int pilotL_hyperspace( lua_State *L );
static int pilotL_land( lua_State *L );
static int pilotL_hailPlayer( lua_State *L );
static int pilotL_hookClear( lua_State *L );
static const luaL_reg pilotL_methods[] = {
   /* General. */
   { "addRaw", pilotL_addFleetRaw },
   { "add", pilotL_addFleet },
   { "rm", pilotL_remove },
   { "get", pilotL_getPilots },
   { "__eq", pilotL_eq },
   /* Info. */
   { "name", pilotL_name },
   { "id", pilotL_id },
   { "exists", pilotL_exists },
   { "target", pilotL_target },
   { "inrange", pilotL_inrange },
   { "nav", pilotL_nav },
   { "weapset", pilotL_weapset },
   { "weapsetHeat", pilotL_weapsetHeat },
   { "actives", pilotL_actives },
   { "outfits", pilotL_outfits },
   { "rename", pilotL_rename },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "dir", pilotL_dir },
   { "ew", pilotL_ew },
   { "temp", pilotL_temp },
   { "cooldown", pilotL_cooldown },
   { "faction", pilotL_faction },
   { "spaceworthy", pilotL_spaceworthy },
   { "health", pilotL_getHealth },
   { "energy", pilotL_getEnergy },
   { "lockon", pilotL_getLockon },
   { "stats", pilotL_getStats },
   { "colour", pilotL_getColour },
   { "hostile", pilotL_getHostile },
   { "flags", pilotL_flags },
   /* System. */
   { "clear", pilotL_clear },
   { "toggleSpawn", pilotL_toggleSpawn },
   /* Modify. */
   { "changeAI", pilotL_changeAI },
   { "setTemp", pilotL_setTemp },
   { "setHealth", pilotL_setHealth },
   { "setEnergy", pilotL_setEnergy },
   { "setNoboard", pilotL_setNoboard },
   { "setNodisable", pilotL_setNodisable },
   { "setSpeedLimit", pilotL_setSpeedLimit },
   { "setPos", pilotL_setPosition },
   { "setVel", pilotL_setVelocity },
   { "setDir", pilotL_setDir },
   { "setFaction", pilotL_setFaction },
   { "setHostile", pilotL_setHostile },
   { "setFriendly", pilotL_setFriendly },
   { "setInvincible", pilotL_setInvincible },
   { "setInvincPlayer", pilotL_setInvincPlayer },
   { "setInvisible", pilotL_setInvisible },
   { "setVisplayer", pilotL_setVisplayer },
   { "setVisible", pilotL_setVisible },
   { "setHilight", pilotL_setHilight },
   { "setActiveBoard", pilotL_setActiveBoard },
   { "setNoDeath", pilotL_setNoDeath },
   { "disable", pilotL_disable },
   { "setCooldown", pilotL_setCooldown },
   { "setNoJump", pilotL_setNoJump },
   { "setNoLand", pilotL_setNoLand },
   /* Talk. */
   { "broadcast", pilotL_broadcast },
   { "comm", pilotL_comm },
   /* Outfits. */
   { "addOutfit", pilotL_addOutfit },
   { "rmOutfit", pilotL_rmOutfit },
   { "setFuel", pilotL_setFuel },
   /* Ship. */
   { "cargoList", pilotL_cargoList },
   { "ship", pilotL_ship },
   { "cargoFree", pilotL_cargoFree },
   { "cargoHas", pilotL_cargoHas },
   { "cargoAdd", pilotL_cargoAdd },
   { "cargoRm", pilotL_cargoRm },
   { "cargoList", pilotL_cargoList },
   /* Manual AI control. */
   { "idle", pilotL_idle },
   { "control", pilotL_control },
   { "memory", pilotL_memory },
   { "memoryCheck", pilotL_memoryCheck },
   { "taskClear", pilotL_taskclear },
   { "goto", pilotL_goto },
   { "face", pilotL_face },
   { "brake", pilotL_brake },
   { "follow", pilotL_follow },
   { "attack", pilotL_attack },
   { "runaway", pilotL_runaway },
   { "hyperspace", pilotL_hyperspace },
   { "land", pilotL_land },
   /* Misc. */
   { "hailPlayer", pilotL_hailPlayer },
   { "hookClear", pilotL_hookClear },
   {0,0}
}; /**< Pilot metatable methods. */
static const luaL_reg pilotL_cond_methods[] = {
   /* General. */
   { "get", pilotL_getPilots },
   { "__eq", pilotL_eq },
   /* Info. */
   { "name", pilotL_name },
   { "id", pilotL_id },
   { "exists", pilotL_exists },
   { "target", pilotL_target },
   { "inrange", pilotL_inrange },
   { "nav", pilotL_nav },
   { "weapset", pilotL_weapset },
   { "weapsetHeat", pilotL_weapsetHeat },
   { "actives", pilotL_actives },
   { "outfits", pilotL_outfits },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "dir", pilotL_dir },
   { "ew", pilotL_ew },
   { "temp", pilotL_temp },
   { "cooldown", pilotL_cooldown },
   { "faction", pilotL_faction },
   { "spaceworthy", pilotL_spaceworthy },
   { "health", pilotL_getHealth },
   { "energy", pilotL_getEnergy },
   { "lockon", pilotL_getLockon },
   { "stats", pilotL_getStats },
   { "colour", pilotL_getColour },
   { "hostile", pilotL_getHostile },
   { "flags", pilotL_flags },
   /* Ship. */
   { "ship", pilotL_ship },
   { "cargoFree", pilotL_cargoFree },
   { "cargoHas", pilotL_cargoHas },
   { "cargoList", pilotL_cargoList },
   /* Manual AI control. */
   { "memoryCheck", pilotL_memoryCheck },
   {0,0}
};




/**
 * @brief Loads the space library.
 *
 *    @param L State to load space library into.
 *    @return 0 on success.
 */
int nlua_loadPilot( lua_State *L, int readonly )
{
   /* Create the metatable */
   luaL_newmetatable(L, PILOT_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, pilotL_cond_methods);
   else
      luaL_register(L, NULL, pilotL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, PILOT_METATABLE);

   /* Pilot always loads ship. */
   nlua_loadShip(L,readonly);

   return 0;
}


/**
 * @brief Lua bindings to interact with pilots.
 *
 * This will allow you to create and manipulate pilots in-game.
 *
 * An example would be:
 * @code
 * p = pilot.add( "Sml Trader Convoy" ) -- Create a trader convoy
 * for k,v in pairs(p) do
 *    v:setFriendly() -- Make it friendly
 * end
 * @endcode
 *
 * @luamod pilot
 */
/**
 * @brief Gets pilot at index.
 *
 *    @param L Lua state to get pilot from.
 *    @param ind Index position to find the pilot.
 *    @return Pilot found at the index in the state.
 */
LuaPilot lua_topilot( lua_State *L, int ind )
{
   return *((LuaPilot*) lua_touserdata(L,ind));
}
/**
 * @brief Gets pilot at index or raises error if there is no pilot at index.
 *
 *    @param L Lua state to get pilot from.
 *    @param ind Index position to find pilot.
 *    @return Pilot found at the index in the state.
 */
LuaPilot luaL_checkpilot( lua_State *L, int ind )
{
   if (lua_ispilot(L,ind))
      return lua_topilot(L,ind);
   luaL_typerror(L, ind, PILOT_METATABLE);
   return 0;
}
/**
 * @brief Makes sure the pilot is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the pilot to validate.
 *    @return The pilot (doesn't return if fails - raises Lua error ).
 */
Pilot* luaL_validpilot( lua_State *L, int ind )
{
   Pilot *p;

   /* Get the pilot. */
   p  = pilot_get(luaL_checkpilot(L,ind));
   if (p==NULL) {
      NLUA_ERROR(L,"Pilot is invalid.");
      return NULL;
   }

   return p;
}
/**
 * @brief Pushes a pilot on the stack.
 *
 *    @param L Lua state to push pilot into.
 *    @param pilot Pilot to push.
 *    @return Newly pushed pilot.
 */
LuaPilot* lua_pushpilot( lua_State *L, LuaPilot pilot )
{
   LuaPilot *p;
   p = (LuaPilot*) lua_newuserdata(L, sizeof(LuaPilot));
   *p = pilot;
   luaL_getmetatable(L, PILOT_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}
/**
 * @brief Checks to see if ind is a pilot.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a pilot.
 */
int lua_ispilot( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, PILOT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Wrapper with common code for pilotL_addFleet and pilotL_addFleetRaw.
 */
static int pilotL_addFleetFrom( lua_State *L, int from_ship )
{
   Fleet *flt;
   Ship *ship;
   const char *fltname, *fltai, *faction;
   int i, first;
   unsigned int p;
   double a, r;
   Vector2d vv,vp, vn;
   FleetPilot *plt;
   LuaFaction lf;
   StarSystem *ss;
   Planet *planet;
   JumpPoint *target;
   int jump;
   PilotFlags flags;
   int *jumpind, njumpind;
   int *ind, nind;
   double chance;
   int ignore_rules;

   /* Default values. */
   pilot_clearFlagsRaw( flags );
   vectnull(&vn); /* Need to determine angle. */
   jump = -1;
   a    = 0.;

   /* Parse first argument - Fleet Name */
   fltname = luaL_checkstring(L,1);

   /* pull the fleet */
   if (from_ship) {
      ship = ship_get( fltname );
      if (ship == NULL) {
         NLUA_ERROR(L,"Ship '%s' not found!", fltname);
         return 0;
      }
      faction = luaL_checkstring(L,4);
      lf = faction_get(faction);
      if (lf < 0) {
         NLUA_ERROR(L,"Faction '%s' not found in stack.", faction );
         return 0;
      }
   }
   else {
      flt = fleet_get( fltname );
      if (flt == NULL) {
         NLUA_ERROR(L,"Fleet '%s' doesn't exist.", fltname);
         return 0;
      }
      lf = flt->faction;
   }

   /* Parse second argument - Fleet AI Override */
   if ((lua_gettop(L) < 2) || lua_isnil(L,2))
      fltai = NULL;
   else
      fltai = luaL_checkstring(L,2);

   /* Handle third argument. */
   if (lua_isvector(L,3)) {
      vp = *lua_tovector(L,3);
      a = RNGF() * 2.*M_PI;
      vectnull( &vv );
   }
   else if (lua_issystem(L,3)) {
      ss = system_getIndex( lua_tosystem(L,3) );
      for (i=0; i<cur_system->njumps; i++) {
         if ((cur_system->jumps[i].target == ss)
               && !jp_isFlag( jump_getTarget( cur_system, cur_system->jumps[i].target ), JP_EXITONLY )) {
            jump = i;
            break;
         }
      }
      if (jump < 0) {
         if (cur_system->njumps > 0) {
            WARN("Fleet '%s' jumping in from non-adjacent system '%s' to '%s'.",
                  fltname, ss->name, cur_system->name );
            jump = RNG_SANE(0,cur_system->njumps-1);
         }
         else
            WARN("Fleet '%s' attempting to jump in from '%s', but '%s' has no jump points.",
                  fltname, ss->name, cur_system->name );
      }
   }
   else if (lua_isplanet(L,3)) {
      planet  = luaL_validplanet(L,3);
      pilot_setFlagRaw( flags, PILOT_TAKEOFF );
      a = RNGF() * 2. * M_PI;
      r = RNGF() * planet->radius;
      vect_cset( &vp,
            planet->pos.x + r * cos(a),
            planet->pos.y + r * sin(a) );
      a = RNGF() * 2.*M_PI;
      vectnull( &vv );
   }
   /* Random. */
   else {
      /* Check if we should ignore the strict rules. */
      ignore_rules = 0;
      if (lua_isboolean(L,3) && lua_toboolean(L,3))
         ignore_rules = 1;

      /* Build landable planet table. */
      ind   = NULL;
      nind  = 0;
      if (cur_system->nplanets > 0) {
         ind = malloc( sizeof(int) * cur_system->nplanets );
         for (i=0; i<cur_system->nplanets; i++)
            if (planet_hasService(cur_system->planets[i],PLANET_SERVICE_INHABITED) &&
                  !areEnemies(lf,cur_system->planets[i]->faction))
               ind[ nind++ ] = i;
      }

      /* Build jumpable jump table. */
      jumpind  = NULL;
      njumpind = 0;
      if (cur_system->njumps > 0) {
         jumpind = malloc( sizeof(int) * cur_system->njumps );
         for (i=0; i<cur_system->njumps; i++) {
            /* The jump into the system must not be exit-only, and unless
             * ignore_rules is set, must also be non-hidden and have faction
             * presence matching the pilot's on the remote side.
             */
            target = jump_getTarget( cur_system, cur_system->jumps[i].target );
            if (!jp_isFlag( target, JP_EXITONLY ) && (ignore_rules ||
                  (!jp_isFlag( &cur_system->jumps[i], JP_HIDDEN ) &&
                  (system_getPresence( cur_system->jumps[i].target, lf ) > 0))))
               jumpind[ njumpind++ ] = i;
         }
      }

      /* Crazy case no landable nor presence, we'll just jump in randomly. */
      if ((nind == 0) && (njumpind==0)) {
         if (cur_system->njumps > 0) {
            jumpind = malloc( sizeof(int) * cur_system->njumps );
            for (i=0; i<cur_system->njumps; i++)
               jumpind[ njumpind++ ] = i;
         }
         else {
            WARN("Creating pilot in system with no jumps nor planets to take off from!");
            vectnull( &vp );
            a = RNGF() * 2.*M_PI;
            vectnull( &vv );
         }
      }

      /* Calculate jump chance. */
      if ((ind != NULL) || (jumpind != NULL)) {
         chance = njumpind;
         chance = chance / (chance + nind);

         /* Random jump in. */
         if ((ind == NULL) || ((RNGF() <= chance) && (jumpind != NULL)))
            jump = jumpind[ RNG_SANE(0,njumpind-1) ];
         /* Random take off. */
         else if (ind !=NULL && nind != 0) {
            planet = cur_system->planets[ ind[ RNG_SANE(0,nind-1) ] ];
            pilot_setFlagRaw( flags, PILOT_TAKEOFF );
            a = RNGF() * 2. * M_PI;
            r = RNGF() * planet->radius;
            vect_cset( &vp,
                  planet->pos.x + r * cos(a),
                  planet->pos.y + r * sin(a) );
            a = RNGF() * 2.*M_PI;
            vectnull( &vv );
         }
      }

      /* Free memory allocated. */
      free( ind );
      free( jumpind );
   }

   /* Set up velocities and such. */
   if (jump >= 0) {
      space_calcJumpInPos( cur_system, cur_system->jumps[jump].target, &vp, &vv, &a );
      pilot_setFlagRaw( flags, PILOT_HYP_END );
   }

   /* Make sure angle is sane. */
   a = fmod( a, 2.*M_PI );
   if (a < 0.)
      a += 2.*M_PI;

   lua_newtable(L);
   if (from_ship) {
      /* Create the pilot. */
      p = pilot_create( ship, fltname, lf, fltai, a, &vp, &vv, flags, -1 );

      /* we push each pilot created into a table and return it */
      lua_pushnumber(L,1); /* index, starts with 1 */
      lua_pushpilot(L,p); /* value = LuaPilot */
      lua_rawset(L,-3); /* store the value in the table */
   }
   else {
      /* now we start adding pilots and toss ids into the table we return */
      first = 1;
      for (i=0; i<flt->npilots; i++) {
         plt = &flt->pilots[i];

         /* Fleet displacement - first ship is exact. */
         if (!first)
            vect_cadd(&vp, RNG(75,150) * (RNG(0,1) ? 1 : -1),
                  RNG(75,150) * (RNG(0,1) ? 1 : -1));
         first = 0;

         /* Create the pilot. */
         p = fleet_createPilot( flt, plt, a, &vp, &vv, fltai, flags, -1 );

         /* we push each pilot created into a table and return it */
         lua_pushnumber(L,i+1); /* index, starts with 1 */
         lua_pushpilot(L,p); /* value = LuaPilot */
         lua_rawset(L,-3); /* store the value in the table */
      }
   }
   return 1;
}


/**
 * @brief Adds a fleet to the system.
 *
 * You can then iterate over the pilots to change parameters like so:
 * @code
 * p = pilot.add( "Sml Trader Convoy" )
 * for k,v in pairs(p) do
 *    v:setHostile()
 * end
 * @endcode
 *
 * How param works (by type of value passed): <br/>
 *  - nil: spawns pilot randomly entering from jump points with presence of their faction or taking off from non-hostile planets <br/>
 *  - planet: pilot takes off from the planet <br/>
 *  - system: jumps pilot in from the system <br/>
 *  - vec2: pilot is created at the position (no jump/takeoff) <br/>
 *  - true: Acts like nil, but does not avoid jump points with no presence <br/>
 *
 * @usage p = pilot.add( "Pirate Hyena" ) -- Just adds the pilot (will jump in or take off).
 * @usage p = pilot.add( "Trader Llama", "dummy" ) -- Overrides AI with dummy ai.
 * @usage p = pilot.add( "Sml Trader Convoy", nil, vec2.new( 1000, 200 ) ) -- Pilot won't jump in, will just appear.
 * @usage p = pilot.add( "Empire Pacifier", nil, system.get("Goddard") ) -- Have the pilot jump in from the system.
 * @usage p = pilot.add( "Goddard Goddard", nil, planet.get("Zhiru") ) -- Have the pilot take off from a planet.
 *
 *    @luatparam string fleetname Name of the fleet to add.
 *    @luatparam[opt] string|nil ai If set will override the standard fleet AI.  nil means use default.
 *    @luatparam System|Planet param Position to create pilot at, if it's a system it'll try to jump in from that system, if it's
 *              a planet it'll try to take off from it.
 *    @luatreturn {Pilot,...} Table populated with all the pilots created.  The keys are ordered numbers.
 * @luafunc add( fleetname, ai, param )
 */
static int pilotL_addFleet( lua_State *L )
{
   return pilotL_addFleetFrom( L, 0 );
}


/**
 * @brief Adds a ship with an AI and faction to the system (instead of a predefined fleet).
 *
 * @usage p = pilot.addRaw( "Empire Shark", "empire", nil, "Empire" ) -- Creates a pilot analogous to the Empire Shark fleet.
 *
 *    @luatparam string shipname Name of the ship to add.
 *    @luatparam string|nil ai AI to give the pilot.
 *    @luatparam System|Planet param Position to create the pilot at. See pilot.add for further information.
 *    @luatparam Faction faction Faction to give the pilot.
 *    @luatreturn {Pilot,...} Table populated with the created pilot.
 * @luafunc addRaw( shipname, ai, param, faction )
 */
static int pilotL_addFleetRaw(lua_State *L )
{
   return pilotL_addFleetFrom( L, 1 );
}


/**
 * @brief Removes a pilot without explosions or anything.
 *
 * @usage p:rm() -- pilot will be destroyed
 *
 *    @luatparam Pilot p Pilot to remove.
 * @luafunc rm( p )
 */
static int pilotL_remove( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Make sure it's not the player. */
   if (player.p == p) {
      NLUA_ERROR( L, "Trying to remove the bloody player!" );
      return 0;
   }

   /* Deletes the pilot. */
   pilot_delete(p);

   return 0;
}
/**
 * @brief Clears the current system of pilots.  Used for epic battles and such.
 *
 * Be careful with this command especially in big systems. It will most likely
 *  cause issues if multiple missions are in the same system.
 *
 * @note Clears all global pilot hooks too.
 *
 * @usage pilot.clear()
 *
 * @luafunc clear()
 */
static int pilotL_clear( lua_State *L )
{
   (void) L;
   pilots_clear();
   weapon_clear();
   return 0;
}
/**
 * @brief Disables or enables pilot spawning in the current system.
 *
 * If player jumps the spawn is enabled again automatically. Global spawning takes priority over faction spawning.
 *
 * @usage pilot.toggleSpawn() -- Defaults to flipping the global spawning (true->false and false->true)
 * @usage pilot.toggleSpawn( false ) -- Disables global spawning
 * @usage pliot.toggleSpawn( "Pirates" ) -- Defaults to disabling pirate spawning
 * @usage pilot.toggleSpawn( "Pirates", true ) -- Turns on pirate spawning
 *
 *    @luatparam[opt] Faction fid Faction to enable or disable spawning off. If ommited it works on global spawning.
 *    @luatparam[opt] boolean enable true enables spawn, false disables it.
 *    @luatreturn boolean The current spawn state.
 * @luafunc toggleSpawn( fid, enable )
 */
static int pilotL_toggleSpawn( lua_State *L )
{
   int i, f, b;

   /* Setting it directly. */
   if (lua_gettop(L) > 0) {
      if (lua_isfaction(L,1) || lua_isstring(L,1)) {

         f = luaL_validfaction(L,1);
         b = !lua_toboolean(L,2);

         /* Find the faction and set. */
         for (i=0; i<cur_system->npresence; i++) {
            if (cur_system->presence[i].faction != f)
               continue;
            cur_system->presence[i].disabled = b;
            break;
         }

      }
      else if (lua_isboolean(L,1))
         space_spawn = lua_toboolean(L,1);
      else
         NLUA_INVALID_PARAMETER(L);
   }
   /* Toggling. */
   else
      space_spawn = !space_spawn;

   lua_pushboolean(L, space_spawn);
   return 1;
}
/**
 * @brief Gets the pilots available in the system by a certain criteria.
 *
 * @usage p = pilot.get() -- Gets all the pilots
 * @usage p = pilot.get( { faction.get("Empire") } ) -- Only gets empire pilots.
 * @usage p = pilot.get( nil, true ) -- Gets all pilots including disabled
 * @usage p = pilot.get( { faction.get("Empire") }, true ) -- Only empire pilots with disabled
 *
 *    @luatparam Faction|{Faction,...} factions If f is a table of factions, it will only get pilots matching those factions.  Otherwise it gets all the pilots.
 *    @luatparam boolean disabled Whether or not to get disabled ships (default is off if parameter is omitted).
 *    @luatreturn {Pilot,...} A table containing the pilots.
 * @luafunc get( factions, disabled )
 */
static int pilotL_getPilots( lua_State *L )
{
   int i, j, k, d;
   int *factions;
   int nfactions;

   /* Whether or not to get disabled. */
   d = lua_toboolean(L,2);

   /* Check for belonging to faction. */
   if (lua_istable(L,1) || lua_isfaction(L,1)) {
      if (lua_isfaction(L,1)) {
         nfactions = 1;
         factions = malloc( sizeof(int) );
         factions[0] = lua_tofaction(L,1);
      }
      else {
         /* Get table length and preallocate. */
         nfactions = (int) lua_objlen(L,1);
         factions = malloc( sizeof(int) * nfactions );
         /* Load up the table. */
         lua_pushnil(L);
         i = 0;
         while (lua_next(L, -2) != 0) {
            if (lua_isfaction(L,-1)) {
               factions[i++] = lua_tofaction(L, -1);
            }
            lua_pop(L,1);
         }
      }

      /* Now put all the matching pilots in a table. */
      lua_newtable(L);
      k = 1;
      for (i=0; i<pilot_nstack; i++) {
         for (j=0; j<nfactions; j++) {
            if ((pilot_stack[i]->faction == factions[j]) &&
                  (d || !pilot_isDisabled(pilot_stack[i])) &&
                  !pilot_isFlag(pilot_stack[i], PILOT_DELETE)) {
               lua_pushnumber(L, k++); /* key */
               lua_pushpilot(L, pilot_stack[i]->id); /* value */
               lua_rawset(L,-3); /* table[key] = value */
            }
         }
      }

      /* clean up. */
      free(factions);
   }
   else if ((lua_isnil(L,1)) || (lua_gettop(L) == 0)) {
      /* Now put all the matching pilots in a table. */
      lua_newtable(L);
      k = 1;
      for (i=0; i<pilot_nstack; i++) {
         if ((d || !pilot_isDisabled(pilot_stack[i])) &&
               !pilot_isFlag(pilot_stack[i], PILOT_DELETE)) {
            lua_pushnumber(L, k++); /* key */
            lua_pushpilot(L, pilot_stack[i]->id); /* value */
            lua_rawset(L,-3); /* table[key] = value */
         }
      }
   }
   else {
      NLUA_INVALID_PARAMETER(L);
   }

   return 1;
}

/**
 * @brief Checks to see if pilot and p are the same.
 *
 * @usage if p == p2 then -- Pilot 'p' and 'p2' match.
 *
 *    @luatparam Pilot p Pilot to compare.
 *    @luatparam Pilot comp Pilot to compare against.
 *    @luatreturn boolean true if they are the same.
 * @luafunc __eq( p, comp )
 */
static int pilotL_eq( lua_State *L )
{
   LuaPilot p1, p2;

   /* Get parameters. */
   p1 = luaL_checkpilot(L,1);
   p2 = luaL_checkpilot(L,2);

   /* Push result. */
   lua_pushboolean(L, p1 == p2);
   return 1;
}

/**
 * @brief Gets the pilot's current name.
 *
 * @usage name = p:name()
 *
 *    @luatparam Pilot p Pilot to get the name of.
 *    @luatreturn string The current name of the pilot.
 * @luafunc name( p )
 */
static int pilotL_name( lua_State *L )
{
   Pilot *p;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);

   /* Get name. */
   lua_pushstring(L, p->name);
   return 1;
}

/**
 * @brief Gets the ID of the pilot.
 *
 * @usage id = p:id()
 *
 *    @luaparam p Pilot Pilot to get the ID of.
 *    @luareturn number The ID of the current pilot.
 * @luafunc id( p )
 */
static int pilotL_id( lua_State *L )
{
   Pilot *p;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);

   /* Get name. */
   lua_pushnumber(L, p->id);
   return 1;
}

/**
 * @brief Checks to see if pilot is still in the system and alive.
 *
 * Pilots cease to exist if they die or jump out.
 *
 * @usage if p:exists() then -- Pilot still exists
 *
 *    @luatparam Pilot p Pilot to check to see if is still exists.
 *    @luatreturn boolean true if pilot is still exists.
 * @luafunc exists( p )
 */
static int pilotL_exists( lua_State *L )
{
   Pilot *p;
   int exists;

   /* Parse parameters. */
   p  = pilot_get( luaL_checkpilot(L,1) );

   /* Must still be kicking and alive. */
   if (p==NULL)
      exists = 0;
   else if (pilot_isFlag( p, PILOT_DEAD ))
      exists = 0;
   else
      exists = 1;

   /* Check if the pilot exists. */
   lua_pushboolean(L, exists);
   return 1;
}


/**
 * @brief Gets the pilot target of the pilot.
 *
 * @usage target = p:target()
 *
 *    @luatparam Pilot p Pilot to get target of.
 *    @luatreturn Pilot|nil nil if no target is selected, otherwise the target of the pilot.
 * @luafunc target( p )
 */
static int pilotL_target( lua_State *L )
{
   Pilot *p;
   p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;
   /* Must be targeted. */
   if (p->target == p->id)
      return 0;
   /* Must be valid. */
   if (pilot_get(p->target) == NULL)
      return 0;
   /* Push target. */
   lua_pushpilot(L, p->target);
   return 1;
}


/**
 * @brief Checks to see if pilot is in range of pilot.
 *
 * @usage detected, scanned = p:inrange( target )
 *
 *    @luatparam Pilot p Pilot to see if another pilot is in range.
 *    @luatparam Pilot target Target pilot.
 *    @luatreturn boolean Checks to see if the target is detected and if it's scanned.
 * @luafunc inrange( p, target )
 */
static int pilotL_inrange( lua_State *L )
{
   Pilot *p, *t;
   int ret;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   t = luaL_validpilot(L,2);

   /* Check if in range. */
   ret = pilot_inRangePilot( p, t );
   if (ret == 1) { /* In range. */
      lua_pushboolean(L,1);
      lua_pushboolean(L,1);
   }
   else if (ret == 0) { /* Not in range. */
      lua_pushboolean(L,0);
      lua_pushboolean(L,0);
   }
   else { /* Detected fuzzy. */
      lua_pushboolean(L,1);
      lua_pushboolean(L,0);
   }
   return 2;
}


/**
 * @brief Gets the nav target of the pilot.
 *
 * This will only terminate when the target following pilot disappears (land, death, jump, etc...).
 *
 * @usage planet, hyperspace = p:nav()
 *
 *    @luatparam Pilot p Pilot to get nav info of.
 *    @luatreturn Planet|nil The pilot's planet target.
 *    @luatreturn System|nil The pilot's hyperspace target.
 * @luafunc nav( p )
 */
static int pilotL_nav( lua_State *L )
{
   LuaSystem ls;
   Pilot *p;

   /* Get pilot. */
   p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;

   /* Get planet target. */
   if (p->nav_planet < 0)
      lua_pushnil(L);
   else
      lua_pushplanet( L, cur_system->planets[ p->nav_planet ]->id );

   /* Get hyperspace target. */
   if (p->nav_hyperspace < 0)
      lua_pushnil(L);
   else {
      ls = cur_system->jumps[ p->nav_hyperspace ].targetid;
      lua_pushsystem( L, ls );
   }

   return 2;
}


/**
 * @brief Gets the weapset weapon of the pilot.
 *
 * The weapon sets have the following structure: <br />
 * <ul>
 *  <li> name: name of the set. </li>
 *  <li> cooldown: [0:1] value indicating if ready to shoot (1 is ready). </li>
 *  <li> charge: [0:1] charge level of beam weapon (1 is full). </li>
 *  <li> ammo: Name of the ammo or nil if not applicable. </li>
 *  <li> left: Absolute ammo left or nil if not applicable. </li>
 *  <li> left_p: Relative ammo left [0:1] or nil if not applicable </li>
 *  <li> lockon: Lockon [0:1] for seeker weapons or nil if not applicable. </li>
 *  <li> in_arc: Whether or not the target is in targetting arc or nil if not applicable. </li>
 *  <li> level: Level of the weapon (1 is primary, 2 is secondary). </li>
 *  <li> temp: Temperature of the weapon. </li>
 *  <li> type: Type of the weapon. </li>
 *  <li> dtype: Damage type of the weapon. </li>
 *  <li> track: Tracking level of the weapon. </li>
 * </ul>
 *
 * An example would be:
 * @code
 * ws_name, ws = p:weapset( true )
 * print( "Weapnset Name: " .. ws_name )
 * for _,w in ipairs(ws) do
 *    print( "Name: " .. w.name )
 *    print( "Cooldown: " .. tostring(cooldown) )
 *    print( "Level: " .. tostring(level) )
 * end
 * @endcode
 *
 * @usage set_name, slots = p:weapset( true ) -- Gets info for all active weapons
 * @usage set_name, slots = p:weapset() -- Get info about the current set
 * @usage set_name, slots = p:weapset( 5 ) -- Get info about the set number 5
 *
 *    @luatparam Pilot p Pilot to get weapset weapon of.
 *    @luatparam[opt] number id ID of the set to get information of. Defaults to currently active set.
 *    @luatreturn string The name of the set.
 *    @luatreturn table A table with each slot's information.
 * @luafunc weapset( p, id)
 */
static int pilotL_weapset( lua_State *L )
{
   Pilot *p, *target;
   int i, j, k, n;
   PilotWeaponSetOutfit *po_list;
   PilotOutfitSlot *slot;
   Outfit *ammo, *o;
   double delay, firemod, enermod, t;
   int id, all, level, level_match;
   int is_lau, is_fb;
   const Damage *dmg;

   /* Defaults. */
   po_list = NULL;

   /* Parse parameters. */
   all = 0;
   p   = luaL_validpilot(L,1);
   if (lua_gettop(L) > 1) {
      if (lua_isnumber(L,2))
         id = luaL_checkinteger(L,2) - 1;
      else if (lua_isboolean(L,2)) {
         all = lua_toboolean(L,2);
         id  = p->active_set;
      }
      else
         NLUA_INVALID_PARAMETER(L);
   }
   else
      id = p->active_set;
   id = CLAMP( 0, PILOT_WEAPON_SETS, id );

   /* Get target. */
   if (p->target != p->id)
      target = pilot_get( p->target );
   else
      target = NULL;

   /* Push name. */
   lua_pushstring( L, pilot_weapSetName( p, id ) );

   /* Push set. */
   if (all)
      n = p->noutfits;
   else
      po_list = pilot_weapSetList( p, id, &n );

   k = 0;
   lua_newtable(L);
   for (j=0; j<=PILOT_WEAPSET_MAX_LEVELS; j++) {
      /* Level to match. */
      level_match = (j==PILOT_WEAPSET_MAX_LEVELS) ? -1 : j;

      /* Iterate over weapons. */
      for (i=0; i<n; i++) {
         /* Get base look ups. */
         if (all) {
            slot     = p->outfits[i];
            o        = slot->outfit;
            if (o == NULL)
               continue;
            is_lau   = outfit_isLauncher(o);
            is_fb    = outfit_isFighterBay(o);

            /* Must be valid weapon. */
            if (!(outfit_isBolt(o) || outfit_isBeam(o) ||
                  is_lau || is_fb))
               continue;

            level    = slot->level;
         }
         else {
            slot     = po_list[i].slot;
            o        = slot->outfit;
            if (o == NULL)
               continue;
            is_lau   = outfit_isLauncher(o);
            is_fb    = outfit_isFighterBay(o);
            level    = po_list[i].level;
         }

         /* Must match level. */
         if (level != level_match)
            continue;

         /* Must be weapon. */
         if (outfit_isJammer(o) ||
               outfit_isMod(o) ||
               outfit_isAfterburner(o))
            continue;

         /* Set up for creation. */
         lua_pushnumber(L,++k);
         lua_newtable(L);

         /* Name. */
         lua_pushstring(L,"name");
         lua_pushstring(L,slot->outfit->name);
         lua_rawset(L,-3);


         /* Beams require special handling. */
         if (outfit_isBeam(o)) {
            pilot_getRateMod( &firemod, &enermod, p, slot->outfit );

            /* When firing, cooldown is always zero. When recharging,
             * it's the usual 0-1 readiness value.
             */
            lua_pushstring(L,"cooldown");
            if (slot->u.beamid > 0)
               lua_pushnumber(L, 0.);
            else {
               delay = (slot->timer / outfit_delay(o)) * firemod;
               lua_pushnumber( L, CLAMP( 0., 1., 1. - delay ) );
            }
            lua_rawset(L,-3);

            /* When firing, slot->timer represents the remaining duration. */
            lua_pushstring(L,"charge");
            if (slot->u.beamid > 0)
               lua_pushnumber(L, CLAMP( 0., 1., slot->timer / o->u.bem.duration ) );
            else
               lua_pushnumber( L, CLAMP( 0., 1., 1. - delay ) );
            lua_rawset(L,-3);
         }
         else {
            /* Set cooldown. */
            lua_pushstring(L,"cooldown");
            pilot_getRateMod( &firemod, &enermod, p, slot->outfit );
            delay = outfit_delay(slot->outfit) * firemod;
            if (delay > 0.)
               lua_pushnumber( L, CLAMP( 0., 1., 1. - slot->timer / delay ) );
            else
               lua_pushnumber( L, 1. );
            lua_rawset(L,-3);
         }

         /* Ammo name. */
         ammo = outfit_ammo(slot->outfit);
         if (ammo != NULL) {
            lua_pushstring(L,"ammo");
            lua_pushstring(L,ammo->name);
            lua_rawset(L,-3);
         }

         /* Ammo quantity absolute. */
         if ((is_lau || is_fb) &&
               (slot->u.ammo.outfit != NULL)) {
            lua_pushstring(L,"left");
            lua_pushnumber( L, slot->u.ammo.quantity );
            lua_rawset(L,-3);

         /* Ammo quantity relative. */
            lua_pushstring(L,"left_p");
            lua_pushnumber( L, (double)slot->u.ammo.quantity / (double)outfit_amount(slot->outfit) );
            lua_rawset(L,-3);
         }

         /* Launcher lockon. */
         if (is_lau) {
            t = slot->u.ammo.lockon_timer;
            lua_pushstring(L, "lockon");
            if (t <= 0.)
               lua_pushnumber(L, 1.);
            else
               lua_pushnumber(L, 1. - (t / slot->outfit->u.lau.lockon));
            lua_rawset(L,-3);

         /* Is in arc. */
            lua_pushstring(L, "in_arc");
            lua_pushboolean(L, slot->u.ammo.in_arc);
            lua_rawset(L,-3);
         }

         /* Level. */
         lua_pushstring(L,"level");
         lua_pushnumber(L, level+1);
         lua_rawset(L,-3);

         /* Temperature. */
         lua_pushstring(L,"temp");
         lua_pushnumber(L, pilot_heatFirePercent(slot->heat_T));
         lua_rawset(L,-3);

         /* Type. */
         lua_pushstring(L, "type");
         lua_pushstring(L, outfit_getType(slot->outfit));
         lua_rawset(L,-3);

         /* Damage type. */
         if (is_lau && (slot->u.ammo.outfit != NULL))
            dmg = outfit_damage( slot->u.ammo.outfit );
         else
            dmg = outfit_damage( slot->outfit );
         if (dmg != NULL) {
            lua_pushstring(L, "dtype");
            lua_pushstring(L, dtype_damageTypeToStr( dmg->type ) );
            lua_rawset(L,-3);
         }

         /* Track. */
         if (slot->outfit->type == OUTFIT_TYPE_TURRET_BOLT) {
            lua_pushstring(L, "track");
            if (target != NULL)
               lua_pushnumber(L, pilot_ewWeaponTrack( p, target, slot->outfit->u.blt.track ));
            else
               lua_pushnumber(L, -1);
            lua_rawset(L,-3);
         }

         /* Set table in table. */
         lua_rawset(L,-3);
      }
   }
   return 2;
}


/**
 * @brief Gets heat information for a weapon set.
 *
 * Heat is a 0-2 value that corresponds to three separate ranges:
 *
 * <ul>
 *  <li>0: Weapon set isn't overheating and has no penalties.</li>
 *  <li>0-1: Weapon set has reduced accuracy.</li>
 *  <li>1-2: Weapon set has full accuracy penalty plus reduced fire rate.</li>
 * </ul>
 *
 * @usage hmean, hpeak = p:weapsetHeat( true ) -- Gets info for all active weapons
 * @usage hmean, hpeak = p:weapsetHeat() -- Get info about the current set
 * @usage hmean, hpeak = p:weapsetHeat( 5 ) -- Get info about the set number 5
 *
 *    @luatparam Pilot p Pilot to get weapset weapon of.
 *    @luatparam[opt] number id ID of the set to get information of. Defaults to currently active set.
 *    @luatreturn number Mean heat.
 *    @luatreturn number Peak heat.
 * @luafunc weapsetHeat(p, id)
 */
static int pilotL_weapsetHeat( lua_State *L )
{
   Pilot *p;
   PilotWeaponSetOutfit *po_list;
   PilotOutfitSlot *slot;
   Outfit *o;
   int i, j, n;
   int id, all, level, level_match;
   double heat, heat_mean, heat_peak, nweapons;

   /* Defaults. */
   po_list = NULL;
   heat_mean = 0.;
   heat_peak = 0.;
   nweapons = 0;

   /* Parse parameters. */
   all = 0;
   p   = luaL_validpilot(L,1);
   if (lua_gettop(L) > 1) {
      if (lua_isnumber(L,2))
         id = luaL_checkinteger(L,2) - 1;
      else if (lua_isboolean(L,2)) {
         all = lua_toboolean(L,2);
         id  = p->active_set;
      }
      else
         NLUA_INVALID_PARAMETER(L);
   }
   else
      id = p->active_set;
   id = CLAMP( 0, PILOT_WEAPON_SETS, id );

   /* Push set. */
   if (all)
      n = p->noutfits;
   else
      po_list = pilot_weapSetList( p, id, &n );

   for (j=0; j<=PILOT_WEAPSET_MAX_LEVELS; j++) {
      /* Level to match. */
      level_match = (j==PILOT_WEAPSET_MAX_LEVELS) ? -1 : j;

       /* Iterate over weapons. */
      for (i=0; i<n; i++) {
         /* Get base look ups. */
         if (all)
            slot = p->outfits[i];
         else
            slot = po_list[i].slot;

         o = slot->outfit;
         if (o == NULL)
            continue;

         if (all)
            level    = slot->level;
         else
            level    = po_list[i].level;

         /* Must match level. */
         if (level != level_match)
            continue;

         /* Must be weapon. */
         if (outfit_isJammer(o) ||
               outfit_isMod(o) ||
               outfit_isAfterburner(o))
            continue;

         nweapons++;
         heat = pilot_heatFirePercent(slot->heat_T);
         heat_mean += heat;
         if (heat > heat_peak)
            heat_peak = heat;
      }
   }

   /* Post-process. */
   if (nweapons > 0)
      heat_mean /= nweapons;

   lua_pushnumber( L, heat_mean );
   lua_pushnumber( L, heat_peak );

   return 2;
}


/**
 * @brief Gets the active outfits and their states of the pilot.
 *
 * The active outfits have the following structure: <br />
 * <ul>
 *  <li> name: Name of the set. </li>
 *  <li> type: Type of the outfit. </li>
 *  <li> temp: The heat of the outfit's slot. A value between 0 and 1, where 1 is fully overheated. </li>
 *  <li> weapset: The first weapon set that the outfit appears in, if any. </li>
 *  <li> state: State of the outfit, which can be one of { "off", "warmup", "on", "cooldown" }. </li>
 *  <li> duration: Set only if state is "on". Indicates duration value (0 = just finished, 1 = just on). </li>
 *  <li> cooldown: Set only if state is "cooldown". Indicates cooldown value (0 = just ending, 1 = just started cooling down). </li>
 * </ul>
 *
 * An example would be:
 * @code
 * act_outfits = p:actives()
 * print( "Weapnset Name: " .. ws_name )
 * for _,o in ipairs(act_outfits) do
 *    print( "Name: " .. o.name )
 *    print( "State: " .. o.state )
 * end
 * @endcode
 *
 * @usage act_outfits = p:actives() -- Gets the table of active outfits
 *
 *    @luatparam Pilot p Pilot to get active outfits of.
 *    @luatreturn table The table with each active outfit's information.
 * @luafunc actives( p )
 */
static int pilotL_actives( lua_State *L )
{
   Pilot *p;
   int i, k, sort;
   PilotOutfitSlot *o;
   PilotOutfitSlot **outfits;
   const char *str;
   double d;

   /* Parse parameters. */
   p   = luaL_validpilot(L,1);

   if (lua_gettop(L) > 1)
      sort = lua_toboolean(L, 1);
   else
      sort = 0;

   k = 0;
   lua_newtable(L);

   if (sort) {
      outfits = malloc( sizeof(PilotOutfitSlot*) * p->noutfits );
      memcpy( outfits, p->outfits, sizeof(PilotOutfitSlot*) * p->noutfits );
      qsort( outfits, p->noutfits, sizeof(PilotOutfitSlot*), outfit_compareActive );
   }
   else
      outfits  = p->outfits;

   for (i=0; i<p->noutfits; i++) {

      /* Get active outfits. */
      o = outfits[i];
      if (o->outfit == NULL)
         continue;
      if (!o->active)
         continue;
      if (!outfit_isJammer(o->outfit) &&
            !outfit_isMod(o->outfit) &&
            !outfit_isAfterburner(o->outfit))
         continue;

      /* Set up for creation. */
      lua_pushnumber(L,++k);
      lua_newtable(L);

      /* Name. */
      lua_pushstring(L,"name");
      lua_pushstring(L,o->outfit->name);
      lua_rawset(L,-3);

      /* Type. */
      lua_pushstring(L, "type");
      lua_pushstring(L, outfit_getType(o->outfit));
      lua_rawset(L,-3);

      /* Heat. */
      lua_pushstring(L, "temp");
      lua_pushnumber(L, 1 - pilot_heatEfficiencyMod(o->heat_T,
                            o->outfit->u.afb.heat_base,
                            o->outfit->u.afb.heat_cap));
      lua_rawset(L,-3);

      /* Find the first weapon set containing the outfit, if any. */
      if (outfits[i]->weapset != -1) {
         lua_pushstring(L, "weapset");
         lua_pushnumber(L, outfits[i]->weapset + 1);
         lua_rawset(L, -3);
      }

      /* State and timer. */
      switch (o->state) {
         case PILOT_OUTFIT_OFF:
            str = "off";
            break;
         case PILOT_OUTFIT_WARMUP:
            str = "warmup";
            break;
         case PILOT_OUTFIT_ON:
            str = "on";
            d = outfit_duration(o->outfit);
            if (d==0.)
               d = 1.;
            else if (!isinf(o->stimer))
               d = o->stimer / d;
            lua_pushstring(L,"duration");
            lua_pushnumber(L, d );
            lua_rawset(L,-3);
            break;
         case PILOT_OUTFIT_COOLDOWN:
            str = "cooldown";
            d = outfit_cooldown(o->outfit);
            if (d==0.)
               d = 0.;
            else if (!isinf(o->stimer))
               d = o->stimer / d;
            lua_pushstring(L,"cooldown");
            lua_pushnumber(L, d );
            lua_rawset(L,-3);
            break;
      }
      lua_pushstring(L,"state");
      lua_pushstring(L,str);
      lua_rawset(L,-3);

      /* Set table in table. */
      lua_rawset(L,-3);
   }

   /* Clean up. */
   if (sort)
      free(outfits);

   return 1;
}


/**
 * @brief qsort compare function for active outfits.
 */
static int outfit_compareActive( const void *slot1, const void *slot2 )
{
   const PilotOutfitSlot *s1, *s2;

   s1 = *(const PilotOutfitSlot**) slot1;
   s2 = *(const PilotOutfitSlot**) slot2;

   /* Compare weapon set indexes. */
   if (s1->weapset < s2->weapset)
      return +1;
   else if (s1->weapset > s2->weapset)
      return -1;

   /* Compare positions within the outfit array. */
   if (s1->id < s2->id)
      return +1;
   else if (s1->id > s2->id)
      return -1;

   return 0;
}


/**
 * @brief Gets the outfits of a pilot.
 *
 *    @luatparam Pilot p Pilot to get outfits of.
 *    @luatreturn table The outfits of the pilot in an ordered list.
 * @luafunc outfits( p )
 */
static int pilotL_outfits( lua_State *L )
{
   int i, j;
   Pilot *p;

   /* Parse parameters */
   p  = luaL_validpilot(L,1);

   j  = 1;
   lua_newtable( L );
   for (i=0; i<p->noutfits; i++) {

      /* Get outfit. */
      if (p->outfits[i]->outfit == NULL)
         continue;

      /* Set the outfit. */
      lua_pushnumber( L, j++ );
      lua_pushoutfit( L, p->outfits[i]->outfit );
      lua_rawset( L, -3 );
   }

   return 1;
}


/**
 * @brief Changes the pilot's name.
 *
 * @usage p:rename( "Black Beard" )
 *
 *    @luatparam Pilot p Pilot to change name of.
 *    @luatparam string name Name to change to.
 * @luafunc rename( p, name )
 */
static int pilotL_rename( lua_State *L )
{
   const char *name;
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   name  = luaL_checkstring(L,2);

   /* Change name. */
   if (p->name != NULL)
      free(p->name);
   p->name = strdup(name);

   return 0;
}

/**
 * @brief Gets the pilot's position.
 *
 * @usage v = p:pos()
 *
 *    @luatparam Pilot p Pilot to get the position of.
 *    @luatreturn Vec2 The pilot's current position.
 * @luafunc pos( p )
 */
static int pilotL_position( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push position. */
   lua_pushvector(L, p->solid->pos);
   return 1;
}

/**
 * @brief Gets the pilot's velocity.
 *
 * @usage vel = p:vel()
 *
 *    @luatparam Pilot p Pilot to get the velocity of.
 *    @luatreturn Vec2 The pilot's current velocity.
 * @luafunc vel( p )
 */
static int pilotL_velocity( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push velocity. */
   lua_pushvector(L, p->solid->vel);
   return 1;
}

/**
 * @brief Gets the pilot's evasion.
 *
 * @usage d = p:ew()
 *
 *    @luatparam Pilot p Pilot to get the evasion of.
 *    @luatreturn number The pilot's current evasion value.
 * @luafunc ew( p )
 */
static int pilotL_ew( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push direction. */
   lua_pushnumber( L, p->ew_evasion );
   return 1;
}

/**
 * @brief Gets the pilot's direction.
 *
 * @usage d = p:dir()
 *
 *    @luatparam Pilot p Pilot to get the direction of.
 *    @luatreturn number The pilot's current direction as a number (in degrees).
 * @luafunc dir( p )
 */
static int pilotL_dir( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push direction. */
   lua_pushnumber( L, p->solid->dir * 180./M_PI );
   return 1;
}

/**
 * @brief Gets the temperature of a pilot.
 *
 * @usage t = p:temp()
 *
 *    @luatparam Pilot p Pilot to get temperature of.
 *    @luatreturn number The pilot's current temperature (in kelvin).
 * @luafunc temp( p )
 */
static int pilotL_temp( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push direction. */
   lua_pushnumber( L, p->heat_T );
   return 1;
}

/**
 * @brief Gets the pilot's faction.
 *
 * @usage f = p:faction()
 *
 *    @luatparam Pilot p Pilot to get the faction of.
 *    @luatreturn Faction The faction of the pilot.
 * @luafunc faction( p )
 */
static int pilotL_faction( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push faction. */
   lua_pushfaction(L,p->faction);
   return 1;
}


/**
 * @brief Checks the pilot's spaceworthiness
 *
 * @usage spaceworthy = p:spaceworthy()
 *
 *    @luatparam Pilot p Pilot to get the spaceworthy status of
 *    @luatreturn boolean Whether the pilot's ship is spaceworthy
 * @luafunc spaceworthy( p )
 */
static int pilotL_spaceworthy( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push position. */
   lua_pushboolean( L, (pilot_checkSpaceworthy(p) == NULL) ? 1 : 0 );
   return 1;
}


/**
 * @brief Sets the pilot's position.
 *
 * @usage p:setPos( vec2.new( 300, 200 ) )
 *
 *    @luatparam Pilot p Pilot to set the position of.
 *    @luatparam Vec2 pos Position to set.
 * @luafunc setPos( p, pos )
 */
static int pilotL_setPosition( lua_State *L )
{
   Pilot *p;
   Vector2d *vec;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   vec   = luaL_checkvector(L,2);

   /* Warp pilot to new position. */
   p->solid->pos = *vec;

   /* Update if necessary. */
   if (pilot_isPlayer(p))
      cam_update( 0. );

   return 0;
}

/**
 * @brief Sets the pilot's velocity.
 *
 * @usage p:setVel( vec2.new( 300, 200 ) )
 *
 *    @luatparam Pilot p Pilot to set the velocity of.
 *    @luatparam Vec2 vel Velocity to set.
 * @luafunc setVel( p, vel )
 */
static int pilotL_setVelocity( lua_State *L )
{
   Pilot *p;
   Vector2d *vec;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   vec   = luaL_checkvector(L,2);

   /* Warp pilot to new position. */
   p->solid->vel = *vec;
   return 0;
}

/**
 * @brief Sets the pilot's direction.
 *
 * @note Right is 0, top is 90, left is 180, bottom is 270.
 *
 * @usage p:setDir( 180. )
 *
 *    @luatparam Pilot p Pilot to set the direction of.
 *    @luatparam number dir Direction to set.
 * @luafunc setDir( p, dir )
 */
static int pilotL_setDir( lua_State *L )
{
   Pilot *p;
   double d;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   d     = luaL_checknumber(L,2);

   /* Set direction. */
   p->solid->dir = fmodf( d*M_PI/180., 2*M_PI );
   if (p->solid->dir < 0.)
      p->solid->dir += 2*M_PI;

   return 0;
}

/**
 * @brief Makes the pilot broadcast a message.
 *
 * @usage p:broadcast( "Mayday! Requesting assistance!" )
 * @usage p:broadcast( "Help!", true ) -- Will ignore interference
 *
 *    @luatparam Pilot p Pilot to broadcast the message.
 *    @luatparam string msg Message to broadcast.
 *    @luatparam[opt=false] boolean ignore_int Whether or not it should ignore interference.
 * @luafunc broadcast( p, msg, ignore_int )
 */
static int pilotL_broadcast( lua_State *L )
{
   Pilot *p;
   const char *msg;
   int ignore_int;

   /* Parse parameters. */
   p           = luaL_validpilot(L,1);
   msg         = luaL_checkstring(L,2);
   ignore_int  = lua_toboolean(L,3);

   /* Broadcast message. */
   pilot_broadcast( p, msg, ignore_int );
   return 0;
}

/**
 * @brief Sends a message to the target or player if no target is passed.
 *
 * @usage p:comm( "How are you doing?" ) -- Messages the player
 * @usage p:comm( "You got this?", true ) -- Messages the player ignoring interference
 * @usage p:comm( target, "Heya!" ) -- Messages target
 * @usage p:comm( target, "Got this?", true ) -- Messages target ignoring interference
 *
 *    @luatparam Pilot p Pilot to message the player.
 *    @luatparam Pilot target Target to send message to.
 *    @luatparam string msg Message to send.
 *    @luatparam[opt=false] boolean ignore_int Whether or not it should ignore interference.
 * @luafunc comm( p, target, msg, ignore_int )
 */
static int pilotL_comm( lua_State *L )
{
   Pilot *p, *t;
   LuaPilot target;
   const char *msg;
   int ignore_int;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isstring(L,2)) {
      target = 0;
      msg   = luaL_checkstring(L,2);
      ignore_int = lua_toboolean(L,3);
   }
   else {
      target = luaL_checkpilot(L,2);
      msg   = luaL_checkstring(L,3);
      ignore_int = lua_toboolean(L,4);
   }

   /* Check to see if pilot is valid. */
   if (target == 0)
      t = player.p;
   else {
      t = pilot_get(target);
      if (t == NULL) {
         NLUA_ERROR(L,"Pilot param 2 not found in pilot stack!");
         return 0;
      }
   }

   /* Broadcast message. */
   pilot_message( p, t->id, msg, ignore_int );
   return 0;
}

/**
 * @brief Sets the pilot's faction.
 *
 * @usage p:setFaction( "Empire" )
 * @usage p:setFaction( faction.get( "Dvaered" ) )
 *
 *    @luatparam Pilot p Pilot to change faction of.
 *    @luatparam Faction faction Faction to set by name or faction.
 * @luafunc setFaction( p, faction )
 */
static int pilotL_setFaction( lua_State *L )
{
   Pilot *p;
   int fid;
   const char *faction;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isstring(L,2)) {
      faction = lua_tostring(L,2);
      fid = faction_get(faction);
   }
   else if (lua_isfaction(L,2)) {
      fid = lua_tofaction(L,2);
   }
   else NLUA_INVALID_PARAMETER(L);

   /* Set the new faction. */
   p->faction = fid;

   return 0;
}


/**
 * @brief Controls the pilot's hostility towards the player.
 *
 * @usage p:setHostile() -- Pilot is now hostile.
 * @usage p:setHostile(false) -- Make pilot non-hostile.
 *
 *    @luatparam Pilot p Pilot to set the hostility of.
 *    @luatparam[opt=true] boolean state Whether to set or unset hostile.
 * @luafunc setHostile( p, state )
 */
static int pilotL_setHostile( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set as hostile. */
   if (state) {
      pilot_rmFlag(p, PILOT_FRIENDLY);
      pilot_setHostile(p);
   }
   else
      pilot_rmHostile(p);

   return 0;
}


/**
 * @brief Controls the pilot's friendliness towards the player.
 *
 * @usage p:setFriendly() -- Pilot is now friendly.
 * @usage p:setFriendly(false) -- Make pilot non-friendly.
 *
 *    @luatparam Pilot p Pilot to set the friendliness of.
 *    @luatparam[opt=true] boolean state Whether to set or unset friendly.
 * @luafunc setFriendly( p, state )
 */
static int pilotL_setFriendly( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Remove hostile and mark as friendly. */
   if (state)
      pilot_setFriendly(p);
   /* Remove friendly flag. */
   else
      pilot_rmFriendly(p);

   return 0;
}


/**
 * @brief Sets the pilot's invincibility status.
 *
 * @usage p:setInvincible() -- p can not be hit anymore
 * @usage p:setInvincible(true) -- p can not be hit anymore
 * @usage p:setInvincible(false) -- p can be hit again
 *
 *    @luatparam Pilot p Pilot to set invincibility status of.
 *    @luatparam[opt=true] boolean state State to set invincibility.
 * @luafunc setInvincible( p, state )
 */
static int pilotL_setInvincible( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_INVINCIBLE);
   else
      pilot_rmFlag(p, PILOT_INVINCIBLE);

   return 0;
}


/**
 * @brief Sets the pilot's invincibility status towards the player.
 *
 * @usage p:setInvincPlayer() -- p can not be hit by the player anymore
 * @usage p:setInvincPlayer(true) -- p can not be hit by the player anymore
 * @usage p:setInvincPlayer(false) -- p can be hit by the player again
 *
 *    @luatparam Pilot p Pilot to set invincibility status of (only affects player).
 *    @luatparam[opt=true] boolean state State to set invincibility.
 * @luafunc setInvincPlayer( p, state )
 */
static int pilotL_setInvincPlayer( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_INVINC_PLAYER);
   else
      pilot_rmFlag(p, PILOT_INVINC_PLAYER);

   return 0;
}


/**
 * @brief Sets the pilot's invisibility status.
 *
 * An invisible pilot is neither updated nor drawn. It stays frozen in time
 *  until the invisibility is lifted.
 *
 * @usage p:setInvisible() -- p will disappear
 * @usage p:setInvisible(true) -- p will disappear
 * @usage p:setInvisible(false) -- p will appear again
 *
 *    @luatparam Pilot p Pilot to set invisibility status of.
 *    @luatparam boolean state State to set invisibility.
 * @luafunc setInvisible( p, state )
 */
static int pilotL_setInvisible( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_INVISIBLE);
   else
      pilot_rmFlag(p, PILOT_INVISIBLE);

   return 0;
}


/**
 * @brief Marks the pilot as always visible for the player.
 *
 * This cancels out ewarfare visibility ranges and only affects the visibility of the player.
 *
 * @usage p:setVisplayer( true )
 *
 *    @luatparam Pilot p Pilot to set player visibility status of.
 *    @luatparam[opt=true] boolean state State to set player visibility.
 * @luafunc setVisplayer( p, state )
 */
static int pilotL_setVisplayer( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_VISPLAYER);
   else
      pilot_rmFlag(p, PILOT_VISPLAYER);

   return 0;
}


/**
 * @brief Marks the pilot as always visible for other pilots.
 *
 * This cancels out ewarfare visibility ranges and affects every pilot.
 *
 * @usage p:setVisible( true )
 *
 *    @luatparam Pilot p Pilot to set visibility status of.
 *    @luatparam[opt=true] boolean state State to set visibility.
 * @luafunc setVisible( p, state )
 */
static int pilotL_setVisible( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_VISIBLE);
   else
      pilot_rmFlag(p, PILOT_VISIBLE);

   return 0;
}


/**
 * @brief Makes pilot stand out on radar and the likes.
 *
 * This makes the pilot stand out in the map overlay and radar to increase noticability.
 *
 * @usage p:setHilight( true )
 *
 *    @luatparam Pilot p Pilot to set hilight status of.
 *    @luatparam[opt=true] boolean state State to set hilight.
 * @luafunc setHilight( p, state )
 */
static int pilotL_setHilight( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_HILIGHT);
   else
      pilot_rmFlag(p, PILOT_HILIGHT);

   return 0;
}


/**
 * @brief Allows the pilot to be boarded when not disabled.
 *
 * @usage p:setActiveBoard( true )
 *
 *    @luatparam Pilot p Pilot to set boardability of.
 *    @luatparam[opt=true] boolean state State to set boardability.
 * @luafunc setActiveBoard( p, state )
 */
static int pilotL_setActiveBoard( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_BOARDABLE);
   else
      pilot_rmFlag(p, PILOT_BOARDABLE);

   return 0;
}


/**
 * @brief Makes it so the pilot never dies, stays at 1. armour.
 *
 * @usage p:setNoDeath( true ) -- Pilot will never die
 *
 *    @luatparam Pilot p Pilot to set never die state of.
 *    @luatparam[opt=true] boolean state Whether or not to set never die state.
 * @luafunc setNoDeath( p, state )
 */
static int pilotL_setNoDeath( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_NODEATH);
   else
      pilot_rmFlag(p, PILOT_NODEATH);

   return 0;
}


/**
 * @brief Disables a pilot.
 *
 * @usage p:disable()
 *
 *    @luatparam Pilot p Pilot to disable.
 * @luafunc disable( p )
 */
static int pilotL_disable( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Disable the pilot. */
   p->shield = 0.;
   p->stress = p->armour;
   pilot_updateDisable(p, 0);

   return 0;
}


/**
 * @brief Gets a pilot's cooldown state.
 *
 * @usage cooldown, braking = p:cooldown()
 *
 *    @luatparam Pilot p Pilot to check the cooldown status of.
 *    @luatreturn boolean Cooldown status.
 *    @luatreturn boolean Cooldown braking status.
 * @luafunc cooldown( p )
 */
static int pilotL_cooldown( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get the cooldown status. */
   lua_pushboolean( L, pilot_isFlag(p, PILOT_COOLDOWN) );
   lua_pushboolean( L, pilot_isFlag(p, PILOT_COOLDOWN_BRAKE) );

   return 2;
}


/**
 * @brief Starts or stops a pilot's cooldown mode.
 *
 * @usage p:setCooldown( true )
 *
 *    @luatparam Pilot p Pilot to modify the cooldown status of.
 *    @luatparam[opt=true] boolean state Whether to enable or disable cooldown.
 * @luafunc setCooldown( p, state )
 */
static int pilotL_setCooldown( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

  /* Get state. */
  if (lua_gettop(L) > 1)
     state = lua_toboolean(L, 2);
  else
     state = 1;

   /* Set status. */
   if (state)
      pilot_cooldown( p );
   else
      pilot_cooldownEnd(p, NULL);

   return 0;
}


/**
 * @brief Enables or disables a pilot's hyperspace engine.
 *
 * @usage p:setNoJump( true )
 *
 *    @luatparam Pilot p Pilot to modify.
 *    @luatparam[opt=true] boolean state true or false
 * @luafunc setNoJump( p, state )
 */
static int pilotL_setNoJump( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_NOJUMP);
   else
      pilot_rmFlag(p, PILOT_NOJUMP);

   return 0;
}


/**
 * @brief Enables or disables landing for a pilot.
 *
 * @usage p:setNoLand( true )
 *
 *    @luatparam Pilot p Pilot to modify.
 *    @luatparam[opt] boolean state true or false
 * @luafunc setNoLand( p, state )
 */
static int pilotL_setNoLand( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_gettop(L) > 1)
      state = lua_toboolean(L, 2);
   else
      state = 1;

   /* Set status. */
   if (state)
      pilot_setFlag(p, PILOT_NOLAND);
   else
      pilot_rmFlag(p, PILOT_NOLAND);

   return 0;
}


/**
 * @brief Adds an outfit to a pilot.
 *
 * This by default tries to add them to the first empty or defaultly equipped slot.
 *
 * @usage added = p:addOutfit( "Laser Cannon", 5 ) -- Adds 5 laser cannons to p
 *
 *    @luatparam Pilot p Pilot to add outfit to.
 *    @luatparam string outfit Name of the outfit to add.
 *    @luatparam[opt=1] number q Quantity of the outfit to add.
 *    @luatparam[opt=false] boolean bypass Whether to skip CPU and slot size checks before adding an outfit.
 *              Will not overwrite existing non-default outfits.
 *    @luatreturn number The number of outfits added.
 * @luafunc addOutfit( p, outfit, q, bypass )
 */
static int pilotL_addOutfit( lua_State *L )
{
   int i;
   Pilot *p;
   const char *outfit;
   Outfit *o;
   int ret;
   int q, added, bypass;

   /* Get parameters. */
   p      = luaL_validpilot(L,1);
   outfit = luaL_checkstring(L,2);
   q      = 1;
   if (lua_gettop(L) > 2 && !lua_isnil(L,2))
      q = luaL_checkint(L,3);
   if (lua_gettop(L) > 3)
      bypass = lua_toboolean(L, 4);
   else
      bypass = 0;

   /* Get the outfit. */
   o = outfit_get( outfit );
   if (o == NULL) {
      NLUA_ERROR(L, "Outfit '%s' not found!", outfit );
      return 0;
   }

   /* Add outfit. */
   added = 0;
   for (i=0; i<p->noutfits; i++) {
      /* Must still have to add outfit. */
      if (q <= 0)
         break;

      /* Must not have outfit (excluding default) already. */
      if ((p->outfits[i]->outfit != NULL) &&
            (p->outfits[i]->outfit != p->outfits[i]->sslot->data))
         continue;

      if (!bypass) {
         /* Must fit slot. */
         if (!outfit_fitsSlot( o, &p->outfits[i]->sslot->slot ))
            continue;

         /* Test if can add outfit. */
         ret = pilot_addOutfitTest( p, o, p->outfits[i], 0 );
         if (ret)
            break;
      }
      /* Only do a basic check. */
      else
         if (!outfit_fitsSlotType( o, &p->outfits[i]->sslot->slot ))
            continue;

      /* Add outfit - already tested. */
      ret = pilot_addOutfitRaw( p, o, p->outfits[i] );
      pilot_calcStats( p );

      /* Add ammo if needed. */
      if ((ret==0) && (outfit_ammo(o) != NULL))
         pilot_addAmmo( p, p->outfits[i], outfit_ammo(o), outfit_amount(o) );

      /* We added an outfit. */
      q--;
      added++;
   }

   /* Update the weapon sets. */
   if ((added > 0) && p->autoweap)
      pilot_weaponAuto(p);

   /* Update equipment window if operating on the player's pilot. */
   if (player.p != NULL && player.p == p && added > 0)
      outfits_updateEquipmentOutfits();

   lua_pushnumber(L,added);
   return 1;
}


/**
 * @brief Removes an outfit from a pilot.
 *
 * "all" will remove all outfits except cores.
 * "cores" will remove all cores, but nothing else.
 *
 * @usage p:rmOutfit( "all" ) -- Leaves the pilot naked (except for cores).
 * @usage p:rmOutfit( "cores" ) -- Strips the pilot of its cores, leaving it dead in space.
 * @usage p:rmOutfit( "Neutron Disruptor" ) -- Removes a neutron disruptor.
 * @usage p:rmOutfit( "Neutron Disruptor", 2 ) -- Removes two neutron disruptor.
 *
 *    @luatparam Pilot p Pilot to remove outfit from.
 *    @luatparam string outfit Name of the outfit to remove.
 *    @luatparam number q Quantity of the outfit to remove.
 *    @luatreturn number The number of outfits removed.
 * @luafunc rmOutfit( p, outfit, q )
 */
static int pilotL_rmOutfit( lua_State *L )
{
   int i;
   Pilot *p;
   const char *outfit;
   Outfit *o;
   int q, removed;

   /* Get parameters. */
   removed = 0;
   p      = luaL_validpilot(L,1);
   outfit = luaL_checkstring(L,2);
   q      = 1;
   if (lua_gettop(L) > 2)
      q = luaL_checkint(L,3);

   /* If outfit is "all", we remove everything except cores. */
   if (strcmp(outfit,"all")==0) {
      for (i=0; i<p->noutfits; i++) {
         if (p->outfits[i]->sslot->required)
            continue;
         pilot_rmOutfitRaw( p, p->outfits[i] );
         removed++;
      }
      pilot_calcStats( p ); /* Recalculate stats. */
   }
   /* If outfit is "cores", we remove cores only. */
   else if (strcmp(outfit,"cores")==0) {
      for (i=0; i<p->noutfits; i++) {
         if (!p->outfits[i]->sslot->required)
            continue;
         pilot_rmOutfitRaw( p, p->outfits[i] );
         removed++;
      }
      pilot_calcStats( p ); /* Recalculate stats. */
   }
   else {
      /* Get the outfit. */
      o = outfit_get( outfit );
      if (o == NULL) {
         NLUA_ERROR(L,"Outfit isn't found in outfit stack.");
         return 0;
      }

      /* Remove the outfit outfit. */
      for (i=0; i<p->noutfits; i++) {
         /* Must still need to remove. */
         if (q <= 0)
            break;

         /* Not found. */
         if (p->outfits[i]->outfit != o)
            continue;

         /* Remove outfit. */
         pilot_rmOutfit( p, p->outfits[i] );
         q--;
         removed++;
      }
   }

   /* Update equipment window if operating on the player's pilot. */
   if (player.p != NULL && player.p == p && removed > 0)
      outfits_updateEquipmentOutfits();

   lua_pushnumber( L, removed );
   return 1;
}


/**
 * @brief Sets the fuel of a pilot.
 *
 * @usage p:setFuel( true ) -- Sets fuel to max
 *
 *    @luatparam Pilot p Pilot to set fuel of.
 *    @luatparam boolean|number f true sets fuel to max, false sets fuel to 0, a number sets
 *              fuel to that amount in units.
 *    @luatreturn number The amount of fuel the pilot has.
 * @luafunc setFuel( p, f )
 */
static int pilotL_setFuel( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get the parameter. */
   if (lua_isboolean(L,2)) {
      if (lua_toboolean(L,2))
         p->fuel = p->fuel_max;
      else
         p->fuel = 0.;
   }
   else if (lua_isnumber(L,2)) {
      p->fuel = CLAMP( 0., p->fuel_max, lua_tonumber(L,2) );
   }
   else
      NLUA_INVALID_PARAMETER(L);

   /* Return amount of fuel. */
   lua_pushnumber(L, p->fuel);
   return 1;
}


/**
 * @brief Changes the pilot's AI.
 *
 * @usage p:changeAI( "empire" ) -- set the pilot to use the Empire AI
 *
 *    @luatparam Pilot p Pilot to change AI of.
 *    @luatparam string newai Name of Ai to use.
 *
 * @luafunc changeAI( p, newai )
 */
static int pilotL_changeAI( lua_State *L )
{
   Pilot *p;
   const char *str;
   int ret;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   str = luaL_checkstring(L,2);

   /* Get rid of current AI. */
   ai_destroy(p);

   /* Create the new AI. */
   ret = ai_pinit( p, str );
   lua_pushboolean(L, ret);
   return 1;
}


/**
 * @brief Sets the temperature of a pilot.
 *
 * All temperatures are in Kelvins. Note that temperatures cannot go below the base temperature of the Naev galaxy, which is 250K.
 *
 * @usage p:setTemp( 300, true ) -- Sets ship temperature to 300K, as well as all outfits.
 * @usage p:setTemp( 500, false ) -- Sets ship temperature to 500K, but leaves outfits alone.
 * @usage p:setTemp( 0 ) -- Sets ship temperature to the base temperature, as well as all outfits.
 *
 *    @luatparam Pilot p Pilot to set health of.
 *    @luatparam number temp Value to set temperature to. Values below base temperature will be clamped.
 *    @luatparam[opt=true] boolean slots Whether slots should also be set to this temperature.
 * @luafunc setTemp( p, temp, slots )
 */
static int pilotL_setTemp( lua_State *L )
{
   Pilot *p;
   int i, setOutfits = 1;
   double kelvins;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   kelvins  = luaL_checknumber(L, 2);
   if (lua_gettop(L) < 3)
      setOutfits = 1;
   else
      setOutfits = lua_toboolean(L, 3);

   /* Temperature must not go below base temp. */
   kelvins = MAX(kelvins, CONST_SPACE_STAR_TEMP);

   /* Handle pilot ship. */
   p->heat_T = kelvins;

   /* Handle pilot outfits (maybe). */
   if (setOutfits)
      for (i = 0; i < p->noutfits; i++)
         p->outfits[i]->heat_T = kelvins;

   return 0;
}


/**
 * @brief Sets the health of a pilot.
 *
 * This recovers the pilot's disabled state, although he may become disabled afterwards.
 *
 * @usage p:setHealth( 100, 100 ) -- Sets pilot to full health
 * @usage p:setHealth(  70,   0 ) -- Sets pilot to 70% armour
 * @usage p:setHealth( 100, 100, 0 ) -- Sets pilot to full health and no stress
 *
 *    @luatparam Pilot p Pilot to set health of.
 *    @luatparam number armour Value to set armour to, should be double from 0-100 (in percent).
 *    @luatparam number shield Value to set shield to, should be double from 0-100 (in percent).
 *    @luatparam[opt=0] number stress Value to set stress (disable damage) to, should be double from 0-100 (in percent of current armour).
 * @luafunc setHealth( p, armour, shield, stress )
 */
static int pilotL_setHealth( lua_State *L )
{
   Pilot *p;
   double a, s, st;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   a  = luaL_checknumber(L, 2);
   s  = luaL_checknumber(L, 3);
   if (lua_gettop(L) > 3)
      st = luaL_checknumber(L, 4);
   else
      st = 0;

   a  /= 100.;
   s  /= 100.;
   st /= 100.;

   /* Set health. */
   p->armour = a * p->armour_max;
   p->shield = s * p->shield_max;
   p->stress = st * p->armour;

   /* Update disable status. */
   pilot_updateDisable(p, 0);

   return 0;
}


/**
 * @brief Sets the energy of a pilot.
 *
 * @usage p:setEnergy( 100 ) -- Sets pilot to full energy.
 * @usage p:setEnergy(  70 ) -- Sets pilot to 70% energy.
 *
 *    @luatparam Pilot p Pilot to set energy of.
 *    @luatparam number energy Value to set energy to, should be double from 0-100 (in percent).
 *
 * @luafunc setEnergy( p, energy )
 */
static int pilotL_setEnergy( lua_State *L )
{
   Pilot *p;
   double e;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   e  = luaL_checknumber(L, 2);
   e /= 100.;

   /* Set energy. */
   p->energy = e * p->energy_max;

   return 0;
}


/**
 * @brief Sets the ability to board the pilot.
 *
 * No parameter is equivalent to true.
 *
 * @usage p:setNoboard( true ) -- Pilot can not be boarded by anyone
 *
 *    @luatparam Pilot p Pilot to set disable boarding.
 *    @luatparam[opt=true] number noboard If true it disallows boarding of the pilot, otherwise
 *              it allows boarding which is the default.
 * @luafunc setNoboard( p, noboard )
 */
static int pilotL_setNoboard( lua_State *L )
{
   Pilot *p;
   int disable;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_gettop(L)==1)
      disable = 1;
   else
      disable = lua_toboolean(L, 2);

   /* See if should prevent boarding. */
   if (disable)
      pilot_setFlag(p, PILOT_NOBOARD);
   else
      pilot_rmFlag(p, PILOT_NOBOARD);

   return 0;
}


/**
 * @brief Sets the ability of the pilot to be disabled.
 *
 * No parameter is equivalent to true.
 *
 * @usage p:setNodisable( true ) -- Pilot can not be disabled anymore.
 *
 *    @luatparam Pilot p Pilot to set disable disabling.
 *    @luatparam[opt=true] boolean disable If true it disallows disabled of the pilot, otherwise
 *              it allows disabling which is the default.
 * @luafunc setNodisable( p, disable )
 */
static int pilotL_setNodisable( lua_State *L )
{
   Pilot *p;
   int disable;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_gettop(L)==1)
      disable = 1;
   else
      disable = lua_toboolean(L, 2);

   /* See if should prevent disabling. */
   if (disable)
      pilot_setFlag(p, PILOT_NODISABLE);
   else
      pilot_rmFlag(p, PILOT_NODISABLE);

   return 0;
}


/**
 * @brief Limits the speed of a pilot.
 *
 * @usage p:setSpeedLimit( 100 ) -- Sets maximumspeed to 100px/s.
 * @usage p:setSpeedLimit( 0 ) removes speed limit.
 *    @luatparam pilot p Pilot to set speed of.
 *    @luatparam number speed Value to set speed to.
 *
 * @luafunc setSpeedLimit( p, speed )
 */
static int pilotL_setSpeedLimit(lua_State* L)
{

   Pilot *p;
   double s;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   s  = luaL_checknumber(L, 2);

   /* Limit the speed */
   p->speed_limit = s;
   if (s > 0.)
     pilot_setFlag( p, PILOT_HASSPEEDLIMIT );
   else
     pilot_rmFlag( p, PILOT_HASSPEEDLIMIT );

   pilot_updateMass(p);
   return 0;
}


/**
 * @brief Gets the pilot's health.
 *
 * @usage armour, shield, stress, dis = p:health()
 *
 *    @luatparam Pilot p Pilot to get health of.
 *    @luatreturn number The armour in % [0:100]. 
 *    @luatreturn number The shield in % [0:100].
 *    @luatreturn number The stress in % [0:100].
 *    @luatreturn boolean Indicates if pilot is disabled.
 * @luafunc health( p )
 */
static int pilotL_getHealth( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Return parameters. */
   lua_pushnumber(L,(p->armour_max > 0.) ? p->armour / p->armour_max * 100. : 0. );
   lua_pushnumber(L,(p->shield_max > 0.) ? p->shield / p->shield_max * 100. : 0. );
   lua_pushnumber(L, MIN( 1., p->stress / p->armour ) * 100. );
   lua_pushboolean(L, pilot_isDisabled(p));

   return 4;
}


/**
 * @brief Gets the pilot's energy.
 *
 * @usage energy = p:energy()
 *
 *    @luatparam Pilot p Pilot to get energy of.
 *    @luatreturn number The energy of the pilot in % [0:100].
 * @luafunc energy( p )
 */
static int pilotL_getEnergy( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Return parameter. */
   lua_pushnumber(L, (p->energy_max > 0.) ? p->energy / p->energy_max * 100. : 0. );

   return 1;
}


/**
 * @brief Gets the lockons on the pilot.
 *
 * @usage lockon = p:lockon()
 *
 *    @luatparam Pilot p Pilot to get lockons of.
 *    @luatreturn number The number of lockons on the pilot.
 * @luafunc lockon( p )
 */
static int pilotL_getLockon( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Return. */
   lua_pushnumber(L, p->lockons );
   return 1;
}


#define PUSH_DOUBLE( L, name, value ) \
lua_pushstring( L, name ); \
lua_pushnumber( L, value ); \
lua_rawset( L, -3 )
#define PUSH_INT( L, name, value ) \
lua_pushstring( L, name ); \
lua_pushinteger( L, value ); \
lua_rawset( L, -3 )
/**
 * @brief Gets stats of the pilot.
 *
 * Some of the stats are:<br />
 * <ul>
 *  <li> cpu </li>
 *  <li> cpu_max </li>
 *  <li> crew </li>
 *  <li> fuel </li>
 *  <li> fuel_max </li>
 *  <li> fuel_consumption </li>
 *  <li> mass </li>
 *  <li> thrust </li>
 *  <li> speed </li>
 *  <li> speed_max </li>
 *  <li> turn </li>
 *  <li> absorb </li>
 *  <li> armour </li>
 *  <li> shield </li>
 *  <li> energy </li>
 *  <li> armour_regen </li>
 *  <li> shield_regen </li>
 *  <li> energy_regen </li>
 *  <li> jump_delay </li>
 *  <li> jumps </li>
 * </ul>
 *
 * @usage stats = p:stats() print(stats.armour)
 *
 *    @luatparam Pilot p Pilot to get stats of.
 *    @luatreturn table A table containing the stats of p.
 * @luafunc stats( p )
 */
static int pilotL_getStats( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Create table with information. */
   lua_newtable(L);
   /* Core. */
   PUSH_DOUBLE( L, "cpu", p->cpu );
   PUSH_INT( L, "cpu_max", p->cpu_max );
   PUSH_INT( L, "crew", (int)round( p->crew ) );
   PUSH_DOUBLE( L, "fuel", p->fuel );
   PUSH_DOUBLE( L, "fuel_max", p->fuel_max );
   PUSH_DOUBLE( L, "fuel_consumption", p->fuel_consumption );
   PUSH_DOUBLE( L, "mass", p->solid->mass );
   /* Movement. */
   PUSH_DOUBLE( L, "thrust", p->thrust / p->solid->mass );
   PUSH_DOUBLE( L, "speed", p->speed );
   PUSH_DOUBLE( L, "turn", p->turn * 180. / M_PI ); /* Convert back to grad. */
   PUSH_DOUBLE( L, "speed_max", solid_maxspeed(p->solid, p->speed, p->thrust) );
   /* Health. */
   PUSH_DOUBLE( L, "absorb", p->dmg_absorb );
   PUSH_DOUBLE( L, "armour", p->armour_max );
   PUSH_DOUBLE( L, "shield", p->shield_max );
   PUSH_DOUBLE( L, "energy", p->energy_max );
   PUSH_DOUBLE( L, "armour_regen", p->armour_regen );
   PUSH_DOUBLE( L, "shield_regen", p->shield_regen );
   PUSH_DOUBLE( L, "energy_regen", p->energy_regen );
   /* Stats. */
   PUSH_DOUBLE( L, "jump_delay", ntime_convertSTU( pilot_hyperspaceDelay(p) ) );
   PUSH_INT( L, "jumps", pilot_getJumps(p) );

   return 1;
}
#undef PUSH_DOUBLE


/**
 * @brief Gets the free cargo space the pilot has.
 *
 *    @luatparam Pilot p The pilot to get the free cargo space of.
 *    @luatreturn number The free cargo space in tons of the player.
 * @luafunc cargoFree( p )
 */
static int pilotL_cargoFree( lua_State *L )
{
   Pilot *p;
   p = luaL_validpilot(L,1);

   lua_pushnumber(L, pilot_cargoFree(p) );
   return 1;
}


/**
 * @brief Checks to see how many tons of a specific type of cargo the pilot has.
 *
 *    @luatparam Pilot p The pilot to get the cargo count of.
 *    @luatparam string type Type of cargo to count.
 *    @luatreturn number The amount of cargo the player has.
 * @luafunc cargoHas( p, type )
 */
static int pilotL_cargoHas( lua_State *L )
{
   Pilot *p;
   const char *str;
   int quantity;

   p = luaL_validpilot(L,1);
   str = luaL_checkstring( L, 2 );
   quantity = pilot_cargoOwned( p, str );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Tries to add cargo to the pilot's ship.
 *
 * @usage n = pilot.cargoAdd( player.pilot(), "Food", 20 )
 *
 *    @luatparam Pilot p The pilot to add cargo to.
 *    @luatparam string type Name of the cargo to add.
 *    @luatparam number quantity Quantity of cargo to add.
 *    @luatreturn number The quantity of cargo added.
 * @luafunc cargoAdd( p, type, quantity )
 */
static int pilotL_cargoAdd( lua_State *L )
{
   Pilot *p;
   const char *str;
   int quantity;
   Commodity *cargo;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   str      = luaL_checkstring( L, 2 );
   quantity = luaL_checknumber( L, 3 );

   /* Get cargo. */
   cargo    = commodity_get( str );
   if (cargo == NULL) {
      NLUA_ERROR( L, "Cargo '%s' does not exist!", str );
      return 0;
   }

   if (quantity < 0) {
      NLUA_ERROR( L, "Quantity must be positive for pilot.cargoAdd (if removing, use pilot.cargoRm)" );
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_cargoAdd( p, cargo, quantity, 0 );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Tries to remove cargo from the pilot's ship.
 *
 * @usage n = pilot.cargoRm( player.pilot(), "Food", 20 )
 *
 *    @luatparam Pilot p The pilot to remove cargo from.
 *    @luatparam string type Name of the cargo to remove.
 *    @luatparam number quantity Quantity of the cargo to remove.
 *    @luatreturn number The number of cargo removed.
 * @luafunc cargoRm( p, type, quantity )
 */
static int pilotL_cargoRm( lua_State *L )
{
   Pilot *p;
   const char *str;
   int quantity;
   Commodity *cargo;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   str      = luaL_checkstring( L, 2 );
   quantity = luaL_checknumber( L, 3 );

   /* Get cargo. */
   cargo    = commodity_get( str );
   if (cargo == NULL) {
      NLUA_ERROR( L, "Cargo '%s' does not exist!", str );
      return 0;
   }

   if (quantity < 0) {
      NLUA_ERROR( L, "Quantity must be positive for pilot.cargoRm (if adding, use pilot.cargoAdd)" );
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_cargoRm( p, cargo, quantity );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Lists the cargo the pilot has.
 *
 * The list has the following members:<br />
 * <ul>
 * <li><b>name:</b> name of the cargo.</li>
 * <li><b>q:</b> quantity of the cargo.</li>
 * <li><b>m:</b> true if cargo is for a mission.</li>
 * </ul>
 *
 * @usage for _,v in ipairs(pilot.cargoList(player.pilot())) do print( string.format("%s: %d", v.name, v.q ) ) end
 *
 *    @luatparam Pilot p Pilot to list cargo of.
 *    @luatreturn table An ordered list with the names of the cargo the pilot has.
 * @luafunc cargoList( p )
 */
static int pilotL_cargoList( lua_State *L )
{
   Pilot *p;
   int i;

   p = luaL_validpilot(L,1);
   lua_newtable(L); /* t */
   for (i=0; i<p->ncommodities; i++) {
      lua_pushnumber(L, i+1); /* t, i */

      /* Represents the cargo. */
      lua_newtable(L); /* t, i, t */
      lua_pushstring(L, "name"); /* t, i, t, i */
      lua_pushstring(L, p->commodities[i].commodity->name); /* t, i, t, i, s */
      lua_rawset(L,-3); /* t, i, t */
      lua_pushstring(L, "q"); /* t, i, t, i */
      lua_pushnumber(L, p->commodities[i].quantity); /* t, i, t, i, s */
      lua_rawset(L,-3); /* t, i, t */
      lua_pushstring(L, "m"); /* t, i, t, i */
      lua_pushboolean(L, p->commodities[i].id); /* t, i, t, i, s */
      lua_rawset(L,-3); /* t, i, t */

      lua_rawset(L,-3); /* t */
   }
   return 1;

}


/**
 * @brief Gets the pilot's colour based on hostility or friendliness to the player.
 *
 * @usage p:colour()
 *
 *    @luatparam Pilot p Pilot to get the colour of.
 *    @luatreturn Colour The pilot's colour.
 * @luafunc colour( p )
 */
static int pilotL_getColour( lua_State *L )
{
   Pilot *p;
   const glColour *col;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   col = pilot_getColour(p);
   lua_pushcolour( L, *col );

   return 1;
}


/**
 * @brief Returns whether the pilot is hostile to the player.
 *
 * @usage p:hostile()
 *
 *    @luatparam Pilot p Pilot to get the hostility of.
 *    @luatreturn boolean The pilot's hostility status.
 * @luafunc hostile( p )
 */
static int pilotL_getHostile( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Push value. */
   lua_pushboolean( L, pilot_isFlag(p, PILOT_HOSTILE) );
   return 1;
}


/**
 * @brief Small struct to handle flags.
 */
struct pL_flag {
   char *name; /**< Name of the flag. */
   int id;     /**< Id of the flag. */
};
static const struct pL_flag pL_flags[] = {
   { .name = "hailing", .id = PILOT_HAILING },
   { .name = "boardable", .id = PILOT_BOARDABLE },
   { .name = "nojump", .id = PILOT_NOJUMP },
   { .name = "noland", .id = PILOT_NOLAND },
   { .name = "nodeath", .id = PILOT_NODEATH },
   { .name = "nodisable", .id = PILOT_NODISABLE },
   { .name = "escort", .id = PILOT_ESCORT },
   { .name = "visible", .id = PILOT_VISIBLE },
   { .name = "visplayer", .id = PILOT_VISPLAYER },
   { .name = "hilight", .id = PILOT_HILIGHT },
   { .name = "invisible", .id = PILOT_INVISIBLE },
   { .name = "invincible", .id = PILOT_INVINCIBLE },
   { .name = "invinc_player", .id = PILOT_INVINC_PLAYER },
   { .name = "friendly", .id = PILOT_FRIENDLY },
   { .name = "hostile", .id = PILOT_HOSTILE },
   { .name = "refueling", .id = PILOT_REFUELING },
   { .name = "disabled", .id = PILOT_DISABLED },
   { .name = "takingoff", .id = PILOT_TAKEOFF },
   { .name = "manualcontrol", .id = PILOT_MANUAL_CONTROL },
   { .name = "combat", .id = PILOT_COMBAT },
   {NULL, -1}
}; /**< Flags to get. */
/**
 * @brief Gets the pilot's flags.
 *
 * Valid flags are:<br/>
 * <ul>
 *  <li> hailing: pilot is hailing the player.</li>
 *  <li> boardable: pilot is boardable while active.</li>
 *  <li> nojump: pilot cannot jump.</li>
 *  <li> noland: pilot cannot land.</li>
 *  <li> nodeath: pilot cannot die.</li>
 *  <li> nodisable: pilot cannot be disabled.</li>
 *  <li> escort: pilot is an escort.</li>
 *  <li> visible: pilot is always visible.</li>
 *  <li> visplayer: pilot is always visible to the player.</li>
 *  <li> hilight: pilot is hilighted on the map.</li>
 *  <li> invisible: pilot is not displayed.</li>
 *  <li> invincible: pilot cannot be hit.</li>
 *  <li> invinc_player: pilot cannot be hit by the player.</li>
 *  <li> friendly: pilot is friendly toward the player.</li>
 *  <li> hostile: pilot is hostile toward the player.</li>
 *  <li> refueling: pilot is refueling another pilot.</li>
 *  <li> disabled: pilot is disabled.</li>
 *  <li> takingoff: pilot is currently taking off.</li>
 *  <li> manualcontrol: pilot is under manual control.</li>
 *  <li> combat: pilot is engaged in combat.</li>
 * </ul>
 *    @luatparam Pilot p Pilot to get flags of.
 *    @luatreturn table Table with flag names an index, boolean as value.
 * @luafunc flags( p )
 */
static int pilotL_flags( lua_State *L )
{
   int i;
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Create flag table. */
   lua_newtable(L);
   for (i=0; pL_flags[i].name != NULL; i++) {
      lua_pushboolean( L, pilot_isFlag( p, pL_flags[i].id ) );
      lua_setfield(L, -2, pL_flags[i].name);
   }
   return 1;
}


/**
 * @brief Gets the pilot's ship.
 *
 * @usage s = p:ship()
 *
 *    @luatparam Pilot p Pilot to get ship of.
 *    @luatreturn Ship The ship of the pilot.
 * @luafunc ship( p )
 */
static int pilotL_ship( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Create the ship. */
   lua_pushship(L, p->ship);
   return 1;
}


/**
 * @brief Checks to see if the pilot is idle.
 *
 * @usage idle = p:idle() -- Returns true if the pilot is idle
 *
 *    @luatparam Pilot p Pilot to check to see if is idle.
 *    @luatreturn boolean true if pilot is idle, false otherwise
 * @luafunc idle( p )
 */
static int pilotL_idle( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Check to see if is idle. */
   lua_pushboolean(L, p->task==0);
   return 1;
}


/**
 * @brief Sets manual control of the pilot.
 *
 * @usage p:control() -- Same as p:control(true), enables manual control of the pilot
 * @usage p:control(false) -- Restarts AI control of the pilot
 *
 *    @luatparam Pilot p Pilot to change manual control settings.
 *    @luatparam[opt=1] boolean enable If true or nil enables pilot manual control, otherwise enables automatic AI.
 * @luasee goto
 * @luasee brake
 * @luasee follow
 * @luasee attack
 * @luasee runaway
 * @luasee hyperspace
 * @luasee land
 * @luafunc control( p, enable )
 */
static int pilotL_control( lua_State *L )
{
   Pilot *p;
   int enable;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_gettop(L)==1)
      enable = 1;
   else
      enable = lua_toboolean(L, 2);

   if (enable) {
      pilot_setFlag(p, PILOT_MANUAL_CONTROL);
      if (pilot_isPlayer(p))
         ai_pinit( p, "player" );
   }
   else {
      pilot_rmFlag(p, PILOT_MANUAL_CONTROL);
      if (pilot_isPlayer(p))
         ai_destroy( p );
      /* Note, we do not set p->ai to NULL, we just clear the tasks and memory.
       * This is because the player always has an ai named "player", which is
       * used for manual control among other things. Basically a pilot always
       * has to have an AI even if it's the player for things to work. */
   }

   /* Clear task. */
   pilotL_taskclear( L );

   return 0;
}


/**
 * @brief Copies a value between two states.
 *
 *    @param to State to copy to.
 *    @param from State to copy from.
 *    @param ind Index of item to copy from state.
 * @return 0 on success.
 */
static int lua_copyvalue( lua_State *to, lua_State *from, int ind )
{
   switch (lua_type( from, ind )) {
      case LUA_TNIL:
         lua_pushnil( to);
         break;
      case LUA_TNUMBER:
         lua_pushnumber( to, lua_tonumber(from, ind) );
         break;
      case LUA_TBOOLEAN:
         lua_pushboolean( to, lua_toboolean(from, ind) );
         break;
      case LUA_TSTRING:
         lua_pushstring( to, lua_tostring(from, ind) );
         break;

      default:
         NLUA_ERROR(from,"Unsupported value of type '%s'", lua_typename(from, ind));
         break;
   }
   return 0;
}


/**
 * @brief Changes a parameter in the pilot's memory.
 *
 *    @luatparam Pilot p Pilot to change memory of.
 *    @luatparam string key Key of the memory part to change.
 *    @luatparam nil|number|boolean|string value Value to set to.
 * @luafunc memory( p, key, value )
 */
static int pilotL_memory( lua_State *L )
{
   lua_State *pL;
   Pilot *p;

   if (lua_gettop(L) < 3) {
      NLUA_ERROR(L, "pilot.memory requires 3 arguments!");
      return 0;
   }

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Set the pilot's memory. */
   if (p->ai == NULL) {
      NLUA_ERROR(L,"Pilot does not have AI.");
      return 0;
   }
   pL = p->ai->L;

   /* Copy it over. */
   lua_getglobal( pL, AI_MEM );        /* pilotmem */
   lua_pushnumber( pL, p->id );        /* pilotmem, id */
   lua_gettable( pL, -2);              /* pilotmem, table */
   lua_copyvalue( pL, L, 2 );          /* pilotmem, table, key */
   lua_copyvalue( pL, L, 3 );          /* pilotmem, table, key, value */
   lua_settable( pL, -3 );             /* pilotmem, table */
   lua_pop( pL, 2 );                   /* */

   return 0;
}


/**
 * @brief Gets the value of the pilot's memory.
 *
 * Equivalent to reading the pilot's "mem.str".
 *
 * @usage aggr = p:memoryCheck( "aggressive" ) -- See if the pilot is aggressive
 *
 *    @luatparam Pilot p Pilot to get memory of.
 *    @luatparam string str Name of the memory to get.
 *    @luatreturn nil|number|boolean|string The value stored.
 * @luafunc memoryCheck( p, str )
 */
static int pilotL_memoryCheck( lua_State *L )
{
   lua_State *pL;
   Pilot *p;

   if (lua_gettop(L) < 2) {
      NLUA_ERROR(L, "pilot.memoryCheck requires 2 arguments!");
      return 0;
   }

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Set the pilot's memory. */
   if (p->ai == NULL) {
      NLUA_ERROR(L,"Pilot does not have AI.");
      return 0;
   }
   pL = p->ai->L;

   /* Copy it over. */
   lua_getglobal( pL, AI_MEM );        /* pilotmem */
   lua_pushnumber( pL, p->id );        /* pilotmem, id */
   lua_gettable( pL, -2);              /* pilotmem, table */
   lua_copyvalue( pL, L, 2 );          /* pilotmem, table, key */
   lua_gettable( pL, -2 );             /* pilotmem, table, value */
   lua_copyvalue( L, pL, -1 );         /* pilotmem, table, value */
   lua_pop( pL, 3 );                   /* */

   return 1;
}


/**
 * @brief Clears all the tasks of the pilot.
 *
 * @usage p:taskClear()
 *
 *    @luatparam Pilot p Pilot to clear tasks of.
 * @luafunc taskClear( p )
 */
static int pilotL_taskclear( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Clean up tasks. */
   ai_cleartasks( p );

   return 0;
}


/**
 * @brief Does a new task.
 */
static Task *pilotL_newtask( lua_State *L, Pilot* p, const char *task )
{
   Task *t;

   /* Must be on manual control. */
   if (!pilot_isFlag( p, PILOT_MANUAL_CONTROL)) {
      NLUA_ERROR( L, "Pilot is not on manual control." );
      return 0;
   }

   /* Creates the new task. */
   t = ai_newtask( p, task, 0, 1 );

   return t;
}


/**
 * @brief Makes the pilot goto a position.
 *
 * Pilot must be under manual control for this to work.
 *
 * @usage p:goto( v ) -- Goes to v precisely and braking
 * @usage p:goto( v, true, true ) -- Same as p:goto( v )
 * @usage p:goto( v, false ) -- Goes to v without braking compensating velocity
 * @usage p:goto( v, false, false ) -- Really rough approximation of going to v without braking
 *
 *    @luatparam Pilot p Pilot to tell to go to a position.
 *    @luatparam Vec2 v Vector target for the pilot.
 *    @luatparam[opt=1] boolean brake If true (or nil) brakes the pilot near target position,
 *              otherwise pops the task when it is about to brake.
 *    @luatparam[opt=1] boolean compensate If true (or nil) compensates for velocity, otherwise it
 *              doesn't. It only affects if brake is not set.
 * @luasee control
 * @luafunc goto( p, v, brake, compensate )
 */
static int pilotL_goto( lua_State *L )
{
   Pilot *p;
   Task *t;
   Vector2d *vec;
   int brake, compensate;
   const char *tsk;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   vec = luaL_checkvector(L,2);
   if (lua_gettop(L) > 2)
      brake = lua_toboolean(L,3);
   else
      brake = 1;
   if (lua_gettop(L) > 3)
      compensate = lua_toboolean(L,4);
   else
      compensate = 1;


   /* Set the task. */
   if (brake) {
      tsk = "__goto_precise";
   }
   else {
      if (compensate)
         tsk = "__goto_nobrake";
      else
         tsk = "__goto_nobrake_raw";
   }
   t        = pilotL_newtask( L, p, tsk );
   t->dtype = TASKDATA_VEC2;
   t->dat.vec = *vec;

   return 0;
}


/**
 * @brief Makes the pilot face a target.
 *
 * @usage p:face( enemy_pilot ) -- Face enemy pilot
 * @usage p:face( vec2.new( 0, 0 ) ) -- Face origin
 * @usage p:face( enemy_pilot, true ) -- Task lasts until the enemy pilot is faced
 *
 *    @luatparam Pilot p Pilot to add task to.
 *    @luatparam Vec2|Pilot target Target to face.
 *    @luatparam[opt=false] boolean towards Makes the task end when the target is faced (otherwise it's an enduring state).
 * @luafunc face( p, target, towards )
 */
static int pilotL_face( lua_State *L )
{
   Pilot *p, *pt;
   Vector2d *vec;
   Task *t;
   int towards;

   /* Get parameters. */
   pt = NULL;
   vec = NULL;
   p  = luaL_validpilot(L,1);
   if (lua_ispilot(L,2))
      pt = luaL_validpilot(L,2);
   else
      vec = luaL_checkvector(L,2);
   if (lua_gettop(L) > 2)
      towards = lua_toboolean(L,3);
   else
      towards = 0;

   /* Set the task. */
   if (towards)
      t     = pilotL_newtask( L, p, "__face_towards" );
   else
      t     = pilotL_newtask( L, p, "__face" );
   if (pt != NULL) {
      t->dtype = TASKDATA_PILOT;
      t->dat.num = pt->id;
   }
   else {
      t->dtype = TASKDATA_VEC2;
      t->dat.vec = *vec;
   }

   return 0;
}


/**
 * @brief Makes the pilot brake.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to brake.
 * @luasee control
 * @luafunc brake( p )
 */
static int pilotL_brake( lua_State *L )
{
   Pilot *p;

   /* Get parameters. */
   p = luaL_validpilot(L,1);

   /* Set the task. */
   pilotL_newtask( L, p, "brake" );

   return 0;
}


/**
 * @brief Makes the pilot follow another pilot.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to follow another pilot.
 *    @luatparam Pilot pt Target pilot to follow.
 *    @luatparam[opt=false] boolean accurate If true, use a PD controller which
 *              parameters can be defined using the pilot's memory.
 * @luasee control
 * @luasee memory
 * @luafunc follow( p, pt, accurate )
 */
static int pilotL_follow( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;
   int accurate;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   pt = luaL_validpilot(L,2);

   if (lua_gettop(L) > 2)
      accurate = lua_toboolean(L,3);
   else
      accurate = 0;

   /* Set the task. */
   if (accurate == 0)
      t = pilotL_newtask( L, p, "follow" );
   else
      t = pilotL_newtask( L, p, "follow_accurate" );

   t->dtype = TASKDATA_PILOT;
   t->dat.num = pt->id;

   return 0;
}


/**
 * @brief Makes the pilot attack another pilot.
 *
 * Pilot must be under manual control for this to work.
 *
 * @usage p:attack( another_pilot ) -- Attack another pilot
 * @usage p:attack() -- Attack nearest pilot.
 *
 *    @luatparam Pilot p Pilot to tell to attack another pilot.
 *    @luatparam[opt] Pilot pt Target pilot to attack (or nil to attack nearest enemy).
 * @luasee control
 * @luafunc attack( p, pt )
 */
static int pilotL_attack( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;
   unsigned int pid;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_gettop(L) == 1) {
      pid = pilot_getNearestEnemy( p );
      if (pid == 0) /* No enemy found. */
         return 0;
   }
   else {
      pt  = luaL_validpilot(L,2);
      pid = pt->id;
   }

   /* Set the task. */
   t        = pilotL_newtask( L, p, "attack" );
   t->dtype = TASKDATA_PILOT;
   t->dat.num = pid;

   return 0;
}


/**
 * @brief Makes the pilot runaway from another pilot.
 *
 * By default the pilot tries to jump when running away.
 *
 * @usage p:runaway( p_enemy ) -- Run away from p_enemy
 * @usage p:runaway( p_enemy, true ) -- Run away from p_enemy but do not jump
 *    @luatparam Pilot p Pilot to tell to runaway from another pilot.
 *    @luatparam Pilot tp Target pilot to runaway from.
 *    @luatparam[opt=false] boolean nojump Whether or not the pilot should try to jump when running away.
 * @luasee control
 * @luafunc runaway( p, tp, nojump )
 */
static int pilotL_runaway( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;
   int nojump;

   /* Get parameters. */
   p      = luaL_validpilot(L,1);
   pt     = luaL_validpilot(L,2);
   nojump = lua_toboolean(L,3);

   /* Set the task. */
   t        = pilotL_newtask( L, p, (nojump) ? "__runaway_nojump" : "__runaway" );
   t->dtype = TASKDATA_PILOT;
   t->dat.num = pt->id;

   return 0;
}


/**
 * @brief Tells the pilot to hyperspace.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to hyperspace.
 *    @luatparam[opt] System sys Optional System to jump to, uses random if nil.
 *    @luatparam[opt] boolean shoot Whether or not to shoot at targets while running away with turrets.
 * @luasee control
 * @luafunc hyperspace( p, sys, shoot )
 */
static int pilotL_hyperspace( lua_State *L )
{
   Pilot *p;
   Task *t;
   StarSystem *ss;
   int i;
   JumpPoint *jp;
   double a, rad;
   int shoot;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if ((lua_gettop(L) > 1) && !lua_isnil(L,2))
      ss = system_getIndex( luaL_checksystem( L, 2 ) );
   else
      ss = NULL;
   shoot = lua_toboolean(L,3);

   /* Set the task. */
   if (shoot)
      t = pilotL_newtask( L, p, "__hyperspace_shoot" );
   else
      t = pilotL_newtask( L, p, "__hyperspace" );
   if (ss != NULL) {
      /* Find the jump. */
      for (i=0; i < cur_system->njumps; i++) {
         jp = &cur_system->jumps[i];
         if (jp->target != ss)
            continue;
         /* Found target. */

         if (jp_isFlag( jp, JP_EXITONLY )) {
            NLUA_ERROR( L, "Pilot '%s' can't jump out exit only jump '%s'", p->name, ss->name );
            return 0;
         }
         break;
      }
      if (i >= cur_system->njumps) {
         NLUA_ERROR( L, "System '%s' is not adjacent to current system '%s'", ss->name, cur_system->name );
         return 0;
      }

      /* Set nav target. */
      p->nav_hyperspace = i;

      /* Copy vector. */
      t->dtype = TASKDATA_VEC2;
      t->dat.vec = jp->pos;

      /* Introduce some error. */
      a     = RNGF() * M_PI * 2.;
      rad   = RNGF() * 0.5 * jp->radius;
      vect_cadd( &t->dat.vec, rad*cos(a), rad*sin(a) );
   }

   return 0;
}


/**
 * @brief Tells the pilot to land
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to land.
 *    @luatparam[opt] Planet planet Planet to land on, uses random if nil.
 * @luasee control
 * @luafunc land( p, planet )
 */
static int pilotL_land( lua_State *L )
{
   Pilot *p;
   Task *t;
   Planet *pnt;
   int i;
   double a, r;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if ((lua_gettop(L) > 0) && (!lua_isnil(L,2)))
      pnt = luaL_validplanet( L, 2 );
   else
      pnt = NULL;

   /* Set the task. */
   t = pilotL_newtask( L, p, "__land" );
   if (pnt != NULL) {
      /* Find the jump. */
      for (i=0; i < cur_system->nplanets; i++) {
         if (cur_system->planets[i] == pnt) {
            break;
         }
      }
      if (i >= cur_system->nplanets) {
         NLUA_ERROR( L, "Planet '%s' not found in system '%s'", pnt->name, cur_system->name );
         return 0;
      }

      /* Copy vector. */
      p->nav_planet = i;
      t->dtype = TASKDATA_VEC2;
      t->dat.vec = pnt->pos;
      if (p->id == PLAYER_ID)
         gui_setNav();

      /* Introduce some error. */
      a = RNGF() * 2. * M_PI;
      r = RNGF() * pnt->radius;
      vect_cadd( &t->dat.vec, r*cos(a), r*sin(a) );
   }

   return 0;
}


/**
 * @brief Marks the pilot as hailing the player.
 *
 * Automatically deactivated when pilot is hailed.
 *
 * @usage p:hailPlayer() -- Player will be informed he's being hailed and pilot will have an icon
 *    @luatparam Pilot p Pilot to hail the player.
 *    @luatparam[opt=true] boolean enable If true hails the pilot, if false disables the hailing.
 * @luafunc hailPlayer( p, enable )
 */
static int pilotL_hailPlayer( lua_State *L )
{
   Pilot *p;
   int enable;
   char c;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if (lua_gettop(L) > 1)
      enable = lua_toboolean(L,3);
   else
      enable = 1;


   /* Set the flag. */
   if (enable) {
      /* Send message. */
      c = pilot_getFactionColourChar( p );
      player_message( "\e%c%s\e0 is hailing you.", c, p->name );

      /* Set flag. */
      pilot_setFlag( p, PILOT_HAILING );
      player_hailStart();
   }
   else
      pilot_rmFlag( p, PILOT_HAILING );

   return 0;
}


/**
 * @brief Clears the pilot's hooks.
 *
 * Clears all the hooks set on the pilot.
 *
 * @usage p:hookClear()
 *    @luatparam Pilot p Pilot to clear hooks.
 * @luafunc hookClear( p )
 */
static int pilotL_hookClear( lua_State *L )
{
   Pilot *p;

   p = luaL_validpilot(L,1);
   pilot_clearHooks( p );

   return 0;
}


