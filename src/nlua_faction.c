/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_faction.c
 *
 * @brief Handles the Lua faction bindings.
 *
 * These bindings control the factions.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_faction.h"

#include "array.h"
#include "faction.h"
#include "log.h"
#include "nlua_colour.h"
#include "nlua_tex.h"
#include "nluadef.h"

/* Internal useful functions. */
static LuaFaction luaL_validfactionSilent( lua_State *L, int ind );
/* Faction metatable methods */
static int factionL_exists( lua_State *L );
static int factionL_get( lua_State *L );
static int factionL_player( lua_State *L );
static int factionL_eq( lua_State *L );
static int factionL_name( lua_State *L );
static int factionL_nameRaw( lua_State *L );
static int factionL_longname( lua_State *L );
static int factionL_areenemies( lua_State *L );
static int factionL_areallies( lua_State *L );
static int factionL_usesHiddenJumps( lua_State *L );
static int factionL_modplayer( lua_State *L );
static int factionL_modplayersingle( lua_State *L );
static int factionL_modplayerraw( lua_State *L );
static int factionL_setplayerstanding( lua_State *L );
static int factionL_playerstanding( lua_State *L );
static int factionL_playerstandingdefault( lua_State *L );
static int factionL_enemies( lua_State *L );
static int factionL_allies( lua_State *L );
static int factionL_logo( lua_State *L );
static int factionL_colour( lua_State *L );
static int factionL_isKnown( lua_State *L );
static int factionL_setKnown( lua_State *L );
static int factionL_isInvisible( lua_State *L );
static int factionL_isStatic( lua_State *L );
static int factionL_tags( lua_State *L );
static int factionL_dynAdd( lua_State *L );
static int factionL_dynAlly( lua_State *L );
static int factionL_dynEnemy( lua_State *L );
static const luaL_Reg faction_methods[] = {
   { "exists", factionL_exists },
   { "get", factionL_get },
   { "player", factionL_player },
   { "__eq", factionL_eq },
   { "__tostring", factionL_name },
   { "name", factionL_name },
   { "nameRaw", factionL_nameRaw },
   { "longname", factionL_longname },
   { "areEnemies", factionL_areenemies },
   { "areAllies", factionL_areallies },
   { "modPlayer", factionL_modplayer },
   { "modPlayerSingle", factionL_modplayersingle },
   { "modPlayerRaw", factionL_modplayerraw },
   { "setPlayerStanding", factionL_setplayerstanding },
   { "playerStanding", factionL_playerstanding },
   { "playerStandingDefault", factionL_playerstandingdefault },
   { "enemies", factionL_enemies },
   { "allies", factionL_allies },
   { "usesHiddenJumps", factionL_usesHiddenJumps },
   { "logo", factionL_logo },
   { "colour", factionL_colour },
   { "known", factionL_isKnown },
   { "setKnown", factionL_setKnown },
   { "invisible", factionL_isInvisible },
   { "static", factionL_isStatic },
   { "tags", factionL_tags },
   { "dynAdd", factionL_dynAdd },
   { "dynAlly", factionL_dynAlly },
   { "dynEnemy", factionL_dynEnemy },
   {0,0}
}; /**< Faction metatable methods. */

/**
 * @brief Loads the faction library.
 *
 *    @param env Environment to load faction library into.
 *    @return 0 on success.
 */
int nlua_loadFaction( nlua_env env )
{
   nlua_register(env, FACTION_METATABLE, faction_methods, 1);
   return 0; /* No error */
}

/**
 * @brief Lua bindings to deal with factions.
 *
 * Use like:
 * @code
 * f = faction.get( "Empire" )
 * if f:playerStanding() < 0 then
 *    -- player is hostile to "Empire"
 * end
 * @endcode
 *
 * @luamod faction
 */
/**
 * @brief Gets a faction if it exists.
 *
 * @usage f = faction.exists( "Empire" )
 *
 *    @luatparam string name Name of the faction to get if exists.
 *    @luatreturn Faction The faction matching name or nil if not matched.
 * @luafunc exists
 */
