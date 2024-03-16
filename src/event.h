/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "claim.h"
#include "nlua.h"

/**
 * @brief Activated event structure.
 */
typedef struct Event_s {
   unsigned int id;     /**< Event ID. */
   int          data;   /**< EventData parent. */
   nlua_env     env;    /**< The environment of the running Lua code. */
   int          save;   /**< Whether or not it should be saved. */
   Claim_t     *claims; /**< Event claims. */
} Event_t;

/**
 * @brief Possibly event triggers.
 */
typedef enum EventTrigger_s {
   EVENT_TRIGGER_NULL = -1, /**< Invalid trigger. */
   EVENT_TRIGGER_NONE = 0,  /**< No enter trigger. */
   EVENT_TRIGGER_ENTER,     /**< Entering a system (jump/takeoff). */
   EVENT_TRIGGER_LAND,      /**< Landing on a system. */
   EVENT_TRIGGER_LOAD       /**< Loading or starting a new save game. */
} EventTrigger_t;

/*
 * Loading/exiting.
 */
int  events_load( void );
void events_exit( void );
void events_cleanup( void );
void event_checkValidity( void );
int  event_reload( const char *name );

/*
 * Triggering.
 */
int  event_start( const char *name, unsigned int *id );
void events_trigger( EventTrigger_t trigger );

/*
 * Handling.
 */
Event_t    *event_get( unsigned int eventid );
void        event_remove( unsigned int eventid );
int         event_save( unsigned int eventid );
const char *event_getData( unsigned int eventid );
int         event_isUnique( unsigned int eventid );

/*
 * Data.
 */
int         event_dataID( const char *evdata );
const char *event_dataName( int dataid );

/*
 * Claims.
 */
void event_activateClaims( void );
int  event_testClaims( unsigned int eventid, int sys );

/*
 * Misc.
 */
int  event_alreadyRunning( int data );
void event_toLuaTable( lua_State *L, int eventid );
