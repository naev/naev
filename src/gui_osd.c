/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "gui_osd.h"

#include "array.h"
#include "font.h"
#include "log.h"
#include "nstring.h"
#include "ntracing.h"
#include "opengl.h"

/**
 * @brief On Screen Display element.
 */
typedef struct OSD_s {
   unsigned int id;         /**< OSD id. */
   int          priority;   /**< Priority level. */
   int          skip;       /**< Not rendered. */
   int          hide;       /**< Actively hidden. */
   int          duplicates; /**< How many duplicates of this OSD there are. */
   char        *title;      /**< Title of the OSD. */
   char       **titlew;     /**< Wrapped version of the title. */

   char  **msg;   /**< Array (array.h): Stored messages. */
   char ***items; /**< Array of array (array.h) of allocated strings. */

   unsigned int active; /**< Active item. */
} OSD_t;

/*
 * OSD array.
 */
static unsigned int osd_idgen = 0;    /**< ID generator for OSD. */
static OSD_t       *osd_list  = NULL; /**< Array (array.h) for OSD. */

/*
 * Dimensions.
 */
static int osd_x         = 0;
static int osd_y         = 0;
static int osd_w         = 0;
static int osd_h         = 0;
static int osd_lines     = 0;
static int osd_rh        = 0;
static int osd_tabLen    = 0;
static int osd_hyphenLen = 0;

/*
 * Prototypes.
 */
static OSD_t *osd_get( unsigned int osd );
static int    osd_free( OSD_t *osd );
static void   osd_calcDimensions( void );
/* Sort. */
static int  osd_sortCompare( const void *arg1, const void *arg2 );
static void osd_sort( void );
static void osd_wordwrap( OSD_t *osd );

static int osd_sortCompare( const void *arg1, const void *arg2 )
{
   const OSD_t *osd1, *osd2;
   int          ret, m;

   osd1 = (OSD_t *)arg1;
   osd2 = (OSD_t *)arg2;

   /* Compare priority. */
   if ( osd1->priority > osd2->priority )
      return +1;
   else if ( osd1->priority < osd2->priority )
      return -1;

   /* Compare name. */
   ret = strcmp( osd1->title, osd2->title );
   if ( ret != 0 )
      return ret;

   /* Compare items. */
   m = MIN( array_size( osd1->items ), array_size( osd2->items ) );
   for ( int i = 0; i < m; i++ ) {
      ret = strcmp( osd1->msg[i], osd2->msg[i] );
      if ( ret != 0 )
         return ret;
   }

   /* Compare on length. */
   if ( array_size( osd1->items ) > array_size( osd2->items ) )
      return +1;
   if ( array_size( osd1->items ) < array_size( osd2->items ) )
      return -1;

   /* Compare ID. */
   if ( osd1->id > osd2->id )
      return +1;
   else if ( osd1->id < osd2->id )
      return -1;
   return 0;
}

/**
 * @brief Sorts the OSD list.
 */
static void osd_sort( void )
{
   qsort( osd_list, array_size( osd_list ), sizeof( OSD_t ), osd_sortCompare );
}

/**
 * @brief Creates an on-screen display.
 *
 *    @param title Title of the display.
 *    @param nitems Number of items in the display.
 *    @param items Items in the display.
 *    @return ID of newly created OSD.
 */
unsigned int osd_create( const char *title, int nitems, const char **items,
                         int priority )
{
   int    id;
   OSD_t *osd;

   /* Create. */
   if ( osd_list == NULL )
      osd_list = array_create( OSD_t );
   osd = &array_grow( &osd_list );
   memset( osd, 0, sizeof( OSD_t ) );
   osd->id = id = ++osd_idgen;
   osd->active  = 0;

   /* Copy text. */
   osd->title    = strdup( title );
   osd->priority = priority;
   osd->msg      = array_create_size( char *, nitems );
   osd->items    = array_create_size( char **, nitems );
   osd->titlew   = array_create( char   *);
   for ( int i = 0; i < nitems; i++ ) {
      array_push_back( &osd->msg, strdup( items[i] ) );
      array_push_back( &osd->items, array_create( char * ) );
   }

   osd_sort(); /* THIS INVALIDATES THE osd POINTER. */
   osd_calcDimensions();

   return id;
}

