/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file news.c
 *
 * @brief Handles news generation.
 */
/** @cond */
#include <stdint.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "news.h"

#include "array.h"
#include "faction.h"
#include "log.h"
#include "nlua.h"
#include "nlua_diff.h"
#include "nlua_faction.h"
#include "nlua_misn.h"
#include "nlua_var.h"
#include "nluadef.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "nxml_lua.h"
#include "space.h"
#include "toolkit.h"

#define NEWS_MAX_LENGTH       8192  /**< Maximum length to print. */

/*
 * News stack.
 */
news_t *news_list    = NULL;  /**< Linked list containing all articles */
static int next_id   = 0; /**< next number to use as ID */

/**
 * News line buffer.
 */
static char buf[NEWS_MAX_LENGTH];
static int len;

static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
static char **news_lines      = NULL; /**< Array (array.h) of each line's text. */
static glFontRestore *news_restores = NULL; /**< Array (array.h) of restorations. */
static double textlength      = 0.;

/**
 * Save/load
 */
static int largestID;

/*
 * Prototypes
 */
static void news_render( double bx, double by, double w, double h, void *data );
static void news_focusLose( unsigned int wid, const char* wgtname );
static int news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, double rx, double ry, void *data );
static int news_parseArticle( xmlNodePtr parent );
int news_saveArticles( xmlTextWriterPtr writer ); /* externed in save.c */
int news_loadArticles( xmlNodePtr parent ); /* externed in load.c */
static void clear_newslines (void);

static int news_cmp( const void *p1, const void *p2 )
{
   const news_t *n1, *n2;
   int diff;
   n1 = (const news_t*) p1;
   n2 = (const news_t*) p2;
   diff = n1->priority - n2->priority;
   if (diff != 0)
      return diff;
   if (n1->date < n2->date)
      return +1;
   else if (n1->date > n2->date)
      return -1;
   return n1->id - n2->id;
}

/**
 * @brief makes a new article and puts it into the list
 *    @param title   the article title
 *    @param content the article content
 *    @param faction the article faction
 *    @param tag Tag to set.
 *    @param date date to put
 *    @param date_to_rm date to remove the article
 *    @param priority Priority to use.
 * @return ID of newly added news.
 */
int news_add( const char *title, const char *content,
      const char *faction, const char *tag,
      ntime_t date, ntime_t date_to_rm, int priority )
{
   news_t *n;
   int id = ++next_id;

   if (news_list==NULL)
      news_list = array_create( news_t );
   n = &array_grow( &news_list );
   memset( n, 0, sizeof(news_t) );
   n->id      = id;
   n->title   = strdup( title );
   n->desc    = strdup( content );
   n->faction = strdup( faction );
   if (tag != NULL)
      n->tag = strdup( tag );
   n->date    = date;
   n->date_to_rm = date_to_rm;
   n->priority = priority;

   /* Sort it! */
   qsort( news_list, array_size(news_list), sizeof(news_t), news_cmp );

   return id;
}

/**
 * @brief Initiate news linked list with a stack
 */
int news_init (void)
{
   /* init news list with dummy article */
   if (news_list != NULL)
      news_exit();

   news_list = array_create( news_t );
   news_lines = array_create( char* );
   news_restores = array_create( glFontRestore );

   return 0;
}

/**
 * @brief Kills the old news thread
 */
void news_exit (void)
{
   if (news_list == NULL)
      return;

   for (int i=0; i<array_size(news_list); i++) {
      news_t *n = &news_list[i];

      free(n->faction);
      free(n->title);
      free(n->desc);
      free(n->tag);
   }
   array_free(news_list);
   news_list = NULL;

   for (int i=0; i<array_size(news_lines); i++)
      free(news_lines[i]);
   array_free(news_lines);
   array_free(news_restores);
   news_lines  = NULL;
   news_restores = NULL;
   textlength  = 0;

   news_list = NULL;
}

/**
 * @brief gets the article with id ID, else NULL
 */
news_t* news_get( int id )
{
   for (int i=0; i<array_size(news_list); i++) {
      news_t *n = &news_list[i];
      if (n->id == id)
         return n;
   }
   return NULL;
}

void news_free( news_t *n )
{
   free( n->title );
   free( n->desc );
   free( n->faction );
   free( n->tag );
}

void news_rm( int id )
{
   news_t *n = news_get( id );
   if (n==NULL)
      return;
   news_free( n );
   array_erase( &news_list, &n[0], &n[1] );
}

/**
 * @brief Generates news from newslist from specific faction AND Generic news
 *
 *    @param faction the faction of wanted news
 * @return 0 on success
 */
