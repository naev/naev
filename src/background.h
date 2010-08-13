/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef BACKGROUND_H
#  define BACKGROUND_H


/* Stars. */
void background_initStars( int n );
void background_renderStars( const double dt );


/* Clean up. */
void background_free (void);


#endif /* BACKGROUND_H */


