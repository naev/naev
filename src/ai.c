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
 *  AI uses the goal (task) based AI approach with tasks scripted in Lua,
 * additionally there is a task that is hard-coded and obligatory in any AI
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
 *     - Tasks can have subtasks which will be closed when parent task is dead.
 *     -  "control" task is a special task that MUST exist in any given  Pilot AI
 *        (missiles and such will use "seek")
 *     - "control" task is not permanent, but transitory
 *     - "control" task sets another task
 *   - "control" task is also run at a set rate (depending on Lua global "control_rate")
 *     to choose optimal behaviour (task)
 *
 * Memory
 *
 *  The AI currently has per-pilot memory which is accessible as "mem".  This
 * memory is actually stored in the table pilotmem[cur_pilot->id].  This allows
 * the pilot to keep some memory always accessible between runs without having
 * to rely on the storage space a task has.
 *
 * Garbage Collector
 *
 *  The tasks are not deleted directly but are marked for deletion and are then
 * cleaned up in a garbage collector. This is to avoid accessing invalid task
 * memory.
 *
 * @note Nothing in this file can be considered reentrant.  Plan accordingly.
 *
 * @todo Clean up most of the code, it was written as one of the first
 *         subsystems and is pretty lacking in quite a few aspects. Notably
 *         removing the entire lightuserdata thing and actually go with full
 *         userdata.
 */


#include "ai.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h> /* malloc realloc */
#include <math.h>
#include <ctype.h> /* isdigit */

/* yay more Lua */
#include <lauxlib.h>
#include <lualib.h>

#include "nstring.h" /* strncpy strlen strncat strcmp strdup */
#include "log.h"
#include "pilot.h"
#include "player.h"
#include "physics.h"
#include "ndata.h"
#include "rng.h"
#include "space.h"
#include "faction.h"
#include "escort.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_vec2.h"
#include "nlua_rnd.h"
#include "nlua_pilot.h"
#include "nlua_planet.h"
#include "nlua_faction.h"
#include "board.h"
#include "hook.h"
#include "array.h"


/*
 * ai flags
 *
 * They can be used for stuff like movement or for pieces of code which might
 *  run AI stuff when the AI module is not reentrant.
 */
#define ai_setFlag(f)   (pilot_flags |= f ) /**< Sets pilot flag f */
#define ai_isFlag(f)    (pilot_flags & f ) /**< Checks pilot flag f */
/* flags */
#define AI_PRIMARY      (1<<0)   /**< Firing primary weapon */
#define AI_SECONDARY    (1<<1)   /**< Firing secondary weapon */
#define AI_DISTRESS     (1<<2)   /**< Sent distress signal. */


/*
 * file info
 */
#define AI_SUFFIX       ".lua" /**< AI file suffix. */
#define AI_MEM_DEF      "def" /**< Default pilot memory. */


/*
 * all the AI profiles
 */
static AI_Profile* profiles = NULL; /**< Array of AI_Profiles loaded. */
static nlua_env equip_env = LUA_NOREF; /**< Equipment enviornment. */


/*
 * extern pilot hacks
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * prototypes
 */
/* Internal C routines */
static void ai_run( nlua_env env, const char *funcname );
static int ai_loadProfile( const char* filename );
static void ai_setMemory (void);
static void ai_create( Pilot* pilot );
static int ai_loadEquip (void);
/* Task management. */
static void ai_taskGC( Pilot* pilot );
static Task* ai_curTask( Pilot* pilot );
static Task* ai_createTask( lua_State *L, int subtask );
static int ai_tasktarget( lua_State *L, Task *t );



/*
 * AI routines for Lua
 */
/* tasks */
static int aiL_pushtask( lua_State *L ); /* pushtask( string, number/pointer ) */
static int aiL_poptask( lua_State *L ); /* poptask() */
static int aiL_taskname( lua_State *L ); /* string taskname() */
static int aiL_gettarget( lua_State *L ); /* pointer gettarget() */
static int aiL_pushsubtask( lua_State *L ); /* pushsubtask( string, number/pointer, number ) */
static int aiL_popsubtask( lua_State *L ); /* popsubtask() */
static int aiL_subtaskname( lua_State *L ); /* string subtaskname() */
static int aiL_getsubtarget( lua_State *L ); /* pointer subtarget() */

/* consult values */
static int aiL_pilot( lua_State *L ); /* number pilot() */
static int aiL_getrndpilot( lua_State *L ); /* number getrndpilot() */
static int aiL_getnearestpilot( lua_State *L ); /* number getnearestpilot() */
static int aiL_getdistance( lua_State *L ); /* number getdist(Vector2d) */
static int aiL_getflybydistance( lua_State *L ); /* number getflybydist(Vector2d) */
static int aiL_minbrakedist( lua_State *L ); /* number minbrakedist( [number] ) */
static int aiL_isbribed( lua_State *L ); /* bool isbribed( number ) */
static int aiL_getstanding( lua_State *L ); /* number getstanding( number ) */

/* boolean expressions */
static int aiL_ismaxvel( lua_State *L ); /* boolean ismaxvel() */
static int aiL_isstopped( lua_State *L ); /* boolean isstopped() */
static int aiL_isenemy( lua_State *L ); /* boolean isenemy( number ) */
static int aiL_isally( lua_State *L ); /* boolean isally( number ) */
static int aiL_haslockon( lua_State *L ); /* boolean haslockon() */

/* movement */
static int aiL_accel( lua_State *L ); /* accel(number); number <= 1. */
static int aiL_turn( lua_State *L ); /* turn(number); abs(number) <= 1. */
static int aiL_face( lua_State *L ); /* face( number/pointer, bool) */
static int aiL_careful_face( lua_State *L ); /* face( number/pointer, bool) */
static int aiL_aim( lua_State *L ); /* aim(number) */
static int aiL_iface( lua_State *L ); /* iface(number/pointer) */
static int aiL_dir( lua_State *L ); /* dir(number/pointer) */
static int aiL_idir( lua_State *L ); /* idir(number/pointer) */
static int aiL_drift_facing( lua_State *L ); /* drift_facing(number/pointer) */
static int aiL_brake( lua_State *L ); /* brake() */
static int aiL_getnearestplanet( lua_State *L ); /* Vec2 getnearestplanet() */
static int aiL_getrndplanet( lua_State *L ); /* Vec2 getrndplanet() */
static int aiL_getlandplanet( lua_State *L ); /* Vec2 getlandplanet() */
static int aiL_land( lua_State *L ); /* bool land() */
static int aiL_stop( lua_State *L ); /* stop() */
static int aiL_relvel( lua_State *L ); /* relvel( number ) */
static int aiL_follow_accurate( lua_State *L ); /* follow_accurate() */

/* Hyperspace. */
static int aiL_sethyptarget( lua_State *L );
static int aiL_nearhyptarget( lua_State *L ); /* pointer rndhyptarget() */
static int aiL_rndhyptarget( lua_State *L ); /* pointer rndhyptarget() */
static int aiL_hyperspace( lua_State *L ); /* [number] hyperspace() */

/* escorts */
static int aiL_dock( lua_State *L ); /* dock( number ) */

/* combat */
static int aiL_combat( lua_State *L ); /* combat( number ) */
static int aiL_settarget( lua_State *L ); /* settarget( number ) */
static int aiL_weapSet( lua_State *L ); /* weapset( number ) */
static int aiL_shoot( lua_State *L ); /* shoot( number ); number = 1,2,3 */
static int aiL_hascannons( lua_State *L ); /* bool hascannons() */
static int aiL_hasturrets( lua_State *L ); /* bool hasturrets() */
static int aiL_hasjammers( lua_State *L ); /* bool hasjammers() */
static int aiL_hasafterburner( lua_State *L ); /* bool hasafterburner() */
static int aiL_getenemy( lua_State *L ); /* number getenemy() */
static int aiL_getenemy_size( lua_State *L ); /* number getenemy_size() */
static int aiL_getenemy_heuristic( lua_State *L ); /* number getenemy_heuristic() */
static int aiL_hostile( lua_State *L ); /* hostile( number ) */
static int aiL_getweaprange( lua_State *L ); /* number getweaprange() */
static int aiL_getweapspeed( lua_State *L ); /* number getweapspeed() */
static int aiL_canboard( lua_State *L ); /* boolean canboard( number ) */
static int aiL_relsize( lua_State *L ); /* boolean relsize( number ) */
static int aiL_reldps( lua_State *L ); /* boolean reldps( number ) */
static int aiL_relhp( lua_State *L ); /* boolean relhp( number ) */

/* timers */
static int aiL_settimer( lua_State *L ); /* settimer( number, number ) */
static int aiL_timeup( lua_State *L ); /* boolean timeup( number ) */

/* messages */
static int aiL_distress( lua_State *L ); /* distress( string [, bool] ) */
static int aiL_getBoss( lua_State *L ); /* number getBoss() */

/* loot */
static int aiL_credits( lua_State *L ); /* credits( number ) */

/* misc */
static int aiL_board( lua_State *L ); /* boolean board() */
static int aiL_refuel( lua_State *L ); /* boolean, boolean refuel() */
static int aiL_messages( lua_State *L );


static const luaL_reg aiL_methods[] = {
   /* tasks */
   { "pushtask", aiL_pushtask },
   { "poptask", aiL_poptask },
   { "taskname", aiL_taskname },
   { "target", aiL_gettarget },
   { "pushsubtask", aiL_pushsubtask },
   { "popsubtask", aiL_popsubtask },
   { "subtaskname", aiL_subtaskname },
   { "subtarget", aiL_getsubtarget },
   /* is */
   { "ismaxvel", aiL_ismaxvel },
   { "isstopped", aiL_isstopped },
   { "isenemy", aiL_isenemy },
   { "isally", aiL_isally },
   { "haslockon", aiL_haslockon },
   /* get */
   { "pilot", aiL_pilot },
   { "rndpilot", aiL_getrndpilot },
   { "nearestpilot", aiL_getnearestpilot },
   { "dist", aiL_getdistance },
   { "flyby_dist", aiL_getflybydistance },
   { "minbrakedist", aiL_minbrakedist },
   { "isbribed", aiL_isbribed },
   { "getstanding", aiL_getstanding },
   /* movement */
   { "nearestplanet", aiL_getnearestplanet },
   { "rndplanet", aiL_getrndplanet },
   { "landplanet", aiL_getlandplanet },
   { "land", aiL_land },
   { "accel", aiL_accel },
   { "turn", aiL_turn },
   { "face", aiL_face },
   { "careful_face", aiL_careful_face },
   { "iface", aiL_iface },
   { "dir", aiL_dir },
   { "idir", aiL_idir },
   { "drift_facing", aiL_drift_facing },
   { "brake", aiL_brake },
   { "stop", aiL_stop },
   { "relvel", aiL_relvel },
   { "follow_accurate", aiL_follow_accurate },
   /* Hyperspace. */
   { "sethyptarget", aiL_sethyptarget },
   { "nearhyptarget", aiL_nearhyptarget },
   { "rndhyptarget", aiL_rndhyptarget },
   { "hyperspace", aiL_hyperspace },
   { "dock", aiL_dock },
   /* combat */
   { "aim", aiL_aim },
   { "combat", aiL_combat },
   { "settarget", aiL_settarget },
   { "weapset", aiL_weapSet },
   { "hascannons", aiL_hascannons },
   { "hasturrets", aiL_hasturrets },
   { "hasjammers", aiL_hasjammers },
   { "hasafterburner", aiL_hasafterburner },
   { "shoot", aiL_shoot },
   { "getenemy", aiL_getenemy },
   { "getenemy_size", aiL_getenemy_size },
   { "getenemy_heuristic", aiL_getenemy_heuristic },
   { "hostile", aiL_hostile },
   { "getweaprange", aiL_getweaprange },
   { "getweapspeed", aiL_getweapspeed },
   { "canboard", aiL_canboard },
   { "relsize", aiL_relsize },
   { "reldps", aiL_reldps },
   { "relhp", aiL_relhp },
   /* timers */
   { "settimer", aiL_settimer },
   { "timeup", aiL_timeup },
   /* messages */
   { "distress", aiL_distress },
   { "getBoss", aiL_getBoss },
   /* loot */
   { "setcredits", aiL_credits },
   /* misc */
   { "board", aiL_board },
   { "refuel", aiL_refuel },
   { "messages", aiL_messages },
   {0,0} /* end */
}; /**< Lua AI Function table. */



