/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DEV_MAPEDIT_H
#  define DEV_MAPEDIT_H

#include "mapData.h"
#include "outfit.h"

#define MAPEDIT_FILENAME_MAX     50    /**< Maximum mapedit map filename length. */
#define MAPEDIT_NAME_MAX         50    /**< Maximum mapedit map name length. */
#define MAPEDIT_DESCRIPTION_MAX 200    /**< Maximum mapedit map description length. */

void mapedit_open( unsigned int wid_unused, char *unused );
void mapedit_selectText (void);
char *mapedit_nameFilter( char *name );


#endif /* DEV_MAPEDIT_H */
