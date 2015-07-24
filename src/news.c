/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file news.c
 *
 * @brief Handles news generation.
 */


#include "news.h"

#include "naev.h"

#include <stdint.h>
#include <stdlib.h>


#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_misn.h"
#include "nlua_faction.h"
#include "nlua_diff.h"
#include "nlua_var.h"
#include "ndata.h"
#include "toolkit.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "nxml_lua.h"
#include "space.h"


#define NEWS_MAX_LENGTH       8192

/*
 * News stack.
 */
news_t* news_list             = NULL;  /**< Linked list containing all articles */

static int next_id            = 1; /**< next number to use as ID */

/**
 * News line buffer.
 */
static char buf[NEWS_MAX_LENGTH];

static int len;

static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
static char **news_lines      = NULL; /**< Text per line. */
static glFontRestore *news_restores = NULL; /**< Restorations. */
static int news_nlines        = 0; /**< Number of lines used. */
static int news_mlines        = 0; /**< Lines allocated. */
static double textlength      = 0.;

/**
 * Save/load
 */
static int largestID;

/*
 * Prototypes
 */
static void news_render( double bx, double by, double w, double h, void *data );
static int news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data );
static int news_parseArticle( xmlNodePtr parent );
int news_saveArticles( xmlTextWriterPtr writer ); /* externed in save.c */
int news_loadArticles( xmlNodePtr parent ); /* externed in load.c */
static char* make_clean( char* unclean );
static char* get_fromclean( char *clean );
static void clear_newslines (void);

/**
 * @brief makes a new article and puts it into the list
 *    @param title   the article title
 *    @param content the article content
 *    @param faction the article faction
 *    @param date date to put
 *    @param date_to_rm date to remove the article
 * @return pointer to new article
 */
news_t* new_article(char* title, char* content, char* faction, ntime_t date,
    ntime_t date_to_rm)
{
   news_t *article_ptr, *n_article;

   /* make new article */
   n_article = calloc(sizeof(news_t), 1);

   n_article->id = next_id++;

   /* allocate it */
   if ( !( (n_article->title = strdup(title)) &&
         (n_article->desc = strdup(content)) &&
         (n_article->faction = strdup(faction)))) {
      ERR("Out of Memory.");
      return NULL;
   }

   n_article->date = date;
   n_article->date_to_rm = date_to_rm;

   /* If it belongs first*/
   if (news_list->date <= date) {
      n_article->next = news_list;
      news_list = n_article;
   }
   /* article_ptr is the one BEFORE the one we want*/
   else {
      article_ptr = news_list;

      while ((article_ptr->next != NULL) && (article_ptr->next->date > date))
         article_ptr = article_ptr->next;

      n_article->next = article_ptr->next;

      article_ptr->next = n_article;
   }

   return n_article;
}


/**
 * @brief frees an article in the news list
 *    @param id the id of the article to remove
 */
int free_article(int id)
{
   news_t *article_ptr, *article_to_rm;

   article_ptr = news_list;

   /* if the first article is the one we're looking for */
   if (news_list->id == id) {
      article_to_rm = news_list;
      news_list = news_list->next;

      if (article_to_rm->next == NULL) {
         WARN("\nLast article, do not remove");
         return -1;
      }
   }
   else {
      /* get the article before the one we're looking for */
      while ((article_ptr->next != NULL) && (article_ptr->next->id != id))
         article_ptr = article_ptr->next;

      if (article_ptr->next == NULL) {
         WARN("\nArticle to remove not found");
         return -1;
      }

      article_to_rm = article_ptr->next;
      article_ptr->next = article_to_rm->next;
   }

   free(article_to_rm->title);
   free(article_to_rm->desc);
   free(article_to_rm->faction);
   free(article_to_rm->tag);
   free(article_to_rm);

   return 0;
}


/**
 * @brief Initiate news linked list with a stack
 */
int news_init (void)
{
   /* init news list with dummy article */
   if (news_list != NULL)
      news_exit();

   news_list = calloc(sizeof(news_t), 1);

   return 0;
}


/**
 * @brief Kills the old news thread
 */
void news_exit (void)
{
   int i;
   news_t *article_ptr, *temp;

   if (news_list == NULL)
      return;

   article_ptr = news_list;

   while (article_ptr != NULL) {
      temp = article_ptr;
      article_ptr = article_ptr->next;

      free(temp->faction);
      free(temp->title);
      free(temp->desc);
      free(temp->tag);

      free(temp);
   }

   if (news_nlines != 0) {
      for (i=0; i<news_nlines; i++)
         free(news_lines[i]);
      news_nlines = 0;
   }

   free(news_lines);
   free(news_restores);
   news_lines  = NULL;
   news_restores = NULL;
   news_nlines = 0;
   news_mlines = 0;
   textlength  = 0;

   news_list = NULL;

}



