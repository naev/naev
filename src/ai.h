

#ifndef AI_H
#  define AI_H


/* yay lua */
#include "lua.h"


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
