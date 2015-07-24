/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAPDATA_H
#  define MAPDATA_H


#include "space.h"

/**
 * @brief Represents a map, is not actually stored on a ship but put into the nav system.
 *
 * Basically just marks an amount of systems when the player buys it as known.
 */
struct OutfitMapData_s {
   StarSystem **systems; /**< systems to mark as known. */
   JumpPoint **jumps; /**< jump points to mark as known. */
   Planet **assets; /**< assets to mark as known. */
};

#endif /* MAPDATA_H */
