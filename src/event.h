/*
 * See Licensing and Copyright notice in naev.h
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
   EVENT_TRIGGER_ENTER, /**< Entering a system (jump/takeoff). */
   EVENT_TRIGGER_LAND,  /**< Landing on a system. */
   EVENT_TRIGGER_LOAD,  /**< Loading or starting a new save game. */
} EventTrigger_t;


/*
 * Loading/exitting.
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