/**
 * @brief gets the article with id ID, else NULL
 */
news_t* news_get(int id)
{
   news_t *article_ptr;

   article_ptr = news_list;

   while ((article_ptr != NULL) && (article_ptr->id != id))
      article_ptr = article_ptr->next;

   if (article_ptr == NULL)
      return NULL;

   return article_ptr;
}


/**
 * @brief Generates news from newslist from specific faction AND Generic news
 *
 *    @param the faction of wanted news
 * @return 0 on success
 */
int *generate_news( char* faction )
{
   news_t *temp, *article_ptr;
   int p;

   p = 0;
   article_ptr = news_list;

   /* Put all acceptable news into buf */
   do {
      /* If we've reached the end of the list */
      if (article_ptr->faction == NULL)
         break;

      /* if the article is due for removal */
      if (article_ptr->date_to_rm <= ntime_get()) {
         temp = article_ptr->next;
         free_article(article_ptr->id);
         article_ptr = temp;
         continue;
      }

      /* if article is okay */
      if (!strcmp(article_ptr->faction, "Generic") || !strcmp(article_ptr->faction, faction)) {
         if (article_ptr->date && article_ptr->date<40000000000000) {
            p += nsnprintf( buf+p, NEWS_MAX_LENGTH-p,
               " %s \n"
               "%s: %s\e0\n\n"
               , article_ptr->title, ntime_pretty(article_ptr->date, 1), article_ptr->desc );
         }
         else {
            p += nsnprintf( buf+p, NEWS_MAX_LENGTH-p,
               " %s \n"
               "%s\e0\n\n"
               , article_ptr->title, article_ptr->desc );
         }
      }

      article_ptr = article_ptr->next;

   } while(article_ptr != NULL);

   if (p == 0)
      nsnprintf(buf, NEWS_MAX_LENGTH, "\n\nSorry, no news today\n\n\n");

   len = p;

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
   int i, p;

   /* Sane defaults. */
   news_pos    = h/3;
   news_tick   = SDL_GetTicks();

   if (news_nlines>0)
      clear_newslines();


   /* Now load up the text. */
   i = 0;
   p = 0;
   news_nlines = 0;

   while (p < len) {

      /* Get the length. */
      i = gl_printWidthForText( NULL, &buf[p], w-40 );

      /* Copy the line. */
      if (news_nlines+1 > news_mlines) {
         if (news_mlines == 0)
            news_mlines = 256;
         else
            news_mlines *= 2;
         news_lines    = realloc( news_lines, sizeof(char*) * news_mlines );
         news_restores = realloc( news_restores, sizeof(glFontRestore) * news_mlines );
      }
      news_lines[ news_nlines ]    = malloc( i + 1 );
      strncpy( news_lines[news_nlines], buf+p, i );
      news_lines[ news_nlines ][i] = '\0';
      if (news_nlines == 0)
         gl_printRestoreInit( &news_restores[ news_nlines ] );
      else {
         memcpy( &news_restores[ news_nlines ], &news_restores[ news_nlines-1 ], sizeof(glFontRestore) );
         gl_printStore( &news_restores[ news_nlines ], news_lines[ news_nlines-1 ] );
      }

      p += i + 1;    /* Move pointer. */
      news_nlines++; /* New line. */
   }
   /* </load text> */

   /* Create the custom widget. */
   window_addCust( wid, x, y, w, h, "cstNews", 1, news_render, news_mouse, NULL );
}



/* clears newslines for bar text, for when taking off */
void clear_newslines (void)
{
   int i;
   for (i=0; i<news_nlines; i++)
      free(news_lines[i]);

   news_nlines = 0;
}


/**
 * @brief wid Window receiving the mouse events.
 *
 *    @param event Mouse event being received.
 *    @param mx X position of the mouse.
 *    @param my Y position of the mouse.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static int news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;

   switch (event->type) {
#if SDL_VERSION_ATLEAST(2,0,0)
      case SDL_MOUSEWHEEL:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;

         if (event->wheel.y > 0)
            news_pos -= h/3.;
         else
            news_pos += h/3.;
         return 1;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;

#if SDL_VERSION_ATLEAST(2,0,0)
         if (!news_drag)
#else /* SDL_VERSION_ATLEAST(2,0,0) */
         if (event->button.button == SDL_BUTTON_WHEELUP)
            news_pos -= h/3.;
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            news_pos += h/3.;
         else if (!news_drag)
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
            news_drag = 1;
         return 1;

      case SDL_MOUSEBUTTONUP:
         if (news_drag)
            news_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (news_drag)
            news_pos -= event->motion.yrel;
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
 */
