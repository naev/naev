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
#include "nlua.h"
#include "nluadef.h"
#include "nlua_planet.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_ship.h"
#include "nlua_outfit.h"
#include "nlua_commodity.h"
#include "nlua_col.h"
#include "log.h"
#include "rng.h"
#include "land.h"
#include "map.h"
#include "nmath.h"
#include "nstring.h"
#include "nfile.h"

//functions
assetStatePtr unistate_populateList(xmlNodePtr root);
void unistate_freeList(assetStatePtr list);


static int unistateL_changeowner(lua_State *L);
static int unistateL_changepresence(lua_State *L);
static int unistateL_getpresence(lua_State *L);
static const luaL_reg unistate_methods[] = {
   { "changeAssetOwner", unistateL_changeowner },
   { "changeSysPresence", unistateL_changepresence },
   { "getSysPresence", unistateL_getpresence },
   {0,0}
};
static const luaL_reg unistate_cond_methods[] = {
   { "changeAssetOwner", unistateL_changeowner },
   { "changeSysPresence", unistateL_changepresence },
   { "getSysPresence", unistateL_getpresence },
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
   NLUA_ERROR(L, "Not implemented yet :(");
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
 * @brief Gets the net faction presence in a system.
 * 
 */
int unistateL_getpresence(lua_State *L)
{
   NLUA_ERROR(L, "Not implemented yet :(");
   return 0;
}

/**
 * @brief Saves the current state of the universe
 * 
 * @param writer xml writer used when writing to save file
 * @return  1 or 0 on success, -1 on failure
 */
int unistate_save(xmlTextWriterPtr writer)
{
   xmlDocPtr doc;
   xmlNodePtr rootNode = NULL;
   assetStatePtr list = NULL, curEntry = NULL;
   char unistateFile[PATH_MAX], debugBuffer[PATH_MAX];
  
   //get the unistate file, if it doesn't exists return
   snprintf(unistateFile, sizeof(char)*(PATH_MAX-1), "%s/unistate.xml", nfile_dataPath());
   if(!(doc = xmlParseFile(unistateFile))) return 0;
  
   //if the root element isn't "unistate", then this is a different file than we want.
   rootNode = xmlDocGetRootElement(doc);
   if(!xml_isNode(rootNode,"unistate"))
   {
      WARN("Invalid unistate file at %s", unistateFile);
      xmlFreeDoc(doc);
      return -2;
   }
   
   //get all the entries in our unistate file
   list = unistate_populateList(rootNode);
   //free our document pointer
   xmlFreeDoc(doc);
   //checks for empty list
   if(!list) return 0;
   else curEntry = list;
   //iterate over entries, and save them to gamesave
   xmlw_startElem(writer, "uni_state");
   do {
      //start code block
      xmlw_startElem(writer, "asset_state");
      //for debug purposes
      snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1), "Adding entry to gamesave: %s, %s, %i\n", curEntry->name, curEntry->faction, curEntry->presence);
      cli_addMessage(debugBuffer);
      //print data 
      xmlw_elem(writer, "name", "%s", curEntry->name);
      xmlw_elem(writer, "faction", "%s", curEntry->faction);
      xmlw_elem(writer, "presence", "%i", curEntry->presence);
      //close code block
      xmlw_endElem(writer);  
   } while((curEntry = curEntry->next) != NULL);
   xmlw_endElem(writer);
   //free the memory allocated by the list
   unistate_freeList(list);
   //...and we're done here
   return 1;
   
}

/**
 * @brief Allocates space for and populates a list of asset mods from unistate.xml.
 * 
 * @param rootNode A node pointer to the root node of unistate.xml
 * @return A pointer to the first node of the linked list.
 */
assetStatePtr unistate_populateList(xmlNodePtr root)
{
   assetStatePtr listHead = NULL, curElement = NULL, lastElement = NULL;
   xmlNodePtr elementNode = NULL, listNode = NULL;
   
   //no root node?
   if(!root) return NULL;
   //root has no children?
   if(!(listNode = root->children)) return NULL;
   //iterate over assets
   do {
      //if the node is named right
      if(xml_isNode(listNode, "asset_state"))
      {
	 elementNode = listNode->xmlChildrenNode;
	 //set up list element
	 curElement = malloc(sizeof(assetState));
	 if(!listHead) listHead = curElement;
	 curElement->next = NULL;
	 if(lastElement != NULL) lastElement->next = curElement;
	 lastElement = curElement;
	 //get info from file
	 do {
	    xmlr_strd(elementNode, "name", curElement->name);
	    xmlr_strd(elementNode, "faction", curElement->faction);
	    xmlr_int(elementNode, "presence", curElement->presence);
	 } while(xml_nextNode(elementNode));
      }
   } while(xml_nextNode(listNode));
   //return the list
   return listHead;
}

/**
 * @brief Frees a list made by unistate_populateList()
 * 
 * @param list list to be cleaned
 */
void unistate_freeList(assetStatePtr list)
{
   if(!list) return;
   if(list->next != NULL) unistate_freeList(list->next);
   free(list);
   return;
}