/*
 * current pilot "thinking" and assorted variables
 */
Pilot *cur_pilot           = NULL; /**< Current pilot.  All functions use this. */
static double pilot_acc    = 0.; /**< Current pilot's acceleration. */
static double pilot_turn   = 0.; /**< Current pilot's turning. */
static int pilot_flags     = 0; /**< Handle stuff like weapon firing. */
static char aiL_distressmsg[PATH_MAX]; /**< Buffer to store distress message. */

/*
 * ai status, used so that create functions can't be used elsewhere
 */
#define AI_STATUS_NORMAL      1 /**< Normal AI function behaviour. */
#define AI_STATUS_CREATE      2 /**< AI is running create function. */
static int aiL_status = AI_STATUS_NORMAL; /**< Current AI run status. */


/**
 * @brief Runs the garbage collector on the pilot's tasks.
 *
 *    @param pilot Pilot to clean up.
 */
static void ai_taskGC( Pilot* pilot )
{
   Task *t, *prev, *pointer;

   prev  = NULL;
   t     = pilot->task;
   while (t != NULL) {
      if (t->done) {
         pointer = t;
         /* Unattach pointer. */
         t       = t->next;
         if (prev == NULL)
            pilot->task = t;
         else
            prev->next  = t;
         /* Free pointer. */
         pointer->next = NULL;
         ai_freetask( pointer );
      }
      else {
         prev    = t;
         t       = t->next;
      }
   }
}


/**
 * @brief Gets the current running task.
 */
static Task* ai_curTask( Pilot* pilot )
{
   Task *t;
   /* Get last task. */
   for (t=pilot->task; t!=NULL; t=t->next)
      if (!t->done)
         return t;
   return NULL;
}


/**
 * @brief Sets the cur_pilot's ai.
 */
static void ai_setMemory (void)
{
   nlua_env env;
   env = cur_pilot->ai->env;

   nlua_getenv(env, AI_MEM); /* pm */
   lua_rawgeti(naevL, -1, cur_pilot->id); /* pm, t */
   nlua_setenv(env, "mem"); /* pm */
   lua_pop(naevL, 1); /* */
}


/**
 * @brief Sets the pilot for further AI calls.
 *
 *    @param p Pilot to set.
 */
void ai_setPilot( Pilot *p )
{
   cur_pilot = p;
   ai_setMemory();
}


/**
 * @brief Attempts to run a function.
 *
 *    @param[in] L Lua state to run function on.
 *    @param[in] funcname Function to run.
 */
static void ai_run( nlua_env env, const char *funcname )
{
   nlua_getenv(env, funcname);

#ifdef DEBUGGING
   if (lua_isnil(naevL, -1)) {
      WARN("Pilot '%s' ai -> '%s': attempting to run non-existant function",
            cur_pilot->name, funcname );
      lua_pop(naevL,1);
      return;
   }
#endif /* DEBUGGING */

   if (nlua_pcall(env, 0, 0)) { /* error has occurred */
      WARN("Pilot '%s' ai -> '%s': %s", cur_pilot->name, funcname, lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}


/**
 * @brief Initializes the pilot in the ai.
 *
 * Mainly used to create the pilot's memory table.
 *
 *    @param p Pilot to initialize in AI.
 *    @param ai AI to initialize pilot.
 *    @return 0 on success.
 */
int ai_pinit( Pilot *p, const char *ai )
{
   AI_Profile *prof;
   char buf[PATH_MAX];

   strncpy(buf, ai, sizeof(buf));

   /* Set up the profile. */
   prof = ai_getProfile(buf);
   if (prof == NULL) {
      WARN("AI Profile '%s' not found, using dummy fallback.", buf);
      nsnprintf(buf, sizeof(buf), "dummy" );
      prof = ai_getProfile(buf);
   }
   p->ai = prof;

   /* Adds a new pilot memory in the memory table. */
   nlua_getenv(p->ai->env, AI_MEM);  /* pm */
   lua_newtable(naevL);              /* pm, nt */
   lua_pushvalue(naevL, -1);         /* pm, nt, nt */
   lua_rawseti(naevL, -3, p->id);    /* pm, nt */

   /* Copy defaults over. */
   lua_pushstring(naevL, AI_MEM_DEF);/* pm, nt, s */
   lua_gettable(naevL, -3);          /* pm, nt, dt */
#if DEBUGGING
   if (lua_isnil(naevL,-1))
      WARN( "AI profile '%s' has no default memory for pilot '%s'.",
            buf, p->name );
#endif
   lua_pushnil(naevL);               /* pm, nt, dt, nil */
   while (lua_next(naevL,-2) != 0) { /* pm, nt, dt, k, v */
      lua_pushvalue(naevL,-2);       /* pm, nt, dt, k, v, k */
      lua_pushvalue(naevL,-2);       /* pm, nt, dt, k, v, k, v */
      lua_remove(naevL, -3);         /* pm, nt, dt, k, k, v */
      lua_settable(naevL,-5);        /* pm, nt, dt, k */
   }                             /* pm, nt, dt */
   lua_pop(naevL,3);                 /* */

   /* Create the pilot. */
   ai_create( p );
   pilot_setFlag(p, PILOT_CREATED_AI);

   /* Set fuel.  Hack until we do it through AI itself. */
   if (!pilot_isPlayer(p)) {
      p->fuel  = (RNG_2SIGMA()/4. + 0.5) * (p->fuel_max - p->fuel_consumption);
      p->fuel += p->fuel_consumption;
   }

   return 0;
}


/**
 * @brief Clears the pilot's tasks.
 *
 *    @param p Pilot to clear tasks of.
 */
void ai_cleartasks( Pilot* p )
{
   /* Clean up tasks. */
   if (p->task)
      ai_freetask( p->task );
   p->task = NULL;
}


/**
 * @brief Destroys the ai part of the pilot
 *
 *    @param[in] p Pilot to destroy its AI part.
 */
void ai_destroy( Pilot* p )
{
   nlua_env env;
   env = p->ai->env;

   /* Get rid of pilot's memory. */
   if (!pilot_isPlayer(p)) { /* Player is an exception as more than one ship shares pilot id. */
      nlua_getenv(env, AI_MEM);  /* t */
      lua_pushnil(naevL);        /* t, nil */
      lua_rawseti(naevL,-2, p->id);/* t */
      lua_pop(naevL, 1);         /* */
   }

   /* Clear the tasks. */
   ai_cleartasks( p );
}


/**
 * @brief Initializes the AI stuff which is basically Lua.
 *
 *    @return 0 on no errors.
 */
int ai_load (void)
{
   char** files;
   uint32_t nfiles, i;
   char path[PATH_MAX];
   int flen, suflen;
   int n;

   /* get the file list */
   files = ndata_list( AI_PATH, &nfiles );

   /* load the profiles */
   suflen = strlen(AI_SUFFIX);
   for (i=0; i<nfiles; i++) {
      flen = strlen(files[i]);
      if ((flen > suflen) &&
            strncmp(&files[i][flen-suflen], AI_SUFFIX, suflen)==0) {

         nsnprintf( path, PATH_MAX, AI_PATH"%s", files[i] );
         if (ai_loadProfile(path)) /* Load the profile */
            WARN("Error loading AI profile '%s'", path);
      }

      /* Clean up. */
      free(files[i]);
   }

   n = array_size(profiles);
   DEBUG("Loaded %d AI Profile%c", n, (n==1)?' ':'s');

   /* More clean up. */
   free(files);

   /* Load equipment thingy. */
   return ai_loadEquip();
}


/**
 * @brief Loads the equipment selector script.
 */
static int ai_loadEquip (void)
{
   char *buf;
   uint32_t bufsize;
   const char *filename = "dat/factions/equip/generic.lua";

   /* Make sure doesn't already exist. */
   if (equip_env != LUA_NOREF)
      nlua_freeEnv(equip_env);

   /* Create new state. */
   equip_env = nlua_newEnv(1);
   nlua_loadStandard(equip_env);

   /* Load the file. */
   buf = ndata_read( filename, &bufsize );
   if (nlua_dobufenv(equip_env, buf, bufsize, filename) != 0) {
      WARN("Error loading file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check",
            filename, lua_tostring(naevL, -1));
      return -1;
   }
   free(buf);

   return 0;
}


/**
 * @brief Initializes an AI_Profile and adds it to the stack.
 *
 *    @param[in] filename File to create the profile from.
 *    @return 0 on no error.
 */
static int ai_loadProfile( const char* filename )
{
   char* buf = NULL;
   uint32_t bufsize = 0;
   nlua_env env;
   AI_Profile *prof;
   size_t len;

   /* Create array if necessary. */
   if (profiles == NULL)
      profiles = array_create( AI_Profile );

   /* Grow array. */
   prof = &array_grow(&profiles);

   /* Set name. */
   len = strlen(filename)-strlen(AI_PATH)-strlen(AI_SUFFIX);
   prof->name = malloc(len+1);
   strncpy( prof->name, &filename[strlen(AI_PATH)], len );
   prof->name[len] = '\0';

   /* Create Lua. */
   env = nlua_newEnv(1);
   nlua_loadStandard(env);
   prof->env = env;

   /* Register C functions in Lua */
   nlua_register(env, "ai", aiL_methods, 0);

   /* Add the player memory table. */
   lua_newtable(naevL);              /* pm */
   lua_pushvalue(naevL, -1);         /* pm, pm */
   nlua_setenv(env, AI_MEM);         /* pm */

   /* Set "mem" to be default template. */
   lua_newtable(naevL);              /* pm, nt */
   lua_pushvalue(naevL,-1);          /* pm, nt, nt */
   lua_setfield(naevL,-3,AI_MEM_DEF); /* pm, nt */
   nlua_setenv(env, "mem");          /* pm */
   lua_pop(naevL, 1);                /*  */

   /* Now load the file since all the functions have been previously loaded */
   buf = ndata_read( filename, &bufsize );
   if (nlua_dobufenv(env, buf, bufsize, filename) != 0) {
      WARN("Error loading AI file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check",
            filename, lua_tostring(naevL,-1));
      array_erase( &profiles, prof, &prof[1] );
      free(prof->name);
      nlua_freeEnv( env );
      free(buf);
      return -1;
   }
   free(buf);

   return 0;
}


/**
 * @brief Gets the AI_Profile by name.
 *
 *    @param[in] name Name of the profile to get.
 *    @return The profile or NULL on error.
 */
AI_Profile* ai_getProfile( char* name )
{
   int i;

   if (profiles == NULL)
      return NULL;

   for (i=0; i<array_size(profiles); i++)
      if (strcmp(name,profiles[i].name)==0)
         return &profiles[i];

   WARN("AI Profile '%s' not found in AI stack", name);
   return NULL;
}


/**
 * @brief Cleans up global AI.
 */
void ai_exit (void)
{
   int i;

   /* Free AI profiles. */
   for (i=0; i<array_size(profiles); i++) {
      free(profiles[i].name);
      nlua_freeEnv(profiles[i].env);
   }
   array_free( profiles );

   /* Free equipment Lua. */
   if (equip_env != LUA_NOREF)
      nlua_freeEnv(equip_env);
   equip_env = LUA_NOREF;
}


/**
 * @brief Heart of the AI, brains of the pilot.
 *
 *    @param pilot Pilot that needs to think.
 */
void ai_think( Pilot* pilot, const double dt )
{
   nlua_env env;
   (void) dt;

   Task *t;

   /* Must have AI. */
   if (cur_pilot->ai == NULL)
      return;

   ai_setPilot(pilot);
   env = cur_pilot->ai->env; /* set the AI profile to the current pilot's */

   /* Clean up some variables */
   pilot_acc         = 0;
   pilot_turn        = 0.;
   pilot_flags       = 0;
   /* pilot_setTarget( cur_pilot, cur_pilot->id ); */
   pilot_weapSetAIClear( cur_pilot ); /* Hack so shit works. TODO fix. */

   /* Get current task. */
   t = ai_curTask( cur_pilot );

   /* control function if pilot is idle or tick is up */
   if ((cur_pilot->tcontrol < 0.) || (t == NULL)) {
      if (pilot_isFlag(pilot,PILOT_PLAYER) ||
          pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL)) {
         nlua_getenv(env, "control_manual");
         if (!lua_isnil(naevL, -1))
            ai_run(env, "control_manual");
         lua_pop(naevL, 1);
      } else {
         ai_run(env, "control"); /* run control */
      }

      nlua_getenv(env, "control_rate");
      cur_pilot->tcontrol = lua_tonumber(naevL,-1);
      lua_pop(naevL,1);

      /* Task may have changed due to control tick. */
      t = ai_curTask( cur_pilot );
   }

   if (pilot_isFlag(pilot,PILOT_PLAYER) &&
       !pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL))
      return;

   /* pilot has a currently running task */
   if (t != NULL) {
      /* Run subtask if available, otherwise run main task. */
      if (t->subtask != NULL)
         ai_run(env, t->subtask->name);
      else
         ai_run(env, t->name);

      /* Manual control must check if IDLE hook has to be run. */
      if (pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL)) {
         /* We must yet check again to see if there still is a current task running. */
         if (ai_curTask( cur_pilot ) == NULL)
            pilot_runHook( cur_pilot, PILOT_HOOK_IDLE );
      }
   }

   /* make sure pilot_acc and pilot_turn are legal */
   pilot_acc   = CLAMP( -1., 1., pilot_acc );
   pilot_turn  = CLAMP( -1., 1., pilot_turn );

   /* Set turn and thrust. */
   pilot_setTurn( cur_pilot, pilot_turn );
   pilot_setThrust( cur_pilot, pilot_acc );

   /* fire weapons if needed */
   if (ai_isFlag(AI_PRIMARY))
      pilot_shoot(cur_pilot, 0); /* primary */
   if (ai_isFlag(AI_SECONDARY))
      pilot_shoot(cur_pilot, 1 ); /* secondary */

   /* other behaviours. */
   if (ai_isFlag(AI_DISTRESS))
      pilot_distress(cur_pilot, NULL, aiL_distressmsg, 0);

   /* Clean up if necessary. */
   ai_taskGC( cur_pilot );
}


