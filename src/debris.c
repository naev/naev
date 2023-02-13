/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file debris.c
 *
 * @brief Handles scattering debris around.
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "debris.h"

#include "log.h"
#include "nstring.h"
#include "pilot.h"
#include "rng.h"
#include "spfx.h"

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
      snprintf( buf, sizeof(buf), "Dbr%d", i );
      i++;
   } while (spfx_get(buf) != -1);
   debris_nspfx = i-1;

   /* Check to make sure they exist. */
   if (debris_nspfx <= 0) {
      WARN(_("No debris special effects found."));
      return -1;
   }

   /* Allocate. */
   debris_spfx = malloc( sizeof(int) * debris_nspfx );

   /* Second pass to fill. */
   for (i=0; i<debris_nspfx; i++) {
      snprintf( buf, sizeof(buf), "Dbr%d", i );
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
   int n;

   if (!space_isSimulationEffects())
      return;

   /* Lazy allocator. */
   if (debris_spfx == NULL)
      if (debris_load() < 0)
         return;

   /* Get number of debris to render. */
   n = (int) ceil( sqrt(mass) / 1.5 );

   /* Now add the spfx. */
   for (int i=0; i<n; i++) {
      double npx,npy, nvx,nvy;
      double a, d;

      /* Get position. */
      d = r/2. * RNG_2SIGMA();
      a = RNGF()*2.*M_PI;
      npx = px + d*cos(a);
      npy = py + d*sin(a);

      /* Get velocity. */
      d = n * RNG_2SIGMA();
      a = RNGF()*2.*M_PI;
      nvx = vx + d*cos(a);
      nvy = vy + d*sin(a);

      /* Create sprite. */
      spfx_add( debris_spfx[ RNG( 0, debris_nspfx-1 ) ],
            npx, npy, nvx, nvy, RNG(0,1) );
   }
}
