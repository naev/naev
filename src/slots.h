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

