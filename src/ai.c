/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ai.c
 *
 * @brief Controls the Pilot AI.
 *
 * AI Overview
 *
 * Concept: Goal (Task) Based AI with additional Optimization
 *
 *  AI uses the goal (task) based AI approach with tasks scripted in lua,
 * additionally there is a task that is hardcoded and obligatory in any AI
 * script, the 'control' task, whose sole purpose is to assign tasks if there
 * is no current tasks and optimizes or changes tasks if there are.
 *
 *  For example: Pilot A is attacking Pilot B.  Say that Pilot C then comes in
 * the same system and is of the same faction as Pilot B, and therefore attacks
 * Pilot A.  Pilot A would keep on fighting Pilot B until the control task
 * kicks in.  Then he/she could run if it deems that Pilot C and Pilot B
 * together are too strong for him/her, or attack Pilot C because it's an
 * easier target to finish off then Pilot B.  Therefore there are endless
 * possibilities and it's up to the AI coder to set up.
 *
 *
 * Specification
 *
 *   -  AI will follow basic tasks defined from Lua AI script.  
 *     - if Task is NULL, AI will run "control" task
 *     - Task is continued every frame
 *     -  "control" task is a special task that MUST exist in any given  Pilot AI
 *        (missiles and such will use "seek")
 *     - "control" task is not permanent, but transitory
 *     - "control" task sets another task
 *   - "control" task is also run at a set rate (depending on Lua global "control_rate")
 *     to choose optimal behaviour (task)
 *
 * 
 * Memory
 *
 *  The AI currently has per-pilot memory which is accessible as "mem".  This
 * memory is actually stored in the table pilotmem[cur_pilot->id].  This allows
 * the pilot to keep some memory always accesible between runs without having
 * to rely on the storage space a task has.
 *
 * @todo Clean up most of the code, it was written as one of the first
 *         subsystems and is pretty lacking in quite a few aspects. Notably
 *         removing the entire lightuserdata thing and actually go with full
 *         userdata.
 */


#include "ai.h"

#include <stdlib.h>
#include <stdio.h> /* malloc realloc */
#include <string.h> /* strncpy strlen strncat strcmp strdup */
#include <math.h>

/* yay more lua */
#include "lauxlib.h"
#include "lualib.h"

#include "naev.h"
#include "log.h"
#include "pilot.h"
#include "player.h"
#include "physics.h"
#include "pack.h"
#include "rng.h"
#include "space.h"
#include "faction.h"
#include "escort.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"


/**
 * @def lua_regnumber(l,s,n)
 *
 * @brief Registers a number constant n to name s (syntax like lua_regfunc).
 */
#define lua_regnumber(l,s,n)  \
(lua_pushnumber(l,n), lua_setglobal(l,s))


/*
 * ai flags
 */
#define ai_setFlag(f)   (pilot_flags |= f ) /**< Sets pilot flag f */
#define ai_isFlag(f)    (pilot_flags & f ) /**< Checks pilot flag f */
/* flags */
#define AI_PRIMARY      (1<<0)   /**< Firing primary weapon */
#define AI_SECONDARY    (1<<1)   /**< Firing secondary weapon */


/*
 * file info
 */
#define AI_PREFIX       "ai/" /**< AI file prefix. */
#define AI_SUFFIX       ".lua" /**< AI file suffix. */
#define AI_INCLUDE      "include/" /**< Where to search for includes. */


/*
 * all the AI profiles
 */
static AI_Profile* profiles = NULL; /**< Array of AI_Profiles loaded. */
static int nprofiles = 0; /**< Number of AI_Profiles loaded. */


/*
 * extern pilot hacks
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * prototypes
 */
/* Internal C routines */
static void ai_run( lua_State *L, const char *funcname );
static int ai_loadProfile( char* filename );
static void ai_freetask( Task* t );
static void ai_setMemory (void);
static void ai_create( Pilot* pilot, char *param );
/* External C routines */
void ai_attacked( Pilot* attacked, const unsigned int attacker ); /* weapon.c */
/* C Routines made External */
int ai_pinit( Pilot *p, char *ai ); /* pilot.c */
void ai_destroy( Pilot* p ); /* pilot.c */
void ai_think( Pilot* pilot ); /* pilot.c */
void ai_setPilot( Pilot *p ); /* escort.c */


/*
 * AI routines for Lua
 */
/* tasks */
static int ai_pushtask( lua_State *L ); /* pushtask( string, number/pointer, number ) */
static int ai_poptask( lua_State *L ); /* poptask() */
static int ai_taskname( lua_State *L ); /* number taskname() */

/* consult values */
static int ai_getplayer( lua_State *L ); /* number getPlayer() */
static int ai_gettarget( lua_State *L ); /* pointer gettarget() */
static int ai_getrndpilot( lua_State *L ); /* number getrndpilot() */
static int ai_armour( lua_State *L ); /* armour() */
static int ai_shield( lua_State *L ); /* shield() */
static int ai_parmour( lua_State *L ); /* parmour() */
static int ai_pshield( lua_State *L ); /* pshield() */
static int ai_getdistance( lua_State *L ); /* number getdist(Vector2d) */
static int ai_getpos( lua_State *L ); /* getpos(number) */
static int ai_minbrakedist( lua_State *L ); /* number minbrakedist() */
static int ai_cargofree( lua_State *L ); /* number cargofree() */
static int ai_shipclass( lua_State *L ); /* string shipclass() */

/* boolean expressions */
static int ai_exists( lua_State *L ); /* boolean exists() */
static int ai_ismaxvel( lua_State *L ); /* boolean ismaxvel() */
static int ai_isstopped( lua_State *L ); /* boolean isstopped() */
static int ai_isenemy( lua_State *L ); /* boolean isenemy( number ) */
static int ai_isally( lua_State *L ); /* boolean isally( number ) */
static int ai_incombat( lua_State *L ); /* boolean incombat( [number] ) */
static int ai_haslockon( lua_State *L ); /* boolean haslockon() */

