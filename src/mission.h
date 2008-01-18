/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef MISSION_H
#  define MISSION_H


#include "misn_lua.h"

#include "faction.h"


/* availability by location */
#define	MIS_AVAIL_COMPUTER	1
#define	MIS_AVAIL_BAR			2
#define	MIS_AVAIL_OUTFIT		3
#define	MIS_AVAIL_SHIPYARD	4
#define	MIS_AVAIL_LAND			5


/* flag functions */
#define mis_isFlag(m,f)		((m)->flags & (f))
#define mis_setFlag(m,f)	((m)->flags |= (f))
#define mis_rmFlag(m,f)		((m)->flags ^= (f))
/* actual flags */
#define	MISSION_UNIQUE			1 /* unique missions can't be repeated */


typedef struct MissionData_ {
	char *name; /* the name of the mission */

	/* availability */
	struct {
		int loc; /* location */
		double chance; /* chance of it appearing */

		/* for specific cases */
		char *planet;
		char *system;

		/* for generic cases */
		Faction *factions;
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

	char *title; /* not to be confused with name */
	char *desc; /* description of the mission */
	char *reward; /* rewards in text */

	lua_State *L;
} Mission;


/*
 * current player missions
 */
#define MISSION_MAX  6  /* no sense in allowing the player have infinite missions */
extern Mission player_missions[MISSION_MAX];


/*
 * load/quit
 */
int missions_load (void);
void missions_free (void);
void missions_cleanup (void);


#endif /* MISSION_H */


