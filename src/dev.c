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

#include "log.h"
#include "dev_outfit.h"
#include "dev_ship.h"


/**
 */
void dev_csv (void)
{
   DEBUG("Generating CSV data...");

   DEBUG("   bolt.csv...");
   dout_csvBolt( "bolt.csv" );
   DEBUG("\b   done!");

   DEBUG("   ship.csv...");
   dship_csv( "ship.csv" );
   DEBUG("\b   done!");
}


