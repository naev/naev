

#include "ai.h"

/* yay lua */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "all.h"
#include "log.h"
#include "pilot.h"
#include "physics.h"


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


/* calls the AI function with name f */
#define AI_LCALL(f)	      (lua_getglobal(L, f), lua_call(L, 0, 0))


/*
 * prototypes
 */
static int ai_minbrakedist( lua_State *L ); /* minimal breaking distance */
static int ai_accel( lua_State *L ); /* accelerate */


/*
 * Basic task
 *  @name is the task's name (function name in Lua)
 *  @target is the target which will depend on the task itself
 */
typedef struct {
	char *name;
	union {
		void *target;
		unsigned int ID;
	};
} Task;


/* Global AI Lua interpreter */
static lua_State *L = NULL;


/*
 * current pilot "thinking" and assorted variables
 */
static Pilot *cur_pilot = NULL;
static double pilot_acc = 0.;
static double pilot_turn = 0.;


/* initializes the AI stuff which is basically Lua */
int ai_init (void)
{  
	L = luaL_newstate();
	if (L == NULL)
		return -1;

	/* Register C functions in Lua */
	lua_register(L, "minbrakedist", ai_minbrakedist);
	lua_register(L, "accel", ai_accel);

	if (luaL_dofile(L, "ai_basic.lua") != 0) {
		WARN("Unable to load AI file: %s","ai_basic.lua");
		return -1;
	}

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
	cur_pilot = pilot; /* set current pilot being processed */
	pilot_acc = pilot_turn = 0.; /* clean up some variables */

	if (pilot->action == NULL) { /* pilot is IDLE */
		AI_LCALL("control");
	}


	cur_pilot->solid->dir_vel = 0.;
	if (pilot_turn)
		cur_pilot->solid->dir_vel -= cur_pilot->ship->turn * pilot_turn;
	vect_pset( &cur_pilot->solid->force, /* set the velocity vector */
			cur_pilot->ship->thrust * pilot_acc, cur_pilot->solid->dir );
}



/*
 * C Functions to call from Lua
 */

/*
 * gets the minimum breaking distance
 *
 * braking vel ==> v * t = 0.5 a * t^2 => t = 2 * v / a
 * add turn around time (to inital vel) ==> 180.*360./cur_pilot->ship->turn
 * add it to general euler equation  x = v * t + 0.5 * a * t^2
 * and voila!
 */
static int ai_minbrakedist( lua_State *L )
{
	double time = 2. * VMOD(cur_pilot->solid->vel) /
			(cur_pilot->ship->thrust/cur_pilot->solid->mass);
	double dist =  VMOD(cur_pilot->solid->vel)*(time+0.5*(180.*360./cur_pilot->ship->turn)) -
			0.5*(cur_pilot->ship->thrust/cur_pilot->solid->mass)*time*time;

	lua_pushnumber(L, dist); /* return */
	return 1; /* returns one thing */
}


static int ai_accel( lua_State *L )
{
	pilot_acc = (lua_isnumber(L,1)) ? (double)lua_tonumber(L,1) : 1. ;
	return 0;
}

