

#include "ai.h"

#include <math.h>

/* yay more lua */
#include "lauxlib.h"
#include "lualib.h"

#include "main.h"
#include "log.h"
#include "pilot.h"
#include "player.h"
#include "physics.h"
#include "pack.h"
#include "rng.h"
#include "space.h"
#include "faction.h"


/*
 * AI Overview
 *
 *	Concept: Goal (Task) Based AI with additional Optimization
 *
 *	  AI uses the goal (task) based AI approach with tasks scripted in lua,
 *	additionally there is a task that is hardcoded and obligatory in any AI
 *	script, the 'control' task, whose sole purpose is to assign tasks if there
 *	is no current tasks and optimizes or changes tasks if there are.
 *	  For example: Pilot A is attacking Pilot B.  Say that Pilot C then comes in
 *	the same system and is of the same faction as Pilot B, and therefore attacks
 *	Pilot A.  Pilot A would keep on fighting Pilot B until the control task kicks
 *	in.  Then he/she could run if it deems that Pilot C and Pilot B together are too
 *	strong for him/her, or attack Pilot C because it's an easier target to finish off
 *	then Pilot B.  Therefore there are endless possibilities and it's up to the AI
 *	coder to set up.
 *
 *
 * Specification
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
#define AI_LCALL(f)				(lua_getglobal(L, f), lua_pcall(L, 0, 0, 0))
/* registers a number constant n to name s (syntax like lua_register) */
#define lua_regnumber(l,s,n)	(lua_pushnumber(l,n), lua_setglobal(l,s))
/* L state, void* buf, int n size, char* s identifier */
#define luaL_dobuffer(L, b, n, s) \
	(luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

/* makes the function not run if n minimum parameters aren't passed */
#define MIN_ARGS(n)			if (lua_gettop(L) < n) return 0


#define MIN_DIR_ERR		5.0*M_PI/180.
#define MAX_DIR_ERR		2.5*M_PI/180.
#define MIN_VEL_ERR		2.5


/*
 * file info
 */
#define AI_PREFIX			"ai/"
#define AI_SUFFIX			".lua"


/*
 * all the AI profiles
 */
static AI_Profile* profiles = NULL;
static int nprofiles = 0;
/* Current AI Lua interpreter */
static lua_State* L = NULL;


/*
 * extern pilot hacks
 */
extern Pilot** pilot_stack;
extern int pilots;


/*
 * prototypes
 */
/* Internal C routines */
static int ai_loadProfile( char* filename );
static void ai_freetask( Task* t );
/* External C routines */
void ai_attacked( Pilot* attacked, const unsigned int attacker ); /* weapon.c */

/* AI routines for Lua */
/* tasks */
static int ai_pushtask( lua_State *L ); /* pushtask( string, number/pointer, number ) */
static int ai_poptask( lua_State *L ); /* poptask() */
static int ai_taskname( lua_State *L ); /* number taskname() */
/* consult values */
static int ai_gettarget( lua_State *L ); /* pointer gettarget() */
static int ai_gettargetid( lua_State *L ); /* number gettargetid() */
static int ai_armor( lua_State *L ); /* armor() */
static int ai_shield( lua_State *L ); /* shield() */
static int ai_parmor( lua_State *L ); /* parmor() */
static int ai_pshield( lua_State *L ); /* pshield() */
static int ai_getdistance( lua_State *L ); /* number getdist(Vector2d) */
static int ai_getpos( lua_State *L ); /* getpos(number) */
static int ai_minbrakedist( lua_State *L ); /* number minbrakedist() */
/* boolean expressions */
static int ai_ismaxvel( lua_State *L ); /* boolean ismaxvel() */
static int ai_isstopped( lua_State *L ); /* boolean isstopped() */
static int ai_isenemy( lua_State *L ); /* bool isenemy( number ) */
static int ai_isally( lua_State *L ); /* bool isally( number ) */
/* movement */
static int ai_accel( lua_State *L ); /* accel(number); number <= 1. */
static int ai_turn( lua_State *L ); /* turn(number); abs(number) <= 1. */
static int ai_face( lua_State *L ); /* face(number/pointer) */
static int ai_brake( lua_State *L ); /* brake() */
static int ai_getnearestplanet( lua_State *L ); /* pointer getnearestplanet() */
static int ai_getrndplanet( lua_State *L ); /* pointor getrndplanet() */
/* combat */
static int ai_shoot( lua_State *L ); /* shoot(number); number = 1,2,3 */
static int ai_getenemy( lua_State *L ); /* number getenemy() */
/* timers */
static int ai_settimer( lua_State *L ); /* settimer( number, number ) */
static int ai_timeup( lua_State *L ); /* bool timeup( number ) */
/* misc */
static int ai_createvect( lua_State *L ); /* createvect( number, number ) */
static int ai_comm( lua_State *L ); /* say( number, string ) */
static int ai_broadcast( lua_State *L ); /* broadcast( string ) */
static int ai_rng( lua_State *L ); /* rng( number, number ) */


/*
 * current pilot "thinking" and assorted variables
 */
static Pilot *cur_pilot = NULL;
static double pilot_acc = 0.;
static double pilot_turn = 0.;
static int pilot_primary = 0;


/*
 * destroys the ai part of the pilot
 */
void ai_destroy( Pilot* p )
{
	if (p->task)
		ai_freetask( p->task );
}


/* 
 * initializes the AI stuff which is basically Lua
 */
int ai_init (void)
{
	char** files;
	uint32_t nfiles,i;

	/* get the file list */
	files = pack_listfiles( data, &nfiles );

	/* load the profiles */
	for (i=0; i<nfiles; i++)
		if ((strncmp( files[i], AI_PREFIX, strlen(AI_PREFIX))==0) &&
				(strncmp( files[i] + strlen(files[i]) - strlen(AI_SUFFIX),
					AI_SUFFIX, strlen(AI_SUFFIX))==0))
			if (ai_loadProfile(files[i]))
				WARN("Error loading AI profile '%s'", files[i]);

	/* free the char* allocated by pack */
	for (i=0; i<nfiles; i++)
		free(files[i]);
	free(files);

	DEBUG("Loaded %d AI Profile%c", nprofiles, (nprofiles==1)?' ':'s');

	return 0;
}


/*
 * initializes an AI_Profile and adds it to the stack
 */
static int ai_loadProfile( char* filename )
{
	char* buf = NULL;
	uint32_t bufsize = 0;

	profiles = realloc( profiles, sizeof(AI_Profile)*(++nprofiles) );
	profiles[nprofiles-1].name = strndup( filename+strlen(AI_PREFIX),
			strlen(filename)-strlen(AI_PREFIX)-strlen(AI_SUFFIX));

	profiles[nprofiles-1].L = luaL_newstate();

	if (profiles[nprofiles-1].L == NULL) {
		ERR("Unable to create a new Lua state");
		return -1;
	}

	L = profiles[nprofiles-1].L;

	/* opens the standard lua libraries */
	/* luaL_openlibs(L); */

	/* constants */
	lua_regnumber(L, "player", PLAYER_ID); /* player ID */

	/* Register C functions in Lua */
	/* tasks */
	lua_register(L, "pushtask", ai_pushtask);
	lua_register(L, "poptask", ai_poptask);
	lua_register(L, "taskname", ai_taskname);
	/* consult values */
	lua_register(L, "gettarget", ai_gettarget);
	lua_register(L, "gettargetid", ai_gettargetid);
	lua_register(L, "armor", ai_armor);
	lua_register(L, "shield", ai_shield);
	lua_register(L, "parmor", ai_parmor);
	lua_register(L, "pshield", ai_pshield);
	lua_register(L, "getdist", ai_getdistance);
	lua_register(L, "getpos", ai_getpos);
	lua_register(L, "minbrakedist", ai_minbrakedist);
	/* boolean expressions */
	lua_register(L, "ismaxvel", ai_ismaxvel);
	lua_register(L, "isstopped", ai_isstopped);
	lua_register(L, "isenemy", ai_isenemy);
	lua_register(L, "isally", ai_isally);
	/* movement */
	lua_register(L, "accel", ai_accel);
	lua_register(L, "turn", ai_turn);
	lua_register(L, "face", ai_face);
	lua_register(L, "brake", ai_brake);
	lua_register(L, "getnearestplanet", ai_getnearestplanet);
	lua_register(L, "getrndplanet", ai_getrndplanet);
	/* combat */
	lua_register(L, "shoot", ai_shoot);
	lua_register(L, "getenemy", ai_getenemy);
	/* timers */
	lua_register(L, "settimer", ai_settimer);
	lua_register(L, "timeup", ai_timeup);
	/* misc */
	lua_register(L, "createvect", ai_createvect);
	lua_register(L, "comm", ai_comm);
	lua_register(L, "broadcast", ai_broadcast);
	lua_register(L, "rng", ai_rng);

	/* now load the file since all the functions have been previously loaded */
	buf = pack_readfile( DATA, filename, &bufsize );
	if (luaL_dobuffer(L, buf, bufsize, filename) != 0) {
		ERR("Error loading AI file: %s",filename);
		ERR("%s",lua_tostring(L,-1));
		WARN("Most likely Lua file has improper syntax, please check");
		return -1;
	}
	free(buf);

	return 0;
}


/*
 * gets the AI_Profile with name
 */
AI_Profile* ai_getProfile( char* name )
{
	if (profiles == NULL) return NULL;

	int i;

	for (i=0; i<nprofiles; i++)
		if (strcmp(name,profiles[i].name)==0)
			return &profiles[i];

	WARN("AI Profile '%s' not found in AI stack", name);
	return NULL;
}


/*
 * cleans up global AI
 */
void ai_exit (void)
{
	int i;
	for (i=0; i<nprofiles; i++) {
		free(profiles[i].name);
		lua_close(profiles[i].L);
	}
	free(profiles);
}


/*
 * heart of the AI, brains of the pilot
 */
void ai_think( Pilot* pilot )
{
	cur_pilot = pilot; /* set current pilot being processed */
	L = cur_pilot->ai->L; /* set the AI profile to the current pilot's */

	/* clean up some variables */
	pilot_acc = pilot_turn = 0.;
	pilot_primary = 0;

	
	/* control function if pilot is idle or tick is up */
	if ((cur_pilot->tcontrol < SDL_GetTicks()) || (cur_pilot->task == NULL)) {
		AI_LCALL("control"); /* run control */
		lua_getglobal(L,"control_rate");
		cur_pilot->tcontrol = SDL_GetTicks() +  1000*(int)lua_tonumber(L,-1);
	}

	/* pilot has a currently running task */
	if (cur_pilot->task != NULL)
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

	if (pilot_primary) pilot_shoot(pilot, 0); /* is shooting */
}


/*
 * pilot is attacked
 */
void ai_attacked( Pilot* attacked, const unsigned int attacker )
{
	cur_pilot = attacked;
	L = cur_pilot->ai->L;
	lua_getglobal(L, "attacked");
	lua_pushnumber(L, attacker);
	lua_pcall(L, 1, 0, 0);
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
	if (t->dtype == TYPE_PTR) free(t->target);
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
	t->target = NULL;

	if (lua_gettop(L) > 2) {
		if (lua_isnumber(L,3)) {
			t->dtype = TYPE_INT;
			t->ID = (unsigned int)lua_tonumber(L,3);
		}
		else if (lua_islightuserdata(L,3)) { /* only pointer valid is Vector2d* in Lua */
			t->dtype = TYPE_PTR;
			t->target = MALLOC_ONE(Vector2d);
			vectcpy( t->target, (Vector2d*)lua_topointer(L,3) );
		}
		else t->dtype = TYPE_NULL;
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
	(void)L; /* hack to avoid -W -Wall warnings */
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
	else lua_pushstring(L, "none");
	return 1;
}

/*
 * gets the target pointer
 */
static int ai_gettarget( lua_State *L )
{
	if (cur_pilot->task->dtype == TYPE_PTR) {
		lua_pushlightuserdata(L, cur_pilot->task->target);
		return 1;
	}
	return 0;
}

/*
 * gets the target ID
 */
static int ai_gettargetid( lua_State *L )
{
	if (cur_pilot->task->dtype == TYPE_INT) {
		lua_pushnumber(L, cur_pilot->task->ID);
		return 1;
	}
	return 0;
}

/*
 * gets the pilot's armor
 */
static int ai_armor( lua_State *L )
{
	lua_pushnumber(L, cur_pilot->armor);
	return 1;
}

/*
 * gets the pilot's shield
 */
static int ai_shield( lua_State *L )
{
	lua_pushnumber(L, cur_pilot->shield);
	return 1;
}

/*
 * gets the pilot's armor in percent
 */
static int ai_parmor( lua_State *L )
{
	lua_pushnumber(L, cur_pilot->armor / cur_pilot->armor_max * 100.);
	return 1;
}

/* 
 * gets the pilot's shield in percent
 */              
static int ai_pshield( lua_State *L )
{                               
	lua_pushnumber(L, cur_pilot->shield / cur_pilot->shield_max * 100.);
	return 1;
} 

/*
 * gets the distance from the pointer
 */
static int ai_getdistance( lua_State *L )
{
	MIN_ARGS(1);
	Vector2d *vect = (Vector2d*)lua_topointer(L,1);
	lua_pushnumber(L, vect_dist(vect,&cur_pilot->solid->pos));
	return 1;
}

/*
 * gets the pilot's position
 */
static int ai_getpos( lua_State *L )
{
	Pilot *p;
	if (lua_isnumber(L,1)) p = pilot_get((int)lua_tonumber(L,1)); /* Pilot ID */
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
 * is at maximum velocity?
 */
static int ai_ismaxvel( lua_State *L )
{
	lua_pushboolean(L,(VMOD(cur_pilot->solid->vel) == cur_pilot->ship->speed));
	return 1;
}


/*
 * is stopped?
 */
static int ai_isstopped( lua_State *L )
{
	lua_pushboolean(L,(VMOD(cur_pilot->solid->vel) < MIN_VEL_ERR));
	return 1;
}


/*
 * checks if pilot is an enemy
 */
static int ai_isenemy( lua_State *L )
{
	if (lua_isnumber(L,1))
		lua_pushboolean(L,areEnemies(cur_pilot->faction,
				pilot_get(lua_tonumber(L,1))->faction));
	return 1;
}


/*
 * checks if pillot is an ally
 */
static int ai_isally( lua_State *L )
{
	if (lua_isnumber(L,1))
		lua_pushboolean(L,areAllies(cur_pilot->faction,
				pilot_get(lua_tonumber(L,1))->faction));
	return 1;
}


/*
 * starts accelerating the pilot based on a parameter
 */
static int ai_accel( lua_State *L )
{
	pilot_acc = (lua_gettop(L) > 1 && lua_isnumber(L,1)) ? ABS((double)lua_tonumber(L,1)) : 1. ;
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
	if (lua_isnumber(L,1)) v = &pilot_get((unsigned int)lua_tonumber(L,1))->solid->pos;
	else if (lua_islightuserdata(L,1)) v = (Vector2d*)lua_topointer(L,1);

	double mod = -10;
	if (lua_gettop(L) > 1 && lua_isnumber(L,2))
		switch ((int)lua_tonumber(L,2)) {
			case 0: break;
			case 1: mod *= -1; break;
			case 2: break;
		}
	double diff = angle_diff(cur_pilot->solid->dir,vect_angle(&cur_pilot->solid->pos, v));

	pilot_turn = mod*diff;

	lua_pushnumber(L, ABS(diff*180./M_PI));
	return 1;
}


/*
 * brakes the pilot
 */
static int ai_brake( lua_State *L )
{
	(void)L; /* hack to avoid -W -Wall warnings */
	double diff = angle_diff(cur_pilot->solid->dir,VANGLE(cur_pilot->solid->vel));
	pilot_turn = 10*diff;
	if (diff < MAX_DIR_ERR && VMOD(cur_pilot->solid->vel) > MIN_VEL_ERR)
		pilot_acc = 1.;
	return 0;
}

/*
 * returns the nearest friendly planet's position to the pilot
 */
static int ai_getnearestplanet( lua_State *L )
{
	if (cur_system->nplanets == 0) return 0; /* no planets */

	double dist, d;
	int i, j;

	/* cycle through planets */
	for (dist=0., j=-1, i=0; i<cur_system->nplanets; i++) {
		d = vect_dist( &cur_system->planets[i].pos, &cur_pilot->solid->pos );
		if ((!areEnemies(cur_pilot->faction,cur_system->planets[i].faction)) &&
				(d < dist)) { /* closer friendly planet */
			j = i;
			dist = d;
		}
	}

	/* no friendly planet found */
	if (j == -1) return 0;

	lua_pushlightuserdata( L, &cur_system->planets[j].pos );
	return 1;
}

/*
 * returns a random friendly planet's position to the pilot
 */
static int ai_getrndplanet( lua_State *L )
{
	if (cur_system->nplanets == 0) return 0; /* no planets */

	Planet** planets;
	int nplanets, i;
	planets = malloc( sizeof(Planet*) * cur_system->nplanets );

	for (nplanets=0, i=0; i<cur_system->nplanets; i++)
		if (!areEnemies(cur_pilot->faction,cur_system->planets[i].faction))
			planets[nplanets++] = &cur_system->planets[i];

	/* no planet to land on found */
	if (nplanets==0) {
		free(planets);
		return 0;
	}

	/* we can actually get a random planet now */
	i = RNG(0,nplanets-1);
	lua_pushlightuserdata( L, &planets[i]->pos );
	free(planets);
	return 1;
}

/*
 * makes the pilot shoot
 */
static int ai_shoot( lua_State *L )
{
	int n = 1;
	if (lua_isnumber(L,1)) n = (int)lua_tonumber(L,1);

	if (n==1) pilot_primary = 1;
	/* else if (n==2) pilot_secondary = 1;
		else if  (n==3) pilot_primary = pilot_secondary = 1; */

	return 0;
}


/*
 * gets the nearest enemy
 */
static int ai_getenemy( lua_State *L )
{
	lua_pushnumber(L,pilot_getNearest(cur_pilot));
	return 1;
}


/*
 * sets the timer
 */
static int ai_settimer( lua_State *L )
{
	MIN_ARGS(2);

	int n; /* get the timer */
	if (lua_isnumber(L,1)) n = lua_tonumber(L,1);

	cur_pilot->timer[n] = (lua_isnumber(L,2)) ? lua_tonumber(L,2) + SDL_GetTicks() : 0;

	return 0;
}

/*
 * checks the timer
 */
static int ai_timeup( lua_State *L )
{
	MIN_ARGS(1);

	int n; /* get the timer */
	if (lua_isnumber(L,1)) n = lua_tonumber(L,1);

	lua_pushboolean(L, cur_pilot->timer[n] < SDL_GetTicks());
	return 1;
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

	lua_pushlightuserdata(L, v);
	return 1;
}

/*
 * makes the pilot say something to the player
 */
static int ai_comm( lua_State *L )
{
	MIN_ARGS(2);
	
	if (lua_isnumber(L,1) && (lua_tonumber(L,1)==PLAYER_ID) && lua_isstring(L,2))
		player_message( "Comm %s> \"%s\"", cur_pilot->name, lua_tostring(L,2));

	return 0;
}

/*
 * broadcasts to the entire area
 */
static int ai_broadcast( lua_State *L )
{
	MIN_ARGS(1);

	if (lua_isstring(L,1))
		player_message( "Broadcast %s> \"%s\"", cur_pilot->name, lua_tostring(L,1));

	return 0;
}

/*
 * returns a random number between low and high
 */
static int ai_rng( lua_State *L )
{
	MIN_ARGS(2);

	int l,h;

	if (lua_isnumber(L,1)) l = (int)lua_tonumber(L,1);
	if (lua_isnumber(L,2)) h = (int)lua_tonumber(L,2);

	lua_pushnumber(L,RNG(l,h));
	return 1;
}

