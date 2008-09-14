/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef UNIDIFF_H
#  define UNIDIFF_H


int diff_apply( char *name );
void diff_remove( char *name );
void diff_clear (void);
int diff_isApplied( char *name );


#endif /* UNIDIFF_H */

