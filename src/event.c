/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file event.c
 *
 * @brief Handles internal events.
 *
 * Events are a lot like events except the player has no control over when
 *  or how they happen.  They can simple do something simple or actually lead up
 *  to and open an entire set of events.
 */
/** @cond */
#include "nstring.h"
#include <stdint.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "event.h"

#include "conf.h"
#include "array.h"
#include "cond.h"
#include "hook.h"
#include "log.h"
#include "land.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_audio.h"
#include "nlua_bkg.h"
#include "nlua_camera.h"
#include "nlua_evt.h"
#include "nlua_hook.h"
#include "nlua_music.h"
#include "nlua_tex.h"
#include "nlua_tk.h"
#include "nluadef.h"
#include "npc.h"
#include "nxml.h"
#include "nxml_lua.h"
#include "player.h"
#include "rng.h"

#define XML_EVENT_ID          "Events" /**< XML document identifier */
#define XML_EVENT_TAG         "event" /**< XML event tag. */

#define EVENT_FLAG_UNIQUE     (1<<0) /**< Unique event. */

/**
 * @brief Event data structure.
 */
typedef struct EventData_ {
   char *name; /**< Name of the event. */
   char *sourcefile; /**< Source file code. */
   char *lua; /**< Lua code. */
   unsigned int flags; /**< Bit flags. */

   /* For specific cases. */
   char *spob; /**< Spob name. */
   char *system; /**< System name. */
   char *chapter; /**< Chapter name. */
   int *factions; /**< Faction checks. */
   pcre2_code *chapter_re; /**< Compiled regex chapter if applicable. */

   EventTrigger_t trigger; /**< What triggers the event. */
   char *cond; /**< Conditional Lua code to execute. */
   double chance; /**< Chance of appearing. */
   int priority; /**< Event priority: 0 = main plot, 5 = default, 10 = insignificant. */

   char **tags; /**< Tags. */
} EventData;

/*
 * Event data.
 */
static EventData *event_data   = NULL; /**< Allocated event data. */

/*
 * Active events.
 */
static unsigned int event_genid  = 0; /**< Event ID generator. */
static Event_t *event_active     = NULL; /**< Active events. */

/*
 * Prototypes.
 */
static unsigned int event_genID (void);
static int event_cmp( const void* a, const void* b );
static int event_parseFile( const char* file, EventData *temp );
static int event_parseXML( EventData *temp, const xmlNodePtr parent );
static void event_freeData( EventData *event );
static int event_create( int dataid, unsigned int *id );
int events_saveActive( xmlTextWriterPtr writer );
int events_loadActive( xmlNodePtr parent );
static int events_parseActive( xmlNodePtr parent );

/**
 * @brief Gets an event.
 */
Event_t *event_get( unsigned int eventid )
{
   /* Iterate. */
   for (int i=0; i<array_size(event_active); i++) {
      Event_t *ev = &event_active[i];
      if (ev->id == eventid)
         return ev;
   }

   return NULL;
}

/**
 * @brief Starts an event.
 *
 *    @param name Name of the event to start.
 *    @param[out] id ID of the newly created event.
 *    @return 0 on success, <0 on error.
 */
int event_start( const char *name, unsigned int *id )
{
   int ret, edat;
   unsigned int eid;

   edat = event_dataID( name );
   if (edat < 0)
      return -1;
   eid  = 0;
   ret  = event_create( edat, &eid );

   if ((ret == 0) && (id != NULL))
      *id = eid;
   return ret;
}

/**
 * @brief Gets the name of the event data.
 *
 *    @param eventid Event to get name of data from.
 *    @return Name of data ev has.
 */
const char *event_getData( unsigned int eventid )
{
   Event_t *ev = event_get( eventid );
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
   Event_t *ev = event_get( eventid );
   if (ev == NULL)
      return -1;

   return !!(event_data[ ev->data ].flags & EVENT_FLAG_UNIQUE);
}

/**
 * @brief Generates a new event ID.
 */
static unsigned int event_genID (void)
{
   unsigned int id;
   do {
      id = ++event_genid; /* Create unique ID. */
   } while (event_get(id) != NULL);
   return id;
}