/* movement */
static int ai_accel( lua_State *L ); /* accel(number); number <= 1. */
static int ai_turn( lua_State *L ); /* turn(number); abs(number) <= 1. */
static int ai_face( lua_State *L ); /* face(number/pointer) */
static int ai_aim( lua_State *L ); /* aim(number) */
static int ai_brake( lua_State *L ); /* brake() */
static int ai_getnearestplanet( lua_State *L ); /* Vec2 getnearestplanet() */
static int ai_getrndplanet( lua_State *L ); /* Vec2 getrndplanet() */
static int ai_getlandplanet( lua_State *L ); /* Vec2 getlandplanet() */
static int ai_hyperspace( lua_State *L ); /* [number] hyperspace() */
static int ai_stop( lua_State *L ); /* stop() */
static int ai_relvel( lua_State *L ); /* relvel( number ) */

/* escorts */
static int ai_e_attack( lua_State *L ); /* bool e_attack() */
static int ai_e_hold( lua_State *L ); /* bool e_hold() */
static int ai_e_clear( lua_State *L ); /* bool e_clear() */
static int ai_e_return( lua_State *L ); /* bool e_return() */
static int ai_dock( lua_State *L ); /* dock( number ) */

/* combat */
static int ai_combat( lua_State *L ); /* combat( number ) */
static int ai_settarget( lua_State *L ); /* settarget( number ) */
static int ai_secondary( lua_State *L ); /* string secondary() */
static int ai_hasturrets( lua_State *L ); /* bool hasturrets() */
static int ai_shoot( lua_State *L ); /* shoot( number ); number = 1,2,3 */
static int ai_getenemy( lua_State *L ); /* number getenemy() */
static int ai_hostile( lua_State *L ); /* hostile( number ) */
static int ai_getweaprange( lua_State *L ); /* number getweaprange() */

/* timers */
static int ai_settimer( lua_State *L ); /* settimer( number, number ) */
static int ai_timeup( lua_State *L ); /* boolean timeup( number ) */

/* messages */
static int ai_comm( lua_State *L ); /* say( number, string ) */
static int ai_broadcast( lua_State *L ); /* broadcast( string ) */

/* loot */
static int ai_credits( lua_State *L ); /* credits( number ) */
static int ai_cargo( lua_State *L ); /* cargo( name, quantity ) */
static int ai_shipprice( lua_State *L ); /* shipprice() */


static const luaL_reg ai_methods[] = {
   /* tasks */
   { "pushtask", ai_pushtask },
   { "poptask", ai_poptask },
   { "taskname", ai_taskname },
   /* is */
   { "exists", ai_exists },
   { "ismaxvel", ai_ismaxvel },
   { "isstopped", ai_isstopped },
   { "isenemy", ai_isenemy },
   { "isally", ai_isally },
   { "incombat", ai_incombat },
   { "haslockon", ai_haslockon },
   /* get */
   { "getPlayer", ai_getplayer },
   { "target", ai_gettarget },
   { "rndpilot", ai_getrndpilot },
   { "armour", ai_armour },
   { "shield", ai_shield },
   { "parmour", ai_parmour },
   { "pshield", ai_pshield },
   { "dist", ai_getdistance },
   { "pos", ai_getpos },
   { "minbrakedist", ai_minbrakedist },
   { "cargofree", ai_cargofree },
   { "shipclass", ai_shipclass },
   /* movement */
   { "nearestplanet", ai_getnearestplanet },
   { "rndplanet", ai_getrndplanet },
   { "landplanet", ai_getlandplanet },
   { "accel", ai_accel },
   { "turn", ai_turn },
   { "face", ai_face },
   { "brake", ai_brake },
   { "stop", ai_stop },
   { "hyperspace", ai_hyperspace },
   { "relvel", ai_relvel },
   /* escorts */
   { "e_attack", ai_e_attack },
   { "e_hold", ai_e_hold },
   { "e_clear", ai_e_clear },
   { "e_return", ai_e_return },
   { "dock", ai_dock },
   /* combat */
   { "aim", ai_aim },
   { "combat", ai_combat },
   { "settarget", ai_settarget },
   { "secondary", ai_secondary },
   { "hasturrets", ai_hasturrets },
   { "shoot", ai_shoot },
   { "getenemy", ai_getenemy },
   { "hostile", ai_hostile },
   { "getweaprange", ai_getweaprange },
   /* timers */
   { "settimer", ai_settimer },
   { "timeup", ai_timeup },
   /* messages */
   { "comm", ai_comm },
   { "broadcast", ai_broadcast },
   /* loot */
   { "setcredits", ai_credits },
   { "setcargo", ai_cargo },
   { "shipprice", ai_shipprice },
   {0,0} /* end */
};



/*
 * current pilot "thinking" and assorted variables
 */
static Pilot *cur_pilot = NULL; /**< Current pilot.  All functions use this. */
static double pilot_acc = 0.; /**< Current pilot's acceleration. */
static double pilot_turn = 0.; /**< Current pilot's turning. */
static int pilot_flags = 0; /**< Handle stuff like weapon firing. */

/*
 * ai status, used so that create functions can't be used elsewhere
 */
#define AI_STATUS_NORMAL      1 /**< Normal ai function behaviour. */
#define AI_STATUS_CREATE      2 /**< AI is running create function. */
static int ai_status = AI_STATUS_NORMAL; /**< Current AI run status. */


/**
 * @fn static void ai_setMemory (void)
 *
 * @brief Sets the cur_pilot's ai.
 */
static void ai_setMemory (void)
{
   lua_State *L;
   L = cur_pilot->ai->L;

   lua_getglobal(L, "pilotmem");
   lua_pushnumber(L, cur_pilot->id);
   lua_gettable(L, -2);
   lua_setglobal(L, "mem");
}


/**
 * @fn void ai_setPilot( Pilot *p )
 *
 * @brief Sets the pilot for furthur AI calls.
 */
