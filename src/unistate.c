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
#include "nfile.h"


//functions
assetStatePtr unistate_populateList(xmlNodePtr root);
int unistate_writeFile(assetStatePtr list, xmlTextWriterPtr writer);

//global pointer to unistate list
assetStatePtr unistateList = NULL;

/**
 * @brief Saves the current state of the universe (Used in save.c)
 * 
 * @param writer xml writer used when writing to save file
 * @return  0 on success
 */
int unistate_save(xmlTextWriterPtr writer)
{
   //write data to game save
   return unistate_writeFile(unistateList, writer);
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
   if(!list) return 0;
   if(!writer) return -1;
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

/**
 * @brief Loads the state of the universe into an xml file (Used in load.c)
 * 
 * @param rootNode pointer to the root node of the game save file we are loading off of
 * @return 0 on success
 */
int unistate_load(xmlNodePtr rootNode)
{
   if(!rootNode) return -1;
   
   xmlNodePtr elementNode = NULL;
   
   elementNode = rootNode->children;
   do {
      if(xml_isNode(elementNode, "uni_state"))
      {
	 //TODO: more error checking in this section
	 
	 //populate list
	 if(!(unistateList = unistate_populateList(elementNode))) 
	    return -1;
	 return 0;
      }
   } while(xml_nextNode(elementNode));
   //if it fell through to here then it didn't find what we were looking for
   return -2;
}
   
   