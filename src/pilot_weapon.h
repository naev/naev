/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

#include "target.h"

#define WEAPSET_INRANGE_PLAYER_DEF  0 /**< Default weaponset inrange parameter for the player. */

/* Freedom. */
void pilot_weapSetFree( Pilot* p );

/* Shooting. */
Pilot *pilot_weaponTarget( Pilot *p, Target *wt );
int pilot_shoot( Pilot* p, int level );
void pilot_shootStop( Pilot* p, int level );
int pilot_shootWeapon( Pilot *p, PilotOutfitSlot *w, const Target *target, double time, int aim );
void pilot_stopBeam( const Pilot *p, PilotOutfitSlot *w );
void pilot_getRateMod( double *rate_mod, double* energy_mod,
      const Pilot* p, const Outfit* o );
double pilot_weapFlyTime( const Outfit *o, const Pilot *parent,
      const vec2 *pos, const vec2 *vel);

/* Updating. */
void pilot_weapSetUpdateStats( Pilot *p );
void pilot_weapSetAIClear( Pilot* p );
void pilot_weapSetPress( Pilot* p, int id, int type );
void pilot_weapSetUpdate( Pilot* p );

/* Weapon Set. */
PilotWeaponSet* pilot_weapSet( Pilot* p, int id );
const char *pilot_weapSetName( Pilot* p, int id );
const char *pilot_weapSetTypeName( WeaponSetType t );
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o, int level );
void pilot_weapSetRm( Pilot* p, int id, PilotOutfitSlot *o );
void pilot_weapSetClear( Pilot* p, int id );
int pilot_weapSetInSet( Pilot* p, int id, const PilotOutfitSlot *o );
int pilot_weapSetCheck( Pilot* p, int id, const PilotOutfitSlot *o );
double pilot_weapSetRange( Pilot* p, int id, int level );
double pilot_weapSetSpeed( Pilot* p, int id, int level );
double pilot_weapSetAmmo( Pilot *p, int id, int level );
void pilot_weapSetCleanup( Pilot* p, int id );
PilotWeaponSetOutfit* pilot_weapSetList( Pilot* p, int id );
void pilot_weapSetCopy( PilotWeaponSet *dest, const PilotWeaponSet *src );

/* Properties. */
int pilot_weapSetTypeCheck( Pilot* p, int id );
void pilot_weapSetType( Pilot* p, int id, WeaponSetType type );
int pilot_weapSetInrangeCheck( Pilot* p, int id );
void pilot_weapSetInrange( Pilot* p, int id, int inrange );
int pilot_weapSetManualCheck( Pilot *p, int id );
void pilot_weapSetManual( Pilot* p, int id, int manual );
int pilot_weapSetVolleyCheck( Pilot *p, int id );
void pilot_weapSetVolley( Pilot* p, int id, int volley );

/* High level. */
void pilot_weaponClear( Pilot *p );
void pilot_weaponAuto( Pilot *p );
void pilot_weaponSetDefault( Pilot *p );
void pilot_weaponSafe( Pilot *p );
void pilot_afterburn ( Pilot *p );
void pilot_afterburnOver ( Pilot *p );
int pilot_outfitOff( Pilot *p, PilotOutfitSlot *o );
int pilot_outfitOffAll( Pilot *p );
int pilot_outfitOn( Pilot *p, PilotOutfitSlot *o );

/* Weaponset stuff. */
void ws_copy( PilotWeaponSet dest[PILOT_WEAPON_SETS], const PilotWeaponSet src[PILOT_WEAPON_SETS] );
void ws_free( PilotWeaponSet ws[PILOT_WEAPON_SETS] );
