/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file news.c
 *
 * @brief Handles news generation.
 */


#include "news.h"
#include "ntime.h"

#include "naev.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


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
#include "nxml.h"
#include "nxml_lua.h"
#include "space.h"


#define news_max_length       8192   
#define MAX_LINES             1024

/*
 * News stack.
 */					
news_t* news_list             = NULL;  /**< Linked list containing all articles */

static int next_id			   = 1; /**< next number to use as ID */

// static int nlines=0;	/**< number of lines in */

static char buf[8192];

static int len;

/**
 * News line buffer.
 */
static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
static char **news_lines      = NULL; /**< Text per line. */
static glFontRestore *news_restores = NULL; /**< Restorations. */
static int news_nlines        = 0; /**< Number of lines used. */
static int news_mlines        = 0; /**< Lines allocated. */
double textlength = 0.;

/**
 * Save/load
 */
static int largestID=1;

/*
 *	Prototypes
 */
static void news_render( double bx, double by, double w, double h, void *data );
static void news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data );
static int news_parseArticle( xmlNodePtr parent );
int news_saveArticles( xmlTextWriterPtr writer );
int news_loadArticles( xmlNodePtr parent );
// void news_clear(void);


extern ntime_t naev_time;

/**
 *	@brief makes a new article and puts it into the list
 *		@param title	the article title
 *		@param content	the article content
 *		@param faction	the article faction, NULL for generic
 *		@param currentdate date to put
 *	@return pointer to new article
 *
 */
news_t* new_article(char* title, char* content, char* faction, ntime_t date)
{
	printf("\nAdding new article with date %li",date);

	news_t* article_ptr;

		/* make new article */
	news_t* n_article = calloc(sizeof(news_t),1);

	n_article->id=next_id++;


	n_article->faction = n_article->faction ? NULL : strdup(faction);
		//@@@ ### will not warn if out of memory

		/* allocate it */
	if ( !( (n_article->title = strdup(title)) && 
			(n_article->desc = strdup(content))) ){
		ERR("Out of Memory.");
		return NULL;
	}

   n_article->date=date;

      /* If it belongs first*/
   if (news_list->date <= date){
      n_article->next=news_list;
      news_list=n_article;
   }
      /*article_ptr is the one BEFORE the one we want*/
   else{

      article_ptr=news_list;

      while ( article_ptr->next!=NULL && article_ptr->next->date > date){
         article_ptr = article_ptr->next;
      }

      n_article->next=article_ptr->next;

      article_ptr->next=n_article;
   }

	return n_article;
}


/**
 *	@brief frees an article in the news list
 *		@param id the id of the article to remove
 */
int free_article(int id)
{
	printf("\nRemoving an article");
	news_t* article_ptr = news_list;
	news_t* article_to_rm;

		/* if the first article is the one we're looking for */
	if (news_list->id == id){
		article_to_rm = news_list;
		news_list = news_list->next;
		if (news_list->next == NULL){	//###shouldn't happen, but just in case
			WARN("\nLast article, do not remove");
			return -1;
		}
	}
	else
	{
			/* get the article before the one we're looking for */
		while ( article_ptr->next!=NULL && article_ptr->next->id!=id )
			article_ptr=article_ptr->next;

		if ( article_ptr->next == NULL  ){
			WARN("\nArticle to remove not found");
			return -1;
		}

		article_to_rm = article_ptr->next;
		article_ptr->next = article_to_rm->next;
	}

	free(article_to_rm->title);
	free(article_to_rm->desc);
	free(article_to_rm->faction);
	free(article_to_rm);

	return 0;
}


/**
 *	@brief Initiate news linked list with a stack
 */
int news_init (void)
{

	printf("\nInitiating the news");
		/* init news list with dummy article */
   if (news_list!=NULL){
      news_exit();
   }

	news_list = calloc(sizeof(news_t),1);

   news_list->date=0;

   printf("\nFinished initiating");

	return 0;
}


/**
 *	Kills the old news thread
 */
