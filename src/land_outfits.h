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


#ifndef LAND_OUTFITS_H
#  define LAND_OUTFITS_H


#include "land.h"
#include "outfit.h"


void outfits_open( unsigned int wid );
void outfits_regenList( unsigned int wid, char *str );
void outfits_update( unsigned int wid, char* str );
void outfits_updateEquipmentOutfits( void );
int outfits_filter( Outfit **outfits, glTexture **toutfits, int n,
      int(*filter)( const Outfit *o ), char *name );
int outfit_canBuy( char *outfit, Planet *planet );
int outfit_canSell( char *outfit );
void outfits_cleanup( void );

#endif /* LAND_OUTFITS_H */