int *generate_news( int faction )
{
   const char* fname;
   ntime_t curtime = ntime_get();
   int p = 0;
   const char **tags;

   fname = (faction >= 0) ? faction_name( faction ) : NULL;

   /* First pass to remove old articles. */
   for (int i=array_size(news_list)-1; i>=0; i--) {
      news_t *n = &news_list[i];

      /* if the article is due for removal */
      if (n->date_to_rm <= curtime)
         news_rm( n->id );
   }

   /* Put all acceptable news into buf */
   tags = (faction >= 0) ? faction_tags( faction ) : NULL;
   for (int i=0; i<array_size(news_list); i++) {
      news_t *n = &news_list[i];
      int match_tag = 0;

      /* Check to see if matches tag. */
      if (tags != NULL) {
         for (int j=0; j<array_size(tags); j++) {
            if (strcasecmp( tags[j], n->faction)==0) {
               match_tag = 1;
               break;
            }
         }
      }

      /* if article is okay */
      if (match_tag || ((fname != NULL) && (strcasecmp(n->faction, fname) == 0))) {
         if (n->date && (n->date != 0)) {
            char *article_time = ntime_pretty( n->date, 1 );
            p += scnprintf( buf+p, NEWS_MAX_LENGTH-p,
               " %s \n"
               "%s: %s#0\n\n", n->title, article_time, n->desc );
            free( article_time );
         }
         else {
            p += scnprintf( buf+p, NEWS_MAX_LENGTH-p,
               " %s \n"
               "%s#0\n\n", n->title, n->desc );
         }
      }
   }

   if (p == 0)
      p = scnprintf(buf, NEWS_MAX_LENGTH, "\n\n%s\n\n\n", _("No news is available."));

   len = MIN( p, NEWS_MAX_LENGTH );

   return 0;
}

/**
 * @brief Creates a news widget.
 *
 *    @param wid Window to create news widget on.
 *    @param x X position of the widget to create.
 *    @param y Y position of the widget to create.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
void news_widget( unsigned int wid, int x, int y, int w, int h )
{
   glPrintLineIterator iter;

   /* Safe defaults. */
   news_pos    = h/3;
   news_tick   = SDL_GetTicks();

   clear_newslines();

   /* Now load up the text. */
   gl_printLineIteratorInit( &iter, NULL, buf, w-40 );

   while (gl_printLineIteratorNext( &iter )) {
      /* Copy the line. */
      array_push_back( &news_lines, strndup( &buf[iter.l_begin], iter.l_end - iter.l_begin ) );
      if (array_size( news_restores ) == 0)
         gl_printRestoreInit( &array_grow( &news_restores ) );
      else {
         glFontRestore restore = array_back( news_restores );
         gl_printStore( &restore, news_lines[ array_size(news_lines)-2 ] );
         array_push_back( &news_restores, restore );
      }
   }

   /* Create the custom widget. */
   window_addCust( wid, x, y, w, h, "cstNews", 1, news_render, news_mouse, NULL, news_focusLose, NULL );
   window_custSetDynamic( wid, "cstNews", 1 );
   window_canFocusWidget( wid, "cstNews", 0 );
}

/* clears newslines for bar text, for when taking off */
void clear_newslines (void)
{
   for (int i=0; i<array_size(news_lines); i++)
      free(news_lines[i]);

   array_resize(&news_lines, 0);
   array_resize(&news_restores, 0);
}

/**
 * @brief Called when it's de-focused.
 */
static void news_focusLose( unsigned int wid, const char* wgtname )
{
   (void) wid;
   (void) wgtname;
   news_drag = 0;
}

/**
 * @brief News widget mouse event handler.
 *
 *    @param wid Window receiving the mouse events.
 *    @param event Mouse event being received.
 *    @param mx X position of the mouse.
 *    @param my Y position of the mouse.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param rx Relative X movement (only valid for motion).
 *    @param ry Relative Y movement (only valid for motion).
 *    @param data Unused.
 */
static int news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, double rx, double ry, void *data )
{
   (void) data;
   (void) rx;

   switch (event->type) {
      case SDL_MOUSEWHEEL:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;

         if (event->wheel.y > 0)
            news_pos -= h/3.;
         else if (event->wheel.y < 0)
            news_pos += h/3.;
         return 1;

      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;
    window_setFocus( wid, "cstNews" );

    news_drag = 1;
         return 1;

      case SDL_MOUSEBUTTONUP:
    news_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (news_drag)
            news_pos -= ry;
         break;
   }

   return 0;
}

/**
 * @brief Renders a news widget.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param data Unused.
 */