/**
 * @brief Triggers the attacked() function in the pilot's AI.
 *
 *    @param attacked Pilot that is attacked.
 *    @param[in] attacker ID of the attacker.
 *    @param[i] dmg Damage done by the attacker.
 */
void ai_attacked( Pilot* attacked, const unsigned int attacker, double dmg )
{
   HookParam hparam[2];

   /* Custom hook parameters. */
   hparam[0].type       = HOOK_PARAM_PILOT;
   hparam[0].u.lp       = attacker;
   hparam[1].type       = HOOK_PARAM_NUMBER;
   hparam[1].u.num      = dmg;

   /* Behaves differently if manually overridden. */
   pilot_runHookParam( attacked, PILOT_HOOK_ATTACKED, hparam, 2 );
   if (pilot_isFlag( attacked, PILOT_MANUAL_CONTROL ))
      return;

   /* Must have an AI profile and not be player. */
   if (attacked->ai == NULL)
      return;

   ai_setPilot( attacked ); /* Sets cur_pilot. */

   nlua_getenv(cur_pilot->ai->env, "attacked");

   lua_pushpilot(naevL, attacker);
   if (nlua_pcall(cur_pilot->ai->env, 1, 0)) {
      WARN("Pilot '%s' ai -> 'attacked': %s", cur_pilot->name, lua_tostring(naevL, -1));
      lua_pop(naevL, 1);
   }
}


/**
 * @brief Has a pilot attempt to refuel the other.
 *
 *    @param refueler Pilot doing the refueling.
 *    @param target Pilot to refuel.
 */
void ai_refuel( Pilot* refueler, unsigned int target )
{
   Task *t;

   /* Create the task. */
   t           = calloc( 1, sizeof(Task) );
   t->name     = strdup("refuel");
   lua_pushinteger(naevL, target);
   t->dat      = luaL_ref(naevL, LUA_REGISTRYINDEX);

   /* Prepend the task. */
   t->next     = refueler->task;
   refueler->task = t;

   return;
}


/**
 * @brief Sends a distress signal to a pilot.
 *
 *    @param p Pilot receiving the distress signal.
 *    @param distressed Pilot sending the distress signal.
 */
void ai_getDistress( Pilot *p, const Pilot *distressed, const Pilot *attacker )
{
   /* Ignore distress signals when under manual control. */
   if (pilot_isFlag( p, PILOT_MANUAL_CONTROL ))
      return;

   /* Must have AI. */
   if (cur_pilot->ai == NULL)
      return;

   /* Set up the environment. */
   ai_setPilot(p);

   /* See if function exists. */
   nlua_getenv(cur_pilot->ai->env, "distress");
   if (lua_isnil(naevL,-1)) {
      lua_pop(naevL,1);
      return;
   }

   /* Run the function. */
   lua_pushpilot(naevL, distressed->id);
   if (attacker != NULL)
      lua_pushpilot(naevL, attacker->id);
   else /* Default to the victim's current target. */
      lua_pushpilot(naevL, distressed->target);
   
   if (nlua_pcall(cur_pilot->ai->env, 2, 0)) {
      WARN("Pilot '%s' ai -> 'distress': %s", cur_pilot->name, lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}


/**
 * @brief Runs the create() function in the pilot.
 *
 * Should create all the gear and such the pilot has.
 *
 *    @param pilot Pilot to "create".
 */
static void ai_create( Pilot* pilot )
{
   nlua_env env;
   char *func;

   env = equip_env;
   func = "equip_generic";

   /* Set creation mode. */
   if (!pilot_isFlag(pilot, PILOT_CREATED_AI))
      aiL_status = AI_STATUS_CREATE;

   /* Create equipment first - only if creating for the first time. */
   if (!pilot_isFlag(pilot,PILOT_PLAYER) && (aiL_status==AI_STATUS_CREATE) &&
            !pilot_isFlag(pilot, PILOT_EMPTY)) {
      if  (faction_getEquipper( pilot->faction ) != LUA_NOREF) {
         env = faction_getEquipper( pilot->faction );
         func = "equip";
      }
      nlua_getenv(env, func);
      nlua_pushenv(env);
      lua_setfenv(naevL, -2);
      lua_pushpilot(naevL, pilot->id);
      if (nlua_pcall(env, 1, 0)) { /* Error has occurred. */
         WARN("Pilot '%s' equip -> '%s': %s", pilot->name, func, lua_tostring(naevL, -1));
         lua_pop(naevL, 1);
      }
   }

   /* Since the pilot changes outfits and cores, we must heal him up. */
   pilot_healLanded( pilot );

   /* Must have AI. */
   if (pilot->ai == NULL)
      return;

   /* Prepare AI (this sets cur_pilot among others). */
   ai_setPilot( pilot );

   /* Prepare stack. */
   nlua_getenv(cur_pilot->ai->env, "create");

   /* Run function. */
   if (nlua_pcall(cur_pilot->ai->env, 0, 0)) { /* error has occurred */
      WARN("Pilot '%s' ai -> '%s': %s", cur_pilot->name, "create", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

   /* Recover normal mode. */
   if (!pilot_isFlag(pilot, PILOT_CREATED_AI))
      aiL_status = AI_STATUS_NORMAL;
}


/**
 * @brief Creates a new AI task.
 */
Task *ai_newtask( Pilot *p, const char *func, int subtask, int pos )
{
   Task *t, *curtask, *pointer;

   /* Create the new task. */
   t           = calloc( 1, sizeof(Task) );
   t->name     = strdup(func);
   lua_pushnil(naevL);
   t->dat      = luaL_ref(naevL, LUA_REGISTRYINDEX);

   /* Handle subtask and general task. */
   if (!subtask) {
      if ((pos == 1) && (p->task != NULL)) { /* put at the end */
         for (pointer = p->task; pointer->next != NULL; pointer = pointer->next);
         pointer->next = t;
      }
      else {
         t->next = p->task;
         p->task = t;
      }
   }
   else {
      /* Must have valid task. */
      curtask = ai_curTask( p );
      if (curtask == NULL) {
         WARN("Trying to add subtask '%s' to non-existant task.", func);
         ai_freetask( t );
         return NULL;
      }

      /* Add the subtask. */
      if ((pos == 1) && (curtask->subtask != NULL)) { /* put at the end */
         for (pointer = curtask->subtask; pointer->next != NULL; pointer = pointer->next);
         pointer->next = t;
      }
      else {
         t->next           = curtask->subtask;
         curtask->subtask  = t;
      }
   }

   return t;
}


/**
 * @brief Frees an AI task.
 *
 *    @param t Task to free.
 */
void ai_freetask( Task* t )
{
   luaL_unref(naevL, LUA_REGISTRYINDEX, t->dat);

   /* Recursive subtask freeing. */
   if (t->subtask != NULL) {
      ai_freetask(t->subtask);
      t->subtask = NULL;
   }

   /* Free next task in the chain. */
   if (t->next != NULL) {
      ai_freetask(t->next); /* yay recursive freeing */
      t->next = NULL;
   }

   if (t->name)
      free(t->name);
   free(t);
}


/**
 * @brief Creates a new task based on stack information.
 */
static Task* ai_createTask( lua_State *L, int subtask )
{
   const char *func;
   Task *t;

   /* Parse basic parameters. */
   func  = luaL_checkstring(L,1);

   /* Creates a new AI task. */
   t     = ai_newtask( cur_pilot, func, subtask, 0 );

   /* Set the data. */
   if (lua_gettop(L) > 1) {
      t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
   }

   return t;
}


/**
 * @brief Pushes a task target.
 */
static int ai_tasktarget( lua_State *L, Task *t )
{
   lua_rawgeti(L, LUA_REGISTRYINDEX, t->dat);
   return 1;
}


/**
 * @defgroup AI Lua AI Bindings
 *
 * @brief Handles how the AI interacts with the universe.
 *
 * Usage is:
 * @code
 * ai.function( params )
 * @endcode
 *
 * @luamod ai
 *
 * @{
 */
/**
 * @brief Pushes a task onto the pilot's task list.
 *    @luatparam string func Name of function to call for task.
 *    @luaparam[opt] data Data to pass to the function.  Supports any lua type.
 * @luafunc pushtask( func, data )
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_pushtask( lua_State *L )
{
   ai_createTask( L, 0 );

   return 0;
}
/**
 * @brief Pops the current running task.
 * @luafunc poptask()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_poptask( lua_State *L )
{
   Task* t = ai_curTask( cur_pilot );

   /* Tasks must exist. */
   if (t == NULL) {
      NLUA_ERROR(L, "Trying to pop task when there are no tasks on the stack.");
      return 0;
   }

   t->done = 1;
   return 0;
}

/**
 * @brief Gets the current task's name.
 *    @luatreturn string The current task name or nil if there are no tasks.
 * @luafunc taskname()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_taskname( lua_State *L )
{
   Task *t = ai_curTask( cur_pilot );
   if (t)
      lua_pushstring(L, t->name);
   else
      lua_pushnil(L);
   return 1;
}

/**
 * @brief Gets the pilot's task target.
 *    @luareturn The pilot's target ship identifier or nil if no target.
 *    @luasee pushtask
 * @luafunc target()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_gettarget( lua_State *L )
{
   Task *t = ai_curTask( cur_pilot );

   /* Must have a task. */
   if (t == NULL)
      return 0;

   return ai_tasktarget( L, t );
}

/**
 * @brief Pushes a subtask onto the pilot's task's subtask list.
 *    @luatparam string func Name of function to call for task.
 *    @luaparam[opt] data Data to pass to the function.  Supports any lua type.
 * @luafunc pushsubtask( func, data )
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_pushsubtask( lua_State *L )
{
   ai_createTask(L, 1);
   return 0;
}

/**
 * @brief Pops the current running task.
 * @luafunc popsubtask()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_popsubtask( lua_State *L )
{
   Task *t, *st;
   t = ai_curTask( cur_pilot );

   /* Tasks must exist. */
   if (t == NULL) {
      NLUA_ERROR(L, "Trying to pop task when there are no tasks on the stack.");
      return 0;
   }
   if (t->subtask == NULL) {
      NLUA_ERROR(L, "Trying to pop subtask when there are no subtasks for the task '%s'.", t->name);
      return 0;
   }

   /* Exterminate, annihilate destroy. */
   st          = t->subtask;
   t->subtask  = st->next;
   st->next    = NULL;
   ai_freetask(st);
   return 0;
}

/**
 * @brief Gets the current subtask's name.
 *    @luatreturn string The current subtask name or nil if there are no subtasks.
 * @luafunc subtaskname()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_subtaskname( lua_State *L )
{
   Task *t = ai_curTask( cur_pilot );
   if ((t != NULL) && (t->subtask != NULL))
      lua_pushstring(L, t->subtask->name);
   else
      lua_pushnil(L);
   return 1;
}

/**
 * @brief Gets the pilot's subtask target.
 *    @luareturn The pilot's target ship identifier or nil if no target.
 *    @luasee pushsubtask
 * @luafunc subtarget()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_getsubtarget( lua_State *L )
{
   Task *t = ai_curTask( cur_pilot );
   /* Must have a subtask. */
   if ((t == NULL) || (t->subtask == NULL))
      return 0;

   return ai_tasktarget( L, t->subtask );
}