void ai_setPilot( Pilot *p )
{
   cur_pilot = p;
   ai_setMemory();
}


/**
 * @fn static void ai_run( lua_State *L, const char *funcname )
 *
 * @brief Attempts to run a function.
 *
 *    @param[in] L Lua state to run function on.
 *    @param[in] funcname Function to run.
 */
static void ai_run( lua_State *L, const char *funcname )
{
   lua_getglobal(L, funcname);
   if (lua_pcall(L, 0, 0, 0)) /* error has occured */
      WARN("Pilot '%s' ai -> '%s': %s", cur_pilot->name, funcname, lua_tostring(L,-1));
}


/**
 * @fn int ai_pinit( Pilot *p, char *ai )
 *
 * @brief Initializes the pilot in the ai.
 *
 * Mainly used to create the pilot's memory table.
 *
 *    @param p Pilot to initialize in AI.
 *    @param ai AI to initialize pilot.
 */
int ai_pinit( Pilot *p, char *ai )
{
   int i, n;
   AI_Profile *prof;
   lua_State *L;
   char buf[PATH_MAX], param[PATH_MAX];

   /* Split parameter from ai itself. */
   n = 0;
   for (i=0; ai[i] != '\0'; i++) {
      /* Overflow protection. */
      if (i > PATH_MAX)
         break;

      /* Check to see if we find the splitter. */
      if (ai[i] == '*') {
         buf[i] = '\0';
         n = i+1;
         continue;
      }

      if (n==0)
         buf[i] = ai[i];
      else
         param[i-n] = ai[i];
   }
   if (n!=0) param[i-n] = '\0'; /* Terminate string if needed. */
   else buf[i] = '\0';

   prof = ai_getProfile(buf);
   p->ai = prof;
   L = p->ai->L;

   /* Adds a new pilot memory in the memory table. */
   lua_getglobal(L, "pilotmem");
   lua_pushnumber(L, p->id);
   lua_newtable(L);
   lua_settable(L,-3);

   /* Create the pilot. */
   ai_create( p, (n!=0) ? param : NULL );

   return 0;
}


/**
 * @fn void ai_destroy( Pilot* p )
 *
 * @brief Destroys the ai part of the pilot
 *
 *    @param[in] p Pilot to destroy it's AI part.
 */
void ai_destroy( Pilot* p )
{
   lua_State *L;
   L = p->ai->L;

   /* Get rid of pilot's memory. */
   lua_getglobal(L, "pilotmem");
   lua_pushnumber(L, p->id);
   lua_pushnil(L);
   lua_settable(L,-3);

   /* Clean up tasks. */
   if (p->task)
      ai_freetask( p->task );
   p->task = NULL;
}


/**
 * @fn int ai_init (void)
 *
 * @brief Initializes the AI stuff which is basically Lua.
 *
 *    @return 0 on no errors.
 */
int ai_init (void)
{
   char** files;
   uint32_t nfiles,i;

   /* get the file list */
   files = pack_listfiles( data, &nfiles );

   /* load the profiles */
   for (i=0; i<nfiles; i++)
      if ((strncmp( files[i], AI_PREFIX, strlen(AI_PREFIX))==0) && /* prefixed */
            (strncmp( files[i] + strlen(AI_PREFIX), AI_INCLUDE, /* not an include */
               strlen(AI_INCLUDE))!=0) &&
            (strncmp( files[i] + strlen(files[i]) - strlen(AI_SUFFIX), /* suffixed */
               AI_SUFFIX, strlen(AI_SUFFIX))==0))
         if (ai_loadProfile(files[i])) /* Load the profile */
            WARN("Error loading AI profile '%s'", files[i]);

   /* free the char* allocated by pack */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   DEBUG("Loaded %d AI Profile%c", nprofiles, (nprofiles==1)?' ':'s');

   return 0;
}


/*
 * @fn static int ai_loadProfile( char* filename )
 * 
 * @brief Initializes an AI_Profile and adds it to the stack.
 *
 *    @param[in] filename File to create the profile from.
 *    @return 0 on no error.
 */
static int ai_loadProfile( char* filename )
{
   char* buf = NULL;
   uint32_t bufsize = 0;
   lua_State *L;

   profiles = realloc( profiles, sizeof(AI_Profile)*(++nprofiles) );

   profiles[nprofiles-1].name =
      malloc(sizeof(char)*(strlen(filename)-strlen(AI_PREFIX)-strlen(AI_SUFFIX))+1 );
   snprintf( profiles[nprofiles-1].name,
         strlen(filename)-strlen(AI_PREFIX)-strlen(AI_SUFFIX)+1,
         "%s", filename+strlen(AI_PREFIX) );

   profiles[nprofiles-1].L = nlua_newState();

   if (profiles[nprofiles-1].L == NULL) {
      ERR("Unable to create a new Lua state");
      return -1;
   }

   L = profiles[nprofiles-1].L;

   /* open basic lua stuff */
   nlua_loadBasic(L);

   /* constants */
   lua_regnumber(L, "player", PLAYER_ID); /* player ID */

   /* Register C functions in Lua */
   luaL_register(L, "ai", ai_methods);
   lua_loadRnd(L);

   /* Metatables to register. */
   lua_loadVector(L);

   /* now load the file since all the functions have been previously loaded */
   buf = pack_readfile( DATA, filename, &bufsize );
   if (luaL_dobuffer(L, buf, bufsize, filename) != 0) {
      ERR("Error loading AI file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check",
            filename, lua_tostring(L,-1));
      return -1;
   }
   free(buf);

   /* Add the player memory table. */
   lua_newtable(L);
   lua_setglobal(L, "pilotmem");

   return 0;
}


/**
 * @fn AI_Profile* ai_getProfile( char* name )
 *
 * @brief Gets the AI_Profile by name.
 *
 *    @param[in] name Name of the profile to get.
 *    @return The profile or NULL on error.
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


/**
 * @fn void ai_exit (void)
 *
 * @brief Cleans up global AI.
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


/**
 * @fn void ai_think( Pilot* pilot )
 *
 * @brief Heart of the AI, brains of the pilot.
 *
 *    @param pilot Pilot that needs to think.
 */
