/*
 * See Licensing and Copyright notice in naev.h
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
void ai_getDistress( Pilot *p, const Pilot *distressed, const Pilot *attacker );
void ai_think( Pilot* pilot, const double dt );
void ai_setPilot( Pilot *p );



#endif /* AI_EXTRA_H */