/**
 * @brief Gets the AI's pilot.
 *    @luatreturn Pilot The AI's pilot.
 * @luafunc pilot()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_pilot( lua_State *L )
{
   lua_pushpilot(L, cur_pilot->id);
   return 1;
}


/**
 * @brief Gets a random pilot in the system.
 *    @luatreturn Pilot|nil
 * @luafunc rndpilot()
 *    @param L Lua state.
 *    @return Number of Lua parameters.
 */
static int aiL_getrndpilot( lua_State *L )
{
   int p;

   p = RNG(0, pilot_nstack-1);
   /* Make sure it can't be the same pilot. */
   if (pilot_stack[p]->id == cur_pilot->id) {
      p++;
      if (p >= pilot_nstack)
         p = 0;
   }
   /* Last check. */
   if (pilot_stack[p]->id == cur_pilot->id)
      return 0;
   /* Actually found a pilot. */
   lua_pushpilot(L, pilot_stack[p]->id);
   return 1;
}

/**
 * @brief gets the nearest pilot to the current pilot
 *
 *    @luatreturn Pilot|nil
 *    @luafunc nearestpilot()
 */
static int aiL_getnearestpilot( lua_State *L )
{

   /*dist will be initialized to a number*/
   /*this will only seek out pilots closer than dist*/
   int dist=1000;
   int i;
   int candidate_id = -1;

   /*cycle through all the pilots and find the closest one that is not the pilot */

   for(i = 0; i<pilot_nstack; i++)
   {
       if(pilot_stack[i]->id != cur_pilot->id && vect_dist(&pilot_stack[i]->solid->pos, &cur_pilot->solid->pos) < dist)
       {
            dist = vect_dist(&pilot_stack[i]->solid->pos, &cur_pilot->solid->pos);
            candidate_id = i;
       }
   }

   /* Last check. */
   if (candidate_id == -1)
      return 0;

   /* Actually found a pilot. */
   lua_pushpilot(L, pilot_stack[candidate_id]->id);
   return 1;
}

/**
 * @brief Gets the distance from the pointer.
 *
 *    @luatparam Vec2|Pilot pointer
 *    @luatreturn number The distance from the pointer.
 *    @luafunc dist( pointer )
 */
static int aiL_getdistance( lua_State *L )
{
   Vector2d *v;
   Pilot *p;

   /* vector as a parameter */
   if (lua_isvector(L,1))
      v = lua_tovector(L,1);

   /* pilot as parameter */
   else if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      v = &p->solid->pos;
   }

   /* wrong parameter */
   else
      NLUA_INVALID_PARAMETER(L);

   lua_pushnumber(L, vect_dist(v, &cur_pilot->solid->pos));
   return 1;
}

/**
 * @brief Gets the distance from the pointer perpendicular to the current pilot's flight vector.
 *
 *    @luatparam Vec2|Pilot pointer
 *    @luatreturn number offset_distance
 *    @luafunc flyby_dist( pointer )
 */
static int aiL_getflybydistance( lua_State *L )
{
   Vector2d *v;
   Vector2d perp_motion_unit, offset_vect;
   Pilot *p;
   int offset_distance;

   v = NULL;

   /* vector as a parameter */
   if (lua_isvector(L,1))
      v = lua_tovector(L,1);
   /* pilot id as parameter */
   else if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      v = &p->solid->pos;

      /*vect_cset(&v, VX(pilot->solid->pos) - VX(cur_pilot->solid->pos), VY(pilot->solid->pos) - VY(cur_pilot->solid->pos) );*/
   }
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

   vect_cset(&offset_vect, VX(*v) - VX(cur_pilot->solid->pos), VY(*v) - VY(cur_pilot->solid->pos) );
   vect_pset(&perp_motion_unit, 1, VANGLE(cur_pilot->solid->vel)+M_PI_2);
   offset_distance = vect_dot(&perp_motion_unit, &offset_vect);

   lua_pushnumber(L, offset_distance);
   return 1;
}

/**
 * @brief Gets the minimum braking distance.
 *
 * braking vel ==> 0 = v - a*dt
 * add turn around time (to inital vel) ==> 180.*360./cur_pilot->turn
 * add it to general euler equation  x = v * t + 0.5 * a * t^2
 * and voila!
 *
 * I hate this function and it'll probably need to get changed in the future
 *
 *
 *    @luatreturn number Minimum braking distance.
 *    @luafunc minbrakedist()
 */
static int aiL_minbrakedist( lua_State *L )
{
   double time, dist, vel;
   Vector2d vv;
   Pilot *p;

   /* More complicated calculation based on relative velocity. */
   if (lua_gettop(L) > 0) {
      p = luaL_validpilot(L,1);

      /* Set up the vectors. */
      vect_cset( &vv, p->solid->vel.x - cur_pilot->solid->vel.x,
            p->solid->vel.y - cur_pilot->solid->vel.y );

      /* Run the same calculations. */
      time = VMOD(vv) /
            (cur_pilot->thrust / cur_pilot->solid->mass);

      /* Get relative velocity. */
      vel = MIN(cur_pilot->speed - VMOD(p->solid->vel), VMOD(vv));
      if (vel < 0.)
         vel = 0.;
   }

   /* Simple calculation based on distance. */
   else {
      /* Get current time to reach target. */
      time = VMOD(cur_pilot->solid->vel) /
            (cur_pilot->thrust / cur_pilot->solid->mass);

      /* Get velocity. */
      vel = MIN(cur_pilot->speed,VMOD(cur_pilot->solid->vel));
   }
   /* Get distance to brake. */
   dist = vel*(time+1.1*M_PI/cur_pilot->turn) -
         0.5*(cur_pilot->thrust/cur_pilot->solid->mass)*time*time;

   lua_pushnumber(L, dist); /* return */
   return 1; /* returns one thing */
}


/**
 * @brief Checks to see if target has bribed pilot.
 *
 *    @luatparam Pilot target
 *    @luatreturn boolean Whether the target has bribed pilot.
 *    @luafunc isbribed( target )
 */
static int aiL_isbribed( lua_State *L )
{
   Pilot *p;
   p = luaL_validpilot(L,1);
   lua_pushboolean(L, (p->id == PLAYER_ID) && pilot_isFlag(cur_pilot,PILOT_BRIBED));
   return 1;
}


/**
 * @brief Gets the standing of the target pilot with the current pilot.
 *
 *    @luatparam Pilot target Pilot to get faction standing of.
 *    @luatreturn number|nil The faction standing of the target [-100,100] or nil if invalid.
 * @luafunc getstanding( target )
 */
static int aiL_getstanding( lua_State *L )
{
   Pilot *p;

   /* Get parameters. */
   p = luaL_validpilot(L,1);

   /* Get faction standing. */
   if (p->faction == FACTION_PLAYER)
      lua_pushnumber(L, faction_getPlayer(cur_pilot->faction));
   else {
      if (areAllies( cur_pilot->faction, p->faction ))
         lua_pushnumber(L, 100);
      else if (areEnemies( cur_pilot->faction, p->faction ))
         lua_pushnumber(L,-100);
      else
         lua_pushnumber(L, 0);
   }

   return 1;
}


/**
 * @brief Checks to see if pilot is at maximum velocity.
 *
 *    @luatreturn boolean Whether the pilot is at maximum velocity.
 *    @luafunc ismaxvel()
 */
static int aiL_ismaxvel( lua_State *L )
{
   lua_pushboolean(L,(VMOD(cur_pilot->solid->vel) > cur_pilot->speed-MIN_VEL_ERR));
   return 1;
}


/**
 * @brief Checks to see if pilot is stopped.
 *
 *    @luatreturn boolean Whether the pilot is stopped.
 *    @luafunc isstopped()
 */