/**
 * @brief Creates an event.
 *
 *    @param dataid Data to base event off of.
 *    @param id ID to use (0 to generate).
 *    @return 0 on success.
 */
static int event_create( int dataid, unsigned int *id )
{
   Event_t *ev;
   EventData *data;
   unsigned int eid;

   if (event_active==NULL)
      event_active = array_create( Event_t );

   /* Create the event. */
   ev = &array_grow( &event_active );
   memset( ev, 0, sizeof(Event_t) );
   if ((id != NULL) && (*id != 0))
      eid = *id;
   else
      eid = event_genID();
   ev->id = eid;

   /* Add the data. */
   ev->data = dataid;
   data = &event_data[dataid];

   /* Open the new state. */
   ev->env = nlua_newEnv();
   nlua_loadStandard(ev->env);
   nlua_loadEvt(ev->env);
   nlua_loadHook(ev->env);
   nlua_loadCamera(ev->env);
   nlua_loadTex(ev->env);
   nlua_loadBackground(ev->env);
   nlua_loadMusic(ev->env);
   nlua_loadTk(ev->env);

   /* Create the "mem" table for persistence. */
   lua_newtable(naevL);
   nlua_setenv(naevL, ev->env, "mem");

   /* Load file. */
   if (nlua_dobufenv(ev->env, data->lua, strlen(data->lua), data->sourcefile) != 0) {
      WARN(_("Error loading event file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
            data->sourcefile, lua_tostring(naevL,-1));
      return -1;
   }

   /* Run Lua. */
   if ((id==NULL) || (*id==0))
      event_run( ev->id, "create" );
   if (id != NULL)
      *id = eid; /* The ev pointer can change and be invalidated in event_run,
                    so we pass eid instead of ev->id. */

   return 0;
}

/**
 * @brief Cleans up an event.
 *
 *    @param ev Event to clean up.
 */
static void event_cleanup( Event_t *ev )
{
   /* Free lua env. */
   nlua_freeEnv(ev->env);

   /* Free hooks. */
   hook_rmEventParent(ev->id);

   /* Free NPC. */
   npc_rm_parentEvent(ev->id);

   /* Free claims. */
   if (ev->claims != NULL)
      claim_destroy( ev->claims );
}

/**
 * @brief Removes an event by ID.
 *
 *    @param eventid ID of the event to remove.
 */
void event_remove( unsigned int eventid )
{
   /* Find the event. */
   for (int i=0; i<array_size(event_active); i++) {
      Event_t *ev = &event_active[i];
      if (ev->id == eventid) {
         /* Clean up event. */
         event_cleanup(ev);

         /* Move memory. */
         array_erase( &event_active, &event_active[i], &event_active[i+1] );
         return;
      }
   }

   WARN(_("Event ID '%u' not valid."), eventid);
}

/**
 * @brief Checks to see if an event should be saved.
 */
int event_save( unsigned int eventid )
{
   Event_t *ev = event_get(eventid);
   if (ev == NULL)
      return 0;
   return ev->save;
}

/**
 * @brief Check to see if an event is already running.
 *
 *    @param data ID of data event to check if is already running.
 */
int event_alreadyRunning( int data )
{
   /* Find events. */
   for (int i=0; i<array_size(event_active); i++) {
      Event_t *ev = &event_active[i];
      if (ev->data == data)
         return 1;
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
   int created = 0;
   for (int i=0; i<array_size(event_data); i++) {
      EventData *ed = &event_data[i];

      if (naev_isQuit())
         return;

      /* Make sure trigger matches. */
      if (ed->trigger != trigger)
         continue;

      /* Spob. */
      if ((trigger==EVENT_TRIGGER_LAND || trigger==EVENT_TRIGGER_LOAD) && (ed->spob != NULL) && (strcmp(ed->spob,land_spob->name)!=0))
         continue;

      /* System. */
      if ((ed->system != NULL) && (strcmp(ed->system,cur_system->name)!=0))
         continue;

      /* Make sure chance is succeeded. */
      if (RNGF() > ed->chance)
         continue;

      /* Test uniqueness. */
      if ((ed->flags & EVENT_FLAG_UNIQUE) &&
            (player_eventAlreadyDone(i) || event_alreadyRunning(i)))
         continue;

      /* Test factions. */
      if (ed->factions != NULL) {
         int fct, match = 0;
         if (trigger==EVENT_TRIGGER_ENTER)
            fct = cur_system->faction;
         else if (trigger==EVENT_TRIGGER_LOAD || trigger==EVENT_TRIGGER_LAND)
            fct = land_spob->presence.faction;
         else
            match = -1; /* Don't hae to check factions. */

         if (match==0) {
            for (int j=0; j<array_size(ed->factions); j++) {
               if (fct == ed->factions[j]) {
                  match = 1;
                  break;
               }
            }
            if (!match)
               continue;
         }
      }

      /* If chapter, must match chapter regex. */
      if (ed->chapter_re != NULL) {
         pcre2_match_data *match_data = pcre2_match_data_create_from_pattern( ed->chapter_re, NULL );
         int rc = pcre2_match( ed->chapter_re, (PCRE2_SPTR)player.chapter, strlen(player.chapter), 0, 0, match_data, NULL );
         pcre2_match_data_free( match_data );
         if (rc < 0) {
            switch (rc) {
               case PCRE2_ERROR_NOMATCH:
                  continue;
               default:
                  WARN(_("Matching error %d"), rc );
                  break;
            }
            continue;
         }
         else if (rc == 0)
            continue;
      }

      /* Test conditional. */
      if (ed->cond != NULL) {
         int c = cond_check(ed->cond);
         if (c<0) {
            WARN(_("Conditional for event '%s' failed to run."), ed->name);
            continue;
         }
         else if (!c)
            continue;
      }

      /* Create the event. */
      event_create( i, NULL );
      created++;
   }

   /* Run claims if necessary. */
   if (created)
      claim_activateAll();
}

/**
 * @brief Loads up an event from an XML node.
 *
 *    @param temp Event to load up.
 *    @param parent Event parent node.
 *    @return 0 on success.
 */
static int event_parseXML( EventData *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;

   memset( temp, 0, sizeof(EventData) );

   /* Defaults. */
   temp->trigger = EVENT_TRIGGER_NULL;

   /* get the name */
   xmlr_attr_strd(parent, "name", temp->name);
   if (temp->name == NULL)
      WARN(_("Event in %s has invalid or no name"), EVENT_DATA_PATH);

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      /* Only check nodes. */
      xml_onlyNodes(node);

      xmlr_strd(node,"spob",temp->spob);
      xmlr_strd(node,"system",temp->system);
      xmlr_strd(node,"chapter",temp->chapter);

      xmlr_strd(node,"cond",temp->cond);
      xmlr_float(node,"chance",temp->chance);
      xmlr_int(node,"priority",temp->priority);

      if (xml_isNode(node,"faction")) {
         if (temp->factions == NULL)
            temp->factions = array_create( int );
         array_push_back( &temp->factions, faction_get( xml_get(node) ) );
         continue;
      }

      /* Trigger. */
      if (xml_isNode(node,"location")) {
         char *buf = xml_get(node);
         if (buf == NULL)
            WARN(_("Event '%s': Null trigger type."), temp->name);
         else if (strcmp(buf,"enter")==0)
            temp->trigger = EVENT_TRIGGER_ENTER;
         else if (strcmp(buf,"land")==0)
            temp->trigger = EVENT_TRIGGER_LAND;
         else if (strcmp(buf,"load")==0)
            temp->trigger = EVENT_TRIGGER_LOAD;
         else if (strcmp(buf,"none")==0)
            temp->trigger = EVENT_TRIGGER_NONE;
         else
            WARN(_("Event '%s' has invalid 'trigger' parameter: %s"), temp->name, buf);

         continue;
      }

      /* Flags. */
      else if (xml_isNode(node,"unique")) { /* unique event. */
         temp->flags |= EVENT_FLAG_UNIQUE;
         continue;
      }

      /* Tags. */
      if (xml_isNode(node,"tags")) {
         xmlNodePtr cur = node->children;
         temp->tags = array_create( char* );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "tag")) {
               char *tmp = xml_get(cur);
               if (tmp != NULL)
                  array_push_back( &temp->tags, strdup(tmp) );
               continue;
            }
            WARN(_("Event '%s' has unknown node in tags '%s'."), temp->name, cur->name );
         } while (xml_nextNode(cur));
         continue;
      }

      /* Notes for the python mission mapping script. */
      else if (xml_isNode(node,"notes"))
         continue;

      WARN(_("Unknown node '%s' in event '%s'"), node->name, temp->name);
   } while (xml_nextNode(node));

   /* Process. */
   temp->chance /= 100.;

   if (temp->chapter != NULL) {
      int errornumber;
      PCRE2_SIZE erroroffset;
      temp->chapter_re = pcre2_compile( (PCRE2_SPTR)temp->chapter, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL );
      if (temp->chapter_re == NULL){
         PCRE2_UCHAR buffer[256];
         pcre2_get_error_message( errornumber, buffer, sizeof(buffer) );
         WARN(_("Mission '%s' chapter PCRE2 compilation failed at offset %d: %s"), temp->name, (int)erroroffset, buffer );
      }
   }

