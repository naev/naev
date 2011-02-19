/*
 * See Licensing and Copyright notice in naev.h
 */


#include "gui_omsg.h"

#include "naev.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "opengl.h"
#include "font.h"
#include "array.h"


/**
 * @brief Message struct.
 */
typedef struct omsg_s {
   unsigned int id;  /**< Unique ID. */
   char *msg;        /**< Message to display. */
   double duration;  /**< Time left. */
} omsg_t;
static omsg_t *omsg_array    = NULL;  /**< Array of messages. */
static unsigned int omsg_idgen      = 0;     /**< Unique ID generator. */

static double omsg_center_x         = 0.;    /**< X center of the overlay messages. */
static double omsg_center_y         = 0.;    /**< Y center of the overlay messages. */
static double omsg_center_w         = 0.;    /**< Widtho f the overlay messages. */


/*
 * Prototypes.
 */
static omsg_t* omsg_get( unsigned int id );


/**
 * @brief Gets an overlay message from id.
 */
static omsg_t* omsg_get( unsigned int id )
{
   int i;
   for (i=0; i<array_size(omsg_array); i++)
      if (omsg_array[i].id == id)
         return &omsg_array[i];
   return NULL;
}


/**
 * @brief Positions the overlay messages.
 *
 *    @param center_x X center.
 *    @param center_y Y center.
 *    @param width Width to target.
 */
void omsg_position( double center_x, double center_y, double width )
{
   omsg_center_x = center_x;
   omsg_center_y = center_y;
   omsg_center_w = width;
}

/**
 * @brief Cleans up after the overlay.
 */
void omsg_cleanup (void)
{
   int i;

   /* Free memory. */
   for (i=0; i<array_size(omsg_array); i++)
      if (omsg_array[i].msg != NULL)
         free( omsg_array[i].msg );

   /* Destroy. */
   array_free( &omsg_array );
   omsg_array = NULL;
}


/**
 * @brief Updates the overlay messages.
 *
 *    @param dt Current delta tick.
 */
void omsg_update( double dt )
{
   int i;
   omsg_t *omsg;

   /* Free memory. */
   for (i=0; i<array_size(omsg_array); i++) {
      omsg = &omsg_array[i];
      omsg->duration -= dt;
      if (omsg->duration < 0.) {
         if (omsg->msg != NULL)
            free( omsg->msg );
         array_erase( &omsg_array, &omsg[0], &omsg[1] );
         i--;
      }
   }
}


/**
 * @brief Renders the overlays.
 */
void omsg_render (void)
{
   int i;
   omsg_t *omsg;

   /* Free memory. */
   for (i=0; i<array_size(omsg_array); i++) {
      omsg = &omsg_array[i];
   }
}


/**
 * @brief Adds a message to the overlay.
 *
 *    @param msg Message to add.
 *    @param duration Duration of message on screen (in seconds).
 *    @return Unique ID to the message.
 */
unsigned int omsg_add( char *msg, double duration )
{
   omsg_t *omsg;

   /* Create if necessary. */
   if (omsg_array == NULL)
      omsg_array = array_create( omsg_t );

   /* Create the message. */
   omsg           = &array_grow( &omsg_array );
   memset( omsg, 0, sizeof(omsg_t) );
   omsg->msg      = strdup( msg );
   omsg->id       = ++omsg_idgen;
   omsg->duration = duration;

   return omsg->id;
}


/**
 * @brief Changes an overlay message.
 *
 *    @param id ID of the message to change.
 *    @param msg New message for the overlay.
 *    @param duration New duration for the overlay.
 *    @return 0 on success.
 */
int omsg_change( unsigned int id, char *msg, double duration )
{
   omsg_t *omsg;

   omsg = omsg_get(id);
   if (omsg == NULL)
      return -1;

   if (omsg->msg != NULL)
      free( omsg->msg );

   omsg->msg      = strdup( msg );
   omsg->duration = duration;
   return 0;
}


/**
 * @brief Checks to see if an overlay message exists.
 *
 *    @param id ID of the message to check if exists.
 *    @return 1 if it exists, 0 otherwise.
 */
int omsg_exists( unsigned int id )
{
   return (omsg_get(id) != NULL);
}


/**
 * @brief Removes an overlay message.
 *
 *    @param id ID of the message to remove.
 */
void omsg_rm( unsigned int id )
{
   omsg_t *omsg;

   omsg = omsg_get(id);
   if (omsg == NULL)
      return;

   /* Destroy. */
   if (omsg->msg != NULL)
      free( omsg->msg );
   array_erase( &omsg_array, &omsg[0], &omsg[1] );
}