static int aiL_isstopped( lua_State *L )
{
   lua_pushboolean(L,(VMOD(cur_pilot->solid->vel) < MIN_VEL_ERR));
   return 1;
}


/**
 * @brief Checks to see if target is an enemy.
 *
 *    @luatparam Pilot target
 *    @luatreturn boolean Whether the target is an enemy.
 *    @luafunc isenemy( target )
 */
static int aiL_isenemy( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Player needs special handling in case of hostility. */
   if (p->faction == FACTION_PLAYER) {
      lua_pushboolean(L, pilot_isHostile(cur_pilot));
      lua_pushboolean(L,1);
      return 1;
   }

   /* Check if is ally. */
   lua_pushboolean(L,areEnemies(cur_pilot->faction, p->faction));
   return 1;
}

/**
 * @brief Checks to see if target is an ally.
 *
 *    @luatparam Pilot target
 *    @luatreturn boolean Whether the target is an ally.
 *    @luafunc isally( target )
 */
static int aiL_isally( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   /* Player needs special handling in case of friendliness. */
   if (p->faction == FACTION_PLAYER) {
      lua_pushboolean(L, pilot_isFriendly(cur_pilot));
      return 1;
   }

   /* Check if is ally. */
   lua_pushboolean(L,areAllies(cur_pilot->faction, p->faction));
   return 1;
}


/**
 * @brief Checks to see if pilot has a missile lockon.
 *
 *    @luatreturn boolean Whether the pilot has a missile lockon.
 *    @luafunc haslockon()
 */

static int aiL_haslockon( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->lockons > 0);
   return 1;
}


/**
 * @brief Starts accelerating the pilot.
 *
 *    @luatparam[opt=1.] number acceleration Fraction of pilot's maximum acceleration from 0 to 1.
 *    @luafunc accel( acceleration )
 */
static int aiL_accel( lua_State *L )
{
   double n;

   if (lua_gettop(L) > 1 && lua_isnumber(L,1)) {
      n = (double)lua_tonumber(L,1);

      if (n > 1.) n = 1.;
      else if (n < 0.) n = 0.;
      pilot_acc = n;
   }
   else
      pilot_acc = 1.;

   return 0;
}


/**
 * @brief Starts turning the pilot.
 *
 *    @luatparam number vel Directional velocity from -1 to 1.
 *    @luafunc turn( vel )
 */
static int aiL_turn( lua_State *L )
{
   pilot_turn = luaL_checknumber(L,1);
   return 0;
}


/**
 * @brief Faces the target.
 *
 * @usage ai.face( a_pilot ) -- Face a pilot
 * @usage ai.face( a_pilot, true ) -- Face away from a pilot
 * @usage ai.face( a_pilot, nil, true ) -- Compensate velocity facing a pilot
 *
 *    @luatparam Pilot|Vec2|number target Target to face.
 *    @luatparam boolean invert Invert away from target.
 *    @luatparam boolean compensate Compensate for velocity?
 *    @luatreturn number Angle offset in degrees.
 * @luafunc face( target, invert, compensate )
 */
static int aiL_face( lua_State *L )
{
   Vector2d *tv; /* get the position to face */
   Pilot* p;
   double k_diff, k_vel, d, diff, vx, vy, dx, dy;
   int vel;

   /* Get first parameter, aka what to face. */
   if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      /* Target vector. */
      tv = &p->solid->pos;
   }
   else if (lua_isnumber(L,1)) {
      d = (double)lua_tonumber(L,1);
      if (d < 0.)
         tv = &cur_pilot->solid->pos;
      else
         NLUA_INVALID_PARAMETER(L);
   }
   else if (lua_isvector(L,1))
      tv = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L);

   /* Default gain. */
   k_diff = 10.;
   k_vel  = 100.; /* overkill gain! */

   /* Check if must invert. */
   if (lua_toboolean(L,2))
      k_diff *= -1;

   /* Third parameter. */
   vel = lua_toboolean(L, 3);

   /* Tangential component of velocity vector
    *
    * v: velocity vector
    * d: direction vector
    *
    *                  d       d                d
    * v_t = v - ( v . --- ) * --- = v - ( v . ----- ) * d
    *                 |d|     |d|             |d|^2
    */
   /* Velocity vector. */
   vx = cur_pilot->solid->vel.x;
   vy = cur_pilot->solid->vel.y;
   /* Direction vector. */
   dx = tv->x - cur_pilot->solid->pos.x;
   dy = tv->y - cur_pilot->solid->pos.y;
   if (vel) {
      /* Calculate dot product. */
      d = (vx * dx + vy * dy) / (dx*dx + dy*dy);
      /* Calculate tangential velocity. */
      vx = vx - d * dx;
      vy = vy - d * dy;

      /* Add velocity compensation. */
      dx += -k_vel * vx;
      dy += -k_vel * vy;
   }

   /* Compensate error and rotate. */
   diff = angle_diff( cur_pilot->solid->dir, atan2( dy, dx ) );

   /* Make pilot turn. */
   pilot_turn = k_diff * diff;

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, ABS(diff*180./M_PI));
   return 1;
}


/**
 * @brief Gives the direction to follow in order to reach the target while
 *  minimizating risk.
 *
 * This method is based on a simplified version of trajectory generation in
 * mobile robotics using the potential method.
 *
 * The principle is to consider the mobile object (ship) as a mechanical object.
 * Obstacles (enemies) and the target exert 
 * attractive or repulsive force on this object.
 *
 * Only visible ships are taken into account.
 *
 *    @luatparam Pilot|Vec2|number target Target to go to.
 * @luafunc careful_face( target )
 */
static int aiL_careful_face( lua_State *L )
{
   Vector2d *tv, F, F1;
   Pilot* p;
   Pilot *p_i;
   double k_diff, k_goal, k_enemy, k_mult,
          d, diff, dist, factor;
   int i;

   /* Init some variables */
   p = cur_pilot;

   /* Get first parameter, aka what to face. */
   if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      /* Target vector. */
      tv = &p->solid->pos;
   }
   else if (lua_isnumber(L,1)) {
      d = (double)lua_tonumber(L,1);
      if (d < 0.)
         tv = &cur_pilot->solid->pos;
      else
         NLUA_INVALID_PARAMETER(L);
   }
   else if (lua_isvector(L,1))
      tv = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L);

   /* Default gains. */
   k_diff = 10.;
   k_goal = 1.;
   k_enemy = 6000000.;

   /* Init the force */
   vect_cset( &F, 0., 0.) ;
   vect_cset( &F1, tv->x - cur_pilot->solid->pos.x, tv->y - cur_pilot->solid->pos.y) ;
   dist = VMOD(F1) + 0.1; /* Avoid / 0*/
   vect_cset( &F1, F1.x * k_goal / dist, F1.y * k_goal / dist) ;

   /* Cycle through all the pilots in order to compute the force */
   for(i=0; i<pilot_nstack; i++) {
      p_i = pilot_stack[i];

      /* Valid pilot isn't self, is in range, isn't the target and isn't disabled */
      if (pilot_isDisabled(p_i) ) continue;
      if (p_i->id == cur_pilot->id) continue;
      if (p_i->id == p->id) continue; 
      if (pilot_inRangePilot(cur_pilot, p_i) != 1) continue;

      /* If the enemy is too close, ignore it*/
      dist = vect_dist(&p_i->solid->pos, &cur_pilot->solid->pos);
      if (dist < 750) continue;

      k_mult = pilot_relhp( p_i, cur_pilot ) * pilot_reldps( p_i, cur_pilot );

      /* Check if friendly or not */
      if (areEnemies(cur_pilot->faction, p_i->faction)) {
         factor = k_enemy * k_mult / (dist*dist*dist);
         vect_cset( &F, F.x + factor * (cur_pilot->solid->pos.x - p_i->solid->pos.x),
                F.y + factor * (cur_pilot->solid->pos.y - p_i->solid->pos.y) );
      }
   }

   vect_cset( &F, F.x + F1.x, F.y + F1.y );

   /* Rotate. */
   diff = angle_diff( cur_pilot->solid->dir, VANGLE(F) );

   /* Make pilot turn. */
   pilot_turn = k_diff * diff;

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, ABS(diff*180./M_PI));
   return 1;
}


/**
 * @brief Aims at a pilot, trying to hit it rather than move to it.
 *
 * This method uses a polar UV decomposition to get a more accurate time-of-flight
 *
 *    @luatparam Pilot target The pilot to aim at
 *    @luatreturn number The offset from the target aiming position (in degrees).
 * @luafunc aim( target )
 */
static int aiL_aim( lua_State *L )
{
   double x,y;
   double t;
   Pilot *p;
   Vector2d tv, approach_vector, relative_location, orthoradial_vector;
   double dist, diff;
   double mod;
   double speed;
   double radial_speed;
   double orthoradial_speed;

   /* Only acceptable parameter is pilot */
   p = luaL_validpilot(L,1);

   /* Get the distance */
   dist = vect_dist( &cur_pilot->solid->pos, &p->solid->pos );

   /* Check if should recalculate weapon speed with secondary weapon. */
   speed = pilot_weapSetSpeed( cur_pilot, cur_pilot->active_set, -1 );

   /* determine the radial, or approach speed */
   /*
    *approach_vector (denote Va) is the relative velocites of the pilot and target
    *relative_location (denote Vr) is the vector that points from the target to the pilot
    *
    *Va dot Vr is the rate of approach between the target and the pilot.
    *If this is greater than 0, the target is approaching the pilot, if less than 0, the target is fleeing.
    *
    *Va dot Vr + ShotSpeed is the net closing velocity for the shot, and is used to compute the time of flight for the shot.
    *
    *Position prediction logic is the same as the previous function
    */
   vect_cset(&approach_vector, VX(cur_pilot->solid->vel) - VX(p->solid->vel), VY(cur_pilot->solid->vel) - VY(p->solid->vel) );
   vect_cset(&relative_location, VX(p->solid->pos) -  VX(cur_pilot->solid->pos),  VY(p->solid->pos) - VY(cur_pilot->solid->pos) );
   vect_cset(&orthoradial_vector, VY(cur_pilot->solid->pos) - VY(p->solid->pos), VX(p->solid->pos) -  VX(cur_pilot->solid->pos) );

   radial_speed = vect_dot(&approach_vector, &relative_location);
   radial_speed = radial_speed / VMOD(relative_location);

   orthoradial_speed = vect_dot(&approach_vector, &orthoradial_vector);
   orthoradial_speed = orthoradial_speed / VMOD(relative_location);

   /* Time for shots to reach that distance */
   /* t is the real positive solution of a 2nd order equation*/
   /* if the target is not hittable (i.e., fleeing faster than our shots can fly, determinant <= 0), just face the target */
   if( ((speed*speed - VMOD(approach_vector)*VMOD(approach_vector)) != 0) && (speed*speed - orthoradial_speed*orthoradial_speed) > 0)
      t = dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) - radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));
   else
      t = 0;

   /* if t < 0, try the other solution*/
   if (t < 0)
      t = - dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) + radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));

   /* if t still < 0, no solution*/
   if (t < 0)
      t = 0;

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


/**
 * @brief Maintains an intercept pursuit course.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to intercept.
 *    @luatreturn number The offset from the proper intercept course (in degrees).
 * @luafunc iface( target )
 */