/**
 * @brief Calculates the word-wrapped osd->items from osd->msg.
 */
static void osd_wordwrap( OSD_t *osd )
{
   glPrintLineIterator iter;
   char title[STRMAX_SHORT]; /* Needs to be in the scope of the entire function
                                as it is used in gl_printLineIteratorNext
                                indirectly. */

   /* Do title. */
   for ( int i = 0; i < array_size( osd->titlew ); i++ )
      free( osd->titlew[i] );
   array_resize( &osd->titlew, 0 );

   /* Handle the case same mission is repeated. */
   if ( osd->duplicates > 0 ) {
      snprintf( title, sizeof( title ), _( "%s #b(%dx)#0" ), osd->title,
                osd->duplicates + 1 );
      gl_printLineIteratorInit( &iter, &gl_smallFont, title, osd_w );
   } else
      gl_printLineIteratorInit( &iter, &gl_smallFont, osd->title, osd_w );

   /* Figure out the length. */
   while ( gl_printLineIteratorNext( &iter ) ) {
      /* Copy text over. */
      int   chunk_len = iter.l_end - iter.l_begin + 1;
      char *chunk     = malloc( chunk_len );
      snprintf( chunk, chunk_len, "%s", &iter.text[iter.l_begin] );
      array_push_back( &osd->titlew, chunk );
   }

   /* Do items. */
   for ( int i = 0; i < array_size( osd->items ); i++ ) {
      int         msg_len, w, has_tab;
      const char *chunk_fmt;
      for ( int l = 0; l < array_size( osd->items[i] ); l++ )
         free( osd->items[i][l] );
      array_resize( &osd->items[i], 0 );

      msg_len = strlen( osd->msg[i] );
      if ( msg_len == 0 )
         continue;

      /* Test if tabbed. */
      has_tab = !!( osd->msg[i][0] == '\t' );
      w       = osd_w - ( has_tab ? osd_tabLen : osd_hyphenLen );
      gl_printLineIteratorInit( &iter, &gl_smallFont, &osd->msg[i][has_tab],
                                w );
      chunk_fmt = has_tab ? "   %s" : "- %s";

      while ( gl_printLineIteratorNext( &iter ) ) {
         /* Copy text over. */
         int   chunk_len = iter.l_end - iter.l_begin + strlen( chunk_fmt ) - 1;
         char *chunk     = malloc( chunk_len );
         snprintf( chunk, chunk_len, chunk_fmt, &iter.text[iter.l_begin] );
         array_push_back( &osd->items[i], chunk );
         chunk_fmt  = has_tab ? "   %s" : "%s";
         iter.width = has_tab ? osd_w - osd_tabLen - osd_hyphenLen
                              : osd_w - osd_hyphenLen;
      }
   }
}

/**
 * @brief Gets an OSD by ID.
 *
 *    @param osd ID of the OSD to get.
 */
static OSD_t *osd_get( unsigned int osd )
{
   for ( int i = 0; i < array_size( osd_list ); i++ ) {
      OSD_t *ll = &osd_list[i];
      if ( ll->id == osd )
         return ll;
   }
   WARN( _( "OSD '%d' not found." ), osd );
   return NULL;
}

/**
 * @brief Frees an OSD struct.
 */
static int osd_free( OSD_t *osd )
{
   free( osd->title );
   for ( int i = 0; i < array_size( osd->items ); i++ ) {
      free( osd->msg[i] );
      for ( int j = 0; j < array_size( osd->items[i] ); j++ )
         free( osd->items[i][j] );
      array_free( osd->items[i] );
   }
   array_free( osd->msg );
   array_free( osd->items );
   for ( int i = 0; i < array_size( osd->titlew ); i++ )
      free( osd->titlew[i] );
   array_free( osd->titlew );

   return 0;
}

/**
 * @brief Destroys an OSD.
 *
 *    @param osd ID of the OSD to destroy.
 */