void ai_think( Pilot* pilot )
{
   lua_State *L;

   ai_setPilot(pilot);
   L = cur_pilot->ai->L; /* set the AI profile to the current pilot's */

   /* clean up some variables */
   pilot_acc = 0;
   pilot_turn = 0.;
   pilot_flags = 0;
   cur_pilot->target = cur_pilot->id;

   
   /* control function if pilot is idle or tick is up */
   if ((cur_pilot->tcontrol < SDL_GetTicks()) || (cur_pilot->task == NULL)) {
      ai_run(L, "control"); /* run control */
      lua_getglobal(L,"control_rate");
      cur_pilot->tcontrol = SDL_GetTicks() +  (int)(1000.*lua_tonumber(L,-1));
   }

   /* pilot has a currently running task */
   if (cur_pilot->task)
      ai_run(L, cur_pilot->task->name);

   /* make sure pilot_acc and pilot_turn are legal */
   if (pilot_acc > 1.) pilot_acc = 1.; /* value must be <= 1 */
   if (pilot_turn > 1.) pilot_turn = 1.; /* value must between -1 and 1 */
   else if (pilot_turn < -1.) pilot_turn = -1.;

   cur_pilot->solid->dir_vel = 0.;
   if (pilot_turn) /* set the turning velocity */
      cur_pilot->solid->dir_vel += cur_pilot->turn * pilot_turn;
   vect_pset( &cur_pilot->solid->force, /* set the velocity vector */
         cur_pilot->thrust * pilot_acc, cur_pilot->solid->dir );

   /* fire weapons if needed */
   if (ai_isFlag(AI_PRIMARY)) pilot_shoot(cur_pilot, 0); /* primary */
   if (ai_isFlag(AI_SECONDARY)) pilot_shoot(cur_pilot, 1); /* secondary */
}


/**
 * @fn void ai_attacked( Pilot* attacked, const unsigned int attacker )
 *
 * @brief Triggers the attacked() function in the pilot's AI.
 *
 *    @param attacked Pilot that is attacked.
 *    @param[in] attacker ID of the attacker.
 */
void ai_attacked( Pilot* attacked, const unsigned int attacker )
{
   lua_State *L;

   ai_setPilot(attacked);
   L = cur_pilot->ai->L;
   lua_getglobal(L, "attacked");
   lua_pushnumber(L, attacker);
   if (lua_pcall(L, 1, 0, 0))
      WARN("Pilot '%s' ai -> 'attacked': %s", cur_pilot->name, lua_tostring(L,-1));
}


/**
 * @fn static void ai_create( Pilot* pilot, char *param )
 *
 * @brief Runs the create() function in the pilot.
 *
 * Should create all the gear and sucth the pilot has.
 *
 *    @param pilot Pilot to "create".
 *    @param param Parameter to pass to "create" function.
 */
static void ai_create( Pilot* pilot, char *param )
{
   lua_State *L;

   ai_setPilot( pilot );
   L = cur_pilot->ai->L;

   /* Set creation mode. */
   ai_status = AI_STATUS_CREATE;

   /* Prepare stack. */
   lua_getglobal(L, "create");

   /* Parse parameter. */
   if (param != NULL) {
      /* Number */
      if (isdigit(param[0]))
         lua_pushnumber(L, atoi(param));
      /* Special case player. */
      else if (strcmp(param,"player")==0)
         lua_pushnumber(L, PLAYER_ID);
      /* Default. */
      else
         lua_pushstring(L, param);
   }

   /* Run function. */
   if (lua_pcall(L, (param!=NULL) ? 1 : 0, 0, 0)) /* error has occured */
      WARN("Pilot '%s' ai -> '%s': %s", cur_pilot->name, "create", lua_tostring(L,-1));

   /* Recover normal mode. */
   ai_status = AI_STATUS_NORMAL;
}


/*
 * internal use C functions
 */
/**
 * @fn static void ai_freetask( Task* t )
 *
 * @brief Frees an AI task.
 *
 *    @param t Task to free.
 */
static void ai_freetask( Task* t )
{
   if (t->next != NULL) {
      ai_freetask(t->next); /* yay recursive freeing */
      t->next = NULL;
   }

   if (t->name) free(t->name);
   free(t);
}


/**
 * @group AI Lua AI Bindings
 *
 * @brief Handles how the AI interacts with the universe.
 *
 * Usage is:
 * @code
 * ai.function( params )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int ai_pushtask( lua_State *L )
 *
 * @brief pushtask( number pos, string func [, data] )
 *
 *    @param pos Position to push into stack, 0 is front, 1 is back.
 *    @param func Function to call for task.
 *    @param data Data to pass to the function.  Only lightuserdata or number
 *           is currently supported.
 */
static int ai_pushtask( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int pos;
   char *func;
   Task *t, *pointer;

   /* Parse basic parameters. */
   if (lua_isnumber(L,1)) pos = (int)lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) func = (char*)lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   t = MALLOC_ONE(Task);
   t->next = NULL;
   t->name = strdup(func);
   t->dtype = TYPE_NULL;

   if (lua_gettop(L) > 2) {
      if (lua_isnumber(L,3)) {
         t->dtype = TYPE_INT;
         t->dat.num = (unsigned int)lua_tonumber(L,3);
      }
      else NLUA_INVALID_PARAMETER();
   }

   if (pos == 1) { /* put at the end */
      for (pointer = cur_pilot->task; pointer->next != NULL; pointer = pointer->next);
      pointer->next = t;
   }
   else { /* default put at the beginning */
      t->next = cur_pilot->task;
      cur_pilot->task = t;
   }

   return 0;
}
/**
 * @fn static int ai_poptask( lua_State *L )
 *
 * @brief poptask( nil )
 *
 * Pops the current running task.
 */
