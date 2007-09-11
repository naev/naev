/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef AI_H
#  define AI_H


/* yay lua */
#include "lua.h"


#define MIN_DIR_ERR		1.0*M_PI/180.
#define MAX_DIR_ERR		0.1*M_PI/180.
#define MIN_VEL_ERR		1.0


/* maximum number of AI timers */
#define MAX_AI_TIMERS	2


typedef enum { TYPE_NULL, TYPE_INT, TYPE_PTR } TaskData;

/* 
 * Basic task
 *  @name is the task's name (function name in Lua)
 *  @target is the target which will depend on the task itself
 */
typedef struct Task {
	struct Task* next;
	char *name;
	
	TaskData dtype;
	union {
		void *target; /* Vector2d, etc... */
		unsigned int ID; /* Pilot ID, etc... */
	};
} Task;


/*
 * the AI profile
 */
typedef struct {
	char* name;
	lua_State *L;
} AI_Profile;


/*
 * misc
 */
AI_Profile* ai_getProfile( char* name );


/*
 * init/exit
 */
int ai_init (void);
void ai_exit (void);


#endif /* AI_H */
