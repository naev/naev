/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev.c
 *
 * @brief Generic development routines.
 */

/** @cond */
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "dev.h"

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
   DEBUG(_("Generating CSV data..."));

   /* Create directory. */
   if (nfile_dirMakeExist( CSV_DIR )) {
      WARN( _("Unable to generate 'naev_csv' dir.") );
      return;
   }

   DEBUG("   bolt.csv...");
   dout_csvBolt( CSV_DIR"/bolt.csv" );

   DEBUG("   beam.csv...");
   dout_csvBeam( CSV_DIR"/beam.csv" );

   DEBUG("   launcher.csv...");
   dout_csvLauncher( CSV_DIR"/launcher.csv" );

   DEBUG("   ammo.csv...");
   dout_csvAmmo( CSV_DIR"/ammo.csv" );

   DEBUG("   mod.csv...");
   dout_csvMod( CSV_DIR"/mod.csv" );

   DEBUG("   ship.csv...");
   dship_csv( CSV_DIR"/ship.csv" );

   DEBUG("   ship_stat.csv...");
   dship_csvStat( CSV_DIR"/ship_stat.csv" );

   DEBUG(_("Generation complete!"));
}


