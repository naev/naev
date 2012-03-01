/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_WEAPON_H
#  define PILOT_WEAPON_H


#include "pilot.h"


#define WEAPSET_INRANGE_PLAYER_DEF  0 /**< Default weaponset inrange parameter for the player. */

#define WEAPSET_TYPE_CHANGE   0  /**< Changes weaponsets. */
#define WEAPSET_TYPE_WEAPON   1  /**< Activates weapons (while held down). */
#define WEAPSET_TYPE_ACTIVE   2  /**< Toggles outfits (if on it deactivates). */


/* Freedom. */
void pilot_weapSetFree( Pilot* p );

/* Shooting. */
int pilot_shoot( Pilot* p, int level );
void pilot_shootStop( Pilot* p, int level );
void pilot_getRateMod( double *rate_mod, double* energy_mod,
      Pilot* p, Outfit* o );


/* Updating. */
void pilot_weapSetAIClear( Pilot* p );
void pilot_weapSetPress( Pilot* p, int id, int type );
void pilot_weapSetUpdate( Pilot* p );


/* Weapon Set. */
const char *pilot_weapSetName( Pilot* p, int id );
void pilot_weapSetRmSlot( Pilot *p, int id, OutfitSlotType type );
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o, int level );
void pilot_weapSetRm( Pilot* p, int id, PilotOutfitSlot *o );
int pilot_weapSetCheck( Pilot* p, int id, PilotOutfitSlot *o );
double pilot_weapSetRange( Pilot* p, int id, int level );
double pilot_weapSetSpeed( Pilot* p, int id, int level );
void pilot_weapSetCleanup( Pilot* p, int id );
PilotWeaponSetOutfit* pilot_weapSetList( Pilot* p, int id, int *n );


/* Properties. */
int pilot_weapSetTypeCheck( Pilot* p, int id );
void pilot_weapSetType( Pilot* p, int id, int type );
int pilot_weapSetInrangeCheck( Pilot* p, int id );
void pilot_weapSetInrange( Pilot* p, int id, int inrange );


/* High level. */
void pilot_weaponClear( Pilot *p );
void pilot_weaponAuto( Pilot *p );
void pilot_weaponSetDefault( Pilot *p );
void pilot_weaponSane( Pilot *p );
void pilot_afterburn ( Pilot *p );
void pilot_afterburnOver ( Pilot *p );
void pilot_outfitOff( Pilot *p, PilotOutfitSlot *o );
int pilot_outfitOffAll( Pilot *p );
//void pilot_outfitOn( Pilot *p, Outfit *outfit );


#endif /* PILOT_WEAPON_H */
