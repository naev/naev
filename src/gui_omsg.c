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
   char **msg;       /**< Message to display. */
   int nlines;       /**< Message lines. */
   double duration;  /**< Time left. */
   glFont *font;     /**< Font to use. */
   glColour *col;    /**< Colour to use. */
} omsg_t;
static omsg_t *omsg_array    = NULL;  /**< Array of messages. */
static unsigned int omsg_idgen      = 0;     /**< Unique ID generator. */

static double omsg_center_x         = 0.;    /**< X center of the overlay messages. */
static double omsg_center_y         = 0.;    /**< Y center of the overlay messages. */
static double omsg_center_w         = 100.;    /**< Widtho f the overlay messages. */


/*
 * Prototypes.
 */
static omsg_t* omsg_get( unsigned int id );
static void omsg_free( omsg_t *omsg );
static void omsg_setMsg( omsg_t *omsg, const char *msg );


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
 * @brief Frees all internal stuff of a msg.
 */
static void omsg_free( omsg_t *omsg )
{
   int i;

   if (omsg->msg != NULL) {
      for (i=0; i<omsg->nlines; i++)
         free( omsg->msg[i] );
      free( omsg->msg );
   }
}


/**
 * @brief Sets the message for an omsg.
 */
static void omsg_setMsg( omsg_t *omsg, const char *msg )
{
   int i, l, n, s, m;

   /* Clean up after old stuff. */
   if (omsg->msg != NULL) {
      for (i=0; i<omsg->nlines; i++)
         free( omsg->msg[i] );
      free( omsg->msg );

      omsg->msg    = 0;
      omsg->nlines = 0;
   }

   /* Create data. */
   l  = strlen( msg );
   /* First pass size. */
   n  = 0;
   m  = 0;
   while (n < l) {
      s  = gl_printWidthForText( omsg->font, &msg[n], omsg_center_w );
      n += s+1;
      m++;
   }
   /* Second pass allocate. */
   omsg->msg = malloc( m * sizeof(char*) );
   omsg->nlines = m;
   n  = 0;
   m  = 0;
   while (n < l) {
      s  = gl_printWidthForText( omsg->font, &msg[n], omsg_center_w );
      omsg->msg[m] = malloc( s+1 );
      snprintf( omsg->msg[m], s+1, "%s", &msg[n] );
      m++;
      n += s+1;
   }
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
      omsg_free( &omsg_array[i] );

   /* Destroy. */
   if (omsg_array != NULL)
      array_free( omsg_array );
   omsg_array = NULL;
}


/**
 * @brief Renders the overlays.
 */
void omsg_render( double dt )
{
   int i, j;
   double x, y;
   omsg_t *omsg;

   /* Case nothing to do. */
   if (omsg_array == NULL)
      return;

   /* Center. */
   x  = omsg_center_x - omsg_center_w/2.;
   y  = omsg_center_y;

   /* Render. */
   for (i=0; i<array_size(omsg_array); i++) {
      omsg  = &omsg_array[i];

      /* Check if time to erase. */
      omsg->duration -= dt;
      if (omsg->duration < 0.) {
         omsg_free( omsg );
         array_erase( &omsg_array, &omsg[0], &omsg[1] );
         i--;
         continue;
      }

      /* Must have a message. */
      if (omsg->msg == NULL)
         continue;

      /* Render. */
      for (j=0; j<omsg->nlines; j++) {
         gl_printMidRaw( omsg->font, omsg_center_w, x, y, omsg->col, omsg->msg[j] );
         y -= omsg->font->h * 1.5;
      }
   }
}


/**
 * @brief Adds a message to the overlay.
 *
 *    @param msg Message to add.
 *    @param duration Duration of message on screen (in seconds).
 *    @return Unique ID to the message.
 */
unsigned int omsg_add( const char *msg, double duration )
{
   omsg_t *omsg;

   /* Create if necessary. */
   if (omsg_array == NULL)
      omsg_array = array_create( omsg_t );

   /* Create the message. */
   omsg           = &array_grow( &omsg_array );
   memset( omsg, 0, sizeof(omsg_t) );
   omsg->id       = ++omsg_idgen;
   omsg->duration = duration;
   omsg->font     = &gl_defFont;
   omsg->col      = &cWhite;
   omsg_setMsg( omsg, msg );

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
int omsg_change( unsigned int id, const char *msg, double duration )
{
   omsg_t *omsg;

   omsg = omsg_get(id);
   if (omsg == NULL)
      return -1;

   omsg_setMsg( omsg, msg );
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
   omsg_free( omsg );
   array_erase( &omsg_array, &omsg[0], &omsg[1] );
}




