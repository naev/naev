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

   DEBUG("Generation complete!");
}


