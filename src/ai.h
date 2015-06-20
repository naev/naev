/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef AI_H
#  define AI_H


/* yay Lua */
#include <lua.h>

#include "physics.h"

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
   TASKDATA_VEC2
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


#endif /* AI_H */
