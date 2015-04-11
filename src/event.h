/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef EVENT_H
#  define EVENT_H


#include "nlua.h"
#include "claim.h"


/**
 * @brief Activated event structure.
 */
typedef struct Event_s {
   unsigned int id; /**< Event ID. */
   int data; /**< EventData parent. */
   lua_State *L; /**< Event Lua State. */
   int save; /**< Whether or not it should be saved. */
   SysClaim_t *claims; /**< Event claims. */
} Event_t;


/**
 * @brief Possibly event triggers.
 */
typedef enum EventTrigger_s {
   EVENT_TRIGGER_NULL,  /**< Invalid trigger. */
   EVENT_TRIGGER_NONE,  /**< No enter trigger. */
   EVENT_TRIGGER_ENTER, /**< Entering a system (jump/takeoff). */
   EVENT_TRIGGER_LAND,  /**< Landing on a system. */
   EVENT_TRIGGER_LOAD   /**< Loading or starting a new save game. */
} EventTrigger_t;


/*
 * Loading/exiting.
 */
int events_load (void);
void events_exit (void);
void events_cleanup (void);
void event_checkSanity (void);


/*
 * Triggering.
 */
int event_start( const char *name, unsigned int *id );
lua_State *event_runStart( unsigned int eventid, const char *func );
int event_runFunc( unsigned int eventid, const char *func, int nargs );
int event_run( unsigned int eventid, const char *func );
void events_trigger( EventTrigger_t trigger );


/*
 * Handling.
 */
Event_t *event_get( unsigned int eventid );
void event_remove( unsigned int eventid );
int event_save( unsigned int eventid );
const char *event_getData( unsigned int eventid );
int event_isUnique( unsigned int eventid );


/*
 * Data.
 */
int event_dataID( const char *evdata );
const char *event_dataName( int dataid );


/*
 * Claims.
 */
void event_activateClaims (void);
int event_testClaims( unsigned int eventid, int sys );


/*
 * Misc.
 */
int event_alreadyRunning( int data );


#endif /* EVENT_H */


