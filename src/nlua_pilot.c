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

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "space.h"


/*
 * From pilot.c
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * Prototypes.
 */
/* External. */
extern int ai_pinit( Pilot *p, const char *ai );
extern void ai_destroy( Pilot* p );


/* Pilot metatable methods. */
static int pilot_getPlayer( lua_State *L );
static int pilot_addFleet( lua_State *L );
static int pilot_clear( lua_State *L );
static int pilot_toggleSpawn( lua_State *L );
static int pilot_getPilots( lua_State *L );
static int pilotL_eq( lua_State *L );
static int pilotL_name( lua_State *L );
static int pilotL_alive( lua_State *L );
static int pilotL_rename( lua_State *L );
static int pilotL_position( lua_State *L );
static int pilotL_velocity( lua_State *L );
static int pilotL_warp( lua_State *L );
static int pilotL_broadcast( lua_State *L );
static int pilotL_setFaction( lua_State *L );
static int pilotL_setHostile( lua_State *L );
static int pilotL_setFriendly( lua_State *L );
static int pilotL_disable( lua_State *L );
static int pilotL_addOutfit( lua_State *L );
static int pilotL_rmOutfit( lua_State *L );
static int pilotL_changeAI( lua_State *L );
static const luaL_reg pilotL_methods[] = {
   { "player", pilot_getPlayer },
   { "add", pilot_addFleet },
   { "clear", pilot_clear },
   { "toggleSpawn", pilot_toggleSpawn },
   { "get", pilot_getPilots },
   { "__eq", pilotL_eq },
   { "name", pilotL_name },
   { "alive", pilotL_alive },
   { "rename", pilotL_rename },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "warp", pilotL_warp },
   { "broadcast", pilotL_broadcast },
   { "setFaction", pilotL_setFaction },
   { "setHostile", pilotL_setHostile },
   { "setFriendly", pilotL_setFriendly },
   { "disable", pilotL_disable },
   { "addOutfit", pilotL_addOutfit },
   { "rmOutfit", pilotL_rmOutfit },
   { "changeAI", pilotL_changeAI },
   {0,0}
}; /**< Pilot metatable methods. */




/**
 * @brief Loads the space library.
 *
 *    @param L State to load space library into.
 *    @return 0 on success.
 */
