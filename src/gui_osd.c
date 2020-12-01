/*
 * See Licensing and Copyright notice in naev.h
 */


#include "gui_osd.h"

#include "naev.h"

#include <stdlib.h>
#include "nstring.h"

#include "log.h"
#include "opengl.h"
#include "font.h"
#include "array.h"


/**
 * @brief On Screen Display element.
 */
typedef struct OSD_s {
   unsigned int id; /**< OSD id. */
   int priority; /**< Priority level. */
   char *title; /**< Title of the OSD. */

   char **msg; /**< Stored messages. */
   char ***items; /**< Array of array (array.h) of allocated strings. */

   int active; /**< Active item. */
} OSD_t;


/*
 * OSD array.
 */
static unsigned int osd_idgen = 0; /**< ID generator for OSD. */
static OSD_t *osd_list        = NULL; /**< Array (array.h) for OSD. */


/*
 * Dimensions.
 */
static int osd_x = 0;
static int osd_y = 0;
static int osd_w = 0;
static int osd_h = 0;
static int osd_lines = 0;
static int osd_rh = 0;
static int osd_tabLen = 0;
static int osd_hyphenLen = 0;


/*
 * Prototypes.
 */
static OSD_t *osd_get( unsigned int osd );
static int osd_free( OSD_t *osd );
static void osd_calcDimensions (void);
/* Sort. */
static int osd_sortCompare( const void * arg1, const void * arg2 );
static void osd_sort (void);
static void osd_wordwrap( OSD_t* osd );


static int osd_sortCompare( const void *arg1, const void *arg2 )
{
   const OSD_t *osd1, *osd2;
   int ret, i, m;

   osd1 = (OSD_t*)arg1;
   osd2 = (OSD_t*)arg2;

   /* Compare priority. */
   if (osd1->priority > osd2->priority)
      return +1;
   else if (osd1->priority < osd2->priority)
      return -1;

   /* Compare name. */
   ret = strcmp( osd1->title, osd2->title );
   if (ret != 0)
      return ret;

   /* Compare items. */
   m = MIN(array_size(osd1->items), array_size(osd2->items));
   for (i=0; i<m; i++) {
      ret = strcmp( osd1->msg[i], osd2->msg[i] );
      if (ret != 0)
         return ret;
   }

   /* Compare on length. */
   if (array_size(osd1->items) > array_size(osd2->items))
      return +1;
   if (array_size(osd1->items) < array_size(osd2->items))
      return -1;

   /* Compare ID. */
   if (osd1->id > osd2->id)
      return +1;
   else if (osd1->id < osd2->id)
      return -1;
   return 0;
}


/**
 * @brief Sorts the OSD list.
 */
static void osd_sort (void)
{
   qsort( osd_list, array_size(osd_list), sizeof(OSD_t), osd_sortCompare );
}


/**
 * @brief Creates an on-screen display.
 *
 *    @param title Title of the display.
 *    @param nitems Number of items in the display.
 *    @param items Items in the display.
 *    @return ID of newly created OSD.
 */
unsigned int osd_create( const char *title, int nitems, const char **items, int priority )
{
   int i, id;
   OSD_t *osd;

   /* Create. */
   if (osd_list == NULL)
      osd_list = array_create( OSD_t );
   osd = &array_grow( &osd_list );
   memset( osd, 0, sizeof(OSD_t) );
   osd->id = id = ++osd_idgen;
   osd->active = 0;

   /* Copy text. */
   osd->title  = strdup(title);
   osd->priority = priority;
   osd->msg = malloc( sizeof(char*) * nitems );
   osd->items = array_create_size( char**, nitems );
   for (i=0; i<nitems; i++) {
      osd->msg[i] = strdup( items[i] );
      array_push_back( &osd->items, array_create(char*));
   }

   osd_wordwrap( osd );
   osd_sort(); /* THIS INVALIDATES THE osd POINTER. */
   osd_calcDimensions();

   return id;
}


/**
 * @brief Calculates the word-wrapped osd->items from osd->msg.
 */
