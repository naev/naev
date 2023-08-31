/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_pilot.c
 *
 * @brief Handles the Lua pilot bindings.
 *
 * These bindings control the spobs and systems.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua_pilot.h"

#include "ai.h"
#include "array.h"
#include "camera.h"
#include "damagetype.h"
#include "debug.h"
#include "escort.h"
#include "gui.h"
#include "land_outfits.h"
#include "log.h"
#include "nlua.h"
#include "nlua_asteroid.h"
#include "nlua_canvas.h"
#include "nlua_colour.h"
#include "nlua_gfx.h"
#include "nlua_commodity.h"
#include "nlua_faction.h"
#include "nlua_jump.h"
#include "nlua_pilotoutfit.h"
#include "nlua_outfit.h"
#include "nlua_spob.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_vec2.h"
#include "nlua_tex.h"
#include "nluadef.h"
#include "pilot.h"
#include "pilot_heat.h"
#include "player.h"
#include "rng.h"
#include "space.h"
#include "weapon.h"

/*
 * From ai.c
 */
extern Pilot *cur_pilot;

/*
 * Prototypes.
 */
static int pilotL_getFriendOrFoe( lua_State *L, int friend );
static Task *pilotL_newtask( lua_State *L, Pilot* p, const char *task );
static int outfit_compareActive( const void *slot1, const void *slot2 );
static int pilotL_setFlagWrapper( lua_State *L, int flag );
static int pilot_outfitAddSlot( Pilot *p, const Outfit *o, PilotOutfitSlot *s, int bypass_slot, int bypass_cpu );
static int luaL_checkweapset( lua_State *L, int idx );
static PilotOutfitSlot *luaL_checkslot( lua_State *L, Pilot *p, int idx );

/* Pilot metatable methods. */
static int pilotL_add( lua_State *L );
static int pilotL_clone( lua_State *L );
static int pilotL_remove( lua_State *L );
static int pilotL_clear( lua_State *L );
static int pilotL_clearSelect( lua_State *L );
static int pilotL_canSpawn( lua_State *L );
static int pilotL_toggleSpawn( lua_State *L );
static int pilotL_getPilots( lua_State *L );
static int pilotL_getAllies( lua_State *L );
static int pilotL_getEnemies( lua_State *L );
static int pilotL_getVisible( lua_State *L );
static int pilotL_getInrange( lua_State *L );
static int pilotL_eq( lua_State *L );
static int pilotL_tostring( lua_State *L );
static int pilotL_name( lua_State *L );
static int pilotL_id( lua_State *L );
static int pilotL_exists( lua_State *L );
static int pilotL_target( lua_State *L );
static int pilotL_setTarget( lua_State *L );
static int pilotL_targetAsteroid( lua_State *L );
static int pilotL_setTargetAsteroid( lua_State *L );
static int pilotL_inrange( lua_State *L );
static int pilotL_inrangeAsteroid( lua_State *L );
static int pilotL_scandone( lua_State *L );
static int pilotL_withPlayer( lua_State *L );
static int pilotL_nav( lua_State *L );
static int pilotL_navSpob( lua_State *L );
static int pilotL_navJump( lua_State *L );
static int pilotL_weapsetActive( lua_State *L );
static int pilotL_weapset( lua_State *L );
static int pilotL_weapsetType( lua_State *L );
static int pilotL_weapsetAdd( lua_State *L );
static int pilotL_weapsetRm( lua_State *L );
static int pilotL_weapsetCleanup( lua_State *L );
static int pilotL_weapsetHeat( lua_State *L );
static int pilotL_weapsetSetInrange( lua_State *L );
static int pilotL_actives( lua_State *L );
static int pilotL_outfitsList( lua_State *L );
static int pilotL_outfits( lua_State *L );
static int pilotL_outfitsEquip( lua_State *L );
static int pilotL_outfitGet( lua_State *L );
static int pilotL_outfitToggle( lua_State *L );
static int pilotL_outfitReady( lua_State *L );
static int pilotL_rename( lua_State *L );
static int pilotL_position( lua_State *L );
static int pilotL_velocity( lua_State *L );
static int pilotL_isStopped( lua_State *L );
static int pilotL_dir( lua_State *L );
static int pilotL_evasion( lua_State *L );
static int pilotL_temp( lua_State *L );
static int pilotL_mass( lua_State *L );
static int pilotL_thrust( lua_State *L );
static int pilotL_speed( lua_State *L );
static int pilotL_speed_max( lua_State *L );
static int pilotL_turn( lua_State *L );
static int pilotL_faction( lua_State *L );
static int pilotL_areEnemies( lua_State *L );
static int pilotL_areAllies( lua_State *L );
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
static int pilotL_setHide( lua_State *L );
static int pilotL_setInvisible( lua_State *L );
static int pilotL_setNoRender( lua_State *L );
static int pilotL_setVisplayer( lua_State *L );
static int pilotL_setVisible( lua_State *L );
static int pilotL_setHilight( lua_State *L );
static int pilotL_setBribed( lua_State *L );
static int pilotL_getColour( lua_State *L );
static int pilotL_colourChar( lua_State *L );
static int pilotL_getHostile( lua_State *L );
static int pilotL_flags( lua_State *L );
static int pilotL_hasIllegal( lua_State *L );
static int pilotL_setActiveBoard( lua_State *L );
static int pilotL_setNoDeath( lua_State *L );
static int pilotL_disable( lua_State *L );
static int pilotL_cooldown( lua_State *L );
static int pilotL_setCooldown( lua_State *L );
static int pilotL_cooldownCycle( lua_State *L );
static int pilotL_setNoJump( lua_State *L );
static int pilotL_setNoLand( lua_State *L );
static int pilotL_setNoClear( lua_State *L );
static int pilotL_outfitAdd( lua_State *L );
static int pilotL_outfitRm( lua_State *L );
static int pilotL_outfitSlot( lua_State *L );
static int pilotL_outfitAddSlot( lua_State *L );
static int pilotL_outfitRmSlot( lua_State *L );
static int pilotL_outfitAddIntrinsic( lua_State *L );
static int pilotL_outfitRmIntrinsic( lua_State *L );
static int pilotL_getFuel( lua_State *L );
static int pilotL_setFuel( lua_State *L );
static int pilotL_intrinsicReset( lua_State *L );
static int pilotL_intrinsicSet( lua_State *L );
static int pilotL_intrinsicGet( lua_State *L );
static int pilotL_shippropReset( lua_State *L );
static int pilotL_shippropSet( lua_State *L );
static int pilotL_shippropGet( lua_State *L );
static int pilotL_effectClear( lua_State *L );
static int pilotL_effectAdd( lua_State *L );
static int pilotL_effectRm( lua_State *L );
static int pilotL_effectGet( lua_State *L );
static int pilotL_ai( lua_State *L );
static int pilotL_changeAI( lua_State *L );
static int pilotL_setTemp( lua_State *L );
static int pilotL_setHealth( lua_State *L );
static int pilotL_setHealthAbs( lua_State *L );
static int pilotL_addHealth( lua_State *L );
static int pilotL_setEnergy( lua_State *L );
static int pilotL_fillAmmo( lua_State *L );
static int pilotL_setNoboard( lua_State *L );
static int pilotL_setNoDisable( lua_State *L );
static int pilotL_setSpeedLimit( lua_State *L);
static int pilotL_getHealth( lua_State *L );
static int pilotL_getArmour( lua_State *L );
static int pilotL_getShield( lua_State *L );
static int pilotL_getEnergy( lua_State *L );
static int pilotL_getLockon( lua_State *L );
static int pilotL_getStats( lua_State *L );
static int pilotL_getShipStat( lua_State *L );
static int pilotL_getDetectedDistance( lua_State *L );
static int pilotL_cargoFree( lua_State *L );
static int pilotL_cargoHas( lua_State *L );
static int pilotL_cargoAdd( lua_State *L );
static int pilotL_cargoRm( lua_State *L );
static int pilotL_cargoJet( lua_State *L );
static int pilotL_cargoList( lua_State *L );
static int pilotL_credits( lua_State *L );
static int pilotL_worth( lua_State *L );
static int pilotL_ship( lua_State *L );
static int pilotL_points( lua_State *L );
static int pilotL_radius( lua_State *L );
static int pilotL_idle( lua_State *L );
static int pilotL_control( lua_State *L );
static int pilotL_memory( lua_State *L );
static int pilotL_shipmemory( lua_State *L );
static int pilotL_ainame( lua_State *L );
static int pilotL_task( lua_State *L );
static int pilotL_taskname( lua_State *L );
static int pilotL_taskstack( lua_State *L );
static int pilotL_taskdata( lua_State *L );
static int pilotL_taskclear( lua_State *L );
static int pilotL_pushtask( lua_State *L );
static int pilotL_poptask( lua_State *L );
static int pilotL_refuel( lua_State *L );
static int pilotL_moveto( lua_State *L );
static int pilotL_face( lua_State *L );
static int pilotL_brake( lua_State *L );
static int pilotL_follow( lua_State *L );
static int pilotL_attack( lua_State *L );
static int pilotL_board( lua_State *L );
static int pilotL_runaway( lua_State *L );
static int pilotL_gather( lua_State *L );
static int pilotL_canHyperspace( lua_State *L );
static int pilotL_hyperspace( lua_State *L );
static int pilotL_stealth( lua_State *L );
static int pilotL_tryStealth( lua_State *L );
static int pilotL_land( lua_State *L );
static int pilotL_hailPlayer( lua_State *L );
static int pilotL_msg( lua_State *L );
static int pilotL_mothership( lua_State *L );
static int pilotL_leader( lua_State *L );
static int pilotL_setLeader( lua_State *L );
static int pilotL_followers( lua_State *L );
static int pilotL_hookClear( lua_State *L );
static int pilotL_choosePoint( lua_State *L );
static int pilotL_collisionTest( lua_State *L );
static int pilotL_damage( lua_State *L );
static int pilotL_kill( lua_State *L );
static int pilotL_knockback( lua_State *L );
static int pilotL_calcStats( lua_State *L );
static int pilotL_showEmitters( lua_State *L );
static int pilotL_shipvarPeek( lua_State *L );
static int pilotL_shipvarPush( lua_State *L );
static int pilotL_shipvarPop( lua_State *L );
static int pilotL_render( lua_State *L );
static int pilotL_renderTo( lua_State *L );
static const luaL_Reg pilotL_methods[] = {
   /* General. */
   { "add", pilotL_add },
   { "clone", pilotL_clone },
   { "rm", pilotL_remove },
   { "get", pilotL_getPilots },
   { "getAllies", pilotL_getAllies },
   { "getEnemies", pilotL_getEnemies },
   { "getVisible", pilotL_getVisible },
   { "getInrange", pilotL_getInrange },
   { "__eq", pilotL_eq },
   { "__tostring", pilotL_tostring },
   /* Info. */
   { "name", pilotL_name },
   { "id", pilotL_id },
   { "exists", pilotL_exists },
   { "target", pilotL_target },
   { "setTarget", pilotL_setTarget },
   { "targetAsteroid", pilotL_targetAsteroid },
   { "setTargetAsteroid", pilotL_setTargetAsteroid },
   { "inrange", pilotL_inrange },
   { "inrangeAsteroid", pilotL_inrangeAsteroid },
   { "scandone", pilotL_scandone },
   { "withPlayer", pilotL_withPlayer },
   { "nav", pilotL_nav },
   { "navSpob", pilotL_navSpob },
   { "navJump", pilotL_navJump },
   { "weapsetActive", pilotL_weapsetActive },
   { "weapset", pilotL_weapset },
   { "weapsetType", pilotL_weapsetType },
   { "weapsetAdd", pilotL_weapsetAdd },
   { "weapsetRm", pilotL_weapsetRm },
   { "weapsetCleanup", pilotL_weapsetCleanup },
   { "weapsetHeat", pilotL_weapsetHeat },
   { "weapsetSetInrange", pilotL_weapsetSetInrange },
   { "actives", pilotL_actives },
   { "outfitsList", pilotL_outfitsList },
   { "outfits", pilotL_outfits },
   { "outfitsEquip", pilotL_outfitsEquip },
   { "outfitGet", pilotL_outfitGet },
   { "outfitToggle", pilotL_outfitToggle },
   { "outfitReady", pilotL_outfitReady },
   { "rename", pilotL_rename },
   { "pos", pilotL_position },
   { "vel", pilotL_velocity },
   { "isStopped", pilotL_isStopped },
   { "dir", pilotL_dir },
   { "evasion", pilotL_evasion },
   { "temp", pilotL_temp },
   { "mass", pilotL_mass },
   { "thrust", pilotL_thrust },
   { "speed", pilotL_speed },
   { "speedMax", pilotL_speed_max },
   { "turn", pilotL_turn },
   { "cooldown", pilotL_cooldown },
   { "faction", pilotL_faction },
   { "areEnemies", pilotL_areEnemies },
   { "areAllies", pilotL_areAllies },
   { "spaceworthy", pilotL_spaceworthy },
   { "health", pilotL_getHealth },
   { "armour", pilotL_getArmour },
   { "shield", pilotL_getShield },
   { "energy", pilotL_getEnergy },
   { "lockon", pilotL_getLockon },
   { "stats", pilotL_getStats },
   { "shipstat", pilotL_getShipStat },
   { "detectedDistance", pilotL_getDetectedDistance },
   { "colour", pilotL_getColour },
   { "colourChar", pilotL_colourChar },
   { "hostile", pilotL_getHostile },
   { "flags", pilotL_flags },
   { "hasIllegal", pilotL_hasIllegal },
   /* System. */
   { "clear", pilotL_clear },
   { "clearSelect", pilotL_clearSelect },
   { "canSpawn", pilotL_canSpawn },
   { "toggleSpawn", pilotL_toggleSpawn },
   /* Modify. */
   { "ai", pilotL_ai },
   { "changeAI", pilotL_changeAI },
   { "setTemp", pilotL_setTemp },
   { "setHealth", pilotL_setHealth },
   { "setHealthAbs", pilotL_setHealthAbs },
   { "addHealth", pilotL_addHealth },
   { "setEnergy", pilotL_setEnergy },
   { "fillAmmo", pilotL_fillAmmo },
   { "setNoboard", pilotL_setNoboard },
   { "setNoDisable", pilotL_setNoDisable },
   { "setSpeedLimit", pilotL_setSpeedLimit },
   { "setPos", pilotL_setPosition },
   { "setVel", pilotL_setVelocity },
   { "setDir", pilotL_setDir },
   { "setFaction", pilotL_setFaction },
   { "setHostile", pilotL_setHostile },
   { "setFriendly", pilotL_setFriendly },
   { "setInvincible", pilotL_setInvincible },
   { "setInvincPlayer", pilotL_setInvincPlayer },
   { "setHide", pilotL_setHide },
   { "setInvisible", pilotL_setInvisible },
   { "setNoRender", pilotL_setNoRender },
   { "setVisplayer", pilotL_setVisplayer },
   { "setVisible", pilotL_setVisible },
   { "setHilight", pilotL_setHilight },
   { "setBribed", pilotL_setBribed },
   { "setActiveBoard", pilotL_setActiveBoard },
   { "setNoDeath", pilotL_setNoDeath },
   { "disable", pilotL_disable },
   { "setCooldown", pilotL_setCooldown },
   { "cooldownCycle", pilotL_cooldownCycle },
   { "setNoJump", pilotL_setNoJump },
   { "setNoLand", pilotL_setNoLand },
   { "setNoClear", pilotL_setNoClear },
   /* Talk. */
   { "broadcast", pilotL_broadcast },
   { "comm", pilotL_comm },
   /* Outfits. */
   { "outfitAdd", pilotL_outfitAdd },
   { "outfitRm", pilotL_outfitRm },
   { "outfitSlot", pilotL_outfitSlot },
   { "outfitAddSlot", pilotL_outfitAddSlot },
   { "outfitRmSlot", pilotL_outfitRmSlot },
   { "outfitAddIntrinsic", pilotL_outfitAddIntrinsic },
   { "outfitRmIntrinsic", pilotL_outfitRmIntrinsic },
   { "fuel", pilotL_getFuel },
   { "setFuel", pilotL_setFuel },
   { "intrinsicReset", pilotL_intrinsicReset },
   { "intrinsicSet", pilotL_intrinsicSet },
   { "intrinsicGet", pilotL_intrinsicGet },
   { "shippropReset", pilotL_shippropReset },
   { "shippropSet", pilotL_shippropSet },
   { "shippropGet", pilotL_shippropGet },
   { "effectClear", pilotL_effectClear },
   { "effectAdd", pilotL_effectAdd },
   { "effectRm", pilotL_effectRm },
   { "effects", pilotL_effectGet },
   /* Ship. */
   { "ship", pilotL_ship },
   { "radius", pilotL_radius },
   { "points", pilotL_points },
   /* Cargo and moolah. */
   { "cargoFree", pilotL_cargoFree },
   { "cargoHas", pilotL_cargoHas },
   { "cargoAdd", pilotL_cargoAdd },
   { "cargoRm", pilotL_cargoRm },
   { "cargoJet", pilotL_cargoJet },
   { "cargoList", pilotL_cargoList },
   { "credits", pilotL_credits },
   { "worth", pilotL_worth },
   /* Manual AI control. */
   { "idle", pilotL_idle },
   { "control", pilotL_control },
   { "memory", pilotL_memory },
   { "shipMemory", pilotL_shipmemory },
   { "ainame", pilotL_ainame },
   { "task", pilotL_task },
   { "taskname", pilotL_taskname },
   { "taskstack", pilotL_taskstack },
   { "taskdata", pilotL_taskdata },
   { "taskClear", pilotL_taskclear },
   { "pushtask", pilotL_pushtask },
   { "poptask", pilotL_poptask },
   { "refuel", pilotL_refuel },
   { "moveto", pilotL_moveto },
   { "face", pilotL_face },
   { "brake", pilotL_brake },
   { "follow", pilotL_follow },
   { "attack", pilotL_attack },
   { "board", pilotL_board },
   { "runaway", pilotL_runaway },
   { "gather", pilotL_gather },
   { "canHyperspace", pilotL_canHyperspace },
   { "hyperspace", pilotL_hyperspace },
   { "stealth", pilotL_stealth },
   { "tryStealth", pilotL_tryStealth },
   { "land", pilotL_land },
   /* Misc. */
   { "hailPlayer", pilotL_hailPlayer },
   { "msg", pilotL_msg },
   { "mothership" ,pilotL_mothership },
   { "leader", pilotL_leader },
   { "setLeader", pilotL_setLeader },
   { "followers", pilotL_followers },
   { "hookClear", pilotL_hookClear },
   { "choosePoint", pilotL_choosePoint },
   { "collisionTest", pilotL_collisionTest },
   { "damage", pilotL_damage },
   { "kill", pilotL_kill },
   { "knockback", pilotL_knockback },
   { "calcStats", pilotL_calcStats },
   { "showEmitters", pilotL_showEmitters },
   { "shipvarPeek", pilotL_shipvarPeek },
   { "shipvarPush", pilotL_shipvarPush },
   { "shipvarPop", pilotL_shipvarPop },
   { "render", pilotL_render },
   { "renderTo", pilotL_renderTo },
   {0,0},
}; /**< Pilot metatable methods. */

/**
 * @brief Loads the pilot library.
 *
 *    @param env Environment to load library into.
 *    @return 0 on success.
 */
int nlua_loadPilot( nlua_env env )
{
   nlua_register(env, PILOT_METATABLE, pilotL_methods, 1);

   /* Pilot always loads ship and asteroid. */
   nlua_loadShip(env);
   nlua_loadAsteroid(env);

   return 0;
}

/**
 * @brief Wrapper to simplify flag setting stuff.
 */
static int pilotL_setFlagWrapper( lua_State *L, int flag )
{
   int state;
   Pilot *p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_isnone(L,2))
      state = 1;
   else
      state = lua_toboolean(L, 2);

   /* Set or remove the flag. */
   if (state)
      pilot_setFlag( p, flag );
   else
      pilot_rmFlag( p, flag );

   return 0;
}

/**
 * @brief Lua bindings to interact with pilots.
 *
 * This will allow you to create and manipulate pilots in-game.
 *
 * An example would be:
 * @code
 * p = pilot.add( "Llama", "Miner" ) -- Create a Miner Llama
 * p:setFriendly() -- Make it friendly
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
   Pilot *p = pilot_get(luaL_checkpilot(L,ind));
   if (p==NULL) {
      NLUA_ERROR(L,_("Pilot is invalid."));
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
   LuaPilot *p = (LuaPilot*) lua_newuserdata(L, sizeof(LuaPilot));
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
 * @brief Returns a suitable jumpin spot for a given pilot.
 * @usage point = pilot.choosePoint( f, i, g )
 *
 *    @luatparam Faction f Faction the pilot will belong to.
 *    @luatparam[opt=false] boolean i Whether to ignore rules.
 *    @luatparam[opt=false] boolean g Whether to behave as guerilla (spawn in deep space)
 *    @luatreturn Spob|Vec2|Jump A randomly chosen suitable spawn point.
 * @luafunc choosePoint
 */
static int pilotL_choosePoint( lua_State *L )
{
   LuaFaction lf;
   int ignore_rules, guerilla;
   Spob *spob = NULL;
   JumpPoint *jump = NULL;
   vec2 vp;

   /* Parameters. */
   lf             = luaL_validfaction(L,1);
   ignore_rules   = lua_toboolean(L,2);
   guerilla       = lua_toboolean(L,3);

   pilot_choosePoint( &vp, &spob, &jump, lf, ignore_rules, guerilla );

   if (spob != NULL)
      lua_pushspob(L, spob->id );
   else if (jump != NULL)
      lua_pushsystem(L, jump->from->id);
   else
      lua_pushvector(L, vp);

   return 1;
}