void news_exit (void)
{

	printf("\nKilling the news");

   if (news_list==NULL)
      return;

	news_t* article_ptr = news_list;
	news_t* temp;

	while (article_ptr->next!=NULL){

		temp=article_ptr;
		article_ptr=article_ptr->next; 

		free(temp);
	}

   int i=0;
   if (news_nlines != 0) {
      for (i=0; i<news_nlines; i++)
         free(news_lines[i]);
      news_nlines = 0;
   }

   free(news_lines);
   free(news_restores);
   news_lines  = NULL;
   news_mlines = 0;

   news_list=NULL;

}



/**
 * @brief gets the article with ID, else NULL
 */
news_t* news_get(int id)
{
   news_t* article_ptr=news_list;

   while (article_ptr!=NULL && article_ptr->id!=id)
      article_ptr=article_ptr->next;

   if (article_ptr->id!=id)
      return NULL;

   return article_ptr;
}


/**
 * @brief Generates news from newslist from specific faction AND generic news
 *
 *    @param the faction of wanted news
 * @return 0 on success
 */
int *generate_news( char* faction )
{
	printf("\nGenerating news, faction is %s",faction);

   // static char buf[news_max_length]; /**< where the news text is held */
	news_t* article_ptr = news_list;
	int p=0;

      /* Put all acceptable news into buf */
	do{
         /* If we've reached the end of the list */
      if (article_ptr->faction==NULL){
         break;
      }

         /* if article is okay */
		if ( !strcmp(article_ptr->faction,"Generic") || !strcmp(article_ptr->faction,faction) )
		{
			if (article_ptr->date && article_ptr->date<40000000000000){
      		p += nsnprintf( buf+p, news_max_length-p,
           		" %s \n"
           		"%s: %s\n\n"
           		, article_ptr->title, ntime_pretty(article_ptr->date,1), article_ptr->desc );
      	}else{
      		p+=nsnprintf( buf+p, news_max_length-p,
               " %s \n"
               "%s\n\n"
           		, article_ptr->title, article_ptr->desc );
      	}
		}

	}while( (article_ptr = article_ptr->next) != NULL );

	if (p==0)
		nsnprintf(buf, news_max_length, "\n\nSorry, no news today\n\n\n");

   len=p;


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
	printf("\nStarting widget");

   /* Sane defaults. */
   news_pos    = h/3;
   news_tick   = SDL_GetTicks();

   int i=0;
   news_t* article_ptr=news_list;
   do{
      printf("\n%d: - %s - :%li: %s",i,article_ptr->title,article_ptr->date, article_ptr->desc);
      i++;
   } while ((article_ptr=article_ptr->next)!=NULL);




      /* Now load up the text. */
   // int len=p;
   int p = 0;
   news_nlines = 0;

   printf("\n\n\nLoading lines...");
   while (p < len) {

      printf("\nnew line");
      /* Get the length. */
      i = gl_printWidthForText( NULL, &buf[p], w-40 );

      printf("\n\tline length will be %d",i);

      /* Copy the line. */
      if (news_nlines+1 > news_mlines) {
         printf("\n\tReallocating space");
         if (news_mlines == 0)
            news_mlines = 256;
         else
            news_mlines *= 2;
         news_lines    = realloc( news_lines, sizeof(char*) * news_mlines );
         news_restores = realloc( news_restores, sizeof(glFontRestore) * news_mlines );
      }
      printf("\n\tMallocing the space...");
      news_lines[ news_nlines ]    = malloc( i + 1 );
      printf("\n\tPutting in the text");
      strncpy( news_lines[news_nlines], buf+p, i );
      printf(" ... ");
      news_lines[ news_nlines ][i] = '\0';
      printf("\n\tdoin... stuff?");
      if (news_nlines==0)
         gl_printRestoreInit( &news_restores[ news_nlines ] );
      else  {
         memcpy( &news_restores[ news_nlines ], &news_restores[ news_nlines-1 ], sizeof(glFontRestore) );
         gl_printStore( &news_restores[ news_nlines ], news_lines[ news_nlines-1 ] );
      }
 
      p += i + 1; /* Move pointer. */
      news_nlines++; /* New line. */
   }
   printf("\nDone loading lines...\n\n");
   /* </load text> */

   for (i=0;i<news_nlines;i++)
   {
      printf("\n%s",news_lines[i]);
   }


      /* Create the custom widget. */
   window_addCust( wid, x, y, w, h,
         "cstNews", 1, news_render, news_mouse, NULL );
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
static void news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;

   switch (event->type) {
      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return;

         if (event->button.button == SDL_BUTTON_WHEELUP)
            news_pos -= h/3.;
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            news_pos += h/3.;
         else if (!news_drag)
            news_drag = 1;
         break;

      case SDL_MOUSEBUTTONUP:
         if (news_drag)
            news_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (news_drag)
            news_pos -= event->motion.yrel;
         break;
   }
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
   s = MAX(0,p-m);
   p = MIN(p+1,news_nlines-1);

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
 * @saves all current articles
*    @return 0 on success
 */

int news_saveArticles( xmlTextWriterPtr writer )
{
   printf("\nSaving articles\n");

   news_t* article_ptr = news_list;

   xmlw_startElem(writer,"news");

   do {

      if ( article_ptr->title!=NULL && article_ptr->desc!=NULL && article_ptr->faction!=NULL )
      {

         xmlw_startElem(writer,"article");
   
         xmlw_attr(writer,"title","%s",article_ptr->title);
         xmlw_attr(writer,"desc","%s",article_ptr->desc);
         xmlw_attr(writer,"faction","%s",article_ptr->faction);
         xmlw_attr(writer,"date","%li",article_ptr->date);
         xmlw_attr(writer,"id","%i",article_ptr->id);
   
         xmlw_endElem(writer); /* "article" */
      }

   } while ((article_ptr=article_ptr->next)!=NULL);

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
   printf("Loading articles");

   xmlNodePtr node;

   largestID=1;

   news_exit();
   news_init();

      /* Get and parse news/articles */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"news"))
         if (news_parseArticle( node ) < 0) return -1;
   } while (xml_nextNode(node));

   next_id=largestID;

   return 0;
}



