/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "physics.h"

/* Forward declaration to avoid cyclical import. */
struct Pilot_;
typedef struct Pilot_ Pilot;

#define MIN_DIR_ERR     5.0*M_PI/180. /**< Minimum direction error. */
#define MAX_DIR_ERR     0.5*M_PI/180. /**< Maximum direction error. */
#define MIN_VEL_ERR     5.0 /**< Minimum velocity error. */

/* maximum number of AI timers */
#define MAX_AI_TIMERS   2 /**< Max amount of AI timers. */

/**
 * @struct Task
 *
 * @brief Basic AI task.
 */
typedef struct Task_ {
   struct Task_* next; /**< Next task */
   char *name; /**< Task name. */
   int func; /**< Reference to the function to be run. */
   int done; /**< Task is done and ready for deletion. */

   struct Task_* subtask; /**< Subtasks of the current task. */

   int dat; /**< Lua reference to the data (index in registry). */
} Task;

/**
 * @struct AI_Profile
 *
 * @brief Basic AI profile.
 */
typedef struct AI_Profile_ {
   char* name;       /**< Name of the profile. */
   nlua_env env;     /**< Assosciated Lua Environment. */
   double control_rate; /**< Control rate of the AI. */
   int lua_mem;      /**< Initial memory state. */
   int ref_control;  /**< Profile control reference function. */
   int ref_control_manual; /**< Profile manual control reference function. */
   int ref_refuel;   /**< Profile refuel reference function. */
   int ref_create;   /**< Run when pilot is created (or initialized in the case of persistent pilots). */
} AI_Profile;

/**
 * @struct AIMemory
 *
 * @brief Represents a temporary pilot memory. For use with ai_setPilot and ai_unsetPilot
 * @see ai_setPilot
 * @see ai_unsetPilot
 */
typedef struct AIMemory_ {
   int mem;    /**< Lua memory. */
   Pilot *p;   /**< Pilot pointer. */
} AIMemory;

/*
 * misc
 */
AI_Profile* ai_getProfile( const char *name );

/*
 * init/exit
 */
int ai_load (void);
void ai_exit (void);
int nlua_loadAI( nlua_env env );

/*
 * Init, destruction.
 */
int ai_pinit( Pilot *p, const char *ai );
void ai_destroy( Pilot* p );

/*
 * Task related.
 */
Task *ai_newtask( lua_State *L, Pilot *p, const char *func, int subtask, int pos );
Task* ai_curTask( Pilot* pilot );
void ai_freetask( Task* t );
void ai_cleartasks( Pilot* p );

/*
 * Misc functions.
 */
void ai_attacked( Pilot* attacked, const unsigned int attacker, double dmg );
void ai_discovered( Pilot* discovered );
void ai_hail( Pilot* recipient );
void ai_refuel( Pilot* refueler, unsigned int target );
void ai_getDistress( const Pilot *p, const Pilot *distressed, const Pilot *attacker );
void ai_think( Pilot* pilot, const double dt );
AIMemory ai_setPilot( Pilot *p );
void ai_unsetPilot( AIMemory oldmem );
void ai_thinkSetup (void);
void ai_thinkApply( Pilot *p );
void ai_init( Pilot *p );