#define MELEMENT(o,s) \
   if (o) WARN(_("Event '%s' missing/invalid '%s' element"), temp->name, s)
   MELEMENT(temp->trigger==EVENT_TRIGGER_NULL,"location");
   MELEMENT((temp->trigger!=EVENT_TRIGGER_NONE) && (temp->chance==0.),"chance");
#undef MELEMENT

   return 0;
}

static int event_cmp( const void* a, const void* b )
{
   const EventData *ea, *eb;
   ea = (const EventData*) a;
   eb = (const EventData*) b;
   if (ea->priority < eb->priority)
      return -1;
   else if (ea->priority > eb->priority)
      return +1;
   return strcmp( ea->name, eb->name );
}

/**
 * @brief Loads all the events.
 *
 *    @return 0 on success.
 */
int events_load (void)
{
   char **event_files = ndata_listRecursive( EVENT_DATA_PATH );
   Uint32 time = SDL_GetTicks();

   /* Run over events. */
   event_data = array_create_size( EventData, array_size( event_files ) );
   for (int i=0; i < array_size( event_files ); i++) {
      event_parseFile( event_files[i], NULL );
      free( event_files[ i ] );
   }
   array_free( event_files );
   array_shrink( &event_data );

#ifdef DEBUGGING
   for (int i=0; i<array_size(event_data); i++) {
      EventData *ed = &event_data[i];
      for (int j=i+1; j<array_size(event_data); j++)
         if (strcmp( ed->name, event_data[j].name )==0)
            WARN(_("Duplicate event '%s'!"), ed->name);
   }
#endif /* DEBUGGING */

   /* Sort based on priority so higher priority missions can establish claims first. */
   qsort( event_data, array_size(event_data), sizeof(EventData), event_cmp );

   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_("Loaded %d Event in %.3f s", "Loaded %d Events in %.3f s", array_size(event_data) ), array_size(event_data), time/1000. );
   }
   else
      DEBUG( n_("Loaded %d Event", "Loaded %d Events", array_size(event_data) ), array_size(event_data) );

   return 0;
}