int lua_loadPilot( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Create the metatable */
   luaL_newmetatable(L, PILOT_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, pilotL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, PILOT_METATABLE);

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
   if (lua_isuserdata(L,ind)) {
      return (LuaPilot*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, PILOT_METATABLE);
   return NULL;
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
 * @usage p = pilot.add( "Pirate Hyena" )
 * @usage p = pilot.add( "Trader Llama", "dummy" )
 * @usage p = pilot.add( "Sml Trader Convoy", "def", vec2.new( 1000, 200 ) )
 *
 *    @luaparam fleetname Name of the fleet to add.
 *    @luaparam ai If set will override the standard fleet AI.  "def" means use default.
 *    @luaparam pos Position to create pilots around instead of choosing randomly.
 *    @luareturn Table populated with all the pilots created.
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
         p = fleet_createPilot( flt, plt, a, &vp, &vv, fltai, 0 );

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
 * @brief Clears the current system of pilots.  Used for epic battles and such.
 *
 * @usage pilot.clear()
 * 
 * @luafunc clear()
 */
static int pilot_clear( lua_State *L )
{
   (void) L;
   pilots_clean();
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
 * @brief Gets the pilots available in the system by a certain criteria.
 *
 * @usage p = pilot.get() -- Gets all the pilots
 * @usage p = pilot.get( { faction.get("Empire") } ) -- Only gets empire pilots.
 *
 *    @luaparam f If f is a table of factions, it will only get pilots matching those factions.  Otherwise it gets all the pilots.
 *    @luareturn A table containing the pilots.
 * @luafunc get( f )
 */
static int pilot_getPilots( lua_State *L )
{
   int i, j, k;
   int *factions;
   int nfactions;
   LuaFaction *f;
   LuaPilot p;

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
                  !pilot_isDisabled(pilot_stack[i])) {
               lua_pushnumber(L, k++); /* key */
               p.pilot = pilot_stack[i]->id;
               lua_pushpilot(L, p); /* value */
               lua_rawset(L,-3); /* table[key] = value */
               break; /* Continue to next pilot. */
            }
         }
      }

      /* clean up. */
      free(factions);
   }
   else {
      /* Now put all the matching pilots in a table. */
      lua_newtable(L);
      k = 1;
      for (i=0; i<pilot_nstack; i++) {
         if (!pilot_isDisabled(pilot_stack[i])) {
            lua_pushnumber(L, k++); /* key */
            p.pilot = pilot_stack[i]->id;
            lua_pushpilot(L, p); /* value */
            lua_rawset(L,-3); /* table[key] = value */
            break; /* Continue to next pilot. */
         }
      }
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
 * @brief Checks to see if pilot is still alive.
 *
 * @usage if p:alive() then -- Pilot is still alive
 *
 *    @luaparam p Pilot to check to see if is still alive.
 *    @luareturn true if pilot is still alive.
 * @luafunc alive( p )
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
 * @brief Sets the pilot's position.
 *
 * @usage p:warp( vec2.new( 300, 200 ) )
 *
 *    @luaparam p Pilot to set the position of.
 *    @luaparam pos Position to set.
 * @luafunc warp( p, pos )
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
 * @brief Makes the pilot broadcast a message.
 *
 * @usage p:broadcast( "Mayday! Requesting assistance!" )
 *
 *    @luaparam p Pilot to broadcast the message.
 *    @luaparam msg Message to broadcast.
 * @luafunc broadcast( p, msg )
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
 * @brief Sets the pilot's faction.
 *
 * @usage p:setFaction( "Empire" )
 * @usage p:setFaction( faction.get( "Dvaered" ) )
 *
 *    @luaparam p Pilot to change faction of.
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
   LuaPilot *lp;
   Pilot *p;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Set as hostile. */
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
   LuaPilot *lp;
   Pilot *p;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Remove hostile and mark as friendly. */
   pilot_setFriendly(p);

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
   LuaPilot *lp;
   Pilot *p;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Disable the pilot. */
   p->shield = 0.;
   p->armour = PILOT_DISABLED_ARMOR * p->armour_max;
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
 *    @luaparam q Amount to add.
 *    @luareturn The amount actually added.
 * @luafunc addOutfit( p, outfit, q )
 */
static int pilotL_addOutfit( lua_State *L )
{
   LuaPilot *lp;
   Pilot *p;
   const char *outfit;
   int q, n;
   Outfit *o;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Get parameters. */
   outfit = luaL_checkstring(L,2);
   q      = luaL_checkint(L,3);

   /* Get the outfit. */
   o = outfit_get( outfit );
   if (o == NULL)
      return 0;
   
   /* Add outfit. */
   n = pilot_addOutfit( p, o, q );
   lua_pushnumber(L,n);
   return 1;
}


/**
 * @brief Removes an outfit from a pilot.
 *
 * @usage p:rmOutfit( "Neutron Disruptor", 1 ) -- Removes a neutron disruptor.
 *
 *    @luparam p Pilot to remove outfit from.
 *    @luaparam outfit Name of the outfit to remove.
 *    @luaparam q Amount to remove.
 *    @luareturn The amount actually removed.
 * @luafunc rmOutfit( p, outfit, q )
 */
static int pilotL_rmOutfit( lua_State *L )
{
   LuaPilot *lp;
   Pilot *p;
   const char *outfit;
   int q, n;
   Outfit *o;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Get parameters. */
   outfit = luaL_checkstring(L,2);
   q      = luaL_checkint(L,3);

   /* Get the outfit. */
   o = outfit_get( outfit );
   if (o == NULL)
      return 0;
   
   /* Add outfit. */
   n = pilot_rmOutfit( p, o, q );
   lua_pushnumber(L,n);
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
   LuaPilot *lp;
   Pilot *p;
   const char *str;
   int ret;

   /* Get the pilot. */
   lp = lua_topilot(L,1);
   p = pilot_get(lp->pilot);
   if (p==NULL) return 0;

   /* Get parameters. */
   str = luaL_checkstring(L,2);

   /* Get rid of current AI. */
   ai_destroy(p);

   /* Create the new AI. */
   ret = ai_pinit( p, str );
   lua_pushboolean(L, ret);
   return 1;
}