void osd_wordwrap( OSD_t* osd )
{
   int i, n, l, s, w, t;
   char *chunk;
   for (i=0; i<array_size(osd->items); i++) {
      for (l=0; l<array_size(osd->items[i]); l++)
         free(osd->items[i][l]);
      array_resize( &osd->items[i], 0 );

      l = strlen(osd->msg[i]); /* Message length. */
      n = 0; /* Text position. */
      t = 0; /* Tabbed? */
      w = osd_w-osd_hyphenLen;
      while (n < l) {
         /* Test if tabbed. */
         if (n==0) {
            if (osd->msg[i][n] == '\t') {
               t = 1;
               w = osd_w - osd_tabLen;
            }
            else {
               t = 0;
               w = osd_w - osd_hyphenLen;
            }
         }

         /* Get text size. */
         s = gl_printWidthForText( &gl_smallFont, &osd->msg[i][n], w );

         if (n==0 && t==1)
            w -= osd_hyphenLen;

         /* Copy text over. */
         if (n==0) {
            if (t==1) {
               chunk = malloc(s+4);
               nsnprintf( chunk, s+4, "   %s", &osd->msg[i][n+1] );
            }
            else {
               chunk = malloc(s+3);
               nsnprintf( chunk, s+3, "- %s", &osd->msg[i][n] );
            }
         }
         else if (t==1) {
            chunk = malloc(s+4);
            nsnprintf( chunk, s+4, "   %s", &osd->msg[i][n] );
         }
         else {
            chunk = malloc(s+1);
            nsnprintf( chunk, s+1, "%s", &osd->msg[i][n] );
         }
         array_push_back( &osd->items[i], chunk );

         /* Go to next line. */
         n += s + 1;
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
   int i;
   OSD_t *ll;

   for (i=0; i<array_size(osd_list); i++) {
      ll = &osd_list[i];
      if (ll->id == osd)
         return ll;
   }

   WARN(_("OSD '%d' not found."), osd);
   return NULL;
}


/**
 * @brief Frees an OSD struct.
 */
static int osd_free( OSD_t *osd )
{
   int i, j;

   if (osd->title != NULL)
      free(osd->title);

   for (i=0; i<array_size(osd->items); i++) {
      free( osd->msg[i] );
      for (j=0; j<array_size(osd->items[i]); j++)
         free(osd->items[i][j]);
      array_free(osd->items[i]);
   }
   free(osd->msg);
   array_free(osd->items);

   return 0;
}


/**
 * @brief Destroys an OSD.
 *
 *    @param osd ID of the OSD to destroy.
 */
int osd_destroy( unsigned int osd )
{
   int i;
   OSD_t *ll;

   for (i=0; i<array_size( osd_list ); i++) {
      ll = &osd_list[i];
      if (ll->id != osd)
         continue;

      /* Clean up. */
      osd_free( &osd_list[i] );

      /* Remove. */
      array_erase( &osd_list, &osd_list[i], &osd_list[i+1] );

      /* Recalculate dimensions. */
      osd_calcDimensions();

      /* Remove the OSD, if empty. */
      if (array_size(osd_list) == 0)
         osd_exit();

      /* Done here. */
      return 0;
   }

   WARN(_("OSD '%u' not found to destroy."), osd );
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
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL)
      return -1;

   if ((msg < 0) || (msg >= array_size(o->items))) {
      WARN(_("OSD '%s' only has %d items (requested %d)"), o->title, array_size(o->items), msg );
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
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL)
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
   int i, must_rewrap;
   /* Set offsets. */
   must_rewrap = (osd_w != w) && (osd_list != NULL);
   osd_x = x;
   osd_y = y;
   osd_w = w;
   osd_lines = h / (gl_smallFont.h+5);
   osd_h = h - h % (gl_smallFont.h+5);

   /* Calculate some font things. */
   osd_tabLen = gl_printWidthRaw( &gl_smallFont, "   " );
   osd_hyphenLen = gl_printWidthRaw( &gl_smallFont, "- " );

   if (must_rewrap)
      for (i=0; i<array_size(osd_list); i++)
         osd_wordwrap( &osd_list[i] );
   osd_calcDimensions();

   return 0;
}


/**
 * @brief Destroys all the OSD.
 */
void osd_exit (void)
{
   int i;
   OSD_t *ll;

   if (osd_list == NULL)
      return;

   for (i=0; i<array_size(osd_list); i++) {
      ll = &osd_list[i];
      osd_free( ll );
   }

   array_free( osd_list );
   osd_list = NULL;
}


/**
 * @brief Renders all the OSD.
 */
void osd_render (void)
{
   OSD_t *ll;
   double p;
   int i, j, k, l, m;
   int w, x;
   const glColour *c;
   int *ignore;
   int nignore;
   int is_duplicate, duplicates;
   char title[1024];

   /* Nothing to render. */
   if (osd_list == NULL)
      return;

   nignore = array_size(osd_list);
   ignore  = calloc( nignore, sizeof( int ) );

   /* Background. */
   gl_renderRect( osd_x-5., osd_y-(osd_rh+5.), osd_w+10., osd_rh+10, &cBlackHilight );

   /* Render each thingy. */
   p = osd_y-gl_smallFont.h;
   l = 0;
   for (k=0; k<array_size(osd_list); k++) {
      if (ignore[k])
         continue;

      ll = &osd_list[k];
      x = osd_x;
      w = osd_w;

      /* Check how many duplicates we have, mark duplicates for ignoring */
      duplicates = 0;
      for (m=k+1; m<array_size(osd_list); m++) {
         if ((strcmp(osd_list[m].title, ll->title) == 0) &&
               (array_size(osd_list[m].items) == array_size(ll->items)) &&
               (osd_list[m].active == ll->active)) {
            is_duplicate = 1;
            for (i=osd_list[m].active; i<array_size(osd_list[m].items); i++) {
               if (array_size(osd_list[m].items[i]) == array_size(ll->items[i])) {
                  for (j=0; j<array_size(osd_list[m].items[i]); j++) {
                     if (strcmp(osd_list[m].items[i][j], ll->items[i][j]) != 0 ) {
                        is_duplicate = 0;
                        break;
                     }
                  }
               } else {
                  is_duplicate = 0;
               }
               if (!is_duplicate)
                  break;
            }
            if (is_duplicate) {
               duplicates++;
               ignore[m] = 1;
            }
         }
      }

      /* Print title. */
      if (duplicates > 0)
         nsnprintf( title, sizeof(title), "%s (%d)", ll->title, duplicates + 1 );
      else
         strncpy( title, ll->title, sizeof(title)-1 );
      title[sizeof(title)-1] = '\0';
      gl_printMaxRaw( &gl_smallFont, w, x, p, NULL, -1., title);
      p -= gl_smallFont.h + 5.;
      l++;
      if (l >= osd_lines) {
         free(ignore);
         return;
      }

      /* Print items. */
      for (i=ll->active; i<array_size(ll->items); i++) {
         x = osd_x;
         w = osd_w;
         c = (ll->active == i) ? &cFontWhite : &cFontGrey;
         for (j=0; j<array_size(ll->items[i]); j++) {
            gl_printMaxRaw( &gl_smallFont, w, x, p,
                  c, -1., ll->items[i][j] );
            if (j==0) {
               w = osd_w - osd_hyphenLen;
               x = osd_x + osd_hyphenLen;
            }
            p -= gl_smallFont.h + 5.;
            l++;
            if (l >= osd_lines) {
               free(ignore);
               return;
            }
         }
      }
   }

   free(ignore);
}


/**
 * @brief Calculates and sets the length of the OSD.
 */
static void osd_calcDimensions (void)
{
   OSD_t *ll;
   int i, j, k, m;
   double len;
   int *ignore;
   int nignore;
   int is_duplicate, duplicates;

   /* Nothing to render. */
   if (osd_list == NULL)
      return;

   nignore = array_size(osd_list);
   ignore  = calloc( nignore, sizeof( int ) );

   /* Render each thingy. */
   len = 0;
   for (k=0; k<array_size(osd_list); k++) {
      if (ignore[k])
         continue;

      ll = &osd_list[k];

      /* Check how many duplicates we have, mark duplicates for ignoring */
      duplicates = 0;
      for (m=k+1; m<array_size(osd_list); m++) {
         if ((strcmp(osd_list[m].title, ll->title) == 0) &&
               (array_size(osd_list[m].items) == array_size(ll->items)) &&
               (osd_list[m].active == ll->active)) {
            is_duplicate = 1;
            for (i=osd_list[m].active; i<array_size(osd_list[m].items); i++) {
               if (array_size(osd_list[m].items[i]) == array_size(ll->items[i])) {
                  for (j=0; j<array_size(osd_list[m].items[i]); j++) {
                     if (strcmp(osd_list[m].items[i][j], ll->items[i][j]) != 0 ) {
                        is_duplicate = 0;
                        break;
                     }
                  }
               } else {
                  is_duplicate = 0;
               }
               if (!is_duplicate)
                  break;
            }
            if (is_duplicate) {
               duplicates++;
               ignore[m] = 1;
            }
         }
      }

      /* Print title. */
      len += gl_smallFont.h + 5.;

      /* Print items. */
      for (i=ll->active; i<array_size(ll->items); i++)
         for (j=0; j<array_size(ll->items[i]); j++)
            len += gl_smallFont.h + 5.;
   }
   osd_rh = MIN( len, osd_h );
   free(ignore);
}


/**
 * @brief Gets the title of an OSD.
 *
 *    @param osd OSD to get title of.
 *    @return Title of the OSD.
 */
char *osd_getTitle( unsigned int osd )
{
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL)
      return NULL;

   return o->title;
}


/**
 * @brief Gets the items of an OSD.
 *
 *    @param osd OSD to get items of.
 *    @param[out] nitems Number of OSD items.
 */
char **osd_getItems( unsigned int osd, int *nitems )
{
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL) {
      *nitems = 0;
      return NULL;
   }

   *nitems = array_size(o->items);
   return o->msg;
}


