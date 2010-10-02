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
 * @brief Handles input to the map overlay.
 */
int ovr_input( SDL_Event *event )
{
   int mx, my;
   double x, y;

   /* We only want mouse events. */
   if (event->type != SDL_MOUSEBUTTONDOWN)
      return 0;
  
   /* Autogo. */
   if (event->button.button == SDL_BUTTON_RIGHT) {
      /* Translate from window to screen. */
      mx = event->button.x;
      my = event->button.y;
      gl_windowToScreenPos( &mx, &my, mx, my );

      /* Translate to space coords. */
      x  = ((double)mx - SCREEN_W/2.) * ovr_res;
      y  = ((double)my - SCREEN_H/2.) * ovr_res;

      /* Go to position. */
      player_autonavPos( x, y );

      return 1;
   }
   
   return 0;
}


/**
 * @brief Refreshes the map overlay recalculating the dimensions it should have.
 *
 * This should be called if the planets or the likes change at any given time.
 */
void ovr_refresh (void)
{
   double max_x, max_y;
   int i;

   /* Must be open. */
   if (!ovr_isOpen())
      return;

   /* Calculate max size. */
   max_x = 0.;
   max_y = 0.;
   for (i=0; i<cur_system->njumps; i++) {
      max_x = MAX( max_x, ABS(cur_system->jumps[i].pos.x) );
      max_y = MAX( max_y, ABS(cur_system->jumps[i].pos.y) );
   }
   for (i=0; i<cur_system->nplanets; i++) {
      max_x = MAX( max_x, ABS(cur_system->planets[i]->pos.x) );
      max_y = MAX( max_y, ABS(cur_system->planets[i]->pos.y) );
   }

   /* We need to calculate the radius of the rendering. */
   ovr_res = 2. * 1.2 * MAX( max_x / SCREEN_W, max_y / SCREEN_H );
}


static void ovr_setOpen( int open )
{
   if (open) {
      ovr_open = 1;
      SDL_ShowCursor( SDL_ENABLE );
   }
   else {
      ovr_open = 0;
      SDL_ShowCursor( SDL_DISABLE );
   }
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
         ovr_setOpen(0);
      else {
         ovr_setOpen(1);
         ovr_opened  = t;

         /* Refresh overlay size. */
         ovr_refresh();
      }
   }
   else if (type < 0) {
      if (t - ovr_opened > 300)
         ovr_setOpen(0);
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


