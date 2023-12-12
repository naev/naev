/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>
#include <time.h>
#include "physfs.h"
/** @endcond */

#include "ntime.h"

typedef enum SaveCompatibility_ {
   SAVE_COMPATIBILITY_OK=0,
   SAVE_COMPATIBILITY_NAEV_VERSION,
   SAVE_COMPATIBILITY_PLUGINS,
} SaveCompatibility;

/**
 * @brief A naev save.
 */
typedef struct nsave_s {
   char *save_name; /** Snapshot name. */
   char *player_name; /**< Player name. */
   char *path; /**< File path relative to PhysicsFS write directory. */
   PHYSFS_sint64 modtime; /**< Last modified time. */

   /* Naev info. */
   char *version; /**< Naev version. */
   char *data; /**< Data name. */

   /* Plugins. */
   char **plugins; /**< Plugins used in the game. */
   SaveCompatibility compatible; /**< Compatibility status. */

   /* Player info. */
   char *spob; /**< Spob player is at. */
   ntime_t date; /**< Date. */
   uint64_t credits; /**< Credits player has. */
   char *chapter; /**< Player's current chapter. */
   char *difficulty; /**< Difficulty setting of the player. */

   /* Ship info. */
   char *shipname; /**< Name of the ship. */
   char *shipmodel; /**< Model of the ship. */
} nsave_t;

void load_loadGameMenu (void);
void load_loadSnapshotMenu( const char *name, int disablesave );

int load_gameDiff( const char* file );
int load_gameFile( const char* file );

int load_refresh (void);
void load_free (void);
const nsave_t *load_getList( const char *name );
