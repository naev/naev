/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NEWS_H
#  define NEWS_H


typedef struct news_s {
   char *title; /**< Title of the news article. */
   char *desc; /**< Description of the news article. */
} news_t;


int news_init (void);
void news_exit (void);

const news_t *news_generate( int *ngen, int n );


#endif /* NEWS_H */
