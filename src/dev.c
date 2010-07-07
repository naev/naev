/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev.c
 *
 * @brief Generic developement routines.
 */

#include "dev.h"

#include "naev.h"

#include "SDL.h"

#include "dev_outfit.h"


/**
 */
void dev_csv (void)
{
   printf("Generating CSV data...\n");

   printf("  bolt.csv...");
   dout_csvBolt( "bolt.csv" );
   printf("done!\n");
}


