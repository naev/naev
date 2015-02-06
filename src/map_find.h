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


#ifndef MAP_FIND_H
#  define MAP_FIND_H


#include "space.h"


/**
 * @brief Represents a found target.
 */
typedef struct map_find_s {
   Planet *pnt;         /**< Planet available at. */
   StarSystem *sys;     /**< System available at. */
   char display[128];   /**< Name to display. */
   int jumps;           /**< Jumps to system. */
   double distance;     /**< Distance to system. */
} map_find_t;


void map_inputFind( unsigned int parent, char* str );
void map_inputFindType( unsigned int parent, char *type );


#endif /* MAP_FIND_H */

