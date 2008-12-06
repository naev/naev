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

#include "lauxlib.h"

#include "log.h"
#include "naev.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"
#include "nlua_faction.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "space.h"


/*
 * Prototypes
 */
static int pilotL_createmetatable( lua_State *L );

/* pilots */
static int pilot_getPlayer( lua_State *L );
static int pilot_addFleet( lua_State *L );
static int pilot_clear( lua_State *L );
static int pilot_toggleSpawn( lua_State *L );
static const luaL_reg pilot_methods[] = {
   { "player", pilot_getPlayer },
   { "add", pilot_addFleet },
   { "clear", pilot_clear },
   { "toggleSpawn", pilot_toggleSpawn },
   {0,0}
}; /**< Pilot lua methods. */


/* Pilot metatable methods. */
static int pilotL_eq( lua_State *L );
static int pilotL_name( lua_State *L );
static int pilotL_alive( lua_State *L );
static int pilotL_rename( lua_State *L );
static int pilotL_position( lua_State *L );
static int pilotL_velocity( lua_State *L );
static int pilotL_warp( lua_State *L );
static int pilotL_broadcast( lua_State *L );
static int pilotL_setFaction( lua_State *L );
static const luaL_reg pilotL_methods[] = {
   { "__eq", pilotL_eq },
   { "name", pilotL_name },
   { "alive", pilotL_alive },
   { "rename", pilotL_rename },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "warp", pilotL_warp },
   { "broadcast", pilotL_broadcast },
   { "setFaction", pilotL_setFaction },
   {0,0}
}; /**< Pilot metatable methods. */




/**
 * @fn int lua_loadPilot( lua_State *L, int readonly )
 *
 * @brief Loads the space library.
 *
 *    @param L State to load space library into.
 *    @return 0 on success.
 */
int lua_loadPilot( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Register the functions. */
   luaL_register(L, "pilot", pilot_methods);

   /* Register the metatables. */
   pilotL_createmetatable( L );

   return 0;
}


/**
 * @fn static int pilotL_createmetatable( lua_State *L )
 *
 * @brief Registers the pilot metatable.
 *
 *    @param L Lua state to register metatable in.
 *    @return 0 on success.
 */
static int pilotL_createmetatable( lua_State *L )
{
   /* Create the metatable */
   luaL_newmetatable(L, PILOT_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, pilotL_methods);

   return 0; /* No error */
}




/**
 * @defgroup PILOT Pilot Lua bindings.
 *
 * @brief Lua bindings to interact with pilots.
 * @luamod pilot
 * Functions should be called like:
 *
 * @code
 * pilot.function( parameters )
 * @endcode
 */
/**
 * @defgroup META_PILOT Pilot Metatable
 *
 * @brief Reperesents a pilot in Lua.
 *
 * To call members of the metatable always use:
 * @code 
 * pilot:function( param )
 * @endcode
 */
/**
 * @fn LuaPilot* lua_topilot( lua_State *L, int ind )
 *
 * @brief Gets pilot at index.
 *
 *    @param L Lua state to get pilot from.
 *    @param ind Index position to find the pilot.
 *    @return Pilot found at the index in the state.
 */
