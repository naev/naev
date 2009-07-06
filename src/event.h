/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef EVENT_H
#  define EVENT_H


#include "nlua.h"


#define EVENT_TIMER_MAX       3 /**< Maximum amount of event timers. */


/**
 * @brief Activated event structure.
 */
typedef struct Event_s {
   unsigned int id; /**< Event ID. */
   int data; /**< EventData parent. */
   lua_State *L; /**< Event Lua State. */

   /* Timers. */
   double timer[EVENT_TIMER_MAX]; /**< Event timers. */
   char *tfunc[EVENT_TIMER_MAX]; /**< Functions assosciated to the timers. */
} Event_t;


/**
 * @brief Possibly event triggers.
 */
typedef enum EventTrigger_s {
   EVENT_TRIGGER_NULL, /**< Invalid trigger. */
   EVENT_TRIGGER_ENTER /**< Entering a system (jump/takeoff). */
} EventTrigger_t;


/*
 * Loading/exitting.
 */
int events_load (void);
void events_exit (void);
void events_cleanup (void);


/*
 * Updating.
 */
void events_update( double dt );


/*
 * Triggering.
 */
int event_run( unsigned int eventid, const char *func );
void events_trigger( EventTrigger_t trigger );


/*
 * Handling.
 */
void event_remove( unsigned int eventid );
const char *event_getData( unsigned int eventid );


#endif /* EVENT_H */


