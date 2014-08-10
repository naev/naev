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
