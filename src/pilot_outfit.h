/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_OUTFIT_H
#  define PILOT_OUTFIT_H


#include "pilot.h"


/* Raw changes. */
int pilot_addOutfitRaw( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s );
int pilot_addOutfitTest( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s, int warn );
int pilot_rmOutfitRaw( Pilot* pilot, PilotOutfitSlot *s );

/* Changes with checks. */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s );
int pilot_rmOutfit( Pilot* pilot, PilotOutfitSlot *s );

/* Ammo. */
int pilot_addAmmo( Pilot* pilot, PilotOutfitSlot *s, Outfit* ammo, int quantity );
int pilot_rmAmmo( Pilot* pilot, PilotOutfitSlot *s, int quantity );

/* Checks. */
const char* pilot_checkSanity( Pilot *p );
const char* pilot_canEquip( Pilot *p, PilotOutfitSlot *s, Outfit *o, int add );

/* Lockons. */
void pilot_lockUpdateSlot( Pilot *p, PilotOutfitSlot *o, Pilot *t, double *a, double dt );
void pilot_lockClear( Pilot *p );

/* Other. */
char* pilot_getOutfits( Pilot* pilot );
void pilot_calcStats( Pilot* pilot );
void pilot_updateMass( Pilot *pilot );

/* Special outfit stuff. */
int pilot_getMount( const Pilot *p, const PilotOutfitSlot *w, Vector2d *v );


#endif /* PILOT_OUTFIT_H */
