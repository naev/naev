/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NEWS_H
#  define NEWS_H


typedef struct news_s {
   char *title; /**< Title of the news article. */
   char *desc; /**< Description of the news article. */
} news_t;


/*
 * Create/destroy
 */
int news_init (void);
void news_exit (void);


/*
 * Display.
 */
const news_t *news_generate( int *ngen, int n );
void news_widget( unsigned int wid, int x, int y, int w, int h );


#endif /* NEWS_H */