LuaPilot* lua_topilot( lua_State *L, int ind )
{
   if (lua_isuserdata(L,ind)) {
      return (LuaPilot*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, PILOT_METATABLE);
   return NULL;
}
/**
 * @fn LuaPilot* lua_pushpilot( lua_State *L, LuaPilot planet )
 *
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
 * @fn int lua_ispilot( lua_State *L, int ind )
 *
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
 * @ingroup PILOT
 *
 * @brief Gets the player's pilot.
 *    @return Pilot pointing to the player.
 * @luafunc player()
 */
static int pilot_getPlayer( lua_State *L )
{
   LuaPilot lp;

   if (player == NULL) {
      lua_pushnil(L);
      return 1;
   }

   lp.pilot = player->id;
   lua_pushpilot(L,lp);
   return 1;
}

/**
 * @ingroup PILOT
 *
 * @brief Adds a fleet to the system.
 *    @luaparam fleetname Name of the fleet to add.
 *    @luaparam ai If set will override the standard fleet AI.  "def" means use default.
 *    @luaparam pos Position to create pilots around instead of choosing randomly.
 *    @return Table populated with all the pilots created.
 * @luafunc add( fleetname, ai, pos )
 */
static int pilot_addFleet( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   Fleet *flt;
   char *fltname, *fltai;
   int i, j;
   unsigned int p;
   double a;
   double d;
   Vector2d vv,vp, vn;
   FleetPilot *plt;
   LuaPilot lp;
   LuaVector *lv;

   /* Parse first argument - Fleet Name */
   if (lua_isstring(L,1)) fltname = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   
   /* Parse second argument - Fleet AI Override */
   if (lua_gettop(L) > 1) {
      if (lua_isstring(L,2)) {
         fltai = (char*) lua_tostring(L,2);
         if (strcmp(fltai, "def")==0) /* Check if set to default */
            fltai = NULL;
      }
      else NLUA_INVALID_PARAMETER();
   }
   else fltai = NULL;

   /* Parse third argument - Position */
   if (lua_gettop(L) > 2) {
      if (lua_isvector(L,3))
         lv = lua_tovector(L,3);
      else NLUA_INVALID_PARAMETER();
   }
   else lv = NULL;

   /* Needed to determine angle. */
   vectnull(&vn);

   /* pull the fleet */
   flt = fleet_get( fltname );
   if (flt == NULL) {
      NLUA_DEBUG("Fleet not found!");
      return 0;
   }

   /* Use position passed if possible. */
   if (lv != NULL)
      vectcpy( &vp, &lv->vec );
   else {
      d = RNGF()*(HYPERSPACE_ENTER_MAX-HYPERSPACE_ENTER_MIN) + HYPERSPACE_ENTER_MIN;
      vect_pset( &vp, d, RNG(0,360)*M_PI/180.);
   }

   /* now we start adding pilots and toss ids into the table we return */
   j = 0;
   lua_newtable(L);
   for (i=0; i<flt->npilots; i++) {

      plt = &flt->pilots[i];

      if (RNG(0,100) <= plt->chance) {

         /* fleet displacement */
         vect_cadd(&vp, RNG(75,150) * (RNG(0,1) ? 1 : -1),
               RNG(75,150) * (RNG(0,1) ? 1 : -1));

         /* Set velocity only if no position is set.. */
         if (lv != NULL) {
            if (VMOD(lv->vec) > HYPERSPACE_ENTER_MIN*0.9) {
               a = vect_angle(&vp,&vn);
               vect_pset( &vv, HYPERSPACE_VEL, a );
            }
            else
               vectnull( &vv );
         }
         else { /* Entering via hyperspace. */
            a = vect_angle(&vp,&vn);
            vect_pset( &vv, HYPERSPACE_VEL, a );
         }

         /* Create the pilot. */
         p = pilot_create( plt->ship,
               plt->name,
               flt->faction,
               (fltai != NULL) ? /* Lua AI Override */
                     fltai : 
                     (plt->ai != NULL) ? /* Pilot AI Override */
                        plt->ai : flt->ai,
               a,
               &vp,
               &vv,
               0 );

         /* we push each pilot created into a table and return it */
         lua_pushnumber(L,++j); /* index, starts with 1 */
         lp.pilot = p;
         lua_pushpilot(L,lp); /* value = LuaPilot */
         lua_rawset(L,-3); /* store the value in the table */
      }
   }
   return 1;
}
/**
 * @ingroup PILOT
 *
 * @brief Clears the current system of pilots.  Used for epic battles and such.
 * @luafunc clear()
 */
static int pilot_clear( lua_State *L )
{
   (void) L;
   pilots_clean();
   return 0;
}
/**
 * @ingroup PILOT
 *
 * @brief Disables or enables pilot spawning in the current system.  If player
 *  jumps the spawn is enabled again automatically.
 *    @luaparam enable true enables spawn, false disables it.
 *    @return The current spawn state.
 * @brief togglespawn( enable )
 */
static int pilot_toggleSpawn( lua_State *L )
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
 * @ingroup META_PILOT
 *
 * @brief Checks to see if pilot and p are the same.
 *    @luaparam p Pilot to compare against.
 *    @return true if they are the same.
 * @luafunc __eq( pilot )
 */
static int pilotL_eq( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaPilot *p1, *p2;

   /* Get parameters. */
   p1 = lua_topilot(L,1);
   if (lua_ispilot(L,2))
      p2 = lua_topilot(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Push result. */
   lua_pushboolean(L, p1->pilot == p2->pilot);
   return 1;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Gets the pilot's current name.
 *    @return The current name of the pilot.
 * @luafunc name()
 */
static int pilotL_name( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaPilot *p1;
   Pilot *p;

   /* Parse parameters. */
   p1 = lua_topilot(L,1);
   p = pilot_get( p1->pilot );

   /* Pilot must exist. */
   if (p == NULL) return 0;

   /* Get name. */
   lua_pushstring(L, p->name);
   return 1;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Checks to see if pilot is still alive.
 *    @return true if pilot is still alive.
 * @luafunc alive()
 */
static int pilotL_alive( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaPilot *lp;
   Pilot *p;

   /* Parse parameters. */
   lp = lua_topilot(L,1);
   p = pilot_get( lp->pilot );

   /* Check if is alive. */
   lua_pushboolean(L, p!=NULL);
   return 1;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Changes the pilot's name.
 *    @param name Name to change to.
 * @luafunc rename( name )
 */
static int pilotL_rename( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaPilot *p1;
   char *name;
   Pilot *p;

   /* Parse parameters */
   p1 = lua_topilot(L,1);
   p = pilot_get( p1->pilot );
   if (lua_isstring(L,2))
      name = (char*)lua_tostring(L, 2);
   else NLUA_INVALID_PARAMETER();

   /* Pilot must exist. */
   if (p == NULL) return 0;

   /* Change name. */
   if (p->name != NULL)
      free(p->name);
   p->name = strdup(name);

   return 0;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Gets the pilot's position.
 *    @return The pilot's current position.
 * @luafunc pos()
 */
static int pilotL_position( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaPilot *p1;
   Pilot *p;
   LuaVector v;

   /* Parse parameters */
   p1 = lua_topilot(L,1);
   p = pilot_get( p1->pilot );

   /* Pilot must exist. */
   if (p == NULL) return 0;

   /* Push position. */
   vectcpy( &v.vec, &p->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @fn static int pilotL_velocity( lua_State *L )
 * @ingroup META_PILOT
 *
 * @brief Gets the pilot's velocity.
 *    @return The pilot's current velocity.
 * @luafunc vel()
 */
static int pilotL_velocity( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaPilot *p1;
   Pilot *p;
   LuaVector v;

   /* Parse parameters */
   p1 = lua_topilot(L,1);
   p = pilot_get( p1->pilot );

   /* Pilot must exist. */
   if (p == NULL) return 0;

   /* Push velocity. */
   vectcpy( &v.vec, &p->solid->vel );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Sets the pilot's position.
 *    @luaparam pos Position to set.
 * @luafunc warp( pos )
 */
static int pilotL_warp( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaPilot *p1;
   Pilot *p;
   LuaVector *v;

   /* Parse parameters */
   p1 = lua_topilot(L,1);
   p = pilot_get( p1->pilot );
   if (lua_isvector(L,2))
      v = lua_tovector(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Pilot must exist. */
   if (p == NULL) return 0;

   /* Warp pilot to new position. */
   vectcpy( &p->solid->pos, &v->vec );
   vectnull( &p->solid->vel ); /* Clear velocity otherwise it's a bit weird. */
   return 0;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Makes the pilot broadcast a message.
 *    @luaparam msg Message to broadcast.
 * @luafunc broadcast( msg )
 */
static int pilotL_broadcast( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   Pilot *p;
   LuaPilot *lp;
   char *msg;

   /* Parse parameters. */
   lp = lua_topilot(L,1);
   if (lua_isstring(L,2))
      msg = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Check to see if pilot is valid. */
   p = pilot_get(lp->pilot);
   if (p == NULL)
      return 0;

   /* Broadcast message. */
   player_message( "Broadcast %s> \"%s\"", p->name, msg);
   return 0;
}

/**
 * @ingroup META_PILOT
 *
 * @brief Sets the pilot's faction.
 *    @luaparam faction Faction to set by name or faction.
 * @luafunc setFaction( faction )
 */
static int pilotL_setFaction( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   Pilot *p;
   LuaPilot *lp;
   LuaFaction *f;
   int fid;
   char *faction;

   /* Parse parameters. */
   lp = lua_topilot(L,1);
   if (lua_isstring(L,2)) {
      faction = (char*) lua_tostring(L,2);
      fid = faction_get(faction);
   }
   else if (lua_isfaction(L,2)) {
      f = lua_tofaction(L,2);
      fid = f->f;
   }
   else NLUA_INVALID_PARAMETER();

   /* Get pilot/faction. */
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Set the new faction. */
   p->faction = fid;

   return 0;
}

