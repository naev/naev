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


#ifndef PILOT_HOOK_H
#  define PILOT_HOOK_H


#include "pilot.h"


/*
 * Hooks.
 */
void pilot_addHook( Pilot *pilot, int type, unsigned int hook );
int pilot_runHook( Pilot* p, int hook_type );
void pilots_rmHook( unsigned int hook );
void pilot_clearHooks( Pilot *p );


/*
 * Global hooks.
 */
void pilots_addGlobalHook( int type, unsigned int hook );
void pilots_rmGlobalHook( unsigned int hook );
void pilots_clearGlobalHooks (void);
void pilot_freeGlobalHooks (void);


#endif /* PILOT_HOOK_H */
