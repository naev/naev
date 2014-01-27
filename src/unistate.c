/**
 * @file unistate.c
 * 
 * @brief handles the state of the universe (heh), aka changes to asset ownership and such
 */


//uncomment this line for debug output to lua console
#define UNISTATE_DEBUG

#include "unistate.h"

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
int unistate_writeFile(assetStatePtr list, xmlTextWriterPtr writer);


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
   assetStatePtr list = NULL;
   char unistateFile[PATH_MAX];
   
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
   //write data to game save
   unistate_writeFile(list, writer);
   //free the memory allocated by the list
   unistate_freeList(list);
   //...and we're done here
   return 1;
   
}

/**
 * @brief Allocates space for and populates a list of asset mods from a unistate xml tree.
 * 
 * @param rootNode A node pointer to the root node of unistate tree
 * @return A pointer to the first node of the linked list.
 */
assetStatePtr unistate_populateList(xmlNodePtr root)
{
   assetStatePtr listHead = NULL, curElement = NULL, lastElement = NULL;
   xmlNodePtr elementNode = NULL, listNode = NULL;
#ifdef UNISTATE_DEBUG
   char debugBuffer[PATH_MAX];
#endif
   
   //no root node?
   if(!root) return NULL;
   //root has no children?
   if(!(listNode = root->children)) return NULL;
   //iterate over assets
   do {
      //if the node is named right
      if(xml_isNode(listNode, "asset"))
      {
	 elementNode = listNode->children;
	 //set up list element
	 curElement = malloc(sizeof(assetState));
	 if(!listHead) listHead = curElement;
	 curElement->next = NULL;
	 curElement->name = curElement->faction = NULL;
	 if(lastElement != NULL) lastElement->next = curElement;
	 lastElement = curElement;
	 //get info from file
	 xmlr_attr(listNode, "name", curElement->name);
	 //debug stuff
#ifdef UNISTATE_DEBUG
	 snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1), "UniState Debug: Asset name '%s' parsed\n", curElement->name);
	 cli_addMessage(debugBuffer);
#endif
	 do {
	    xml_onlyNodes(elementNode);
	    xmlr_strd(elementNode, "faction", curElement->faction);
	    xmlr_int(elementNode, "presence", curElement->presence);
	 } while(xml_nextNode(elementNode));
	 //more debug stuff
#ifdef UNISTATE_DEBUG
	 snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1), "UniState Debug: Asset faction '%s' and Asset presence '%i' parsed\n", curElement->faction, curElement->presence);
	 cli_addMessage(debugBuffer);
#endif
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
   unistate_freeList(list->next);
   free(list);
   return;
}

/**
 * @brief Writes unistate data to xml writer passed
 * 
 * @param list pointer to first element of list to be written
 * @param writer xml writer to be used when writing
 * @return 0 on success
 */
int unistate_writeFile(assetStatePtr list, xmlTextWriterPtr writer)
{
   if(!list || !writer) return -2;
   assetStatePtr curEntry = NULL;
#ifdef UNISTATE_DEBUG
   char debugBuffer[PATH_MAX];
#endif
   curEntry = list;
   //iterate over entries, and save them to gamesave
   xmlw_startElem(writer, "uni_state");
   do {
      //start code block
      xmlw_startElem(writer, "asset");
      //for debug purposes
#ifdef UNISTATE_DEBUG
      snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1), "UniState Debug: Adding entry: %s, %s, %i\n", curEntry->name, curEntry->faction, curEntry->presence);
      cli_addMessage(debugBuffer);
#endif
      //print data 
      xmlw_attr(writer, "name", "%s", curEntry->name);
      xmlw_elem(writer, "faction", "%s", curEntry->faction);
      xmlw_elem(writer, "presence", "%i", curEntry->presence);
      //close code block
      xmlw_endElem(writer);  
   } while((curEntry = curEntry->next) != NULL);
   xmlw_endElem(writer);
   return 0;
}

//int unistate_load(xmlNodePtr rootNode)
//{
//   if(!rootNode) return -2;
//   if(isNode(
   
   
   