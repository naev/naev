/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
int *generate_news( char* faction );
void news_widget( unsigned int wid, int x, int y, int w, int h );

/*
 * News interactions
 */
news_t* new_article( char* title, char* content, char* faction, ntime_t date,
      ntime_t date_to_rm );
int free_article(int id);
news_t* news_get(int id);




#endif /* NEWS_H */
