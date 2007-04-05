

#include "ai.h"

/* yay lua */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "all.h"
#include "log.h"
#include "pilot.h"


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

