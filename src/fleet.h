/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FLEET_H
#  define FLEET_H


#include "pilot.h"


/**
 * @struct FleetPilot
 *
 * @brief Represents a pilot in a fleet.
 *
 * @sa Fleet
 * @sa Pilot
 */   
typedef struct FleetPilot_ {
   Ship* ship; /**< ship the pilot is flying */
   char* name; /**< used if they have a special name like uniques */
   int chance; /**< chance of this pilot appearing in the leet */
   char *ai; /**< ai different of fleet's global ai */
} FleetPilot;


/**
 * @struct Fleet
 *
 * @brief Represents a fleet.
 *
 * Fleets are used to create pilots, both from being in a system and from
 *  mission triggers.
 *
 * @sa FleetPilot
 */
typedef struct Fleet_ {
   char* name; /**< fleet name, used as the identifier */
   int faction; /**< faction of the fleet */

   char *ai; /**< AI profile to use */

   FleetPilot* pilots; /**< the pilots in the fleet */
   int npilots; /**< total number of pilots */
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


#endif /* FLEET_H */

