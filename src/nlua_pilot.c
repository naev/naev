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

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_planet.h"
#include "log.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "space.h"
#include "ai.h"
#include "ai_extra.h"
#include "nlua_col.h"
#include "weapon.h"
#include "gui.h"
#include "camera.h"


/*
 * From pilot.c
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * Prototypes.
 */
static Task *pilotL_newtask( lua_State *L, Pilot* p, const char *task );


/* Pilot metatable methods. */
static int pilotL_getPlayer( lua_State *L );
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
static int pilotL_rename( lua_State *L );
static int pilotL_position( lua_State *L );
static int pilotL_velocity( lua_State *L );
static int pilotL_dir( lua_State *L );
static int pilotL_faction( lua_State *L );
static int pilotL_setPosition( lua_State *L );
static int pilotL_setVelocity( lua_State *L );
static int pilotL_setDir( lua_State *L );
static int pilotL_broadcast( lua_State *L );
static int pilotL_comm( lua_State *L );
static int pilotL_setFaction( lua_State *L );
static int pilotL_setHostile( lua_State *L );
static int pilotL_setFriendly( lua_State *L );
static int pilotL_setInvincible( lua_State *L );
static int pilotL_setInvisible( lua_State *L );
static int pilotL_setVisplayer( lua_State *L );
static int pilotL_setVisible( lua_State *L );
static int pilotL_setHilight( lua_State *L );
static int pilotL_getColour( lua_State *L );
static int pilotL_getHostile( lua_State *L );
static int pilotL_disable( lua_State *L );
static int pilotL_addOutfit( lua_State *L );
static int pilotL_rmOutfit( lua_State *L );
static int pilotL_setFuel( lua_State *L );
static int pilotL_changeAI( lua_State *L );
static int pilotL_setHealth( lua_State *L );
static int pilotL_setEnergy( lua_State *L );
static int pilotL_setNoboard( lua_State *L );
static int pilotL_setNodisable( lua_State *L );
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
   { "player", pilotL_getPlayer },
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
   { "rename", pilotL_rename },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "dir", pilotL_dir },
   { "faction", pilotL_faction },
   { "health", pilotL_getHealth },
   { "energy", pilotL_getEnergy },
   { "lockon", pilotL_getLockon },
   { "stats", pilotL_getStats },
   { "colour", pilotL_getColour },
   { "hostile", pilotL_getHostile },
   /* System. */
   { "clear", pilotL_clear },
   { "toggleSpawn", pilotL_toggleSpawn },
   /* Modify. */
   { "changeAI", pilotL_changeAI },
   { "setHealth", pilotL_setHealth },
   { "setEnergy", pilotL_setEnergy },
   { "setNoboard", pilotL_setNoboard },
   { "setNodisable", pilotL_setNodisable },
   { "setPos", pilotL_setPosition },
   { "setVel", pilotL_setVelocity },
   { "setDir", pilotL_setDir },
   { "setFaction", pilotL_setFaction },
   { "setHostile", pilotL_setHostile },
   { "setFriendly", pilotL_setFriendly },
   { "setInvincible", pilotL_setInvincible },
   { "setInvisible", pilotL_setInvisible },
   { "setVisplayer", pilotL_setVisplayer },
   { "setVisible", pilotL_setVisible },
   { "setHilight", pilotL_setHilight },
   { "disable", pilotL_disable },
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
   { "player", pilotL_getPlayer },
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
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "dir", pilotL_dir },
   { "faction", pilotL_faction },
   { "health", pilotL_getHealth },
   { "energy", pilotL_getEnergy },
   { "lockon", pilotL_getLockon },
   { "stats", pilotL_getStats },
   { "colour", pilotL_getColour },
   { "hostile", pilotL_getHostile },
   /* Ship. */
   { "ship", pilotL_ship },
   { "cargoFree", pilotL_cargoFree },
   { "cargoHas", pilotL_cargoHas },
   { "cargoList", pilotL_cargoList },
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
LuaPilot* lua_topilot( lua_State *L, int ind )
{
   return (LuaPilot*) lua_touserdata(L,ind);
}
/**
 * @brief Gets pilot at index or raises error if there is no pilot at index.
 *
 *    @param L Lua state to get pilot from.
 *    @param ind Index position to find pilot.
 *    @return Pilot found at the index in the state.
 */
