/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"

/* Bit flags to specify what standing to get. */
#define SAFELANES_FRIENDLY                                                     \
   ( 1 << 0 ) /**< Get safelanes friendly to faction.                          \
               */
#define SAFELANES_NEUTRAL                                                      \
   ( 1 << 1 ) /**< Get safelanes neutral (neither friendly nor hostile) to     \
                 faction. */
#define SAFELANES_HOSTILE ( 1 << 3 ) /**< Get safelanes hostile to faction. */

/**
 * @brief Defines the type of object at a safe lane's end.
 */
typedef enum SafeLaneLocType_ {
   SAFELANE_LOC_SPOB,     /**< ID refers to a spob. */
   SAFELANE_LOC_DEST_SYS, /**< ID refers to a jump point in the source system,
                             and is of the target system. */
} SafeLaneLocType;

/**
 * @brief Describes a safe lane, patrolled by a faction, within a system.
 */
typedef struct SafeLane_ {
   int             faction;       /**< ID of the faction which owns the lane. */
   SafeLaneLocType point_type[2]; /**< Type of the patrol's endpoints. */
   int             point_id[2];   /**< ID of the patrol's endpoints. */
   double          map_alpha;     /**< Alpha to display on the map. */
} SafeLane;

void      safelanes_init( void );
void      safelanes_destroy( void );
SafeLane *safelanes_get( int faction, int standing, const StarSystem *system );
void      safelanes_recalculate( void );
int       safelanes_calculated( void );
