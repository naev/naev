/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_weapon.c
 *
 * @brief Handles pilot weapon sets.
 */


#include "pilot.h"

#include "naev.h"

#include "nxml.h"

#include "log.h"
#include "array.h"


/*
 * Prototypes.
 */
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id );
static void pilot_weapSetFire( Pilot *p, PilotWeaponSet *set );


/**
 * @brief Gets a weapon set from id.
 *
 *    @param p Pilot to get weapon set from.
 *    @param id ID of the weapon set.
 *    @return The weapon set matching id.
 */
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id )
{
   return &p->weapon_sets[ id ];
}


/**
 * @brief Fires a weapon set.
 */
static void pilot_weapSetFire( Pilot *p, PilotWeaponSet *set )
{
}


/**
 * @brief Executes a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 */
void pilot_weapSetExec( Pilot* p, int id )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   if (ws->fire)
      pilot_weapSetFire( p, ws );
   else
      p->active_set = id;
}


/**
 * @brief Adds an outfit to a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param o Outfit to add.
 */
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o )
{
   PilotWeaponSet *ws;
   PilotOutfitSlot **slot;
   int i;

   ws = pilot_weapSet(p,id);

   /* Create if needed. */
   if (ws->slots == NULL)
      ws->slots = array_create( PilotOutfitSlot* );

   /* Check if already there. */
   for (i=0; i<array_size(&ws->slots); i++)
      if (ws->slots[i] == o)
         return;

   /* Add it. */
   slot = &array_grow( &ws->slots );
   *slot = o;
}


/**
 * @brief Removes a slot from a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @param o Outfit to remove.
 */
void pilot_weapSetRm( Pilot* p, int id, PilotOutfitSlot *o )
{
   PilotWeaponSet *ws;
   int i;

   /* Make sure it has slots. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return;

   /* Find the slot. */
   for (i=0; i<array_size(&ws->slots); i++) {
      if (ws->slots[i] == o) {
         array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );
         return;
      }
   }
}


/**
 * @brief Cleans up a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set to clean up.
 */
void pilot_weapSetCleanup( Pilot* p, int id )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);

   if (ws->slots != NULL)
      array_free( ws->slots );
   ws->slots = NULL;
}