/**
 * @brief Parses an event file.
 *
 *    @param file Source file path.
 *    @param temp Data to load into, or NULL for initial load.
 */
static int event_parseFile( const char* file, EventData *temp )
{
   size_t bufsize;
   xmlNodePtr node;
   xmlDocPtr doc;
   char *filebuf;
   const char *pos, *start_pos;

#ifdef DEBUGGING
   /* To check if event is valid. */
   int ret;
#endif /* DEBUGGING */

   /* Load string. */
   filebuf = ndata_read( file, &bufsize );
   if (filebuf == NULL) {
      WARN(_("Unable to read data from '%s'"), file);
      return -1;
   }
   if (bufsize == 0) {
      free( filebuf );
      return -1;
   }

   /* Skip if no XML. */
   pos = strnstr( filebuf, "</event>", bufsize );
   if (pos==NULL) {
      pos = strnstr( filebuf, "function create", bufsize );
      if ((pos != NULL) && !strncmp(pos,"--common",bufsize))
         WARN(_("Event '%s' has create function but no XML header!"), file);
      free(filebuf);
      return 0;
   }

   /* Separate XML header and Lua. */
   start_pos = strnstr( filebuf, "<?xml ", bufsize );
   pos = strnstr( filebuf, "--]]", bufsize );
   if (pos == NULL || start_pos == NULL) {
      WARN(_("Event file '%s' has missing XML header!"), file);
      return -1;
   }

   /* Parse the header. */
   doc = xmlParseMemory( start_pos, pos-start_pos );
   if (doc == NULL) {
      WARN(_("Unable to parse document XML header for Event '%s'"), file);
      return -1;
   }

   /* Get the root node. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_EVENT_TAG)) {
      WARN(_("Malformed '%s' file: missing root element '%s'"), file, XML_EVENT_TAG);
      return -1;
   }

   if (temp == NULL)
      temp = &array_grow(&event_data);
   event_parseXML( temp, node );
   temp->lua = strdup(filebuf);
   temp->sourcefile = strdup(file);

#ifdef DEBUGGING
   /* Check to see if syntax is valid. */
   ret = luaL_loadbuffer(naevL, temp->lua, strlen(temp->lua), temp->name );
   if (ret == LUA_ERRSYNTAX) {
      WARN(_("Event Lua '%s' syntax error: %s"),
            file, lua_tostring(naevL,-1) );
   } else {
      lua_pop(naevL, 1);
   }
#endif /* DEBUGGING */

   /* Clean up. */
   xmlFreeDoc(doc);
   free(filebuf);

   return 0;
}

