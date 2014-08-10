/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef EQUIPMENT_H
#  define EQUIPMENT_H


#include "pilot.h"
#include "outfit.h"


/*
 * Main.
 */
void equipment_open( unsigned int wid );
void equipment_cleanup (void);


/*
 * Misc.
 */
void equipment_addAmmo (void);
void equipment_regenLists( unsigned int wid, int outfits, int ships );
void equipment_updateShips( unsigned int wid, char* str );
void equipment_updateOutfits( unsigned int wid, char* str );
int equipment_shipStats( char *buf, int max_len,  const Pilot *s, int dpseps );


/**
 * Custom slot widget.
 */
typedef struct CstSlotWidget_ {
   Pilot *selected; /**< Selected pilot ship. */
   Outfit *outfit; /**< Selected outfit. */
   int slot; /**< Selected equipment slot. */
   int mouseover; /**< Mouse over slot. */
   double altx; /**< Alt X text position. */
   double alty; /**< Alt Y text position. */
   int canmodify; /**< Whether or not it can modify stuff. */
   int weapons; /**< Hack to render colours for currently selected weapon set. */
} CstSlotWidget; /**< Slot widget. */
void equipment_slotWidget( unsigned int wid,
      double x, double y, double w, double h,
      CstSlotWidget *data );


#endif /* EQUIPMENT_H */