static int ai_poptask( lua_State *L )
{
   (void)L; /* hack to avoid -W -Wall warnings */
   Task* t = cur_pilot->task;

   /* Tasks must exist. */
   if (t == NULL) {
      NLUA_DEBUG("Trying to pop task when there are no tasks on the stack.");
      return 0;
   }

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
 * Gets the player.
 */
static int ai_getplayer( lua_State *L )
{
   lua_pushnumber(L, PLAYER_ID);
   return 1;
}

/*
 * gets the target
 */
static int ai_gettarget( lua_State *L )
{
   switch (cur_pilot->task->dtype) {
      case TYPE_INT:
         lua_pushnumber(L, cur_pilot->task->dat.num);
         return 1;

      default:
         return 0;
   }
}

/*
 * Gets a random target's ID
 */
static int ai_getrndpilot( lua_State *L )
{
   lua_pushnumber(L, pilot_stack[ RNG(0, pilot_nstack-1) ]->id );
   return 1;
}

/*
 * gets the pilot's armour
 */
static int ai_armour( lua_State *L )
{
   double d;

   if (lua_isnumber(L,1)) d = pilot_get((unsigned int)lua_tonumber(L,1))->armour;
   else d = cur_pilot->armour;

   lua_pushnumber(L, d);
   return 1;
}

/*
 * gets the pilot's shield
 */
static int ai_shield( lua_State *L )
{
   double d;

   if (lua_isnumber(L,1)) d = pilot_get((unsigned int)lua_tonumber(L,1))->shield;
   else d = cur_pilot->shield;

   lua_pushnumber(L, d);
   return 1;
}

/*
 * gets the pilot's armour in percent
 */
static int ai_parmour( lua_State *L )
{
   double d;
   Pilot* p;

   if (lua_isnumber(L,1)) {
      p = pilot_get((unsigned int)lua_tonumber(L,1));
      d = p->armour / p->armour_max * 100.;
   }
   else d = cur_pilot->armour / cur_pilot->armour_max * 100.;

   lua_pushnumber(L, d);
   return 1;
}

/* 
 * gets the pilot's shield in percent
 */              
static int ai_pshield( lua_State *L )
{
   double d;
   Pilot* p;

   if (lua_isnumber(L,1)) {
      p = pilot_get((unsigned int)lua_tonumber(L,1));
      d = p->shield / p->shield_max * 100.;
   }
   else d = cur_pilot->shield / cur_pilot->shield_max * 100.;

   lua_pushnumber(L, d);
   return 1;
} 

/*
 * gets the distance from the pointer
 */
static int ai_getdistance( lua_State *L )
{
   Vector2d *v;
   LuaVector *lv;
   Pilot *pilot;
   unsigned int n;

   NLUA_MIN_ARGS(1);

   /* vector as a parameter */
   if (lua_isvector(L,1)) {
      lv = lua_tovector(L,1);
      v = &lv->vec;
   }

   else if (lua_islightuserdata(L,1))
      v = lua_touserdata(L,1);
   
   /* pilot id as parameter */
   else if (lua_isnumber(L,1)) {
      n = (unsigned int) lua_tonumber(L,1);
      pilot = pilot_get( (unsigned int) lua_tonumber(L,1) );
      if (pilot==NULL) { 
         NLUA_DEBUG("Pilot '%d' not found in stack", n );
         return 0;
      }
      v = &pilot->solid->pos;
   }
   
   /* wrong parameter */
   else
      NLUA_INVALID_PARAMETER();

   lua_pushnumber(L, vect_dist(v, &cur_pilot->solid->pos));
   return 1;
}

/*
 * gets the pilot's position
 */
static int ai_getpos( lua_State *L )
{
   Pilot *p;

   if (lua_isnumber(L,1)) {
      p = pilot_get((int)lua_tonumber(L,1)); /* Pilot ID */
      if (p==NULL) return 0;
   }
   else p = cur_pilot; /* default to self */

   lua_pushlightuserdata(L, &p->solid->pos );

   return 1;
}

/*
 * gets the minimum braking distance
 *
 * braking vel ==> 0 = v - a*dt
 * add turn around time (to inital vel) ==> 180.*360./cur_pilot->turn
 * add it to general euler equation  x = v * t + 0.5 * a * t^2
 * and voila!
 *
 * I hate this function and it'll probably need to get changed in the future
 */
static int ai_minbrakedist( lua_State *L )
{
   double time, dist, vel;
   time = VMOD(cur_pilot->solid->vel) /
         (cur_pilot->thrust / cur_pilot->solid->mass);

   vel = MIN(cur_pilot->speed,VMOD(cur_pilot->solid->vel));
   dist = vel*(time+1.1*180./cur_pilot->turn) -
         0.5*(cur_pilot->thrust/cur_pilot->solid->mass)*time*time;

   lua_pushnumber(L, dist); /* return */
   return 1; /* returns one thing */
}

/*
 * gets the pilot's free cargo space
 */
static int ai_cargofree( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(cur_pilot));
   return 1;
}


static int ai_shipclass( lua_State *L )
{
   lua_pushstring(L, ship_class(cur_pilot->ship));
   return 1;
}


/*
 * pilot exists?
 */
static int ai_exists( lua_State *L )
{
   Pilot *p;
   int i;

   if (lua_isnumber(L,1)) {
      i = 1;
      p = pilot_get((unsigned int)lua_tonumber(L,1));
      if (p==NULL) i = 0;
      else if (pilot_isFlag(p,PILOT_DEAD)) i = 0;
      lua_pushboolean(L, i );
      return 1;
   }

   /* Default to false for everything that isn't a pilot */
   lua_pushboolean(L, 0);
   return 0;
}


/*
 * is at maximum velocity?
 */