/**
 * @brief Parses an individual article
 *
 *    @param parent Parent node to parse.
 *    @return 0 on success.
 */
static int news_parseArticle( xmlNodePtr parent )
{
   char* title;
   char* desc;
   char* faction;
   char* buff;
   ntime_t date;
   xmlNodePtr node;

   node = parent->xmlChildrenNode;
   do {

      // printf("\tParsing article");

      if (!xml_isNode(node,"article"))
         continue;

      xmlr_attr(node,"title",title);
      if (title==NULL) {
         WARN("Event has missing 'name' attribute, skipping.");
         continue;
      }
      xmlr_attr(node,"desc",desc);
      if (desc==NULL) {
         free(title);
         WARN("Event is missing content, skipping");
         continue;
      }
      xmlr_attr(node,"faction",faction);
      if (faction==NULL) {
         free(title); free(desc);
         WARN("Event has missing faction attribute, skipping.");
         continue;
      }
      xmlr_attr(node,"date",buff);
      if (faction==NULL) {
         free(title); free(desc); free(faction);
         WARN("Event has missing date attribute, skipping.");
         continue;
      }
      date = atol(buff);
      free(buff);
      xmlr_attr(node,"id",buff);
      if (faction==NULL) {
         free(title); free(desc); free(faction);
         WARN("Event has missing date attribute, skipping.");
         continue;
      }
      next_id = atoi(buff);
      largestID=MAX(largestID,next_id+1);
      free(buff);

      printf("\t\tMaking new article");

         /* make the article*/
      new_article(title,desc,faction,date);

      free(title);
      free(desc);
      free(faction);

   } while (xml_nextNode(node));

   return 0;
}


/*
 * Temporary header initializer
 */
int news_addHeaders(void)
{
   printf("\nadding headers");

   if (news_list==NULL)
      news_init();

   new_article("===Dvaered news===\n","Welcome to the Dvaered News Centre. All that happens. In simple words. So you can understand\n","Dvaered",40000000000000);

   new_article("===Welcome to the Empire News Centre===\n","Fresh news from around the Empire\n","Empire",40000000000000);

   new_article("===Goddard news===\n","Welcome to Goddard News Centre. We bring you the news from around the Empire\n","Goddard",40000000000000);

   new_article("===Pirate News. News that matters===\n","Laughing at the Emperor\n","Pirate",40000000000000);

   new_article("===Sirian news===\n","Sirius News Reel. Words of the Sirichana for all\n","Sirius",40000000000000);

   // new_article(,40000000000000);  //independent?

   return 0;

}