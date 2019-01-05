/*
 * See Licensing and Copyright notice in naev.h
 */


#include "plasmaf.h"

#include "naev.h"

#include "math.h"

#include "SDL.h"

#include "log.h"
#include "rng.h"
#include "opengl.h"


/*
 * prototypes
 */
static double* pf_genFractalMap( const int w, const int h, double rug );
static void pf_divFractal( double *map, const double x, const double y,
      const double w, const double h, const double rw, const double rh,
      double c1, double c2, double c3, double c4, double rug );


/*
 * actually generates the fractal and loads it up in an opengl texture
 */
glTexture* pf_genFractal( const int w, const int h, double rug )
{
   int i;
   double *map;
   SDL_Surface *sur;
   uint32_t *pix;
   glTexture *tex;
   double c;

   map = pf_genFractalMap( w, h, rug );

   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* convert from mapping to actual colours */
   SDL_LockSurface( sur );
   for (i=0; i<h*w; i++) {
      c = map[i];
      pix[i] = RMASK + BMASK + GMASK + (AMASK & (uint32_t)(AMASK*c));
   }
   SDL_UnlockSurface( sur );

   free(map);

   tex = gl_loadImage( sur, 0 );

   return tex;
}


/*
 * generates a map representing the fractal
 */
static double* pf_genFractalMap( const int w, const int h, double rug )
{
   double *map; /* we'll use it to map out the fractal before saving */
   double cx, cy;

   map = malloc( w*h * sizeof(double) );
   if (map == NULL) {
      WARN(_("Out of Memory"));
      return NULL;
   }

   /* set up initial values */
   cx = (double)w/2.;
   cy = (double)h/2.;

   /* start by doing the four squares */
   pf_divFractal( map, 0., 0., cx, cy, w, h, 0., 0., 1., 0., rug );
   pf_divFractal( map, cx, 0., cx, cy, w, h, 0., 0., 0., 1., rug );
   pf_divFractal( map, cx, cy, cx, cy, w, h, 1., 0., 0., 0., rug );
   pf_divFractal( map, 0., cy, cx, cy, w, h, 0., 1., 0., 0., rug );

   return map;
}


static void pf_divFractal( double *map, const double x, const double y,
      const double w, const double h, const double rw, const double rh,
      double c1, double c2, double c3, double c4, double rug )
{
   double nw, nh; /* new dimensions */
   double m, e1,e2,e3,e4; /* middle and edges */

   /* still need to subdivide */
   if ((w>1.) || (h>1.)) {
      /* calculate new dimensions */
      nw = w/2.;
      nh = h/2.;

      /* edges */
      m = (c1 + c2 + c3 + c4)/4.;
      e1 = (c1 + c2)/2.;
      e2 = (c2 + c3)/2.;
      e3 = (c3 + c4)/2.;
      e4 = (c4 + c1)/2.;

      /* now change the middle colour */
      m += rug*(RNGF()-0.5) * ((nw+nh)/(rw+rh) * 3.);
      if (m < 0.) m = 0.;
      else if (m > 1.) m = 1.;

      /* recursivation */
      pf_divFractal( map, x,    y,    nw, nh, rw, rh, c1, e1, m,  e4, rug );
      pf_divFractal( map, x+nw, y,    nw, nh, rw, rh, e1, c2, e2, m,  rug );
      pf_divFractal( map, x+nw, y+nh, nw, nh, rw, rh, m,  e2, c3, e3, rug );
      pf_divFractal( map, x,    y+nh, nw, nh, rw, rh, e4, m,  e3, c4, rug );
   }
   else /* actually write the pixel */
      map[(int)y*(int)rw + (int)x] = (c1 + c2 + c3 + c4)/4.;
}




