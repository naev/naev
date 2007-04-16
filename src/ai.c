

#include "ai.h"

#include <math.h>

/* yay lua */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "all.h"
#include "log.h"
#include "pilot.h"
#include "physics.h"
#include "pack.h"


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

/* makes the function not run if n minimum parameters aren't passed */
#define MIN_ARGS(n)			if (lua_gettop(L) < n) return 0


/*
 * prototypes
 */
/* Internal C routines */
static void ai_freetask( Task* t );
/* AI routines for Lua */
/* tasks */
static int ai_pushtask( lua_State *L ); /* pushtask( string, number/pointer, number ) */
static int ai_poptask( lua_State *L ); /* poptask() */
static int ai_taskname( lua_State *L ); /* number taskname() */
/* consult values */
static int ai_gettarget( lua_State *L ); /* pointer gettarget() */
static int ai_gettargetid( lua_State *L ); /* number gettargetid() */
static int ai_getdistance( lua_State *L ); /* number getdist(Vector2d) */
static int ai_getpos( lua_State *L ); /* getpos(number/Pilot) */
static int ai_minbrakedist( lua_State *L ); /* number minbrakedist() */
/* movement */
static int ai_accel( lua_State *L ); /* accel(number); number <= 1. */
static int ai_turn( lua_State *L ); /* turn(number); abs(number) <= 1. */
static int ai_face( lua_State *L ); /* face(number/pointer) */
/* misc */
static int ai_createvect( lua_State *L ); /* createvect( number, number ) */


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

	/* opens the standard lua libraries */
	luaL_openlibs(L);

	/* Register C functions in Lua */
	lua_register(L, "pushtask", ai_pushtask);
	lua_register(L, "poptask", ai_poptask);
	lua_register(L, "taskname", ai_taskname);
	lua_register(L, "gettarget", ai_gettarget);
	lua_register(L, "gettargetid", ai_gettargetid);
	lua_register(L, "getdist", ai_getdistance);
	lua_register(L, "getpos", ai_getpos);
	lua_register(L, "minbrakedist", ai_minbrakedist);
	lua_register(L, "accel", ai_accel);
	lua_register(L, "turn", ai_turn);
	lua_register(L, "face", ai_face);
	lua_register(L, "createvect", ai_createvect);

	char *buf = pack_readfile( DATA, "ai/basic.lua", NULL );
	
	if (luaL_dostring(L, buf) != 0) {
		WARN("Unable to load AI file: %s","ai/basic.lua");
		return -1;
	}

	free(buf);


/*	if (luaL_dofile(L, "ai/basic.lua") != 0) {
		WARN("Unable to load AI file: %s","ai_basic.lua");
		return -1;
	}*/

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

	if (cur_pilot->task == NULL) /* pilot is IDLE */
		AI_LCALL("control");

	else /* pilot has a currently running task */
		AI_LCALL(cur_pilot->task->name);

	/* make sure pilot_acc and pilot_turn are legal */
	if (pilot_acc > 1.) pilot_acc = 1.; /* value must be <= 1 */
	if (pilot_turn > 1.) pilot_turn = 1.; /* value must between -1 and 1 */
	else if (pilot_turn < -1.) pilot_turn = -1.;

	cur_pilot->solid->dir_vel = 0.;
	if (pilot_turn) /* set the turning velocity */
		cur_pilot->solid->dir_vel -= cur_pilot->ship->turn * pilot_turn;
	vect_pset( &cur_pilot->solid->force, /* set the velocity vector */
			cur_pilot->ship->thrust * pilot_acc, cur_pilot->solid->dir );

}


/*
 * internal use C functions
 */
/*
 * frees the task
 */
static void ai_freetask( Task* t )
{
	if (t->next) ai_freetask(t->next); /* yay recursive freeing */

	if (t->name) free(t->name);
	if (t->target) free(t->target);
	free(t);
}


/*
 * C Functions to call from Lua
 */
/*
 * pushes the current stack
 */
static int ai_pushtask( lua_State *L )
{
	int pos;
	if (lua_isnumber(L,1)) pos = (int)lua_tonumber(L,1);
	else return 0; /* invalid param */

	Task* t = MALLOC_ONE(Task);
	t->name = (lua_isstring(L,2)) ? strdup((char*)lua_tostring(L,2)) : NULL;
	t->next = NULL;

	if (lua_gettop(L) > 2) {
		if (lua_isnumber(L,3))
			t->ID = (unsigned int)lua_tonumber(L,3);
		else if (lua_islightuserdata(L,3))
			t->target = (void*)lua_topointer(L,3);
	}

	if (cur_pilot->task == NULL) /* no other tasks */
		cur_pilot->task = t;
	else if (pos == 1) { /* put at the end */
		Task* pointer;                                                                
		for (pointer = cur_pilot->task; pointer->next; pointer = pointer->next);
		pointer->next = t;
	}
	else { /* default put at the beginning */
		t->next = cur_pilot->task;
		cur_pilot->task = t;
	}

	return 0;
}

