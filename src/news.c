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

   //@@@ replace these arrays with dynamic arrays
static char news_text[8192]; /**< where the news text is held */
static char* news_lines[1024]; /**< temporary line storage */
static int nlines=0;	/**< number of lines in */

/**
 * News line buffer.
 */
static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
// static char **news_lines      = NULL; /**< Text per line. */
// static glFontRestore *news_restores = NULL; /**< Restorations. */
// static int news_nlines        = 0; /**< Number of lines used. */
// static int news_mlines        = 0; /**< Lines allocated. */
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
 * @brief gets the article with ID, else NULL
 */
news_t* get_article(int id)
{
   news_t* article_ptr = news_list;

   while (article_ptr!=NULL && article_ptr->id!=id)
      article_ptr=article_ptr->next;

   if (article_ptr==NULL)
      return NULL;

   return article_ptr;
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

   news_list=NULL;

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

	news_t* article_ptr = news_list;
	int l, i, p=0;

      /* Put all acceptable news into news_text */
	do{
         /* If we've reached the end of the list */
      if (article_ptr->faction==NULL){
         break;
      }

         /* if article is okay */
		if ( !strcmp(article_ptr->faction,"Generic") || !strcmp(article_ptr->faction,faction) )
		{
			if (article_ptr->date && article_ptr->date<40000000000000){
      		p += nsnprintf( news_text+p, news_max_length-p,
           		" - %s - \n"
           		"%s: %s\n\n"
           		, article_ptr->title, ntime_pretty(article_ptr->date,1), article_ptr->desc );
      	}else{
      		p+=nsnprintf( news_text+p, news_max_length-p,
               " - %s - \n"
               "%s\n\n"
           		, article_ptr->title, article_ptr->desc );
      	}
		}

	}while( (article_ptr = article_ptr->next) != NULL );

	if (p==0)
		nsnprintf(news_text, news_max_length, "\n\nSorry, no news today\n\n\n");

      /* transcribe to news_lines*/
   nlines=0;
   l=0;  
   for(i=0; news_text[i]!=0; i++ ){

      if (news_text[i]=='\n'){

         news_lines[nlines] = malloc(i-l+1);   //@@@###important, this must be freed later
         strncpy(news_lines[nlines], &news_text[l], i-l );
         *(news_lines[nlines]+i-l) = 0;
         nlines++;
         l=i;
      }

   }


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

   /* Create the custom widget. */
   window_addCust( wid, x, y, w, h,
         "cstNews", 1, news_render, news_mouse, NULL );

   int i=0;
   news_t* article_ptr=news_list;
   do{
      printf("\n%d: - %s - :%li: %s",i,article_ptr->title,article_ptr->date, article_ptr->desc);
      i++;
   } while ((article_ptr=article_ptr->next)!=NULL);


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
   // int i, s, m, p;
   unsigned int t;
   double y, dt;

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   t = SDL_GetTicks();

   /* Calculate offset. */
   if (!news_drag) {
      dt = (double)(t-news_tick)/1000.;
      news_pos += dt * 25.;
   }
   news_tick = t;

   news_pos=MAX(0,news_pos);


   y = by - textlength + news_pos;


   /* <Draw the text> */
   // gl_printRestore( &news_restores[i] );  //@@@

   int i, i0, i1, pline_i;
   int length;

   int width=w/8;

      //a buffer so run-on lines are played correctly
   char buf[32][64];   //32 lines of 64 chars

   textlength=y;

      /* print the new lines, with temporary breakable line breaking */
   for (i=nlines-1;i>=0;i--){

      i0=0;
      pline_i=0;
      i1=strlen(news_lines[i]);
      length=strlen(news_lines[i]);

         /* Break it down and put into buf */
      while (i0!=i1){

         while (i1-i0>width || ( i1!=length && *(news_lines[i]+i1)!=' ' ) )
            i1--;

         strncpy(buf[pline_i], news_lines[i]+i0, i1-i0 );
         buf[pline_i][i1-i0]=0;
         pline_i++;

         i0=i1;
         i1=length;
      }

         /* print buf */
      for (pline_i--;pline_i>=0;pline_i--){

         gl_printMidRaw( news_font, w-40., bx+10, by+y, &cConsole, buf[pline_i] );
         y+=15.;


      }
   }

   /* </draw text> */


   textlength = y-textlength;

   if (news_pos > textlength+h-by)
      news_pos = 0.;

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
   xmlNodePtr node;

   largestID=1;

   if (news_list!=NULL){
      news_exit();
   }

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
   char* buf;
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
      xmlr_attr(node,"date",buf);
      if (faction==NULL) {
         free(title); free(desc); free(faction);
         WARN("Event has missing date attribute, skipping.");
         continue;
      }
      date = atol(buf);
      free(buf);
      xmlr_attr(node,"id",buf);
      if (faction==NULL) {
         free(title); free(desc); free(faction);
         WARN("Event has missing date attribute, skipping.");
         continue;
      }
      next_id = atoi(buf);
      largestID=MAX(largestID,next_id+1);
      free(buf);

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
   if (news_list==NULL)
      news_init();

   new_article("Dvaered news","Welcome to the Dvaered News Centre. All that happens. In simple words. So you can understand","Dvaered",40000000000000);

   new_article("Welcome to the Empire News Centre","Fresh news from around the Empire","Empire",40000000000000);

   new_article("Goddard news","Welcome to Goddard News Centre. We bring you the news from around the Empire","Goddard",40000000000000);

   new_article("Pirate News. News that matters","Laughing at the Emperor","Pirate",40000000000000);

   new_article("Sirian news","Sirius News Reel. Words of the Sirichana for all","Sirius",40000000000000);

   // new_article(,40000000000000);  //independent?

   return 0;

}