static int aiL_iface( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   Vector2d *vec, drift, reference_vector; /* get the position to face */
   Pilot* p;
   double diff, heading_offset_azimuth, drift_radial, drift_azimuthal;
   int azimuthal_sign;
   double speedmap;

   /* Get first parameter, aka what to face. */
   p  = NULL;
   vec = NULL;
   if (lua_ispilot(L,1))
      p = luaL_validpilot(L,1);
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else NLUA_INVALID_PARAMETER(L);

   if (vec==NULL) {
      if (p == NULL)
         return 0; /* Return silently when attempting to face an invalid pilot. */
      /* Establish the current pilot velocity and position vectors */
      vect_cset( &drift, VX(p->solid->vel) - VX(cur_pilot->solid->vel), VY(p->solid->vel) - VY(cur_pilot->solid->vel));
      /* Establish the in-line coordinate reference */
      vect_cset( &reference_vector, VX(p->solid->pos) - VX(cur_pilot->solid->pos), VY(p->solid->pos) - VY(cur_pilot->solid->pos));
   }
   else {
      /* Establish the current pilot velocity and position vectors */
      vect_cset( &drift, -VX(cur_pilot->solid->vel), -VY(cur_pilot->solid->vel));
      /* Establish the in-line coordinate reference */
      vect_cset( &reference_vector, VX(*vec) - VX(cur_pilot->solid->pos), VY(*vec) - VY(cur_pilot->solid->pos));
   }

   /* Break down the the velocity vectors of both craft into UV coordinates */
   vect_uv(&drift_radial, &drift_azimuthal, &drift, &reference_vector);
   heading_offset_azimuth = angle_diff(cur_pilot->solid->dir, VANGLE(reference_vector));

   /* Now figure out what to do...
    * Are we pointing anywhere inside the correct UV quadrant?
    * if we're outside the correct UV quadrant, we need to get into it ASAP
    * Otherwise match velocities and approach */
   if (fabs(heading_offset_azimuth) < M_PI_2) {
      /* This indicates we're in the correct plane*/
      /* 1 - 1/(|x|+1) does a pretty nice job of mapping the reals to the interval (0...1). That forms the core of this angle calculation */
      /* There is nothing special about the scaling parameter of 200; it can be tuned to get any behavior desired. A lower
         number will give a more dramatic 'lead' */
      speedmap = -1*copysign(1 - 1 / (fabs(drift_azimuthal/200) + 1), drift_azimuthal) * M_PI_2;
      diff = angle_diff(heading_offset_azimuth, speedmap);
      azimuthal_sign = -1;

      /* This indicates we're drifting to the right of the target
       * And we need to turn CCW */
      if (diff > 0)
         pilot_turn = azimuthal_sign;
      /* This indicates we're drifting to the left of the target
       * And we need to turn CW */
      else if (diff < 0)
         pilot_turn = -1*azimuthal_sign;
      else
         pilot_turn = 0;
   }
   /* turn most efficiently to face the target. If we intercept the correct quadrant in the UV plane first, then the code above will kick in */
   /* some special case logic is added to optimize turn time. Reducing this to only the else cases would speed up the operation
      but cause the pilot to turn in the less-than-optimal direction sometimes when between 135 and 225 degrees off from the target */
   else {
      /* signal that we're not in a productive direction for thrusting */
      diff = M_PI;
      azimuthal_sign = 1;


      if(heading_offset_azimuth >0)
         pilot_turn = azimuthal_sign;
      else
         pilot_turn = -1*azimuthal_sign;
   }

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, ABS(diff*180./M_PI));
   return 1;
}

/**
 * @brief calculates the direction that the target is relative to the current pilot facing.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to compare facing to
 *    @luatreturn number The facing offset to the target (in degrees).
 * @luafunc dir( target )
 *
 */
static int aiL_dir( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   Vector2d *vec, sv, tv; /* get the position to face */
   Pilot* p;
   double diff;
   int n;

   /* Get first parameter, aka what to face. */
   n  = -2;
   vec = NULL;
   if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      vect_cset( &tv, VX(p->solid->pos), VY(p->solid->pos) );
   }
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else NLUA_INVALID_PARAMETER(L);

   vect_cset( &sv, VX(cur_pilot->solid->pos), VY(cur_pilot->solid->pos) );

   if (vec==NULL) /* target is dynamic */
      diff = angle_diff(cur_pilot->solid->dir,
            (n==-1) ? VANGLE(sv) :
            vect_angle(&sv, &tv));
   else /* target is static */
      diff = angle_diff( cur_pilot->solid->dir,
            (n==-1) ? VANGLE(cur_pilot->solid->pos) :
            vect_angle(&cur_pilot->solid->pos, vec));


   /* Return angle in degrees away from target. */
   lua_pushnumber(L, diff*180./M_PI);
   return 1;
}

/**
 * @brief calculates angle between pilot facing and intercept-course to target.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to compare facing to
 *    @luatreturn number The facing offset to intercept-course to the target (in degrees).
 * @luafunc idir( target )
 */
static int aiL_idir( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   Vector2d *vec, drift, reference_vector; /* get the position to face */
   Pilot* p;
   double diff, heading_offset_azimuth, drift_radial, drift_azimuthal;
   double speedmap;
   /*char announcebuffer[255] = " ", announcebuffer2[128];*/

   /* Get first parameter, aka what to face. */
   p  = NULL;
   vec = NULL;
   if (lua_ispilot(L,1))
      p = luaL_validpilot(L,1);
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else NLUA_INVALID_PARAMETER(L);

   if (vec==NULL) {
      if (p == NULL)
         return 0; /* Return silently when attempting to face an invalid pilot. */
      /* Establish the current pilot velocity and position vectors */
      vect_cset( &drift, VX(p->solid->vel) - VX(cur_pilot->solid->vel), VY(p->solid->vel) - VY(cur_pilot->solid->vel));
      /* Establish the in-line coordinate reference */
      vect_cset( &reference_vector, VX(p->solid->pos) - VX(cur_pilot->solid->pos), VY(p->solid->pos) - VY(cur_pilot->solid->pos));
   }
   else {
      /* Establish the current pilot velocity and position vectors */
      vect_cset( &drift, -VX(cur_pilot->solid->vel), -VY(cur_pilot->solid->vel));
      /* Establish the in-line coordinate reference */
      vect_cset( &reference_vector, VX(*vec) - VX(cur_pilot->solid->pos), VY(*vec) - VY(cur_pilot->solid->pos));
   }

   /* Break down the the velocity vectors of both craft into UV coordinates */
   vect_uv(&drift_radial, &drift_azimuthal, &drift, &reference_vector);
   heading_offset_azimuth = angle_diff(cur_pilot->solid->dir, VANGLE(reference_vector));

   /* now figure out what to do*/
   /* are we pointing anywhere inside the correct UV quadrant? */
   /* if we're outside the correct UV quadrant, we need to get into it ASAP */
   /* Otherwise match velocities and approach*/
   if (fabs(heading_offset_azimuth) < M_PI_2) {
      /* This indicates we're in the correct plane
       * 1 - 1/(|x|+1) does a pretty nice job of mapping the reals to the interval (0...1). That forms the core of this angle calculation
       * there is nothing special about the scaling parameter of 200; it can be tuned to get any behavior desired. A lower
       * number will give a more dramatic 'lead' */
      speedmap = -1*copysign(1 - 1 / (fabs(drift_azimuthal/200) + 1), drift_azimuthal) * M_PI_2;
      diff = angle_diff(heading_offset_azimuth, speedmap);

   }
   /* Turn most efficiently to face the target. If we intercept the correct quadrant in the UV plane first, then the code above will kick in
      some special case logic is added to optimize turn time. Reducing this to only the else cases would speed up the operation
      but cause the pilot to turn in the less-than-optimal direction sometimes when between 135 and 225 degrees off from the target */
   else{
      /* signal that we're not in a productive direction for thrusting */
      diff        = M_PI;
   }

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, diff*180./M_PI);
   return 1;
}

/**
 * @brief Calculate the offset between the pilot's current direction of travel and the pilot's current facing.
 *
 *    @luatreturn number Offset
 *    @luafunc drift_facing()
 */
static int aiL_drift_facing( lua_State *L )
{
    double drift;
    drift = angle_diff(VANGLE(cur_pilot->solid->vel), cur_pilot->solid->dir);
    lua_pushnumber(L, drift*180./M_PI);
    return 1;
}

/**
 * @brief Brakes the pilot.
 *
 *    @luatreturn boolean Whether braking is finished.
 *    @luafunc brake()
 */

static int aiL_brake( lua_State *L )
{
   int ret;

   ret = pilot_brake( cur_pilot );

   pilot_acc = cur_pilot->solid->thrust / cur_pilot->thrust;
   pilot_turn = cur_pilot->solid->dir_vel / cur_pilot->turn;

   lua_pushboolean(L, ret);
   return 1;
}


/**
 * @brief Get the nearest friendly planet to the pilot.
 *
 *    @luatreturn Planet|nil
 *    @luafunc nearestplanet()
 */
static int aiL_getnearestplanet( lua_State *L )
{
   double dist, d;
   int i, j;
   LuaPlanet planet;

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

   planet = cur_system->planets[j]->id;
   lua_pushplanet(L, planet);

   return 1;
}


/**
 * @brief Get a random planet.
 *
 *    @luatreturn Planet|nil
 *    @luafunc rndplanet()
 */
static int aiL_getrndplanet( lua_State *L )
{
   LuaPlanet planet;
   int p;

   if (cur_system->nplanets == 0) return 0; /* no planets */

   /* get a random planet */
   p = RNG(0, cur_system->nplanets-1);

   /* Copy the data into a vector */
   planet = cur_system->planets[p]->id;
   lua_pushplanet(L, planet);

   return 1;
}

/**
 * @brief Get a random friendly planet.
 *
 *    @luatparam boolean only_friend Only check for ally planets.
 *    @luatreturn Planet|nil
 * @luafunc landplanet( only_friend )
 */
static int aiL_getlandplanet( lua_State *L )
{
   int *ind;
   int nplanets, i;
   LuaPlanet planet;
   Planet *p;
   int only_friend;

   /* Must have planets. */
   if (cur_system->nplanets == 0)
      return 0; /* no planets */

   /* Check if we should get only friendlies. */
   only_friend = lua_toboolean(L, 1);

   /* Allocate memory. */
   ind = malloc( sizeof(int) * cur_system->nplanets );

   /* Copy friendly planet.s */
   for (nplanets=0, i=0; i<cur_system->nplanets; i++) {
      if (!planet_hasService(cur_system->planets[i],PLANET_SERVICE_INHABITED))
         continue;

      /* Check conditions. */
      if (only_friend && !areAllies( cur_pilot->faction, cur_system->planets[i]->faction ))
         continue;
      else if (!only_friend && areEnemies(cur_pilot->faction,cur_system->planets[i]->faction))
         continue;

      /* Add it. */
      ind[ nplanets++ ] = i;
   }

   /* no planet to land on found */
   if (nplanets==0) {
      free(ind);
      return 0;
   }

   /* we can actually get a random planet now */
   i = RNG(0,nplanets-1);
   p = cur_system->planets[ ind[i] ];
   planet = p->id;
   lua_pushplanet( L, planet );
   cur_pilot->nav_planet   = ind[ i ];
   free(ind);

   return 1;
}


/**
 * @brief Lands on a planet.
 *
 *    @luatreturn boolean Whether landing was successful.
 *    @luafunc land()
 */