static int factionL_exists( lua_State *L )
{
   const char *s = luaL_checkstring(L,1);
   if (faction_exists(s)) {
      lua_pushfaction(L, faction_get(s));
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the faction based on its name.
 *
 * @usage f = faction.get( "Empire" )
 *
 *    @luatparam string name Name of the faction to get.
 *    @luatreturn Faction The faction matching name.
 * @luafunc get
 */
static int factionL_get( lua_State *L )
{
   LuaFaction f = luaL_validfaction(L,1);
   lua_pushfaction(L,f);
   return 1;
}

/**
 * @brief Gets the player's faction.
 *
 * @usage pf = faction.player()
 *
 *    @luareturn Faction The player's faction.
 * @luafunc player
 */
static int factionL_player( lua_State *L )
{
   lua_pushfaction(L,faction_player);
   return 1;
}

/**
 * @brief Gets faction at index.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
LuaFaction lua_tofaction( lua_State *L, int ind )
{
   return *((LuaFaction*) lua_touserdata(L,ind));
}

static LuaFaction luaL_validfactionSilent( lua_State *L, int ind )
{
   if (lua_isfaction(L,ind))
      return lua_tofaction(L,ind);
   else if (lua_isstring(L,ind))
      return faction_get( lua_tostring(L, ind) );
   luaL_typerror(L, ind, FACTION_METATABLE);
   return 0;
}

/**
 * @brief Gets faction (or faction name) at index, raising an error if type isn't a valid faction.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
LuaFaction luaL_validfaction( lua_State *L, int ind )
{
   int id = luaL_validfactionSilent( L, ind );
   if (id == -1)
      NLUA_ERROR(L,_("Faction '%s' not found in stack."), lua_tostring(L,ind) );
   return id;
}

/**
 * @brief Pushes a faction on the stack.
 *
 *    @param L Lua state to push faction into.
 *    @param faction Faction to push.
 *    @return Newly pushed faction.
 */
LuaFaction* lua_pushfaction( lua_State *L, LuaFaction faction )
{
   LuaFaction *f = (LuaFaction*) lua_newuserdata(L, sizeof(LuaFaction));
   *f = faction;
   luaL_getmetatable(L, FACTION_METATABLE);
   lua_setmetatable(L, -2);
   return f;
}
/**
 * @brief Checks to see if ind is a faction.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a faction.
 */
int lua_isfaction( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, FACTION_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief __eq (equality) metamethod for factions.
 *
 * You can use the '==' operator within Lua to compare factions with this.
 *
 * @usage if f == faction.get( "Dvaered" ) then
 *
 *    @luatparam Faction f Faction comparing.
 *    @luatparam Faction comp faction to compare against.
 *    @luatreturn boolean true if both factions are the same.
 * @luafunc __eq
 */
static int factionL_eq( lua_State *L )
{
   int a = luaL_validfaction(L,1);
   int b = luaL_validfaction(L,2);
   lua_pushboolean(L, a == b);
   return 1;
}

/**
 * @brief Gets the faction's translated short name.
 *
 * This translated name should be used for display purposes (e.g.
 * messages) where the shorter version of the faction's display name
 * should be used. It cannot be used as an identifier for the faction;
 * for that, use faction.nameRaw() instead.
 *
 * @usage shortname = f:name()
 *
 *    @luatparam Faction f The faction to get the name of.
 *    @luatreturn string The name of the faction.
 * @luafunc name
 */
static int factionL_name( lua_State *L )
{
   int f = luaL_validfaction(L,1);
   lua_pushstring(L, faction_shortname(f));
   return 1;
}

/**
 * @brief Gets the faction's raw / "real" (untranslated, internal) name.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to faction.get()). It should not be used for
 * display purposes; for that, use faction.name() or faction.longname()
 * instead.
 *
 * @usage name = f:nameRaw()
 *
 *    @luatparam Faction f The faction to get the name of.
 *    @luatreturn string The name of the faction.
 * @luafunc nameRaw
 */
static int factionL_nameRaw( lua_State *L )
{
   int f = luaL_validfaction(L,1);
   lua_pushstring(L, faction_name(f));
   return 1;
}

/**
 * @brief Gets the faction's translated long name.
 *
 * This translated name should be used for display purposes (e.g.
 * messages) where the longer version of the faction's display name
 * should be used. It cannot be used as an identifier for the faction;
 * for that, use faction.nameRaw() instead.
 *
 * @usage longname = f:longname()
 *    @luatparam Faction f Faction to get long name of.
 *    @luatreturn string The long name of the faction (translated).
 * @luafunc longname
 */
static int factionL_longname( lua_State *L )
{
   int f = luaL_validfaction(L,1);
   lua_pushstring(L, faction_longname(f));
   return 1;
}

/**
 * @brief Checks to see if f is an enemy of e.
 *
 * @usage if f:areEnemies( faction.get( "Dvaered" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam Faction e Faction to check if is an enemy.
 *    @luatreturn string true if they are enemies, false if they aren't.
 * @luafunc areEnemies
 */
static int factionL_areenemies( lua_State *L )
{
   int f  = luaL_validfaction(L,1);
   int ff = luaL_validfaction(L,2);
   lua_pushboolean(L, areEnemies( f, ff ));
   return 1;
}

/**
 * @brief Checks to see if f is an ally of a.
 *
 * @usage if f:areAllies( faction.get( "Pirate" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam faction a Faction to check if is an enemy.
 *    @luatreturn boolean true if they are enemies, false if they aren't.
 * @luafunc areAllies
 */
static int factionL_areallies( lua_State *L )
{
   int f  = luaL_validfaction(L,1);
   int ff = luaL_validfaction(L,2);
   lua_pushboolean(L, areAllies( f, ff ));
   return 1;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Also modifies standing with allies and enemies of the faction.
 *
 * @usage f:modPlayer( -5 ) -- Lowers faction by 5
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayer
 */
static int factionL_modplayer( lua_State *L )
{
   int f    = luaL_validfaction(L,1);
   double n = luaL_checknumber(L,2);
   faction_modPlayer( f, n, "script" );
   return 0;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Does not affect other faction standings.
 *
 * @usage f:modPlayerSingle( 10 )
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayerSingle
 */
static int factionL_modplayersingle( lua_State *L )
{
   int f    = luaL_validfaction(L,1);
   double n = luaL_checknumber(L,2);
   faction_modPlayerSingle( f, n, "script" );
   return 0;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Does not affect other faction standings and is not processed by the faction
 *  Lua script, so it indicates exactly the amount to be changed.
 *
 * @usage f:modPlayerRaw( 10 )
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayerRaw
 */
static int factionL_modplayerraw( lua_State *L )
{
   int f    = luaL_validfaction(L,1);
   double n = luaL_checknumber(L,2);
   faction_modPlayerRaw( f, n );
   return 0;
}

/**
 * @brief Sets the player's standing with the faction.
 *
 * @usage f:setPlayerStanding(70) -- Make player an ally
 *
 *    @luatparam Faction f Faction to set the player's standing for.
 *    @luatparam number value Value to set the player's standing to (from -100 to 100).
 * @luafunc setPlayerStanding
 */
static int factionL_setplayerstanding( lua_State *L )
{
   int f    = luaL_validfaction( L, 1 );
   double n = luaL_checknumber( L, 2 );
   faction_setPlayer( f, n );
   return 0;
}

/**
 * @brief Gets the player's standing with the faction.
 *
 * @usage if f:playerStanding() > 70 then -- Player is an ally
 *
 *    @luatparam Faction f Faction to get player's standing with.
 *    @luatreturn number The value of the standing and the human readable string.
 *    @luatreturn string The text corresponding to the standing (translated).
 * @luafunc playerStanding
 */
static int factionL_playerstanding( lua_State *L )
{
   int f    = luaL_validfaction( L, 1 );
   double n = faction_getPlayer( f );
   lua_pushnumber( L, n );
   lua_pushstring( L, faction_getStandingText( f ) );
   return 2;
}

/**
 * @brief Gets the player's default standing with the faction.
 *
 *    @luatparam Faction f Faction to get player's default standing with.
 *    @luatreturn number The value of the standing and the human readable string.
 * @luafunc playerStandingDefault
 */
static int factionL_playerstandingdefault( lua_State *L )
{
   int f    = luaL_validfaction( L, 1 );
   double n = faction_getPlayerDef( f );
   lua_pushnumber( L, n );
   return 1;
}

/**
 * @brief Gets the enemies of the faction.
 *
 * @usage for k,v in pairs(f:enemies()) do -- Iterates over enemies
 *
 *    @luatparam Faction f Faction to get enemies of.
 *    @luatreturn {Faction,...} A table containing the enemies of the faction.
 * @luafunc enemies
 */
static int factionL_enemies( lua_State *L )
{
   const int *factions;
   int f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getEnemies( f );
   for (int i=0; i<array_size(factions); i++) {
      lua_pushfaction(L, factions[i]); /* value */
      lua_rawseti(L,-2,i+1);
   }

   return 1;
}

/**
 * @brief Gets the allies of the faction.
 *
 * @usage for k,v in pairs(f:allies()) do -- Iterate over faction allies
 *
 *    @luatparam Faction f Faction to get allies of.
 *    @luatreturn {Faction,...} A table containing the allies of the faction.
 * @luafunc allies
 */
static int factionL_allies( lua_State *L )
{
   const int *factions;
   int f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getAllies( f );
   for (int i=0; i<array_size(factions); i++) {
      lua_pushfaction(L, factions[i]); /* value */
      lua_rawseti(L,-2,i+1);
   }

   return 1;
}

/**
 * @brief Gets whether or not a faction uses hidden jumps.
 *
 *    @luatparam Faction f Faction to get whether or not they use hidden jumps.
 *    @luatreturn boolean true if the faction uses hidden jumps, false otherwise.
 * @luafunc usesHiddenJumps
 */
static int factionL_usesHiddenJumps( lua_State *L )
{
   int f = luaL_validfaction(L,1);
   lua_pushboolean( L, faction_usesHiddenJumps(f) );
   return 1;
}

/**
 * @brief Gets the faction logo.
 *
 *    @luatparam Faction f Faction to get logo from.
 *    @luatreturn Tex The faction logo or nil if not applicable.
 * @luafunc logo
 */
static int factionL_logo( lua_State *L )
{
   int lf               = luaL_validfaction(L,1);
   const glTexture *tex = faction_logo( lf );
   if (tex == NULL)
      return 0;
   lua_pushtex( L, gl_dupTexture( tex ) );
   return 1;
}

/**
 * @brief Gets the faction colour.
 *
 *    @luatparam Faction f Faction to get colour from.
 *    @luatreturn Colour|nil The faction colour or nil if not applicable.
 * @luafunc colour
 */
static int factionL_colour( lua_State *L )
{
   int lf               = luaL_validfaction(L,1);
   const glColour *col  = faction_getColour(lf);
   if (col == NULL)
      return 0;
   lua_pushcolour( L, *col );
   return 1;
}

/**
 * @brief Checks to see if a faction is known by the player.
 *
 * @usage b = f:known()
 *
 *    @luatparam Faction f Faction to check if the player knows.
 *    @luatreturn boolean true if the player knows the faction.
 * @luafunc known
 */
static int factionL_isKnown( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   lua_pushboolean(L, faction_isKnown(fac));
   return 1;
}

/**
 * @brief Sets a faction's known state.
 *
 * @usage f:setKnown( false ) -- Makes faction unknown.
 *    @luatparam Faction f Faction to set known.
 *    @luatparam[opt=false] boolean b Whether or not to set as known.
 * @luafunc setKnown
 */
static int factionL_setKnown( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   int b   = lua_toboolean(L, 2);
   faction_setKnown( fac, b );
   return 0;
}

/**
 * @brief Checks to see if a faction is invisible the player.
 *
 * @usage b = f:invisible()
 *
 *    @luatparam Faction f Faction to check if is invisible to the player.
 *    @luatreturn boolean true if the faction is invisible to the player.
 * @luafunc invisible
 */
static int factionL_isInvisible( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   lua_pushboolean(L, faction_isInvisible(fac));
   return 1;
}

/**
 * @brief Checks to see if a faction has a static standing with the player.
 *
 * @usage b = f:static()
 *
 *    @luatparam Faction f Faction to check if has a static standing to the player.
 *    @luatreturn boolean true if the faction is static to the player.
 * @luafunc static
 */
static int factionL_isStatic( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   lua_pushboolean(L, faction_isStatic(fac));
   return 1;
}

/**
 * @brief Gets the tags a faction has.
 *
 * @usage for k,v in ipairs(f:tags()) do ... end
 * @usage if f:tags().likes_cheese then ... end
 *    @luatparam Faction f Faction to get tags of.
 *    @luatreturn table A table containing all the tags of the faction.
 * @luafunc tags
 */
static int factionL_tags( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   const char **tags = faction_tags( fac );
   lua_newtable(L);
   for (int i=0; i<array_size(tags); i++) {
      lua_pushstring(L, tags[i]);
      lua_pushboolean(L,1);
      lua_rawset(L,-3);
   }
   return 1;
}

/**
 * @brief Adds a faction dynamically. Note that if the faction already exists as a dynamic faction, the existing one is returned.
 *
 * @note Defaults to known.
 *
 *    @luatparam Faction|nil base Faction to base it off of or nil for new faction.
 *    @luatparam string name Name to give the faction.
 *    @luatparam[opt] string display Display name to give the faction.
 *    @luatparam[opt] table params Table of parameters. Options include "ai" (string) to set the AI to use, "clear_allies" (boolean) to clear all allies, and "clear_enemies" (boolean) to clear all enemies.
 * @luafunc dynAdd
 */
static int factionL_dynAdd( lua_State *L )
{
   LuaFaction fac, newfac;
   const char *name, *display, *ai;
   const glColour *colour;
   int clear_allies, clear_enemies;
   double player;
   int set_player;

   if (!lua_isnoneornil(L, 1))
      fac   = luaL_validfactionSilent(L,1); /* Won't error. */
   else
      fac   = -1;
   name     = luaL_checkstring(L,2);
   display  = luaL_optstring(L,3,name);
   set_player = 0;

   /* Just return existing and ignore the rest. */
   if (faction_exists(name)) {
      int f = faction_get(name);
      if (!faction_isDynamic(f))
         NLUA_ERROR(L,_("Trying to overwrite existing faction '%s' with dynamic faction!"),name);

      lua_pushfaction( L, f );
      return 1;
   }

   /* Parse parameters. */
   if (lua_istable(L,4)) {
      lua_getfield(L,4,"ai");
      ai = luaL_optstring(L,-1,NULL);
      lua_pop(L,1);

      lua_getfield(L,4,"clear_allies");
      clear_allies = lua_toboolean(L,-1);
      lua_pop(L,1);

      lua_getfield(L,4,"clear_enemies");
      clear_enemies = lua_toboolean(L,-1);
      lua_pop(L,1);

      lua_getfield(L,4,"player");
      if (lua_isnumber(L,-1)) {
         player = lua_tonumber(L,-1);
         set_player = 1;
      }
      lua_pop(L,1);

      lua_getfield(L,4,"colour");
      if (lua_isstring(L,-1))
         colour = col_fromName( lua_tostring(L,-1) );
      else
         colour = lua_tocolour( L, -1 );
      lua_pop(L,1);
   }
   else {
      ai             = NULL;
      clear_allies   = 0;
      clear_enemies  = 0;
      player         = 0.;
      colour         = NULL;
   }

   /* Create new faction. */
   newfac = faction_dynAdd( fac, name, display, ai, colour );

   /* Clear if necessary. */
   if (clear_allies)
      faction_clearAlly( newfac );
   if (clear_enemies)
      faction_clearEnemy( newfac );
   if (set_player)
      faction_setPlayer( newfac, player );

   lua_pushfaction( L, newfac );
   return 1;
}

/**
 * @brief Adds or removes allies to a faction. Only works with dynamic factions.
 *
 *    @luatparam Faction fac Faction to add ally to.
 *    @luatparam Faction ally Faction to add as an ally.
 *    @luatparam[opt=false] boolean remove Whether or not to remove the ally from the faction instead of adding it.
 * @luafunc dynAlly
 */
static int factionL_dynAlly( lua_State *L )
{
   LuaFaction fac, ally;
   int remove;
   fac      = luaL_validfaction(L,1);
   if (!faction_isDynamic(fac))
      NLUA_ERROR(L,_("Can only add allies to dynamic factions"));
   ally     = luaL_validfaction(L,2);
   remove   = lua_toboolean(L,3);
   if (remove)
      faction_rmAlly(fac, ally);
   else
      faction_addAlly(fac, ally);
   return 0;
}

/**
 * @brief Adds or removes enemies to a faction. Only works with dynamic factions.
 *
 *    @luatparam Faction fac Faction to add enemy to.
 *    @luatparam Faction enemy Faction to add as an enemy.
 *    @luatparam[opt=false] boolean remove Whether or not to remove the enemy from the faction instead of adding it.
 * @luafunc dynEnemy
 */
static int factionL_dynEnemy( lua_State *L )
{
   LuaFaction fac, enemy;
   int remove;
   fac      = luaL_validfaction(L,1);
   if (!faction_isDynamic(fac))
      NLUA_ERROR(L,_("Can only add allies to dynamic factions"));
   enemy    = luaL_validfaction(L,2);
   remove   = lua_toboolean(L,3);
   if (remove)
      faction_rmEnemy(fac, enemy);
   else
      faction_addEnemy(fac, enemy);
   return 0;
}
