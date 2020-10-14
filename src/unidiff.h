/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef UNIDIFF_H
#  define UNIDIFF_H


int diff_loadAvailable (void);
NONNULL( 1 ) int diff_apply( const char *name );
NONNULL( 1 ) void diff_remove( const char *name );
void diff_clear (void);
NONNULL( 1 ) int diff_isApplied( const char *name );


#endif /* UNIDIFF_H */