static int aiL_land( lua_State *L )
{
   int ret;
   Planet *planet;
   HookParam hparam;

   ret = 0;

   if (cur_pilot->nav_planet < 0) {
      NLUA_ERROR( L, "Pilot '%s' has no land target", cur_pilot->name );
      return 0;
   }

   /* Get planet. */
   planet = cur_system->planets[ cur_pilot->nav_planet ];

   /* Check landability. */
   if (!planet_hasService(planet,PLANET_SERVICE_INHABITED))
      ret++;

   /* Check distance. */
   if (vect_dist2(&cur_pilot->solid->pos,&planet->pos) > pow2(planet->radius))
      ret++;

   /* Check velocity. */
   if ((pow2(VX(cur_pilot->solid->vel)) + pow2(VY(cur_pilot->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL))
      ret++;

   /* Check landing functionality. */
   if (pilot_isFlag(cur_pilot, PILOT_NOLAND))
      ret++;

   if (!ret) {
      cur_pilot->ptimer = PILOT_LANDING_DELAY;
      pilot_setFlag( cur_pilot, PILOT_LANDING );

      hparam.type    = HOOK_PARAM_ASSET;
      hparam.u.la    = planet->id;

      pilot_runHookParam( cur_pilot, PILOT_HOOK_LAND, &hparam, 1 );
   }

   lua_pushboolean(L,!ret);
   return 1;
}


/**
 * @brief Tries to enter hyperspace.
 *
 *    @luatreturn number|nil Distance if too far away.
 *    @luafunc hyperspace()
 */
static int aiL_hyperspace( lua_State *L )
{
   int dist;

   dist = space_hyperspace(cur_pilot);
   if (dist == 0.) {
      pilot_shootStop( cur_pilot, 0 );
      pilot_shootStop( cur_pilot, 1 );
      return 0;
   }

   lua_pushnumber(L,dist);
   return 1;
}


/**
 * @brief Sets hyperspace target.
 *
 *    @luatparam Jump target Hyperspace target
 *    @luareturn Vec2 Where to go to jump
 *    @luafunc sethyptarget(target)
 */
static int aiL_sethyptarget( lua_State *L )
{
   JumpPoint *jp;
   LuaJump *lj;
   Vector2d vec;
   double a, rad;

   lj = luaL_checkjump( L, 1 );
   jp = luaL_validjump( L, 1 );

   if ( lj->srcid != cur_system->id )
      NLUA_ERROR(L, "Jump point must be in current system.");

   /* Copy vector. */
   vec = jp->pos;

   /* Introduce some error. */
   a     = RNGF() * M_PI * 2.;
   rad   = RNGF() * 0.5 * jp->radius;
   vect_cadd( &vec, rad*cos(a), rad*sin(a) );

   /* Set up target. */
   cur_pilot->nav_hyperspace = jp - cur_system->jumps;

   /* Return vector. */
   lua_pushvector( L, vec );

   return 1;
}


/**
 * @brief Gets the nearest hyperspace target.
 *
 *    @luatreturn JumpPoint|nil
 *    @luafunc nearhyptarget()
 */
static int aiL_nearhyptarget( lua_State *L )
{
   JumpPoint *jp, *jiter;
   double mindist, dist;
   int i;
   LuaJump lj;

   /* No jumps. */
   if (cur_system->njumps == 0)
      return 0;

   /* Find nearest jump .*/
   mindist = INFINITY;
   jp      = NULL;
   for (i=0; i <cur_system->njumps; i++) {
      jiter = &cur_system->jumps[i];
      /* We want only standard jump points to be used. */
      if (jp_isFlag(jiter, JP_HIDDEN) || jp_isFlag(jiter, JP_EXITONLY))
         continue;
      /* Get nearest distance. */
      dist  = vect_dist2( &cur_pilot->solid->pos, &jiter->pos );
      if (dist < mindist) {
         jp       = jiter;
         mindist  = dist;
      }
   }
   /* None available. */
   if (jp == NULL)
      return 0;

   lj.destid = jp->targetid;
   lj.srcid = cur_system->id;

   /* Return Jump. */
   lua_pushjump( L, lj );
   return 1;
}


/**
 * @brief Gets a random hyperspace target.
 *
 *    @luatreturn JumpPoint|nil
 *    @luafunc rndhyptarget()
 */
static int aiL_rndhyptarget( lua_State *L )
{
   JumpPoint **jumps, *jiter;
   int i, j, r;
   int *id;
   LuaJump lj;

   /* No jumps in the system. */
   if (cur_system->njumps == 0)
      return 0;

   /* Find usable jump points. */
   jumps = malloc( sizeof(JumpPoint*) * cur_system->njumps );
   id    = malloc( sizeof(int) * cur_system->njumps );
   j = 0;
   for (i=0; i < cur_system->njumps; i++) {
      jiter = &cur_system->jumps[i];
      /* We want only standard jump points to be used. */
      if (jp_isFlag(jiter, JP_HIDDEN) || jp_isFlag(jiter, JP_EXITONLY))
         continue;
      id[j]      = i;
      jumps[j++] = jiter;
   }

   /* Choose random jump point. */
   r = RNG(0, j-1);

   lj.destid = jumps[r]->targetid;
   lj.srcid = cur_system->id;

   /* Clean up. */
   free(jumps);
   free(id);

   /* Return Jump. */
   lua_pushjump( L, lj );
   return 1;
}

/**
 * @brief Gets the relative velocity of a pilot.
 *
 *    @luatreturn number Relative velocity.
 * @luafunc relvel()
 */
static int aiL_relvel( lua_State *L )
{
   double dot, mod;
   Pilot *p;
   Vector2d vv, pv;
   int absolute;

   p = luaL_validpilot(L,1);

   if (lua_gettop(L) > 1)
      absolute = lua_toboolean(L,2);
   else
      absolute = 0;

   /* Get the projection of target on current velocity. */
   if (absolute == 0)
      vect_cset( &vv, p->solid->vel.x - cur_pilot->solid->vel.x,
            p->solid->vel.y - cur_pilot->solid->vel.y );
   else
      vect_cset( &vv, p->solid->vel.x, p->solid->vel.y);

   vect_cset( &pv, p->solid->pos.x - cur_pilot->solid->pos.x,
         p->solid->pos.y - cur_pilot->solid->pos.y );
   dot = vect_dot( &pv, &vv );
   mod = MAX(VMOD(pv), 1.); /* Avoid /0. */

   lua_pushnumber(L, dot / mod );
   return 1;
}

/**
 * @brief Computes the point to face in order to
 *        follow an other pilot using a PD controller.
 *
 *    @luatparam Pilot target The pilot to follow
 *    @luatparam number radius The requested distance between p and target
 *    @luatparam number angle The requested angle between p and target
 *    @luatparam number Kp The first controller parameter
 *    @luatparam number Kd The second controller parameter
 *    @luatparam[opt] string method Method to compute goal angle
 *    @luareturn The point to go to as a vector2.
 * @luafunc follow_accurate( target, radius, angle, Kp, Kd, method )
 */
static int aiL_follow_accurate( lua_State *L )
{
   Vector2d point, cons, goal, pv;
   double radius, angle, Kp, Kd, angle2;
   Pilot *p, *target;
   const char *method;

   p = cur_pilot;
   target = luaL_validpilot(L,1);
   radius = luaL_checklong(L,2);
   angle = luaL_checklong(L,3);
   Kp = luaL_checklong(L,4);
   Kd = luaL_checklong(L,5);

   if (lua_isnoneornil(L, 6))
      method = "velocity";
   else
      method = luaL_checkstring(L,6);

   if (strcmp( method, "absolute" ) == 0)
      angle2 = angle * M_PI/180;
   else if (strcmp( method, "keepangle" ) == 0){
      vect_cset( &pv, p->solid->pos.x - target->solid->pos.x,
            p->solid->pos.y - target->solid->pos.y );
      angle2 = VANGLE(pv);
      }
   else /* method == "velocity" */
      angle2 = angle * M_PI/180 + VANGLE( target->solid->vel );

   vect_cset( &point, VX(target->solid->pos) + radius * cos(angle2),
         VY(target->solid->pos) + radius * sin(angle2) );

   /*  Compute the direction using a pd controller */
   vect_cset( &cons, (point.x - p->solid->pos.x) * Kp + 
         (target->solid->vel.x - p->solid->vel.x) *Kd,
         (point.y - p->solid->pos.y) * Kp +
         (target->solid->vel.y - p->solid->vel.y) *Kd );

   vect_cset( &goal, cons.x + p->solid->pos.x, cons.y + p->solid->pos.y);

   /* Push info */
   lua_pushvector( L, goal );

   return 1;

}

/**
 * @brief Completely stops the pilot if it is below minimum vel error (no insta-stops).
 *
 *    @luafunc stop()
 */
static int aiL_stop( lua_State *L )
{
   (void) L; /* avoid gcc warning */

   if (VMOD(cur_pilot->solid->vel) < MIN_VEL_ERR)
      vect_pset( &cur_pilot->solid->vel, 0., 0. );

   return 0;
}

/**
 * @brief Docks the ship.
 * 
 *    @luatparam Pilot target Pilot to dock with.
 *    @luafunc dock( target )
 */
static int aiL_dock( lua_State *L )
{
   Pilot *p;

   /* Target is another ship. */
   p = luaL_validpilot(L,1);
   pilot_dock(cur_pilot, p, 1);

   return 0;
}


/**
 * @brief Sets the combat flag.
 * 
 *    @luatparam[opt=true] boolean val Value to set flag to.
 *    @luafunc combat( val )
 */
static int aiL_combat( lua_State *L )
{
   int i;

   if (lua_gettop(L) > 0) {
      i = lua_toboolean(L,1);
      if (i==1) pilot_setFlag(cur_pilot, PILOT_COMBAT);
      else if (i==0) pilot_rmFlag(cur_pilot, PILOT_COMBAT);
   }
   else pilot_setFlag(cur_pilot, PILOT_COMBAT);

   return 0;
}


/**
 * @brief Sets the pilot's target.
 * 
 *    @luaparam target Pilot to target.
 *    @luafunc settarget( target )
 */
static int aiL_settarget( lua_State *L )
{
   Pilot *p;
   p = luaL_validpilot(L,1);
   pilot_setTarget( cur_pilot, p->id );
   return 0;
}


/**
 * @brief Sets the active weapon set, fires another weapon set or activate an outfit.
 *
 *    @luatparam number id ID of the weapon set to switch to or fire.
 *    @luatparam[opt=true] boolean type true to activate, false to deactivate.
 * @luafunc weapset( id, type )
 */
static int aiL_weapSet( lua_State *L )
{
   Pilot* p;
   int id, type, on, l, i;
   PilotWeaponSet *ws;

   p = cur_pilot;
   id = lua_tonumber(L,1);

   if (lua_gettop(L) > 1)
      type = lua_toboolean(L,2);
   else
      type = 1;

   ws = &p->weapon_sets[id];

   if (ws->type == WEAPSET_TYPE_ACTIVE)
   {
      /* Check if outfit is on */
      on = 1;
      l  = array_size(ws->slots);
      for (i=0; i<l; i++) {
         if (ws->slots[i].slot->state == PILOT_OUTFIT_OFF) {
            on = 0;
            break;
         }
      }

      /* activate */
      if (type && !on)
         pilot_weapSetPress(p, id, 1 );
      /* deactivate */
      if (!type && on)
         pilot_weapSetPress(p, id, 1 );
   }
   else /* weapset type is weapon or change */
      pilot_weapSetPress( cur_pilot, id, 1 );
   return 0;
}


/**
 * @brief Does the pilot have cannons?
 *
 *    @luatreturn boolean True if the pilot has cannons.
 * @luafunc hascannons()
 */
static int aiL_hascannons( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->ncannons > 0 );
   return 1;
}


/**
 * @brief Does the pilot have turrets?
 *
 *    @luatreturn boolean True if the pilot has turrets.
 * @luafunc hasturrets()
 */
static int aiL_hasturrets( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->nturrets > 0 );
   return 1;
}