/*
 * pops the current task
 */
static int ai_poptask( lua_State *L )
{
	Task* t = cur_pilot->task;
	cur_pilot->task = t->next;
	t->next = NULL;
	ai_freetask(t);
	return 0;
}

/*
 * gets the current task's name
 */
static int ai_taskname( lua_State *L )
{
	if (cur_pilot->task) lua_pushstring(L, cur_pilot->task->name);
	else lua_pushnil(L);
	return 1;
}

/*
 * gets the target pointer
 */
static int ai_gettarget( lua_State *L )
{
	lua_pushlightuserdata(L, cur_pilot->task->target);
	return 1;
}
/*
 * gets the target ID
 */
static int ai_gettargetid( lua_State *L )
{
	lua_pushnumber(L, cur_pilot->task->ID);
	return 1;
}

/*
 * gets the distance from the pointer
 */
static int ai_getdistance( lua_State *L )
{
	MIN_ARGS(1);
	Vector2d *vect = (Vector2d*)lua_topointer(L,1);
	lua_pushnumber(L, MOD(vect->x-cur_pilot->solid->pos.x,vect->y-cur_pilot->solid->pos.y));
	return 1;
}

/*
 * gets the pilot's position
 */
static int ai_getpos( lua_State *L )
{
	Pilot *p;
	if (lua_isnumber(L,1)) p = get_pilot((int)lua_tonumber(L,1)); /* Pilot ID */
	else if (lua_islightuserdata(L,1)) p = (Pilot*)lua_topointer(L,1); /* Pilot pointer */
	else p = cur_pilot; /* default to self */

	lua_pushlightuserdata(L, &p->solid->pos );

	return 1;
}

/*
 * gets the minimum breaking distance
 *
 * braking vel ==> 0 = v - a*dt
 * add turn around time (to inital vel) ==> 180.*360./cur_pilot->ship->turn
 * add it to general euler equation  x = v * t + 0.5 * a * t^2
 * and voila!
 */
static int ai_minbrakedist( lua_State *L )
{
	double time = VMOD(cur_pilot->solid->vel) /
			(cur_pilot->ship->thrust / cur_pilot->solid->mass);
	double dist =  VMOD(cur_pilot->solid->vel)*(time+cur_pilot->ship->turn/360.) -
			0.5*(cur_pilot->ship->thrust/cur_pilot->solid->mass)*time*time;

	lua_pushnumber(L, dist); /* return */
	return 1; /* returns one thing */
}


/*
 * starts accelerating the pilot based on a parameter
 */
static int ai_accel( lua_State *L )
{
	MIN_ARGS(1);
	pilot_acc = (lua_isnumber(L,1)) ? ABS((double)lua_tonumber(L,1)) : 1. ;
	return 0;
}


/*
 * starts turning the pilot based on a parameter
 */
static int ai_turn( lua_State *L )
{
	MIN_ARGS(1);
	pilot_turn = (lua_isnumber(L,1)) ? (double)lua_tonumber(L,1) : 0. ;
	return 0;
}


/*
 * faces the target
 */
static int ai_face( lua_State *L )
{
	MIN_ARGS(1);
	Vector2d* v; /* get the position to face */
	if (lua_isnumber(L,1)) v = &get_pilot((unsigned int)lua_tonumber(L,1))->solid->pos;
	else if (lua_islightuserdata(L,1)) v = (Vector2d*)lua_topointer(L,1);

	double mod = -10;
	if (lua_gettop(L) > 1 && lua_isnumber(L,2))
		switch ((int)lua_tonumber(L,2)) {
			case 1: mod *= -1; break;
			case 2: break;
		}

	pilot_turn = mod*angle_diff(cur_pilot->solid->dir,vect_angle(&cur_pilot->solid->pos, v));

	return 0;
}


/*
 * creates a Vector2d
 */
static int ai_createvect( lua_State *L )
{
	MIN_ARGS(2);
	Vector2d* v = MALLOC_ONE(Vector2d);
	double x = (lua_isnumber(L,1)) ? (double)lua_tonumber(L,1) : 0. ;
	double y = (lua_isnumber(L,2)) ? (double)lua_tonumber(L,2) : 0. ;

	vect_cset(v, x, y);

	lua_pushlightuserdata(L, (void*)v);
	return 1;
}