static void news_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int i, s, m, p;
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
   if (p > news_nlines + m + 1) {
      news_pos = 0.;
      return;
   }

   /* Get positions to make sure inbound. */
   s = MAX(0, p - m);
   p = MIN(p + 1, news_nlines - 1);

   /* Get start position. */
   y = news_pos - s * (news_font->h+5.);

   /* Draw loop. */
   for (i=s; i<p; i++) {

      gl_printRestore( &news_restores[i] );
      gl_printMidRaw( news_font, w-40.,
            bx+10, by+y, &cConsole, news_lines[i] );

      /* Increment line and position. */
      y -= news_font->h + 5.;
   }

}

/*
 * @brief replace ascii character 27 with the string "\027"
 */
static char* make_clean( char* unclean )
{
   int i, j;
   char *new;

   new = malloc( 4*strlen(unclean)+1 );

   for (i=0, j=0; unclean[i] != 0; i++, j++) {
      if (unclean[i] == 27) {
         new[j++] = '\\';
         j += sprintf( &new[j], "%.3d", unclean[i] )-1;
      }
      else
         new[j] = unclean[i];
   }

   new[j] = 0;

   return new;

}


/*
 * @brief replace any \027 strings with the ascii character
 */
static char* get_fromclean( char *clean)
{
   int line_max, i, j;
   char *new, *unclean;

   line_max = 1024;
   new = malloc(line_max);

   for (i=0, j=0; clean[i] != 0; i++, j++) {
      if  (j>=line_max-3) {
         line_max = line_max*2;
         new = realloc(new, line_max);
      }
      if (clean[i] == '\\' && clean[i+1] == '0' && clean[i+2] == '2' &&
            clean[i+3] == '7') {
         new[j] = 27;
         i += 3;
      }
      else
         new[j] = clean[i];
   }
   new[j] = 0;

   unclean = strdup(new);

   free(new);

   return unclean;
}


/*
 * @brief saves all current articles
 *    @return 0 on success
 */
int news_saveArticles( xmlTextWriterPtr writer )
{
   news_t *article_ptr;
   char *ntitle, *ndesc;

   article_ptr = news_list;

   xmlw_startElem(writer, "news");
   do {

      if ( article_ptr->title != NULL && article_ptr->desc!=NULL &&
            article_ptr->faction != NULL ) {
         xmlw_startElem(writer, "article");

         ntitle = make_clean( article_ptr->title );
         ndesc  = make_clean( article_ptr->desc );

         xmlw_attr(writer, "title", "%s", ntitle);
         xmlw_attr(writer, "desc", "%s", ndesc);
         xmlw_attr(writer, "faction", "%s", article_ptr->faction);
         xmlw_attr(writer, "date", "%"PRIi64, article_ptr->date);
         xmlw_attr(writer, "date_to_rm", "%"PRIi64, article_ptr->date_to_rm);
         xmlw_attr(writer, "id", "%i", article_ptr->id);

         if (article_ptr->tag != NULL)
            xmlw_attr(writer, "tag", "%s", article_ptr->tag);

         free(ntitle);
         free(ndesc);

         xmlw_endElem(writer); /* "article" */
      }

   } while ((article_ptr = article_ptr->next) != NULL);

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

   largestID = 1;

   news_exit();
   news_init();

   /* Get and parse news/articles */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node, "news"))
         if (news_parseArticle( node ) < 0) return -1;
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
   char *ntitle, *ndesc, *title, *desc, *faction, *tag;
   char *buff;
   ntime_t date, date_to_rm;
   xmlNodePtr node;

   news_t *n_article;

   node = parent->xmlChildrenNode;

#define NEWS_READ(elem, s) \
xmlr_attr(node, s, elem); \
if (elem == NULL) { WARN("Event is missing '"s"', skipping."); goto cleanup; }

   do {

      if (!xml_isNode(node, "article"))
         continue;

      /* Reset parameters. */
      ntitle  = NULL;
      ndesc   = NULL;
      title   = NULL;
      desc    = NULL;
      faction = NULL;
      tag     = NULL;

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

      largestID = MAX(largestID, next_id + 1);

      ntitle = get_fromclean(title);
      ndesc  = get_fromclean(desc);

      /* Optional. */
      xmlr_attr(node, "tag", tag);

      /* make the article*/
      n_article = new_article(ntitle, ndesc, faction, date, date_to_rm);
      if (tag != NULL)
         n_article->tag = strdup(tag);

cleanup:
      free(ntitle);
      free(ndesc);
      free(title);
      free(desc);
      free(faction);
      free(tag);
   } while (xml_nextNode(node));
#undef NEWS_READ

   return 0;
}
