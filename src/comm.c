/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file comm.c
 *
 * @brief For communicating with planets/pilots.
 */


#include "comm.h"

#include "naev.h"
#include "log.h"
#include "toolkit.h"
#include "pilot.h"


#define BUTTON_WIDTH    80 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


static Pilot *comm_pilot = NULL; /**< Pilot currently talking to. */


/*
 * Prototypes.
 */


/**
 * @brief Opens the communication dialogue with a pilot.
 *
 *    @param pilot Pilot to communicate with.
 *    @return 0 on success.
 */
int comm_open( unsigned int pilot )
{
   int x,y, w;
   glTexture *logo, *gfx_comm;
   char *name, *stand;
   unsigned int wid;

   /* Get the pilot. */
   comm_pilot = pilot_get( pilot );
   if (comm_pilot == NULL)
      return -1;

   /* Get graphics and text. */
   gfx_comm = comm_pilot->ship->gfx_comm;
   logo = faction_logoSmall(comm_pilot->faction);
   name = comm_pilot->name;
   stand = faction_getStandingBroad(faction_getPlayer( comm_pilot->faction ));
   w = MAX(gl_printWidth( NULL, name ), gl_printWidth( NULL, stand ));
   y = gl_defFont.h*2 + 15;
   if (logo != NULL) {
      w += logo->w;
      y = MAX( y, logo->w );
   }
   x = (gfx_comm->w - w) / 2;

   /* Create the window. */
   wid = window_create( "Communication Channel", -1, -1,
         20 + gfx_comm->w + 20 + BUTTON_WIDTH + 20, 30 + gfx_comm->h + y + 5 + 20 );

   /* Create the ship image. */
   window_addRect( wid, 20, -30, gfx_comm->w, gfx_comm->h + y + 5, "rctShip", &cGrey10, 1 );
   window_addImage( wid, 20, -30, "imgShip", gfx_comm, 0 );

   /* Faction logo. */
   if (logo != NULL) {
      window_addImage( wid, x, -30 - gfx_comm->h - 5,
            "imgFaction", logo, 0 );
      x += logo->w + 10;
      y -= (logo->w - (gl_defFont.h*2 + 15)) / 2;
   }
   
   /* Name. */
   window_addText( wid, x, -30 - gfx_comm->h - y + gl_defFont.h*2 + 10,
         gfx_comm->w - x, 20, 0, "txtName",
         NULL, &cDConsole, name );

   /* Standing. */
   window_addText( wid, x, -30 - gfx_comm->h - y + gl_defFont.h + 5,
         gfx_comm->w - x, 20, 0, "txtStanding", NULL,
         faction_getColour( comm_pilot->faction ), stand );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );
   window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnGreet", "Greet", NULL );
   if (faction_getPlayer( comm_pilot->faction ) < 0)
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBribe", "Bribe", NULL );
   else
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnRequest", "Request...", NULL );

   return 0;
}

