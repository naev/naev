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
#include "nlua_evt.h"
#include "nlua_hook.h"
#include "nlua_tk.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"
#include "cond.h"
#include "hook.h"
#include "player.h"
#include "npc.h"


#define XML_EVENT_ID          "Events" /**< XML document identifier */
#define XML_EVENT_TAG         "event" /**< XML mission tag. */

#define EVENT_DATA            "dat/event.xml" /**< Path to missions XML. */
#define EVENT_LUA_PATH        "dat/events/" /**< Path to Lua files. */

#define EVENT_CHUNK           32 /**< Size to grow event data by. */


#define EVENT_FLAG_UNIQUE     (1<<0) /**< Unique event. */


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


/*
 * Event data.
 */
static EventData_t *event_data   = NULL; /**< Allocated event data. */
static int event_ndata           = 0; /**< Number of actual event data. */


/*
 * Active events.
 */
static unsigned int event_genid  = 0; /**< Event ID generator. */
static Event_t *event_active     = NULL; /**< Active events. */
static int event_nactive         = 0; /**< Number of active events. */
static int event_mactive         = 0; /**< Allocated space for active events. */


/*
 * Prototypes.
 */
static Event_t *event_get( unsigned int eventid );
static int event_alreadyRunning( int data );
static int event_parse( EventData_t *temp, const xmlNodePtr parent );
static void event_freeData( EventData_t *event );
static int event_create( int dataid );


/**
 * @brief Gets an event.
 */
static Event_t *event_get( unsigned int eventid )
{
   int i;
   Event_t *ev;

   /* Iterate. */
   for (i=0; i<event_nactive; i++) {
      ev = &event_active[i];
      if (ev->id == eventid)
         return ev;
   }

   WARN( "Event '%u' not found in stack.", eventid );
   return NULL;
}


/**
 * @brief Starts running a function, allows programmer to set up arguments.
 */
lua_State *event_runStart( unsigned int eventid, const char *func )
{
   Event_t *ev;

   ev = event_get( eventid );
   if (ev == NULL)
      return NULL;

   return event_setupLua( ev, func );
}


/**
 * @brief Runs a function previously set up with event_runStart.
 */
int event_runFunc( unsigned int eventid, const char *func, int nargs )
{
   Event_t *ev;

   ev = event_get( eventid );
   if (ev == NULL)
      return 0;

   return event_runLuaFunc( ev, func, nargs );
}


/**
 * @brief Runs the event function.
 *
 *    @param eventid ID of the event to run Lua function on.
 *    @param func Name of the function to run.
 *    @return 0 on success.
 */
int event_run( unsigned int eventid, const char *func )
{
   Event_t *ev;

   ev = event_get( eventid );
   if (ev == NULL)
      return -1;

   return event_runLua( ev, func );
}


/**
 * @brief Gets the name of the event data.
 *
 *    @param ev Event to get name of data from.
 *    @return Name of data ev has.
 */
const char *event_getData( unsigned int eventid )
{
   Event_t *ev;

   ev = event_get( eventid );
   if (ev == NULL)
      return NULL;

   return event_data[ ev->data ].name;
}


/**
 * @brief Checks to see if an event is unique.
 *
 *    @param eventid ID of event to see if is unique.
 *    @return 0 if isn't unique, 1 if is.
 */
int event_isUnique( unsigned int eventid )
{
   Event_t *ev;

   ev = event_get( eventid );
   if (ev == NULL)
      return -1;

   return !!(event_data[ ev->data ].flags & EVENT_FLAG_UNIQUE);
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
   Event_t *ev;
   EventData_t *data;

   /* Create the event. */
   event_nactive++;
   if (event_nactive > event_mactive) {
      event_mactive += EVENT_CHUNK;
      event_active = realloc( event_active, sizeof(Event_t) * event_mactive );
   }
   ev = &event_active[ event_nactive-1 ];
   memset( ev, 0, sizeof(Event_t) );
   ev->id = ++event_genid; /* Create unique ID. */

   /* Add the data. */
   ev->data = dataid;
   data = &event_data[dataid];

   /* Open the new state. */
   ev->L = nlua_newState();
   L = ev->L;
   nlua_loadStandard(L,0);
   nlua_loadEvt(L);
   nlua_loadHook(L);
   nlua_loadTk(L);

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
   event_runLua( ev, "create" );

   return 0;
}


/**
 * @brief Cleans up an event.
 *
 *    @param ev Event to clean up.
 */
static void event_cleanup( Event_t *ev )
{
   /* Destroy Lua. */
   lua_close(ev->L);

   /* Free hooks. */
   hook_rmEventParent(ev->id);

   /* Free NPC. */
   npc_rm_parentEvent(ev->id);
}


/**
 * @brief Removes an event by ID.
 *
 *    @param eventid ID of the event to remove.
 */