int osd_destroy( unsigned int osd )
{
   for ( int i = 0; i < array_size( osd_list ); i++ ) {
      OSD_t *ll = &osd_list[i];
      if ( ll->id != osd )
         continue;

      /* Clean up. */
      osd_free( &osd_list[i] );

      /* Remove. */
      array_erase( &osd_list, &osd_list[i], &osd_list[i + 1] );

      /* Recalculate dimensions. */
      osd_calcDimensions();

      /* Remove the OSD, if empty. */
      if ( array_size( osd_list ) == 0 )
         osd_exit();

      /* Done here. */
      return 0;
   }

   WARN( _( "OSD '%u' not found to destroy." ), osd );
   return 0;
}

/**
 * @brief Makes an OSD message active.
 *
 *    @param osd OSD to change active message.
 *    @param msg Message to make active in OSD.
 *    @return 0 on success.
 */
int osd_active( unsigned int osd, int msg )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;

   if ( ( msg < 0 ) || ( msg >= array_size( o->items ) ) ) {
      WARN( _( "OSD '%s' only has %d items (requested %d)" ), o->title,
            array_size( o->items ), msg );
      return -1;
   }

   o->active = msg;
   osd_calcDimensions();
   return 0;
}

/**
 * @brief Gets the active OSD MESSAGE>
 *
 *    @param osd OSD to get active message.
 *    @return The active OSD message or -1 on error.
 */
int osd_getActive( unsigned int osd )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;

   return o->active;
}

/**
 * @brief Sets up the OSD window.
 *
 *    @param x X position to render at.
 *    @param y Y position to render at.
 *    @param w Width to render.
 *    @param h Height to render.
 */
int osd_setup( int x, int y, int w, int h )
{
   /* Set offsets. */
   osd_x     = x;
   osd_y     = y;
   osd_w     = w;
   osd_lines = h / ( gl_smallFont.h + 5 );
   osd_h     = h - h % ( gl_smallFont.h + 5 );

   /* Calculate some font things. */
   osd_tabLen    = gl_printWidthRaw( &gl_smallFont, "   " );
   osd_hyphenLen = gl_printWidthRaw( &gl_smallFont, "- " );

   osd_calcDimensions();

   return 0;
}

/**
 * @brief Destroys all the OSD.
 */
void osd_exit( void )
{
   for ( int i = 0; i < array_size( osd_list ); i++ ) {
      OSD_t *ll = &osd_list[i];
      osd_free( ll );
   }

   array_free( osd_list );
   osd_list = NULL;
}

/**
 * @brief Renders all the OSD.
 */
void osd_render( void )
{
   double p;
   int    l;

   /* Nothing to render. */
   if ( osd_list == NULL )
      return;

   NTracingZone( _ctx, 1 );

   /* Background. */
   gl_renderPane( osd_x - 5., osd_y - ( osd_rh + 5. ), osd_w + 10., osd_rh + 10,
                  &cBlackHilight );

   /* Render each thingy. */
   p = osd_y - gl_smallFont.h;
   l = 0;
   for ( int k = 0; k < array_size( osd_list ); k++ ) {
      int    x, w;
      OSD_t *ll = &osd_list[k];

      if ( ll->skip )
         continue;

      x = osd_x;
      w = osd_w;

      /* Print title. */
      for ( int i = 0; i < array_size( ll->titlew ); i++ ) {
         gl_printMaxRaw( &gl_smallFont, w, x, p, NULL, -1., ll->titlew[i] );
         p -= gl_smallFont.h + 5.;
         l++;
      }
      if ( l >= osd_lines ) {
         NTracingZoneEnd( _ctx );
         return;
      }

      /* Print items. */
      for ( int i = ll->active; i < array_size( ll->items ); i++ ) {
         const glColour *c =
            ( i == (int)ll->active ) ? &cFontWhite : &cFontGrey;
         x = osd_x;
         for ( int j = 0; j < array_size( ll->items[i] ); j++ ) {
            gl_printRaw( &gl_smallFont, x, p, c, -1., ll->items[i][j] );
            if ( j == 0 )
               x = osd_x + osd_hyphenLen;
            p -= gl_smallFont.h + 5.;
            l++;
            if ( l >= osd_lines ) {
               NTracingZoneEnd( _ctx );
               return;
            }
         }
      }
   }
   NTracingZoneEnd( _ctx );
}

