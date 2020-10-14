/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_OUTFITS_H
#  define LAND_OUTFITS_H


#include "land.h"
#include "outfit.h"
#include "tk/widget/imagearray.h"


void outfits_open( unsigned int wid, Outfit **outfits, int noutfits );
void outfits_regenList( unsigned int wid, char *str );
void outfits_update( unsigned int wid, char* str );
void outfits_updateEquipmentOutfits( void );
int outfits_filter( Outfit **outfits, int n,
      int(*filter)( const Outfit *o ), char *name );
ImageArrayCell *outfits_imageArrayCells( Outfit **outfits, int *n );
int             outfit_canBuy( const char *outfit, Planet *planet );
int             outfit_canSell( const char *outfit );
void outfits_cleanup( void );


#endif /* LAND_OUTFITS_H */
