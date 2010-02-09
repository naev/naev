/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FLEET_H
#  define FLEET_H


#include "pilot.h"


/*
 * Flags.
 */
#define FLEET_FLAG_GUARD         (1<<0) /**< Fleet is considered to be guarding. */
#define fleet_setFlag(x,f)       ((x)->flags |= (f)) /**< Sets a fleet flag. */
#define fleet_isFlag(x,f)        ((x)->flags & (f)) /**< Checks to see if fleet has flag. */


/**
 * @brief Represents a pilot in a fleet.
 *
 * @sa Fleet
 * @sa Pilot
 */   
typedef struct FleetPilot_ {
   Ship *ship; /**< Ship the pilot is flying. */
   char *name; /**< Used if they have a special name like uniques. */
   int chance; /**< Chance of this pilot appearing in the fleet. */
   char *ai; /**< AI different of fleet's global AI. */
} FleetPilot;


/**
 * @brief Represents a fleet.
 *
 * Fleets are used to create pilots, both from being in a system and from
 *  mission triggers.
 *
 * @sa FleetPilot
 * @sa FleetGroup
 */
typedef struct Fleet_ {
   char* name; /**< Fleet name, used as the identifier. */
   int faction; /**< Faction of the fleet. */
   char *ai; /**< AI profile to use. */
   FleetPilot* pilots; /**< The pilots in the fleet. */
   int npilots; /**< Total number of pilots. */
   unsigned int flags; /**< Fleet flags. */
   double pilot_avg; /**< Average amount of pilots. */
   double mass_avg; /**< Average mass for security concerns. */
} Fleet;


/**
 * @brief Represents a group of fleets.
 *
 * Used to simplify creation of star systems and easily synchronize all systems
 *  with new ship additions.
 *
 * @sa Fleet
 */
typedef struct FleetGroup_ {
   char *name; /**< Name of the fleetgroup, used as the identifier. */
   Fleet **fleets; /**< List of fleets in the group. */
   int *chance; /**< Chance of each fleet in the group. */
   int nfleets; /**< Number of fleets in the group. */
} FleetGroup;


/*
 * getting fleet stuff
 */
Fleet* fleet_get( const char* name );
FleetGroup* fleet_getGroup( const char* name );


/*
 * load / clean up
 */
int fleet_load (void);
void fleet_free (void);


/*
 * creation
 */
unsigned int fleet_createPilot( Fleet *flt, FleetPilot *plt, double dir,
      Vector2d *pos, Vector2d *vel, const char* ai, unsigned int flags );


#endif /* FLEET_H */

