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

#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"
#include "cond.h"


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


/*
 * Active events.
 */
static Event_t *event_active     = NULL; /**< Active events. */
static int event_nactive         = 0; /**< Number of active events. */
static int event_mactive         = 0; /**< Allocated space for active events. */


/*
 * Prototypes.
 */
static int event_parse( EventData_t *temp, const xmlNodePtr parent );
static void event_freeData( EventData_t *event );
static int event_runLua( Event_t *ev, const char *func );
static int event_create( int dataid );


/**
 * @brief Runs the event function.
 *
 *    @param eventid ID of the event to run Lua function on.
 *    @param func Name of the function to run.
 *    @return 0 on success.
 */
int event_run( int eventid, const char *func )
{
   Event_t *ev;

#ifdef DEBUGGING
   if ((eventid >= event_nactive) || (eventid < 0)) {
      WARN("Event ID not valid.");
      return -1;
   }
#endif /* DEBUGGING */

   ev = &event_active[ eventid ];

   return event_runLua( ev, func );
}


/**
 * @brief Runs the Lua for an event.
 */
static int event_runLua( Event_t *ev, const char *func )
{
   int ret;
   const char* err;
   lua_State *L;

   L = ev->L;

   ret = lua_pcall(L, 0, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      if (strcmp(err,"Event Done")!=0)
         WARN("Event '%s' -> '%s': %s",
               event_data[ev->data].name, func, (err) ? err : "unknown error");
      else
         ret = 1;
   }

   return ret;
}


/**
 * @brief Creates an event.
 *
 *    @param data Data to base event off of.
 */
static int event_create( int dataid )
{
   lua_State *L;
   uint32_t bufsize;
   char *buf;
   Event_t ev;
   EventData_t *data;

   /* Add the data. */
   ev.data = dataid;
   data = &event_data[dataid];

   /* Open the new state. */
   ev.L = nlua_newState();
   L = ev.L;
   nlua_loadStandard(L,0);

   /* Load file. */
   buf = ndata_read( data->lua, &bufsize );
   if (buf == NULL) {
      WARN("Event '%s' Lua script not found.", data->lua );
      return -1;
   }
   if (luaL_dobuffer(L, buf, bufsize, data->lua) != 0) {
      WARN("Error loading event file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check",
            data->lua, lua_tostring(L,-1));
      return -1;
   }
   free(buf);

   /* Run Lua. */
   event_runLua( &ev, "create" );

   /* For now destroy state.
    * @todo save state if needed. */
   lua_close(L);

   return 0;
}


/**
 * @brief Runs all the events matching a trigger.
 *
 *    @param trigger Trigger to match.
 */
void events_trigger( EventTrigger_t trigger )
{
   int i;

   for (i=0; i<event_ndata; i++) {
      /* Make sure trigger matches. */
      if (event_data[i].trigger != trigger)
         continue;

      /* Make sure chance is succeeded. */
      if (RNGF() > event_data[i].chance)
         continue;

      /* Test conditional. */
      if (!cond_check(event_data[i].cond))
         continue;

      /* Create the event. */
      event_create( i );
   }
}


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
   }
}


