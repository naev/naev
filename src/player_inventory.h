/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "player.h"

/**
 * @brief Represents an item in the player inventory.
 */
typedef struct PlayerItem_ {
   char *name;     /**< Name of the item (untranslated). */
   int   quantity; /**< Amount. */
} PlayerItem;

void              player_inventoryClear( void );
const PlayerItem *player_inventory( void );
int               player_inventoryAmount( const char *name );
int               player_inventoryAdd( const char *name, int amount );
int               player_inventoryRemove( const char *name, int amount );
