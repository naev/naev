/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_OUTFIT_H
#  define PILOT_OUTFIT_H


#include "pilot.h"


#define PILOT_OUTFIT_LUA_UPDATE_DT     (1.0/10.0)   /* How often the Lua outfits run their update script (in seconds).  */


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
int pilot_countAmmo( const Pilot* pilot );
int pilot_maxAmmo( const Pilot* pilot );
int pilot_maxAmmoO( const Pilot* p, const Outfit *o );
void pilot_fillAmmo( Pilot* pilot );

/* Checks. */
int pilot_slotsCheckSafety( Pilot *p );
int pilot_slotsCheckRequired( Pilot *p );
const char* pilot_checkSpaceworthy( Pilot *p );
int pilot_reportSpaceworthy( Pilot *p, char buf[], int buffSize );
const char* pilot_canEquip( Pilot *p, PilotOutfitSlot *s, Outfit *o );

/* Lock-ons. */
void pilot_lockUpdateSlot( Pilot *p, PilotOutfitSlot *o, Pilot *t, double *a, double dt );
void pilot_lockClear( Pilot *p );

/* Other. */
char* pilot_getOutfits( const Pilot *pilot );
void pilot_calcStats( Pilot *pilot );
void pilot_updateMass( Pilot *pilot );
void pilot_healLanded( Pilot *pilot );

/* Special outfit stuff. */
int pilot_getMount( const Pilot *p, const PilotOutfitSlot *w, Vector2d *v );

/* Lua outfit stuff. */
int pilot_slotIsActive( const PilotOutfitSlot *o );
void pilot_outfitLInitAll( Pilot *pilot );
int pilot_outfitLInit( Pilot *pilot, PilotOutfitSlot *po );
void pilot_outfitLUpdate( Pilot *pilot, double dt );
void pilot_outfitLOutfofenergy( Pilot *pilot );
void pilot_outfitLOnhit( Pilot *pilot, double armour, double shield, unsigned int attacker );
int pilot_outfitLOntoggle( Pilot *pilot, PilotOutfitSlot *po, int on );
void pilot_outfitLCooldown( Pilot *pilot, int done, int success, double timer );
void pilot_outfitLCleanup( Pilot *pilot );


#endif /* PILOT_OUTFIT_H */
