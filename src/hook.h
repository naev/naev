/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef HOOK_H
#  define HOOK_H


#include "mission.h"

#include "nlua_pilot.h"
#include "nlua_faction.h"
#include "nlua_planet.h"
#include "nlua_jump.h"


#define HOOK_MAX_PARAM  3 /**< Maximum hook params, to avoid dynamic allocation. */


/**
 * @Brief The hook parameter types.
 */
typedef enum HookParamType_e {
   HOOK_PARAM_NIL, /**< No hook parameter. */
   HOOK_PARAM_NUMBER, /**< Number parameter. */
   HOOK_PARAM_STRING, /**< String parameter. */
   HOOK_PARAM_BOOL, /**< Boolean parameter. */
   HOOK_PARAM_PILOT, /**< Pilot hook parameter. */
   HOOK_PARAM_FACTION, /**< Faction hook parameter. */
   HOOK_PARAM_ASSET, /**< Asset hook parameter. */
   HOOK_PARAM_JUMP, /**< Jump point hook parameter. */
   HOOK_PARAM_SENTINEL /**< Enum sentinel. */
} HookParamType;

/**
 * @brief The actual hook parameter.
 */
typedef struct HookParam_s {
   HookParamType type; /**< Type of parameter. */
   union {
      double num; /**< Number parameter. */
      const char *str; /**< String parameter. */
      int b; /**< Boolean parameter. */
      LuaPilot lp; /**< Hook parameter pilot data. */
      LuaFaction lf; /**< Hook parameter faction data. */
      LuaPlanet la; /**< Hook parameter planet data. */
      LuaJump lj; /**< Hook parameter jump data. */
   } u; /**< Hook parameter data. */
} HookParam;

/*
 * Exclusion.
 */
void hook_exclusionStart (void);
void hook_exclusionEnd( double dt );

/* add/run hooks */
unsigned int hook_addMisn( unsigned int parent, const char *func, const char *stack );
unsigned int hook_addEvent( unsigned int parent, const char *func, const char *stack );
unsigned int hook_addFunc( int (*func)(void*), void* data, const char *stack );
void hook_rm( unsigned int id );
void hook_rmMisnParent( unsigned int parent );
void hook_rmEventParent( unsigned int parent );
int hook_hasMisnParent( unsigned int parent );
int hook_hasEventParent( unsigned int parent );

/* pilot hook. */
int pilot_runHookParam( Pilot* p, int hook_type, HookParam *param, int nparam );

nlua_env hook_env( unsigned int hook );

/*
 * run hooks
 *
 * Currently used:
 *  - General
 *    - "safe" - Runs once each frame at a same time (last in the frame), good place to do breaking stuff.
 *    - "takeoff" - When taking off
 *    - "jumpin" - When player jumps (after changing system)
 *    - "jumpout" - When player jumps (before changing system)
 *    - "time" - When time is increment drastically (hyperspace and taking off)
 *    - "hail" - When any pilot is hailed
 *    - "board" - When any pilot is boarded
 *    - "input" - When an input command is pressed
 *    - "standing" - Whenever faction changes.
 *    - "load" - Run on load.
 *    - "discover" - When something is discovered.
 *  - Landing
 *    - "land" - When landed
 *    - "outfits" - When visited outfitter
 *    - "shipyard" - When visited shipyard
 *    - "bar" - When visited bar
 *    - "mission" - When visited mission computer
 *    - "commodity" - When visited commodity exchange
 *    - "equipment" - When visiting equipment place < br/>
 */
int hooks_runParam( const char* stack, HookParam *param );
int hooks_run( const char* stack );
int hook_runIDparam( unsigned int id, HookParam *param );
int hook_runID( unsigned int id ); /* runs hook of specific id */

/* destroys hooks */
void hook_cleanup (void);

/* Timer hooks. */
void hooks_update( double dt );
unsigned int hook_addTimerMisn( unsigned int parent, const char *func, double ms );
unsigned int hook_addTimerEvt( unsigned int parent, const char *func, double ms );

/* Date hooks. */
void hooks_updateDate( ntime_t change );
unsigned int hook_addDateMisn( unsigned int parent, const char *func, ntime_t resolution );
unsigned int hook_addDateEvt( unsigned int parent, const char *func, ntime_t resolution );


#endif /* HOOK_H */