LuaPilot* luaL_checkpilot( lua_State *L, int ind )
{
   if (lua_ispilot(L,ind))
      return lua_topilot(L,ind);
   luaL_typerror(L, ind, PILOT_METATABLE);
   return NULL;
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
   LuaPilot *lp;
   Pilot *p;

   /* Get the pilot. */
   lp = luaL_checkpilot(L,ind);
   p  = pilot_get(lp->pilot);
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
 * @brief Gets the player's pilot.
 *
 * @usage player = pilot.player()
 *
 *    @luareturn Pilot pointing to the player.
 * @luafunc player()
 */
static int pilotL_getPlayer( lua_State *L )
{
   LuaPilot lp;

   if (player.p == NULL) {
      lua_pushnil(L);
      return 1;
   }

   lp.pilot = player.p->id;
   lua_pushpilot(L,lp);
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
 *    @luaparam fleetname Name of the fleet to add.
 *    @luaparam ai If set will override the standard fleet AI.  nil means use default.
 *    @luaparam param Position to create pilot at, if it's a system it'll try to jump in from that system, if it's
 *              a planet it'll try to take off from it.
 *    @luareturn Table populated with all the pilots created.  The keys are ordered numbers.
 * @luafunc add( fleetname, ai, param )
 */
static int pilotL_addFleet( lua_State *L )
{
   Fleet *flt;
   const char *fltname, *fltai;
   int i, first;
   unsigned int p;
   double a, r;
   Vector2d vv,vp, vn;
   FleetPilot *plt;
   LuaPilot lp;
   LuaVector *lv;
   LuaSystem *ls;
   StarSystem *ss;
   Planet *planet;
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

   /* Parse first argument - Fleet Name */
   fltname = luaL_checkstring(L,1);

   /* pull the fleet */
   flt = fleet_get( fltname );
   if (flt == NULL) {
      NLUA_ERROR(L,"Fleet '%s' doesn't exist.", fltname);
      return 0;
   }

   /* Parse second argument - Fleet AI Override */
   if ((lua_gettop(L) < 2) || lua_isnil(L,2))
      fltai = NULL;
   else
      fltai = luaL_checkstring(L,2);

   /* Handle third argument. */
   if (lua_isvector(L,3)) {
      lv = lua_tovector(L,3);
      vectcpy( &vp, &lv->vec );
      a = RNGF() * 2.*M_PI;
      vectnull( &vv );
   }
   else if (lua_issystem(L,3)) {
      ls    = lua_tosystem(L,3);
      ss    = system_getIndex( ls->id );
      for (i=0; i<cur_system->njumps; i++) {
         if (cur_system->jumps[i].target == ss) {
            jump = i;
            break;
         }
      }
      if (jump < 0) {
         WARN("Fleet '%s' jumping in from non-adjacent system '%s' to '%s'.",
               fltname, ss->name, cur_system->name );
         jump = RNG_SANE(0,cur_system->njumps-1);
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
                  !areEnemies(flt->faction,cur_system->planets[i]->faction))
               ind[ nind++ ] = i;
      }

      /* Build jumpable jump table. */
      jumpind  = NULL;
      njumpind = 0;
      if (cur_system->njumps > 0) {
         jumpind = malloc( sizeof(int) * cur_system->njumps );
         for (i=0; i<cur_system->njumps; i++)
            if (!ignore_rules && (system_getPresence( cur_system->jumps[i].target, flt->faction ) > 0))
               jumpind[ njumpind++ ] = i;
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
         if ((ind == NULL) || ((RNGF() <= chance) && (jumpind != NULL))) {
            jump = jumpind[ RNG_SANE(0,njumpind-1) ];
         }
         /* Random take off. */
         else if (ind !=NULL) {
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

         /* Free memory allocated. */
         if (ind != NULL )
            free( ind );
         if (jumpind != NULL)
            free( jumpind );
      }
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

   /* now we start adding pilots and toss ids into the table we return */
   first = 1;
   lua_newtable(L);
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
      lp.pilot = p;
      lua_pushpilot(L,lp); /* value = LuaPilot */
      lua_rawset(L,-3); /* store the value in the table */
   }
   return 1;
}


/**
 * @brief Removes a pilot without explosions or anything.
 *
 * @usage p:rm() -- pilot will be destroyed
 *
 *    @luaparam p Pilot to remove.
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
 * If player jumps the spawn is enabled again automatically.
 *
 * @usage pilot.togglespawn( false )
 *
 *    @luaparam enable true enables spawn, false disables it.
 *    @luareturn The current spawn state.
 * @luafunc toggleSpawn( enable )
 */
static int pilotL_toggleSpawn( lua_State *L )
{
   /* Setting it directly. */
   if ((lua_gettop(L) > 0) && lua_isboolean(L,1))
      space_spawn = lua_toboolean(L,1);
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
 *    @luaparam factions If f is a table of factions, it will only get pilots matching those factions.  Otherwise it gets all the pilots.
 *    @luaparam disabled Whether or not to get disabled ships (default is off if parameter is ommitted).
 *    @luareturn A table containing the pilots.
 * @luafunc get( factions, disabled )
 */
static int pilotL_getPilots( lua_State *L )
{
   int i, j, k, d;
   int *factions;
   int nfactions;
   LuaFaction *f;
   LuaPilot p;

   /* Whether or not to get disabled. */
   d = lua_toboolean(L,2);

   /* Check for belonging to faction. */
   if (lua_istable(L,1)) {
      /* Get table length and preallocate. */
      nfactions = (int) lua_objlen(L,1);
      factions = malloc( sizeof(int) * nfactions );
      /* Load up the table. */
      lua_pushnil(L);
      i = 0;
      while (lua_next(L, -2) != 0) {
         f = lua_tofaction(L, -1);
         factions[i++] = f->f;
         lua_pop(L,1);
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
               p.pilot = pilot_stack[i]->id;
               lua_pushpilot(L, p); /* value */
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
            p.pilot = pilot_stack[i]->id;
            lua_pushpilot(L, p); /* value */
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
 *    @luaparam p Pilot to compare.
 *    @luaparam comp Pilot to compare against.
 *    @luareturn true if they are the same.
 * @luafunc __eq( p, comp )
 */
static int pilotL_eq( lua_State *L )
{
   LuaPilot *p1, *p2;

   /* Get parameters. */
   p1 = luaL_checkpilot(L,1);
   p2 = luaL_checkpilot(L,2);

   /* Push result. */
   lua_pushboolean(L, p1->pilot == p2->pilot);
   return 1;
}

/**
 * @brief Gets the pilot's current name.
 *
 * @usage name = p:name()
 *
 *    @luaparam p Pilot to get the name of.
 *    @luareturn The current name of the pilot.
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
 *    @luaparam p Pilot to get the ID of.
 *    @luareturn The ID of the curent pilot.
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
 *    @luaparam p Pilot to check to see if is still exists.
 *    @luareturn true if pilot is still exists.
 * @luafunc exists( p )
 */
static int pilotL_exists( lua_State *L )
{
   LuaPilot *lp;
   Pilot *p;
   int exists;

   /* Parse parameters. */
   lp = luaL_checkpilot(L,1);
   p  = pilot_get( lp->pilot );

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
 *    @luaparam p Pilot to get target of.
 *    @luareturn nil if no target is selected, otherwise the target of the target.
 * @luafunc target( p )
 */
static int pilotL_target( lua_State *L )
{
   Pilot *p;
   LuaPilot lp;
   p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;
   lp.pilot = p->target;
   /* Must be targetted. */
   if (p->target == p->id)
      return 0;
   /* Must be valid. */
   if (pilot_get(p->target) == NULL)
      return 0;
   /* Push target. */
   lua_pushpilot(L, lp);
   return 1;
}


/**
 * @brief Checks to see if pilot is in range of pilot.
 *
 * @usage detected, fuzzy = p:inrange( target )
 *
 *    @luaparam p Pilot to see if another pilot is in range.
 *    @luareturn Checks to see if the target is detected and if it's scanned.
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
   if (ret == 1) {
      lua_pushboolean(L,1);
      lua_pushboolean(L,0);
   }
   else if (ret == 0) {
      lua_pushboolean(L,0);
      lua_pushboolean(L,0);
   }
   else {
      lua_pushboolean(L,1);
      lua_pushboolean(L,1);
   }
   return 2;
}


/**
 * @brief Gets the nav target of the pilot.
 *
 * @usage planet, hyperspace = p:nav()
 *
 *    @luaparam p Pilot to get nav info of.
 *    @luareturn The planet target followed by the hyperspace target or nil if not targetted.
 * @luafunc nav( p )
 */
static int pilotL_nav( lua_State *L )
{
   LuaPlanet lp;
   LuaSystem ls;
   Pilot *p;

   /* Get pilot. */
   p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;

   /* Get planet target. */
   if (p->nav_planet < 0)
      lua_pushnil(L);
   else {
      lp.id = cur_system->planets[ p->nav_planet ]->id;
      lua_pushplanet( L, lp  );
   }

   /* Get hyperspace target. */
   if (p->nav_hyperspace < 0)
      lua_pushnil(L);
   else {
      ls.id = cur_system->jumps[ p->nav_hyperspace ].targetid;
      lua_pushsystem( L, ls );
   }

   return 2;
}


/**
 * @brief Gets the weapset weapon of the pilot.
 *
 * The weapon sets have the following structure: <br />
 * <ul>
 *  <li> name: name of the set <br />
 *  <li> cooldown: [0:1] value indicating if ready to shoot (1 is ready) <br />
 *  <li> ammo: Name of the ammo or nil if not applicable <br />
 *  <li> left: Absolute ammo left or nil if not applicable <br />
 *  <li> left_p: Relative ammo left [0:1] or nil if not applicable <br />
 *  <li> level: Level of the weapon (1 is primary, 2 is secondary). <br />
 *  <li> temp: Temperature of the weapon. <br />
 *  <li> type: Type of the weapon. <br />
 *  <li> dtype: Damage type of the weapon. <br />
 *  <li> track: Tracking level of the weapon. <br />
 * </ul>
 *
 * An example would be:
 * <pre><code>
 * ws_name, ws = p:weapset( true )
 * print( "Weapnset Name: " .. ws_name )
 * for _,w in ipairs(ws) do
 *    print( "Name: " .. w.name )
 *    print( "Cooldown: " .. tostring(cooldown) )
 *    print( "Level: " .. tostring(level) )
 * end
 * </code></pre>
 *
 * @usage set_name, slots = p:weapset( true ) -- Gets info for all active weapons
 * @usage set_name, slots = p:weapset() -- Get info about the current set
 * @usage set_name, slots = p:weapset( 5 ) -- Get info about the set number 5
 *
 *    @luaparam p Pilot to get weapset weapon of.
 *    @luaparam id ID of the set to get information of. Defaults to currently active set.
 *    @luareturn The name of the set and a table with each slot's information.
 * @luafunc weapset( p, id)
 */
static int pilotL_weapset( lua_State *L )
{
   Pilot *p, *target;
   int i, j, k, n;
   PilotWeaponSetOutfit *po_list;
   PilotOutfitSlot *slot;
   Outfit *ammo, *o;
   double delay;
   int id, all, level, level_match;

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
         if (all) {
            slot  = p->outfits[i];
            o     = slot->outfit;

            /* Must be valid weapon. */
            if ((o == NULL) || !(outfit_isBolt(o) || outfit_isBeam(o) ||
                  outfit_isLauncher(o) || outfit_isFighterBay(o)))
               continue;

            level = slot->level;
         }
         else {
            slot  = po_list[i].slot;
            level = po_list[i].level;
         }

         /* Must match level. */
         if (level != level_match)
            continue;

         /* Set up for creation. */
         lua_pushnumber(L,++k);
         lua_newtable(L);

         /* Name. */
         lua_pushstring(L,"name");
         lua_pushstring(L,slot->outfit->name);
         lua_rawset(L,-3);

         /* Set cooldown. */
         lua_pushstring(L,"cooldown");
         delay = outfit_delay(slot->outfit);
         if (delay > 0.)
            lua_pushnumber( L, CLAMP( 0., 1., 1. - slot->timer / delay ) );
         else
            lua_pushnumber( L, 1. );
         lua_rawset(L,-3);

         /* Ammo name. */
         ammo = outfit_ammo(slot->outfit);
         if (ammo != NULL) {
            lua_pushstring(L,"ammo");
            lua_pushstring(L,ammo->name);
            lua_rawset(L,-3);
         }

         /* Ammo quantity absolute. */
         if ((outfit_isLauncher(slot->outfit) ||
                  outfit_isFighterBay(slot->outfit)) &&
               (slot->u.ammo.outfit != NULL)) {
            lua_pushstring(L,"left");
            lua_pushnumber( L, slot->u.ammo.quantity );
            lua_rawset(L,-3);
         }

         /* Ammo quantity relative. */
         if ((outfit_isLauncher(slot->outfit) ||
                  outfit_isFighterBay(slot->outfit)) &&
               (slot->u.ammo.outfit != NULL)) {
            lua_pushstring(L,"left_p");
            lua_pushnumber( L, (double)slot->u.ammo.quantity / (double)outfit_amount(slot->outfit) );
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
         lua_pushstring(L, "dtype");
         if (outfit_isLauncher(slot->outfit) && (slot->u.ammo.outfit != NULL))
            lua_pushstring(L, outfit_damageTypeToStr( outfit_damageType(slot->u.ammo.outfit) ));
         else
            lua_pushstring(L, outfit_damageTypeToStr( outfit_damageType(slot->outfit) ));
         lua_rawset(L,-3);

         /* Track. */
         if ((target != NULL) && (slot->outfit->type == OUTFIT_TYPE_TURRET_BOLT)) {
            lua_pushstring(L, "track");
            lua_pushnumber(L, pilot_ewWeaponTrack( p, target, slot->outfit->u.blt.track ));
            lua_rawset(L,-3);
         }

         /* Set table in table. */
         lua_rawset(L,-3);
      }
   }
   return 2;
}


/**
 * @brief Changes the pilot's name.
 *
 * @usage p:rename( "Black Beard" )
 *
 *    @luaparam p Pilot to change name of.
 *    @luaparam name Name to change to.
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
 *    @luaparam p Pilot to get the position of.
 *    @luareturn The pilot's current position as a vec2.
 * @luafunc pos( p )
 */
static int pilotL_position( lua_State *L )
{
   Pilot *p;
   LuaVector v;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push position. */
   vectcpy( &v.vec, &p->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Gets the pilot's velocity.
 *
 * @usage vel = p:vel()
 *
 *    @luaparam p Pilot to get the velocity of.
 *    @luareturn The pilot's current velocity as a vec2.
 * @luafunc vel( p )
 */
static int pilotL_velocity( lua_State *L )
{
   Pilot *p;
   LuaVector v;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push velocity. */
   vectcpy( &v.vec, &p->solid->vel );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Gets the pilot's direction;
 *
 * @usage d = p:dir()
 *
 *    @luaparam p Pilot to get the direction of.
 *    @luareturn The pilot's current direction as a number (in radians).
 * @luafunc dir( p )
 */
static int pilotL_dir( lua_State *L )
{
   Pilot *p;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push direction. */
   lua_pushnumber( L, p->solid->dir );
   return 1;
}

/**
 * @brief Gets the pilot's faction.
 *
 * @usage f = p:faction()
 *
 *    @luaparam p Pilot to get the faction of.
 *    @luareturn The faction of the pilot.
 * @luafunc faction( p )
 */
static int pilotL_faction( lua_State *L )
{
   Pilot *p;
   LuaFaction f;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);

   /* Push faction. */
   f.f   = p->faction;
   lua_pushfaction(L,f);
   return 1;
}

/**
 * @brief Sets the pilot's position.
 *
 * @usage p:setPos( vec2.new( 300, 200 ) )
 *
 *    @luaparam p Pilot to set the position of.
 *    @luaparam pos Position to set.
 * @luafunc setPos( p, pos )
 */
static int pilotL_setPosition( lua_State *L )
{
   Pilot *p;
   LuaVector *v;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   v     = luaL_checkvector(L,2);

   /* Warp pilot to new position. */
   vectcpy( &p->solid->pos, &v->vec );

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
 *    @luaparam p Pilot to set the velocity of.
 *    @luaparam vel Velocity to set.
 * @luafunc setVel( p, vel )
 */
static int pilotL_setVelocity( lua_State *L )
{
   Pilot *p;
   LuaVector *v;

   /* Parse parameters */
   p     = luaL_validpilot(L,1);
   v     = luaL_checkvector(L,2);

   /* Warp pilot to new position. */
   vectcpy( &p->solid->vel, &v->vec );
   return 0;
}

/**
 * @brief Sets the pilot's direction.
 *
 * @note Right is 0, top is 90, left is 180, bottom is 270.
 *
 * @usage p:setDir( 180. )
 *
 *    @luaparam p Pilot to set the direction of.
 *    @luaparam dir Direction to set.
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
 *    @luaparam p Pilot to broadcast the message.
 *    @luaparam msg Message to broadcast.
 *    @luaparam ignore_int Whether or not it should ignore interference.
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
 *    @luaparam p Pilot to message the player.
 *    @ulaparam target Target to send message to.
 *    @luaparam msg Message to send.
 * @luafunc comm( p, target, msg )
 */
static int pilotL_comm( lua_State *L )
{
   Pilot *p, *t;
   LuaPilot *target;
   const char *msg;
   int ignore_int;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isstring(L,2)) {
      target = NULL;
      msg   = luaL_checkstring(L,2);
      ignore_int = lua_toboolean(L,3);
   }
   else {
      target = luaL_checkpilot(L,2);
      msg   = luaL_checkstring(L,3);
      ignore_int = lua_toboolean(L,4);
   }

   /* Check to see if pilot is valid. */
   if (target == NULL)
      t = player.p;
   else {
      t = pilot_get(target->pilot);
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
 *    @luaparam p Pilot to change faction of.
 *    @luaparam faction Faction to set by name or faction.
 * @luafunc setFaction( p, faction )
 */
static int pilotL_setFaction( lua_State *L )
{
   Pilot *p;
   LuaFaction *f;
   int fid;
   const char *faction;

   /* Parse parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isstring(L,2)) {
      faction = lua_tostring(L,2);
      fid = faction_get(faction);
   }
   else if (lua_isfaction(L,2)) {
      f = lua_tofaction(L,2);
      fid = f->f;
   }
   else NLUA_INVALID_PARAMETER(L);

   /* Set the new faction. */
   p->faction = fid;

   return 0;
}


/**
 * @brief Sets the pilot as hostile to player.
 *
 * @usage p:setHostile()
 *
 *    @luaparam p Pilot to set as hostile.
 * @luafunc setHostile( p )
 */
static int pilotL_setHostile( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Set as hostile. */
   pilot_rmFlag(p, PILOT_FRIENDLY);
   pilot_setHostile(p);

   return 0;
}


/**
 * @brief Sets the pilot as friendly to player.
 *
 * @usage p:setFriendly()
 *
 *    @luaparam p Pilot to set as friendly.
 * @luafunc setFriendly( p )
 */
static int pilotL_setFriendly( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Remove hostile and mark as friendly. */
   pilot_setFriendly(p);

   return 0;
}


/**
 * @brief Sets the pilot's invincibility status.
 *
 * @usage p:setInvincible() -- p can not be hit anymore
 * @usage p:setInvincible(true) -- p can not be hit anymore
 * @usage p:setInvincible(false) -- p can be hit again
 *
 *    @luaparam p Pilot to set invincibility status of.
 *    @luaparam state State to set invincibility, if omitted defaults to true.
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
 * @brief Sets the pilot's invisibility status.
 *
 * An invisible pilot is neither updated nor drawn. It stays frozen in time
 *  until the invisibility is lifted.
 *
 * @usage p:setInvisible() -- p will disappear
 * @usage p:setInvisible(true) -- p will disappear
 * @usage p:setInvisible(false) -- p will appear again
 *
 *    @luaparam p Pilot to set invisibility status of.
 *    @luaparam state State to set invisibility, if omitted defaults to true.
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
 *    @luaparam p Pilot to set player visibility status of.
 *    @luaparam state State to set player visibility, defaults to true.
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
 *    @luaparam p Pilot to set visibility status of.
 *    @luaparam state State to set visibility, defaults to true.
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
 * This makes the pilot stand out in the map overlay and radar to increase noticeability.
 *
 * @usage p:setVisplayer( true )
 *
 *    @luaparam p Pilot to set hilight status of.
 *    @luaparam state State to set hilight, defaults to true.
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
 * @brief Disables a pilot.
 *
 * @usage p:disable()
 *
 *    @luaparam p Pilot to disable.
 * @luafunc disable( p )
 */
static int pilotL_disable( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Disable the pilot. */
   p->shield = 0.;
   p->armour = PILOT_DISABLED_ARMOR * p->ship->armour;
   pilot_setFlag( p, PILOT_DISABLED );

   return 0;
}


/**
 * @brief Adds an outfit to a pilot.
 *
 * @usage added = p:addOutfit( "Laser Cannon", 5 ) -- Adds 5 laser cannons to p
 *
 *    @luaparam p Pilot to add outfit to.
 *    @luaparam outfit Name of the outfit to add.
 *    @lusparam q Amount of the outfit to add (defaults to 1).
 *    @luareturn The number of outfits added.
 * @luafunc addOutfit( p, outfit, q )
 */
static int pilotL_addOutfit( lua_State *L )
{
   int i;
   Pilot *p;
   const char *outfit;
   Outfit *o;
   int ret;
   int q, added;

   /* Get parameters. */
   p      = luaL_validpilot(L,1);
   outfit = luaL_checkstring(L,2);
   q      = 1;
   if (lua_gettop(L) > 2)
      q = luaL_checkint(L,3);

   /* Get the outfit. */
   o = outfit_get( outfit );
   if (o == NULL)
      return 0;

   /* Add outfit. */
   added = 0;
   for (i=0; i<p->noutfits; i++) {
      /* Must still have to add outfit. */
      if (q <= 0)
         break;

      /* Must not have outfit already. */
      if (p->outfits[i]->outfit != NULL)
         continue;

      /* Must fit slot. */
      if (!outfit_fitsSlot( o, &p->outfits[i]->slot ))
         continue;

      /* Test if can add outfit. */
      ret = pilot_addOutfitTest( p, o, p->outfits[i], 0 );
      if (ret)
         break;

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

   lua_pushnumber(L,added);
   return 1;
}


/**
 * @brief Removes an outfit from a pilot.
 *
 * "all" will remove all outfits.
 *
 * @usage p:rmOutfit( "all" ) -- Leaves the pilot naked.
 * @usage p:rmOutfit( "Neutron Disruptor" ) -- Removes a neutron disruptor.
 * @usage p:rmOutfit( "Neutron Disruptor", 2 ) -- Removes two neutron disruptor.
 *
 *    @luparam p Pilot to remove outfit from.
 *    @luaparam outfit Name of the outfit to remove.
 * @luafunc rmOutfit( p, outfit )
 */
static int pilotL_rmOutfit( lua_State *L )
{
   int i;
   Pilot *p;
   const char *outfit;
   Outfit *o;
   int q;

   /* Get parameters. */
   p      = luaL_validpilot(L,1);
   outfit = luaL_checkstring(L,2);
   q      = 1;
   if (lua_gettop(L) > 2)
      q = luaL_checkint(L,3);

   /* If outfit is "all", we remove everything. */
   if (strcmp(outfit,"all")==0) {
      for (i=0; i<p->noutfits; i++) {
         pilot_rmOutfitRaw( p, p->outfits[i] );
      }
      pilot_calcStats( p ); /* Recalculate stats. */
      return 0;
   }

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
   }
   return 0;
}


/**
 * @brief Sets the fuel of a pilot.
 *
 * @usage p:setFuel( true ) -- Sets fuel to max
 *
 *    @luaparam p Pilot to set fuel of.
 *    @luaparam f true sets fuel to max, false sets fuel to 0, a number sets
 *              fuel to that amount in units.
 *    @luareturn The amount of fuel the pilot has.
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
 * @brief Sets the health of a pilot.
 *
 * This recovers the pilot's disabled state, although he may become disabled afterwards.
 *
 * @usage p:setHealth( 100, 100 ) -- Sets pilot to full health
 * @usage p:setHealth(  70,   0 ) -- Sets pilot to 70% armour
 *
 *    @luaparam p Pilot to set health of.
 *    @luaparam armour Value to set armour to, should be double from 0-100 (in percent).
 *    @luaparam shield Value to set shield to, should be double from 0-100 (in percent).
 * @luafunc setHealth( p, armour, shield )
 */
static int pilotL_setHealth( lua_State *L )
{
   Pilot *p;
   double a, s;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   a  = luaL_checknumber(L, 2);
   s  = luaL_checknumber(L, 3);
   a /= 100.;
   s /= 100.;

   /* Set health. */
   p->armour = a * p->armour_max;
   p->shield = s * p->shield_max;

   /* Undisable if was disabled. */
   pilot_rmFlag( p, PILOT_DISABLED );

   return 0;
}


/**
 * @brief Sets the energy of a pilot.
 *
 * @usage p:setEnergy( 100 ) -- Sets pilot to full energy.
 * @usage p:setEnergy(  70 ) -- Sets pilot to 70% energy.
 *
 *    @luaparam p Pilot to set energy of.
 *    @luaparam energy Value to set energy to, should be double from 0-100 (in percent).
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
 *    @luaparam p Pilot to set disable boarding.
 *    @luaparam noboard If true it disallows boarding of the pilot, otherwise
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

   /* See if should mark as boarded. */
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
 *    @luaparam p Pilot to set disable disabling.
 *    @luaparam disable If true it disallows disabled of the pilot, otherwise
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

   /* See if should mark as boarded. */
   if (disable)
      pilot_setFlag(p, PILOT_NODISABLE);
   else
      pilot_rmFlag(p, PILOT_NODISABLE);

   return 0;
}


/**
 * @brief Gets the pilot's health.
 *
 * @usage armour, shield, dis = p:health()
 *
 *    @luaparam p Pilot to get health of.
 *    @luareturn The armour and shield of the pilot in % [0:100], followed by a boolean indicating if piloti s disabled.
 * @luafunc health( p )
 */
static int pilotL_getHealth( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Return parameters. */
   lua_pushnumber(L, p->armour / p->armour_max * 100. );
   lua_pushnumber(L, p->shield / p->shield_max * 100. );
   lua_pushboolean(L, pilot_isDisabled(p));

   return 3;
}


/**
 * @brief Gets the pilot's energy.
 *
 * @usage energy = p:energy()
 *
 *    @luaparam p Pilot to get energy of.
 *    @luareturn The energy of the pilot in % [0:100].
 * @luafunc energy( p )
 */
static int pilotL_getEnergy( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Return parameter. */
   lua_pushnumber(L, p->energy / p->energy_max * 100. );

   return 1;
}


/**
 * @brief Gets the lockons on the pilot.
 *
 * @usage lockon = p:lockon()
 *
 *    @luaparam p Pilot to get lockons of.
 *    @luareturn The number of lockons on the pilot.
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
/**
 * @brief Gets stats of the pilot.
 *
 * Some of the stats are:<br />
 * <ul>
 *  <li> cpu <br />
 *  <li> cpu_max <br />
 *  <li> fuel <br />
 *  <li> thrust <br />
 *  <li> speed <br />
 *  <li> turn <br />
 *  <li> armour <br />
 *  <li> shield <br />
 *  <li> energy <br />
 *  <li> armour_regen <br />
 *  <li> shield_regen <br />
 *  <li> energy_regen <br />
 *  <li> jam_range <br />
 *  <li> jam_chance <br />
 *  <li> jump_delay <br />
 * </ul>
 *
 * @usage stats = p:stats() print(stats.armour)
 *
 *    @luaparam p Pilot to get stats of.
 *    @luareturn A table containing the stats of p.
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
   PUSH_DOUBLE( L, "cpu_max", p->cpu_max );
   PUSH_DOUBLE( L, "fuel", p->fuel );
   PUSH_DOUBLE( L, "mass", p->solid->mass );
   /* Movement. */
   PUSH_DOUBLE( L, "thrust", p->thrust );
   PUSH_DOUBLE( L, "speed", p->speed );
   PUSH_DOUBLE( L, "turn", p->turn );
   PUSH_DOUBLE( L, "speed_max", solid_maxspeed(p->solid, p->speed, p->thrust) );
   /* Health. */
   PUSH_DOUBLE( L, "armour", p->armour_max );
   PUSH_DOUBLE( L, "shield", p->shield_max );
   PUSH_DOUBLE( L, "energy", p->energy_max );
   PUSH_DOUBLE( L, "armour_regen", p->armour_regen );
   PUSH_DOUBLE( L, "shield_regen", p->shield_regen );
   PUSH_DOUBLE( L, "energy_regen", p->energy_regen );
   /* Jam. */
   PUSH_DOUBLE( L, "jam_range", p->jam_range );
   PUSH_DOUBLE( L, "jam_chance", p->jam_chance );
   /* Stats. */
   PUSH_DOUBLE( L, "jump_delay", pilot_hyperspaceDelay(p) );

   return 1;
}
#undef PUSH_DOUBLE


/**
 * @brief Gets the free cargo space the pilot has.
 *
 *    @luaparam p The pilot to get the free cargo space of.
 *    @luareturn The free cargo space in tons of the player.
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
 *    @luaparam p The pilot to get the cargo count of.
 *    @luaparam type Type of cargo to count.
 *    @luareturn The amount of cargo the player has.
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
 *    @luaparam p The pilot to add cargo to.
 *    @luaparam type Name of the cargo to add.
 *    @luaparam quantity Quantity of cargo to add.
 *    @luareturn The quantity of cargo added.
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
      NLUA_ERROR( L, "Quantity must be positive for pilot.cargoAdd otherwise use pilot.cargoRm" );;
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_cargoAdd( player.p, cargo, quantity );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Tries to remove cargo from the pilot's ship.
 *
 * @usage n = pilot.cargoRm( player.pilot(), "Food", 20 )
 *
 *    @luaparam p The pilot to remove cargo from.
 *    @luaparam type Name of the cargo to remove.
 *    @luaparam quantity Quantity of the cargo to remove.
 *    @luareturn The number of cargo removed.
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

   if (quantity > 0) {
      NLUA_ERROR( L, "Quantity must be positive for pilot.cargoRm otherwise use pilot.cargoAdd" );;
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
 * <li><b>name:</b> name of the cargo.
 * <li><b>q:</b> quantity of the targo.
 * <li><b>m:</b> true if cargo is for a mission.
 * </ul>
 *
 * @usage for _,v in ipairs(pilot.cargoList(player.pilot())) do print( string.format("%s: %d", v.name, v.q ) ) end
 *
 *    @luaparam p Pilot to list cargo of.
 *    @luareturn An ordered list with the names of the cargo the pilot has.
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
 *    @luaparam p Pilot to get the colour of.
 *    @luareturn The pilot's colour.
 * @luafunc colour( p )
 */
static int pilotL_getColour( lua_State *L )
{
   Pilot *p;
   glColour *col;
   LuaColour lc;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Set as hostile. */
   if (pilot_inRangePilot(player.p, p) == -1) col = &cMapNeutral;
   else if (pilot_isDisabled(p)) col = &cInert;
   else if (pilot_isFlag(p,PILOT_BRIBED)) col = &cNeutral;
   else if (pilot_isHostile(p)) col = &cHostile;
   else if (pilot_isFriendly(p)) col = &cFriend;
   else col = faction_getColour(p->faction);

   memcpy( &lc.col, col, sizeof(glColour) );
   lua_pushcolour( L, lc );

   return 1;
}


/**
 * @brief Returns whether the pilot is hostile to the player.
 *
 * @usage p:hostile()
 *
 *    @luaparam p Pilot to get the hostility of.
 *    @luareturn The pilot's hostility status.
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
 * @brief Gets the pilot's ship.
 *
 * @usage s = p:ship()
 *
 *    @luaparam p Pilot to get ship of.
 *    @luareturn The ship of the pilot.
 * @luafunc ship( p )
 */
static int pilotL_ship( lua_State *L )
{
   Pilot *p;
   LuaShip ls;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Create the ship. */
   ls.ship = p->ship;
   lua_pushship(L, ls);
   return 1;
}


/**
 * @brief Checks to see if the pilot is idle.
 *
 * @usage idle = p:idle() -- Returns true if the pilot is idle
 *
 *    @luaparam p Pilot to check to see if is idle.
 *    @luareturn true if pilot is idle, false otherwise
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
 *    @luaparam p Pilot to change manual control settings.
 *    @luaparam enable If true or nil enables pilot manual control, otherwise enables automatic AI.
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

   /* See if should mark as boarded. */
   if (enable) {
      pilot_setFlag(p, PILOT_MANUAL_CONTROL);
      if (pilot_isPlayer(p))
         ai_pinit( p, "player" );
   }
   else {
      pilot_rmFlag(p, PILOT_MANUAL_CONTROL);
      if (pilot_isPlayer(p))
         ai_destroy( p );
   }

   /* Clear task. */
   pilotL_taskclear( L );

   return 0;
}


/**
 * @brief Copies a value between two states.
 */
static int lua_copyvalue( lua_State *to, lua_State *from, int ind )
{
   switch (lua_type( from, ind )) {
      case LUA_TNIL:
         lua_pushnil(to);
         break;
      case LUA_TNUMBER:
         lua_pushnumber(to, lua_tonumber(from, ind));
         break;
      case LUA_TBOOLEAN:
         lua_pushboolean(to, lua_toboolean(from, ind));
         break;
      case LUA_TSTRING:
         lua_pushstring(to, lua_tostring(from, ind));
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
 *    @luaparam p Pilot to change memory of.
 *    @luaparam key Key of the memory part to change.
 *    @luaparam value Value to set to.
 * @luafunc memory( p, key, value )
 */
static int pilotL_memory( lua_State *L )
{
   lua_State *pL;
   Pilot *p;

   /* Get the pilot. */
   p  = luaL_validpilot(L,1);

   /* Set the pilot's memory. */
   if (p->ai == NULL) {
      NLUA_ERROR(L,"Pilot does not have AI.");
      return 0;
   }
   pL = p->ai->L;

   /* */
   lua_getglobal(pL, "pilotmem");
   /* pilotmem */
   lua_pushnumber(pL, p->id);
   /* pilotmem, id */
   lua_gettable(pL, -2);
   /* pilotmem, table */
   lua_copyvalue(pL, L, 2);
   /* pilotmem, table, key */
   lua_copyvalue(pL, L, 3);
   /* pilotmem, table, key, value */
   lua_settable(pL, -3);
   /* pilotmem, table */
   lua_pop(L,2);
   /* */

   return 0;
}


/**
 * @brief Clears all the tasks of the pilot.
 *
 * @usage p:taskClear()
 *
 *    @luaparam p Pilot to clear tasks of.
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
 * @usage p:goto( v, false, false ) -- Really rough aproximation of going to v without braking
 *
 *    @luaparam p Pilot to tell to go to a position.
 *    @luaparam v Vector target for the pilot.
 *    @luaparam brake If true (or nil) brakes the pilot near target position,
 *              otherwise pops the task when it is about to brake.
 *    @luaparam compensate If true (or nil) compensates for velocity, otherwise it
 *              doesn't. It only affects if brake is not set.
 * @luasee control
 * @luafunc goto( p, v, brake, compensate )
 */
static int pilotL_goto( lua_State *L )
{
   Pilot *p;
   Task *t;
   LuaVector *lv;
   int brake, compensate;
   const char *tsk;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   lv = luaL_checkvector(L,2);
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
   vectcpy( &t->dat.vec, &lv->vec );

   return 0;
}


/**
 * @brief Makes the pilot face a target.
 *
 * @usage p:face( enemy_pilot ) -- Face enemy pilot
 * @usage p:face( vec2.new( 0, 0 ) ) -- Face origin
 *
 *    @luaparam p Pilot to add task to.
 *    @luaparam target Target to face (can be vec2 or pilot).
 * @luafunc face( p, target )
 */
static int pilotL_face( lua_State *L )
{
   Pilot *p, *pt;
   LuaVector *vt;
   Task *t;

   /* Get parameters. */
   pt = NULL;
   vt = NULL;
   p  = luaL_validpilot(L,1);
   if (lua_ispilot(L,2))
      pt = luaL_validpilot(L,2);
   else
      vt = luaL_checkvector(L,2);

   /* Set the task. */
   t        = pilotL_newtask( L, p, "__face" );
   if (pt != NULL) {
      t->dtype = TASKDATA_INT;
      t->dat.num = pt->id;
   }
   else {
      t->dtype = TASKDATA_VEC2;
      vectcpy( &t->dat.vec, &vt->vec );
   }

   return 0;
}


/**
 * @brief Makes the pilot brake.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luaparam p Pilot to tell to brake.
 * @luasee control
 * @luafunc brake( p )
 */
static int pilotL_brake( lua_State *L )
{
   Pilot *p;
   Task *t;

   /* Get parameters. */
   p = luaL_validpilot(L,1);

   /* Set the task. */
   t = pilotL_newtask( L, p, "brake" );

   return 0;
}


/**
 * @brief Makes the pilot follow another pilot.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luaparam p Pilot to tell to follow another pilot.
 *    @luaparam pt Target pilot to follow.
 * @luasee control
 * @luafunc follow( p, pt )
 */
static int pilotL_follow( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   pt = luaL_validpilot(L,2);

   /* Set the task. */
   t        = pilotL_newtask( L, p, "follow" );
   t->dtype = TASKDATA_INT;
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
 *    @luaparam p Pilot to tell to attack another pilot.
 *    @luaparam pt Target pilot to attack (or nil to attack nearest enemy).
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
   t->dtype = TASKDATA_INT;
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
 *    @luaparam p Pilot to tell to runaway from another pilot.
 *    @luaparam tp Target pilot to runaway from.
 *    @luaparam nojump Whether or not the pilot should try to jump when running away.
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
   t->dtype = TASKDATA_INT;
   t->dat.num = pt->id;

   return 0;
}


/**
 * @brief Tells the pilot to hyperspace.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luaparam p Pilot to tell to hyperspace.
 *    @luaparam sys Optional system argument to jump to, uses random if nil.
 * @luasee control
 * @luafunc hyperspace( p, sys )
 */
static int pilotL_hyperspace( lua_State *L )
{
   Pilot *p;
   Task *t;
   LuaSystem *sys;
   StarSystem *ss;
   int i;
   JumpPoint *jp;
   double a, rad;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if (lua_gettop(L) > 1) {
      sys = luaL_checksystem( L, 2 );
      ss  = system_getIndex( sys->id );
   }
   else
      sys = NULL;

   /* Set the task. */
   t = pilotL_newtask( L, p, "__hyperspace" );
   if (sys != NULL) {
      /* Find the jump. */
      for (i=0; i < cur_system->njumps; i++) {
         jp = &cur_system->jumps[i];
         if (jp->target == ss) {
            break;
         }
      }
      if (i >= cur_system->njumps) {
         NLUA_ERROR( L, "System '%s' is not adjacent to current system '%s'", ss->name, cur_system->name );
         return 0;
      }

      /* Set nav target. */
      p->nav_hyperspace = i;

      /* Copy vector. */
      t->dtype = TASKDATA_VEC2;
      vectcpy( &t->dat.vec, &jp->pos );

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
 *    @luaparam p Pilot to tell to hyperspace.
 *    @luaparam planet Optional planet to land on, uses random if nil.
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
   if (lua_gettop(L) > 0)
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
      vectcpy( &t->dat.vec, &pnt->pos );
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
 *    @luaparam p Pilot to hail the player.
 *    @luaparam enable If true hails the pilot, if false disables the hailing. Defaults to true.
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
 *    @luaparam p Pilot to clear hooks.
 * @luafunc hookClear( p )
 */
static int pilotL_hookClear( lua_State *L )
{
   Pilot *p;

   p = luaL_validpilot(L,1);
   pilot_clearHooks( p );

   return 0;
}


