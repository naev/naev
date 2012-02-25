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
   int priority; /**< Priority level. */
   char *title; /**< Title of the OSD. */

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
   int i, j, n, m, l, s, w, t;
   OSD_t *osd, *ll;

   /* Create. */
   osd         = calloc( 1, sizeof(OSD_t) );
   osd->next   = NULL;
   osd->id     = ++osd_idgen;
   osd->active = 0;

   /* Copy text. */
   osd->title  = strdup(title);
   osd->priority = priority;
   osd->msg    = malloc( sizeof(char*) * nitems );
   osd->items  = malloc( sizeof(OSDmsg_s) * nitems );
   osd->nitems = nitems;
   for (i=0; i<osd->nitems; i++) {
      osd->msg[i] = strdup( items[i] );

      l = strlen(osd->msg[i]); /* Message length. */
      n = 0; /* Text position. */
      j = 0; /* Lines. */
      m = 0; /* Allocated Memory. */
      t = 0; /* Tabbed? */
      osd->items[i].chunks = NULL;
      w = osd_w-osd_hyphenLen;
      while (n < l) {

         /* Test if tabbed. */
         if (j==0) {
            if (items[i][n] == '\t') {
               t  = 1;
               w = osd_w - osd_tabLen;
            }
            else {
               t = 0;
               w = osd_w - osd_hyphenLen;
            }
         }

         /* Get text size. */
         s = gl_printWidthForText( &gl_smallFont, &items[i][n], w );

         if ((j==0) && (t==1))
            w -= osd_hyphenLen;

         if (j+1 > m) {
            if (m==0)
               m = 32;
            else
               m *= 2;
            osd->items[i].chunks = realloc( osd->items[i].chunks, m * sizeof(char*));
         }

         /* Copy text over. */
         if (j==0) {
            if (t==1) {
               osd->items[i].chunks[j] = malloc(s+4);
               nsnprintf( osd->items[i].chunks[j], s+4, "   %s", &items[i][n+1] );
            }
            else {
               osd->items[i].chunks[j] = malloc(s+3);
               nsnprintf( osd->items[i].chunks[j], s+3, "- %s", &items[i][n] );
            }
         }
         else if (t==1) {
            osd->items[i].chunks[j] = malloc(s+4);
            nsnprintf( osd->items[i].chunks[j], s+4, "   %s", &items[i][n] );
         }
         else {
            osd->items[i].chunks[j] = malloc(s+1);
            nsnprintf( osd->items[i].chunks[j], s+1, "%s", &items[i][n] );
         }

         /* Go to next line. */
         n += s + 1;
         j++;
      }
      osd->items[i].nchunks = j;
   }

   /* Append to linked list. */
   if (osd_list == NULL)
      osd_list = osd;
   else {
      for (ll = osd_list; ll->next != NULL; ll = ll->next) {
         if (ll->next->priority > priority) {
            osd->next = ll->next;
            break;
         }
      }
      ll->next = osd;
   }

   /* Recalculate dimensions. */
   osd_calcDimensions();

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
      free(osd->items[i].chunks);
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

         /* Recalculate dimensions. */
         osd_calcDimensions();

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
 *    @return 0 on success.
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
   /* Set offsets. */
   osd_x = x;
   osd_y = y;
   osd_w = w;
   osd_lines = h / (gl_smallFont.h+5);
   osd_h = h - h % (gl_smallFont.h+5);

   /* Calculate some font things. */
   osd_tabLen = gl_printWidthRaw( &gl_smallFont, "   " );
   osd_hyphenLen = gl_printWidthRaw( &gl_smallFont, "- " );

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
   int i, j, l;
   int w, x;
   const glColour *c;

   /* Nothing to render. */
   if (osd_list == NULL)
      return;

   /* Background. */
   gl_renderRect( osd_x-5., osd_y-(osd_rh+5.), osd_w+10., osd_rh+10, &cBlackHilight );

   /* Render each thingy. */
   p = osd_y-gl_smallFont.h;
   l = 0;
   for (ll = osd_list; ll != NULL; ll = ll->next) {
      x = osd_x;
      w = osd_w;

      /* Print title. */
      gl_printMaxRaw( &gl_smallFont, w, x, p, NULL, ll->title );
      p -= gl_smallFont.h + 5.;
      l++;
      if (l >= osd_lines)
         return;

      /* Print items. */
      for (i=0; i<ll->nitems; i++) {
         x = osd_x;
         w = osd_w;
         c = (ll->active == i) ? &cConsole : NULL;
         for (j=0; j<ll->items[i].nchunks; j++) {
            gl_printMaxRaw( &gl_smallFont, w, x, p,
                  c, ll->items[i].chunks[j] );
            if (j==0) {
               w = osd_w - osd_hyphenLen;
               x = osd_x + osd_hyphenLen;
            }
            p -= gl_smallFont.h + 5.;
            l++;
            if (l >= osd_lines)
               return;
         }
      }
   }
}


/**
 * @brief Calculates and sets the length of the OSD.
 */
static void osd_calcDimensions (void)
{
   OSD_t *ll;
   int i, j;
   double len;

   /* Nothing to render. */
   if (osd_list == NULL)
      return;

   /* Render each thingy. */
   len = 0;
   for (ll = osd_list; ll != NULL; ll = ll->next) {

      /* Print title. */
      len += gl_smallFont.h + 5.;

      /* Print items. */
      for (i=0; i<ll->nitems; i++)
         for (j=0; j<ll->items[i].nchunks; j++)
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

   *nitems = o->nitems;
   return o->msg;
}


