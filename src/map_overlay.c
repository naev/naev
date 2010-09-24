/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map_overlay.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "opengl.h"



static Uint32 ovr_opened = 0; /**< Time last opened. */
static int ovr_open = 0; /**< Is the overlay open? */


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
      ovr_open++;
      ovr_opened  = t;
   }
   else if (type < 0) {
      if (ovr_open > 1)
         ovr_open = 0;
      else if (t - ovr_opened < 300)
         ovr_open = 0;
   }
}


/**
 * @brief Renders the overlay map.
 */
void ovr_render( double dt )
{
   glColour c = { .r=0., .g=0., .b=0., .a=0.5 };

   /* Must be open. */
   if (!ovr_open)
      return;


   /* First render the background overlay. */
   gl_renderRect( 0., 0., SCREEN_W, SCREEN_H, &c );
}

