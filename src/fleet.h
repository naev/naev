/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FLEET_H
#  define FLEET_H


#include "pilot.h"


/*
 * Flags.
 */
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
   char *ai; /**< AI different of fleet's global AI. */
} FleetPilot;


/**
 * @brief Represents a fleet.
 *
 * Fleets are used to create pilots, both from being in a system and from
 *  mission triggers.
 *
 * @sa FleetPilot
 */
typedef struct Fleet_ {
   char* name; /**< Fleet name, used as the identifier. */
   int faction; /**< Faction of the fleet. */
   char *ai; /**< AI profile to use. */
   FleetPilot* pilots; /**< The pilots in the fleet. */
   int npilots; /**< Total number of pilots. */
   unsigned int flags; /**< Fleet flags. */
} Fleet;


/*
 * getting fleet stuff
 */
Fleet* fleet_get( const char* name );


/*
 * load / clean up
 */
int fleet_load (void);
void fleet_free (void);


/*
 * creation
 */
unsigned int fleet_createPilot( Fleet *flt, FleetPilot *plt, double dir,
      Vector2d *pos, Vector2d *vel, const char* ai, PilotFlags flags );


#endif /* FLEET_H */

