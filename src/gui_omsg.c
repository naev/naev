/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "gui_omsg.h"

#include "array.h"
#include "font.h"
#include "log.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"

/**
 * @brief Fonts.
 */
typedef struct omsg_font_s {
   int    size; /**< Font size. */
   glFont font; /**< Font itself. */
} omsg_font_t;
static omsg_font_t *omsg_font_array = NULL; /**< Array of fonts. */

/**
 * @brief Message struct.
 */
typedef struct omsg_s {
   unsigned int id;       /**< Unique ID. */
   char       **msg;      /**< Array: Message to display. */
   double       duration; /**< Time left. */
   int          font;     /**< Font to use. */
   glColour     col;      /**< Colour to use. */
} omsg_t;
static omsg_t      *omsg_array = NULL; /**< Array of messages. */
static unsigned int omsg_idgen = 0;    /**< Unique ID generator. */

static double omsg_center_x = 0.;   /**< X center of the overlay messages. */
static double omsg_center_y = 0.;   /**< Y center of the overlay messages. */
static double omsg_center_w = 100.; /**< Width of the overlay messages. */

/*
 * Prototypes.
 */
static omsg_t *omsg_get( unsigned int id );
static void    omsg_free( omsg_t *omsg );
static void    omsg_setMsg( omsg_t *omsg, const char *msg );
static int     omsg_getFontID( int size );
static glFont *omsg_getFont( int font );

/**
 * @brief Gets an overlay message from id.
 */
static omsg_t *omsg_get( unsigned int id )
{
   for ( int i = 0; i < array_size( omsg_array ); i++ )
      if ( omsg_array[i].id == id )
         return &omsg_array[i];
   return NULL;
}

/**
 * @brief Frees all internal stuff of a msg.
 */
static void omsg_free( omsg_t *omsg )
{
   for ( int i = 0; i < array_size( omsg->msg ); i++ )
      free( omsg->msg[i] );
   array_free( omsg->msg );
   omsg->msg = NULL;
}

/**
 * @brief Sets the message for an omsg.
 */
static void omsg_setMsg( omsg_t *omsg, const char *msg )
{
   glFont             *font;
   glPrintLineIterator iter;

   /* Clean up after old stuff. */
   omsg_free( omsg );

   /* Create data. */
   font = omsg_getFont( omsg->font );

   omsg->msg = array_create( char * );
   gl_printLineIteratorInit( &iter, font, msg, omsg_center_w );
   while ( gl_printLineIteratorNext( &iter ) )
      array_push_back( &omsg->msg, strndup( &iter.text[iter.l_begin],
                                            iter.l_end - iter.l_begin ) );
}

/**
 * @brief Gets a font by size.
 */
static int omsg_getFontID( int size )
{
   omsg_font_t *font;

   /* Create array if not done so yet. */
   if ( omsg_font_array == NULL )
      omsg_font_array = array_create( omsg_font_t );

   /* Try to match. */
   for ( int i = 0; i < array_size( omsg_font_array ); i++ )
      if ( size == omsg_font_array[i].size )
         return i;

   /* Create font. */
   font = &array_grow( &omsg_font_array );
   gl_fontInit( &font->font, _( FONT_MONOSPACE_PATH ), size, FONT_PATH_PREFIX,
                0 );
   font->size = size;
   return array_size( omsg_font_array ) - 1;
}

/**
 * @brief Gets a font by id.
 */
static glFont *omsg_getFont( int font )
{
   return &omsg_font_array[font].font;
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
void omsg_cleanup( void )
{
   /* Free fonts. */
   for ( int i = 0; i < array_size( omsg_font_array ); i++ )
      gl_freeFont( &omsg_font_array[i].font );
   array_free( omsg_font_array );
   omsg_font_array = NULL;

   /* Destroy messages. */
   for ( int i = 0; i < array_size( omsg_array ); i++ )
      omsg_free( &omsg_array[i] );
   array_free( omsg_array );
   omsg_array = NULL;
}

/**
 * @brief Renders the overlays.
 */
void omsg_render( double dt )
{
   double x, y;

   /* Case nothing to do. */
   if ( omsg_array == NULL )
      return;

   /* Center. */
   x = omsg_center_x - omsg_center_w / 2.;
   y = omsg_center_y;

   /* Render. */
   for ( int i = 0; i < array_size( omsg_array ); i++ ) {
      omsg_t *omsg = &omsg_array[i];

      /* Render. */
      glFont  *font = omsg_getFont( omsg->font );
      glColour col  = omsg->col;
      if ( omsg->duration < 1. )
         col.a = omsg->duration;
      gl_printRestoreClear();
      for ( int j = 0; j < array_size( omsg->msg ); j++ ) {
         y -= font->h * 1.5;
         gl_printRestoreLast();
         gl_printMidRaw( font, omsg_center_w, x, y, &col, -1., omsg->msg[j] );
      }

      /* Check if time to erase. */
      omsg->duration -= dt;
      if ( omsg->duration < 0. ) {
         omsg_free( omsg );
         array_erase( &omsg_array, &omsg[0], &omsg[1] );
         i--;
         continue;
      }
   }
}

/**
 * @brief Adds a message to the overlay.
 *
 *    @param msg Message to add.
 *    @param duration Duration of message on screen (in seconds).
 *    @param fontsize Size of the font to use.
 *    @param col Colour to use.
 *    @return Unique ID to the message.
 */
unsigned int omsg_add( const char *msg, double duration, int fontsize,
                       const glColour *col )
{
   omsg_t *omsg;
   int     font;

   /* Create if necessary. */
   if ( omsg_array == NULL )
      omsg_array = array_create( omsg_t );

   /* Get font size. */
   font = omsg_getFontID( fontsize );

   /* Create the message. */
   omsg = &array_grow( &omsg_array );
   memset( omsg, 0, sizeof( omsg_t ) );
   omsg->id       = ++omsg_idgen;
   omsg->duration = duration;
   omsg->font     = font;
   omsg->col      = *col;
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
   omsg_t *omsg = omsg_get( id );
   if ( omsg == NULL )
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
   return ( omsg_get( id ) != NULL );
}

/**
 * @brief Removes an overlay message.
 *
 *    @param id ID of the message to remove.
 */
void omsg_rm( unsigned int id )
{
   omsg_t *omsg = omsg_get( id );
   if ( omsg == NULL )
      return;

   /* Destroy. */
   omsg_free( omsg );
   array_erase( &omsg_array, &omsg[0], &omsg[1] );
}
