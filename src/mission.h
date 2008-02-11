/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef MISSION_H
#  define MISSION_H


#include "misn_lua.h"


/* availability by location */
#define  MIS_AVAIL_NONE       0
#define  MIS_AVAIL_COMPUTER   1
#define  MIS_AVAIL_BAR        2
#define  MIS_AVAIL_OUTFIT     3
#define  MIS_AVAIL_SHIPYARD   4
#define  MIS_AVAIL_LAND       5


/* flag functions */
#define mis_isFlag(m,f)    ((m)->flags & (f))
#define mis_setFlag(m,f)   ((m)->flags |= (f))
#define mis_rmFlag(m,f)    ((m)->flags ^= (f))
/* actual flags */
#define  MISSION_UNIQUE       1 /* unique missions can't be repeated */


/*
 * static mission data
 */
typedef struct MissionData_ {
   char *name; /* the name of the mission */

   /* availability */
   struct {
      int loc; /* location */
      int chance; /* chance of it appearing */

      /* for specific cases */
      char *planet;
      char *system;

      /* for generic cases */
      int* factions;
      int nfactions;
   } avail;

   unsigned int flags; /* flags to store binary properties */

   char* lua;
} MissionData;


/*
 * active mission
 */
typedef struct Mission_ {
   MissionData *data;
   unsigned int id; /* unique mission identifier, used for keeping track of hooks */

   char *title; /* not to be confused with name */
   char *desc; /* description of the mission */
   char *reward; /* rewards in text */

   /* mission cargo given to the player - need to cleanup */
   unsigned int *cargo;
   int ncargo;

   lua_State *L; /* the state of the running lua code */
} Mission;


/*
 * current player missions
 */
#define MISSION_MAX  6  /* no sense in allowing the player have infinite missions */
extern Mission player_missions[MISSION_MAX];


/*
 * creates missions for a planet and such
 */
Mission* missions_computer( int *n, int faction,
      char* planet, char* system ); /* for mission computer */
void mission_accept( Mission* mission ); /* player accepted mission - mission computer */
void missions_bar( int faction, char* planet, char* system );

/*
 * misc
 */
int mission_getID( MissionData* misn );


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