static int ai_ismaxvel( lua_State *L )
{
   lua_pushboolean(L,(VMOD(cur_pilot->solid->vel) > cur_pilot->speed-MIN_VEL_ERR));
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
 * checks to see if the pilot is in combat, defaults to self
 */
static int ai_incombat( lua_State *L )
{
   Pilot* p;

   if (lua_isnumber(L,1)) p = pilot_get((unsigned int)lua_tonumber(L,1));
   else p = cur_pilot;

   lua_pushboolean(L, pilot_isFlag(p, PILOT_COMBAT));
   return 1;
}


/*
 * pilot has is locked by some missile
 */
static int ai_haslockon( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->lockons > 0);
   return 1;
}


/*
 * starts accelerating the pilot based on a parameter
 */
static int ai_accel( lua_State *L )
{
   double n;

   if (lua_gettop(L) > 1 && lua_isnumber(L,1)) {
      n = (double)lua_tonumber(L,1);

      if (n > 1.) n = 1.;
      else if (n < 0.) n = 0.;

      if (VMOD(cur_pilot->solid->vel) > (n * cur_pilot->speed))
         pilot_acc = 0.;
   }
   else
      pilot_acc = 1.;

   return 0;
}


/*
 * starts turning the pilot based on a parameter
 */
static int ai_turn( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   pilot_turn = (lua_isnumber(L,1)) ? (double)lua_tonumber(L,1) : 0. ;
   return 0;
}


/*
 * faces the target
 */
static int ai_face( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaVector *lv;
   Vector2d sv, tv; /* get the position to face */
   Pilot* p;
   double mod, diff;
   int n;

   /* Get first parameter, aka what to face. */
   n = -2;
   if (lua_isnumber(L,1)) {
      n = (int)lua_tonumber(L,1);
      if (n >= 0 ) {
         p = pilot_get(n);
         if (p==NULL)
            return 0; /* make sure pilot is valid */
         vect_cset( &tv, VX(p->solid->pos), VY(p->solid->pos) );
         lv = NULL;
      }
   }
   else if (lua_isvector(L,1))
      lv = lua_tovector(L,1);

   mod = 10;

   /* Check if must invert. */
   if (lua_gettop(L) > 1) {
      if (lua_isboolean(L,2) && lua_toboolean(L,2))
         mod *= -1;
   }

   vect_cset( &sv, VX(cur_pilot->solid->pos), VY(cur_pilot->solid->pos) );

   if (lv==NULL) /* target is dynamic */
      diff = angle_diff(cur_pilot->solid->dir,
            (n==-1) ? VANGLE(sv) :
            vect_angle(&sv, &tv));
   else /* target is static */
      diff = angle_diff( cur_pilot->solid->dir,   
            (n==-1) ? VANGLE(cur_pilot->solid->pos) :
            vect_angle(&cur_pilot->solid->pos, &lv->vec));

   /* Make pilot turn. */
   pilot_turn = mod*diff;

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, ABS(diff*180./M_PI));
   return 1;
}

/*
 * brakes the pilot
 */
static int ai_brake( lua_State *L )
{
   (void)L; /* hack to avoid -W -Wall warnings */
   double diff, d;

   d = cur_pilot->solid->dir+M_PI;
   if (d >= 2*M_PI) d = fmodf(d, 2*M_PI);
   
   diff = angle_diff(d,VANGLE(cur_pilot->solid->vel));
   pilot_turn = 10*diff;

   if (ABS(diff) < MAX_DIR_ERR && VMOD(cur_pilot->solid->vel) > MIN_VEL_ERR)
      pilot_acc = 1.;

   return 0;
}

/*
 * returns the nearest friendly planet's position to the pilot
 */
static int ai_getnearestplanet( lua_State *L )
{
   double dist, d;
   int i, j;
   LuaVector lv;

   if (cur_system->nplanets == 0) return 0; /* no planets */

   /* cycle through planets */
   for (dist=0., j=-1, i=0; i<cur_system->nplanets; i++) {
      d = vect_dist( &cur_system->planets[i]->pos, &cur_pilot->solid->pos );
      if ((!areEnemies(cur_pilot->faction,cur_system->planets[i]->faction)) &&
            (d < dist)) { /* closer friendly planet */
         j = i;
         dist = d;
      }
   }

   /* no friendly planet found */
   if (j == -1) return 0;

   vectcpy( &lv.vec, &cur_system->planets[j]->pos );
   lua_pushvector(L, lv);

   return 1;
}


/*
 * returns a random planet's position to the pilot
 */
static int ai_getrndplanet( lua_State *L )
{
   LuaVector lv;
   int p;

   if (cur_system->nplanets == 0) return 0; /* no planets */

   /* get a random planet */
   p = RNG(0, cur_system->nplanets-1);

   /* Copy the data into a vector */
   vectcpy( &lv.vec, &cur_system->planets[p]->pos );
   lua_pushvector(L, lv);

   return 1;
}

/*
 * returns a random friendly planet's position to the pilot
 */
static int ai_getlandplanet( lua_State *L )
{
   Planet** planets;
   int nplanets, i;
   LuaVector lv;
   planets = malloc( sizeof(Planet*) * cur_system->nplanets );

   if (cur_system->nplanets == 0) return 0; /* no planets */

   for (nplanets=0, i=0; i<cur_system->nplanets; i++)
      if (planet_hasService(cur_system->planets[i],PLANET_SERVICE_BASIC) &&
            !areEnemies(cur_pilot->faction,cur_system->planets[i]->faction))
         planets[nplanets++] = cur_system->planets[i];

   /* no planet to land on found */
   if (nplanets==0) {
      free(planets);
      return 0;
   }

   /* we can actually get a random planet now */
   i = RNG(0,nplanets-1);
   vectcpy( &lv.vec, &planets[i]->pos );
   vect_cadd( &lv.vec, RNG(0, planets[i]->gfx_space->sw)-planets[i]->gfx_space->sw/2.,
         RNG(0, planets[i]->gfx_space->sh)-planets[i]->gfx_space->sh/2. );
   lua_pushvector( L, lv );
   free(planets);
   return 1;
}

