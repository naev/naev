/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"

/**
 * @brief Represents a map, is not actually stored on a ship but put into the
 * nav system.
 *
 * Basically just marks an amount of systems when the player buys it as known.
 */
struct OutfitMapData_s {
   StarSystem **systems; /**< systems to mark as known. */
   JumpPoint  **jumps;   /**< jump points to mark as known. */
   Spob       **spobs;   /**< spobs to mark as known. */
};
