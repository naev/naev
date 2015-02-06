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


#ifndef TECH_H
#  define TECH_H


#include "outfit.h"
#include "ship.h"
#include "economy.h"
#include "nxml.h"


/*
 * Forward declaration of tech group struct.
 */
struct tech_group_s;
typedef struct tech_group_s tech_group_t;


/*
 * Load/free.
 */
int tech_load (void);
void tech_free (void);


/*
 * Group creation/destruction.
 */
tech_group_t *tech_groupCreate( void );
tech_group_t *tech_groupCreateXML( xmlNodePtr node );
void tech_groupDestroy( tech_group_t *grp );
int tech_groupWrite( xmlTextWriterPtr writer, tech_group_t *grp );


/*
 * Group addition/removal.
 */
int tech_addItemTech( tech_group_t *tech, const char *value );
int tech_rmItemTech( tech_group_t *tech, const char *value );
int tech_addItem( const char *name, const char *value );
int tech_rmItem( const char *name, const char *value );


/*
 * Get.
 */
int tech_hasItem( tech_group_t *tech, char *item );
int tech_getItemCount( tech_group_t *tech );
char** tech_getItemNames( tech_group_t *tech, int *n );
char** tech_getAllItemNames( int *n );
Outfit** tech_getOutfit( tech_group_t *tech, int *n );
Outfit** tech_getOutfitArray( tech_group_t **tech, int num, int *n );
Ship** tech_getShip( tech_group_t *tech, int *n );
Ship** tech_getShipArray( tech_group_t **tech, int num, int *n );
Commodity** tech_getCommodity( tech_group_t *tech, int *n );
Commodity** tech_getCommodityArray( tech_group_t **tech, int num, int *n );


#endif /* TECH_H */