static void news_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int s, m, p;
   unsigned int t;
   double y, dt;

   t = SDL_GetTicks();

   /* Calculate offset. */
   if (!news_drag) {
      dt = (double)(t-news_tick)/1000.;
      news_pos += dt * 25.;
   }
   news_tick = t;

   /* Make sure user isn't silly and drags it to negative values. */
   if (news_pos < 0.)
      news_pos = 0.;

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render the text. */
   p = (int)ceil( news_pos / (news_font->h + 5.));
   m = (int)ceil(        h / (news_font->h + 5.));
   if (p > array_size( news_lines ) + m + 1) {
      news_pos = 0.;
      return;
   }

   /* Get positions to make sure inbound. */
   s = MAX(0, p - m);
   p = MIN(p + 1, array_size( news_lines ) - 1);

   /* Get start position. */
   y = news_pos - s * (news_font->h+5.);

   /* Draw loop. */
   for (int i=s; i<p; i++) {
      gl_printRestore( &news_restores[i] );
      gl_printMidRaw( news_font, w-40.,
            bx+10, by+y, &cFontGreen, -1., news_lines[i] );

      /* Increment line and position. */
      y -= news_font->h + 5.;
   }
}

/*
 * @brief saves all current articles
 *    @return 0 on success
 */
int news_saveArticles( xmlTextWriterPtr writer )
{
   xmlw_startElem(writer, "news");

   for (int i=0; i<array_size(news_list); i++) {
      const char *ntitle, *ndesc;
      news_t *n = &news_list[i];

      if (n->title == NULL || n->desc==NULL || n->faction == NULL)
         continue;

      xmlw_startElem(writer, "article");

      ntitle = n->title;
      ndesc  = n->desc;

      xmlw_attr(writer, "title", "%s", ntitle);
      xmlw_attr(writer, "desc", "%s", ndesc);
      xmlw_attr(writer, "faction", "%s", n->faction);
      xmlw_attr(writer, "date", "%"PRIi64, n->date);
      xmlw_attr(writer, "date_to_rm", "%"PRIi64, n->date_to_rm);
      xmlw_attr(writer, "id", "%i", n->id);
      xmlw_attr(writer, "priority", "%i", n->priority);

      if (n->tag != NULL)
         xmlw_attr(writer, "tag", "%s", n->tag);

      xmlw_endElem(writer); /* "article" */
   }

   xmlw_endElem(writer); /* "news" */

   return 0;
}

/**
 * @brief Loads the player's active articles from a save, initilizes news
 *
 *    @param parent Node containing the player's active events.
 *    @return 0 on success.
 */
int news_loadArticles( xmlNodePtr parent )
{
   news_tick = 0;

   xmlNodePtr node;

   largestID = 0;

   news_exit();
   news_init();

   /* Get and parse news/articles */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node, "news"))
         if (news_parseArticle( node ) < 0)
            return -1;
   } while (xml_nextNode(node));

   next_id = largestID;

   return 0;
}

/**
 * @brief Parses articles
 *
 *    @param parent Parent node to parse.
 *    @return 0 on success.
 */
static int news_parseArticle( xmlNodePtr parent )
{
   xmlNodePtr node;

   node = parent->xmlChildrenNode;

#define NEWS_READ(elem, s) \
xmlr_attr_strd(node, s, elem); \
if (elem == NULL) { WARN(_("Event is missing '%s', skipping."), s); goto cleanup; }

   do {
      char *title, *desc, *faction, *tag, *buff;
      int priority;
      ntime_t date, date_to_rm;

      if (!xml_isNode(node, "article"))
         continue;

      /* Reset parameters. */
      title   = NULL;
      desc    = NULL;
      faction = NULL;

      NEWS_READ(title, "title");
      NEWS_READ(desc, "desc");
      NEWS_READ(faction, "faction");

      NEWS_READ(buff, "date");
      date = atoll(buff);
      free(buff);

      NEWS_READ(buff, "date_to_rm");
      date_to_rm = atoll(buff);
      free(buff);

      NEWS_READ(buff, "id");
      next_id = atoi(buff);
      free(buff);

      /* Older versions won't have priority. */
      xmlr_attr_strd( node, "priority", buff );
      priority = (buff==NULL) ? 5 : atoi(buff);
      free(buff);

      /* Read optional tag. */
      tag = NULL;
      xmlr_attr_strd( node, "tag", tag );

      largestID = MAX(largestID, next_id + 1);

      /* make the article*/
      news_add( title, desc, faction, tag, date, date_to_rm, priority );
      free( tag );

cleanup:
      free(title);
      free(desc);
      free(faction);
   } while (xml_nextNode(node));
#undef NEWS_READ

   return 0;
}