/**
 * @brief Does the pilot have jammers?
 *
 *    @luatreturn boolean True if the pilot has jammers.
 * @luafunc hasjammers()
 */
static int aiL_hasjammers( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->njammers > 0 );
   return 1;
}


/**
 * @brief Does the pilot have afterburners?
 *
 *    @luatreturn boolean True if the pilot has afterburners.
 * @luafunc hasafterburners()
 */
static int aiL_hasafterburner( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->nafterburners > 0 );
   return 1;
}


/**
 * @brief Makes the pilot shoot
 *
 *    @luatparam[opt=false] boolean secondary Fire secondary weapons instead of primary.
 *    @luafunc shoot( secondary )
 */
static int aiL_shoot( lua_State *L )
{
   /* Cooldown is similar to a ship being disabled, but the AI continues to
    * think during cooldown, and thus must not be allowed to fire weapons. */
   if (pilot_isFlag(cur_pilot, PILOT_COOLDOWN))
      return 0;

   if (lua_toboolean(L,1))
      ai_setFlag(AI_SECONDARY);
   else
      ai_setFlag(AI_PRIMARY);
   return 0;
}


/**
 * @brief Gets the nearest enemy.
 *
 *    @luatreturn Pilot|nil
 *    @luafunc getenemy()
 */
static int aiL_getenemy( lua_State *L )
{
   unsigned int id;

   id = pilot_getNearestEnemy(cur_pilot);

   if (id==0) /* No enemy found */
      return 0;

   lua_pushpilot(L, id);

   return 1;
}

/**
 * @brief Gets the nearest enemy within specified size bounds.
 *
 *  @luatparam number lb Lower size bound
 *  @luatparam number ub upper size bound
 *  @luatreturn Pilot
 *  @luafunc getenemy_size( lb, ub )
 */
static int aiL_getenemy_size( lua_State *L )
{
   unsigned int id;
   unsigned int LB, UB;

   NLUA_MIN_ARGS(2);

   LB = luaL_checklong(L,1);
   UB = luaL_checklong(L,2);

   if (LB > UB) {
      NLUA_ERROR(L, "Invalid Bounds");
      return 0;
   }

   id = pilot_getNearestEnemy_size( cur_pilot, LB, UB );

   if (id==0) /* No enemy found */
      return 0;

   lua_pushpilot(L, id);
   return 1;
}


/**
 * @brief Gets the nearest enemy within specified heuristic.
 *
 *  @luatparam number mass goal mass map (0-1)
 *  @luatparam number dps goal DPS map (0-1)
 *  @luatparam number hp goal HP map (0-1)
 *  @luatparam number range weighting for range (typically > 1)
 *  @luatreturn Pilot the best fitting target
 *  @luafunc getenemy_heuristic( mass, dps, hp, range )
 */
static int aiL_getenemy_heuristic( lua_State *L )
{

   unsigned int id;
   double mass_factor, health_factor, damage_factor, range_factor;

   mass_factor    = luaL_checklong(L,1);
   health_factor  = luaL_checklong(L,2);
   damage_factor  = luaL_checklong(L,3);
   range_factor   = luaL_checklong(L,4);

   id = pilot_getNearestEnemy_heuristic( cur_pilot,
         mass_factor, health_factor, damage_factor, 1./range_factor );

   if (id==0) /* No enemy found */
      return 0;

   lua_pushpilot(L, id);
   return 1;
}


/**
 * @brief Sets the enemy hostile (basically notifies of an impending attack).
 *
 *    @luatparam Pilot target Pilot to set hostile.
 *    @luafunc hostile( target )
 */
static int aiL_hostile( lua_State *L )
{
   Pilot *p;

   p = luaL_validpilot(L,1);

   if (p->faction == FACTION_PLAYER)
      pilot_setHostile(cur_pilot);

   return 0;
}


/**
 * @brief Gets the range of a weapon.
 *
 *    @luatparam[opt] number id Optional parameter indicating id of weapon set to get range of, defaults to selected one.
 *    @luatparam[opt=-1] number level Level of weapon set to get range of.
 *    @luatreturn number The range of the weapon set.
 * @luafunc getweaprange( id, level )
 */
static int aiL_getweaprange( lua_State *L )
{
   int id;
   int level;

   id    = cur_pilot->active_set;
   level = -1;
   if (lua_isnumber(L,1))
      id = luaL_checkint(L,1);
   if (lua_isnumber(L,2))
      level = luaL_checkint(L,2);

   lua_pushnumber(L, pilot_weapSetRange( cur_pilot, id, level ) );
   return 1;
}


/**
 * @brief Gets the speed of a weapon.
 *
 *    @luatparam[opt] number id Optional parameter indicating id of weapon set to get speed of, defaults to selected one.
 *    @luatparam[opt=-1] number level Level of weapon set to get range of.
 *    @luatreturn number The range of the weapon set.
 * @luafunc getweapspeed( id, level )
 */
static int aiL_getweapspeed( lua_State *L )
{
   int id;
   int level;

   id    = cur_pilot->active_set;
   level = -1;
   if (lua_isnumber(L,1))
      id = luaL_checkint(L,1);
   if (lua_isnumber(L,2))
      level = luaL_checkint(L,2);

   lua_pushnumber(L, pilot_weapSetSpeed( cur_pilot, id, level ) );
   return 1;
}


/**
 * @brief Checks to see if pilot can board the target.
 *
 *    @luatparam Pilot target Target to see if pilot can board.
 *    @luatreturn boolean true if pilot can board, false if it can't.
 * @luafunc canboard( target )
 */
static int aiL_canboard( lua_State *L )
{
   Pilot *p;

   /* Get parameters. */
   p = luaL_validpilot(L,1);

   /* Must be disabled. */
   if (!pilot_isDisabled(p)) {
      lua_pushboolean(L, 0);
      return 1;
   }

   /* Check if can be boarded. */
   lua_pushboolean(L, !pilot_isFlag(p, PILOT_BOARDED));
   return 1;
}

/**
 * @brief Gets the relative size (ship mass) between the current pilot and the specified target.
 *
 *    @luatparam Pilot target The pilot whose mass we will compare.
 *    @luatreturn number A number from 0 to 1 mapping the relative masses.
 * @luafunc relsize( target )
 */
static int aiL_relsize( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   lua_pushnumber(L, pilot_relsize(cur_pilot, p));

   return 1;
}


/**
 * @brief Gets the relative damage output (total DPS) between the current pilot and the specified target.
 *
 *    @luatparam Pilot target The pilot whose DPS we will compare.
 *    @luatreturn number A number from 0 to 1 mapping the relative DPSes.
 * @luafunc reldps( target )
 */
static int aiL_reldps( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   lua_pushnumber(L, pilot_reldps(cur_pilot, p));

   return 1;
}


/**
 * @brief Gets the relative health (total shields and armour) between the current pilot and the specified target
 *
 *    @luatparam Pilot target The pilot whose health we will compare.
 *    @luatreturn number A number from 0 to 1 mapping the relative healths.
 *    @luafunc relhp(target)
 */
static int aiL_relhp( lua_State *L )
{
   Pilot *p;

   /* Get the pilot. */
   p = luaL_validpilot(L,1);

   lua_pushnumber(L, pilot_relhp(cur_pilot, p));

   return 1;
}



/**
 * @brief Attempts to board the pilot's target.
 *
 *    @luatreturn boolean true if was able to board the target.
 *    @luafunc board()
 */
static int aiL_board( lua_State *L )
{
   lua_pushboolean(L, pilot_board( cur_pilot ));
   return 1;
}


/**
 * @brief Attempts to refuel the pilot's target.
 *
 *    @luatreturn boolean true if pilot has begun refueling, false if it hasn't.
 *    @luafunc refuel()
 */
static int aiL_refuel( lua_State *L )
{
   lua_pushboolean(L,pilot_refuelStart(cur_pilot));
   return 1;
}


/**
 * @brief Sets a timer.
 *
 *    @luatparam number timer Timer number.
 *    @luatparam[opt=0] number time Number of seconds to set timer to.
 *    @luafunc settimer(timer, time)
 */
static int aiL_settimer( lua_State *L )
{
   int n;

   /* Get parameters. */
   n = luaL_checkint(L,1);

   /* Set timer. */
   cur_pilot->timer[n] = (lua_isnumber(L,2)) ? lua_tonumber(L,2)/1000. : 0;

   return 0;
}


/**
 * @brief Checks a timer.
 *
 *    @luatparam number timer Timer number.
 *    @luatreturn boolean Whether time is up.
 *    @luafunc timeup(timer)
 */

static int aiL_timeup( lua_State *L )
{
   int n;

   /* Get parameters. */
   n = luaL_checkint(L,1);

   lua_pushboolean(L, cur_pilot->timer[n] < 0.);
   return 1;
}


/**
 * @brief Sends a distress signal.
 *
 *    @luatparam string|nil msg Message to send or nil.
 *    @luafunc distress( msg )
 */
static int aiL_distress( lua_State *L )
{
   if (lua_isstring(L,1))
      nsnprintf( aiL_distressmsg, PATH_MAX, "%s", lua_tostring(L,1) );
   else if (lua_isnil(L,1))
      aiL_distressmsg[0] = '\0';
   else
      NLUA_INVALID_PARAMETER(L);

   /* Set flag because code isn't reentrant. */
   ai_setFlag(AI_DISTRESS);

   return 0;
}


/**
 * @brief Picks a pilot that will command the current pilot.
 *
 *    @luatreturn Pilot|nil
 *    @luafunc getBoss()
 */
static int aiL_getBoss( lua_State *L )
{
   unsigned int id;

   id = pilot_getBoss( cur_pilot );

   if (id==0) /* No boss found */
      return 0;

   lua_pushpilot(L, id);

   return 1;
}

/**
 * @brief Sets the pilot_nstack credits. Only call in create().
 *
 *    @luatparam number num Number of credits.
 *    @luafunc setcredits( num )
 */
static int aiL_credits( lua_State *L )
{
   if (aiL_status != AI_STATUS_CREATE) {
      /*NLUA_ERROR(L, "This function must be called in \"create\" only.");*/
      return 0;
   }

   cur_pilot->credits = luaL_checklong(L,1);

   return 0;
}


/**
 * @brief Returns and clears the pilots message queue.
 *
 *    @luafunc messages()
 *    @luatreturn {{},...} Messages.
 */
static int aiL_messages( lua_State *L )
{
   lua_rawgeti(L, LUA_REGISTRYINDEX, cur_pilot->messages);
   lua_newtable(naevL);
   lua_rawseti(L, LUA_REGISTRYINDEX, cur_pilot->messages);
   return 1;
}

/**
 * @}
 */
