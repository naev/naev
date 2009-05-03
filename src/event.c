/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file event.c
 *
 * @brief Handles internal events.
 *
 * Events are a lot like missions except the player has no control over when
 *  or how they happen.  They can simple do something simple or actually lead up
 *  to and open an entire set of missions.
 */


#include "event.h"

#include "naev.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"
#include "nlua_faction.h"
#include "rng.h"
#include "log.h"
#include "hook.h"
#include "ndata.h"
#include "nxml.h"
#include "faction.h"
#include "player.h"
#include "base64.h"
#include "space.h"


#define XML_EVENT_ID          "Events" /**< XML document identifier */
#define XML_EVENT_TAG         "event" /**< XML mission tag. */

#define EVENT_DATA            "dat/event.xml" /**< Path to missions XML. */
#define EVENT_LUA_PATH        "dat/events/" /**< Path to Lua files. */

#define EVENT_CHUNK           32 /**< Size to grow event data by. */


/**
 * @brief Event data structure.
 */
typedef struct EventData_s {
   char *name; /**< Name of the event. */
   char *lua; /**< Name of Lua file to use. */
   unsigned int flags; /**< Bit flags. */

   EventTrigger_t trigger; /**< What triggers the event. */
   char *cond; /**< Conditional Lua code to execute. */
   double chance; /**< Chance of appearing. */
} EventData_t;


/**
 * @brief Activated event structure.
 */
typedef struct Event_s {
   int data; /**< EventData parent. */
   lua_State *L; /**< Event Lua State. */
} Event_t;


/*
 * Event data.
 */
static EventData_t *event_data   = NULL; /**< Allocated event data. */
static int event_ndata           = 0; /**< Number of actual event data. */
static int event_mdata           = 0; /**< Number of event data allocated. */


/*
 * Active events.
 */
static Event_t *event_active     = NULL; /**< Active events. */
static int event_nactive         = 0; /**< Number of active events. */
static int event_mactive         = 0; /**< Allocated space for active events. */


/*
 * Prototypes.
 */



/**
 * @brief Loads up an event from an XML node.
 *
 *    @param temp Event to load up.
 *    @param parent Event parent node.
 *    @return 0 on success.
 */
static int event_parse( EventData_t *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   char str[PATH_MAX] = "\0";
   char *buf;

#ifdef DEBUGGING
   /* To check if mission is valid. */
   lua_State *L;
   int ret;
   uint32_t len;
#endif /* DEBUGGING */

   memset( temp, 0, sizeof(EventData_t) );

   /* get the name */
   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL)
      WARN("Event in "EVENT_DATA" has invalid or no name");

   node = parent->xmlChildrenNode;

   do { /* load all the data */
      if (xml_isNode(node,"lua")) {
         snprintf( str, PATH_MAX, EVENT_LUA_PATH"%s.lua", xml_get(node) );
         temp->lua = strdup( str );
         str[0] = '\0';

#ifdef DEBUGGING
         /* Check to see if syntax is valid. */
         L = luaL_newstate();
         buf = ndata_read( temp->lua, &len );
         ret = luaL_loadbuffer(L, buf, len, temp->name );
         if (ret == LUA_ERRSYNTAX) {
            WARN("Event Lua '%s' of mission '%s' syntax error: %s",
                  temp->name, temp->lua, lua_tostring(L,-1) );
         }
         free(buf);
         lua_close(L);
#endif /* DEBUGGING */
      }

      /* Trigger. */
      else if (xml_isNode(node,"trigger")) {
         buf = xml_get(node);
         
         if (strcmp(buf,"enter")==0)
            temp->trigger = EVENT_TRIGGER_ENTER;
         else
            WARN("Event '%s' has invalid 'trigger' parameter: %s", temp->name, buf);
      }

      /* Condition. */
      else if (xml_isNode(node,"cond"))
         temp->cond = xml_getStrd(node);

      /* Get chance. */
      else if (xml_isNode(node,"chance"))
         temp->chance = xml_getFloat(node) / 100.;

   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
   if (o) WARN("Mission '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->lua==NULL,"lua");
   MELEMENT(temp->cond==NULL,"cond");
   MELEMENT(temp->chance==0.,"chance");
   MELEMENT(temp->trigger==EVENT_TRIGGER_NULL,"trigger");
#undef MELEMENT

   return 0;
}



/**
 * @brief Loads all the events.
 *
 *    @return 0 on success.
 */
int events_load (void)
{
   int m;
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;
 
   /* Load the data. */
   buf = ndata_read( EVENT_DATA, &bufsize );
   if (buf == NULL) {
      WARN("Unable to read data from '%s'", EVENT_DATA);
      return -1;
   }

   /* Load the document. */
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      WARN("Unable to parse document '%s'", EVENT_DATA);
      return -1;
   }

   /* Get the root node. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_EVENT_ID)) {
      WARN("Malformed '"EVENT_DATA"' file: missing root element '"XML_EVENT_ID"'");
      return -1;
   }

   /* Get the first node. */
   node = node->xmlChildrenNode; /* first mission node */
   if (node == NULL) {
      WARN("Malformed '"EVENT_DATA"' file: does not contain elements");
      return -1;
   }

   m = 0;
   do {
      if (xml_isNode(node,XML_EVENT_TAG)) {

         /* See if must grow. */
         event_ndata++;
         if (event_ndata > m) {
            m += EVENT_CHUNK;
            event_data = realloc(event_data, sizeof(EventData_t)*m);
         }

         /* Load it. */
         event_parse( &event_data[event_ndata-1], node );
      }
   } while (xml_nextNode(node));

   /* Shrink to minimum. */
   event_data = realloc(event_data, sizeof(EventData_t)*event_ndata);

   /* Clean up. */                                                        
   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Event%s", event_ndata, (event_ndata==1) ? "" : "s" );

   return 0;
}


/**
 * @brief Frees an EventData structure.
 *
 *    @param event Event Data to free.
 */
static void event_freeData( EventData_t *event )
{
   if (event->name) {
      free(event->name);
      event->name = NULL;
   }
   if (event->lua) {
      free(event->lua);
      event->lua = NULL;
   }
   if (event->cond) {
      free(event->cond);
      event->cond = NULL;
   }
}


/**
 * @brief Exits the event subsystem.
 */
void events_exit (void)
{
   if (event_data != NULL) {
      event_freeData(event_data);
      free(event_data);
      event_data  = NULL;
      event_ndata = 0;
      event_mdata = 0;
   }
}


