/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SLOTPROPERTY_H
#  define SLOTPROPERTY_H


/* Load/exit. */
int sp_load (void);
void sp_cleanup (void);

/* Stuff. */
unsigned int sp_get( const char *name );
const char *sp_display( unsigned int sp );
const char *sp_description( unsigned int sp );
int sp_required( unsigned int spid );
int sp_exclusive( unsigned int spid );


#endif /* SLOTPROPERTY_H */

