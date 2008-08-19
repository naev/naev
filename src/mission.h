/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef MISSION_H
#  define MISSION_H


#include "misn_lua.h"


/* availability by location */
#define  MIS_AVAIL_NONE       0 /**< Mission isn't available. */
#define  MIS_AVAIL_COMPUTER   1 /**< Mission is available at mission computer. */
#define  MIS_AVAIL_BAR        2 /**< Mission is available at bar. */
#define  MIS_AVAIL_OUTFIT     3 /**< Mission is available at outfitter. */
#define  MIS_AVAIL_SHIPYARD   4 /**< Mission is available at shipyard. */
#define  MIS_AVAIL_LAND       5 /**< Mission is available on landing. */


/* flag functions */
#define mis_isFlag(m,f)    ((m)->flags & (f))
#define mis_setFlag(m,f)   ((m)->flags |= (f))
#define mis_rmFlag(m,f)    ((m)->flags ^= (f))
/* actual flags */
#define  MISSION_UNIQUE       1 /**< Unique missions can't be repeated */


/**
 * @struct MissionData
 *
 * @brief Static mission data.
 */
typedef struct MissionData_ {
   char *name; /**< The name of the mission. */

   /* availability */
   struct {
      int loc; /* location */
      int chance; /* chance of it appearing */

      /* for specific cases */
      char *planet; /**< Planet name. */
      char *system; /**< System name. */

      /* for generic cases */
      int* factions; /**< To certain factions. */
      int nfactions; /**< Number of factions in factions. */

      char* cond; /**< Condition that must be met (Lua). */
      char* done; /**< Previous mission that must have been done. */
   } avail; /**< Availability. */

   unsigned int flags; /**< Flags to store binary properties */

   char* lua; /**< Lua file to use. */
} MissionData;


/**
 * @struct Mission
 *
 * @brief Represents an active mission.
 */
typedef struct Mission_ {
   MissionData *data; /**< Data to use. */
   unsigned int id; /**< Unique mission identifier, used for keeping track of hooks. */

   char *title; /**< Not to be confused with name */
   char *desc; /**< Description of the mission */
   char *reward; /**< Rewards in text */

   /* mission cargo given to the player - need to cleanup */
   unsigned int *cargo; /**< Cargos given to player. */
   int ncargo; /**< Number of cargos given to player. */

   char *sys_marker; /**< System to mark. */

   lua_State *L; /**< The state of the running lua code. */
} Mission;


/*
 * current player missions
 */
#define MISSION_MAX  12 /**< No sense in allowing the player have infinite missions. */
extern Mission player_missions[MISSION_MAX]; /**< Player's active missions. */


/*
 * creates missions for a planet and such
 */
Mission* missions_computer( int *n, int faction,
      char* planet, char* sysname ); /* for mission computer */
int mission_accept( Mission* mission ); /* player accepted mission - mission computer */
void missions_bar( int faction, char* planet, char* sysname );

/*
 * misc
 */
int mission_getID( char* name );
MissionData* mission_get( int id );
void mission_sysMark (void);


/*
 * cargo stuff
 */
void mission_linkCargo( Mission* misn, unsigned int cargo_id );
void mission_unlinkCargo( Mission* misn, unsigned int cargo_id );


/*
 * load/quit
 */
int missions_load (void);
void mission_cleanup( Mission* misn );
void missions_free (void);
void missions_cleanup (void);


#endif /* MISSION_H */


