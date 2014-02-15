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
#include "space.h"

//functions that aren't in the theader
assetStatePtr unistate_populateList(xmlNodePtr root);
int unistate_writeFile(assetStatePtr list, xmlTextWriterPtr writer);
assetStatePtr unistate_getNode(char *planet);
int unistate_addNode(char *planet, char *faction, int presence);
void unistate_freeList(assetStatePtr list);

//global pointer to unistate list
assetStatePtr unistateList = NULL;

/**
 * @brief Gets the unistate list (for use outside this file)
 */
assetStatePtr unistate_getList()
{
   return unistateList;
}

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
   assetStatePtr listHead = NULL, curElement = NULL;
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
         curElement->next = listHead;
         listHead = curElement;
         curElement->name = curElement->faction = NULL;
         curElement->presence = -1;
         //get info from file
         xmlr_attr(listNode, "name", curElement->name);
         //debug stuff
   #ifdef UNISTATE_DEBUG
         snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1),\
            "UniState Debug: Asset name '%s' parsed\n", curElement->name);
         logprintf(stdout, debugBuffer);
   #endif
         do {
            xml_onlyNodes(elementNode);
            xmlr_strd(elementNode, "faction", curElement->faction);
            xmlr_int(elementNode, "presence", curElement->presence);
         } while(xml_nextNode(elementNode));
         //more debug stuff
   #ifdef UNISTATE_DEBUG
         snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1),\
            "UniState Debug: Asset faction '%s' and Asset presence '%i' parsed\n",\
            (curElement->faction == NULL ? "<Default>" : curElement->faction), curElement->presence);
         logprintf(stdout, debugBuffer);
   #endif
      }
   } while(xml_nextNode(listNode));
   //return the list
   return listHead;
}

/**
 * @brief unistate quit routine. Used when closing the game.
 */
void unistate_quit()
{
   unistate_freeList(unistateList);
}

/**
 * @brief Frees a list made by unistate_populateList()
 * 
 * @param list list to be cleaned
 */
void unistate_freeList(assetStatePtr list)
{
   if(!list) return;
#ifdef UNISTATE_DEBUG
   logprintf(stdout, "Freeing unistate list...\n");
#endif
   unistate_freeList(list->next);
   if(list->faction) free(list->faction);
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
      snprintf(debugBuffer, sizeof(char) * (PATH_MAX - 1),\
         "UniState Debug: Adding entry: %s, %s, %i\n",\
         curEntry->name, curEntry->faction, curEntry->presence);
      logprintf(stdout, debugBuffer);
#endif
      //print data 
      xmlw_attr(writer, "name", "%s", curEntry->name);
      if(curEntry->faction != NULL)
         xmlw_elem(writer, "faction", "%s", curEntry->faction);
      if(curEntry->presence != -1)
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
   assetStatePtr cur = NULL;
   xmlNodePtr elementNode = NULL;
   Planet *p;
   int f_id;
   elementNode = rootNode->children;
   do {
      if(xml_isNode(elementNode, "uni_state"))
      {
	 //populate list
         if(!(unistateList = unistate_populateList(elementNode))) 
            return -1;
         cur = unistateList;
         while(cur != NULL)
         {
            //Get planet struct and faction ID. If either return errors, bail.
            if((p = planet_get(cur->name)) == NULL) 
            {
               WARN("Invalid planet or faction passed");
               cur = cur->next;
               continue;
            }
            //Change the faction of the planet
            if(cur->faction != NULL && (f_id = faction_get(cur->faction)) != -1) 
               planet_setFaction(p, f_id);
            //Change presence of planet
            if(cur->presence != -1)
               p->presenceAmount = (double)cur->presence;
            //update the universe
            space_reconstructPresences();
            //move along
            cur = cur->next;
         }
         return 0;
      }
   } while(xml_nextNode(elementNode));
   //if it fell through to here then it didn't find what we were looking for
   return -2;
}

/**
 * @brief Gets a unistate node by planet name
 * 
 * @param planet name of planet to search for
 * @return pointer to matching node on success, NULL on failure
 */
assetStatePtr unistate_getNode(char *planet)
{
   assetStatePtr cur = unistateList;
   //Let's hunt
   while(cur != NULL)
   {
      if(!strcmp(cur->name, planet))
         return cur;
      cur = cur->next;
   } 
   return NULL;
}

/**
 * @brief adds new node to unistate list
 * 
 * @param planet name of planet
 * @param faction name of faction planet is changed to, set to NULL if default 
 * @param presence amount of presence the planet is changed to, set to -1 if default
 * @return 0 on success
 */
int unistate_addNode(char *planet, char *faction, int presence)
{
   if(!planet)
      return -1;
   //check if already exists in list
   if(unistate_getNode(planet) != NULL)
   {
      WARN("Planet already in list; not adding new entry");
      return -1;
   }
   assetStatePtr node = NULL;
   //check for malloc errors
   if((node = malloc(sizeof(assetState))) == NULL)
   {
      WARN("Failed to allocate space for new list entry");
      return -1;
   }
   //add entries to new node
   node->name = strdup(planet);
   if(faction != NULL) 
      node->faction = strdup(faction);
   else
      node->faction = NULL;
   node->presence = presence;
   //add to global list
   node->next = unistateList;
   unistateList = node;
   //We're done!
   return 0;
}

/**
 * @brief Changes the ownership of the planet, and adds uni state change to list
 * 
 * @param planet name of planet
 * @param faction name of faction
 * @return 0 on success
 */
int unistate_setFaction(char *planet, char *faction)
{
   
   if(!planet || !faction) return -3;
   assetStatePtr node = NULL;
   Planet *p = NULL;
   int f_id;
   //Get planet struct and faction ID. If either return errors, bail.
   if((p = planet_get(planet)) == NULL || (f_id = faction_get(faction)) == -1) 
      return -2;
   //Change the faction of the planet
   planet_setFaction(p, f_id);
   //update the universe
   space_reconstructPresences();
   //does the planet already have mods?
   if((node = unistate_getNode(planet)) != NULL)
   {
      //if a faction mod hasn't been added yet
      if(node->faction == NULL)
         node->faction = strdup(faction);
      //else wipe old entry and make new one
      else
      {
         free(node->faction);
         node->faction = strdup(faction);
      }
      return 0;
   }
   //else we need to make a new node
   else
      return unistate_addNode(planet, faction, -1);
}
      
/**
 * @brief Changes the presence of a planet.
 * 
 * @param planet name of planet to be modified
 * @param presence presence value to be changed to 
 * @return 0 on success
 */
int unistate_setPresence(char *planet, int presence)
{
   
   if(!planet) return -3;
   assetStatePtr node = NULL;
   Planet *p = NULL;
   //Get planet struct. If it return errors, bail.
   if((p = planet_get(planet)) == NULL) 
      return -2;
   //Change the presence of the planet
   p->presenceAmount = (double)presence;
   //update the universe
   space_reconstructPresences();
   //does the planet already have mods?
   if((node = unistate_getNode(planet)) != NULL)
   {
      //if a faction mod hasn't been added yet
      node->presence = presence;
      return 0;
   }
   //else we need to make a new node
   else
      return unistate_addNode(planet, NULL, presence);
}
   
   
   
   