/**
 * @brief Frees an EventData structure.
 *
 *    @param event Event Data to free.
 */
static void event_freeData( EventData *event )
{
   free( event->name );
   free( event->sourcefile );
   free( event->lua );

   free( event->spob );
   free( event->system );
   free( event->chapter );
   pcre2_code_free( event->chapter_re );

   free( event->cond );

   for (int i=0; i<array_size(event->tags); i++)
      free(event->tags[i]);
   array_free(event->tags);

   /* Clear the memory. */
#if DEBUGGING
   memset( event, 0, sizeof(EventData) );
#endif /* DEBUGGING */
}

/**
 * @brief Cleans up and removes active events.
 */
void events_cleanup (void)
{
   /* Free active events. */
   for (int i=0; i<array_size(event_active); i++)
      event_cleanup( &event_active[i] );
   array_free(event_active);
   event_active = NULL;
}

/**
 * @brief Exits the event subsystem.
 */
void events_exit (void)
{
   events_cleanup();

   /* Free data. */
   for (int i=0; i<array_size(event_data); i++)
      event_freeData( &event_data[i] );
   array_free(event_data);
   event_data  = NULL;
}

/**
 * @brief Gets the event data id from name.
 *
 *    @param evdata Name of the data.
 *    @return ID matching dataname.
 */
