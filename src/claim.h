/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef CLAIM_H
#  define CLAIM_H


#include "nxml.h"


/* Forward declaration. */
struct SysClaim_s;
typedef struct SysClaim_s SysClaim_t;


/*
 * Individual claim handling.
 */
SysClaim_t *claim_create (void);
int claim_add( SysClaim_t *claim, int ss_id );
int claim_test( SysClaim_t *claim );
int claim_testSys( SysClaim_t *claim, int sys );
void claim_destroy( SysClaim_t *claim );


/*
 * Global claim handling.
 */
void claim_clear (void);
void claim_activateAll (void);
void claim_activate( SysClaim_t *claim );


/*
 * Saving/loading.
 */
int claim_xmlSave( xmlTextWriterPtr writer, SysClaim_t *claim );
SysClaim_t *claim_xmlLoad( xmlNodePtr parent );


#endif /* CLAIM_H */
