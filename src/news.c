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


#include "log.h"		//not sure which are necessary
#include "nlua.h"
#include "nluadef.h"
#include "nlua_misn.h"
#include "nlua_faction.h"
#include "nlua_diff.h"
#include "nlua_var.h"
#include "ndata.h"
#include "toolkit.h"
#include "nstring.h"


#define news_max_length       8192   
#define MAX_LINES             1024

/*
 * News stack.
 */					
news_t* news_list             = NULL;  /**< Linked list containing all articles */

static int next_id			  = 1; /**< next number to use as ID */

static char news_text[8192]; /**< where the news text is held */
static char* news_lines[1024]; /**< temporary line storage */
static int nlines=0;	/**< number of lines in */
 	//hack, remove, replace w/news_max_length

/**
 * News line buffer.
 */
static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
// static char **news_lines      = NULL; /**< Text per line. */
// static glFontRestore *news_restores = NULL; /**< Restorations. */
static int news_nlines        = 0; /**< Number of lines used. */
// static int news_mlines        = 0; /**< Lines allocated. */

/*
 *	Prototypes
 */
static void news_render( double bx, double by, double w, double h, void *data );
static void news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data );



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
	printf("\nAdding new article");

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

		/* set the date and put it into the list */
	if (date){
		n_article->date = date;
		n_article->next = news_list;
		news_list = n_article;
	}
		/* if !date, put just after dated articles */
	else{
		n_article->date=0;
		article_ptr = news_list;

		while ( article_ptr->next!=NULL && article_ptr->next->date!=0 )
			article_ptr=article_ptr->next;

		n_article->next = article_ptr->next ? article_ptr->next : NULL;
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
 *	Initiate news linked list with a stack
 */
int news_init (void)
{
	printf("\nInitiating the news");
		/* init news list with dummy article */
	news_list = calloc(sizeof(news_t),1);

	printf("\nAdding new articles");

	new_article("A1","This is article 1, an article that is way too long for it's own goddam good","Generic",300000000);

	return 0;
}


/**
 *	Kills the old news thread
 */
void news_exit (void)
{

	printf("\nKilling the news");

	news_t* article_ptr = news_list;
	news_t* temp;

	while (article_ptr->next!=NULL){

		temp=article_ptr;
		article_ptr=article_ptr->next; 

		free(temp);
	}

}



/**
 * @brief Generates news from newslist from specific faction AND generic news
 *
 *    @param the faction of wanted news
 * @return 0 on success
 */
int *generate_news( char* faction )
{
	printf("\nGenerating news for faction %s\n",faction);

	news_t* article_ptr = news_list;
	int l, i, p=0;

      /* Put all acceptable news into news_text */
	do{

      printf("\nChecking article title %s, faction %s",article_ptr->title,article_ptr->faction);

         /* If we've reached the end of the list */
      if (article_ptr->faction==NULL)
         break;

         /* if article is okay */
		if ( !strcmp(article_ptr->faction,"Generic") || !strcmp(article_ptr->faction,faction) )
		{

			if (article_ptr->date){
      			p += nsnprintf( news_text, news_max_length-p,
            		" - %s - \n"
            		"%s: %s\n\n"
            		, article_ptr->title, ntime_pretty(article_ptr->date,4), article_ptr->desc );
      		}
      		else{
      			p+=nsnprintf( news_text+p, news_max_length-p,
                  " - %s - \n"
                  "%s\n\n"
            		, article_ptr->title, article_ptr->desc );

      		}
		}

	}while( (article_ptr = article_ptr->next) != NULL );

	if (p==0)
		nsnprintf(news_text, news_max_length, "\n\nSorry, no news today\n\n\n");

	// printf("\nText reads \"\"\"%s\"\"\"",news_text);

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

   /* Get current news text */
   generate_news("");	//faction is null for now

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

   /* Draw the text */
   // gl_printRestore( &news_restores[i] );	//???

      /* print the new lines, with temporary breakable line breaking */
   for (i=nlines-1; i>=0; i--){

      gl_printMidRaw( news_font, w-40., bx+10, by+y, &cConsole, news_lines[i] );
      y+=15;

   }



}

