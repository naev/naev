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

#define EVENT_CHUNK           32


/**
 * @brief Event data structure.
 */
typedef struct EventData_s {
   char *name; /**< Name of the event. */
   char *lua; /**< Name of Lua file to use. */
   unsigned int flags; /**< Bit flags. */
   MissionAvail_t avail; /**< Availability. */
} EventData_t;


/**
 * @brief Activated event structure.
 */
typedef struct EventActive_s {
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


/**
 * @brief Loads all the events.
 *
 *    @return 0 on success.
 */
int events_load (void)
{
   return 0;
}


/**
 * @brief Exits the event subsystem.
 */
void events_exit (void)
{
   if (event_data != NULL) {
      free(event_data);
      event_data  = NULL;
      event_ndata = 0;
      event_mdata = 0;
   }
}


