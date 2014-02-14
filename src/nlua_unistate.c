/**
 * @file nlua_unistate.c
 *
 * @brief Handles lua bindings to the state of the universe.
 *
 * This controls asset ownership and presense modifications, for example.
 */


#include "nlua_unistate.h"

#include "naev.h"

#include <lauxlib.h>

#include "console.h"
#include "unistate.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_col.h"
#include "console.h"
#include "nstring.h"

static int unistateL_changeowner(lua_State *L);
static int unistateL_changepresence(lua_State *L);
static int unistateL_dump(lua_State *L);
static const luaL_reg unistate_methods[] = {
   { "changeFaction", unistateL_changeowner },
   { "changePresence", unistateL_changepresence },
   { "dump", unistateL_dump },
   {0,0}
};
static const luaL_reg unistate_cond_methods[] = {
   { "changeFaction", unistateL_changeowner },
   { "changePresence", unistateL_changepresence },
   { "dump", unistateL_dump },
   {0,0}
};

/**
 * @brief Loads the unistate library.
 * 
 *   @param L State to load the library into.
 *   @param readonly Whether to use readonly functions or not
 *   @return 0 on success.
 */
int nlua_loadUnistate( lua_State *L, int readonly )
{
   /* Create the metatable */
   luaL_newmetatable(L, UNISTATE_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, unistate_cond_methods);
   else
      luaL_register(L, NULL, unistate_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, UNISTATE_METATABLE);

   return 0; /* No error */
}

/**
 * @brief Changes the faction ownership of a planet.
 * 
 */
int unistateL_changeowner(lua_State *L)
{
   //check for bad parameter
   if(!(lua_isstring(L,1) && lua_isstring(L,2)))
   {
      NLUA_INVALID_PARAMETER(L);
      return 1;
   }
   int ret;
   char buffer[PATH_MAX], *planet = NULL, *faction = NULL;
   planet = (char*)lua_tostring(L, 1);
   faction = (char*)lua_tostring(L, 2);
   //snprintf(buffer, sizeof(char) * (PATH_MAX - 1), "%s, %s", planet, faction);
   //cli_addMessage(buffer);
   
   if((ret = unistate_setFaction(planet, faction)) != 0)
   {
      switch(ret)
      {
         case -1:
            WARN("Error: could not add node to uni_state list");
            break;
         case -2:
            WARN("Error: invalid planet or faction passed");
            break;
         case -3:
            WARN("Error: NULL param passed");
            break;
         default:
            WARN("Error: An unknown error occurred");
      }
   }
      
   return 0;
}


/**
 * @brief Changes the faction presence in a system.
 * 
 */
int unistateL_changepresence(lua_State *L)
{
   NLUA_ERROR(L, "Not implemented yet :(");
   return 0;
}

/**
 * @brief Dumps all the changes from the default uni to the LUA console
 * 
 */
int unistateL_dump(lua_State *L)
{
   char buffer[PATH_MAX];
   if(unistateList == NULL)
   {
      cli_addMessage("No faction or presence changes found");
   }
   else
   {
      assetStatePtr cur = unistateList;
      do {
	 snprintf(buffer, sizeof(char) * (PATH_MAX - 1), "%s:", cur->name);
	 cli_addMessage(buffer);
	 snprintf(buffer, sizeof(char) * (PATH_MAX - 1), "   Faction: %s", cur->faction);
	 cli_addMessage(buffer);
	 snprintf(buffer, sizeof(char) * (PATH_MAX - 1), "   Presence: %i", cur->presence);
	 cli_addMessage(buffer);
      } while((cur = cur->next) != NULL);
   }
   return 0;
}

