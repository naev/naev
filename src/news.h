/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NEWS_H
#  define NEWS_H

#include "nlua.h"
#include "ntime.h"

/**
 * @brief Represents a news article.
 */
typedef struct news_s {

   int id;

   char *title; /**< Title of the news article. */
   char *desc; /**< Content of the news article. */
   char *faction; /**< Faction of the news article */
   char *tag; /**< tag to identify article, added after creation */

   ntime_t date; /**< Date added ascribed to the article, NULL if none */
   ntime_t date_to_rm; /**< Date after which the article will be removed */

   struct news_s* next; /**< pointer to next article in the list */
} news_t;


/*
 * Create/destroy
 */
int news_init (void);
void news_exit (void);

/*
 * Display.
 */
int *generate_news( const char* faction );
void news_widget( unsigned int wid, int x, int y, int w, int h );

/*
 * News interactions
 */
news_t* new_article( char* title, char* content, const char* faction, ntime_t date,
      ntime_t date_to_rm );
int free_article(int id);
news_t* news_get(int id);




#endif /* NEWS_H */
