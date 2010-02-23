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
 * Get.
 */
int tech_get( const char *name );
Outfit** tech_getOutfit( tech_group_t *tech, int *n );


#endif /* TECH_H */
