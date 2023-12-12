/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "claim.h"
#include "commodity.h"
#include "nlua.h"
#include "nxml.h"
#include "opengl.h"
#include "space.h"
#include "mission_markers.h"

/* availability by location */
typedef enum MissionAvailability_ {
   MIS_AVAIL_UNSET=-1,   /**< Mission isn't set. */
   MIS_AVAIL_NONE=0,     /**< Mission isn't available. */
   MIS_AVAIL_COMPUTER,   /**< Mission is available at mission computer. */
   MIS_AVAIL_BAR,        /**< Mission is available at bar. */
   MIS_AVAIL_LAND,       /**< Mission is available on landing. */
   MIS_AVAIL_ENTER       /**< Mission is available in space when player enters a system. */
} MissionAvailability;

/* flag functions */
#define mis_isFlag(m,f)    ((m)->flags & (f))
#define mis_setFlag(m,f)   ((m)->flags |= (f))
#define mis_rmFlag(m,f)    ((m)->flags &= ~(f))
/* actual flags */
#define MISSION_UNIQUE     (1<<0) /**< Unique missions can't be repeated */

/**
 * @brief Defines the availability of a mission.
 */
typedef struct MissionAvail_s {
   MissionAvailability loc; /**< Location of the mission. */
   int chance; /**< Chance of it appearing, last two digits represent %, first digit represents times it can appear (if 0 it behaves like once). */

   /* For specific cases */
   char *spob; /**< Spob name. */
   char *system; /**< System name. */
   char *chapter; /**< Chapter name. */
   pcre2_code *chapter_re; /**< Compiled regex chapter if applicable. */

   /* For generic cases */
   int *factions; /**< Array (array.h): To certain factions. */

   char *cond; /**< Condition that must be met (Lua). */
   int cond_chunk; /**< Chunk representing the condition. */
   char *done; /**< Previous mission that must have been done. */

   int priority; /**< Mission priority: 0 = main plot, 5 = default, 10 = insignificant. */
} MissionAvail_t;

/**
 * @struct MissionData
 *
 * @brief Static mission data.
 */
typedef struct MissionData_ {
   char *name; /**< The name of the mission. */

   MissionAvail_t avail; /**< Mission availability. */

   unsigned int flags; /**< Flags to store binary properties */
   char *lua; /**< Lua data to use. */
   char *sourcefile; /**< Source file name. */
   int chunk; /**< Lua mission data chunk. */

   /* Tags. */
   char **tags; /**< Mission tags with more information. */
} MissionData;

/**
 * @struct Mission
 *
 * @brief Represents an active mission.
 */
typedef struct Mission_ {
   const MissionData *data; /**< Data to use. */
   unsigned int id; /**< Unique mission identifier, used for keeping track of hooks. */
   int accepted; /**< Mission is a player mission. */

   char *title; /**< Not to be confused with name */
   char *desc; /**< Description of the mission */
   char *reward; /**< Rewards in text */
   credits_t reward_value; /**< Value of the reward (for monetary cases). */
   glTexture *portrait; /**< Portrait of the mission giver if applicable. */
   char *npc; /**< Name of the NPC giving the mission. */
   char *npc_desc; /**< Description of the giver NPC. */

   /* mission cargo given to the player - need to cleanup */
   unsigned int *cargo; /**< Array (array.h): Cargos given to player. */

   /* Markers. */
   MissionMarker *markers; /**< Markers array. */

   /* OSD. */
   unsigned int osd; /**< On-Screen Display ID. */
   int osd_set; /**< OSD was set explicitly. */

   /* Claims. */
   Claim_t *claims; /**< System claims. */

   nlua_env env; /**< The environment of the running Lua code. */
} Mission;

/*
 * current player missions
 */
extern Mission **player_missions; /**< Player's active missions. */

/*
 * creates missions for a spob and such
 */
Mission* missions_genList( int *n, int faction,
      const Spob *pnt, const StarSystem *sys, MissionAvailability loc );
int mission_accept( Mission* mission ); /* player accepted mission for computer/bar */
void missions_run( MissionAvailability loc, int faction, const Spob *pnt, const StarSystem *sys );
int mission_start( const char *name, unsigned int *id );
int mission_test( const char *name );
const char *mission_availabilityStr( MissionAvailability loc );

/*
 * misc
 */
const MissionData *mission_list (void);
int mission_alreadyRunning( const MissionData* misn );
int mission_getID( const char* name );
const MissionData* mission_get( int id );
const MissionData* mission_getFromName( const char* name );
int mission_addMarker( Mission *misn, int id, int sys, MissionMarkerType type );
void mission_sysMark (void);
const StarSystem* mission_sysComputerMark( const Mission* misn );
const StarSystem* mission_getSystemMarker( const Mission* misn );
MissionMarkerType mission_markerTypeSpobToSystem( MissionMarkerType t );
MissionMarkerType mission_markerTypeSystemToSpob( MissionMarkerType t );
void mission_toLuaTable( lua_State *L, const MissionData *m );
const char **mission_loadFailed (void);

/*
 * cargo stuff
 */
int mission_linkCargo( Mission* misn, unsigned int cargo_id );
int mission_unlinkCargo( Mission* misn, unsigned int cargo_id );

/*
 * load/quit
 */
int missions_load (void);
int missions_loadActive( xmlNodePtr parent );
int missions_loadCommodity( xmlNodePtr parent );
Commodity* missions_loadTempCommodity( xmlNodePtr parent );
int missions_saveActive( xmlTextWriterPtr writer );
int missions_saveTempCommodity( xmlTextWriterPtr writer, const Commodity* c );
void mission_cleanup( Mission* misn );
void mission_shift( int pos );
void missions_free (void);
void missions_cleanup (void);
int mission_reload( const char *name );

/*
 * Actually in nlua_misn.h
 */
int misn_tryRun( Mission *misn, const char *func );
void misn_runStart( Mission *misn, const char *func );
int misn_runFunc( const Mission *misn, const char *func, int nargs );
int misn_run( Mission *misn, const char *func );

/*
 * Claims.
 */
void missions_activateClaims (void);