/**
 * @brief Adds a ship with an AI and faction to the system (instead of a predefined fleet).
 *
 * @usage p = pilot.add( "Empire Shark", "Empire" ) -- Creates a standard Empire Shark.
 * @usage p = pilot.add( "Pirate Hyena", "Pirate", _("Blackbeard") ) -- Just adds the pilot (will jump in or take off).
 * @usage p = pilot.add( "Llama", "Trader", nil, _("Trader Llama"), {ai="dummy"} ) -- Overrides AI with dummy ai.
 * @usage p = pilot.add( "Gawain", "Civilian", vec2.new( 1000, 200 ) ) -- Pilot won't jump in, will just appear.
 * @usage p = pilot.add( "Empire Pacifier", "Empire", system.get("Goddard") ) -- Have the pilot jump in from the system.
 * @usage p = pilot.add( "Goddard", "Goddard", spob.get("Zhiru") , _("Goddard Goddard") ) -- Have the pilot take off from a spob.
 *
 * How param works (by type of value passed): <br/>
 *  - nil: spawns pilot randomly entering from jump points with presence of their faction or taking off from non-hostile spobs <br/>
 *  - spob: pilot takes off from the spob <br/>
 *  - system: jumps pilot in from the system <br/>
 *  - vec2: pilot is created at the position (no jump/takeoff) <br/>
 *  - true: Acts like nil, but does not avoid jump points with no presence <br/>
 *
 *    @luatparam Ship|string shipname Name of the ship to add.
 *    @luatparam Faction |stringfaction Faction to give the pilot.
 *    @luatparam System|Spob|Vec2 param Position to create pilot at, if it's a system it'll try to jump in from that system, if it's
 *              a spob it'll try to take off from it.
 *    @luatparam[opt] string pilotname Name to give the pilot. Defaults to shipname.
 *    @luatparam[opt] table parameters Table of extra keyword arguments. Supported arguments:
 *                    "ai" (string): AI to give the pilot. Defaults to the faction's AI.
 *                    "naked" (boolean): Whether or not to have the pilot spawn without outfits. Defaults to false.
 *                    "stealth" (boolean): Whether or not to have the pilot spawn in stealth mode. Defaults to false.
 *    @luatreturn Pilot The created pilot.
 * @luafunc add
 */
static int pilotL_add( lua_State *L )
{
   const Ship *ship;
   const char *pilotname, *ai;
   double a, r;
   vec2 vv, vp, vn;
   LuaFaction lf;
   StarSystem *ss;
   Spob *spob;
   JumpPoint *jump;
   PilotFlags flags;
   int ignore_rules;
   Pilot *p;

   /* Default values. */
   pilot_clearFlagsRaw( flags );
   vectnull(&vn); /* Need to determine angle. */
   ss    = NULL;
   jump  = NULL;
   spob  = NULL;
   a     = 0.;

   /* Parse first argument - Ship Name */
   ship = luaL_validship(L,1);
   /* Get faction from string or number. */
   lf = luaL_validfaction(L,2);
   /* Get pilotname argument if provided. */
   pilotname = luaL_optstring( L, 4, _(ship->name) );

   /* Handle position/origin argument. */
   if (lua_isvector(L,3)) {
      vp = *lua_tovector(L,3);
      a = RNGF() * 2.*M_PI;
      vectnull( &vv );
   }
   else if (lua_issystem(L,3))
      ss = system_getIndex( lua_tosystem(L,3) );
   else if (lua_isjump(L,3))
      ss = system_getIndex( lua_tojump(L,3)->destid );
   else if (lua_isspob(L,3)) {
      spob  = luaL_validspob(L,3);
      pilot_setFlagRaw( flags, PILOT_TAKEOFF );
      a = RNGF() * 2. * M_PI;
      r = RNGF() * spob->radius;
      vec2_cset( &vp,
            spob->pos.x + r * cos(a),
            spob->pos.y + r * sin(a) );
      a = RNGF() * 2.*M_PI;
      vectnull( &vv );
   }
   /* Random. */
   else if (lua_isnoneornil(L,3)) {
      /* Check if we should ignore the strict rules. */
      ignore_rules = 0;
      if (lua_isboolean(L,3) && lua_toboolean(L,3))
         ignore_rules = 1;

      /* Choose the spawn point and act in consequence.*/
      pilot_choosePoint( &vp, &spob, &jump, lf, ignore_rules, 0 );

      if (spob != NULL) {
         pilot_setFlagRaw( flags, PILOT_TAKEOFF );
         a = RNGF() * 2. * M_PI;
         r = RNGF() * spob->radius;
         vec2_cset( &vp,
               spob->pos.x + r * cos(a),
               spob->pos.y + r * sin(a) );
         a = RNGF() * 2.*M_PI;
         vectnull( &vv );
      }
      else {
         a = RNGF() * 2.*M_PI;
         vectnull( &vv );
      }
   }
   else
      NLUA_INVALID_PARAMETER(L);

   /* Handle system. */
   if (ss != NULL) {
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         if ((cur_system->jumps[i].target == ss)
               && !jp_isFlag( cur_system->jumps[i].returnJump, JP_EXITONLY )) {
            jump = cur_system->jumps[i].returnJump;
            break;
         }
      }
      if (jump == NULL) {
         if (array_size(cur_system->jumps) > 0) {
            WARN(_("Ship '%s' jumping in from non-adjacent system '%s' to '%s'."),
                  pilotname, ss->name, cur_system->name );
            jump = cur_system->jumps[RNG_BASE(0, array_size(cur_system->jumps)-1)].returnJump;
         }
         else
            WARN(_("Ship '%s' attempting to jump in from '%s', but '%s' has no jump points."),
                  pilotname, ss->name, cur_system->name );
      }
   }

   /* Parse final argument - table of optional parameters */
   ai = NULL;
   if (lua_gettop( L ) >= 5 && !lua_isnil( L, 5 )) {
      if (!lua_istable( L, 5 )) {
         NLUA_ERROR( L, _("'parameters' should be a table of options or omitted!") );
         return 0;
      }
      lua_getfield( L, 5, "ai" );
      ai = luaL_optstring( L, -1, NULL );
      lua_pop( L, 1 );

      lua_getfield( L, 5, "naked" );
      if (lua_toboolean(L, -1))
         pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );
      lua_pop( L, 1 );

      lua_getfield( L, 5, "stealth" );
      if (lua_toboolean(L, -1))
         pilot_setFlagRaw( flags, PILOT_STEALTH );
      lua_pop( L, 1 );
   }

   /* Set up velocities and such. */
   if (jump != NULL) {
      space_calcJumpInPos( cur_system, jump->from, &vp, &vv, &a, NULL );
      pilot_setFlagRaw( flags, PILOT_HYP_END );
   }

   /* Make sure angle is valid. */
   a = fmod( a, 2.*M_PI );
   if (a < 0.)
      a += 2.*M_PI;

   /* Create the pilot. */
   p = pilot_create( ship, pilotname, lf, ai, a, &vp, &vv, flags, 0, 0 );
   lua_pushpilot(L,p->id);
   if (jump==NULL) {
      ai_newtask( L, p, "idle_wait", 0, 1 );
      p->timer[0] = p->tcontrol;
   }

   /* TODO don't have space_calcJumpInPos called twice when stealth creating. */
   if ((jump != NULL) && pilot_isFlagRaw( flags, PILOT_STEALTH )) {
      space_calcJumpInPos( cur_system, jump->from, &p->solid.pos, &p->solid.vel, &p->solid.dir, p );
   }
   return 1;
}

/**
 * @brief Clones a pilot. It will share nearly all properties including outfits.
 *
 *    @luatparam Pilot p Pilot to clone.
 *    @luatreturn Pilot A new clone of the pilot.
 * @luafunc clone
 */
static int pilotL_clone( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   LuaPilot lp = pilot_clone( p );
   lua_pushpilot( L, lp );
   return 1;
}

/**
 * @brief Removes a pilot without explosions or anything.
 *
 * Does nothing if the pilot does not exist.
 *
 * @usage p:rm() -- pilot will be destroyed
 *
 *    @luatparam Pilot p Pilot to remove.
 * @luafunc rm
 */
static int pilotL_remove( lua_State *L )
{
   Pilot *p = pilot_get( luaL_checkpilot(L,1) );
   if (p==NULL)
      return 0;

   /* Player is destroyed. */
   if (pilot_isPlayer(p))
      player_destroyed();

   /* Deletes the pilot. */
   pilot_delete(p);

   return 0;
}
/**
 * @brief Removes all pilots belonging to a faction from the system.
 *
 * @luatparam Faction fac Faction name/object to selectively clear.
 *
 * @usage pilot.clearSelect("Empire")
 *
 * @luafunc clearSelect
 */
static int pilotL_clearSelect( lua_State *L )
{
   int f = luaL_validfaction(L,1);
   Pilot *const* pilot_stack = pilot_getAll();

   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *pi = pilot_stack[i];
      if ((pi->faction == f) &&
            !pilot_isFlag(pi, PILOT_DELETE) &&
            !pilot_isFlag(pi, PILOT_DEAD) &&
            !pilot_isFlag(pi, PILOT_HIDE))
         pilot_delete(pi);
   }

   return 0;
}
/**
 * @brief Clears the current system of pilots. Used for epic battles and such.
 *
 * Be careful with this command especially in big systems. It will most likely
 * cause issues if multiple missions are in the same system.
 *
 * @note Clears all global pilot hooks too.
 *
 * @usage pilot.clear()
 *
 * @luafunc clear
 */
static int pilotL_clear( lua_State *L )
{
   (void) L;
   pilots_clear();
   weapon_clear();
   return 0;
}

/**
 * @brief Returns if pilots can can spawn naturally in the current system.
 *
 *    @luatreturn boolean The current spawn state.
 * @luafunc canSpawn
 */
static int pilotL_canSpawn( lua_State *L )
{
   lua_pushboolean( L, space_spawn );
   return 1;
}

/**
 * @brief Disables or enables pilot spawning in the current system.
 *
 * If player jumps the spawn is enabled again automatically. Global spawning takes priority over faction spawning.
 *
 * @usage pilot.toggleSpawn() -- Defaults to flipping the global spawning (true->false and false->true)
 * @usage pilot.toggleSpawn( false ) -- Disables global spawning
 * @usage pilot.toggleSpawn( "Pirate" ) -- Defaults to disabling pirate spawning
 * @usage pilot.toggleSpawn( "Pirate", true ) -- Turns on pirate spawning
 *
 *    @luatparam[opt] Faction fid Faction to enable or disable spawning off. If ommited it works on global spawning.
 *    @luatparam[opt] boolean enable true enables spawn, false disables it.
 *    @luatreturn boolean The current spawn state.
 * @luafunc toggleSpawn
 */
