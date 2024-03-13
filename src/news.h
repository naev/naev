/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <stdint.h>

#include "nlua.h"
#include "ntime.h"

#define NEWS_FOREVER INT64_MAX /* For news that should never get removed. */

/**
 * @brief Represents a news article.
 */
typedef struct news_s {
   int id;       /**< ID of the news. */
   int priority; /**< Priority of the news. 5 is default, lower is more
                    important (appears higher). */

   char *title;   /**< Title of the news article. */
   char *desc;    /**< Content of the news article. */
   char *faction; /**< Faction of the news article */
   char *tag;     /**< tag to identify article, added after creation */

   ntime_t date;       /**< Date added ascribed to the article, NULL if none */
   ntime_t date_to_rm; /**< Date after which the article will be removed */
} news_t;

/*
 * Create/destroy
 */
int  news_init( void );
void news_exit( void );

/*
 * Display.
 */
int *generate_news( int faction );
void news_widget( unsigned int wid, int x, int y, int w, int h );

/*
 * News interactions
 */
int     news_add( const char *title, const char *content, const char *faction,
                  const char *tag, ntime_t date, ntime_t date_to_rm, int priority );
news_t *news_get( int id );
void    news_rm( int id );
