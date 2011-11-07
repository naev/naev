/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev.c
 *
 * @brief Generic development routines.
 */

#include "dev.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "nfile.h"
#include "dev_outfit.h"
#include "dev_ship.h"


#define CSV_DIR      "naev_csv" /**< Name of the directory to create all the csv data into. */


/**
 * @brief Generates the naev CSV stuff.
 */
void dev_csv (void)
{
   DEBUG("Generating CSV data...");

   /* Create directory. */
   if (nfile_dirMakeExist( CSV_DIR )) {
      WARN( "Unable to generate 'naev_csv' dir." );
      return;
   }

   DEBUG("   bolt.csv...");
   dout_csvBolt( CSV_DIR"/bolt.csv" );

   DEBUG("   ship.csv...");
   dship_csv( CSV_DIR"/ship.csv" );

   DEBUG("Generation complete!");
}


