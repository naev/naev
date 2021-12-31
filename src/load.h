/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>
#include <time.h>
/** @endcond */

#include "ntime.h"

/**
 * @brief A naev save.
 */
typedef struct nsave_s {
   char *save_name; /** Snapshot name. */
   char *name; /**< Player name. */
   char *path; /**< File path relative to PhysicsFS write directory. */

   /* Naev info. */
   char *version; /**< Naev version. */
   char *data; /**< Data name. */

   /* Player info. */
   char *spob; /**< Spob player is at. */
   ntime_t date; /**< Date. */
   uint64_t credits; /**< Credits player has. */
   char *chapter; /**< Player's current chapter. */

   /* Ship info. */
   char *shipname; /**< Name of the ship. */
   char *shipmodel; /**< Model of the ship. */
} nsave_t;

void load_loadGameMenu (void);
int load_gameDiff( const char* file );
int load_gameFile( const char* file );
int load_game( nsave_t *ns );

int load_refresh (void);
void load_free (void);
const nsave_t *load_getList (void);
