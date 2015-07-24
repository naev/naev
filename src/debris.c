/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file debris.c
 *
 * @brief Handles scattering debris around.
 */


#include "debris.h"

#include "naev.h"

#include "log.h"
#include "pilot.h"
#include "spfx.h"
#include "rng.h"
#include "nstring.h"


static int *debris_spfx = NULL; /**< Debris special effects. */
static int debris_nspfx = 0; /**< Number of debris special effects. */


/**
 * @brief Cleans up after the debris.
 */
void debris_cleanup (void)
{
   free(debris_spfx);
   debris_spfx = NULL;
}


/**
 * @brief Loads the debris spfx into an array.
 *
 *    @return 0 on success.
 */
static int debris_load (void)
{
   int i;
   char buf[32];

   /* Calculate amount. */
   i = 0;
   do {
      nsnprintf( buf, sizeof(buf), "Dbr%d", i );
      i++;
   } while (spfx_get(buf) != -1);
   debris_nspfx = i-1;

   /* Check to make sure they exist. */
   if (debris_nspfx <= 0) {
      WARN("No debris special effects found.");
      return -1;
   }

   /* Allocate. */
   debris_spfx = malloc( sizeof(int) * debris_nspfx );

   /* Second pass to fill. */
   for (i=0; i<debris_nspfx; i++) {
      nsnprintf( buf, sizeof(buf), "Dbr%d", i );
      debris_spfx[i] = spfx_get(buf);
   }

   return 0;
}


/**
 * @brief Creates a cloud of debris.
 *
 *    @param mass Mass of the debris cloud.
 *    @param r Radius of the cloud.
 *    @param px X position to center cloud.
 *    @param py Y position to center cloud.
 *    @param vx X velocity of the cloud center.
 *    @param vy Y velocity of the cloud center.
 */
void debris_add( double mass, double r, double px, double py,
      double vx, double vy )
{
   int i, n;
   double npx,npy, nvx,nvy;
   double a, d;

   /* Lazy allocator. */
   if (debris_spfx == NULL)
      if (debris_load() < 0)
         return;

   /* Get number of debris to render. */
   n = (int) ceil( sqrt(mass) / 1.5 );

   /* Now add the spfx. */
   for (i=0; i<n; i++) {
      /* Get position. */
      d = r/2. * RNG_2SIGMA();
      a = RNGF()*2*M_PI;
      npx = px + d*cos(a);
      npy = py + d*sin(a);

      /* Get velocity. */
      d = n * RNG_2SIGMA();
      a = RNGF()*2*M_PI;
      nvx = vx + d*cos(a);
      nvy = vy + d*sin(a);

      /* Createsprite. */
      spfx_add( debris_spfx[ RNG( 0, debris_nspfx-1 ) ],
            npx, npy, nvx, nvy, RNG(0,1) );
   }
}

