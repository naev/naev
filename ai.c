

#include "ai.h"

/* yay lua */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "all.h"
#include "log.h"
#include "pilot.h"


/*
 * AI Overview
 *
 * @ AI will follow basic tasks defined from Lua AI script.  
 *   @ if Task is NULL, AI will run "control" task
 *   @ Task is continued every frame
 *   @ "control" task is a special task that MUST exist in any given  Pilot AI
 *     (missiles and such will use "seek")
 *     @ "control" task is not permanent, but transitory
 *     @ "control" task sets another task
 *   @ "control" task is also run at a set rate (depending on Lua global "control_rate")
 *     to choose optimal behaviour (task)
 */


/*
 * Basic task
 *  @name is the task's name (function name in Lua)
 *  @target is the target which will depend on the task itself
 */
typedef struct {
	char *name;
	void *target;
} Task;


/* Global AI Lua interpreter */
static lua_State *L = NULL;


/* initializes the AI stuff which is basically Lua */
int ai_init (void)
{  
	L = luaL_newstate();
	if (L == NULL)
		return -1;

	return 0;
}

void ai_exit (void)
{
	lua_close(L);
}


/*
 * heart of the AI, brains of the pilot
 */
void ai_think( Pilot* pilot )
{  
	if (pilot->action == NULL) { /* pilot is IDLE */
	}
}