/*
 * tries to enter the pilot in hyperspace, returns the distance if too far away
 */
static int ai_hyperspace( lua_State *L )
{
   int dist;
   
   pilot_shootStop( cur_pilot, 0 );
   pilot_shootStop( cur_pilot, 1 );
   dist = space_hyperspace(cur_pilot);
   if (dist == 0.) return 0;

   lua_pushnumber(L,dist);
   return 1;
}

/*
 * Gets the relative velocity of a pilot.
 */
static int ai_relvel( lua_State *L )
{
   unsigned int id;
   Pilot *p;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER()

   p = pilot_get(id);
   if (p==NULL) {
      NLUA_DEBUG("Invalid pilot identifier.");
      return 0;
   }

   lua_pushnumber(L, vect_dist( &cur_pilot->solid->vel, &p->solid->vel ));
   return 1;
}

/*
 * completely stops the pilot if it is below minimum vel error (no instastops)
 */
static int ai_stop( lua_State *L )
{
   (void) L; /* avoid gcc warning */

   if (VMOD(cur_pilot->solid->vel) < MIN_VEL_ERR)
      vect_pset( &cur_pilot->solid->vel, 0., 0. );

   return 0;
}

/*
 * Tells the pilot's escort's to attack it's target.
 */
static int ai_e_attack( lua_State *L )
{
   int ret;
   ret = escorts_attack(cur_pilot);
   lua_pushboolean(L,!ret);
   return 1;
}

/* 
 * Tells the pilot's escorts to hold position.
 */
static int ai_e_hold( lua_State *L )
{
   int ret;
   ret = escorts_hold(cur_pilot);
   lua_pushboolean(L,!ret);
   return 1;
}

/*
 * Tells the pilot's escorts to clear orders.
 */
static int ai_e_clear( lua_State *L )
{
   int ret;
   ret = escorts_clear(cur_pilot);
   lua_pushboolean(L,!ret);
   return 1;
}

/*
 * Tells the pilot's escorts to return to dock.
 */
static int ai_e_return( lua_State *L )
{
   int ret;
   ret = escorts_return(cur_pilot);
   lua_pushboolean(L,!ret);
   return 1;
}

/*
 * Docks the ship.
 */
static int ai_dock( lua_State *L )
{
   Pilot *p;

   /* Target is another ship. */
   if (lua_isnumber(L,1)) {
      p = pilot_get(lua_tonumber(L,1));
      if (p==NULL) return 0;
      pilot_dock(cur_pilot, p);
   }
   else NLUA_INVALID_PARAMETER();

   return 0;
}


/*
 * Aims at the pilot, trying to hit it.
 */
static int ai_aim( lua_State *L )
{
   int id;
   double x,y;
   double t;
   Pilot *p;
   Vector2d tv;
   double dist, diff;
   double mod;
   NLUA_MIN_ARGS(1);

   /* Only acceptable parameter is pilot id */
   if (lua_isnumber(L,1))
      id = lua_tonumber(L,1);
   else
      NLUA_INVALID_PARAMETER();

   /* Get pilot */
   p = pilot_get(id);
   if (p==NULL) {
      WARN("Pilot invalid");
      return 0;
   }

   /* Get the distance */
   dist = vect_dist( &cur_pilot->solid->pos, &p->solid->pos );

   /* Time for shots to reach that distance */
   t = dist / cur_pilot->weap_speed;

   /* Position is calculated on where it should be */
   x = p->solid->pos.x + p->solid->vel.x*t
         - (cur_pilot->solid->pos.x + cur_pilot->solid->vel.x*t);
   y = p->solid->pos.y + p->solid->vel.y*t
      - (cur_pilot->solid->pos.y + cur_pilot->solid->vel.y*t);
   vect_cset( &tv, x, y );

   /* Calculate what we need to turn */
   mod = 10.;
   diff = angle_diff(cur_pilot->solid->dir, VANGLE(tv));
   pilot_turn = mod * diff;

   /* Return distance to target (in grad) */
   lua_pushnumber(L, ABS(diff*180./M_PI));
   return 1;
}


/*
 * toggles the combat flag, default is on
 */
static int ai_combat( lua_State *L )
{
   int i;

   if (lua_isnumber(L,1)) {
      i = (int)lua_tonumber(L,1);
      if (i==1) pilot_setFlag(cur_pilot, PILOT_COMBAT);
      else if (i==0) pilot_rmFlag(cur_pilot, PILOT_COMBAT);
   }
   else pilot_setFlag(cur_pilot, PILOT_COMBAT);

   return 0;
}


/*
 * sets the pilot's target
 */
static int ai_settarget( lua_State *L )
{
   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) {
      cur_pilot->target = (int)lua_tonumber(L,1);
      return 1;
   }
   NLUA_INVALID_PARAMETER();
}


static int outfit_isMelee( Pilot *p, PilotOutfit *o )
{
   (void) p;
   if (outfit_isBolt(o->outfit) || outfit_isBeam(o->outfit))
      return 1;
   return 0;
}
static int outfit_isRanged( Pilot *p, PilotOutfit *o )
{
   if (outfit_isFighterBay(o->outfit) || outfit_isLauncher(o->outfit)) {
      /* Must have ammo. */
      if (pilot_getAmmo( p, o->outfit ) <= 0)
         return 0;
      return 1;
   }
   return 0;
}
/*
 * sets the secondary weapon, biased towards launchers
 */
