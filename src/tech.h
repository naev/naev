/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "nxml.h"
#include "outfit.h"
#include "ship.h"

/*
 * Forward declaration of tech group struct.
 */
struct tech_group_s;
typedef struct tech_group_s tech_group_t;

/*
 * Load/free.
 */
int  tech_load( void );
void tech_free( void );

/*
 * Group creation/destruction.
 */
tech_group_t *tech_groupCreate( void );
tech_group_t *tech_groupCreateXML( xmlNodePtr node );
void          tech_groupDestroy( tech_group_t *grp );
int           tech_groupWrite( xmlTextWriterPtr writer, tech_group_t *grp );

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
int         tech_hasItem( const tech_group_t *tech, const char *item );
int         tech_getItemCount( const tech_group_t *tech );
char      **tech_getItemNames( const tech_group_t *tech, int *n );
char      **tech_getAllItemNames( int *n );
Outfit    **tech_getOutfit( const tech_group_t *tech );
Outfit    **tech_getOutfitArray( tech_group_t **tech, int num );
Ship      **tech_getShip( const tech_group_t *tech );
Ship      **tech_getShipArray( tech_group_t **tech, int num );
Commodity **tech_getCommodity( const tech_group_t *tech );
Commodity **tech_getCommodityArray( tech_group_t **tech, int num );

/*
 * Check.
 */
int tech_checkOutfit( const tech_group_t *tech, const Outfit *o );