static int pilotL_toggleSpawn( lua_State *L )
{
   /* Setting it directly. */
   if (!lua_isnoneornil(L,1)) {
      if (lua_isfaction(L,1) || lua_isstring(L,1)) {
         int f = luaL_validfaction(L,1);
         int b = !lua_toboolean(L,2);

         /* Find the faction and set. */
         for (int i=0; i<array_size(cur_system->presence); i++) {
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
 * @luafunc get
 */
static int pilotL_getPilots( lua_State *L )
{
   int d = lua_toboolean(L,2); /* Whether or not to get disabled. */
   Pilot *const* pilot_stack = pilot_getAll();

   /* Check for belonging to faction. */
   if (lua_istable(L,1) || lua_isfaction(L,1)) {
      int *factions;
      if (lua_isfaction(L,1)) {
         factions = array_create( int );
         array_push_back( &factions, lua_tofaction(L,1) );
      }
      else {
         /* Get table length and preallocate. */
         factions = array_create_size( int, lua_objlen(L,1) );
         /* Load up the table. */
         lua_pushnil(L);
         while (lua_next(L, 1) != 0) {
            if (lua_isfaction(L,-1))
               array_push_back( &factions, lua_tofaction(L, -1) );
            lua_pop(L,1);
         }
      }

      /* Now put all the matching pilots in a table. */
      lua_newtable(L);
      int k = 1;
      for (int i=0; i<array_size(pilot_stack); i++) {
         for (int j=0; j<array_size(factions); j++) {
            if ((pilot_stack[i]->faction == factions[j]) &&
                  (d || !pilot_isDisabled(pilot_stack[i])) &&
                  !pilot_isFlag(pilot_stack[i], PILOT_DELETE)) {
               lua_pushpilot(L, pilot_stack[i]->id); /* value */
               lua_rawseti(L,-2, k++); /* table[key] = value */
            }
         }
      }

      /* clean up. */
      array_free( factions );
   }
   else if ((lua_isnil(L,1)) || (lua_gettop(L) == 0)) {
      /* Now put all the matching pilots in a table. */
      lua_newtable(L);
      int k = 1;
      for (int i=0; i<array_size(pilot_stack); i++) {
         if ((d || !pilot_isDisabled(pilot_stack[i])) &&
               !pilot_isFlag(pilot_stack[i], PILOT_DELETE)) {
            lua_pushpilot(L, pilot_stack[i]->id); /* value */
            lua_rawseti(L,-2,k++); /* table[key] = value */
         }
      }
   }
   else {
      NLUA_INVALID_PARAMETER(L);
   }

   return 1;
}

static int getFriendOrFoeTest( const Pilot *p, const Pilot *plt, int friend, double dd, int inrange, int dis, int fighters, const vec2 *v, LuaFaction lf )
{
   /* Check if dead. */
   if (pilot_isFlag(plt, PILOT_DELETE))
      return 0;

   /* Ignore self. */
   if ((p!=NULL) && (p->id==plt->id))
      return 0;

   /* Ignore fighters unless specified. */
   if (!fighters && pilot_isFlag(plt, PILOT_CARRIED))
      return 0;

   /* Check distance if necessary. */
   if ((dd >= 0.) &&
         vec2_dist2(&plt->solid.pos, v) > dd)
      return 0;

   /* Check if disabled. */
   if (dis && pilot_isDisabled(plt))
      return 0;

   /* Check appropriate faction. */
   if (friend) {
      if (p==NULL) {
         if (!areAllies( lf, plt->faction ))
            return 0;
      }
      else {
         if (!pilot_areAllies( p, plt ))
            return 0;
      }
   }
   else {
      if (p==NULL) {
         if (!areEnemies( lf, plt->faction ))
            return 0;
      }
      else {
         if (inrange) {
            if (!pilot_validEnemy( p, plt ))
               return 0;
         }
         else {
            if (!pilot_areEnemies( p, plt ))
               return 0;
         }
      }
   }

   /* Need extra check for friends. */
   if ((p!=NULL) && inrange && friend) {
      if (!pilot_inRangePilot( p, plt, NULL ))
         return 0;
   }

   return 1;
}

/*
 * Helper to get nearby friends or foes.
 */
static int pilotL_getFriendOrFoe( lua_State *L, int friend )
{
   int k;
   double dd;
   const Pilot *p;
   double dist;
   int inrange, dis, fighters;
   const vec2 *v;
   Pilot *const* pilot_stack;
   LuaFaction lf;

   /* Check if using faction. */
   lf = -1;
   if (lua_isfaction(L,1))
      lf = lua_tofaction(L,1);
   else if (lua_isstring(L,1))
      lf = luaL_validfaction(L,1);
   /* Faction case. */
   if (lf >= 0) {
      dist  = luaL_optnumber(L,2,-1.);
      v     = luaL_checkvector(L,3);
      inrange = 0;
      dis   = lua_toboolean(L,5);
      fighters = lua_toboolean(L,6);
      p     = NULL;
   }
   /* Pilot case. */
   else {
      p     = luaL_validpilot(L,1);
      dist  = luaL_optnumber(L,2,-1.);
      v     = luaL_optvector(L,3,&p->solid.pos);
      inrange = !lua_toboolean(L,4);
      dis   = lua_toboolean(L,5);
      fighters = lua_toboolean(L,6);
   }

   if (dist > 0.)
      dd = pow2(dist);
   else
      dd = -1.;

   /* Now put all the matching pilots in a table. */
   pilot_stack = pilot_getAll();
   lua_newtable(L);
   k = 1;
   if (dist >= 0. && dist < INFINITY) {
      int x, y, r;
      const IntList *qt;
      x = round(v->x);
      y = round(v->y);
      r = ceil(dist);
      qt = pilot_collideQuery( x-r, y-r, x+r, y+r );
      for (int i=0; i<il_size(qt); i++) {
         Pilot *plt = pilot_stack[ il_get( qt, i, 0 ) ];

         if (getFriendOrFoeTest( p, plt, friend, dd, inrange, dis, fighters, v, lf )) {
            lua_pushpilot(L, plt->id); /* value */
            lua_rawseti(L,-2, k++); /* table[key] = value */
         }
      }
   }
   else {
      for (int i=0; i<array_size(pilot_stack); i++) {
         Pilot *plt = pilot_stack[i];

         if (getFriendOrFoeTest( p, plt, friend, dd, inrange, dis, fighters, v, lf )) {
            lua_pushpilot(L, plt->id); /* value */
            lua_rawseti(L,-2, k++); /* table[key] = value */
         }
      }
   }
   return 1;
}

/**
 * @brief Gets friendly pilots to a pilot (or faction) within a certain distance.
 *
 * @usage p:getAllies( 5000 ) -- get allies within 5000
 * @usage pilot.getAllies( faction.get("Pirate"), 5000, vec2.new(0,0) ) -- Got allies of "Pirate" faction 5000 units from origin
 *
 *    @luatparam Pilot|faction pilot Pilot or to get allies of.
 *    @luatparam[opt=infinity] number dist Distance to look for allies.
 *    @luatparam[opt=pilot.pos] Vec2 pos Position to check from.
 *    @luatparam[opt=false] boolean ignore_range Whether or not to only check for pilots in range (only in the case of pilot, not faction)
 *    @luatparam[opt=false] boolean disabled Whether or not to count disabled pilots.
 *    @luatparam[opt=false] boolean fighters Whether or not to count deployed fighters.
 *    @luatreturn {Pilot,...} A table containing the pilots.
 * @luafunc getAllies
 * @see getEnemies
 * @see getInrange
 */
static int pilotL_getAllies( lua_State *L )
{
   return pilotL_getFriendOrFoe( L, 1 );
}

/**
 * @brief Gets hostile pilots to a pilot (or faction) within a certain distance.
 *
 * @usage p:getEnemies( 5000 ) -- get hostiles within 5000
 * @usage pilot.getEnemies( faction.get("Pirate"), 5000, vec2.new(0,0) ) -- Got hostiles of "Pirate" faction 5000 units from origin
 *
 *    @luatparam Pilot|faction pilot Pilot or to get hostiles of.
 *    @luatparam[opt=infinity] number dist Distance to look for hostiles.
 *    @luatparam[opt=pilot.pos] Vec2 pos Position to check from.
 *    @luatparam[opt=false] boolean ignore_range Whether or not to ignore checks for pilots in range (only in the case of pilot, not faction)
 *    @luatparam[opt=false] boolean disabled Whether or not to count disabled pilots.
 *    @luatparam[opt=false] boolean fighters Whether or not to count deployed fighters.
 *    @luatreturn {Pilot,...} A table containing the pilots.
 * @luafunc getEnemies
 * @see getAllies
 * @see getInrange
 */
static int pilotL_getEnemies( lua_State *L )
{
   return pilotL_getFriendOrFoe( L, 0 );
}

/**
 * @brief Gets visible pilots to a pilot.
 *
 * @note This function can not use quadtrees and is much slower than getEnemies, getAllies, or getInrange.
 *
 *    @luatparam Pilot pilot Pilot to get visible pilots of.
 *    @luatparam[opt=false] boolean disabled Whether or not to count disabled pilots.
 *    @luatreturn {Pilot,...} A table containing the pilots.
 * @luafunc getVisible
 * @see getEnemies
 * @see getAllies
 * @see getInrange
 */
static int pilotL_getVisible( lua_State *L )
{
   int k;
   const Pilot *p = luaL_validpilot(L,1);
   int dis = lua_toboolean(L,2);
   Pilot *const* pilot_stack;

   /* Now put all the matching pilots in a table. */
   pilot_stack = pilot_getAll();
   lua_newtable(L);
   k = 1;
   for (int i=0; i<array_size(pilot_stack); i++) {
      /* Check if dead. */
      if (pilot_isFlag(pilot_stack[i], PILOT_DELETE))
         continue;
      /* Check if disabled. */
      if (dis && pilot_isDisabled(pilot_stack[i]))
         continue;
      /* Check visibilitiy. */
      if (!pilot_validTarget( p, pilot_stack[i] ))
         continue;

      lua_pushpilot(L, pilot_stack[i]->id); /* value */
      lua_rawseti(L,-2,k++); /* table[key] = value */
   }

   return 1;
}

/**
 * @brief Gets visible pilots to a pilot within a certain distance.
 *
 *    @luatparam vec2 pos Position to get pilots in range of.
 *    @luatparam number d Distance to get pilots in.
 *    @luatparam[opt=false] boolean disabled Whether or not to count disabled pilots.
 *    @luatreturn {Pilot,...} A table containing the pilots.
 * @luafunc getInrange
 * @see getEnemies
 * @see getAllies
 */
static int pilotL_getInrange( lua_State *L )
{
   int k;
   const vec2 *v = luaL_checkvector(L,1);
   double d = luaL_checknumber(L,2);
   int dis = lua_toboolean(L,3);
   int x, y, r;
   const IntList *qt;
   Pilot *const* pilot_stack = pilot_getAll();

   d = pow2(d); /* Square it. */

   /* Now put all the matching pilots in a table. */
   x = round(v->x);
   y = round(v->y);
   r = ceil(d);
   qt = pilot_collideQuery( x-r, y-r, x+r, y+r );
   lua_newtable(L);
   k = 1;
   for (int i=0; i<il_size(qt); i++) {
      Pilot *p = pilot_stack[ il_get( qt, i, 0 ) ];

      /* Check if dead. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;
      /* Check if hidden. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;
      /* Check if disabled. */
      if (dis && pilot_isDisabled(p))
         continue;

      /* Must be in range. */
      if (vec2_dist2( v, &p->solid.pos ) > d )
         continue;

      lua_pushpilot(L, p->id); /* value */
      lua_rawseti(L,-2,k++); /* table[key] = value */
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
 * @luafunc __eq
 */
static int pilotL_eq( lua_State *L )
{
   LuaPilot p1 = luaL_checkpilot(L,1);
   LuaPilot p2 = luaL_checkpilot(L,2);
   lua_pushboolean(L, p1 == p2);
   return 1;
}

/**
 * @brief Gets the pilot's current (translated) name or notes it is inexistent.
 *
 * @usage tostring(p)
 *
 *    @luatparam Pilot p Pilot to convert to string.
 *    @luatreturn string The current name of the pilot or "(inexistent pilot)" if not existent.
 * @luafunc __tostring
 */
static int pilotL_tostring( lua_State *L )
{
   LuaPilot lp = luaL_checkpilot( L, 1 );
   const Pilot *p = pilot_get(lp);
   if (p!=NULL)
      lua_pushstring(L,p->name);
   else
      lua_pushstring(L,"(inexistent pilot)");
   return 1;
}

/**
 * @brief Gets the pilot's current (translated) name.
 *
 * @usage name = p:name()
 *
 *    @luatparam Pilot p Pilot to get the name of.
 *    @luatreturn string The current name of the pilot.
 * @luafunc name
 */
static int pilotL_name( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
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
 * @luafunc id
 */
static int pilotL_id( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
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
 * @luafunc exists
 */
static int pilotL_exists( lua_State *L )
{
   int exists;
   const Pilot *p = pilot_get( luaL_checkpilot(L,1) );

   /* Must still be kicking and alive. */
   if (p==NULL)
      exists = 0;
   else if (pilot_isFlag( p, PILOT_DEAD ) || pilot_isFlag( p, PILOT_HIDE ))
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
 * @luafunc target
 */
static int pilotL_target( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;
   /* Must be valid. */
   if (pilot_getTarget(p) == NULL)
      return 0;
   /* Push target. */
   lua_pushpilot(L, p->target);
   return 1;
}

/**
 * @brief Sets the pilot target of the pilot.
 *
 *    @luatparam Pilot p Pilot to get target of.
 *    @luatparam Pilot|nil t Pilot to set the target to or nil to set no target.
 * @luafunc setTarget
 */
static int pilotL_setTarget( lua_State *L )
{
   unsigned int t;
   Pilot *p = luaL_validpilot(L,1);
   if (lua_isnoneornil(L,2))
      t = p->id;
   else
      t = luaL_validpilot(L,2)->id;
   if (pilot_isPlayer(p))
      player_targetSet( t );
   else
      pilot_setTarget( p, t );
   return 0;
}

/**
 * @brief Gets the asteroid target of the pilot.
 *
 * @usage target = p:targetAsteroid()
 *
 *    @luatparam Pilot p Pilot to get asteroid target of.
 *    @luatreturn table|nil nil if no asteroid is selected, otherwise a table with information about the selected asteroid.
 * @luafunc targetAsteroid
 */
static int pilotL_targetAsteroid( lua_State *L )
{
   LuaAsteroid_t la;
   const Pilot *p = luaL_validpilot(L,1);
   if (p->nav_asteroid < 0)
      return 0;

   la.parent = p->nav_anchor;
   la.id = p->nav_asteroid;
   lua_pushasteroid(L, la);
   return 1;
}

/**
 * @brief Sets the pilot's asteroid target.
 *
 *    @luatparam Pilot p Pilot to set asteroid target of.
 *    @luatparam Asteroid a Asteroid to set pilot's target to.
 * @luafunc setTargetAsteroid
 */
static int pilotL_setTargetAsteroid( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   LuaAsteroid_t *la = luaL_checkasteroid(L,2);

   /* Set the target asteroid. */
   p->nav_anchor = la->parent;
   p->nav_asteroid = la->id;

   /* Untarget pilot. */
   p->target = p->id;
   p->ptarget = NULL;

   return 0;
}

/**
 * @brief Checks to see if a target pilot is in range of a pilot.
 *
 * @usage detected, scanned = p:inrange( target )
 *
 *    @luatparam Pilot p Pilot to see if another pilot is in range.
 *    @luatparam Pilot target Target pilot.
 *    @luatreturn boolean True if the pilot is visible at all.
 *    @luatreturn boolean True if the pilot is visible and well-defined (not fuzzy)
 * @luafunc inrange
 */
static int pilotL_inrange( lua_State *L )
{
   /* Parse parameters. */
   const Pilot *p = luaL_validpilot(L,1);
   const Pilot *t = luaL_validpilot(L,2);

   /* Check if in range. */
   int ret = pilot_inRangePilot( p, t, NULL );
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
 * @brief Checks to see if an asteroid is in range of a pilot.
 *
 *    @luatparam Pilot p Pilot checking to see if an asteroid is in range.
 *    @luatparam Asteroid a Asteroid to check to see if is in range.
 *    @luatreturn boolean true if in range, false otherwise.
 * @luafunc inrangeAsteroid
 */
static int pilotL_inrangeAsteroid( lua_State *L )
{
   /* Parse parameters. */
   const Pilot *p = luaL_validpilot(L,1);
   LuaAsteroid_t *la = luaL_checkasteroid(L,2);

   /* Check if in range. */
   lua_pushboolean(L, pilot_inRangeAsteroid( p, la->id, la->parent ));
   return 1;
}

/**
 * @brief Checks to see if a pilot is done scanning its target.
 *
 *    @luatreturn boolean True if the pilot has finished scanning their target.
 * @luafunc scandone
 */
static int pilotL_scandone( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L, pilot_ewScanCheck( p ) );
   return 1;
}

/**
 * @brief Checks to see if pilot is with player.
 *
 *    @luatparam Pilot p Pilot to check to see if is with player.
 *    @luatreturn boolean true if pilot is with player.
 * @luafunc withPlayer
 */
static int pilotL_withPlayer( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L, pilot_isWithPlayer(p));
   return 1;
}

/**
 * @brief Gets the nav target of the pilot.
 *
 * @usage spob, hyperspace = p:nav()
 *
 *    @luatparam Pilot p Pilot to get nav info of.
 *    @luatreturn Spob|nil The pilot's spob target.
 *    @luatreturn System|nil The pilot's hyperspace target.
 * @luafunc nav
 */
static int pilotL_nav( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;

   /* Get spob target. */
   if (p->nav_spob < 0)
      lua_pushnil(L);
   else
      lua_pushspob( L, cur_system->spobs[ p->nav_spob ]->id );

   /* Get hyperspace target. */
   if (p->nav_hyperspace < 0)
      lua_pushnil(L);
   else {
      LuaSystem ls = cur_system->jumps[ p->nav_hyperspace ].targetid;
      lua_pushsystem( L, ls );
   }

   return 2;
}

/**
 * @brief Gets the nav spob target of the pilot.
 *
 *    @luatparam Pilot p Pilot to get nav info of.
 *    @luatreturn Spob|nil The pilot's spob target.
 * @luafunc navSpob
 */
static int pilotL_navSpob( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;

   /* Get spob target. */
   if (p->nav_spob < 0)
      lua_pushnil(L);
   else
      lua_pushspob( L, cur_system->spobs[ p->nav_spob ]->id );

   return 1;
}

/**
 * @brief Gets the nav jump target of the pilot.
 *
 *    @luatparam Pilot p Pilot to get nav info of.
 *    @luatreturn Jump|nil The pilot's hyperspace target.
 * @luafunc navJump
 */
static int pilotL_navJump( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (p->target == 0)
      return 0;

   /* Get hyperspace target. */
   if (p->nav_hyperspace < 0)
      lua_pushnil(L);
   else {
      LuaJump lj;
      lj.srcid = cur_system->id;
      lj.destid = cur_system->jumps[ p->nav_hyperspace ].targetid;
      lua_pushjump( L, lj );
   }

   return 1;
}

/**
 * @brief Gets the ID (number from 1 to 10) of the current active weapset.
 *
 * @usage set_id = p:weapsetActive() -- A number from 1 to 10
 *
 *    @luatparam Pilot p Pilot to get active weapset ID of.
 *    @luatparam number current active weapset ID.
 *
 * @luafunc weapsetActive
 */
static int pilotL_weapsetActive( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->active_set + 1 );
   return 1;
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
 *  <li> lockon: Lock-on [0:1] for seeker weapons or nil if not applicable. </li>
 *  <li> in_arc: Whether or not the target is in targeting arc or nil if not applicable. </li>
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
 * for i, w in ipairs(ws) do
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
 * @luafunc weapset
 */
static int pilotL_weapset( lua_State *L )
{
   Pilot *p, *target;
   int k, n;
   PilotWeaponSetOutfit *po_list;
   PilotOutfitSlot *slot;
   const Outfit *o;
   double delay, firemod, enermod, t;
   int id, all, level;
   int is_lau, is_fb;
   const Damage *dmg;
   int has_beamid;

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
   po_list = all ? NULL : pilot_weapSetList( p, id );
   n = all ? array_size(p->outfits) : array_size(po_list);

   k = 0;
   lua_newtable(L);
   for (int j=0; j<=PILOT_WEAPSET_MAX_LEVELS; j++) {
      /* Level to match. */
      int level_match = (j==PILOT_WEAPSET_MAX_LEVELS) ? -1 : j;

      /* Iterate over weapons. */
      for (int i=0; i<n; i++) {
         /* Get base look ups. */
         slot = all ?  p->outfits[i] : p->outfits[ po_list[i].slotid ];
         o    = slot->outfit;
         if (o == NULL)
            continue;
         is_lau   = outfit_isLauncher(o);
         is_fb    = outfit_isFighterBay(o);

         /* Must be valid weapon. */
         if (all && !(outfit_isBolt(o) || outfit_isBeam(o)
               || is_lau || is_fb))
            continue;

         level    = slot->level;

         /* Must match level. */
         if (level != level_match)
            continue;

         /* Must be weapon. */
         if (outfit_isMod(o) ||
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
            has_beamid = (slot->u.beamid > 0);
            if (has_beamid)
               lua_pushnumber(L, 0.);
            else {
               delay = (slot->timer / outfit_delay(o)) * firemod;
               lua_pushnumber( L, CLAMP( 0., 1., 1. -delay ) );
            }
            lua_rawset(L,-3);

            /* When firing, slot->timer represents the remaining duration. */
            lua_pushstring(L,"charge");
            if (has_beamid)
               lua_pushnumber(L, CLAMP( 0., 1., slot->timer / o->u.bem.duration ) );
            else
               lua_pushnumber( L, CLAMP( 0., 1., 1. -delay ) );
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

         /* Ammo quantity absolute. */
         if (is_lau || is_fb) {
            lua_pushstring(L,"left");
            lua_pushnumber( L, slot->u.ammo.quantity );
            lua_rawset(L,-3);

         /* Ammo quantity relative. */
            lua_pushstring(L,"left_p");
            lua_pushnumber( L, (double)slot->u.ammo.quantity / (double)pilot_maxAmmoO(p,slot->outfit) );
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
               lua_pushnumber(L, pilot_ewWeaponTrack( p, target, slot->outfit->u.blt.trackmin, slot->outfit->u.blt.trackmax ));
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

static int luaL_checkweapset( lua_State *L, int idx )
{
   int ws = luaL_checkinteger(L,idx)-1;
   if ((ws < 0) || (ws > 9))
      NLUA_ERROR(L,_("Invalid weapon set '%d'!"),idx);
   return ws;
}

static PilotOutfitSlot *luaL_checkslot( lua_State *L, Pilot *p, int idx )
{
   if (lua_isnumber(L,idx)) {
      const int slotid = lua_tointeger(L,idx);
      if ((slotid < 1) || (slotid > array_size(p->outfits))) {
         NLUA_ERROR(L,_("Pilot '%s' with ship '%s' does not have a slot with id '%d'!"), p->name, _(p->ship->name), slotid );
         return NULL;
      }
      /* We have to convert from "Lua IDs" to "C" ids by subtracting 1. */
      return p->outfits[ slotid-1 ];
   }

   const char *slotname = luaL_checkstring(L,idx);
   PilotOutfitSlot *s = pilot_getSlotByName( p, slotname );
   if (s==NULL) {
      WARN(_("Pilot '%s' with ship '%s' does not have named slot '%s'!"), p->name, _(p->ship->name), slotname );
      return NULL;
   }
   return s;
}

/**
 * @brief Sets the type of a weapon set for a pilot.
 *
 *    @luatparam Pilot p Pilot to set weapon set type of.
 *    @luatparam integer id ID of the weapon set as shown in game (from 0 to 9).
 *    @luatparam string type Type of the weapon set. Can be either "change", "instant", or "toggle".
 * @luafunc weapsetType
 */
static int pilotL_weapsetType( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int id = luaL_checkweapset(L,2);
   const char *type = luaL_checkstring(L,3);
   int typeid;
   if (strcmp(type,"change")==0)
      typeid = WEAPSET_TYPE_CHANGE;
   else if (strcmp(type,"instant")==0)
      typeid = WEAPSET_TYPE_ACTIVE;
   else if (strcmp(type,"toggle")==0)
      typeid = WEAPSET_TYPE_TOGGLE;
   else {
      NLUA_ERROR(L,_("Invalid weapon set type '%s'!"),type);
      return 0;
   }
   pilot_weapSetType( p, id, typeid );
   return 0;
}

/**
 * @brief Adds an outfit to a pilot's weapon set.
 *
 * Note that this can change the structure of the weapon set. For example, adding a utility/structural outfit to a weapon weapon set will remove all weapons and change it to be of type "active".
 *
 *    @luatparam Pilot p Pilot to remove weapon from weapon set.
 *    @luatparam integer id ID of the weapon set as shown in game (from 0 to 9).
 *    @luatparam string|integer slot Slot to add to weapon set. Can be passed by either id or name.
 * @luafunc weapsetAdd
 */
static int pilotL_weapsetAdd( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int id = luaL_checkweapset(L,2);
   PilotOutfitSlot *o = luaL_checkslot(L,p,3);
   int level = luaL_optinteger(L,4,0);
   /* Follows same logic as equipment_mouseColumn (equipment.c) */
   if ((o->sslot->slot.type==OUTFIT_SLOT_UTILITY) ||
         (o->sslot->slot.type==OUTFIT_SLOT_UTILITY)) {
      pilot_weapSetRmSlot( p, id, OUTFIT_SLOT_WEAPON );
      pilot_weapSetAdd( p, id, o, level );
      pilot_weapSetType( p, id, WEAPSET_TYPE_TOGGLE );
   }
   else {
      pilot_weapSetRmSlot( p, id, OUTFIT_SLOT_STRUCTURE );
      pilot_weapSetRmSlot( p, id, OUTFIT_SLOT_UTILITY );
      if (pilot_weapSetTypeCheck( p, id) == WEAPSET_TYPE_CHANGE)
         pilot_weapSetType( p, id, WEAPSET_TYPE_CHANGE );
      else {
         pilot_weapSetType( p, id, WEAPSET_TYPE_ACTIVE );
         level = 0;
      }
      pilot_weapSetAdd( p, id, o, level );
   }
   return 0;
}

/**
 * @brief Removes an outfit from a pilot's weapon set.
 *
 *    @luatparam Pilot p Pilot to remove weapon from weapon set.
 *    @luatparam integer id ID of the weapon set as shown in game (from 0 to 9).
 *    @luatparam string|integer slot Slot to remove from weapon set. Can be passed by either id or name.
 * @luafunc weapsetRm
 */
static int pilotL_weapsetRm( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int id = luaL_checkweapset(L,2);
   PilotOutfitSlot *o = luaL_checkslot(L,p,3);
   pilot_weapSetRm( p, id, o );
   return 0;
}

/**
 * @brief Cleans up a weapon set. This removes all properties of the weapon set and resets it.
 *
 *    @luatparam Pilot p Pilot to remove weapon from weapon set.
 *    @luatparam integer id ID of the weapon set as shown in game (from 0 to 9).
 * @luafunc weapsetCleanup
 */
static int pilotL_weapsetCleanup( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int id = luaL_checkweapset(L,2);
   pilot_weapSetCleanup( p, id );
   return 0;
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
 * @luafunc weapsetHeat
 */
static int pilotL_weapsetHeat( lua_State *L )
{
   Pilot *p;
   PilotWeaponSetOutfit *po_list;
   int n, id, all;
   double heat, heat_mean, heat_peak, nweapons;

   /* Defaults. */
   heat_mean = 0.;
   heat_peak = 0.;
   nweapons  = 0;

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
   po_list = all ? NULL : pilot_weapSetList( p, id );
   n = all ? array_size(p->outfits) : array_size(po_list);

   for (int j=0; j<=PILOT_WEAPSET_MAX_LEVELS; j++) {
      /* Level to match. */
      int level_match = (j==PILOT_WEAPSET_MAX_LEVELS) ? -1 : j;

       /* Iterate over weapons. */
      for (int i=0; i<n; i++) {
         int level;
         /* Get base look ups. */
         PilotOutfitSlot *slot = all ?  p->outfits[i] : p->outfits[ po_list[i].slotid ];
         const Outfit *o = slot->outfit;
         if (o == NULL)
            continue;

         level = all ?  slot->level : po_list[i].level;

         /* Must match level. */
         if (level != level_match)
            continue;

         /* Must be weapon. */
         if (outfit_isMod(o) ||
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
 * @brief Sets whether a pilot's weapon set does inrange checks.
 *
 *    @luatparam Pilot p Pilot to get weapset weapon of.
 *    @luatparam[opt=nil] number id ID of the weapon set to set inrange check status, or nil for all.
 *    @luatparam boolean docheck Whether or not to do inrange checks.
 * @luafunc weapsetSetInrange
 */
static int pilotL_weapsetSetInrange( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int id = luaL_optinteger(L,2,-1);
   int inrange = lua_toboolean(L,3);
   if (id<0) {
      for (int i=0; i<PILOT_WEAPON_SETS; i++)
         pilot_weapSetInrange( p, i, inrange );
   }
   else
      pilot_weapSetInrange( p, id, inrange );
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
 * for i, o in ipairs(act_outfits) do
 *    print( "Name: " .. o.name )
 *    print( "State: " .. o.state )
 * end
 * @endcode
 *
 * @usage act_outfits = p:actives() -- Gets the table of active outfits
 *
 *    @luatparam Pilot p Pilot to get active outfits of.
 *    @luatparam[opt=false] boolean sort Whether or not to sort the otufits.
 *    @luatreturn table The table with each active outfit's information.
 * @luafunc actives
 */
static int pilotL_actives( lua_State *L )
{
   const Pilot *p;
   int k, sort;
   PilotOutfitSlot **outfits;
   const char *str;
   double d;

   /* Parse parameters. */
   p     = luaL_validpilot(L,1);
   sort  = lua_toboolean(L,2);

   k = 0;
   lua_newtable(L);

   if (sort) {
      outfits = array_copy( PilotOutfitSlot*, p->outfits );
      qsort( outfits, array_size(outfits), sizeof(PilotOutfitSlot*), outfit_compareActive );
   }
   else
      outfits  = p->outfits;

   for (int i=0; i<array_size(outfits); i++) {
      /* Get active outfits. */
      PilotOutfitSlot *o = outfits[i];
      if (o->outfit == NULL)
         continue;
      if (!o->active)
         continue;
      if (!outfit_isMod(o->outfit) &&
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
            if (!outfit_isMod(o->outfit) || o->outfit->lua_env == LUA_NOREF)
               d = 1.; /* TODO add warmup stuff to normal active outfits (not sure if necessary though. */
            else
               d = o->progress;
            lua_pushstring(L,"warmup");
            lua_pushnumber(L, d );
            lua_rawset(L,-3);
            break;
         case PILOT_OUTFIT_ON:
            str = "on";
            if (!outfit_isMod(o->outfit) || o->outfit->lua_env == LUA_NOREF) {
               d = outfit_duration(o->outfit);
               if (d==0.)
                  d = 1.;
               else if (!isinf(o->stimer))
                  d = o->stimer / d;
            }
            else
               d = o->progress;
            lua_pushstring(L,"duration");
            lua_pushnumber(L, d );
            lua_rawset(L,-3);
            break;
         case PILOT_OUTFIT_COOLDOWN:
            str = "cooldown";
            if (!outfit_isMod(o->outfit) || o->outfit->lua_env == LUA_NOREF) {
               d = outfit_cooldown(o->outfit);
               if (d==0.)
                  d = 0.;
               else if (!isinf(o->stimer))
                  d = o->stimer / d;
            }
            else
               d = o->progress;
            lua_pushstring(L,"cooldown");
            lua_pushnumber(L, d );
            lua_rawset(L,-3);
            break;
         default:
            str = "unknown";
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
      array_free(outfits);

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
 *    @luatparam[opt=nil] string What slot type to get outfits of. Can be either nil, "weapon", "utility", "structure", "intrinsic", or "all". nil implies returning all non-intrinsic outfits.
 *    @luatparam[opt=false] boolean skip_locked Whether or not locked outfits should be ignored.
 *    @luatreturn table The outfits of the pilot in an ordered list.
 * @luafunc outfitsList
 */
static int pilotL_outfitsList( lua_State *L )
{
   int normal = 1;
   int intrinsics = 0;
   const Pilot *p = luaL_validpilot(L,1);
   const char *type = luaL_optstring(L,2,NULL);
   int skip_locked = lua_toboolean(L,3);
   OutfitSlotType ost = OUTFIT_SLOT_NULL;

   /* Get type. */
   if (type != NULL) {
      if (strcmp(type,"all")==0)
         intrinsics = 1;
      else if (strcmp(type,"structure")==0)
         ost = OUTFIT_SLOT_STRUCTURE;
      else if (strcmp(type,"utility")==0)
         ost = OUTFIT_SLOT_UTILITY;
      else if (strcmp(type,"weapon")==0)
         ost = OUTFIT_SLOT_WEAPON;
      else if (strcmp(type,"intrinsic")==0) {
         intrinsics = 1;
         normal = 0;
      }
      else
         NLUA_ERROR(L,_("Unknown slot type '%s'"), type);
   }

   lua_newtable( L );
   int j = 1;
   if (normal) {
      for (int i=0; i<array_size(p->outfits); i++) {
         /* Get outfit. */
         if (p->outfits[i]->outfit == NULL)
            continue;

         /* Only match specific type. */
         if ((ost!=OUTFIT_SLOT_NULL) && (p->outfits[i]->outfit->slot.type!=ost))
            continue;

         /* Skip locked. */
         if (skip_locked && p->outfits[i]->sslot->locked)
            continue;

         /* Set the outfit. */
         lua_pushoutfit( L, p->outfits[i]->outfit );
         lua_rawseti( L, -2, j++ );
      }
   }
   if (intrinsics) {
      for (int i=0; i<array_size(p->outfit_intrinsic); i++) {
         lua_pushoutfit( L, p->outfit_intrinsic[i].outfit );
         lua_rawseti( L, -2, j++ );
      }
   }

   return 1;
}

/**
 * @brief Gets a mapping of outfit slot IDs and outfits of a pilot.
 *
 * @note The index value can be used as a slot identifier.
 *
 *    @luatparam Pilot p Pilot to get outfits of.
 *    @luatreturn table Ordered table of outfits. If an outfit is not equipped at slot it sets the value to false.
 * @luafunc outfits
 */
static int pilotL_outfits( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_newtable( L );
   for (int i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == NULL)
         lua_pushboolean( L, 0 );
      else
         lua_pushoutfit( L, p->outfits[i]->outfit );
      lua_rawseti( L, -2, i+1 );
   }
   return 1;
}

/**
 * @brief Equips a pilot with a set of outfits.
 *
 *    @luatparam Pilot p Pilot to set equipment of.
 *    @luatparam table o Table of outfits to equip the pilot (should be likely taken from pilot.outfits). The key should be the slot id and the value should be the outfit or false if there is no outfit in that slot.
 *    @luatreturn boolean If all the outfits were equipped successfully or not.
 * @luafunc outfits
 * @see outfits
 */
static int pilotL_outfitsEquip( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int ret = 0;
   /* Process outputs. */
   for (int i=1; ; i++) {
      const Outfit *o;
      PilotOutfitSlot *s;

      lua_rawgeti(L,2,i);
      if (lua_isnil(L,-1)) {
         lua_pop(L,1);
         break;
      }

      if (i > array_size(p->outfits)) {
         WARN(_("Trying to equip more outfits than slots available on pilot '%s'!"), p->name);
         lua_pop(L,1);
         break;
      }

      /* No outfit. */
      if (!lua_toboolean(L,-1))
         continue;

      o = luaL_validoutfit(L,-1);
      s = p->outfits[i-1];
      ret |= pilot_outfitAddSlot( p, o, s, 1, 1 );

      lua_pop(L,1);
   }
   lua_pushboolean(L,!ret);
   return 1;
}

/**
 * @brief Gets a pilot's outfit by ID.
 *
 *    @luatparam Pilot p Pilot to get outfit of.
 *    @luatparam number id ID of the outfit to get.
 *    @luatreturn Outfit|nil Outfit equipped in the slot or nil otherwise.
 * @luafunc outfitGet
 */
static int pilotL_outfitGet( lua_State *L )
{
   /* Parse parameters */
   const Pilot *p  = luaL_validpilot(L,1);
   int id    = luaL_checkinteger(L,2)-1;
   if (id < 0 || id >= array_size(p->outfits))
      NLUA_ERROR(L, _("Pilot '%s' outfit ID '%d' is out of range!"), p->name, id);

   if (p->outfits[id]->outfit != NULL)
      lua_pushoutfit( L, p->outfits[id]->outfit );
   else
      lua_pushnil( L );
   return 1;
}

/**
 * @brief Toggles an outfit.
 *
 *    @luatparam Pilot p Pilot to toggle outfit of.
 *    @luatparam integer id ID of the pilot outfit.
 *    @luatparam[opt=false] boolean activate Whether or not to activate or deactivate the outfit.
 * @luafunc outfitToggle
 */
static int pilotL_outfitToggle( lua_State *L )
{
   int isstealth, n = 0;
   Pilot *p = luaL_validpilot(L,1);
   int id   = luaL_checkinteger(L,2)-1;
   int activate = lua_toboolean(L,3);
   if (id < 0 || id >= array_size(p->outfits))
      NLUA_ERROR(L, _("Pilot '%s' outfit ID '%d' is out of range!"), p->name, id);
   PilotOutfitSlot *po = p->outfits[id];
   const Outfit *o = po->outfit;

   /* Ignore NULL outfits. */
   if (o == NULL)
      return 0;

   /* Can't do a thing. */
   if ((pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
      return 0;

   if ((activate && (po->state != PILOT_OUTFIT_OFF)) ||
         (!activate && (po->state != PILOT_OUTFIT_ON)))
      return 0;

   if (activate)
      n = pilot_outfitOn( p, po );
   else
      n = pilot_outfitOff( p, po );

   isstealth = pilot_isFlag( p, PILOT_STEALTH );
   if (n>0 && isstealth)
      pilot_destealth( p ); /* pilot_destealth should run calcStats already. */
   else if (n>0 || pilotoutfit_modified)
      pilot_calcStats( p );

   lua_pushboolean(L,n);
   return 1;
}

/**
 * @brief Sees if an outfit is ready to use.
 *
 *    @luatparam Pilot p Pilot to toggle outfit of.
 *    @luatparam integer id ID of the pilot outfit.
 *    @luatreturn boolean Whether or not the outfit is ready to use.
 * @luafunc outfitReady
 */
static int pilotL_outfitReady( lua_State *L )
{
   /* Parse parameters */
   const Pilot *p = luaL_validpilot(L,1);
   int id    = luaL_checkinteger(L,2)-1;
   if (id < 0 || id >= array_size(p->outfits))
      NLUA_ERROR(L, _("Pilot '%s' outfit ID '%d' is out of range!"), p->name, id);

   if (p->outfits[id]->outfit != NULL)
      lua_pushboolean( L, p->outfits[id]->state==PILOT_OUTFIT_OFF );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Changes the pilot's name.
 *
 * @usage p:rename( _("Black Beard") )
 *
 *    @luatparam Pilot p Pilot to change name of.
 *    @luatparam string name Name to change to.
 * @luafunc rename
 */
static int pilotL_rename( lua_State *L )
{
   /* Parse parameters */
   Pilot *p         = luaL_validpilot(L,1);
   const char *name = luaL_checkstring(L,2);

   /* Change name. */
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
 * @luafunc pos
 */
static int pilotL_position( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushvector(L, p->solid.pos);
   return 1;
}

/**
 * @brief Gets the pilot's velocity.
 *
 * @usage vel = p:vel()
 *
 *    @luatparam Pilot p Pilot to get the velocity of.
 *    @luatreturn Vec2 The pilot's current velocity.
 * @luafunc vel
 */
static int pilotL_velocity( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushvector(L, p->solid.vel);
   return 1;
}

/**
 * @brief Checks to see if a pilot is stopped.
 *
 * @usage if p:isStopped() then ... end
 *
 *    @luatparam Pilot p Pilot to get the velocity of.
 *    @luatreturn boolean Whether the pilot is stopped or not.
 *    @luafunc isStopped
 */
static int pilotL_isStopped( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L,(VMOD(p->solid.vel) < MIN_VEL_ERR));
   return 1;
}

/**
 * @brief Gets the pilot's evasion.
 *
 * @usage d = p:evasion()
 *
 *    @luatparam Pilot p Pilot to get the evasion of.
 *    @luatreturn number The pilot's current evasion value.
 * @luafunc evasion
 */
static int pilotL_evasion( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->ew_evasion );
   return 1;
}

/**
 * @brief Gets the pilot's direction.
 *
 * @usage d = p:dir()
 *
 *    @luatparam Pilot p Pilot to get the direction of.
 *    @luatreturn number The pilot's current direction as a number (in radians).
 * @luafunc dir
 */
static int pilotL_dir( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->solid.dir );
   return 1;
}

/**
 * @brief Gets the temperature of a pilot.
 *
 * @usage t = p:temp()
 *
 *    @luatparam Pilot p Pilot to get temperature of.
 *    @luatreturn number The pilot's current temperature (in kelvin).
 * @luafunc temp
 */
static int pilotL_temp( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->heat_T );
   return 1;
}

/**
 * @brief Gets the mass of a pilot.
 *
 * @usage m = p:mass()
 *
 *    @luatparam Pilot p Pilot to get mass of.
 *    @luatreturn number The pilot's current mass (in tonnes).
 * @luafunc mass
 */
static int pilotL_mass( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->solid.mass );
   return 1;
}

/**
 * @brief Gets the thrust of a pilot.
 *
 *    @luatparam Pilot p Pilot to get thrust of.
 *    @luatreturn number The pilot's current thrust.
 * @luafunc thrust
 */
static int pilotL_thrust( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->thrust/p->solid.mass );
   return 1;
}

/**
 * @brief Gets the speed of a pilot.
 *
 *    @luatparam Pilot p Pilot to get speed of.
 *    @luatreturn number The pilot's current speed.
 * @luafunc speed
 */
static int pilotL_speed( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->speed );
   return 1;
}

/**
 * @brief Gets the maximum speed of a pilot.
 *
 *    @luatparam Pilot p Pilot to get maximum speed of.
 *    @luatreturn number The pilot's current maximum speed.
 * @luafunc speedMax
 * @see speed
 */
static int pilotL_speed_max( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, solid_maxspeed( &p->solid, p->speed, p->thrust ) );
   return 1;
}

/**
 * @brief Gets the turn of a pilot.
 *
 *    @luatparam Pilot p Pilot to get turn of.
 *    @luatreturn number The pilot's current turn (in degrees per second).
 * @luafunc turn
 */
static int pilotL_turn( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, p->turn * 180. / M_PI ); /* TODO use radians. */
   return 1;
}

/**
 * @brief Gets the pilot's faction.
 *
 * @usage f = p:faction()
 *
 *    @luatparam Pilot p Pilot to get the faction of.
 *    @luatreturn Faction The faction of the pilot.
 * @luafunc faction
 */
static int pilotL_faction( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushfaction(L,p->faction);
   return 1;
}

/**
 * @brief Checks to see if two pilots are enemies.
 *
 *    @luatparam Pilot p Pilot to check.
 *    @luatparam Pilot t Target pilot to check.
 *    @luatreturn boolean true if both p and t are enemies, false otherwise.
 * @luafunc areEnemies
 */
static int pilotL_areEnemies( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   const Pilot *t = luaL_validpilot(L,2);
   lua_pushboolean(L,pilot_areEnemies(p,t));
   return 1;
}

/**
 * @brief Checks to see if two pilots are allies.
 *
 *    @luatparam Pilot p Pilot to check.
 *    @luatparam Pilot t Target pilot to check.
 *    @luatreturn boolean true if both p and t are allies, false otherwise.
 * @luafunc areAllies
 */
static int pilotL_areAllies( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   const Pilot *t = luaL_validpilot(L,2);
   lua_pushboolean(L,pilot_areAllies(p,t));
   return 1;
}

/**
 * @brief Checks the pilot's spaceworthiness
 *
 * Message can be non-null even if spaceworthy.
 *
 * @usage spaceworthy = p:spaceworthy()
 *
 *    @luatparam Pilot p Pilot to get the spaceworthy status of.
 *    @luatreturn boolean Whether the pilot's ship is spaceworthy.
 *    @luatreturn string Reason why the pilot is not spaceworthy.
 * @luafunc spaceworthy
 */
static int pilotL_spaceworthy( lua_State *L )
{
   char message[STRMAX_SHORT];
   const Pilot *p = luaL_validpilot(L,1);
   int worthy = !pilot_reportSpaceworthy( p, message, sizeof(message) );
   lua_pushboolean( L, worthy );
   lua_pushstring( L, message );
   return 2;
}

/**
 * @brief Sets the pilot's position.
 *
 * @usage p:setPos( vec2.new( 300, 200 ) )
 *
 *    @luatparam Pilot p Pilot to set the position of.
 *    @luatparam Vec2 pos Position to set.
 * @luafunc setPos
 */
static int pilotL_setPosition( lua_State *L )
{
   /* Parse parameters */
   Pilot *p   = luaL_validpilot(L,1);
   vec2 *vec  = luaL_checkvector(L,2);

   /* Insert skip in trail. */
   pilot_sample_trails( p, 1 );

   /* Warp pilot to new position. */
   p->solid.pos = *vec;

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
 * @luafunc setVel
 */
static int pilotL_setVelocity( lua_State *L )
{
   /* Parse parameters */
   Pilot *p  = luaL_validpilot(L,1);
   vec2 *vec = luaL_checkvector(L,2);

   /* Warp pilot to new position. */
   p->solid.vel = *vec;
   return 0;
}

/**
 * @brief Sets the pilot's direction.
 *
 * @note Right is 0, top is math.pi/2, left is math.pi, bottom is 3*math.pi/2.
 *
 * @usage p:setDir( math.pi/2 )
 *
 *    @luatparam Pilot p Pilot to set the direction of.
 *    @luatparam number dir Direction to set, in radians.
 * @luafunc setDir
 */
static int pilotL_setDir( lua_State *L )
{
   /* Parse parameters */
   Pilot *p  = luaL_validpilot(L,1);
   double d  = luaL_checknumber(L,2);

   /* Set direction. */
   p->solid.dir = fmodf( d, 2*M_PI );
   if (p->solid.dir < 0.)
      p->solid.dir += 2*M_PI;

   return 0;
}

/**
 * @brief Makes the pilot broadcast a message.
 *
 * @usage p:broadcast(_("Mayday! Requesting assistance!"))
 * @usage p:broadcast(_("Help!"), true ) -- Will ignore interference
 * @usage pilot.broadcast( _("Message Buoy"), _("Important annauncement") ) -- Messages player ignoring interference
 *
 *    @luatparam Pilot|string p Pilot to broadcast the message, or string to use as a fictional pilot name. In the case of a string, interference is always ignored, and instead of the ignore_int parameter, a colour character such as 'F' or 'H' can be optionally passed.
 *    @luatparam string msg Message to broadcast.
 *    @luatparam[opt=false] boolean ignore_int Whether or not it should ignore interference.
 * @luafunc broadcast
 */
static int pilotL_broadcast( lua_State *L )
{
   /* Parse parameters. */
   if (lua_isstring(L,1)) {
      const char *s   = luaL_checkstring(L,1);
      const char *msg = luaL_checkstring(L,2);
      const char *col = luaL_optstring(L,3,NULL);

      player_message( _("#%cBroadcast %s>#0 \"%s\""), ((col==NULL)?'N':col[0]), s, msg );
      if (player.p)
         pilot_setCommMsg( player.p, msg );
   }
   else {
      Pilot *p        = luaL_validpilot(L,1);
      const char *msg = luaL_checkstring(L,2);
      int ignore_int  = lua_toboolean(L,3);

      /* Broadcast message. */
      pilot_broadcast( p, msg, ignore_int );
   }

   return 0;
}

/**
 * @brief Sends a message to the target or player if no target is passed.
 *
 * @usage p:comm( _("How are you doing?") ) -- Messages the player
 * @usage p:comm( _("You got this?"), true ) -- Messages the player ignoring interference
 * @usage p:comm( target, _("Heya!") ) -- Messages target
 * @usage p:comm( target, _("Got this?"), true ) -- Messages target ignoring interference
 * @usage pilot.comm( _("Message Buoy"), _("Important information just for you!") ) -- Messages player ignoring interference
 *
 *    @luatparam Pilot|string p Pilot to message the player, or string to use as a fictional pilot name. In the case of a string, interference is always ignored, and instead of ignore_int, a colour character such as 'F' or 'H' can be passed.
 *    @luatparam Pilot target Target to send message to.
 *    @luatparam string msg Message to send.
 *    @luatparam[opt=false] boolean|colour param1 Whether or not it should ignore interference in the case a pilot is being used, otherwise it is a colour string such as 'N' that can be used to colour the text..
 *    @luatparam[opt=false] boolean raw Whether or not to just display the raw text without quotation marks instead of a "message".
 * @luafunc comm
 */
static int pilotL_comm( lua_State *L )
{
   if (lua_isstring(L,1)) {
      const char *s;
      LuaPilot target;
      const char *msg, *col;
      int raw;

      if (player.p==NULL)
         return 0;

      /* Parse parameters. */
      s = luaL_checkstring(L,1);
      if (lua_isstring(L,2)) {
         msg   = luaL_checkstring(L,2);
         col   = luaL_optstring(L,3,NULL);
         raw   = lua_toboolean(L,4);
      }
      else {
         target = luaL_checkpilot(L,2);
         if (target != player.p->id)
            return 0;
         msg   = luaL_checkstring(L,3);
         col   = luaL_optstring(L,4,NULL);
         raw   = lua_toboolean(L,5);
      }

      /* Broadcast message. */
      if (raw)
         player_message( _("#%c%s>#0 %s"), ((col==NULL)?'N':col[0]), s, msg );
      else
         player_message( _("#%cComm %s>#0 \"%s\""), ((col==NULL)?'N':col[0]), s, msg );
      if (player.p)
         pilot_setCommMsg( player.p, msg );
   }
   else {
      Pilot *p;
      LuaPilot target;
      const char *msg;
      int ignore_int, raw;

      /* Parse parameters. */
      p = luaL_validpilot(L,1);
      if (lua_isstring(L,2)) {
         target = 0;
         msg   = luaL_checkstring(L,2);
         ignore_int = lua_toboolean(L,3);
         raw = lua_toboolean(L,4);
      }
      else {
         target = luaL_checkpilot(L,2);
         msg   = luaL_checkstring(L,3);
         ignore_int = lua_toboolean(L,4);
         raw = lua_toboolean(L,5);
      }

      if (player.p==NULL)
         return 0;

      if (!ignore_int && !pilot_inRangePilot( player.p, p, NULL ))
         return 0;

      /* Broadcast message. */
      if (target == 0 || target == PLAYER_ID) {
         char c = pilot_getFactionColourChar( p );
         if (raw)
            player_message( _("#%c%s>#0 %s"), c, p->name, msg );
         else
            player_message( _("#%cComm %s>#0 \"%s\""), c, p->name, msg );

         /* Set comm message. */
         pilot_setCommMsg( p, msg );
      }
   }
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
 * @luafunc setFaction
 */
static int pilotL_setFaction( lua_State *L )
{
   /* Parse parameters. */
   Pilot *p = luaL_validpilot(L,1);
   int fid = luaL_validfaction(L,2);

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
 * @luafunc setHostile
 */
static int pilotL_setHostile( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_isnone(L,2))
      state = 1;
   else
      state = lua_toboolean(L, 2);

   /* Set as hostile. */
   if (state)
      pilot_setHostile(p);
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
 * @luafunc setFriendly
 */
static int pilotL_setFriendly( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_isnone(L,2))
      state = 1;
   else
      state = lua_toboolean(L, 2);

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
 * @luafunc setInvincible
 */
static int pilotL_setInvincible( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_INVINCIBLE );
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
 * @luafunc setInvincPlayer
 */
static int pilotL_setInvincPlayer( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_INVINC_PLAYER );
}

/**
 * @brief Sets the pilot's hide status.
 *
 * A hidden pilot is neither updated nor drawn. It stays frozen in time
 *  until the hide is lifted.
 *
 * @usage p:setHide() -- p will disappear
 * @usage p:setHide(true) -- p will disappear
 * @usage p:setHide(false) -- p will appear again
 *
 *    @luatparam Pilot p Pilot to set hidden status of.
 *    @luatparam boolean state State to set hide.
 * @luafunc setHide
 */
static int pilotL_setHide( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_HIDE );
}

/**
 * @brief Sets the pilot's invisibility status.
 *
 * An invisible pilot is not shown on the radar nor targettable, however, it
 * renders and updates just like normal.
 *
 *    @luatparam Pilot p Pilot to set invisibility status of.
 *    @luatparam boolean state State to set invisibility.
 * @luafunc setInvisible
 */
static int pilotL_setInvisible( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_INVISIBLE );
}

/**
 * @brief Sets the pilot's norender status.
 *
 * The pilot still acts normally but is just not visible and can still take
 * damage. Meant to be used in conjunction with other flags like "invisible".
 *
 *    @luatparam Pilot p Pilot to set norender status of.
 *    @luatparam boolean state State to set norender.
 * @luafunc setInvisible
 */
static int pilotL_setNoRender( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_NORENDER );
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
 * @luafunc setVisplayer
 */
static int pilotL_setVisplayer( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_VISPLAYER );
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
 * @luafunc setVisible
 */
static int pilotL_setVisible( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_VISIBLE );
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
 * @luafunc setHilight
 */
static int pilotL_setHilight( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_HILIGHT );
}

/**
 * @brief Makes pilot act as if bribed by the player.
 *
 * @usage p:setBribed( true )
 *
 *    @luatparam Pilot p Pilot to set bribed status of.
 *    @luatparam[opt=true] boolean state State to set bribed.
 * @luafunc setHilight
 */
static int pilotL_setBribed( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_BRIBED );
}

/**
 * @brief Allows the pilot to be boarded when not disabled.
 *
 * @usage p:setActiveBoard( true )
 *
 *    @luatparam Pilot p Pilot to set boardability of.
 *    @luatparam[opt=true] boolean state State to set boardability.
 * @luafunc setActiveBoard
 */
static int pilotL_setActiveBoard( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_BOARDABLE );
}

/**
 * @brief Makes it so the pilot never dies, stays at 1. armour.
 *
 * @usage p:setNoDeath( true ) -- Pilot will never die
 *
 *    @luatparam Pilot p Pilot to set never die state of.
 *    @luatparam[opt=true] boolean state Whether or not to set never die state.
 * @luafunc setNoDeath
 */
static int pilotL_setNoDeath( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_NODEATH );
}

/**
 * @brief Disables a pilot.
 *
 * @usage p:disable()
 *
 *    @luatparam Pilot p Pilot to disable.
 *    @luatparam[opt=false] boolean nopermanent Whether or not the disable should be not permanent.
 * @luafunc disable
 */
static int pilotL_disable( lua_State *L )
{
   /* Get the pilot. */
   Pilot *p       = luaL_validpilot(L,1);
   int permanent  = !lua_toboolean(L,2);

   /* Disable the pilot. */
   p->shield = 0.;
   p->stress = p->armour;
   pilot_updateDisable(p, 0);

   if (permanent)
      pilot_setFlag(p, PILOT_DISABLED_PERM);
   else
      pilot_rmFlag(p, PILOT_DISABLED_PERM);

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
 * @luafunc cooldown
 */
static int pilotL_cooldown( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
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
 * @luafunc setCooldown
 */
static int pilotL_setCooldown( lua_State *L )
{
   Pilot *p;
   int state;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Get state. */
   if (lua_isnone(L,2))
      state = 1;
   else
      state = lua_toboolean(L, 2);

   /* Set status. */
   if (state)
      pilot_cooldown( p, 1 );
   else
      pilot_cooldownEnd(p, NULL);

   return 0;
}

/**
 * @brief Makes the pilot do an instant full cooldown cycle.
 *
 *    @luatparam Pilot p Pilot to perform cycle.
 * @luafunc cooldownCycle
 */
static int pilotL_cooldownCycle( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   pilot_cooldown( p, 0 );
   p->ctimer = -1.;
   pilot_rmFlag(p, PILOT_COOLDOWN_BRAKE); /* Should allow triggering. */
   pilot_cooldownEnd( p, NULL );
   return 0;
}

/**
 * @brief Enables or disables a pilot's hyperspace engine.
 *
 * @usage p:setNoJump( true )
 *
 *    @luatparam Pilot p Pilot to modify.
 *    @luatparam[opt=true] boolean state true or false
 * @luafunc setNoJump
 */
static int pilotL_setNoJump( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_NOJUMP );
}

/**
 * @brief Enables or disables landing for a pilot.
 *
 * @usage p:setNoLand( true )
 *
 *    @luatparam Pilot p Pilot to modify.
 *    @luatparam[opt] boolean state true or false
 * @luafunc setNoLand
 */
static int pilotL_setNoLand( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_NOLAND );
}

/**
 * @brief Enables or disables making the the pilot exempt from pilot.clear().
 *
 * @usage p:setNoClear( true )
 *
 *    @luatparam Pilot p Pilot to modify.
 *    @luatparam[opt] boolean state true or false
 * @luafunc setNoClear
 */
static int pilotL_setNoClear( lua_State *L )
{
   return pilotL_setFlagWrapper( L, PILOT_NOCLEAR );
}

/**
 * @brief Adds an outfit to a specific slot.
 *
 *    @return 0 on failure to add, -1 on error to add, and 1 if added.
 */
static int pilot_outfitAddSlot( Pilot *p, const Outfit *o, PilotOutfitSlot *s, int bypass_cpu, int bypass_slot)
{
   int ret;

   /* Must not have outfit (excluding default) already. */
   if ((s->outfit != NULL) &&
         (s->outfit != s->sslot->data))
      return 0;

   /* Only do a basic check. */
   if (bypass_slot) {
      if (!outfit_fitsSlotType( o, &s->sslot->slot ))
         return 0;
   }
   else if (bypass_cpu) {
      if (!outfit_fitsSlot( o, &s->sslot->slot ))
         return 0;
   }
   /* Full check. */
   else {
      /* Must fit slot. */
      if (!outfit_fitsSlot( o, &s->sslot->slot ))
         return 0;

      /* Test if can add outfit. */
      ret = pilot_addOutfitTest( p, o, s, 0 );
      if (ret)
         return -1;
   }

   /* Add outfit - already tested. */
   ret = pilot_addOutfitRaw( p, o, s );
   if (ret==0)
      pilot_outfitLInit( p, s );

   /* Add ammo if needed. */
   if (ret==0)
      pilot_addAmmo( p, s, pilot_maxAmmoO(p,o) );

   /* Update GUI if necessary. */
   if (pilot_isPlayer(p))
      gui_setShip();

   return 1;
}

/**
 * @brief Adds an outfit to a pilot.
 *
 * This by default tries to add them to the first empty or defaultly equipped slot. Will not overwrite existing non-default outfits.
 *
 * @usage added = p:outfitAdd( "Laser Cannon", 5 ) -- Adds 5 laser cannons to p
 *
 *    @luatparam Pilot p Pilot to add outfit to.
 *    @luatparam string|outfit outfit Outfit or name of the outfit to add.
 *    @luatparam[opt=1] number q Quantity of the outfit to add.
 *    @luatparam[opt=false] boolean bypass_cpu Whether to skip CPU checks when adding an outfit.
 *    @luatparam[opt=false] boolean bypass_slot Whether or not to skip slot size checks before adding an outfit. Not that this implies skipping the CPU checks.
 *    @luatreturn number The number of outfits added.
 *    @luatreturn number The id of the slot of the first outfit added if applicable.
 * @luafunc outfitAdd
 */
static int pilotL_outfitAdd( lua_State *L )
{
   Pilot *p;
   const Outfit *o;
   int q, added, bypass_cpu, bypass_slot, slotid;

   /* Get parameters. */
   p      = luaL_validpilot(L,1);
   o      = luaL_validoutfit(L,2);
   q      = luaL_optinteger(L,3,1);
   bypass_cpu = lua_toboolean(L,4);
   bypass_slot = lua_toboolean(L,5);

   /* Add outfit. */
   added = 0;
   slotid = -1;
   for (int i=0; i<array_size(p->outfits); i++) {
      int ret;
      PilotOutfitSlot *s = p->outfits[i];

      /* Must still have to add outfit. */
      if (q <= 0)
         break;

      /* Do tests and try to add. */
      ret = pilot_outfitAddSlot( p, o, s, bypass_cpu, bypass_slot );
      if (ret < 0)
         break;
      else if (ret==0)
         continue;

      /* We added an outfit. */
      q--;
      added++;
      if (slotid < 0)
         slotid = i;
   }

   /* Update stats. */
   if (added > 0) {
      pilot_calcStats( p );

      /* Update the weapon sets. */
      if (p->autoweap)
         pilot_weaponAuto(p);

      /* Update equipment window if operating on the player's pilot. */
      if (player.p != NULL && player.p == p)
         outfits_updateEquipmentOutfits();

      /* Update GUI if necessary. */
      if (pilot_isPlayer(p))
         gui_setShip();
   }

   lua_pushnumber(L,added);
   if (slotid < 0)
      return 1;
   lua_pushinteger(L,slotid+1);
   return 2;
}

/**
 * @brief Checks to see outfit a pilot has in a slot.
 *
 *    @luatparam Pilot p Pilot to check outfit slot of.
 *    @luatparam string|integer slot Slot to check. Can be passed as a slot name (string) or slot id (integer).
 *    @luatreturn Outfit|nil Outfit if applicable or nil otherwise.
 * @luafunc outfitSlot
 */
static int pilotL_outfitSlot( lua_State *L )
{
   Pilot *p             = luaL_validpilot(L,1);
   PilotOutfitSlot *s   = luaL_checkslot( L, p, 2 );
   if (s==NULL)
      return 0;
   if (s->outfit) {
      lua_pushoutfit(L,s->outfit);
      return 1;
   }
   return 0;
}

/**
 * @brief Adds an outfit to a pilot by slot name.
 *
 *    @luatparam Pilot p Pilot to add outfit to.
 *    @luatparam string|outfit outfit Outfit or name of the outfit to add.
 *    @luatparam string|integer slot Slot to add to. Can be passed as a slot name (string) or slot id (integer).
 *    @luatparam[opt=false] boolean bypass_cpu Whether to skip CPU checks when adding an outfit.
 *    @luatparam[opt=false] boolean|string bypass_slot Whether or not to skip slot size checks before adding an outfit. Not that this implies skipping the CPU checks. In the case bypass_slot is a string, the outfit gets added to the named slot if possible (no slot check).
 *    @luatreturn boolean Whether or not the outfit was added.
 * @luafunc outfitAddSlot
 */
static int pilotL_outfitAddSlot( lua_State *L )
{
   Pilot *p;
   const Outfit *o;
   int ret, added, bypass_cpu, bypass_slot;
   PilotOutfitSlot *s;

   /* Get parameters. */
   p        = luaL_validpilot(L,1);
   o        = luaL_validoutfit(L,2);
   s        = luaL_checkslot( L, p, 3 );
   bypass_cpu = lua_toboolean(L,4);
   bypass_slot = lua_toboolean(L,5);
   if (s==NULL)
      return 0;

   /* Try to add. */
   ret = pilot_outfitAddSlot( p, o, s, bypass_cpu, bypass_slot );
   added = (ret>0);

   /* Update stats. */
   if (added > 0) {
      pilot_calcStats( p );

      /* Update the weapon sets. */
      if (p->autoweap)
         pilot_weaponAuto(p);

      /* Update equipment window if operating on the player's pilot. */
      if (player.p != NULL && player.p == p)
         outfits_updateEquipmentOutfits();

      /* Update GUI if necessary. */
      if (pilot_isPlayer(p))
         gui_setShip();
   }

   lua_pushboolean(L,added);
   return 1;
}

/**
 * @brief Removes an outfit from a pilot.
 *
 * "all" will remove all outfits except cores and locked outfits.
 * "cores" will remove all cores, but nothing else.
 *
 * @usage p:outfitRm( "all" ) -- Leaves the pilot naked (except for cores and locked outfits).
 * @usage p:outfitRm( "cores" ) -- Strips the pilot of its cores, leaving it dead in space.
 * @usage p:outfitRm( "Neutron Disruptor" ) -- Removes a neutron disruptor.
 * @usage p:outfitRm( "Neutron Disruptor", 2 ) -- Removes two neutron disruptor.
 *
 *    @luatparam Pilot p Pilot to remove outfit from.
 *    @luatparam string|outfit outfit Outfit or name of the outfit to remove.
 *    @luatparam number q Quantity of the outfit to remove.
 *    @luatreturn number The number of outfits removed.
 * @luafunc outfitRm
 */
static int pilotL_outfitRm( lua_State *L )
{
   Pilot *p;
   int q, removed, matched = 0;

   /* Get parameters. */
   removed = 0;
   p      = luaL_validpilot(L,1);
   q      = luaL_optinteger(L,3,1);

   if (lua_isstring(L,2)) {
      const char *outfit = luaL_checkstring(L,2);

      /* If outfit is "all", we remove everything except cores and locked outfits. */
      if (strcmp(outfit,"all")==0) {
         for (int i=0; i<array_size(p->outfits); i++) {
            if (p->outfits[i]->sslot->required)
               continue;
            if (p->outfits[i]->sslot->locked)
               continue;
            pilot_rmOutfitRaw( p, p->outfits[i] );
            removed++;
         }
         pilot_calcStats( p ); /* Recalculate stats. */
         matched = 1;
      }
      /* If outfit is "cores", we remove cores only. */
      else if (strcmp(outfit,"cores")==0) {
         for (int i=0; i<array_size(p->outfits); i++) {
            if (!p->outfits[i]->sslot->required)
               continue;
            pilot_rmOutfitRaw( p, p->outfits[i] );
            removed++;
         }
         pilot_calcStats( p ); /* Recalculate stats. */
         matched = 1;
      }
      /* Purpose fallthrough for if the outfit is passed as a string. */
   }

   if (!matched) {
      const Outfit *o = luaL_validoutfit(L,2);

      /* Remove the outfit outfit. */
      for (int i=0; i<array_size(p->outfits); i++) {
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
 * @brief Removes an outfit from a pilot's named slot.
 *
 * Note that this only works with the `name="foo"` property of slots. It is not meant to be used with unnamed slots. By default all slots are unnamed unless specified.
 *
 *    @luatparam Pilot p Pilot to remove outfit from.
 *    @luatparam string|integer slot Slot to remove from. Can be passed as a slot name (string) or slot id (integer).
 *    @luatreturn boolean true on success.
 * @luafunc outfitRmSlot
 * @see outfitRm
 */
static int pilotL_outfitRmSlot( lua_State *L )
{
   /* Get parameters. */
   int ret, removed = 0;
   Pilot *p = luaL_validpilot(L,1);
   PilotOutfitSlot *s = luaL_checkslot( L, p, 2 );
   if (s==NULL)
      return 0;

   ret = !pilot_rmOutfitRaw( p, s );
   if (ret) {
      pilot_calcStats( p ); /* Recalculate stats. */
      /* Update equipment window if operating on the player's pilot. */
      if (player.p != NULL && player.p == p && removed > 0)
         outfits_updateEquipmentOutfits();
   }

   lua_pushboolean( L, ret );
   return 1;
}

/**
 * @brief Adds an intrinsic outfit to the pilot.
 *
 * Intrinsic outfits are outfits that are associated with a ship, but not their slots.
 *
 *    @luatparam Pilot p Pilot to add intrinsic outfit to.
 *    @luatparam Outfit o Outfit to add as intrinsic outfit (must be modifier outfit).
 * @luafunc outfitAddIntrinsic
 * @see outfitAdd
 */
static int pilotL_outfitAddIntrinsic( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   const Outfit *o = luaL_validoutfit(L,2);
   int ret = pilot_addOutfitIntrinsic( p, o );
   if (ret==0)
      pilot_calcStats(p);
   lua_pushboolean(L,ret);

   /* Update GUI if necessary. */
   if (pilot_isPlayer(p))
      gui_setShip();

   return 1;
}

/**
 * @brief Removes an intrinsic outfit from the pilot.
 *
 * Intrinsic outfits are outfits that are associated with a ship, but not their slots.
 *
 *    @luatparam Pilot p Pilot to remove intrinsic outfit from.
 *    @luatparam Outfit o Outfit to remove from intrinsic outfits.
 * @luafunc outfitRmIntrinsic
 * @see outfitRm
 */
static int pilotL_outfitRmIntrinsic( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   const Outfit *o = luaL_validoutfit(L,2);
   for (int i=0; i<array_size(p->outfit_intrinsic); i++) {
      PilotOutfitSlot *s = &p->outfit_intrinsic[i];
      int ret;
      if (s->outfit != o)
         continue;
      ret = pilot_rmOutfitIntrinsic( p, s );
      if (ret==0)
         pilot_calcStats(p);
      lua_pushboolean(L,ret);
      return 1;
   }
   lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Gets the amount of fuel the pilot has.
 *
 *    @luatreturn number The amount of fuel the pilot has.
 * @luafunc fuel
 */
static int pilotL_getFuel( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, p->fuel);
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
 * @luafunc setFuel
 */
static int pilotL_setFuel( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);

   /* Get the parameter. */
   if (lua_isboolean(L,2)) {
      if (lua_toboolean(L,2))
         p->fuel = p->fuel_max;
      else
         p->fuel = 0;
   }
   else if (lua_isnumber(L,2)) {
      p->fuel = CLAMP( 0, p->fuel_max, lua_tonumber(L,2) );
   }
   else
      NLUA_INVALID_PARAMETER(L);

   /* Return amount of fuel. */
   lua_pushnumber(L, p->fuel);
   return 1;
}

/**
 * @brief Resets the intrinsic stats of a pilot.
 *
 * @luafunc intrinsicReset
 */
static int pilotL_intrinsicReset( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   ss_free( p->intrinsic_stats );
   p->intrinsic_stats = NULL;
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Allows setting intrinsic stats of a pilot.
 *
 * @usage p:intrinsicSet( "turn", -50 ) -- Lowers p's turn by 50%
 *
 *    @luatparam Pilot p Pilot to set stat of.
 *    @luatparam string name Name of the stat to set. It is the same as in the xml.
 *    @luatparam number value Value to set the stat to.
 *    @luatparam boolean replace Whether or not to add to the stat or replace it.
 * @luafunc intrinsicSet
 */
static int pilotL_intrinsicSet( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   const char *name;
   double value;
   int replace;
   /* Case individual parameter. */
   if (!lua_istable(L,2)) {
      name     = luaL_checkstring(L,2);
      value    = luaL_checknumber(L,3);
      replace  = lua_toboolean(L,4);
      p->intrinsic_stats = ss_statsSetList( p->intrinsic_stats, ss_typeFromName(name), value, replace, 0 );
      pilot_calcStats( p );
      return 0;
   }
   replace = lua_toboolean(L,4);
   /* Case set of parameters. */
   lua_pushnil(L);
   while (lua_next(L,2) != 0) {
      name     = luaL_checkstring(L,-2);
      value    = luaL_checknumber(L,-1);
      p->intrinsic_stats = ss_statsSetList( p->intrinsic_stats, ss_typeFromName(name), value, replace, 0 );
      lua_pop(L,1);
   }
   lua_pop(L,1);
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Allows getting an intrinsic stats of a pilot, or gets all of them if name is not specified.
 *
 *    @luatparam Pilot p Pilot to get stat of.
 *    @luatparam[opt=nil] string name Name of the stat to get. It is the same as in the xml.
 *    @luatparam[opt=false] boolean internal Whether or not to use the internal representation.
 *    @luaparam Value of the stat or a table containing all the stats if name is not specified.
 * @luafunc intrinsicGet
 */
static int pilotL_intrinsicGet( lua_State *L )
{
   const Pilot *p    = luaL_validpilot(L,1);
   const char *name  = luaL_optstring(L,2,NULL);
   int internal      = lua_toboolean(L,3);
   ShipStats ss;
   /* TODO get directly the stat from the list. */
   ss_statsInit( &ss );
   ss_statsMergeFromList( &ss, p->intrinsic_stats );
   ss_statsGetLua( L, &ss, name, internal );
   return 1;
}


/**
 * @brief Resets the ship property stats of a pilot.
 *
 * @luafunc shippropReset
 */
static int pilotL_shippropReset( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   ss_free( p->ship_stats );
   p->ship_stats = NULL;
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Allows setting ship property stats of a pilot.
 *
 * @usage p:shippropSet( "turn", -50 ) -- Lowers p's turn by 50%
 *
 *    @luatparam Pilot p Pilot to set stat of.
 *    @luatparam string name Name of the stat to set. It is the same as in the xml.
 *    @luatparam number value Value to set the stat to.
 * @luafunc shippropSet
 */
static int pilotL_shippropSet( lua_State *L )
{
   /* TODO merge with intrinsicSet */
   Pilot *p = luaL_validpilot(L,1);
   const char *name;
   double value;

   if (p->ship->lua_env == LUA_NOREF)
      NLUA_ERROR(L,_("Trying to set ship property of pilot '%s' flying ship '%s' with no ship Lua enabled!"), p->name, p->ship->name);

   /* Case individual parameter. */
   if (!lua_istable(L,2)) {
      name     = luaL_checkstring(L,2);
      value    = luaL_checknumber(L,3);
      p->ship_stats = ss_statsSetList( p->ship_stats, ss_typeFromName(name), value, 1, 0 );
      pilot_calcStats( p );
      return 0;
   }
   /* Case set of parameters. */
   lua_pushnil(L);
   while (lua_next(L,2) != 0) {
      name     = luaL_checkstring(L,-2);
      value    = luaL_checknumber(L,-1);
      p->ship_stats = ss_statsSetList( p->ship_stats, ss_typeFromName(name), value, 1, 0 );
      lua_pop(L,1);
   }
   lua_pop(L,1);
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Allows getting an ship property stats of a pilot, or gets all of them if name is not specified.
 *
 *    @luatparam Pilot p Pilot to get stat of.
 *    @luatparam[opt=nil] string name Name of the stat to get. It is the same as in the xml.
 *    @luatparam[opt=false] boolean internal Whether or not to use the internal representation.
 *    @luaparam Value of the stat or a table containing all the stats if name is not specified.
 * @luafunc shippropGet
 */
static int pilotL_shippropGet( lua_State *L )
{
   const Pilot *p    = luaL_validpilot(L,1);
   const char *name  = luaL_optstring(L,2,NULL);
   int internal      = lua_toboolean(L,3);
   ShipStats ss;
   /* TODO get directly the stat from the list. */
   ss_statsInit( &ss );
   ss_statsMergeFromList( &ss, p->ship_stats );
   ss_statsGetLua( L, &ss, name, internal );
   return 1;
}

/**
 * @brief Clears the effect on a pilot.
 *
 *    @luatparam Pilot p Pilot to clear effects of.
 *    @luatparam boolean keepdebuffs Whether or not to keep debuffs.
 *    @luatparam boolean keepbuffs Whether or not to keep buffs.
 *    @luatparam boolean keepothers Whether or not to keep others.
 * @luafunc effectClear
 */
static int pilotL_effectClear( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int keepdebuffs = lua_toboolean(L,2);
   int keepbuffs = lua_toboolean(L,3);
   int keepothers = lua_toboolean(L,4);
   if (!keepdebuffs && !keepbuffs && !keepothers)
      effect_clear( &p->effects );
   else
      effect_clearSpecific( &p->effects, !keepdebuffs, !keepbuffs, !keepothers );
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Adds an effect to a pilot.
 *
 *    @luatparam Pilot p Pilot to add effect to.
 *    @luatparam string name Name of the effect to add.
 *    @luatparam[opt=-1] duration Duration of the effect or set to negative to be default.
 *    @luatparam[opt=1] scale Scaling factor.
 *    @luatreturn boolean Whether or not the effect was successfully added.
 * @luafunc effectAdd
 */
static int pilotL_effectAdd( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   const char *effectname = luaL_checkstring(L,2);
   double duration = luaL_optnumber(L,3,-1.);
   double scale = luaL_optnumber(L,4,1.);
   const EffectData *efx = effect_get( effectname );
   if (efx != NULL) {
      if (!effect_add( &p->effects, efx, duration, scale, p->id ))
         pilot_calcStats( p );
      lua_pushboolean(L,1);
   }
   else
      lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Removes an effect from the pilot.
 *
 *    @luatparam Pilot p Pilot to remove effect from.
 *    @luatparam string|integer name Name of the effect to add or index in the case of being a number.
 *    @luatparam boolean all Remove all instances of the effect or only the most first instance. Only valid in the case the name is specified as a string.
 * @luafunc effectRm
 */
static int pilotL_effectRm( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   if (lua_isnumber(L,2)) {
      int idx = lua_tointeger(L,2);
      if (effect_rm( &p->effects, idx ))
         pilot_calcStats( p );
   }
   else {
      const char *effectname = luaL_checkstring(L,2);
      int all = lua_toboolean(L,3);
      const EffectData *efx = effect_get( effectname );
      if (efx != NULL) {
         if (effect_rmType( &p->effects, efx, all ))
            pilot_calcStats( p );
      }
   }
   return 0;
}

/**
 * @brief Gets the effects on a pilot.
 *
 *    @luatparam Pilot p Pilot to get effects of.
 *    @luatreturn table Table of effects which are treated as tables with "name" and "timer" elements.
 * @luafunc effects
 */
static int pilotL_effectGet( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_newtable(L);
   for (int i=0; i<array_size(p->effects); i++) {
      const Effect *e = &p->effects[i];
      lua_newtable(L);

      lua_pushstring(L,e->data->name);
      lua_setfield(L,-2,"name");

      lua_pushnumber(L,e->timer);
      lua_setfield(L,-2,"timer");

      lua_pushnumber(L,e->timer);
      lua_setfield(L,-2,"strength");

      lua_pushnumber(L,e->data->duration);
      lua_setfield(L,-2,"duration");

      lua_pushtex(L,gl_dupTexture(e->data->icon));
      lua_setfield(L,-2,"icon");

      if (e->data->flags & EFFECT_BUFF) {
         lua_pushboolean(L,1);
         lua_setfield(L,-2,"buff");
      }

      if (e->data->flags & EFFECT_DEBUFF) {
         lua_pushboolean(L,1);
         lua_setfield(L,-2,"debuff");
      }

      lua_rawseti(L,-2,i+1);
   }
   return 1;
}

/**
 * @brief Gets the pilot's AI.
 *
 *    @luatparam Pilot p Pilot to get AI of.
 *    @luatreturn string Name of the AI being used.
 * @luafunc ai
 */
static int pilotL_ai( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (p->ai == NULL)
      return 0;
   lua_pushstring( L, p->ai->name );
   return 1;
}

/**
 * @brief Changes the pilot's AI.
 *
 * @usage p:changeAI( "empire" ) -- set the pilot to use the Empire AI
 *
 *    @luatparam Pilot p Pilot to change AI of.
 *    @luatparam string newai Name of Ai to use.
 * @luafunc changeAI
 */
static int pilotL_changeAI( lua_State *L )
{
   int ret;
   /* Get parameters. */
   Pilot *p = luaL_validpilot(L,1);
   const char *str = luaL_checkstring(L,2);

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
 *    @luatparam[opt=false] boolean noslots Whether slots should also be set to this temperature.
 * @luafunc setTemp
 */
static int pilotL_setTemp( lua_State *L )
{
   Pilot *p;
   int setOutfits = 1;
   double kelvins;

   /* Handle parameters. */
   p           = luaL_validpilot(L,1);
   kelvins     = luaL_checknumber(L, 2);
   setOutfits  = !lua_toboolean(L,3);

   /* Temperature must not go below base temp. */
   kelvins = MAX(kelvins, CONST_SPACE_STAR_TEMP);

   /* Handle pilot ship. */
   p->heat_T = kelvins;

   /* Handle pilot outfits (maybe). */
   if (setOutfits)
      for (int i=0; i < array_size(p->outfits); i++)
         p->outfits[i]->heat_T = kelvins;

   return 0;
}

/**
 * @brief Sets the health of a pilot.
 *
 * This recovers the pilot's disabled state, although they may become disabled afterwards.
 *
 * @usage p:setHealth( 100, 100 ) -- Sets pilot to full health
 * @usage p:setHealth(  70,   0 ) -- Sets pilot to 70% armour
 * @usage p:setHealth( 100, 100, 0 ) -- Sets pilot to full health and no stress
 *
 *    @luatparam Pilot p Pilot to set health of.
 *    @luatparam[opt=current armour] number armour Value to set armour to, should be double from 0-100 (in percent).
 *    @luatparam[opt=current shield] number shield Value to set shield to, should be double from 0-100 (in percent).
 *    @luatparam[opt=0] number stress Value to set stress (disable damage) to, should be double from 0-100 (in percent of current armour).
 * @luafunc setHealth
 */
static int pilotL_setHealth( lua_State *L )
{
   Pilot *p;
   double a, s, st;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   a  = luaL_optnumber(L, 2, 100.*p->armour / p->armour_max);
   s  = luaL_optnumber(L, 3, 100.*p->shield / p->shield_max);
   st = luaL_optnumber(L,4,0.);

   a  /= 100.;
   s  /= 100.;
   st /= 100.;

   /* Set health. */
   p->armour = a * p->armour_max;
   p->shield = s * p->shield_max;
   p->stress = st * p->armour;

   /* Clear death hooks if not dead. */
   if (p->armour > 0.) {
      pilot_rmFlag( p, PILOT_DISABLED );
      pilot_rmFlag( p, PILOT_DEAD );
      pilot_rmFlag( p, PILOT_DEATH_SOUND );
      pilot_rmFlag( p, PILOT_EXPLODED );
      pilot_rmFlag( p, PILOT_DELETE );
      if (pilot_isPlayer(p))
         player_rmFlag( PLAYER_DESTROYED );
   }
   pilot_rmFlag( p, PILOT_DISABLED_PERM ); /* Remove permanent disable. */

   /* Update disable status. */
   pilot_updateDisable(p, 0);

   return 0;
}

/**
 * @brief Sets the health of a pilot in absolute value.
 *
 * This recovers the pilot's disabled state, although they may become disabled afterwards.
 *
 *    @luatparam Pilot p Pilot to set health of.
 *    @luatparam[opt=current armour] number armour Value to set armour to, in absolute value.
 *    @luatparam[opt=current shield] number shield Value to set shield to, in absolute value
 *    @luatparam[opt=current stress] number stress Value to set stress (disable damage) to, in absolute value.
 * @luafunc setHealth
 */
static int pilotL_setHealthAbs( lua_State *L )
{
   Pilot *p;
   double a, s, st;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   a  = luaL_optnumber(L, 2, p->armour);
   s  = luaL_optnumber(L, 3, p->shield);
   st = luaL_optnumber(L,4,p->stress);

   /* Set health. */
   p->armour = CLAMP( 0., p->armour_max, a );
   p->shield = CLAMP( 0., p->shield_max, s );
   p->stress = CLAMP( 0., p->armour_max, st );

   /* Clear death hooks if not dead. */
   if (p->armour > 0.) {
      pilot_rmFlag( p, PILOT_DISABLED );
      pilot_rmFlag( p, PILOT_DEAD );
      pilot_rmFlag( p, PILOT_DEATH_SOUND );
      pilot_rmFlag( p, PILOT_EXPLODED );
      pilot_rmFlag( p, PILOT_DELETE );
      if (pilot_isPlayer(p))
         player_rmFlag( PLAYER_DESTROYED );
   }
   pilot_rmFlag( p, PILOT_DISABLED_PERM ); /* Remove permanent disable. */

   /* Update disable status. */
   pilot_updateDisable(p, 0);

   return 0;
}

/**
 * @brief Adds health to a pilot.
 *
 * Does not revive dead pilots, use setHealth for that.
 *
 *    @luatparam Pilot p Pilot to add health to.
 *    @luatparam[opt=0.] number armour Armour to add.
 *    @luatparam[opt=0.] number shield Shield to add.
 * @luafunc addHealth
 */
static int pilotL_addHealth( lua_State *L )
{
   Pilot *p;
   double a, s;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   a  = luaL_optnumber(L, 2, 0.);
   s  = luaL_optnumber(L, 3, 0.);

   /* Set health. */
   p->armour = CLAMP( 0., p->armour_max, p->armour + a );
   p->shield = CLAMP( 0., p->shield_max, p->shield + s );

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
 *    @luatparam[opt=false] boolean absolute Whether or not it is being set in relative value or absolute.
 * @luafunc setEnergy
 */
static int pilotL_setEnergy( lua_State *L )
{
   /* Handle parameters. */
   Pilot *p     = luaL_validpilot(L,1);
   double e     = luaL_checknumber(L,2);
   int absolute = lua_toboolean(L,3);

   if (absolute)
      p->energy = CLAMP( 0, p->energy_max, e );
   else
      p->energy = (e/100.) * p->energy_max;

   return 0;
}

/**
 * @brief Fills up the pilot's ammo.
 *
 *    @luatparam Pilot p Pilot to fill ammo.
 * @luafunc fillAmmo
 */
static int pilotL_fillAmmo( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   pilot_fillAmmo( p );
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
 * @luafunc setNoboard
 */
static int pilotL_setNoboard( lua_State *L )
{
   Pilot *p;
   int disable;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_isnone(L,2))
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
 * @usage p:setNoDisable( true ) -- Pilot can not be disabled anymore.
 *
 *    @luatparam Pilot p Pilot to set disable disabling.
 *    @luatparam[opt=true] boolean disable If true it disallows disabled of the pilot, otherwise
 *              it allows disabling which is the default.
 * @luafunc setNoDisable
 */
static int pilotL_setNoDisable( lua_State *L )
{
   Pilot *p;
   int disable;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_isnone(L,2))
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
 * @note Can increase the pilot's speed limit over what would be physically possible.
 *
 * @usage p:setSpeedLimit( 100 ) -- Sets maximumspeed to 100px/s.
 * @usage p:setSpeedLimit( 0 ) removes speed limit.
 *    @luatparam pilot p Pilot to set speed of.
 *    @luatparam number speed Value to set speed to.
 *
 * @luafunc setSpeedLimit
 */
static int pilotL_setSpeedLimit(lua_State* L)
{
   /* Handle parameters. */
   Pilot *p = luaL_validpilot(L,1);
   double s = luaL_checknumber(L, 2);

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
 *    @luatparam[opt=false] boolean absolute Whether or not it shouldn't be relative and be absolute instead.
 *    @luatreturn number The armour in % [0:100] if relative or absolute value otherwise.
 *    @luatreturn number The shield in % [0:100] if relative or absolute value otherwise.
 *    @luatreturn number The stress in % [0:100].
 *    @luatreturn boolean Indicates if pilot is disabled.
 * @luafunc health
 */
static int pilotL_getHealth( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   int absolute = lua_toboolean(L,2);
   /* Return parameters. */
   if (absolute) {
      lua_pushnumber(L, p->armour );
      lua_pushnumber(L, p->shield );
   }
   else {
      lua_pushnumber(L,(p->armour_max > 0.) ? p->armour / p->armour_max * 100. : 0. );
      lua_pushnumber(L,(p->shield_max > 0.) ? p->shield / p->shield_max * 100. : 0. );
   }
   lua_pushnumber(L, MIN( 1., p->stress / p->armour ) * 100. );
   lua_pushboolean(L, pilot_isDisabled(p));
   return 4;
}

/**
 * @brief Gets the pilot's armour.
 *
 * @usage armour = p:armour()
 *
 *    @luatparam Pilot p Pilot to get armour of.
 *    @luatparam[opt=false] boolean absolute Whether or not it shouldn't be relative and be absolute instead.
 *    @luatreturn number The armour in % [0:100] if relative or absolute value otherwise.
 * @luafunc armour
 */
static int pilotL_getArmour( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   int absolute = lua_toboolean(L,2);
   if (absolute)
      lua_pushnumber(L, p->armour );
   else
      lua_pushnumber(L,(p->armour_max > 0.) ? p->armour / p->armour_max * 100. : 0. );
   return 1;
}

/**
 * @brief Gets the pilot's shield.
 *
 * @usage shield = p:shield()
 *
 *    @luatparam Pilot p Pilot to get shield of.
 *    @luatparam[opt=false] boolean absolute Whether or not it shouldn't be relative and be absolute instead.
 *    @luatreturn number The shield in % [0:100] if relative or absolute value otherwise.
 * @luafunc shield
 */
static int pilotL_getShield( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   int absolute = lua_toboolean(L,2);
   if (absolute)
      lua_pushnumber(L, p->shield );
   else
      lua_pushnumber(L,(p->shield_max > 0.) ? p->shield / p->shield_max * 100. : 0. );
   return 1;
}

/**
 * @brief Gets the pilot's energy.
 *
 * @usage energy = p:energy()
 *
 *    @luatparam Pilot p Pilot to get energy of.
 *    @luatparam[opt=false] boolean absolute Whether or not to return the absolute numeric value instead of the relative value.
 *    @luatreturn number The energy of the pilot in % [0:100].
 * @luafunc energy
 */
static int pilotL_getEnergy( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   int absolute = lua_toboolean(L,2);
   if (absolute)
      lua_pushnumber(L, p->energy );
   else
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
 * @luafunc lockon
 */
static int pilotL_getLockon( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
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
 * @luafunc stats
 */
static int pilotL_getStats( lua_State *L )
{
   const Pilot *p  = luaL_validpilot(L,1);

   /* Create table with information. */
   lua_newtable(L);
   /* Core. */
   PUSH_DOUBLE( L, "cpu", p->cpu );
   PUSH_INT( L, "cpu_max", p->cpu_max );
   PUSH_INT( L, "crew", (int)round( p->crew ) );
   PUSH_INT( L, "fuel", p->fuel );
   PUSH_INT( L, "fuel_max", p->fuel_max );
   PUSH_INT( L, "fuel_consumption", p->fuel_consumption );
   PUSH_DOUBLE( L, "mass", p->solid.mass );
   /* Movement. */
   PUSH_DOUBLE( L, "thrust", p->thrust / p->solid.mass );
   PUSH_DOUBLE( L, "speed", p->speed );
   PUSH_DOUBLE( L, "turn", p->turn * 180. / M_PI ); /* Convert back to grad. */
   PUSH_DOUBLE( L, "speed_max", solid_maxspeed(&p->solid, p->speed, p->thrust) );
   /* Health. */
   PUSH_DOUBLE( L, "absorb", p->dmg_absorb );
   PUSH_DOUBLE( L, "armour", p->armour_max );
   PUSH_DOUBLE( L, "shield", p->shield_max );
   PUSH_DOUBLE( L, "energy", p->energy_max );
   PUSH_DOUBLE( L, "armour_regen", p->armour_regen );
   PUSH_DOUBLE( L, "shield_regen", p->shield_regen );
   PUSH_DOUBLE( L, "energy_regen", p->energy_regen );
   /* Stats. */
   PUSH_DOUBLE( L, "ew_detection", p->ew_detection );
   PUSH_DOUBLE( L, "ew_evasion", p->ew_evasion );
   PUSH_DOUBLE( L, "ew_stealth", p->ew_stealth );
   PUSH_DOUBLE( L, "jump_delay", ntime_convertSeconds( pilot_hyperspaceDelay(p) ) );
   PUSH_INT( L, "jumps", pilot_getJumps(p) );

   return 1;
}
#undef PUSH_DOUBLE
#undef PUSH_INT

/**
 * @brief Gets a shipstat from a Pilot by name, or a table containing all the ship stats if not specified.
 *
 * @usage local mod = p:shipstat("tur_damage",true) -- Gets turret damage bonus with internal representation
 *
 *    @luatparam Pilot p Pilot to get ship stat of.
 *    @luatparam[opt=nil] string name Name of the ship stat to get.
 *    @luatparam[opt=false] boolean internal Whether or not to use the internal representation.
 *    @luareturn Value of the ship stat or a table containing all the ship stats if name is not specified.
 * @luafunc shipstat
 */
static int pilotL_getShipStat( lua_State *L )
{
   const Pilot *p    = luaL_validpilot(L,1);
   const char *str   = luaL_optstring(L,2,NULL);
   int internal      = lua_toboolean(L,3);
   ss_statsGetLua( L, &p->stats, str, internal );
   return 1;
}

/**
 * @brief Gets the distance that a pilot can be currently detect at.
 *
 *    @luatparam Pilot p Pilot to get the distance they can be detected at.
 * @luafunc detectedDistance
 */
static int pilotL_getDetectedDistance( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (pilot_isFlag(p,PILOT_STEALTH))
      lua_pushnumber( L, p->ew_stealth );
   else
      lua_pushnumber( L, p->ew_detection );
   return 1;
}

/**
 * @brief Gets the free cargo space the pilot has.
 *
 *    @luatparam Pilot p The pilot to get the free cargo space of.
 *    @luatreturn number The free cargo space in tonnes of the player.
 * @luafunc cargoFree
 */
static int pilotL_cargoFree( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, pilot_cargoFree(p) );
   return 1;
}

/**
 * @brief Checks to see how many tonnes of a specific type of cargo the pilot has.
 *
 *    @luatparam Pilot p The pilot to get the cargo count of.
 *    @luatparam Commodity|string cargo Type of cargo to check, either
 *       as a Commodity object or as the raw (untranslated) name of a
 *       commodity.
 *    @luatreturn number The amount of cargo the player has.
 * @luafunc cargoHas
 */
static int pilotL_cargoHas( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L, 1);
   const Commodity *cargo = luaL_validcommodity(L, 2);
   int quantity = pilot_cargoOwned(p, cargo);
   lua_pushnumber(L, quantity);
   return 1;
}

/**
 * @brief Tries to add cargo to the pilot's ship.
 *
 * @usage n = pilot.cargoAdd( player.pilot(), "Food", 20 )
 *
 *    @luatparam Pilot p The pilot to add cargo to.
 *    @luatparam Commodity|string cargo Type of cargo to add, either as
 *       a Commodity object or as the raw (untranslated) name of a
 *       commodity.
 *    @luatparam number quantity Quantity of cargo to add.
 *    @luatreturn number The quantity of cargo added.
 * @luafunc cargoAdd
 */
static int pilotL_cargoAdd( lua_State *L )
{
   /* Parse parameters. */
   Pilot *p = luaL_validpilot(L, 1);
   const Commodity *cargo = luaL_validcommodity(L, 2);
   int quantity = luaL_checknumber(L, 3);

   if (quantity < 0) {
      NLUA_ERROR( L, _("Quantity must be positive for pilot.cargoAdd (if removing, use pilot.cargoRm)") );
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_cargoAdd( p, cargo, quantity, 0 );
   lua_pushnumber( L, quantity );
   return 1;
}

static int pilotL_cargoRmHelper( lua_State *L, int jet )
{
   Pilot *p;
   int quantity;
   Commodity *cargo = NULL;

   /* Parse parameters. */
   p = luaL_validpilot(L, 1);

   if (lua_isstring(L, 2)) {
      const char *str = lua_tostring(L, 2);

      /* Check for special strings. */
      if (strcmp(str, "all") == 0) {
         quantity = pilot_cargoRmAll(p, 0);
         lua_pushnumber(L, quantity);
         return 1;
      }
   }

   /* No special string handling, just handle as a normal commodity. */
   cargo = luaL_validcommodity(L, 2);
   quantity = luaL_checknumber(L, 3);

   if (quantity < 0) {
      NLUA_ERROR(L,
            _("Quantity must be positive for pilot.cargoRm (if adding, use"
               " pilot.cargoAdd)"));
      return 0;
   }

   /* Try to remove the cargo. */
   if (jet)
      quantity = pilot_cargoJet( p, cargo, quantity, 0 );
   else
      quantity = pilot_cargoRm( p, cargo, quantity );

   lua_pushnumber(L, quantity);
   return 1;
}

/**
 * @brief Tries to remove cargo from the pilot's ship.
 *
 * @usage n = pilot.cargoRm(player.pilot(), "Food", 20)
 * @usage n = pilot.cargoRm(player.pilot(), "all") -- Removes all cargo from the player
 *
 *    @luatparam Pilot p The pilot to remove cargo from.
 *    @luatparam Commodity|string cargo Type of cargo to remove, either
 *       as a Commodity object or as the raw (untranslated) name of a
 *       commodity. You can also pass the special value "all" to
 *       remove all cargo from the pilot, except for mission cargo.
 *    @luatparam number quantity Quantity of the cargo to remove.
 *    @luatreturn number The number of cargo removed.
 * @luafunc cargoRm
 */
static int pilotL_cargoRm( lua_State *L )
{
   return pilotL_cargoRmHelper( L, 0 );
}

/**
 * @brief Tries to remove a cargo from a pilot's ship and jet it into space.
 *
 *    @luatparam Pilot p The pilot to remove cargo from.
 *    @luatparam Commodity|string cargo Type of cargo to remove, either
 *       as a Commodity object or as the raw (untranslated) name of a
 *       commodity. You can also pass the special value "all" to
 *       remove all cargo from the pilot, except for mission cargo.
 *    @luatparam number quantity Quantity of the cargo to remove.
 *    @luatreturn number The number of cargo removed.
 * @luafunc cargoJet
 */
static int pilotL_cargoJet( lua_State *L )
{
   return pilotL_cargoRmHelper( L, 1 );
}

/**
 * @brief Lists the cargo the pilot has.
 *
 * The list has the following members:<br />
 * <ul>
 * <li><b>name:</b> raw (untranslated) name of the cargo (equivalent to the output of commodity.nameRaw()).</li>
 * <li><b>c:</b> the cargo commodity.</li>
 * <li><b>q:</b> quantity of the cargo.</li>
 * <li><b>m:</b> true if cargo is for a mission.</li>
 * </ul>
 *
 * @usage for i, v in ipairs(pilot.cargoList(player.pilot())) do print( string.format("%s: %d", v.name, v.q ) ) end
 *
 *    @luatparam Pilot p Pilot to list cargo of.
 *    @luatreturn table An ordered list with the names of the cargo the pilot has.
 * @luafunc cargoList
 */
static int pilotL_cargoList( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_newtable(L); /* t */
   for (int i=0; i<array_size(p->commodities); i++) {
      PilotCommodity *pc = &p->commodities[i];

      /* Represents the cargo. */
      lua_newtable(L); /* t, t */

      lua_pushstring(L, "name"); /* t, t, i */
      lua_pushstring(L, pc->commodity->name); /* t, t, i, s */
      lua_rawset(L,-3); /* t, t */

      lua_pushstring(L, "c"); /* t, t, i */
      lua_pushcommodity(L, (Commodity*)pc->commodity); /* t, t, i, s */
      lua_rawset(L,-3); /* t, t */

      lua_pushstring(L, "q"); /* t, t, i */
      lua_pushnumber(L, pc->quantity); /* t, t, i, s */
      lua_rawset(L,-3); /* t, t */

      lua_pushstring(L, "m"); /* t, t, i */
      lua_pushboolean(L, pc->id); /* t, t, i, s */
      lua_rawset(L,-3); /* t, t */

      lua_rawseti(L,-2,i+1); /* t */
   }
   return 1;

}

/**
 * @brief Handles the pilots credits
 *
 *    @luatparam Pilot p Pilot to manipulate credits of.
 *    @luatparam[opt=0] number cred Credits to give to the pilot.
 *    @luatreturn number The credits the pilot has.
 * @luafunc credits
 */
static int pilotL_credits( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   pilot_modCredits( p, luaL_optlong( L, 2, 0 ) );
   lua_pushnumber( L, p->credits );
   return 1;
}

/**
 * @brief Gets the worth of a pilot (total value of ship and outfits).
 *
 *    @luatparam Pilot p Pilot to get worth of.
 *    @luatparam[opt=false] boolean count_unique Whether or not to count unique outfits too.
 *    @luatreturn number The credit worth of the pilot.
 * @luafunc worth
 */
static int pilotL_worth( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber( L, pilot_worth(p, lua_toboolean(L,2)) );
   return 1;
}

/**
 * @brief Gets the pilot's colour based on hostility or friendliness to the player.
 *
 * @usage p:colour()
 *
 *    @luatparam Pilot p Pilot to get the colour of.
 *    @luatreturn Colour The pilot's colour.
 * @luafunc colour
 */
static int pilotL_getColour( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   const glColour *col = pilot_getColour(p);
   lua_pushcolour( L, *col );
   return 1;
}

/**
 * @brief Gets the pilot's colour character based on hostility or friendliness to the player. For use with functions that print to the screen.
 *
 *    @luatparam Pilot p Pilot to get the colour of.
 *    @luatreturn string Character representing the pilot's colour for use with specila printing characters.
 * @luafunc colour
 */
static int pilotL_colourChar( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   char str[2];
   str[0] = pilot_getFactionColourChar( p );
   str[1] = '\0';
   lua_pushstring(L,str);
   return 1;
}

/**
 * @brief Returns whether the pilot is hostile to the player.
 *
 * @usage p:hostile()
 *
 *    @luatparam Pilot p Pilot to get the hostility of.
 *    @luatreturn boolean The pilot's hostility status.
 * @luafunc hostile
 */
static int pilotL_getHostile( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean( L, pilot_isHostile( p ) );
   return 1;
}

/**
 * @brief Small struct to handle flags.
 */
struct pL_flag {
   const char *name; /**< Name of the flag. */
   int id;     /**< Id of the flag. */
};
static const struct pL_flag pL_flags[] = {
   { .name = "stealth", .id = PILOT_STEALTH },
   { .name = "refueling", .id = PILOT_REFUELING },
   { .name = "invisible", .id = PILOT_INVISIBLE },
   { .name = "disabled", .id = PILOT_DISABLED },
   { .name = "takingoff", .id = PILOT_TAKEOFF },
   { .name = "jumpingin", .id = PILOT_HYP_END },
   { .name = "jumpingout", .id = PILOT_HYPERSPACE },
   { .name = "manualcontrol", .id = PILOT_MANUAL_CONTROL },
   { .name = "carried", .id = PILOT_CARRIED },
   { .name = "hailing", .id = PILOT_HAILING },
   { .name = "bribed", .id = PILOT_BRIBED },
   { .name = "boardable", .id = PILOT_BOARDABLE },
   { .name = "nojump", .id = PILOT_NOJUMP },
   { .name = "noland", .id = PILOT_NOLAND },
   { .name = "nodeath", .id = PILOT_NODEATH },
   { .name = "nodisable", .id = PILOT_NODISABLE },
   { .name = "visible", .id = PILOT_VISIBLE },
   { .name = "visplayer", .id = PILOT_VISPLAYER },
   { .name = "hilight", .id = PILOT_HILIGHT },
   { .name = "norender", .id = PILOT_NORENDER },
   { .name = "hide", .id = PILOT_HIDE },
   { .name = "invincible", .id = PILOT_INVINCIBLE },
   { .name = "invinc_player", .id = PILOT_INVINC_PLAYER },
   { .name = "friendly", .id = PILOT_FRIENDLY },
   { .name = "hostile", .id = PILOT_HOSTILE },
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
 *  <li> stealth: pilot is in stealth mode.</li>
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
 *  <li> carried: pilot came from a fighter bay.</li>
 * </ul>
 *    @luatparam Pilot p Pilot to get flags of.
 *    @luatparam[opt] string name If provided, return only the individual flag.
 *    @luatreturn table Table with flag names an index, boolean as value.
 * @luafunc flags
 */
static int pilotL_flags( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   const char *name = luaL_optstring( L, 2, NULL );

   if (name != NULL) {
      for (int i=0; pL_flags[i].name != NULL; i++)
         if (strcmp(pL_flags[i].name,name)==0) {
            lua_pushboolean( L, pilot_isFlag( p, pL_flags[i].id ) );
            return 1;
         }
#if DEBUGGING
      WARN(_("Tried to access unknown flag '%s' for pilot '%s'!"), name, p->name);
#endif /* DEBUGGING */
      return 0;
   }

   /* Create flag table. */
   lua_newtable(L);
   for (int i=0; pL_flags[i].name != NULL; i++) {
      lua_pushboolean( L, pilot_isFlag( p, pL_flags[i].id ) );
      lua_setfield(L, -2, pL_flags[i].name);
   }
   return 1;
}

/**
 * @brief Checks to see if the pilot has illegal stuff to a faction.
 *
 *    @luatparam Pilot p Pilot to check.
 *    @luatparam Faction f Faction to see if it is illegal to.
 * @luafunc hasIllegal
 */
static int pilotL_hasIllegal( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   int f = luaL_validfaction(L,2);
   lua_pushboolean(L, pilot_hasIllegal(p,f));
   return 1;
}

/**
 * @brief Gets the pilot's ship.
 *
 * @usage s = p:ship()
 *
 *    @luatparam Pilot p Pilot to get ship of.
 *    @luatreturn Ship The ship of the pilot.
 * @luafunc ship
 */
static int pilotL_ship( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushship(L, p->ship);
   return 1;
}

/**
 * @brief Gets the rough radius of the ship, useful for collision stuff.
 *
 *    @luatparam Pilot p Pilot to get radius of.
 *    @luatreturn number THe radius of the pilot.
 * @luafunc radius
 */
static int pilotL_radius( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, PILOT_SIZE_APPROX * 0.5 * (p->ship->gfx_space->sw+p->ship->gfx_space->sh));
   return 1;
}

/**
 * @brief Gets the points the pilot costs.
 *
 * Note currently equivalent to p:ship():points().
 *
 *    @luatparam Pilot p Pilot to get points of.
 *    @luatreturn number The points of the pilot.
 * @luafunc points
 */
static int pilotL_points( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushinteger(L, p->ship->points);
   return 1;
}

/**
 * @brief Checks to see if the pilot is idle.
 *
 * @usage idle = p:idle() -- Returns true if the pilot is idle
 *
 *    @luatparam Pilot p Pilot to check to see if is idle.
 *    @luatreturn boolean true if pilot is idle, false otherwise
 * @luafunc idle
 */
static int pilotL_idle( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L, p->task==0);
   return 1;
}

/**
 * @brief Sets manual control of the pilot.
 *
 * Note that this will reset the pilot's current task when the state changes. In the case of the player, it will also clear autonav.
 *
 * @usage p:control() -- Same as p:control(true), enables manual control of the pilot
 * @usage p:control(false) -- Restarts AI control of the pilot
 * @usage p:control( true, true ) -- Enables manual control of the pilot and resets tasks.
 *
 *    @luatparam Pilot p Pilot to change manual control settings.
 *    @luatparam[opt=true] boolean enable If true or nil enables pilot manual control, otherwise enables automatic AI.
 *    @luatparam[opt=true if changing modes] boolean Whether or not to clear the tasks for the pilot. Defaults to true when changing from manual to normal mode or viceversa.
 * @luasee moveto
 * @luasee brake
 * @luasee follow
 * @luasee attack
 * @luasee runaway
 * @luasee hyperspace
 * @luasee land
 * @luafunc control
 */
static int pilotL_control( lua_State *L )
{
   Pilot *p;
   int enable, cleartasks;

   /* Handle parameters. */
   p  = luaL_validpilot(L,1);
   if (lua_isnone(L,2))
      enable = 1;
   else
      enable = lua_toboolean(L, 2);
   if (lua_isnone(L,3))
      cleartasks = enable ^ pilot_isFlag(p, PILOT_MANUAL_CONTROL);
   else
      cleartasks = lua_toboolean(L, 3);

   if (enable) {
      int isp = pilot_isPlayer(p);
      if (isp)
         player_autonavAbort(NULL); /* Has to be run before setting the flag. */
      pilot_setFlag(p, PILOT_MANUAL_CONTROL);
      if (isp)
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

   /* Clear task if changing state. */
   if (cleartasks)
      pilotL_taskclear( L );

   return 0;
}

/**
 * @brief Gets a pilots memory table.
 *
 * The resulting table is indexable and mutable.
 *
 * @usage aggr = p:memory().aggressive
 * @usage p:memory().aggressive = false
 *
 *    @luatparam Pilot p Pilot to read memory of.
 * @luafunc memory
 */
static int pilotL_memory( lua_State *L )
{
   /* Get the pilot. */
   Pilot *p  = luaL_validpilot(L,1);

   /* Set the pilot's memory. */
   if (p->ai == NULL) {
      NLUA_ERROR(L,_("Pilot '%s' does not have an AI!"),p->name);
      return 0;
   }

   lua_rawgeti( L, LUA_REGISTRYINDEX, p->lua_mem );
   return 1;
}

/**
 * @brief Gets a pilots ship memory table.
 *
 * The resulting table is indexable and mutable.
 *
 *    @luatparam Pilot p Pilot to get ship memory of.
 * @luafunc memory
 */
static int pilotL_shipmemory( lua_State *L )
{
   /* Get the pilot. */
   Pilot *p  = luaL_validpilot(L,1);

   /* Possible it's not initialized yet, so we do the dirty work here. */
   if (p->lua_ship_mem == LUA_NOREF) {
      lua_newtable( naevL ); /* mem */
      p->lua_ship_mem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* */
   }

   lua_rawgeti( L, LUA_REGISTRYINDEX, p->lua_ship_mem );
   return 1;
}

/**
 * @brief Gets the name of the task the pilot is currently doing.
 *
 *    @luatparam Pilot p Pilot to get task name of.
 *    @luatreturn string Name of the task.
 * @luafunc ainame
 */
static int pilotL_ainame( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   if (p->ai == NULL)
      return 0;
   lua_pushstring(L, p->ai->name);
   return 1;
}

/**
 * @brief Gets the name and data of a pilot's current task.
 *
 *    @luatparam Pilot p Pilot to get task data of.
 *    @luatreturn string Name of the task.
 *    @luareturn Data of the task.
 * @luafunc task
 */
static int pilotL_task( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   Task *t  = ai_curTask(p);
   if (t) {
      lua_pushstring(L, t->name);
      if (t->dat != LUA_NOREF) {
         lua_rawgeti(L, LUA_REGISTRYINDEX, t->dat);
         return 2;
      }
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the name of the task the pilot is currently doing.
 *
 *    @luatparam Pilot p Pilot to get task name of.
 *    @luatreturn string Name of the task.
 *    @luatreturn string|nil Name of the subtask if applicable, or nil otherwise.
 * @luafunc taskname
 */
static int pilotL_taskname( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   Task *t  = ai_curTask(p);
   if (t) {
      lua_pushstring(L, t->name);
      if (t->subtask != NULL) {
         lua_pushstring(L, t->subtask->name);
         return 2;
      }
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the name of all the pilot's current tasks (not subtasks).
 *
 *    @luatparam Pilot p Pilot to get task stack of.
 * @luafunc taskstack
 */
static int pilotL_taskstack( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   int n;

   lua_newtable(L);
   n = 1;
   for (Task *t=p->task; t!=NULL; t=t->next) {
      if (t->done)
         continue;
      lua_pushstring(L,t->name);
      lua_rawseti(L,-2,n++);
   }

   return 1;
}

/**
 * @brief Gets the data of the task the pilot is currently doing.
 *
 *    @luatparam Pilot p Pilot to get task data of.
 *    @luareturn Data of the task.
 * @luafunc taskdata
 */
static int pilotL_taskdata( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   Task *t  = ai_curTask(p);
   if (t && (t->dat != LUA_NOREF)) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, t->dat);
      return 1;
   }
   return 0;
}

/**
 * @brief Clears all the tasks of the pilot.
 *
 * @usage p:taskClear()
 *
 *    @luatparam Pilot p Pilot to clear tasks of.
 * @luafunc taskClear
 */
static int pilotL_taskclear( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   ai_cleartasks( p );
   return 0;
}

/**
 * @brief Pushes a new task to the pilot's AI.
 *
 * Equivalent to ai.pushtask()
 *
 *    @luatparam Pilot p Pilot to push task to.
 *    @luatparam string func Name of the function to be run.
 *    @luatparam any data Data to be passed to the function.
 * @luafunc pushtask
 */
static int pilotL_pushtask( lua_State *L )
{
   Pilot *p          = luaL_validpilot(L,1);
   const char *task  = luaL_checkstring(L,2);

   if (pilot_isPlayer(p) && !pilot_isFlag(p,PILOT_MANUAL_CONTROL))
      return 0;

   Task *t           = ai_newtask( L, p, task, 0, 1 );
   if (!lua_isnoneornil(L,3)) {
      lua_pushvalue( L, 3 );
      t->dat = luaL_ref( L, LUA_REGISTRYINDEX );
   }
   return 0;
}

/**
 * @brief Pops the current task from the pilot's AI.
 *
 * Equivalent to ai.poptask().
 *
 *    @luatparam Pilot p Pilot to pop task from.
 * @luafunc poptask
 */
static int pilotL_poptask( lua_State *L )
{
   Pilot *p  = luaL_validpilot(L,1);
   Task *t = ai_curTask( p );
   /* Tasks must exist. */
   if (t == NULL) {
      NLUA_ERROR(L, _("Trying to pop task when there are no tasks on the stack."));
      return 0;
   }
   t->done = 1;
   return 0;
}

/**
 * @brief Tries to refuel a pilot.
 *
 *    @luatparam Pilot p Pilot to do the refueling.
 *    @luatparam Pilot target Target pilot to give fuel to.
 *    @luatparam[opt=100] number amount Amount to refuel.
 * @luafunc refuel
 */
static int pilotL_refuel( lua_State *L )
{
   Pilot *p       = luaL_validpilot(L,1);
   Pilot *target  = luaL_validpilot(L,2);
   double amount  = luaL_optinteger(L,3,100);
   pilot_rmFlag(  p, PILOT_HYP_PREP);
   pilot_rmFlag(  p, PILOT_HYP_BRAKE );
   pilot_rmFlag(  p, PILOT_HYP_BEGIN);
   pilot_setFlag( p, PILOT_REFUELING);
   ai_refuel(     p, target->id );
   p->refuel_amount = amount;
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
      NLUA_ERROR( L, _("Pilot '%s' is not on manual control."), p->name );
      return 0;
   }

   /* Creates the new task. */
   t = ai_newtask( L, p, task, 0, 1 );
   if (t==NULL)
      NLUA_ERROR( L, _("Failed to create new task for pilot '%s'."), p->name );

   return t;
}

/**
 * @brief Makes the pilot move to a position.
 *
 * Pilot must be under manual control for this to work.
 *
 * @usage p:moveto( v ) -- Goes to v precisely and braking
 * @usage p:moveto( v, true, true ) -- Same as p:moveto( v )
 * @usage p:moveto( v, false ) -- Goes to v without braking compensating velocity
 * @usage p:moveto( v, false, false ) -- Really rough approximation of going to v without braking
 *
 *    @luatparam Pilot p Pilot to tell to go to a position.
 *    @luatparam Vec2 v Vector target for the pilot.
 *    @luatparam[opt=1] boolean brake If true (or nil) brakes the pilot near target position,
 *              otherwise pops the task when it is about to brake.
 *    @luatparam[opt=1] boolean compensate If true (or nil) compensates for velocity, otherwise it
 *              doesn't. It only affects if brake is not set.
 * @luasee control
 * @luafunc moveto
 */
static int pilotL_moveto( lua_State *L )
{
   Pilot *p;
   Task *t;
   vec2 *vec;
   int brake, compensate;
   const char *tsk;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   vec = luaL_checkvector(L,2);
   if (lua_isnone(L,3))
      brake = 1;
   else
      brake = lua_toboolean(L,3);
   if (lua_isnone(L,4))
      compensate = 1;
   else
      compensate = lua_toboolean(L,4);

   /* Set the task. */
   if (brake) {
      tsk = "moveto";
   }
   else {
      if (compensate)
         tsk = "moveto_nobrake";
      else
         tsk = "moveto_nobrake_raw";
   }
   t        = pilotL_newtask( L, p, tsk );
   lua_pushvector( L, *vec );
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);

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
 * @luafunc face
 */
static int pilotL_face( lua_State *L )
{
   Pilot *p, *pt;
   vec2 *vec;
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
   towards = lua_toboolean(L,3);

   /* Set the task. */
   if (towards)
      t     = pilotL_newtask( L, p, "face_towards" );
   else
      t     = pilotL_newtask( L, p, "face" );
   if (pt != NULL) {
      lua_pushpilot(L, pt->id);
   }
   else {
      lua_pushvector(L, *vec);
   }
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);

   return 0;
}

/**
 * @brief Makes the pilot brake.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to brake.
 * @luasee control
 * @luafunc brake
 */
static int pilotL_brake( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
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
 * @luafunc follow
 */
static int pilotL_follow( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;
   int accurate;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   pt = luaL_validpilot(L,2);
   accurate = lua_toboolean(L,3);

   /* Set the task. */
   if (accurate == 0)
      t = pilotL_newtask( L, p, "follow" );
   else
      t = pilotL_newtask( L, p, "follow_accurate" );

   lua_pushpilot(L, pt->id);
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);

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
 * @luafunc attack
 */
static int pilotL_attack( lua_State *L )
{
   Pilot *p;
   Task *t;
   unsigned int pid;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   if (!lua_isnoneornil(L,2)) {
      Pilot *pt = luaL_validpilot(L,2);
      pid = pt->id;
   }
   else {
      pid = pilot_getNearestEnemy( p );
      if (pid == 0) /* No enemy found. */
         return 0;
   }

   /* Set the task. */
   t        = pilotL_newtask( L, p, "attack_forced" );
   lua_pushpilot(L, pid);
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);

   return 0;
}

/**
 * @brief Makes the pilot board another pilot.
 *
 * Pilot must be under manual control for this to work.
 *
 * @usage p:board( another_pilot ) -- Attack another pilot
 *
 *    @luatparam Pilot p Pilot to tell to board another pilot.
 *    @luatparam Pilot pt Target pilot to board
 * @luasee control
 * @luafunc board
 */
static int pilotL_board( lua_State *L )
{
   Pilot *p, *pt;
   Task *t;

   /* Get parameters. */
   p  = luaL_validpilot(L,1);
   pt = luaL_validpilot(L,2);

   /* Set the task. */
   t        = pilotL_newtask( L, p, "board" );
   lua_pushpilot(L, pt->id);
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);

   return 0;
}

/**
 * @brief Makes the pilot runaway from another pilot.
 *
 * By default the pilot tries to jump when running away.
 * Third argument is destination: if false or nil, destination is automatically chosen.
 * If true, the pilot does not jump nor land and stays in system.
 * If Jump is given, the pilot tries to use this jump to go hyperspace.
 * If Spob is given, the pilot tries to land on it.
 *
 * @usage p:runaway( p_enemy ) -- Run away from p_enemy
 * @usage p:runaway( p_enemy, true ) -- Run away from p_enemy but do not jump
 *    @luatparam Pilot p Pilot to tell to runaway from another pilot.
 *    @luatparam Pilot tp Target pilot to runaway from.
 *    @luatparam[opt=false] boolean|Jump|Spob destination.
 * @luasee control
 * @luafunc runaway
 */
static int pilotL_runaway( lua_State *L )
{
   /* Get parameters. */
   Pilot *p   = luaL_validpilot(L,1);
   Pilot *pt  = luaL_validpilot(L,2);

   /* Set the task depending on the last parameter. */
   if (lua_isnoneornil(L,3)) {
      Task *t = pilotL_newtask( L, p, "runaway" );
      lua_pushpilot(L, pt->id);
      t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
   }
   else {
      if (lua_isboolean(L,3)) {
         int nojump = lua_toboolean(L,3);
         Task *t = pilotL_newtask( L, p, (nojump) ? "runaway_nojump" : "runaway" );
         lua_pushpilot(L, pt->id);
         t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
      }
      else if (lua_isjump(L,3)) {
         LuaJump *lj = lua_tojump(L,3);
         Task *t = pilotL_newtask( L, p, "runaway_jump" );
         lua_newtable(L);
         lua_pushpilot(L, pt->id);
         lua_rawseti( L, -2, 1 );
         lua_pushjump(L, *lj);
         lua_rawseti( L, -2, 2 );
         t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
      }
      else if (lua_isspob(L,3)) {
         LuaSpob lp = lua_tospob(L,3);
         Task *t = pilotL_newtask( L, p, "runaway_land" );
         lua_newtable(L);
         lua_pushpilot(L, pt->id);
         lua_rawseti( L, -2, 1 );
         lua_pushspob(L, lp);
         lua_rawseti( L, -2, 2 );
         t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
      }
      else
         NLUA_INVALID_PARAMETER(L);
   }

   return 0;
}

/**
 * @brief Makes the pilot gather stuff.
 *
 * @usage p:gather( ) -- Try to gather stuff
 * @luasee control
 * @luafunc gather
 */
static int pilotL_gather( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   Task *t = pilotL_newtask( L, p, "gather" );
   t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
   return 0;
}

/**
 * @brief Checks to see if the pilot can currently hyperspace (as in has target jump and is in range).
 *
 *    @luatparam Pilot p Pilot to check if they can hyperspace.
 *    @luatreturn boolean Whether or not the pilot can hyperspace.
 * @luafunc canHyperspace
 */
static int pilotL_canHyperspace( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L, space_canHyperspace(p));
   return 1;
}

/**
 * @brief Tells the pilot to hyperspace.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to hyperspace.
 *    @luatparam[opt] System|Jump sys Optional System to jump to, uses random if nil.
 *    @luatparam[opt=false] boolean noshoot Forbids to shoot at targets with turrets while running away.
 * @luasee control
 * @luafunc hyperspace
 */
static int pilotL_hyperspace( lua_State *L )
{
   Pilot *p;
   Task *t;
   StarSystem *ss;
   JumpPoint *jp;
   LuaJump lj;
   int noshoot;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isjump(L,2))
      ss = system_getIndex( lua_tojump(L,2)->destid );
   else
      ss = (lua_isnoneornil(L,2)) ? NULL : luaL_validsystem(L,2);
   noshoot = lua_toboolean(L,3);

   /* Set the task. */
   if (noshoot)
      t = pilotL_newtask( L, p, "hyperspace" );
   else
      t = pilotL_newtask( L, p, "hyperspace_shoot" );

   if (ss == NULL)
      return 0;
   /* Find the jump. */
   for (int i=0; i < array_size(cur_system->jumps); i++) {
      jp = &cur_system->jumps[i];
      if (jp->target != ss)
         continue;
      /* Found target. */

      if (jp_isFlag( jp, JP_EXITONLY )) {
         NLUA_ERROR( L, _("Pilot '%s' can't jump out exit only jump '%s'"), p->name, ss->name );
         return 0;
      }

      /* Push jump. */
      lj.srcid  = cur_system->id;
      lj.destid = jp->targetid;
      lua_pushjump(L, lj);
      t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
      return 0;
   }
   /* Not found. */
   NLUA_ERROR( L, _("System '%s' is not adjacent to current system '%s'"), ss->name, cur_system->name );
   return 0;
}

/**
 * @brief Tells the pilot to try to stealth.
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to try to stealth.
 * @luasee control
 * @luafunc stealth
 */
static int pilotL_stealth( lua_State *L )
{
   /* Get parameters. */
   Pilot *p = luaL_validpilot(L,1);

   /* Set the task. */
   Task *t  = pilotL_newtask( L, p, "stealth" );
   t->dat   = luaL_ref(L, LUA_REGISTRYINDEX);

   return 0;
}

/**
 * @brief Tries to make the pilot stealth.
 *
 *    @luatparam Pilot p Pilot to try to make stealth.
 *    @luatreturn boolean Whether or not the pilot was able to stealth.
 * @luafunc tryStealth
 */
static int pilotL_tryStealth( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean( L, pilot_stealth(p) );
   return 1;
}

/**
 * @brief Tells the pilot to land
 *
 * Pilot must be under manual control for this to work.
 *
 *    @luatparam Pilot p Pilot to tell to land.
 *    @luatparam[opt] Spob spob Spob to land on, uses random if nil.
 *    @luatparam[opt=false] boolean noshoot Forbids to shoot at targets with turrets while running away.
 * @luasee control
 * @luafunc land
 */
static int pilotL_land( lua_State *L )
{
   Pilot *p;
   Task *t;
   Spob *pnt;
   int noshoot;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isnoneornil(L,2))
      pnt = NULL;
   else
      pnt = luaL_validspob( L, 2 );
   noshoot = lua_toboolean(L,3);

   /* Set the task. */
   if (noshoot)
      t = pilotL_newtask( L, p, "land" );
   else
      t = pilotL_newtask( L, p, "land_shoot" );

   if (pnt != NULL) {
      int i;
      /* Find the spob. */
      for (i=0; i < array_size(cur_system->spobs); i++) {
         if (cur_system->spobs[i] == pnt) {
            break;
         }
      }
      if (i >= array_size(cur_system->spobs)) {
         NLUA_ERROR( L, _("Spob '%s' not found in system '%s'"), pnt->name, cur_system->name );
         return 0;
      }

      p->nav_spob = i;
      if (p->id == PLAYER_ID)
         gui_setNav();

      lua_pushspob(L, pnt->id);
      t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
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
 * @luafunc hailPlayer
 */
static int pilotL_hailPlayer( lua_State *L )
{
   Pilot *p;
   int enable;

   /* Get parameters. */
   p = luaL_validpilot(L,1);
   if (lua_isnone(L,2))
      enable = 1;
   else
      enable = lua_toboolean(L,2);

   /* Set the flag. */
   if (enable) {
      /* Send message. */
      char c = pilot_getFactionColourChar( p );
      player_message( _("#%c%s#0 is hailing you."), c, p->name );

      /* Set flag. */
      pilot_setFlag( p, PILOT_HAILING );
      player_hailStart();
   }
   else
      pilot_rmFlag( p, PILOT_HAILING );

   return 0;
}

/**
 * @brief Sends a message to another pilot.
 *
 * Do not confuse with pilot.comm! This is meant to be used by AI and other scripts.
 *
 *    @luatparam Pilot p Pilot to send message.
 *    @luatparam Pilot|{Pilot,...} receiver Pilot(s) to receive message.
 *    @luatparam string type Type of message.
 *    @luaparam[opt] data Data to send with message.
 * @luafunc msg
 */
static int pilotL_msg( lua_State *L )
{
   Pilot *p;
   const char *type;
   unsigned int data;

   if (lua_isnoneornil(L,1))
      p = NULL;
   else
      p = luaL_validpilot(L,1);
   type = luaL_checkstring(L,3);
   data = lua_gettop(L) > 3 ? 4 : 0;

   if (!lua_istable(L,2)) {
      Pilot *receiver = luaL_validpilot(L,2);
      pilot_msg(p, receiver, type, data);
   }
   else {
      lua_pushnil(L);
      while (lua_next(L, 2) != 0) {
         Pilot *receiver = luaL_validpilot(L,-1);
         pilot_msg(p, receiver, type, data);
         lua_pop(L, 1);
      }
      lua_pop(L, 1);
   }

   return 0;
}

/**
 * @brief Gets a pilots mothership (only exists for deployed pilots). Guaranteed to exist or will be nil.
 *
 *    @luatparam Pilot p Pilot to get the mothership of.
 *    @luatreturn Pilot|nil The mothership or nil.
 * @luafunc mothership
 */
static int pilotL_mothership( lua_State *L )
{
   Pilot *p = luaL_validpilot(L, 1);
   if (p->dockpilot != 0) {
      const Pilot *l = pilot_get( p->dockpilot );
      if ((l == NULL) || pilot_isFlag( l, PILOT_DEAD )) {
         lua_pushnil(L);
      }
      else
         lua_pushpilot(L, p->dockpilot);
   }
   else
      lua_pushnil(L);
   return 1;
}

/**
 * @brief Gets a pilots leader. Guaranteed to exist or will be nil.
 *
 *    @luatparam Pilot p Pilot to get the leader of.
 *    @luatreturn Pilot|nil The leader or nil.
 * @luafunc leader
 */
static int pilotL_leader( lua_State *L )
{
   Pilot *p = luaL_validpilot(L, 1);
   if (p->parent != 0) {
      const Pilot *l = pilot_get( p->parent );
      if ((l == NULL) || pilot_isFlag( l, PILOT_DEAD )) {
         p->parent = 0; /* Clear parent for future calls. */
         lua_pushnil(L);
      }
      else
         lua_pushpilot(L, p->parent);
   }
   else
      lua_pushnil(L);
   return 1;
}

/**
 * @brief Set a pilots leader.
 *
 * If leader has a leader itself, the leader will instead be set to that
 * pilot's leader. The leader can not be set for deployed fighters or members
 * of the player's fleet.
 *
 *    @luatparam Pilot p Pilot to set the leader of.
 *    @luatparam Pilot|nil leader Pilot to set as leader.
 * @luafunc setLeader
 */
static int pilotL_setLeader( lua_State *L )
{
   Pilot *p = luaL_validpilot(L, 1);
   Pilot *prev_leader = pilot_get( p->parent );

   /* Remove from previous leader's follower list */
   if (prev_leader != NULL) {
      int found = 0;
      for (int i=0; i<array_size(prev_leader->escorts); i++) {
         Escort_t *e = &prev_leader->escorts[i];
         if (e->id != p->id)
            continue;
         if (e->type != ESCORT_TYPE_MERCENARY) {
            NLUA_ERROR(L,_("Trying to change the leader of pilot '%s' that is a deployed fighter or part of the player fleet!"), p->name);
            return 0;
         }
         escort_rmListIndex( prev_leader, i );
         found = 1;
         break;
      }
      if (!found)
         WARN(_("Pilot '%s' not found in followers of '%s'"), p->name, prev_leader->name );
   }

   /* Just clear parent, will already be gone from parent escort list. */
   if (lua_isnoneornil(L, 2)) {
      p->parent = 0;
   }
   else {
      PilotOutfitSlot* dockslot;
      Pilot *leader = luaL_validpilot(L, 2);

      /* Don't allow setting a pilot's leader to themselves. */
      if (p->id == leader->id)
         NLUA_ERROR(L,_("Trying to set pilot '%s' to be their own leader!"),p->name);

      if ((leader->parent != 0) && (leader->parent != p->id)) {
         Pilot *leader_leader =  pilot_get(leader->parent);
         if (leader_leader != NULL)
            leader = leader_leader;
      }

      p->parent = leader->id;

      /* Reset dock slot */
      dockslot = pilot_getDockSlot( p );
      if (dockslot != NULL) {
         dockslot->u.ammo.deployed--;
         p->dockpilot = 0;
         p->dockslot = -1;
      }

      /* TODO: Figure out escort type */
      escort_addList( leader, p->ship->name, ESCORT_TYPE_MERCENARY, p->id, 0 );

      /* If the pilot has followers, they should be given the new leader as well, and be added as escorts. */
      for (int i=array_size(p->escorts)-1; i>=0; i--) {
         Escort_t *e = &p->escorts[i];
         /* We don't want to deal with fighter bays this way. */
         if (e->type != ESCORT_TYPE_MERCENARY)
            continue;
         Pilot *pe = pilot_get( e->id );
         if (pe == NULL) {
            escort_rmListIndex( p, i ); /* MIght as well remove if they're not there. */
            continue;
         }

         /* Setting an escort as leader, so we clear the leader of the escort. */
         if (pe->id == p->parent) {
            escort_rmListIndex( p, i );
            pe->parent = 0;
            continue;
         }

         pe->parent = p->parent;

         /* Add escort to parent. */
         escort_addList( leader, pe->ship->name, e->type, pe->id, 0 );
         escort_rmListIndex( p, i );
      }
   }

   return 0;
}

/**
 * @brief Get all of a pilots followers.
 *
 *    @luatparam Pilot p Pilot to get the followers of.
 *    @luatreturn {Pilot,...} Table of followers.
 * @luafunc followers
 */
static int pilotL_followers( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L, 1);
   int idx = 1;

   lua_newtable(L);
   for (int i=0; i < array_size(p->escorts); i++) {
      /* Make sure the followers are valid. */
      Pilot *pe = pilot_get( p->escorts[i].id );
      if ((pe==NULL) || pilot_isFlag( pe, PILOT_DEAD ) || pilot_isFlag( pe, PILOT_HIDE ))
         continue;
      lua_pushpilot(L, p->escorts[i].id);
      lua_rawseti(L, -2, idx++);
   }

   return 1;
}

/**
 * @brief Clears the pilot's hooks.
 *
 * Clears all the hooks set on the pilot.
 *
 * @usage p:hookClear()
 *    @luatparam Pilot p Pilot to clear hooks.
 * @luafunc hookClear
 */
static int pilotL_hookClear( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   pilot_clearHooks( p );
   return 0;
}

static const CollPoly *getCollPoly( const Pilot *p )
{
   int k = p->ship->gfx_space->sx * p->tsy + p->tsx;
   return &p->ship->polygon[k];
}
/**
 * @brief Tests to see if two ships collide.
 *
 *    @luatparam Pilot p First pilot to check.
 *    @luatparam Pilot|Asteroid t Second object to check.
 *    @luatreturn Vec2|nil nil if no collision, or Vec2 with collision point if collided.
 * @luafunc collisionTest
 */
static int pilotL_collisionTest( lua_State *L )
{
   vec2 crash;
   const Pilot *p = luaL_validpilot(L,1);

   /* Asteroid treated separately. */
   if (lua_isasteroid(L,2)) {
      Asteroid *a = luaL_validasteroid( L, 2 );
      CollPoly rpoly;
      RotatePolygon( &rpoly, a->polygon, (float) a->ang );
      int ret = CollidePolygon( getCollPoly(p), &p->solid.pos,
            &rpoly, &a->pos, &crash );
      free(rpoly.x);
      free(rpoly.y);
      if (!ret)
         return 0;
      lua_pushvector( L, crash );
      return 1;
   }

   Pilot *t = luaL_validpilot(L,2);

   /* Shouldn't be invincible. */
   if (pilot_isFlag( t, PILOT_INVINCIBLE ))
      return 0;

   /* Shouldn't be landing or taking off. */
   if (pilot_isFlag( t, PILOT_LANDING) ||
         pilot_isFlag( t, PILOT_TAKEOFF ) ||
         pilot_isFlag( t, PILOT_NONTARGETABLE))
      return 0;

   /* Must be able to target. */
   if (!pilot_canTarget( t ))
      return 0;

   int ret = CollidePolygon( getCollPoly(p), &p->solid.pos, getCollPoly(t), &t->solid.pos, &crash );
   if (!ret)
      return 0;

   lua_pushvector( L, crash );
   return 1;
}

/**
 * @brief Damages a pilot.
 *
 *    @luatparam Pilot p Pilot being damaged.
 *    @luatparam number dmg Damage being done.
 *    @luatparam[opt=0.] number disable Disable being done.
 *    @luatparam[opt=0.] number penetration Penetration (in %).
 *    @luatparam[opt="normal"] string type Damage type being done.
 *    @luatparam[opt=nil] Pilot shooter Pilot doing the damage.
 *    @luatreturn number Amount of damage done.
 * @luafunc damage
 */
static int pilotL_damage( lua_State *L )
{
   Damage dmg;
   Pilot *p, *parent;
   double damage;

   p = luaL_validpilot(L,1);
   dmg.damage = luaL_checknumber(L,2);
   dmg.disable = luaL_optnumber(L,3,0.);
   dmg.penetration = luaL_optnumber(L,4,0.) / 100.;
   dmg.type = dtype_get( luaL_optstring(L,5,"normal") );
   parent = (lua_isnoneornil(L,6)) ? NULL : luaL_validpilot(L,6);

   damage = pilot_hit( p, NULL, parent, &dmg, NULL, LUA_NOREF, 1 );
   if (parent != NULL)
      weapon_hitAI( p, parent, damage );

   lua_pushnumber(L, damage);
   return 1;
}

/**
 * @brief Kills a pilot.
 *
 * Can fail to kill a pilot if they have a hook that regenerates them.
 *
 *    @luatparam Pilot p Pilot to kill.
 * @luafunc kill
 */
static int pilotL_kill( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   p->armour = -1.;
   pilot_dead( p, 0 );
   return 0;
}

/**
 * @brief Knocks back a pilot. It can either accept two pilots, or a pilot and an element represented by mass, velocity, and position.
 *
 * @usage pilota:knockback( pilotb, 0. ) -- Inelastic collision between pilota and pilotb
 * @usage pilota:knockback( 100, vec2.new(0,0) ) -- Elastic collision between a 100 mass object with no velocity and pilota
 *
 *    @luatparam Pilot p Pilot being knocked back.
 *    @luatparam number m Mass of object knocking back pilot.
 *    @luatparam Vec2 v Velocity of object knocking back pilot.
 *    @luatparam[opt=p:pos()] Vec2 p Position of the object knocking back the pilot.
 *    @luatparam[opt=1.] number e Coefficient of restitution. Use 1. for elastic collisions, and 0. for inelastic collisions.
 * @luafunc knockback
 */
static int pilotL_knockback( lua_State *L )
{
   Pilot *p1  = luaL_validpilot(L,1);
   double m1  = p1->solid.mass;
   vec2 *v1   = &p1->solid.vel;
   vec2 *x1   = &p1->solid.pos;
   Pilot *p2;
   double m2;
   vec2 *v2;
   vec2 *x2;
   double e;
   if (lua_ispilot(L,2)) {
      p2 = luaL_validpilot(L,2);
      m2 = p2->solid.mass;
      v2 = &p2->solid.vel;
      x2 = &p2->solid.pos;
      e  = luaL_optnumber(L,3,1.);
   }
   else {
      p2 = NULL;
      m2 = luaL_checknumber(L,2);
      v2 = luaL_checkvector(L,3);
      x2 = luaL_optvector(L,4,x1);
      e  = luaL_optnumber(L,5,1.);
   }

   /* Pure inlastic case. */
   if (e==0.) {
      double vx = (m1*v1->x + m2*v2->x) / (m1+m2);
      double vy = (m1*v1->y + m2*v2->y) / (m1+m2);
      vec2_cset( &p1->solid.vel, vx, vy );
      if (p2 != NULL)
         vec2_cset( &p2->solid.vel, vx, vy );
      return 0.;
   }

   /* Pure elastic. */
   double norm    = pow2(x1->x-x2->x) + pow2(x1->y-x2->y);
   double a1      = -e * (2.*m2)/(m1+m2);
   if (norm > 0.)
      a1 *= ((v1->x-v2->x)*(x1->x-x2->x) + (v1->y-v2->y)*(x1->y-x2->y)) / norm;

   vec2_cadd( &p1->solid.vel, a1*(x1->x-x2->x), a1*(x1->y-x2->y) );
   if (p2 != NULL) {
      double a2   = -e * (2.*m1)/(m2+m1);
      if (norm > 0.)
         a2 *= ((v2->x-v1->x)*(x2->x-x1->x) + (v2->y-v1->y)*(x2->y-x1->y)) / norm;
      vec2_cadd( &p2->solid.vel, a2*(x2->x-x1->x), a2*(x2->y-x1->y) );
   }

   /* Modulate. TODO this is probably totally wrong and needs fixing to be physicaly correct. */
   if (e != 1.) {
      double vx = (m1*v1->x + m2*v2->x) / (m1+m2);
      double vy = (m1*v1->y + m2*v2->y) / (m1+m2);
      vec2_cset( &p1->solid.vel, e*v1->x + (1.-e)*vx, e*v1->y + (1.-e)*vy );
      if (p2 != NULL)
         vec2_cset( &p2->solid.vel, e*v2->x + (1.-e)*vx, e*v2->y + (1.-e)*vy );
   }

   return 0;
}

/**
 * @brief Forces a recomputation of the pilots' stats.
 *
 *    @luatparam Pilot p Pilot to recalculate stats of.
 * @luafunc calcStats
 */
static int pilotL_calcStats( lua_State *L )
{
   Pilot *p = luaL_validpilot(L,1);
   pilot_calcStats( p );
   return 0;
}

/**
 * @brief Toggles the emitter marker.
 *
 * @usage pilot.showEmitters() -- Trail emitters are marked with crosses.
 * @usage pilot.showEmitters(false) -- Remove the markers.
 *
 *    @luatparam[opt=true] boolean state Whether to set or unset markers.
 * @luafunc showEmitters
 */
static int pilotL_showEmitters( lua_State *L )
{
   int state;

   /* Get state. */
   if (lua_gettop(L) > 0)
      state = lua_toboolean(L, 1);
   else
      state = 1;

   /* Toggle the markers. */
#if DEBUGGING
   if (state)
      debug_setFlag(DEBUG_MARK_EMITTER);
   else
      debug_rmFlag(DEBUG_MARK_EMITTER);
#else /* DEBUGGING */
   (void) state;
   NLUA_ERROR(L, _("Requires a debug build."));
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Peeks at a ship variable.
 *
 * @usage local exp = p:shipvarPeek( "exp" ) -- Checks the value of the "exp" ship var on the player's current ship
 *
 *    @luatparam Pilot p Pilot whose ship variable is being manipulated.
 *    @luatparam string varname Name of the variable to check value of.
 *    @luatparam[opt] string shipname Name of the ship to check variable of. Defaults to pilot's current ship.
 * @luafunc shipvarPeek
 */
static int pilotL_shipvarPeek( lua_State *L )
{
   const Pilot *p   = luaL_validpilot(L,1);
   const char *str  = luaL_checkstring(L,2);
   lvar *var        = lvar_get( p->shipvar, str );
   if (var != NULL)
      return lvar_push( L, var );
   return 0;
}

/**
 * @brief Pushes a ship variable.
 *
 *    @luatparam Pilot p Pilot whose ship variable is being manipulated.
 *    @luatparam string varname Name of the variable to set value of.
 *    @luaparam val Value to push.
 * @luafunc shipvarPush
 */
static int pilotL_shipvarPush( lua_State *L )
{
   Pilot *p         = luaL_validpilot(L,1);
   const char *str  = luaL_checkstring(L,2);
   lvar var         = lvar_tovar( L, str, 3 );
   if (p->shipvar==NULL)
      p->shipvar = array_create( lvar );
   lvar_addArray( &p->shipvar, &var, 1 );
   return 0;
}

/**
 * @brief Pops a ship variable.
 *
 *    @luatparam Pilot p Pilot whose ship variable is being manipulated.
 *    @luatparam string varname Name of the variable to pop.
 * @luafunc shipvarPop
 */
static int pilotL_shipvarPop( lua_State *L )
{
   Pilot *p         = luaL_validpilot(L,1);
   const char *str  = luaL_checkstring(L,2);
   lvar *var        = lvar_get( p->shipvar, str );
   if (var != NULL)
      lvar_rmArray( &p->shipvar, var );
   return 0;
}

/**
 * @brief Renders the pilot to a canvas
 *
 *    @luatparam Pilot p Pilot whose ship is being rendered.
 *    @luatreturn Canvas The canvas with the pilot drawn on it.
 * @luafunc render
 */
static int pilotL_render( lua_State *L )
{
   LuaCanvas_t lc;
   int w, h;
   Pilot *p = luaL_validpilot(L,1);

   /* TODO handle when effects make the ship render larger than it really is. */
   w = p->ship->gfx_space->sw;
   h = p->ship->gfx_space->sh;
   if (canvas_new( &lc, w, h ))
      NLUA_ERROR( L, _("Error setting up framebuffer!"));

   /* I'me really stumped at why we need to pass gl_screen here for it to work... */
   pilot_renderFramebuffer( p, lc.fbo, gl_screen.rw, gl_screen.rh );

   lua_pushcanvas( L, lc );
   return 1;
}

/**
 * @brief Renders the pilot to a canvas
 *
 *    @luatparam Pilot p Pilot whose ship is being rendered.
 *    @luatparam Canvas The canvas to draw to (uses bottom-left corner).
 *    @luatreturn number Width drawn.
 *    @luatreturn number Height drawn.
 * @luafunc renderTo
 */
static int pilotL_renderTo( lua_State *L )
{
   Pilot *p = luaL_validpilot( L, 1 );
   LuaCanvas_t *lc = luaL_checkcanvas( L, 2 );

   /* TODO handle when effects make the ship render larger than it really is. */
   if ((lc->tex->w < p->ship->gfx_space->sw) || (lc->tex->h < p->ship->gfx_space->sh))
      WARN(_("Canvas is too small to fully render '%s': %.0f x %.0f < %.0f x %.0f"),
            p->name, lc->tex->w, lc->tex->h,
            p->ship->gfx_space->sw, p->ship->gfx_space->sh );

   /* I'me really stumped at why we need to pass gl_screen here for it to work... */
   pilot_renderFramebuffer( p, lc->fbo, gl_screen.rw, gl_screen.rh );

   lua_pushnumber( L, p->ship->gfx_space->sw );
   lua_pushnumber( L, p->ship->gfx_space->sh );
   return 2;
}
