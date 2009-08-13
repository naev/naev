/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef EQUIPMENT_H
#  define EQUIPMENT_H


/*
 * Image array names.
 */
#define  EQUIPMENT_SHIPS      "iarAvailShips"
#define  EQUIPMENT_OUTFITS    "iarAvailOutfits"


/*
 * Main.
 */
void equipment_open( unsigned int wid );
void equipment_cleanup (void);


/*
 * Misc.
 */
void equipment_addAmmo (void);
void equipment_genLists( unsigned int wid );
void equipment_updateShips( unsigned int wid, char* str );
void equipment_updateOutfits( unsigned int wid, char* str );


#endif /* EQUIPMENT_H */
