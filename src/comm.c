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


#define COMM_WIDTH   320 /**< Communication window width. */
#define COMM_HEIGHT  240 /**< Communication window height. */

#define BUTTON_WIDTH    90 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


static Pilot *comm_pilot = NULL; /**< Pilot currently talking to. */
static int comm_wid = 0; /**< Communication Window ID. */


/*
 * Prototypes.
 */
static void comm_close( char *str );


/**
 * @brief Opens the communication dialogue with a pilot.
 *
 *    @param pilot Pilot to communicate with.
 *    @return 0 on success.
 */
int comm_open( unsigned int pilot )
{
   char buf[128];

   /* Get the pilot. */
   comm_pilot = pilot_get( pilot );
   if (comm_pilot == NULL)
      return -1;
  
   /* Create the window. */
   snprintf(buf, 128, "Comm - %s", comm_pilot->name);
   comm_wid = window_create( buf, -1, -1, COMM_WIDTH, COMM_HEIGHT );

   window_addButton( comm_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close Channel", comm_close );

   return 0;
}
static void comm_close( char *str )
{
   (void) str;
   if (comm_wid > 0) {
      window_destroy( comm_wid );
      comm_wid = 0;
   }
}



