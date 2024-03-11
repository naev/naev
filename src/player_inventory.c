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

/**
 * @brief Clears the inventory and frees memory.
 */
void player_inventoryClear (void)
{
   for (int i=0; i<array_size(inventory); i++)
      item_free( &inventory[i] );
   array_free( inventory );
   inventory = NULL;
}

/**
 * @brief Gets the whole player inventory.
 */
const PlayerItem* player_inventory (void)
{
   return inventory;
}

/**
 * @brief Gets the amount of an item the player has.
 *
 *    @param name Name of the item to try to get.
 *    @return Amount the player has or 0 if none.
 */
int player_inventoryAmount( const char *name )
{
   for (int i=0; i<array_size(inventory); i++) {
      const PlayerItem *pi = &inventory[i];
      if (strcmp( pi->name, name )==0)
         return pi->quantity;
   }
   return 0;
}

/**
 * @brief Adds an item to the player inventory.
 *
 *    @param name Item to add.
 *    @param amount Amount to add.
 */
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

/**
 * @brief Removes an item from the player inventory.
 *
 *    @param name Item to remove.
 *    @param amount Amount to remove.
 */
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
