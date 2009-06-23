/*
 * See Licensing and Copyright notice in naev.h
 */


#include "gui_osd.h"

#include "naev.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "opengl.h"
#include "font.h"


/**
 * @brief On Screen Display message element.
 */
typedef struct OSDmsg_s {
   char **chunks; /**< Chunks of the message. */
   int nchunks; /**< Number of chunks message is chopped into. */
} OSDmsg_s;


/**
 * @brief On Screen Display element.
 */
typedef struct OSD_s {
   struct OSD_s *next; /**< Next OSD in the linked list. */

   unsigned int id; /**< OSD id. */
   char *title;

   char **msg; /**< Stored messages. */
   OSDmsg_s *items; /**< Items on the list. */
   int nitems; /**< Number of items on the list. */

   int active; /**< Active item. */
} OSD_t;


/*
 * OSD linked list.
 */
static unsigned int osd_idgen = 0; /**< ID generator for OSD. */
static OSD_t *osd_list        = NULL; /**< Linked list for OSD. */


/*
 * DImensions.
 */
static int osd_x = 0;
static int osd_y = 0;
static int osd_w = 0;
static int osd_h = 0;


/*
 * Prototypes.
 */
static OSD_t *osd_get( unsigned int osd );
static int osd_free( OSD_t *osd );


/**
 * @brief Creates an on-screen display.
 *
 *    @param title Title of the display.
 *    @param nitems Number of items in the display.
 *    @param items Items in the display.
 *    @return ID of newly created OSD.
 */
unsigned int osd_create( const char *title, int nitems, const char **items )
{
   int i, j, n, m, l, s;
   OSD_t *osd, *ll;

   /* Create. */
   osd         = calloc( 1, sizeof(OSD_t) );
   osd->next   = NULL;
   osd->id     = ++osd_idgen;
   osd->active = 0;

   /* Copy text. */
   osd->title  = strdup(title);
   osd->msg    = malloc( sizeof(char*) * nitems );
   osd->items  = malloc( sizeof(OSDmsg_s) * nitems );
   osd->nitems = nitems;
   for (i=0; i<osd->nitems; i++) {
      osd->msg[i] = strdup( items[i] );

      l = strlen(osd->msg[i]);
      n = 0;
      j = 0;
      m = 0;
      osd->items[i].chunks = NULL;
      while (n < l) {
         /* Get text size. */
         s = gl_printWidthForText( &gl_smallFont, &items[i][n], osd_w );

         if (j+1 > m) {
            m += 32;
            osd->items[i].chunks = realloc( osd->items[i].chunks, m );
         }

         /* Copy text over. */
         osd->items[i].chunks[j]    = malloc(s+1);
         strncpy( osd->items[i].chunks[j], &items[i][n], s );
         osd->items[i].chunks[j][s] = '\0';

         /* Go t onext line. */
         n += s + 1;
         j++;
      }
      osd->items[i].nchunks = j;
   }

   /* Append to linked list. */
   if (osd_list == NULL)
      osd_list = osd;
   else {
      for (ll = osd_list; ll->next != NULL; ll = ll->next);
      ll->next = osd;
   }

   return osd->id;
}


/**
 * @brief Gets an OSD by ID.
 *
 *    @param osd ID of the OSD to get.
 */
static OSD_t *osd_get( unsigned int osd )
{
   OSD_t *ll;

   for (ll = osd_list; ll != NULL; ll = ll->next) {
      if (ll->id == osd)
         break;
   }

   if (ll == NULL)
      WARN("OSD '%d' not found.", osd);
   return ll;
}


/**
 * @brief Frees an OSD struct.
 */
static int osd_free( OSD_t *osd )
{
   int i, j;

   if (osd->title != NULL)
      free(osd->title);

   for(i=0; i<osd->nitems; i++) {
      free( osd->msg[i] );
      for (j=0; j<osd->items[i].nchunks; j++)
         free(osd->items[i].chunks[j]);
   }
   free(osd->msg);
   free(osd->items);

   free(osd);

   return 0;
}


/**
 * @brief Destroys an OSD.
 *
 *    @param osd ID of the OSD to destroy.
 */
int osd_destroy( unsigned int osd )
{
   OSD_t *ll, *lp;

   lp = NULL;
   for (ll = osd_list; ll != NULL; ll = ll->next) {

      /* Matches. */
      if (ll->id == osd) {

         /* Remove from list. */
         if (lp == NULL)
            osd_list = ll->next;
         else
            lp->next = ll->next;

         /* Free. */
         osd_free( ll );

         return 0;
      }

      /* Save last iteration. */
      lp = ll;
   }

   return -1;
}


/**
 * @brief Makes an OSD message active.
 *
 *    @param osd OSD to change active message.
 *    @param msg Message to make active in OSD.
 */
int osd_active( unsigned int osd, int msg )
{
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL)
      return -1;

   if ((msg < 0) || (msg >= o->nitems)) {
      WARN("OSD '%s' only has %d items (requested %d)", o->title, o->nitems, msg );
      return -1;
   }

   o->active = msg;
   return 0;
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
   osd_x = x;
   osd_y = y;
   osd_w = w;
   osd_h = h;
   return 0;
}


/**
 * @brief Destroys all the OSD.
 */
void osd_exit (void)
{
   while (osd_list != NULL)
      osd_destroy(osd_list->id);
}


/**
 * @brief Renders all the OSD.
 */
void osd_render (void)
{
   OSD_t *ll;
   double p;
   int i, j;

   /* Nothing to render. */
   if (osd_list == NULL)
      return;

   /* Render each thingy. */
   p = osd_y;
   for (ll = osd_list; ll != NULL; ll = ll->next) {

      /* Print title. */
      gl_printMaxRaw( &gl_smallFont, osd_w, osd_x, p, NULL, ll->title );
      p -= gl_smallFont.h + 5.;
      if (p < osd_y-osd_h)
         return;

      /* Print items. */
      for (i=0; i<ll->nitems; i++) {
         for (j=0; j<ll->items[i].nchunks; j++) {
            gl_printMaxRaw( &gl_smallFont, osd_w, osd_x+10., p,
                 (ll->active == i) ? &cConsole : NULL, ll->items[i].chunks[j] );
            p -= gl_smallFont.h + 5.;
            if (p < osd_y-osd_h)
               return;
         }
      }
   }
}


/**
 * @brief Gets the title of an OSD.
 *
 *    @param osd OSD to get title of.
 *    @return Title of the OSd.
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
 *    @param[out] nitems Numeb of OSD items.
 */
char **osd_getItems( unsigned int osd, int *nitems )
{
   OSD_t *o;

   o = osd_get(osd);
   if (o == NULL) {
      *nitems = 0;
      return NULL;
   }

   *nitems = o->nitems;
   return o->msg;
}


