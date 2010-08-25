/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_WEAPON_H
#  define PILOT_WEAPON_H


#include "pilot.h"


/* Shooting. */
int pilot_shoot( Pilot* p, int level );
void pilot_shootStop( Pilot* p, int level );


/* Weapon Set. */
void pilot_weapSetExec( Pilot* p, int id );
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o );
void pilot_weapSetRm( Pilot* p, int id, PilotOutfitSlot *o );
void pilot_weapSetCleanup( Pilot* p, int id );


#endif /* PILOT_WEAPON_H */
