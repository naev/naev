/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "land.h"
#include "outfit.h"
#include "pilot.h"
#include "tk/widget/imagearray.h"

int outfit_altText( char *buf, int n, const Outfit *o, const Pilot *plt );

void outfits_open( unsigned int wid, const Outfit **outfits, int blackmarket );
void outfits_regenList( unsigned int wid, const char *str );
void outfits_update( unsigned int wid, const char* str );
void outfits_updateEquipmentOutfits( void );
int outfits_filter( const Outfit **outfits, int n,
      int(*filter)( const Outfit *o ), const char *name );
ImageArrayCell *outfits_imageArrayCells( const Outfit **outfits, int *n, const Pilot *plt );
int outfit_canBuy( const Outfit *outfit, int blackmarket );
int outfit_canSell( const Outfit *outfit );
void outfits_cleanup( void );
