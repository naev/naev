/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef CLAIM_H
#  define CLAIM_H


#include "nxml.h"


/* Forward declaration. */
struct Claim_s;
typedef struct Claim_s Claim_t;


/*
 * Individual claim handling.
 */
Claim_t *claim_create (void);
int claim_addStr( Claim_t *claim, const char *str );
int claim_addSys( Claim_t *claim, int ss_id );
int claim_test( Claim_t *claim );
int claim_testStr( Claim_t *claim, const char *str );
int claim_testSys( Claim_t *claim, int sys );
void claim_destroy( Claim_t *claim );


/*
 * Global claim handling.
 */
void claim_clear (void);
void claim_activateAll (void);
void claim_activate( Claim_t *claim );


/*
 * Saving/loading.
 */
int claim_xmlSave( xmlTextWriterPtr writer, Claim_t *claim );
Claim_t *claim_xmlLoad( xmlNodePtr parent );


#endif /* CLAIM_H */
