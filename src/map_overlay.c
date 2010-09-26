/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map_overlay.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "opengl.h"
#include "gui.h"
#include "pilot.h"
#include "player.h"
#include "space.h"


static Uint32 ovr_opened = 0; /**< Time last opened. */
static int ovr_open = 0; /**< Is the overlay open? */
static double ovr_res = 10.; /**< Resolution. */


/**
 * @brief Check to see if the map overlay is open.
 */
int ovr_isOpen (void)
{
   return !!ovr_open;
}


/**
 * @brief Handles a keypress event.
 */
void ovr_key( int type )
{
   Uint32 t;

   t = SDL_GetTicks();

   if (type > 0) {
      if (ovr_open)
         ovr_open = 0;
      else {
         /* We need to calculate the radius of the rendering. */
         ovr_res = (cur_system->radius + 10.) / (MIN( SCREEN_W, SCREEN_H )/2.);

         ovr_open = 1;
         ovr_opened  = t;
      }
   }
   else if (type < 0) {
      if (t - ovr_opened > 300)
         ovr_open = 0;
   }
}


/**
 * @brief Renders the overlay map.
 */
void ovr_render( double dt )
{
   (void) dt;
   int i, j;
   Pilot **pstk;
   int n;
   double w, h, res;
   glColour c = { .r=0., .g=0., .b=0., .a=0.5 };

   /* Must be open. */
   if (!ovr_open)
      return;

   /* Default values. */
   w     = SCREEN_W;
   h     = SCREEN_H;
   res   = ovr_res;

   /* First render the background overlay. */
   gl_renderRect( 0., 0., w, h, &c );

   /* We need to center in the image first. */
   gl_matrixPush();
      gl_matrixTranslate( w/2., h/2. );

   /* Render planets. */
   for (i=0; i<cur_system->nplanets; i++)
      if ((cur_system->planets[ i ]->real == ASSET_REAL) && (i != player.p->nav_planet))
         gui_renderPlanet( i, RADAR_RECT, w, h, res, 1 );
   if (player.p->nav_planet > -1)
      gui_renderPlanet( player.p->nav_planet, RADAR_RECT, w, h, res, 1 );

   /* Render jump points. */
   for (i=0; i<cur_system->njumps; i++)
      if (i != player.p->nav_hyperspace)
         gui_renderJumpPoint( i, RADAR_RECT, w, h, res, 1 );
   if (player.p->nav_hyperspace > -1)
      gui_renderJumpPoint( player.p->nav_hyperspace, RADAR_RECT, w, h, res, 1 );

   /* Render pilots. */
   pstk  = pilot_getAll( &n );
   j     = 0;
   for (i=0; i<n; i++) {
      if (pstk[i]->id == PLAYER_ID) /* Skip player. */
         continue;
      if (pstk[i]->id == player.p->target)
         j = i;
      else
         gui_renderPilot( pstk[i], RADAR_RECT, w, h, res, 1 );
   }
   /* render the targetted pilot */
   if (j!=0)
      gui_renderPilot( pstk[j], RADAR_RECT, w, h, res, 1 );
   
   /* Render the player. */
   gui_renderPlayer( res, 1 );

   /* Pop the matrix. */
   gl_matrixPop();
}