/**
 * @brief Calculates and sets the length of the OSD.
 */
static void osd_calcDimensions( void )
{
   double len;

   /* Nothing to render. */
   if ( osd_list == NULL )
      return;

   /* Reset skips. */
   for ( int k = 0; k < array_size( osd_list ); k++ ) {
      OSD_t *ll      = &osd_list[k];
      ll->skip       = ll->hide;
      ll->duplicates = 0;
   }

   /* Get duplicates. */
   for ( int k = 0; k < array_size( osd_list ); k++ ) {
      int    duplicates;
      OSD_t *ll = &osd_list[k];

      if ( ll->skip )
         continue;

      /* Check how many duplicates we have, mark duplicates for ignoring */
      duplicates = 0;
      for ( int m = k + 1; m < array_size( osd_list ); m++ ) {
         OSD_t *lm = &osd_list[m];

         if ( ( strcmp( lm->title, ll->title ) == 0 ) &&
              ( array_size( lm->items ) == array_size( ll->items ) ) &&
              ( lm->active == ll->active ) ) {
            int is_duplicate = 1;
            for ( int i = lm->active; i < array_size( lm->items ); i++ ) {
               if ( array_size( lm->items[i] ) == array_size( ll->items[i] ) ) {
                  for ( int j = 0; j < array_size( lm->items[i] ); j++ ) {
                     if ( strcmp( lm->items[i][j], ll->items[i][j] ) != 0 ) {
                        is_duplicate = 0;
                        break;
                     }
                  }
               } else {
                  is_duplicate = 0;
               }
               if ( !is_duplicate )
                  break;
            }
            if ( is_duplicate ) {
               duplicates++;
               lm->skip = 1;
            }
         }
      }
      ll->duplicates = duplicates;
   }

   /* Compute total length. */
   len = 0;
   for ( int k = 0; k < array_size( osd_list ); k++ ) {
      OSD_t *ll = &osd_list[k];

      if ( ll->skip )
         continue;

      /* Wordwrap. */
      osd_wordwrap( ll );

      /* Print title. */
      for ( int i = 0; i < array_size( ll->titlew ); i++ )
         len += gl_smallFont.h + 5.;

      /* Print items. */
      for ( int i = ll->active; i < array_size( ll->items ); i++ )
         for ( int j = 0; j < array_size( ll->items[i] ); j++ )
            len += gl_smallFont.h + 5.;
   }
   osd_rh = MIN( len, osd_h );
}

/**
 * @brief Gets the title of an OSD.
 *
 *    @param osd OSD to get title of.
 *    @return Title of the OSD.
 */
char *osd_getTitle( unsigned int osd )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return NULL;

   return o->title;
}

/**
 * @brief Gets the items of an OSD.
 *
 *    @param osd OSD to get items of.
 *    @return Array (array.h) of OSD strings.
 */
char **osd_getItems( unsigned int osd )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return NULL;
   return o->msg;
}

/**
 * @brief Sets the hidden state of an OSD.
 *
 *    @param osd OSD to set state of.
 *    @param state Whether or not to hide the OSD.
 */
int osd_setHide( unsigned int osd, int state )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;

   o->hide = state;
   osd_calcDimensions();
   return 0;
}

/**
 * @brief Gets the hidden state of an OSD.
 *
 *    @param osd OSD to get state of.
 *    @return Whether or not the OSD is hidden or -1 on error.
 */
int osd_getHide( unsigned int osd )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;
   return o->hide;
}

/**
 * @brief Sets the priority of the OSD.
 *
 *    @param osd OSD to set priority of.
 *    @param state Priority of the OSD (lower is more important).
 */
int osd_setPriority( unsigned int osd, int priority )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;

   o->priority = priority;
   osd_sort();
   osd_calcDimensions();
   return 0;
}

/**
 * @brief Gets the priority of an OSD.
 *
 *    @param osd OSD to get state of.
 *    @return The priority of the OSD.
 */
int osd_getPriority( unsigned int osd )
{
   OSD_t *o = osd_get( osd );
   if ( o == NULL )
      return -1;
   return o->priority;
}
