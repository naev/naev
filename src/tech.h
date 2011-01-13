/*
 * See Licensing and Copyright notice in naev.h
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
tech_group_t *tech_groupCreate( xmlNodePtr node );
void tech_groupDestroy( tech_group_t *grp );
int tech_groupWrite( xmlTextWriterPtr writer, tech_group_t *grp );


/*
 * Group addition/removal.
 */
int tech_addItem( const char *name, const char *value );
int tech_rmItem( const char *name, const char *value );


/*
 * Get.
 */
Outfit** tech_getOutfit( tech_group_t *tech, int *n );
Outfit** tech_getOutfitArray( tech_group_t **tech, int num, int *n );
Ship** tech_getShip( tech_group_t *tech, int *n );
Ship** tech_getShipArray( tech_group_t **tech, int num, int *n );


#endif /* TECH_H */