void event_remove( unsigned int eventid )
{
   int i;
   Event_t *ev;

   /* Find the event. */
   for (i=0; i<event_nactive; i++) {
      ev = &event_active[i];
      if (ev->id == eventid) {
         /* Clean up event. */
         event_cleanup(ev);

         /* Move memory. */
         memmove( &event_active[i], &event_active[i+1],
               sizeof(Event_t) * (event_nactive-i-1) );
         event_nactive--;
         return;
      }
   }

   WARN("Event ID '%u' not valid.", eventid);
}


/**
 * @brief Check to see if an event is already running.
 *
 *    @param data ID of data event to check if is already running.
 */
static int event_alreadyRunning( int data )
{
   int i;
   Event_t *ev;

   /* Find events. */
   for (i=0; i<event_nactive; i++) {
      ev = &event_active[i];
      if (ev->data == data) {
         return 1;
      }
   }

   return 0;
}


/**
 * @brief Runs all the events matching a trigger.
 *
 *    @param trigger Trigger to match.
 */
void events_trigger( EventTrigger_t trigger )
{
   int i, c;

   for (i=0; i<event_ndata; i++) {
      /* Make sure trigger matches. */
      if (event_data[i].trigger != trigger)
         continue;

      /* Make sure chance is succeeded. */
      if (RNGF() > event_data[i].chance)
         continue;

      /* Test uniqueness. */
      if ((event_data[i].flags & EVENT_FLAG_UNIQUE) &&
            (player_eventAlreadyDone( i ) || event_alreadyRunning(i)))
         continue;

      /* Test conditional. */
      if (event_data[i].cond != NULL) {
         c = cond_check(event_data[i].cond);
         if (c<0) {
            WARN("Conditional for event '%s' failed to run.", event_data[i].name);
            continue;
         }
         else if (!c)
            continue;
      }

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
   xmlNodePtr node, cur;
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

      /* Only check nodes. */
      xml_onlyNodes(node);

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

         continue;
      }

      /* Trigger. */
      else if (xml_isNode(node,"trigger")) {
         buf = xml_get(node);
         if (buf == NULL)
            WARN("Event '%s': Null trigger type.", temp->name);
         else if (strcmp(buf,"enter")==0)
            temp->trigger = EVENT_TRIGGER_ENTER;
         else if (strcmp(buf,"land")==0)
            temp->trigger = EVENT_TRIGGER_LAND;
         else if (strcmp(buf,"load")==0)
            temp->trigger = EVENT_TRIGGER_LOAD;
         else
            WARN("Event '%s' has invalid 'trigger' parameter: %s", temp->name, buf);

         continue;
      }

      /* Flags. */
      else if (xml_isNode(node,"flags")) { /* set the various flags */
         cur = node->children;
         do {
            if (xml_isNode(cur,"unique"))
               temp->flags |= EVENT_FLAG_UNIQUE;
         } while (xml_nextNode(cur));
         continue;
      }

      /* Condition. */
      xmlr_strd(node,"cond",temp->cond);

      /* Get chance. */
      xmlr_float(node,"chance",temp->chance);

      DEBUG("Unknown node '%s' in event '%s'", node->name, temp->name);
   } while (xml_nextNode(node));

   /* Process. */
   temp->chance /= 100.;

#define MELEMENT(o,s) \
   if (o) WARN("Mission '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->lua==NULL,"lua");
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
 * @brief Cleans up and removes active events.
 */
void events_cleanup (void)
{
   int i;

   /* Free active events. */
   for (i=0; i<event_nactive; i++)
      event_cleanup( &event_active[i] );
   if (event_active != NULL) {
      free(event_active);
   }
   event_active = NULL;
   event_nactive = 0;
   event_mactive = 0;
}


/**
 * @brief Exits the event subsystem.
 */
void events_exit (void)
{
   int i;

   events_cleanup();

   /* Free data. */
   for (i=0; i<event_ndata; i++)
      event_freeData(&event_data[i]);
   if (event_data != NULL) {
      event_freeData(event_data);
      free(event_data);
   }
   event_data  = NULL;
   event_ndata = 0;
}


/**
 * @brief Gets the event data id from name.
 *
 *    @param evdata Name of the data.
 *    @return ID matching dataname.
 */
int event_dataID( const char *evdata )
{
   int i;

   for (i=0; i<event_ndata; i++)
      if (strcmp(event_data[i].name, evdata)==0)
         return i;
   WARN("No event data found matching name '%s'.", evdata);
   return -1;
}


/**
 * @brief Gets the event data name from id.
 *
 *    @param dataid ID of the event data to get name of.
 *    @return Name of the event data.
 */
const char *event_dataName( int dataid )
{
   return event_data[dataid].name;
}

