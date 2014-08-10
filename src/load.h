/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef LOAD_H
#  define LOAD_H


#include <stdint.h>

#include "ntime.h"


/**
 * @brief A naev save.
 */
typedef struct nsave_s {
   char *name; /**< Player name. */
   char *path; /**< File path. */

   /* Naev info. */
   char *version; /**< Naev version. */
   char *data; /**< Data name. */

   /* Player info. */
   char *planet; /**< Planet player is at. */
   ntime_t date; /**< Date. */
   uint64_t credits; /**< Credits player has. */

   /* Ship info. */
   char *shipname; /**< Name of the ship. */
   char *shipmodel; /**< Model of the ship. */
} nsave_t;


void load_loadGameMenu (void);
int load_game( const char* file, int version_diff );

int load_refresh (void);
void load_free (void);
nsave_t *load_getList( int *n );


#endif /* LOAD_H */
