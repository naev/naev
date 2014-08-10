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


#ifndef AI_EXTRA_H
#  define AI_EXTRA_H


#include "ai.h"
#include "pilot.h"


/*
 * Init, destruction.
 */
int ai_pinit( Pilot *p, const char *ai );
void ai_destroy( Pilot* p );

/*
 * Task related.
 */
Task *ai_newtask( Pilot *p, const char *func, int subtask, int pos );
void ai_freetask( Task* t );
void ai_cleartasks( Pilot* p );

/*
 * Misc functions.
 */
void ai_attacked( Pilot* attacked, const unsigned int attacker );
void ai_refuel( Pilot* refueler, unsigned int target );
void ai_getDistress( Pilot* p, const Pilot* distressed );
void ai_think( Pilot* pilot, const double dt );
void ai_setPilot( Pilot *p );



#endif /* AI_EXTRA_H */