static int ai_secondary( lua_State *L )
{
   PilotOutfit *co, *po;
   int i, melee;
   char *str;
   const char *otype;

   po = NULL;

   /* Parse the parameters. */
   if (lua_isstring(L,1)) {
      str = (char*) lua_tostring(L, 1);
      if (strcmp(str, "melee")==0)
         melee = 1;
      else if (strcmp(str, "ranged")==0)
         melee = 0;
      else NLUA_INVALID_PARAMETER();
   }
   else NLUA_INVALID_PARAMETER();

   /* Pilot has secondary selected - use that */
   if (cur_pilot->secondary != NULL) {
      co = cur_pilot->secondary;
      if (melee && outfit_isMelee(cur_pilot,co))
         po = co;
      else if (!melee && outfit_isRanged(cur_pilot,co))
         po = co;
   }

   /* Need to get new secondary */
   if (po==NULL)  {

      /* Iterate over the list */
      po = NULL;
      for (i=0; i<cur_pilot->noutfits; i++) {
         co = &cur_pilot->outfits[i];

         /* Not a secondary weapon. */
         if (!outfit_isProp(co->outfit, OUTFIT_PROP_WEAP_SECONDARY) ||
               outfit_isAmmo(co->outfit))
            continue;

         /* Get the first match. */
         if (melee && outfit_isMelee(cur_pilot,co)) {
            po = co;
            break;
         }
         else if (!melee && outfit_isRanged(cur_pilot,co)) {
            po = co;
            break;
         }
      }
   }

   /* Check to see if we have a good secondary weapon. */
   if (po != NULL) {
      cur_pilot->secondary = po;
      pilot_setAmmo(cur_pilot);
      otype = outfit_getTypeBroad(po->outfit);
      lua_pushstring( L, otype );

      /* Set special flags */
      if (outfit_isLauncher(po->outfit)) {
         if ((po->outfit->type != OUTFIT_TYPE_MISSILE_DUMB))
            lua_pushstring( L, "Smart" );
         else
            lua_pushstring( L, "Dumb" );

         if (cur_pilot->ammo == NULL)
            lua_pushnumber( L, 0. );
         else
            lua_pushnumber( L, cur_pilot->ammo->quantity );
         return 3;
      }
      else if (outfit_isTurret(po->outfit)) {
         lua_pushstring( L, "Turret" );
         return 2;
      }

      return 1;
   }

   /* Nothing found */
   return 0;
}


/*
 * returns true if the pilot has turrets
 */
static int ai_hasturrets( lua_State *L )
{
   lua_pushboolean( L, pilot_isFlag(cur_pilot, PILOT_HASTURRET) );
   return 1;
}


/*
 * makes the pilot shoot
 */
static int ai_shoot( lua_State *L )
{
   int n = 1;
   if (lua_isnumber(L,1)) n = (int)lua_tonumber(L,1);

   if (n==1) ai_setFlag(AI_PRIMARY);
   else if (n==2) ai_setFlag(AI_SECONDARY);
   else if  (n==3) ai_setFlag(AI_PRIMARY | AI_SECONDARY );

   return 0;
}


/*
 * gets the nearest enemy
 */
static int ai_getenemy( lua_State *L )
{
   unsigned int p;

   p = pilot_getNearestEnemy(cur_pilot);

   if (p==0) /* No enemy found */
      return 0;

   lua_pushnumber(L,p);
   return 1;
}


/*
 * sets the enemy hostile (basically notifies of an impending attack)
 */
static int ai_hostile( lua_State *L )
{
   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1) && ((unsigned int)lua_tonumber(L,1) == PLAYER_ID))
      pilot_setFlag(cur_pilot, PILOT_HOSTILE);

   return 0;
}


/*
 * returns the maximum range of weapons, if parameter is 1, then does secondary
 */
static int ai_getweaprange( lua_State *L )
{
   double range;

   /* if 1 is passed as a parameter, secondary weapon is checked */
   if (lua_isnumber(L,1) && ((int)lua_tonumber(L,1) == 1))
      if (cur_pilot->secondary != NULL) {
         /* get range, launchers use ammo's range */
         if (outfit_isLauncher(cur_pilot->secondary->outfit) && (cur_pilot->ammo != NULL))
            range = outfit_range(cur_pilot->ammo->outfit);
         else
            range = outfit_range(cur_pilot->secondary->outfit);

         if (range < 0.) {
            lua_pushnumber(L, 0.); /* secondary doesn't have range */
            return 1;
         }

         /* secondary does have range */
         lua_pushnumber(L, range);
         return 1;
      }

   lua_pushnumber(L,cur_pilot->weap_range);
   return 1;
}


/*
 * sets the timer
 */
static int ai_settimer( lua_State *L )
{
   NLUA_MIN_ARGS(2);

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
   NLUA_MIN_ARGS(1);

   int n; /* get the timer */
   if (lua_isnumber(L,1)) n = lua_tonumber(L,1);

   lua_pushboolean(L, cur_pilot->timer[n] < SDL_GetTicks());
   return 1;
}


/*
 * makes the pilot say something to the player
 */
static int ai_comm( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   
   if (lua_isnumber(L,1) && (lua_tonumber(L,1)==PLAYER_ID) && lua_isstring(L,2))
      player_message( "Comm %s> \"%s\"", cur_pilot->name, lua_tostring(L,2));

   return 0;
}

/*
 * broadcasts to the entire area
 */
static int ai_broadcast( lua_State *L )
{
   NLUA_MIN_ARGS(1);

   if (lua_isstring(L,1))
      player_message( "Broadcast %s> \"%s\"", cur_pilot->name, lua_tostring(L,1));

   return 0;
}


/*
 * sets the pilot_nstack credits
 */
static int ai_credits( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (ai_status != AI_STATUS_CREATE) return 0;

   if (lua_isnumber(L,1))
      cur_pilot->credits = (int)lua_tonumber(L,1);
   
   return 0;
}


/*
 * sets the pilot_nstack cargo
 */
static int ai_cargo( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   if (ai_status != AI_STATUS_CREATE) return 0;

   if (lua_isstring(L,1) && lua_isnumber(L,2))
      pilot_addCargo( cur_pilot, commodity_get(lua_tostring(L,1)),
            (int)lua_tonumber(L,2));

   return 0;
}


/*
 * gets the pilot's ship value
 */
static int ai_shipprice( lua_State *L )
{
   lua_pushnumber(L, cur_pilot->ship->price);
   return 1;
}

/**
 * @}
 */