int event_dataID( const char *evdata )
{
   for (int i=0; i<array_size(event_data); i++)
      if (strcmp(event_data[i].name, evdata)==0)
         return i;
   WARN(_("No event data found matching name '%s'."), evdata);
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

/**
 * @brief Activates all the active event claims.
 */
void event_activateClaims (void)
{
   /* Free active events. */
   for (int i=0; i<array_size(event_active); i++)
      if (event_active[i].claims != NULL)
         claim_activate( event_active[i].claims );
}

/**
 * @brief Tests to see if an event has claimed a system.
 */
int event_testClaims( unsigned int eventid, int sys )
{
   Event_t *ev = event_get( eventid );
   if (ev==NULL) {
      WARN(_("Trying to test claims of unknown event with id '%d'!"), eventid);
      return 0;
   }
   return claim_testSys( ev->claims, sys );
}

/**
 * @brief Checks the event validity and cleans up after them.
 */
void event_checkValidity (void)
{
   /* Iterate. */
   for (int i=0; i<array_size(event_active); i++) {
      Event_t *ev = &event_active[i];

      /* Check if has children. */
      if (hook_hasEventParent( ev->id ) > 0)
         continue;

      /* Must delete. */
      WARN(_("Detected event '%s' without any hooks and is therefore invalid. Removing event."),
            event_dataName( ev->data ));
      event_remove( ev->id );
      i--; /* Keep iteration safe. */
   }
}

/**
 * @brief Saves the player's active events.
 *
 *    @param writer XML Write to use to save events.
 *    @return 0 on success.
 */
int events_saveActive( xmlTextWriterPtr writer )
{
   xmlw_startElem(writer,"events");

   for (int i=0; i<array_size(event_active); i++) {
      Event_t *ev = &event_active[i];
      if (!ev->save) /* Only save events that want to be saved. */
         continue;

      xmlw_startElem(writer,"event");

      xmlw_attr(writer,"name","%s",event_dataName(ev->data));
      xmlw_attr(writer,"id","%u",ev->id);

      /* Claims. */
      xmlw_startElem(writer,"claims");
      claim_xmlSave( writer, ev->claims );
      xmlw_endElem(writer); /* "claims" */

      /* Write Lua magic */
      xmlw_startElem(writer,"lua");
      nxml_persistLua( ev->env, writer );
      xmlw_endElem(writer); /* "lua" */

      xmlw_endElem(writer); /* "event" */
   }

   xmlw_endElem(writer); /* "events" */

   return 0;
}

/**
 * @brief Loads the player's active events from a save.
 *
 *    @param parent Node containing the player's active events.
 *    @return 0 on success.
 */
int events_loadActive( xmlNodePtr parent )
{
   xmlNodePtr node = parent->xmlChildrenNode;

   /* cleanup old events */
   events_cleanup();

   do {
      if (xml_isNode(node,"events"))
         if (events_parseActive( node ) < 0) return -1;
   } while (xml_nextNode(node));

   return 0;
}

/**
 * @brief Parses the actual individual event nodes.
 *
 *    @param parent Parent node to parse.
 *    @return 0 on success.
 */
static int events_parseActive( xmlNodePtr parent )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      char *buf;
      unsigned int id;
      int data;
      xmlNodePtr cur;
      Event_t *ev;

      if (!xml_isNode(node,"event"))
         continue;

      xmlr_attr_strd(node,"name",buf);
      if (buf==NULL) {
         WARN(_("Event has missing 'name' attribute, skipping."));
         continue;
      }
      data = event_dataID( buf );
      if (data < 0) {
         WARN(_("Event in save has name '%s' but event data not found matching name. Skipping."), buf);
         free(buf);
         continue;
      }
      free(buf);
      xmlr_attr_uint(node,"id",id);
      if (id==0) {
         WARN(_("Event with data '%s' has invalid 'id' attribute, skipping."), event_dataName(data));
         continue;
      }

      /* Create the event. */
      event_create( data, &id );
      ev = event_get( id );
      if (ev == NULL) {
         WARN(_("Event with data '%s' was not created, skipping."), event_dataName(data));
         continue;
      }
      ev->save = 1; /* Should save by default again. */

      /* Get the data. */
      cur = node->xmlChildrenNode;
      do {
         if (xml_isNode(cur,"lua"))
            nxml_unpersistLua( ev->env, cur );
      } while (xml_nextNode(cur));

      /* Claims. */
      if (xml_isNode(node,"claims"))
         ev->claims = claim_xmlLoad( node );
   } while (xml_nextNode(node));

   return 0;
}

int event_reload( const char *name )
{
   int res, edat = event_dataID( name );
   EventData save, *temp = edat<0 ? NULL : &event_data[edat];
   if (temp == NULL)
      return -1;
   save = *temp;
   res = event_parseFile( save.sourcefile, temp );
   if (res == 0)
      event_freeData( &save );
   else
      *temp = save;
   return res;
}

void event_toLuaTable( lua_State *L, int eventid )
{
   const EventData *data = &event_data[ eventid ];

   lua_newtable(L);

   lua_pushstring(L, data->name);
   lua_setfield(L,-2,"name");

   lua_pushboolean(L, (data->flags & EVENT_FLAG_UNIQUE));
   lua_setfield(L,-2,"unique");

   lua_newtable(L);
   for (int j=0; j<array_size(data->tags); j++) {
      lua_pushboolean(L,1);
      lua_setfield(L,-2,data->tags[j]);
   }
   lua_setfield(L,-2,"tags");
}
