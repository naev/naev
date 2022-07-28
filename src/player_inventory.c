/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player_inventory.c
 *
 * @brief Inventory management for the player items.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "player_inventory.h"

#include "array.h"

static PlayerItem *inventory = NULL; /**< Items the player has. */

static void item_free( PlayerItem *pi )
{
   free( pi->name );
}

void player_inventoryClear (void)
{
   for (int i=0; i<array_size(inventory); i++)
      item_free( &inventory[i] );
   array_free( inventory );
   inventory = NULL;
}

PlayerItem* player_inventory (void)
{
   return inventory;
}

int player_inventoryAdd( const char *name, int amount )
{
   PlayerItem npi;
   if (inventory==NULL)
      inventory = array_create( PlayerItem );

   for (int i=0; i<array_size(inventory); i++) {
      PlayerItem *pi = &inventory[i];
      if (strcmp( pi->name, name )==0) {
         pi->quantity += amount;
         return amount;
      }
   }

   npi.name = strdup( name );
   npi.quantity = amount;
   array_push_back( &inventory, npi );

   return 0;
}

int player_inventoryRemove( const char *name, int amount )
{
   for (int i=0; i<array_size(inventory); i++) {
      PlayerItem *pi = &inventory[i];
      if (strcmp( pi->name, name )==0) {
         int q = pi->quantity;
         pi->quantity = MAX( 0, pi->quantity-amount );
         q -= pi->quantity;
         if (pi->quantity == 0) {
            item_free( pi );
            array_erase( &inventory, &pi[0], &pi[1] );
         }
         return q;
      }
   }
   return 0;
}
