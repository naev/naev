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


#ifndef AI_H
#  define AI_H


/* yay Lua */
#include <lua.h>

#include "physics.h"

/* Forward declaration to avoid cyclical import. */
struct Pilot_;
typedef struct Pilot_ Pilot;


#define AI_MEM          "__mem" /**< Internal pilot memory. */


#define MIN_DIR_ERR     5.0*M_PI/180. /**< Minimum direction error. */
#define MAX_DIR_ERR     0.5*M_PI/180. /**< Maximum direction error. */
#define MIN_VEL_ERR     5.0 /**< Minimum velocity error. */


/* maximum number of AI timers */
#define MAX_AI_TIMERS   2 /**< Max amount of AI timers. */


/**
 * @enum TaskData
 *
 * @brief Task data types.
 */
typedef enum TaskData_ {
   TASKDATA_NULL,
   TASKDATA_INT,
   TASKDATA_VEC2,
   TASKDATA_PILOT
} TaskData;

/**
 * @struct Task
 *
 * @brief Basic AI task.
 */
typedef struct Task_ {
   struct Task_* next; /**< Next task */
   char *name; /**< Task name. */
   int done; /**< Task is done and ready for deletion. */

   struct Task_* subtask; /**< Subtasks of the current task. */

   TaskData dtype; /**< Data type. */
   union {
      unsigned int num; /**< Pilot ID, etc... */
      Vector2d vec; /**< Vector. */
   } dat; /**< Stores the data. */
} Task;


/**
 * @struct AI_Profile
 *
 * @brief Basic AI profile.
 */
typedef struct AI_Profile_ {
   char* name; /**< Name of the profile. */
   lua_State *L; /**< Assosciated Lua State. */
} AI_Profile;


/*
 * misc
 */
AI_Profile* ai_getProfile( char* name );


/*
 * init/exit
 */
int ai_load (void);
void ai_exit (void);


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
void ai_attacked( Pilot* attacked, const unsigned int attacker, double dmg );
void ai_refuel( Pilot* refueler, unsigned int target );
void ai_getDistress( Pilot *p, const Pilot *distressed, const Pilot *attacker );
void ai_think( Pilot* pilot, const double dt );
void ai_setPilot( Pilot *p );


#endif /* AI_H */
