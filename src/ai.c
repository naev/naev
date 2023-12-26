/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file ai.c
 *
 * @brief Controls the Pilot AI.
 *
 * @luamod ai
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
 *         subsystems and is pretty lacking in quite a few aspects.
 */
/** @cond */
#include <ctype.h>
#include <lauxlib.h>
#include <lualib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "ai.h"

#include "conf.h"
#include "array.h"
#include "board.h"
#include "escort.h"
#include "faction.h"
#include "hook.h"
#include "gatherable.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_asteroid.h"
#include "nlua_faction.h"
#include "nlua_pilot.h"
#include "nlua_spob.h"
#include "nlua_rnd.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "nstring.h"
#include "physics.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "space.h"

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
 * all the AI profiles
 */
static AI_Profile* profiles = NULL; /**< Array of AI_Profiles loaded. */
static nlua_env equip_env = LUA_NOREF; /**< Equipment enviornment. */
static IntList ai_qtquery; /*< Quadtree query. */

/*
 * prototypes
 */
/* Internal C routines */
static void ai_run( nlua_env env, int nargs );
static int ai_loadProfile( AI_Profile *prof, const char* filename );
static int ai_setMemory (void);
static void ai_create( Pilot* pilot );
static int ai_loadEquip (void);
static int ai_sort( const void *p1, const void *p2 );
/* Task management. */
static void ai_taskGC( Pilot* pilot );
static Task* ai_createTask( lua_State *L, int subtask );
static int ai_tasktarget( lua_State *L, const Task *t );

/*
 * AI routines for Lua
 */
/* tasks */
static int aiL_pushtask( lua_State *L ); /* pushtask( string, number/pointer ) */
static int aiL_poptask( lua_State *L ); /* poptask() */
static int aiL_taskname( lua_State *L ); /* string taskname() */
static int aiL_taskdata( lua_State *L ); /* pointer subtaskdata() */
static int aiL_pushsubtask( lua_State *L ); /* pushsubtask( string, number/pointer, number ) */
static int aiL_popsubtask( lua_State *L ); /* popsubtask() */
static int aiL_subtaskname( lua_State *L ); /* string subtaskname() */
static int aiL_subtaskdata( lua_State *L ); /* pointer subtaskdata() */

/* consult values */
static int aiL_pilot( lua_State *L ); /* number pilot() */
static int aiL_getrndpilot( lua_State *L ); /* number getrndpilot() */
static int aiL_getnearestpilot( lua_State *L ); /* number getnearestpilot() */
static int aiL_getdistance( lua_State *L ); /* number getdist(vec2) */
static int aiL_getdistance2( lua_State *L ); /* number getdist(vec2) */
static int aiL_getflybydistance( lua_State *L ); /* number getflybydist(vec2) */
static int aiL_minbrakedist( lua_State *L ); /* number minbrakedist( [number] ) */
static int aiL_isbribed( lua_State *L ); /* bool isbribed( number ) */
static int aiL_getGatherable( lua_State *L ); /* integer getgatherable( radius ) */
static int aiL_instantJump( lua_State *L ); /* bool instantJump() */

/* boolean expressions */
static int aiL_ismaxvel( lua_State *L ); /* boolean ismaxvel() */
static int aiL_isstopped( lua_State *L ); /* boolean isstopped() */
static int aiL_isenemy( lua_State *L ); /* boolean isenemy( number ) */
static int aiL_isally( lua_State *L ); /* boolean isally( number ) */
static int aiL_haslockon( lua_State *L ); /* boolean haslockon() */
static int aiL_hasprojectile( lua_State *L ); /* boolean hasprojectile() */
static int aiL_scandone( lua_State *L );

/* movement */
static int aiL_accel( lua_State *L ); /* accel(number); number <= 1. */
static int aiL_turn( lua_State *L ); /* turn(number); abs(number) <= 1. */
static int aiL_careful_face( lua_State *L ); /* face( number/pointer, bool) */
static int aiL_aim( lua_State *L ); /* aim(number) */
static int aiL_dir( lua_State *L ); /* dir(number/pointer) */
static int aiL_face( lua_State *L ); /* face( number/pointer, bool) */
static int aiL_iface( lua_State *L ); /* iface(number/pointer) */
static int aiL_idir( lua_State *L ); /* idir(number/pointer) */
static int aiL_follow_accurate( lua_State *L ); /* follow_accurate() */
static int aiL_face_accurate( lua_State *L ); /* face_accurate() */
static int aiL_drift_facing( lua_State *L ); /* drift_facing(number/pointer) */
static int aiL_brake( lua_State *L ); /* brake() */
static int aiL_getnearestspob( lua_State *L ); /* Vec2 getnearestspob() */
static int aiL_getspobfrompos( lua_State *L ); /* Vec2 getspobfrompos() */
static int aiL_getrndspob( lua_State *L ); /* Vec2 getrndspob() */
static int aiL_getlandspob( lua_State *L ); /* Vec2 getlandspob() */
static int aiL_land( lua_State *L ); /* bool land() */
static int aiL_stop( lua_State *L ); /* stop() */
static int aiL_relvel( lua_State *L ); /* relvel( number ) */

/* Hyperspace. */
static int aiL_sethyptarget( lua_State *L );
static int aiL_nearhyptarget( lua_State *L ); /* pointer rndhyptarget() */
static int aiL_rndhyptarget( lua_State *L ); /* pointer rndhyptarget() */
static int aiL_hyperspace( lua_State *L ); /* [number] hyperspace() */
static int aiL_canHyperspace( lua_State *L );
static int aiL_hyperspaceAbort( lua_State *L );

/* escorts */
static int aiL_dock( lua_State *L ); /* dock( number ) */

/* combat */
static int aiL_combat( lua_State *L ); /* combat( number ) */
static int aiL_settarget( lua_State *L ); /* settarget( number ) */
static int aiL_weapSet( lua_State *L ); /* weapset( number ) */
static int aiL_shoot( lua_State *L ); /* shoot( number ); number = 1,2,3 */
static int aiL_hascannons( lua_State *L ); /* bool hascannons() */
static int aiL_hasturrets( lua_State *L ); /* bool hasturrets() */
static int aiL_hasfighterbays( lua_State *L ); /* bool hasfighterbays() */
static int aiL_hasafterburner( lua_State *L ); /* bool hasafterburner() */
static int aiL_getenemy( lua_State *L ); /* number getenemy() */
static int aiL_hostile( lua_State *L ); /* hostile( number ) */
static int aiL_getweaprange( lua_State *L ); /* number getweaprange() */
static int aiL_getweapspeed( lua_State *L ); /* number getweapspeed() */
static int aiL_getweapammo( lua_State *L );
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
static int aiL_setasterotarget( lua_State *L ); /* setasterotarget( Asteroid ) */
static int aiL_gatherablePos( lua_State *L ); /* gatherablepos( number ) */
static int aiL_shoot_indicator( lua_State *L ); /* get shoot indicator */
static int aiL_set_shoot_indicator( lua_State *L ); /* set shoot indicator */
static int aiL_stealth( lua_State *L );

static const luaL_Reg aiL_methods[] = {
   /* tasks */
   { "pushtask", aiL_pushtask },
   { "poptask", aiL_poptask },
   { "taskname", aiL_taskname },
   { "taskdata", aiL_taskdata },
   { "pushsubtask", aiL_pushsubtask },
   { "popsubtask", aiL_popsubtask },
   { "subtaskname", aiL_subtaskname },
   { "subtaskdata", aiL_subtaskdata },
   /* is */
   { "ismaxvel", aiL_ismaxvel },
   { "isstopped", aiL_isstopped },
   { "isenemy", aiL_isenemy },
   { "isally", aiL_isally },
   { "haslockon", aiL_haslockon },
   { "hasprojectile", aiL_hasprojectile },
   { "scandone", aiL_scandone },
   /* get */
   { "pilot", aiL_pilot },
   { "rndpilot", aiL_getrndpilot },
   { "nearestpilot", aiL_getnearestpilot },
   { "dist", aiL_getdistance },
   { "dist2", aiL_getdistance2 },
   { "flyby_dist", aiL_getflybydistance },
   { "minbrakedist", aiL_minbrakedist },
   { "isbribed", aiL_isbribed },
   { "getgatherable", aiL_getGatherable },
   { "instantJump", aiL_instantJump },
   /* movement */
   { "nearestspob", aiL_getnearestspob },
   { "spobfrompos", aiL_getspobfrompos },
   { "rndspob", aiL_getrndspob },
   { "landspob", aiL_getlandspob },
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
   { "face_accurate", aiL_face_accurate },
   /* Hyperspace. */
   { "sethyptarget", aiL_sethyptarget },
   { "nearhyptarget", aiL_nearhyptarget },
   { "rndhyptarget", aiL_rndhyptarget },
   { "hyperspace", aiL_hyperspace },
   { "canHyperspace", aiL_canHyperspace },
   { "hyperspaceAbort", aiL_hyperspaceAbort },
   { "dock", aiL_dock },
   /* combat */
   { "aim", aiL_aim },
   { "combat", aiL_combat },
   { "settarget", aiL_settarget },
   { "weapset", aiL_weapSet },
   { "hascannons", aiL_hascannons },
   { "hasturrets", aiL_hasturrets },
   { "hasfighterbays", aiL_hasfighterbays },
   { "hasafterburner", aiL_hasafterburner },
   { "shoot", aiL_shoot },
   { "getenemy", aiL_getenemy },
   { "hostile", aiL_hostile },
   { "getweaprange", aiL_getweaprange },
   { "getweapspeed", aiL_getweapspeed },
   { "getweapammo", aiL_getweapammo },
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
   { "setasterotarget", aiL_setasterotarget },
   { "gatherablepos", aiL_gatherablePos },
   { "shoot_indicator", aiL_shoot_indicator },
   { "set_shoot_indicator", aiL_set_shoot_indicator },
   { "stealth", aiL_stealth },
   {0,0} /* end */
}; /**< Lua AI Function table. */

/*
 * current pilot "thinking" and assorted variables
 */
Pilot *cur_pilot           = NULL; /**< Current pilot.  All functions use this. */
static double pilot_acc    = 0.; /**< Current pilot's acceleration. */
static double pilot_turn   = 0.; /**< Current pilot's turning. */
static int pilot_flags     = 0; /**< Handle stuff like weapon firing. */
static char aiL_distressmsg[STRMAX_SHORT]; /**< Buffer to store distress message. */

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
   Task *prev  = NULL;
   Task *t     = pilot->task;
   while (t != NULL) {
      if (t->done) {
         Task * pointer = t;
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
Task* ai_curTask( Pilot* pilot )
{
   /* Get last task. */
   for (Task *t=pilot->task; t!=NULL; t=t->next)
      if (!t->done)
         return t;
   return NULL;
}

/**
 * @brief Sets the cur_pilot's ai.
 */
static int ai_setMemory (void)
{
   int oldmem;
   nlua_env env = cur_pilot->ai->env;

   nlua_getenv( naevL, env, "mem" ); /* oldmem */
   oldmem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* */

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, cur_pilot->lua_mem );
   nlua_setenv(naevL, env, "mem"); /* pm */

   return oldmem;
}

/**
 * @brief Sets the pilot for further AI calls.
 *
 *    @param p Pilot to set.
 */
AIMemory ai_setPilot( Pilot *p )
{
   AIMemory oldmem;
   cur_pilot = p;
   oldmem.p = cur_pilot;
   oldmem.mem = ai_setMemory();
   return oldmem;
}

/**
 * @brief Finishes setting up a pilot.
 */
void ai_unsetPilot( AIMemory oldmem )
{
   nlua_env env = cur_pilot->ai->env;
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, oldmem.mem );
   nlua_setenv( naevL, env, "mem"); /* pm */
   luaL_unref( naevL, LUA_REGISTRYINDEX, oldmem.mem );
   cur_pilot = oldmem.p;
}

/**
 * @brief Sets up the pilot for thinking.
 */
void ai_thinkSetup (void)
{
   /* Clean up some variables */
   pilot_acc   = 0.;
   pilot_turn  = 0.;
   pilot_flags = 0;
}

/**
 * @brief Applies the result of thinking.
 *
 *    @param p Pilot to apply to.
 */
void ai_thinkApply( Pilot *p )
{
   /* Make sure pilot_acc and pilot_turn are legal */
   pilot_acc   = CLAMP( -1., 1., pilot_acc );
   pilot_turn  = CLAMP( -1., 1., pilot_turn );

   /* Set turn and accel. */
   pilot_setTurn( p, pilot_turn );
   pilot_setAccel( p, pilot_acc );

   /* fire weapons if needed */
   if (ai_isFlag(AI_PRIMARY))
      pilot_shoot(p, 0); /* primary */
   if (ai_isFlag(AI_SECONDARY))
      pilot_shoot(p, 1 ); /* secondary */

   /* other behaviours. */
   if (ai_isFlag(AI_DISTRESS))
      pilot_distress(p, NULL, aiL_distressmsg);
}

/**
 * @brief Attempts to run a function.
 *
 *    @param[in] env Lua env to run function in.
 *    @param[in] nargs Number of arguments to run.
 */
static void ai_run( nlua_env env, int nargs )
{
   if (nlua_pcall(env, nargs, 0)) { /* error has occurred */
      WARN( _("Pilot '%s' ai '%s' error: %s"), cur_pilot->name, cur_pilot->ai->name, lua_tostring(naevL,-1));
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

   strncpy(buf, ai, sizeof(buf)-1);
   buf[sizeof(buf)-1] = '\0';

   /* Set up the profile. */
   prof = ai_getProfile(buf);
   if (prof == NULL) {
      WARN( _("AI Profile '%s' not found, using dummy fallback."), buf);
      prof = ai_getProfile("dummy");
   }
   if (prof == NULL) {
      WARN( _("Dummy AI Profile not valid! Things are going to break.") );
      return -1;
   }
   p->ai = prof;

   /* Adds a new pilot memory in the memory table. */
   lua_newtable(naevL);              /* m  */

   /* Copy defaults over from the global memory table. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, prof->lua_mem ); /* m, d */
   lua_pushnil(naevL);               /* m, d, nil */
   while (lua_next(naevL,-2) != 0) { /* m, d, k, v */
      lua_pushvalue(naevL,-2);       /* m, d, k, v, k */
      lua_pushvalue(naevL,-2);       /* m, d, k, v, k, v */
      lua_remove(naevL, -3);         /* m, d, k, k, v */
      lua_settable(naevL,-5);        /* m, d, k */
   }                                 /* m, d */
   lua_pop(naevL,1);                 /* m */
   p->lua_mem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* */

   /* Create the pilot. */
   ai_create( p );
   pilot_setFlag(p, PILOT_CREATED_AI);

   /* Initialize randomly within a control tick. */
   /* This doesn't work as nicely as one would expect because the pilot
    * has no initial task and control ticks get synchronized if you
    * spawn a bunch at the same time, which is why we add randomness
    * elsewhere. */
   p->tcontrol = RNGF() * p->ai->control_rate;

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
   if (p->ai == NULL)
      return;

   /* Get rid of pilot's memory. */
   if (!pilot_isPlayer(p)) { /* Player is an exception as more than one ship shares pilot id. */
      luaL_unref( naevL, LUA_REGISTRYINDEX, p->lua_mem );
      p->lua_mem = LUA_NOREF;
   }

   /* Clear the tasks. */
   ai_cleartasks( p );
}

static int ai_sort( const void *p1, const void *p2 )
{
   const AI_Profile *ai1 = (const AI_Profile*) p1;
   const AI_Profile *ai2 = (const AI_Profile*) p2;
   return strcmp( ai1->name, ai2->name );
}

/**
 * @brief Initializes the AI stuff which is basically Lua.
 *
 *    @return 0 on no errors.
 */
int ai_load (void)
{
   char** files;
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */

   /* get the file list */
   files = PHYSFS_enumerateFiles( AI_PATH );

   /* Create array. */
   profiles = array_create( AI_Profile );

   /* load the profiles */
   for (size_t i=0; files[i]!=NULL; i++) {
      AI_Profile prof;
      char path[PATH_MAX];
      int ret;

      if (!ndata_matchExt( files[i], "lua" ))
         continue;

      snprintf( path, sizeof(path), AI_PATH"%s", files[i] );
      ret = ai_loadProfile(&prof,path); /* Load the profile */
      if (ret == 0)
         array_push_back( &profiles, prof );
      else
         WARN( _("Error loading AI profile '%s'"), path);

      /* Render if necessary. */
      naev_renderLoadscreen();
   }
   qsort( profiles, array_size(profiles), sizeof(AI_Profile), ai_sort );

   /* More clean up. */
   PHYSFS_freeList( files );

#if DEBUGGING
   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_("Loaded %d AI Profile in %.3f s", "Loaded %d AI Profiles in %.3f s", array_size(profiles) ), array_size(profiles), time/1000. );
   }
   else
      DEBUG( n_("Loaded %d AI Profile", "Loaded %d AI Profiles", array_size(profiles) ), array_size(profiles) );
#endif /* DEBUGGING */

   /* Create collision stuff. */
   il_create( &ai_qtquery, 1 );

   /* Load equipment thingy. */
   return ai_loadEquip();
}

/**
 * @brief Loads the equipment selector script.
 */
static int ai_loadEquip (void)
{
   char *buf;
   size_t bufsize;
   const char *filename = AI_EQUIP_PATH;

   /* Make sure doesn't already exist. */
   nlua_freeEnv(equip_env);

   /* Create new state. */
   equip_env = nlua_newEnv();
   nlua_loadStandard(equip_env);

   /* Load the file. */
   buf = ndata_read( filename, &bufsize );
   if (nlua_dobufenv(equip_env, buf, bufsize, filename) != 0) {
      WARN( _("Error loading file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check"),
            filename, lua_tostring(naevL, -1));
      return -1;
   }
   free(buf);

   return 0;
}

int nlua_loadAI( nlua_env env )
{
   nlua_register(env, "ai", aiL_methods, 0);
   return 0;
}

/**
 * @brief Initializes an AI_Profile and adds it to the stack.
 *
 *    @param prof AI profile to load.
 *    @param[in] filename File to create the profile from.
 *    @return 0 on no error.
 */
static int ai_loadProfile( AI_Profile *prof, const char* filename )
{
   char* buf = NULL;
   size_t bufsize = 0;
   nlua_env env;
   size_t len;
   const char *str;

   /* Set name. */
   len = strlen(filename)-strlen(AI_PATH)-strlen(".lua");
   prof->name = malloc(len+1);
   strncpy( prof->name, &filename[strlen(AI_PATH)], len );
   prof->name[len] = '\0';

   /* Create Lua. */
   env = nlua_newEnv();
   nlua_loadStandard(env);
   prof->env = env;

   /* Register C functions in Lua */
   nlua_register(env, "ai", aiL_methods, 0);

   /* Mark as an ai. */
   lua_pushboolean( naevL, 1 );
   nlua_setenv( naevL, env, "__ai" );

   /* Set "mem" to be default template. */
   lua_newtable(naevL);              /* m */
   lua_pushvalue(naevL,-1);          /* m, m */
   prof->lua_mem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* m */
   nlua_setenv(naevL, env, "mem");   /*  */

   /* Now load the file since all the functions have been previously loaded */
   buf = ndata_read( filename, &bufsize );
   if (nlua_dobufenv(env, buf, bufsize, filename) != 0) {
      WARN( _("Error loading AI file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check"),
            filename, lua_tostring(naevL,-1));
      free(prof->name);
      nlua_freeEnv( env );
      free(buf);
      return -1;
   }
   free(buf);

   /* Find and set up the necessary references. */
   str = _("AI Profile '%s' is missing '%s' function!");
   prof->ref_control = nlua_refenvtype( env, "control", LUA_TFUNCTION );
   if (prof->ref_control == LUA_NOREF)
      WARN( str, filename, "control" );
   prof->ref_control_manual = nlua_refenvtype( env, "control_manual", LUA_TFUNCTION );
   if (prof->ref_control_manual == LUA_NOREF)
      WARN( str, filename, "control_manual" );
   prof->ref_refuel = nlua_refenvtype( env, "refuel", LUA_TFUNCTION );
   if (prof->ref_refuel == LUA_NOREF)
      WARN( str, filename, "refuel" );
   prof->ref_create = nlua_refenvtype( env, "create", LUA_TFUNCTION );
   if (prof->ref_create == LUA_NOREF)
      WARN( str, filename, "create" );

   /* Get the control rate. */
   nlua_getenv(naevL, env, "control_rate");
   prof->control_rate = lua_tonumber(naevL,-1);
   lua_pop(naevL,1);

   return 0;
}

/**
 * @brief Gets the AI_Profile by name.
 *
 *    @param[in] name Name of the profile to get.
 *    @return The profile or NULL on error.
 */
AI_Profile* ai_getProfile( const char *name )
{
   const AI_Profile ai = { .name = (char*)name };
   AI_Profile *ret = bsearch( &ai, profiles, array_size(profiles), sizeof(AI_Profile), ai_sort );
   if (ret==NULL)
      WARN( _("AI Profile '%s' not found in AI stack"), name);
   return ret;
}

/**
 * @brief Cleans up global AI.
 */
void ai_exit (void)
{
   /* Free AI profiles. */
   for (int i=0; i<array_size(profiles); i++) {
      free(profiles[i].name);
      nlua_freeEnv(profiles[i].env);
   }
   array_free( profiles );

   /* Free equipment Lua. */
   nlua_freeEnv(equip_env);
   equip_env = LUA_NOREF;

   /* Clean up query stuff. */
   il_destroy( &ai_qtquery );
}

/**
 * @brief Heart of the AI, brains of the pilot.
 *
 *    @param pilot Pilot that needs to think.
 *    @param dotask Whether or not to do the task, or just control tick.
 */
void ai_think( Pilot* pilot, int dotask )
{
   nlua_env env;
   AIMemory oldmem;
   Task *t;

   /* Must have AI. */
   if (pilot->ai == NULL)
      return;

   oldmem = ai_setPilot(pilot);
   env = cur_pilot->ai->env; /* set the AI profile to the current pilot's */

   ai_thinkSetup();
   pilot_rmFlag( pilot, PILOT_SCANNING ); /* Reset each frame, only set if the pilot is checking ai.scandone. */
   /* So the way this works is that, for other than the player, we reset all
    * the weapon sets every frame, so that the AI has to redo them over and
    * over. Now, this is a horrible hack so shit works and needs a proper fix.
    * TODO fix. */
   /* pilot_setTarget( cur_pilot, cur_pilot->id ); */
   if (cur_pilot->id != PLAYER_ID)
      pilot_weapSetAIClear( cur_pilot );

   /* Get current task. */
   t = ai_curTask( cur_pilot );

   /* control function if pilot is idle or tick is up */
   if ((cur_pilot->tcontrol < 0.) || (t == NULL)) {
      double crate = cur_pilot->ai->control_rate;
      if (pilot_isFlag(pilot,PILOT_PLAYER) ||
          pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL)) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, cur_pilot->ai->ref_control_manual );
         lua_pushnumber( naevL, crate-cur_pilot->tcontrol );
         ai_run(env, 1);
      } else {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, cur_pilot->ai->ref_control );
         lua_pushnumber( naevL, crate-cur_pilot->tcontrol );
         ai_run(env, 1); /* run control */
      }
      /* Try to desync control ticks when possible by adding randomness. */
      cur_pilot->tcontrol = crate * (0.9+0.2*RNGF());

      /* Task may have changed due to control tick. */
      t = ai_curTask( cur_pilot );
   }

   if (!dotask) {
      ai_unsetPilot( oldmem );
      return;
   }

   /* pilot has a currently running task */
   if (t != NULL) {
      int data;
      /* Run subtask if available, otherwise run main task. */
      if (t->subtask != NULL) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, t->subtask->func );
         /* Use subtask data or task data if subtask is not set. */
         data = t->subtask->dat;
         if (data == LUA_NOREF)
            data = t->dat;
      }
      else {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, t->func );
         data = t->dat;
      }
      /* Function should be on the stack. */
      if (data != LUA_NOREF) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, data );
         ai_run(env, 1);
      } else
         ai_run(env, 0);

      /* Manual control must check if IDLE hook has to be run. */
      if (pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL)) {
         /* We must yet check again to see if there still is a current task running. */
         if (ai_curTask( cur_pilot ) == NULL)
            pilot_runHook( cur_pilot, PILOT_HOOK_IDLE );
      }
   }

   /* Applies local variables to the pilot. */
   ai_thinkApply( cur_pilot );

   /* Restore memory. */
   ai_unsetPilot( oldmem );

   /* Clean up if necessary. */
   ai_taskGC( cur_pilot );
}

/**
 * @brief Initializes the AI.
 *
 *    @param p Pilot to run initialization when jumping/entering.
 */
void ai_init( Pilot *p )
{
   AIMemory oldmem;
   if ((p->ai==NULL) || (p->ai->ref_create==LUA_NOREF))
      return;
   oldmem = ai_setPilot( p );
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, p->ai->ref_create );
   ai_run( p->ai->env, 0 ); /* run control */
   ai_unsetPilot( oldmem );
}

/**
 * @brief Triggers the attacked() function in the pilot's AI.
 *
 *    @param attacked Pilot that is attacked.
 *    @param[in] attacker ID of the attacker.
 *    @param[in] dmg Damage done by the attacker.
 */
void ai_attacked( Pilot *attacked, const unsigned int attacker, double dmg )
{
   AIMemory oldmem;
   HookParam hparam[2];

   /* Custom hook parameters. */
   hparam[0].type    = HOOK_PARAM_PILOT;
   hparam[0].u.lp    = attacker;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = dmg;

   /* Behaves differently if manually overridden. */
   pilot_runHookParam( attacked, PILOT_HOOK_ATTACKED, hparam, 2 );

   /* Must have an AI profile. */
   if (attacked->ai == NULL)
      return;

   oldmem = ai_setPilot( attacked ); /* Sets cur_pilot. */
   if (pilot_isFlag( attacked, PILOT_MANUAL_CONTROL ))
      nlua_getenv(naevL, cur_pilot->ai->env, "attacked_manual");
   else
      nlua_getenv(naevL, cur_pilot->ai->env, "attacked");

   lua_pushpilot(naevL, attacker);
   if (nlua_pcall(cur_pilot->ai->env, 1, 0)) {
      WARN( _("Pilot '%s' ai '%s' -> 'attacked': %s"), cur_pilot->name, cur_pilot->ai->name, lua_tostring(naevL, -1));
      lua_pop(naevL, 1);
   }
   ai_unsetPilot( oldmem );
}

/**
 * @brief Triggers the discovered() function in the pilot's AI.
 *
 *    @param discovered Pilot that is discovered.
 */
void ai_discovered( Pilot* discovered )
{
   AIMemory oldmem;

   /* Behaves differently if manually overridden. */
   pilot_runHook( discovered, PILOT_HOOK_DISCOVERED );
   if (pilot_isFlag( discovered, PILOT_MANUAL_CONTROL ))
      return;

   /* Must have an AI profile and not be player. */
   if (discovered->ai == NULL)
      return;

   oldmem = ai_setPilot( discovered ); /* Sets cur_pilot. */

   /* Only run if discovered function exists. */
   nlua_getenv(naevL, cur_pilot->ai->env, "discovered");
   if (lua_isnil(naevL,-1)) {
      lua_pop(naevL,1);
      ai_unsetPilot( oldmem );
      return;
   }

   if (nlua_pcall(cur_pilot->ai->env, 0, 0)) {
      WARN( _("Pilot '%s' ai '%s' -> 'discovered': %s"), cur_pilot->name, cur_pilot->ai->name, lua_tostring(naevL, -1));
      lua_pop(naevL, 1);
   }
   ai_unsetPilot( oldmem );
}

/**
 * @brief Triggers the hail() function in the pilot's AI.
 *
 *    @param recipient Pilot that is being hailed.
 */
void ai_hail( Pilot* recipient )
{
   AIMemory oldmem;

   /* Make sure it's getable. */
   if (!pilot_canTarget( recipient ))
      return;

   /* Must have an AI profile and not be player. */
   if (recipient->ai == NULL)
      return;

   oldmem = ai_setPilot( recipient ); /* Sets cur_pilot. */

   /* Only run if hail function exists. */
   nlua_getenv(naevL, cur_pilot->ai->env, "hail");
   if (lua_isnil(naevL,-1)) {
      lua_pop(naevL,1);
      ai_unsetPilot( oldmem );
      return;
   }

   if (nlua_pcall(cur_pilot->ai->env, 0, 0)) {
      WARN( _("Pilot '%s' ai '%s' -> 'hail': %s"), cur_pilot->name, cur_pilot->ai->name, lua_tostring(naevL, -1));
      lua_pop(naevL, 1);
   }
   ai_unsetPilot( oldmem );
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

   if (cur_pilot->ai->ref_refuel==LUA_NOREF) {
      WARN(_("Pilot '%s' (ai '%s') is trying to refuel when no 'refuel' function is defined!"), cur_pilot->name, cur_pilot->ai->name);
      return;
   }

   /* Create the task. */
   t           = calloc( 1, sizeof(Task) );
   t->name     = strdup("refuel");
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, cur_pilot->ai->ref_refuel);
   t->func     = luaL_ref(naevL, LUA_REGISTRYINDEX);
   lua_pushpilot(naevL, target);
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
 *    @param attacker Pilot attacking the distressed.
 */
void ai_getDistress( const Pilot *p, const Pilot *distressed, const Pilot *attacker )
{
   /* Ignore distress signals when under manual control. */
   if (pilot_isFlag( p, PILOT_MANUAL_CONTROL ))
      return;

   /* Must have AI. */
   if (cur_pilot->ai == NULL)
      return;

   if (attacker != NULL)
      lua_pushpilot(naevL, attacker->id);
   else
      lua_pushnil(naevL);
   pilot_msg( distressed, p, "distress", -1 );
   lua_pop(naevL,1);
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
   /* Set creation mode. */
   if (!pilot_isFlag(pilot, PILOT_CREATED_AI))
      aiL_status = AI_STATUS_CREATE;

   /* Create equipment first - only if creating for the first time. */
   if (!pilot_isFlag(pilot,PILOT_NO_OUTFITS) && !pilot_isFlag(pilot,PILOT_NO_EQUIP) && (aiL_status==AI_STATUS_CREATE)) {
      nlua_env env = equip_env;
      char *func = "equip_generic";

      if (faction_getEquipper( pilot->faction ) != LUA_NOREF) {
         env = faction_getEquipper( pilot->faction );
         func = "equip";
      }
      nlua_getenv(naevL, env, func);
      nlua_pushenv(naevL, env);
      lua_setfenv(naevL, -2);
      lua_pushpilot(naevL, pilot->id);
      if (nlua_pcall(env, 1, 0)) { /* Error has occurred. */
         WARN( _("Pilot '%s' equip '%s' -> '%s': %s"), pilot->name, pilot->ai->name, func, lua_tostring(naevL, -1));
         lua_pop(naevL, 1);
      }

      /* Since the pilot changes outfits and cores, we must heal him up. */
      pilot_healLanded( pilot );
   }

   /* Must have AI. */
   if (pilot->ai == NULL)
      return;

   /* Set up. */
   ai_init( pilot );

   /* Recover normal mode. */
   if (!pilot_isFlag(pilot, PILOT_CREATED_AI))
      aiL_status = AI_STATUS_NORMAL;
}

/**
 * @brief Creates a new AI task.
 */
Task *ai_newtask( lua_State *L, Pilot *p, const char *func, int subtask, int pos )
{
   Task *t, *pointer;

   if (p->ai==NULL) {
      NLUA_ERROR( L, _("Trying to create new task for pilot '%s' that has no AI!"), p->name );
      return NULL;
   }

   /* Check if the function is good. */
   nlua_getenv( L, p->ai->env, func );
   luaL_checktype( L, -1, LUA_TFUNCTION );

   /* Create the new task. */
   t           = calloc( 1, sizeof(Task) );
   t->name     = strdup(func);
   t->func     = luaL_ref( L, LUA_REGISTRYINDEX );
   t->dat      = LUA_NOREF;

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
      Task *curtask = ai_curTask( p );
      if (curtask == NULL) {
         ai_freetask( t );
         NLUA_ERROR( L, _("Trying to add subtask '%s' to non-existent task."), func);
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
   if (t->func != LUA_NOREF)
      luaL_unref(naevL, LUA_REGISTRYINDEX, t->func);

   if (t->dat != LUA_NOREF)
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

   free(t->name);
   free(t);
}

/**
 * @brief Creates a new task based on stack information.
 */
static Task* ai_createTask( lua_State *L, int subtask )
{
   /* Parse basic parameters. */
   const char *func = luaL_checkstring(L,1);

   if (pilot_isPlayer(cur_pilot) && !pilot_isFlag(cur_pilot,PILOT_MANUAL_CONTROL))
      return NULL;

   /* Creates a new AI task. */
   Task *t = ai_newtask( L, cur_pilot, func, subtask, 0 );
   if (t==NULL) {
      NLUA_ERROR( L, _("Failed to create new task for pilot '%s'."), cur_pilot->name );
      return NULL;
   }

   /* Set the data. */
   if (lua_gettop(L) > 1) {
      lua_pushvalue(L,2);
      t->dat = luaL_ref(L, LUA_REGISTRYINDEX);
   }

   return t;
}

/**
 * @brief Pushes a task target.
 */
static int ai_tasktarget( lua_State *L, const Task *t )
{
   if (t->dat == LUA_NOREF)
      return 0;
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
 * @{
 */
/**
 * @brief Pushes a task onto the pilot's task list.
 *    @luatparam string func Name of function to call for task.
 *    @luaparam[opt] data Data to pass to the function.  Supports any lua type.
 * @luafunc pushtask
 *    @return Number of Lua parameters.
 */
static int aiL_pushtask( lua_State *L )
{
   ai_createTask( L, 0 );
   return 0;
}
/**
 * @brief Pops the current running task.
 * @luafunc poptask
 *    @return Number of Lua parameters.
 */
static int aiL_poptask( lua_State *L )
{
   (void) L;
   Task *t = ai_curTask( cur_pilot );
   /* Tasks must exist. */
   if (t == NULL) {
      WARN(_("Trying to pop task when there are no tasks on the stack."));
      return 0;
   }
   /*
   if (strcmp(cur_pilot->ai->name,"escort")==0) {
      if (cur_pilot->task==t) {
         WARN("Popping last task!");
      }
   }
   */
   t->done = 1;
   return 0;
}

/**
 * @brief Gets the current task's name.
 *    @luatreturn string The current task name or nil if there are no tasks.
 * @luafunc taskname
 *    @return Number of Lua parameters.
 */
static int aiL_taskname( lua_State *L )
{
   const Task *t = ai_curTask( cur_pilot );
   if (t)
      lua_pushstring(L, t->name);
   else
      lua_pushnil(L);
   return 1;
}

/**
 * @brief Gets the pilot's task data.
 *    @luareturn The pilot's task data or nil if there is no task data.
 *    @luasee pushtask
 * @luafunc taskdata
 *    @return Number of Lua parameters.
 */
static int aiL_taskdata( lua_State *L )
{
   const Task *t = ai_curTask( cur_pilot );

   /* Must have a task. */
   if (t == NULL)
      return 0;

   return ai_tasktarget( L, t );
}

/**
 * @brief Pushes a subtask onto the pilot's task's subtask list.
 *    @luatparam string func Name of function to call for task.
 *    @luaparam[opt] data Data to pass to the function.  Supports any lua type.
 * @luafunc pushsubtask
 *    @return Number of Lua parameters.
 */
static int aiL_pushsubtask( lua_State *L )
{
   ai_createTask(L, 1);
   return 0;
}

/**
 * @brief Pops the current running task.
 * @luafunc popsubtask
 *    @return Number of Lua parameters.
 */
static int aiL_popsubtask( lua_State *L )
{
   Task *t, *st;
   t = ai_curTask( cur_pilot );

   /* Tasks must exist. */
   if (t == NULL)
      return NLUA_ERROR(L, _("Trying to pop task when there are no tasks on the stack."));
   if (t->subtask == NULL)
      return NLUA_ERROR(L, _("Trying to pop subtask when there are no subtasks for the task '%s'."), t->name);

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
 * @luafunc subtaskname
 *    @return Number of Lua parameters.
 */
static int aiL_subtaskname( lua_State *L )
{
   const Task *t = ai_curTask( cur_pilot );
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
 * @luafunc subtaskdata
 *    @return Number of Lua parameters.
 */
static int aiL_subtaskdata( lua_State *L )
{
    const Task *t = ai_curTask( cur_pilot );
   /* Must have a subtask. */
   if ((t == NULL) || (t->subtask == NULL))
      return 0;

   return ai_tasktarget( L, t->subtask );
}

/**
 * @brief Gets the AI's pilot.
 *    @luatreturn Pilot The AI's pilot.
 * @luafunc pilot
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
 * @luafunc rndpilot
 *    @return Number of Lua parameters.
 */
static int aiL_getrndpilot( lua_State *L )
{
   Pilot *const* pilot_stack = pilot_getAll();
   int p = RNG(0, array_size(pilot_stack)-1);
   /* Make sure it can't be the same pilot. */
   if (pilot_stack[p]->id == cur_pilot->id) {
      p++;
      if (p >= array_size(pilot_stack))
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
 *    @luafunc nearestpilot
 */
static int aiL_getnearestpilot( lua_State *L )
{
   /* dist will be initialized to a number */
   /* this will only seek out pilots closer than dist */
   Pilot *const* pilot_stack = pilot_getAll();
   int dist = 1e6;
   int candidate_id = -1;

   /*cycle through all the pilots and find the closest one that is not the pilot */
   for (int i=0; i<array_size(pilot_stack); i++) {
      if (pilot_stack[i]->id == cur_pilot->id)
         continue;
      if (vec2_dist(&pilot_stack[i]->solid.pos, &cur_pilot->solid.pos) > dist)
         continue;
      dist = vec2_dist(&pilot_stack[i]->solid.pos, &cur_pilot->solid.pos);
      candidate_id = i;
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
 *    @luafunc dist
 */
static int aiL_getdistance( lua_State *L )
{
   const vec2 *v;

   /* vector as a parameter */
   if (lua_isvector(L,1))
      v = lua_tovector(L,1);
   /* pilot as parameter */
   else if (lua_ispilot(L,1)) {
      const Pilot *p = luaL_validpilot(L,1);
      v = &p->solid.pos;
   }
   /* wrong parameter */
   else
      NLUA_INVALID_PARAMETER(L,1);

   lua_pushnumber(L, vec2_dist(v, &cur_pilot->solid.pos));
   return 1;
}

/**
 * @brief Gets the squared distance from the pointer.
 *
 *    @luatparam Vec2|Pilot pointer
 *    @luatreturn number The squared distance from the pointer.
 *    @luafunc dist2
 */
static int aiL_getdistance2( lua_State *L )
{
   const vec2 *v;

   /* vector as a parameter */
   if (lua_isvector(L,1))
      v = lua_tovector(L,1);
   /* pilot as parameter */
   else if (lua_ispilot(L,1)) {
      const Pilot *p = luaL_validpilot(L,1);
      v = &p->solid.pos;
   }
   /* wrong parameter */
   else
      NLUA_INVALID_PARAMETER(L,1);

   lua_pushnumber(L, vec2_dist2(v, &cur_pilot->solid.pos));
   return 1;
}

/**
 * @brief Gets the distance from the pointer perpendicular to the current pilot's flight vector.
 *
 *    @luatparam Vec2|Pilot pointer
 *    @luatreturn number offset_distance
 *    @luafunc flyby_dist
 */
static int aiL_getflybydistance( lua_State *L )
{
   const vec2 *v;
   vec2 perp_motion_unit, offset_vect;
   int offset_distance;

   /* vector as a parameter */
   if (lua_isvector(L,1))
      v = lua_tovector(L,1);
   /* pilot id as parameter */
   else if (lua_ispilot(L,1)) {
      const Pilot *p = luaL_validpilot(L,1);
      v = &p->solid.pos;

      /*vec2_cset(&v, VX(pilot->solid.pos) - VX(cur_pilot->solid.pos), VY(pilot->solid.pos) - VY(cur_pilot->solid.pos) );*/
   }
   else
      NLUA_INVALID_PARAMETER(L,1);

   vec2_cset(&offset_vect, VX(*v) - VX(cur_pilot->solid.pos), VY(*v) - VY(cur_pilot->solid.pos) );
   vec2_pset(&perp_motion_unit, 1, VANGLE(cur_pilot->solid.vel)+M_PI_2);
   offset_distance = vec2_dot(&perp_motion_unit, &offset_vect);

   lua_pushnumber(L, offset_distance);
   return 1;
}

/**
 * @brief Gets the minimum braking distance.
 *
 * braking vel ==> 0 = v - a*dt
 * add turn around time (to initial vel) ==> 180.*360./cur_pilot->turn
 * add it to general euler equation  x = v * t + 0.5 * a * t^2
 * and voila!
 *
 * I hate this function and it'll probably need to get changed in the future
 *
 *    @luatreturn number Minimum braking distance.
 *    @luafunc minbrakedist
 */
static int aiL_minbrakedist( lua_State *L )
{
   /* More complicated calculation based on relative velocity. */
   if (lua_gettop(L) > 0) {
      double time, dist, vel;
      vec2 vv;
      const Pilot *p = luaL_validpilot(L,1);

      /* Set up the vectors. */
      vec2_cset( &vv, p->solid.vel.x - cur_pilot->solid.vel.x,
            p->solid.vel.y - cur_pilot->solid.vel.y );

      /* Run the same calculations. */
      time = VMOD(vv) / cur_pilot->accel;

      /* Get relative velocity. */
      vel = MIN(cur_pilot->speed - VMOD(p->solid.vel), VMOD(vv));
      if (vel < 0.)
         vel = 0.;
      /* Get distance to brake. */
      dist = vel*(time+1.1*M_PI/cur_pilot->turn) -
         0.5*(cur_pilot->accel)*time*time;
      lua_pushnumber(L, dist);
   }
   else
      lua_pushnumber( L, pilot_minbrakedist(cur_pilot) );
   return 1;
}

/**
 * @brief Checks to see if target has bribed pilot.
 *
 *    @luatparam Pilot target
 *    @luatreturn boolean Whether the target has bribed pilot.
 *    @luafunc isbribed
 */
static int aiL_isbribed( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushboolean(L, pilot_isWithPlayer(p) && pilot_isFlag(cur_pilot, PILOT_BRIBED));
   return 1;
}

/**
 * @brief Checks to see if pilot can instant jump.
 *
 *    @luatreturn boolean Whether the pilot can instant jump.
 *    @luafunc instantJump
 */
static int aiL_instantJump( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->stats.misc_instant_jump);
   return 1;
}

/**
 * @brief Checks to see if pilot is at maximum velocity.
 *
 *    @luatreturn boolean Whether the pilot is at maximum velocity.
 *    @luafunc ismaxvel
 */
static int aiL_ismaxvel( lua_State *L )
{
   //lua_pushboolean(L,(VMOD(cur_pilot->solid.vel) > (cur_pilot->speed-MIN_VEL_ERR)));
   lua_pushboolean(L,(VMOD(cur_pilot->solid.vel) >
                   (solid_maxspeed(&cur_pilot->solid, cur_pilot->speed, cur_pilot->accel)-MIN_VEL_ERR)));
   return 1;
}

/**
 * @brief Checks to see if pilot is stopped.
 *
 *    @luatreturn boolean Whether the pilot is stopped.
 *    @luafunc isstopped
 */
static int aiL_isstopped( lua_State *L )
{
   lua_pushboolean(L,(VMOD(cur_pilot->solid.vel) < MIN_VEL_ERR));
   return 1;
}

/**
 * @brief Checks to see if target is an enemy.
 *
 *    @luatparam Pilot target
 *    @luatreturn boolean Whether the target is an enemy.
 *    @luafunc isenemy
 */
static int aiL_isenemy( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);

   /* Player needs special handling in case of hostility. */
   if (pilot_isWithPlayer(p)) {
      lua_pushboolean(L, pilot_isHostile(cur_pilot));
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
 *    @luafunc isally
 */
static int aiL_isally( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);

   /* Player needs special handling in case of friendliness. */
   if (pilot_isWithPlayer(p)) {
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
 *    @luafunc haslockon
 */

static int aiL_haslockon( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->lockons > 0);
   return 1;
}

/**
 * @brief Checks to see if pilot has a projectile after him.
 *
 *    @luatreturn boolean Whether the pilot has a projectile after him.
 *    @luafunc hasprojectile
 */

static int aiL_hasprojectile( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->projectiles > 0);
   return 1;
}

/**
 * @brief Checks to see if pilot has finished scanning their target.
 *
 *    @luatreturn boolean Whether the pilot has scanned their target.
 * @luafunc scandone
 */

static int aiL_scandone( lua_State *L )
{
   pilot_setFlag( cur_pilot, PILOT_SCANNING ); /*< Indicate pilot is scanning this frame. */
   lua_pushboolean(L, pilot_ewScanCheck( cur_pilot ) );
   return 1;
}

/**
 * @brief Starts accelerating the pilot.
 *
 *    @luatparam[opt=1.] number acceleration Fraction of pilot's maximum acceleration from 0 to 1.
 *    @luafunc accel
 */
static int aiL_accel( lua_State *L )
{
   double n = luaL_optnumber( L, 1, 1. );
   pilot_acc = CLAMP( 0., 1., n );
   return 0;
}

/**
 * @brief Starts turning the pilot.
 *
 *    @luatparam number vel Directional velocity from -1 to 1.
 *    @luafunc turn
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
 *    @luatparam Pilot|Vec2|number target Target to face. Can be either a pilot, position (Vec2), or an orientation (number representing angle in radians).
 *    @luatparam[opt=false] boolean invert Invert away from target.
 *    @luatparam[opt=false] boolean compensate Compensate for velocity?
 *    @luatreturn number Angle offset in radians.
 * @luafunc face
 */
static int aiL_face( lua_State *L )
{
   const vec2 *tv; /* get the position to face */
   double k_diff, k_vel, diff, vx, vy, dx, dy;
   int vel;

   /* Default gain. */
   k_diff = 10.;
   k_vel  = 100.; /* overkill gain! */

   /* Check if must invert. */
   if (lua_toboolean(L,2))
      k_diff *= -1;

   /* Get first parameter, aka what to face. */
   if (lua_ispilot(L,1)) {
      Pilot* p = luaL_validpilot(L,1);
      /* Target vector. */
      tv = &p->solid.pos;
   }
   else if (lua_isnumber(L,1)) {
      double d = lua_tonumber(L,1);
      diff = angle_diff( cur_pilot->solid.dir, d );
      /* Make pilot turn. */
      pilot_turn = k_diff * diff;
      /* Return angle away from target. */
      lua_pushnumber(L, ABS(diff));
      return 1;
   }
   else if (lua_isvector(L,1))
      tv = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L,1);

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
   vx = cur_pilot->solid.vel.x;
   vy = cur_pilot->solid.vel.y;
   /* Direction vector. */
   dx = tv->x - cur_pilot->solid.pos.x;
   dy = tv->y - cur_pilot->solid.pos.y;
   if (vel && (dx || dy)) {
      /* Calculate dot product. */
      double d = (vx * dx + vy * dy) / (dx*dx + dy*dy);
      /* Calculate tangential velocity. */
      vx = vx - d * dx;
      vy = vy - d * dy;

      /* Add velocity compensation. */
      dx += -k_vel * vx;
      dy += -k_vel * vy;
   }

   /* Compensate error and rotate. */
   diff = angle_diff( cur_pilot->solid.dir, atan2( dy, dx ) );

   /* Make pilot turn. */
   pilot_turn = k_diff * diff;

   /* Return angle away from target. */
   lua_pushnumber(L, ABS(diff));
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
 *    @luatreturn number Angle offset in radians.
 * @luafunc careful_face
 */
static int aiL_careful_face( lua_State *L )
{
   vec2 *tv, F, F1;
   Pilot* p;
   double d, diff, dist;
   Pilot *const* pilot_stack;
   int x, y, r;

   /* Default gains. */
   const double k_diff = 10.;
   const double k_goal = 1.;
   const double k_enemy = 6e6;

   /* Init some variables */
   pilot_stack = pilot_getAll();
   p = cur_pilot;

   /* Get first parameter, aka what to face. */
   if (lua_ispilot(L,1)) {
      p = luaL_validpilot(L,1);
      /* Target vector. */
      tv = &p->solid.pos;
   }
   else if (lua_isnumber(L,1)) {
      d = (double)lua_tonumber(L,1);
      if (d < 0.)
         tv = &cur_pilot->solid.pos;
      else
         NLUA_INVALID_PARAMETER(L,1);
   }
   else if (lua_isvector(L,1))
      tv = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L,1);

   /* Init the force, where F1 is roughly normalized to norm 1. */
   vec2_csetmin( &F, 0., 0.) ;
   vec2_cset( &F1, tv->x - cur_pilot->solid.pos.x, tv->y - cur_pilot->solid.pos.y) ;
   dist = VMOD(F1) + 0.1; /* Avoid / 0 */
   vec2_cset( &F1, F1.x * k_goal / dist, F1.y * k_goal / dist) ;

   /* Cycle through all the pilots in order to compute the force */
   x = round(cur_pilot->solid.pos.x);
   y = round(cur_pilot->solid.pos.y);
   /* It's modulated by k_enemy * k_mult / dist^2, where k_mult<1 and k_enemy=6e6
    * A distance of 5000 should give a maximum factor of 0.24, but it should be
    * far away enough to not matter (hopefully).. */
   r = 5000;
   pilot_collideQueryIL( &ai_qtquery, x-r, y-r, x+r, y+r );
   for (int i=0; i<il_size(&ai_qtquery); i++ ) {
      const Pilot *p_i = pilot_stack[ il_get( &ai_qtquery, i, 0 ) ];

      /* Valid pilot isn't self, is in range, isn't the target and isn't disabled */
      if (p_i->id == cur_pilot->id)
         continue;
      if (p_i->id == p->id)
         continue;
      if (pilot_isDisabled(p_i) )
         continue;
      if (pilot_inRangePilot(cur_pilot, p_i, &dist) != 1)
         continue;

      /* If the enemy is too close, ignore it*/
      if (dist < pow2(750.))
         continue;
      dist = sqrt(dist); /* Have to undo the square. */

      /* Check if friendly or not */
      if (areEnemies(cur_pilot->faction, p_i->faction)) {
         double k_mult = pilot_relhp( p_i, cur_pilot ) * pilot_reldps( p_i, cur_pilot );
         double factor = k_enemy * k_mult / (dist*dist*dist);
         vec2_csetmin( &F, F.x + factor * (cur_pilot->solid.pos.x - p_i->solid.pos.x),
                F.y + factor * (cur_pilot->solid.pos.y - p_i->solid.pos.y) );
      }
   }

   vec2_cset( &F, F.x + F1.x, F.y + F1.y );

   /* Rotate. */
   diff = angle_diff( cur_pilot->solid.dir, VANGLE(F) );

   /* Make pilot turn. */
   pilot_turn = k_diff * diff;

   /* Return angle away from target. */
   lua_pushnumber(L, ABS(diff));
   return 1;
}

/**
 * @brief Aims at a pilot, trying to hit it rather than move to it.
 *
 * This method uses a polar UV decomposition to get a more accurate time-of-flight
 *
 *    @luatparam Pilot|Asteroid target The pilot to aim at
 *    @luatreturn number The offset from the target aiming position (in radians).
 * @luafunc aim
 */
static int aiL_aim( lua_State *L )
{
   double diff, mod, angle;

   if (lua_isasteroid(L,1)) {
      const Asteroid *a = luaL_validasteroid(L,1);
      angle = pilot_aimAngle( cur_pilot, &a->sol.pos, &a->sol.vel );
   }
   else {
      const Pilot *p = luaL_validpilot(L,1);
      angle = pilot_aimAngle( cur_pilot, &p->solid.pos, &p->solid.vel );
   }

   /* Calculate what we need to turn */
   mod = 10.;
   diff = angle_diff(cur_pilot->solid.dir, angle);
   pilot_turn = mod * diff;

   lua_pushnumber(L, ABS(diff));
   return 1;
}

/**
 * @brief Maintains an intercept pursuit course.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to intercept.
 *    @luatreturn number The offset from the proper intercept course (in radians).
 * @luafunc iface
 */
static int aiL_iface( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   vec2 *vec, drift, reference_vector; /* get the position to face */
   Pilot* p;
   double diff, heading_offset_azimuth, drift_radial, drift_azimuthal;

   /* Get first parameter, aka what to face. */
   p  = NULL;
   vec = NULL;
   if (lua_ispilot(L,1))
      p = luaL_validpilot(L,1);
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L,1);

   if (vec==NULL) {
      if (p == NULL)
         return 0; /* Return silently when attempting to face an invalid pilot. */
      /* Establish the current pilot velocity and position vectors */
      vec2_cset( &drift, VX(p->solid.vel) - VX(cur_pilot->solid.vel), VY(p->solid.vel) - VY(cur_pilot->solid.vel));
      /* Establish the in-line coordinate reference */
      vec2_cset( &reference_vector, VX(p->solid.pos) - VX(cur_pilot->solid.pos), VY(p->solid.pos) - VY(cur_pilot->solid.pos));
   }
   else {
      /* Establish the current pilot velocity and position vectors */
      vec2_cset( &drift, -VX(cur_pilot->solid.vel), -VY(cur_pilot->solid.vel));
      /* Establish the in-line coordinate reference */
      vec2_cset( &reference_vector, VX(*vec) - VX(cur_pilot->solid.pos), VY(*vec) - VY(cur_pilot->solid.pos));
   }

   /* Break down the the velocity vectors of both craft into UV coordinates */
   vec2_uv(&drift_radial, &drift_azimuthal, &drift, &reference_vector);
   heading_offset_azimuth = angle_diff(cur_pilot->solid.dir, VANGLE(reference_vector));

   /* Now figure out what to do...
    * Are we pointing anywhere inside the correct UV quadrant?
    * if we're outside the correct UV quadrant, we need to get into it ASAP
    * Otherwise match velocities and approach */
   if (FABS(heading_offset_azimuth) < M_PI_2) {
      /* This indicates we're in the correct plane*/
      /* 1 - 1/(|x|+1) does a pretty nice job of mapping the reals to the interval (0...1). That forms the core of this angle calculation */
      /* There is nothing special about the scaling parameter of 200; it can be tuned to get any behavior desired. A lower
         number will give a more dramatic 'lead' */
      double speedmap = -1.*copysign(1. - 1. / (FABS(drift_azimuthal/200.) + 1.), drift_azimuthal) * M_PI_2;
      diff = angle_diff(heading_offset_azimuth, speedmap);

      pilot_turn = -10.*diff;
   }
   /* turn most efficiently to face the target. If we intercept the correct quadrant in the UV plane first, then the code above will kick in */
   /* some special case logic is added to optimize turn time. Reducing this to only the else cases would speed up the operation
      but cause the pilot to turn in the less-than-optimal direction sometimes when between 135 and 225 degrees off from the target */
   else {
      /* signal that we're not in a productive direction for accelerating */
      diff = M_PI;
      azimuthal_sign = 1;

      if (heading_offset_azimuth > 0.)
         pilot_turn = 1.;
      else
         pilot_turn = -1.;
   }

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, ABS(diff));
   return 1;
}

/**
 * @brief calculates the direction that the target is relative to the current pilot facing.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to compare facing to
 *    @luatreturn number The facing offset to the target (in radians).
 * @luafunc dir
 *
 */
static int aiL_dir( lua_State *L )
{
   vec2 sv, tv; /* get the position to face */
   double diff;

   /* Get first parameter, aka what to face. */
   vec2_cset( &sv, VX(cur_pilot->solid.pos), VY(cur_pilot->solid.pos) );
   if (lua_ispilot(L,1)) {
      const Pilot *p = luaL_validpilot(L,1);
      vec2_cset( &tv, VX(p->solid.pos), VY(p->solid.pos) );
      diff = angle_diff(cur_pilot->solid.dir,
            vec2_angle(&sv, &tv));
   }
   else if (lua_isvector(L,1)) {
      const vec2 *vec = lua_tovector(L,1);
      diff = angle_diff( cur_pilot->solid.dir,
            vec2_angle(&cur_pilot->solid.pos, vec));
   }
   else
      NLUA_INVALID_PARAMETER(L,1);

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, diff);
   return 1;
}

/**
 * @brief Calculates angle between pilot facing and intercept-course to target.
 *
 *    @luatparam Pilot|Vec2 target Position or pilot to compare facing to
 *    @luatreturn number The facing offset to intercept-course to the target (in radians).
 * @luafunc idir
 */
static int aiL_idir( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   vec2 *vec, drift, reference_vector; /* get the position to face */
   Pilot* p;
   double diff, heading_offset_azimuth, drift_radial, drift_azimuthal;

   /* Get first parameter, aka what to face. */
   p  = NULL;
   vec = NULL;
   if (lua_ispilot(L,1))
      p = luaL_validpilot(L,1);
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else
      NLUA_INVALID_PARAMETER(L,1);

   if (vec==NULL) {
      if (p==NULL)
         return 0; /* Return silently when attempting to face an invalid pilot. */
      /* Establish the current pilot velocity and position vectors */
      vec2_cset( &drift, VX(p->solid.vel) - VX(cur_pilot->solid.vel), VY(p->solid.vel) - VY(cur_pilot->solid.vel));
      /* Establish the in-line coordinate reference */
      vec2_cset( &reference_vector, VX(p->solid.pos) - VX(cur_pilot->solid.pos), VY(p->solid.pos) - VY(cur_pilot->solid.pos));
   }
   else {
      /* Establish the current pilot velocity and position vectors */
      vec2_cset( &drift, -VX(cur_pilot->solid.vel), -VY(cur_pilot->solid.vel));
      /* Establish the in-line coordinate reference */
      vec2_cset( &reference_vector, VX(*vec) - VX(cur_pilot->solid.pos), VY(*vec) - VY(cur_pilot->solid.pos));
   }

   /* Break down the the velocity vectors of both craft into UV coordinates */
   vec2_uv(&drift_radial, &drift_azimuthal, &drift, &reference_vector);
   heading_offset_azimuth = angle_diff(cur_pilot->solid.dir, VANGLE(reference_vector));

   /* now figure out what to do*/
   /* are we pointing anywhere inside the correct UV quadrant? */
   /* if we're outside the correct UV quadrant, we need to get into it ASAP */
   /* Otherwise match velocities and approach*/
   if (FABS(heading_offset_azimuth) < M_PI_2) {
      /* This indicates we're in the correct plane
       * 1 - 1/(|x|+1) does a pretty nice job of mapping the reals to the interval (0...1). That forms the core of this angle calculation
       * there is nothing special about the scaling parameter of 200; it can be tuned to get any behavior desired. A lower
       * number will give a more dramatic 'lead' */
      double speedmap = -1.*copysign(1. - 1. / (FABS(drift_azimuthal/200.) + 1.), drift_azimuthal) * M_PI_2;
      diff = angle_diff(heading_offset_azimuth, speedmap);

   }
   /* Turn most efficiently to face the target. If we intercept the correct quadrant in the UV plane first, then the code above will kick in
      some special case logic is added to optimize turn time. Reducing this to only the else cases would speed up the operation
      but cause the pilot to turn in the less-than-optimal direction sometimes when between 135 and 225 degrees off from the target */
   else {
      /* signal that we're not in a productive direction for accelerating */
      diff        = M_PI;
   }

   /* Return angle in degrees away from target. */
   lua_pushnumber(L, diff);
   return 1;
}

/**
 * @brief Calculate the offset between the pilot's current direction of travel and the pilot's current facing.
 *
 *    @luatreturn number Offset (radians)
 *    @luafunc drift_facing
 */
static int aiL_drift_facing( lua_State *L )
{
   double drift = angle_diff(VANGLE(cur_pilot->solid.vel), cur_pilot->solid.dir);
   lua_pushnumber(L, drift);
   return 1;
}

/**
 * @brief Brakes the pilot.
 *
 *    @luatreturn boolean Whether braking is finished.
 *    @luafunc brake
 */
static int aiL_brake( lua_State *L )
{
   int ret = pilot_brake( cur_pilot );

   pilot_acc = cur_pilot->solid.accel / cur_pilot->accel;
   pilot_turn = cur_pilot->solid.dir_vel / cur_pilot->turn;

   lua_pushboolean(L, ret);
   return 1;
}

/**
 * @brief Get the nearest friendly spob to the pilot.
 *
 *    @luatreturn Spob|nil
 *    @luafunc nearestspob
 */
static int aiL_getnearestspob( lua_State *L )
{
   double dist, d;
   int j;
   LuaSpob spob;

   /* cycle through spobs */
   dist = HUGE_VAL;
   j = -1;
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      if (!spob_hasService(cur_system->spobs[i],SPOB_SERVICE_INHABITED))
         continue;
      d = vec2_dist( &cur_system->spobs[i]->pos, &cur_pilot->solid.pos );
      if ((!areEnemies(cur_pilot->faction,cur_system->spobs[i]->presence.faction)) &&
            (d < dist)) { /* closer friendly spob */
         j = i;
         dist = d;
      }
   }

   /* no friendly spob found */
   if (j == -1) return 0;

   cur_pilot->nav_spob = j;
   spob = cur_system->spobs[j]->id;
   lua_pushspob(L, spob);

   return 1;
}

/**
 * @brief Get the nearest friendly spob to a given position.
 *
 *    @luatparam vec2 pos Position close to the spob.
 *    @luatreturn Spob|nil
 *    @luafunc spobfrompos
 */
static int aiL_getspobfrompos( lua_State *L )
{
   int j;
   double dist;
   LuaSpob spob;
   const vec2 *pos = luaL_checkvector(L,1);

   /* cycle through spobs */
   dist = HUGE_VAL;
   j = -1;
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      double d;
      if (!spob_hasService(cur_system->spobs[i],SPOB_SERVICE_INHABITED))
         continue;
      d = vec2_dist( &cur_system->spobs[i]->pos, pos );
      if ((!areEnemies(cur_pilot->faction,cur_system->spobs[i]->presence.faction)) &&
            (d < dist)) { /* closer friendly spob */
         j = i;
         dist = d;
      }
   }

   /* no friendly spob found */
   if (j == -1) return 0;

   cur_pilot->nav_spob = j;
   spob = cur_system->spobs[j]->id;
   lua_pushspob(L, spob);

   return 1;
}

/**
 * @brief Get a random spob.
 *
 *    @luatreturn Spob|nil
 *    @luafunc rndspob
 */
static int aiL_getrndspob( lua_State *L )
{
   LuaSpob spob;
   int p;

   /* No spobs. */
   if (array_size(cur_system->spobs) == 0)
      return 0;

   /* get a random spob */
   p = RNG(0, array_size(cur_system->spobs)-1);

   /* Copy the data into a vector */
   spob = cur_system->spobs[p]->id;
   lua_pushspob(L, spob);

   return 1;
}

/**
 * @brief Get a random friendly spob.
 *
 *    @luatparam[opt=false] boolean only_friend Only check for ally spobs.
 *    @luatreturn Spob|nil
 * @luafunc landspob
 */
static int aiL_getlandspob( lua_State *L )
{
   int *ind;
   int id;
   LuaSpob spob;
   const Spob *p;
   int only_friend;

   /* If pilot can't land ignore. */
   if (pilot_isFlag(cur_pilot, PILOT_NOLAND))
      return 0;

   /* Check if we should get only friendlies. */
   only_friend = lua_toboolean(L, 1);

   /* Allocate memory. */
   ind = array_create_size( int, array_size(cur_system->spobs) );

   /* Copy friendly spob.s */
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];

      if (!spob_hasService(pnt, SPOB_SERVICE_LAND))
         continue;
      if (!spob_hasService(pnt, SPOB_SERVICE_INHABITED))
         continue;

      /* Check conditions. */
      if (only_friend && !areAllies( cur_pilot->faction, pnt->presence.faction ))
         continue;
      if (areEnemies( cur_pilot->faction, pnt->presence.faction ))
         continue;

      /* Add it. */
      array_push_back( &ind, i );
   }

   /* no spob to land on found */
   if (array_size(ind)==0) {
      array_free(ind);
      return 0;
   }

   /* we can actually get a random spob now */
   id = RNG(0,array_size(ind)-1);
   p = cur_system->spobs[ ind[ id ] ];
   spob = p->id;
   lua_pushspob( L, spob );
   cur_pilot->nav_spob = ind[ id ];
   array_free(ind);

   return 1;
}

/**
 * @brief Lands on a spob.
 *
 *    @luatparam[opt] Spob pnt spob to land on
 *    @luatreturn boolean Whether landing was successful.
 *    @luafunc land
 */
static int aiL_land( lua_State *L )
{
   const Spob *spob;
   HookParam hparam;

   if (!lua_isnoneornil(L,1)) {
      int i;
      const Spob *pnt = luaL_validspob( L, 1 );

      /* Find the spob. */
      for (i=0; i < array_size(cur_system->spobs); i++) {
         if (cur_system->spobs[i] == pnt) {
            break;
         }
      }
      if (i >= array_size(cur_system->spobs))
         return NLUA_ERROR( L, _("Spob '%s' not found in system '%s'"), pnt->name, cur_system->name );

      cur_pilot->nav_spob = i;
   }

   if (cur_pilot->nav_spob < 0)
      return NLUA_ERROR( L, _("Pilot '%s' (ai '%s') has no land target"), cur_pilot->name, cur_pilot->ai->name );

   /* Get spob. */
   spob = cur_system->spobs[ cur_pilot->nav_spob ];

   /* Check landability. */
   if ((spob->lua_can_land==LUA_NOREF) && !spob_hasService(spob,SPOB_SERVICE_LAND)) { /* Basic services */
      lua_pushboolean(L,0);
      return 1;
   }
   /* TODO can_land is player-specific, we need to implement this on a pilot level...
   if ((!pilot_isFlag(cur_pilot, PILOT_MANUAL_CONTROL) && !spob->can_land)) {
      lua_pushboolean(L,0);
      return 1;
   }
   */

   /* Check landing functionality. */
   if (pilot_isFlag(cur_pilot, PILOT_NOLAND)) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* Check distance. */
   if (vec2_dist2(&cur_pilot->solid.pos,&spob->pos) > pow2(spob->radius)) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* Check velocity. */
   if (vec2_odist2( &cur_pilot->solid.vel ) > pow2(MAX_HYPERSPACE_VEL)) {
      lua_pushboolean(L,0);
      return 1;
   }

   if (spob->lua_land == LUA_NOREF) {
      cur_pilot->landing_delay = PILOT_LANDING_DELAY * cur_pilot->ship->dt_default;
      cur_pilot->ptimer = cur_pilot->landing_delay;
      pilot_setFlag( cur_pilot, PILOT_LANDING );
   }
   else {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_land); /* f */
      lua_pushspob( naevL, spob_index(spob) );
      lua_pushpilot( naevL, cur_pilot->id );
      if (nlua_pcall( spob->lua_env, 2, 0 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "land", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
      }
   }

   hparam.type    = HOOK_PARAM_SPOB;
   hparam.u.la    = spob->id;

   pilot_runHookParam( cur_pilot, PILOT_HOOK_LAND, &hparam, 1 );
   lua_pushboolean(L,1);
   return 1;
}

/**
 * @brief Tries to enter hyperspace.
 *
 *    @luatparam[opt] System sys System to jump to
 *    @luatreturn number|nil Distance if too far away.
 *    @luafunc hyperspace
 */
static int aiL_hyperspace( lua_State *L )
{
   int dist;

   /* Find the target jump. */
   if (!lua_isnoneornil(L,1)) {
      const JumpPoint *jp = luaL_validjump( L, 1 );
      const LuaJump *lj = luaL_checkjump( L, 1 );
      if (lj->srcid != cur_system->id)
         return NLUA_ERROR(L, _("Jump point must be in current system."));
      cur_pilot->nav_hyperspace = jp - cur_system->jumps;
   }

   dist = space_hyperspace(cur_pilot);
   if (dist == 0) {
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
 *    @luafunc sethyptarget
 */
static int aiL_sethyptarget( lua_State *L )
{
   const JumpPoint *jp;
   const LuaJump *lj;
   vec2 vec;
   double a, rad;

   lj = luaL_checkjump( L, 1 );
   jp = luaL_validjump( L, 1 );

   if (lj->srcid != cur_system->id)
      return NLUA_ERROR(L, _("Jump point must be in current system."));

   /* Copy vector. */
   vec = jp->pos;

   /* Introduce some error. */
   a     = RNGF() * M_PI * 2.;
   rad   = RNGF() * 0.5 * jp->radius;
   vec2_cadd( &vec, rad*cos(a), rad*sin(a) );

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
 *    @luafunc nearhyptarget
 */
static int aiL_nearhyptarget( lua_State *L )
{
   const JumpPoint *jp;
   double mindist, dist;
   LuaJump lj;

   /* Find nearest jump .*/
   mindist = INFINITY;
   jp      = NULL;
   for (int i=0; i < array_size(cur_system->jumps); i++) {
      const JumpPoint *jiter = &cur_system->jumps[i];
      int useshidden = faction_usesHiddenJumps( cur_pilot->faction );

      /* Ignore exit only. */
      if (jp_isFlag( jiter, JP_EXITONLY ))
         continue;

      /* We want only standard jump points to be used. */
      if (!useshidden && jp_isFlag(jiter, JP_HIDDEN))
         continue;

      /* Only jump if there is presence there. */
      if (system_getPresence( jiter->target, cur_pilot->faction ) <= 0.)
         continue;

      /* Get nearest distance. */
      dist  = vec2_dist2( &cur_pilot->solid.pos, &jiter->pos );
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
 *    @luafunc rndhyptarget
 */
static int aiL_rndhyptarget( lua_State *L )
{
   JumpPoint **jumps;
   int r, useshidden;
   int *id;
   LuaJump lj;

   /* No jumps in the system. */
   if (array_size(cur_system->jumps) == 0)
      return 0;

   useshidden = faction_usesHiddenJumps( cur_pilot->faction );

   /* Find usable jump points. */
   jumps = array_create_size( JumpPoint*, array_size(cur_system->jumps) );
   id    = array_create_size( int, array_size(cur_system->jumps) );
   for (int i=0; i < array_size(cur_system->jumps); i++) {
      JumpPoint *jiter = &cur_system->jumps[i];

      /* We want only standard jump points to be used. */
      if ((!useshidden && jp_isFlag(jiter, JP_HIDDEN)) || jp_isFlag(jiter, JP_EXITONLY))
         continue;

      /* Only jump if there is presence there. */
      if (system_getPresence( jiter->target, cur_pilot->faction ) <= 0.)
         continue;

      array_push_back( &id, i );
      array_push_back( &jumps, jiter );
   }

   /* Try to be more lax. */
   if (array_size(jumps) <= 0) {
      for (int i=0; i < array_size(cur_system->jumps); i++) {
         JumpPoint *jiter = &cur_system->jumps[i];

         /* We want only standard jump points to be used. */
         if ((!useshidden && jp_isFlag(jiter, JP_HIDDEN)) || jp_isFlag(jiter, JP_EXITONLY))
            continue;

         array_push_back( &id, i );
         array_push_back( &jumps, jiter );
      }
   }

   if (array_size(jumps) <= 0) {
      WARN(_("Pilot '%s' can't find jump to leave system!"), cur_pilot->name);
      return 0;
   }

   /* Choose random jump point. */
   r = RNG( 0, MAX( array_size(jumps)-1, 0) );

   lj.destid = jumps[r]->targetid;
   lj.srcid = cur_system->id;

   /* Clean up. */
   array_free(jumps);
   array_free(id);

   /* Return Jump. */
   lua_pushjump( L, lj );
   return 1;
}

/**
 * @brief Gets whether or not the pilot can hyperspace.
 *
 *    @luatreturn boolean Whether or not the pilot can hyperspace.
 * @luafunc canHyperspace
 */
static int aiL_canHyperspace( lua_State *L )
{
   lua_pushboolean(L, space_canHyperspace(cur_pilot));
   return 1;
}

/**
 * @brief Has the AI abandon hyperspace if applicable.
 *
 * @luafunc hyperspaceAbort
 */
static int aiL_hyperspaceAbort( lua_State *L )
{
   (void) L;
   pilot_hyperspaceAbort( cur_pilot );
   return 0;
}

/**
 * @brief Gets the relative velocity of a pilot.
 *
 *    @luatreturn number Relative velocity.
 * @luafunc relvel
 */
static int aiL_relvel( lua_State *L )
{
   double dot, mod;
   vec2 vv, pv;
   int absolute;
   const Pilot *p = luaL_validpilot(L,1);

   if (lua_gettop(L) > 1)
      absolute = lua_toboolean(L,2);
   else
      absolute = 0;

   /* Get the projection of target on current velocity. */
   if (absolute == 0)
      vec2_cset( &vv, p->solid.vel.x - cur_pilot->solid.vel.x,
            p->solid.vel.y - cur_pilot->solid.vel.y );
   else
      vec2_cset( &vv, p->solid.vel.x, p->solid.vel.y);

   vec2_cset( &pv, p->solid.pos.x - cur_pilot->solid.pos.x,
         p->solid.pos.y - cur_pilot->solid.pos.y );
   dot = vec2_dot( &pv, &vv );
   mod = MAX(VMOD(pv), 1.); /* Avoid /0. */

   lua_pushnumber(L, dot / mod );
   return 1;
}

/**
 * @brief Computes the point to face in order to
 *        follow another pilot using a PD controller.
 *
 *    @luatparam Pilot target The pilot to follow
 *    @luatparam number radius The requested distance between p and target
 *    @luatparam number angle The requested angle between p and target (radians)
 *    @luatparam number Kp The first controller parameter
 *    @luatparam number Kd The second controller parameter
 *    @luatparam[opt] string method Method to compute goal angle
 *    @luareturn The point to go to as a vector2.
 * @luafunc follow_accurate
 */
static int aiL_follow_accurate( lua_State *L )
{
   vec2 point, cons, goal, pv;
   double radius, angle, Kp, Kd, angle2;
   const Pilot *p, *target;
   const char *method;

   p = cur_pilot;
   target = luaL_validpilot(L,1);
   radius = luaL_checknumber(L,2);
   angle = luaL_checknumber(L,3);
   Kp = luaL_checknumber(L,4);
   Kd = luaL_checknumber(L,5);
   method = luaL_optstring(L,6,"velocity");

   if (strcmp( method, "absolute" ) == 0)
      angle2 = angle;
   else if (strcmp( method, "keepangle" ) == 0) {
      vec2_cset( &pv, p->solid.pos.x - target->solid.pos.x,
            p->solid.pos.y - target->solid.pos.y );
      angle2 = VANGLE(pv);
      }
   else /* method == "velocity" */
      angle2 = angle + VANGLE( target->solid.vel );

   vec2_cset( &point, VX(target->solid.pos) + radius * cos(angle2),
         VY(target->solid.pos) + radius * sin(angle2) );

   /*  Compute the direction using a pd controller */
   vec2_cset( &cons, (point.x - p->solid.pos.x) * Kp +
         (target->solid.vel.x - p->solid.vel.x) *Kd,
         (point.y - p->solid.pos.y) * Kp +
         (target->solid.vel.y - p->solid.vel.y) *Kd );

   vec2_cset( &goal, cons.x + p->solid.pos.x, cons.y + p->solid.pos.y);

   /* Push info */
   lua_pushvector( L, goal );

   return 1;

}

/**
 * @brief Computes the point to face in order to follow a moving object.
 *
 *    @luatparam vec2 pos The objective vector
 *    @luatparam vec2 vel The objective velocity
 *    @luatparam number radius The requested distance between p and target
 *    @luatparam number angle The requested angle between p and target (radians)
 *    @luatparam number Kp The first controller parameter
 *    @luatparam number Kd The second controller parameter
 *    @luareturn The point to go to as a vector2.
 * @luafunc face_accurate
 */
static int aiL_face_accurate( lua_State *L )
{
   vec2 point, cons, goal, *pos, *vel;
   double radius, angle, Kp, Kd;
   const Pilot *p = cur_pilot;

   pos = lua_tovector(L,1);
   vel = lua_tovector(L,2);
   radius = luaL_checknumber(L,3);
   angle = luaL_checknumber(L,4);
   Kp = luaL_checknumber(L,5);
   Kd = luaL_checknumber(L,6);

   vec2_cset( &point, pos->x + radius * cos(angle),
         pos->y + radius * sin(angle) );

   /*  Compute the direction using a pd controller */
   vec2_cset( &cons, (point.x - p->solid.pos.x) * Kp +
         (vel->x - p->solid.vel.x) *Kd,
         (point.y - p->solid.pos.y) * Kp +
         (vel->y - p->solid.vel.y) *Kd );

   vec2_cset( &goal, cons.x + p->solid.pos.x, cons.y + p->solid.pos.y);

   /* Push info */
   lua_pushvector( L, goal );

   return 1;

}

/**
 * @brief Completely stops the pilot if it is below minimum vel error (no insta-stops).
 *
 *    @luafunc stop
 */
static int aiL_stop( lua_State *L )
{
   (void) L; /* avoid gcc warning */

   if (VMOD(cur_pilot->solid.vel) < MIN_VEL_ERR)
      vec2_pset( &cur_pilot->solid.vel, 0., 0. );

   return 0;
}

/**
 * @brief Docks the ship.
 *
 *    @luatparam Pilot target Pilot to dock with.
 *    @luafunc dock
 */
static int aiL_dock( lua_State *L )
{
   /* Target is another ship. */
   Pilot *p = luaL_validpilot(L,1);
   pilot_dock(cur_pilot, p);
   return 0;
}

/**
 * @brief Sets the combat flag.
 *
 *    @luatparam[opt=true] boolean val Value to set flag to.
 *    @luafunc combat
 */
static int aiL_combat( lua_State *L )
{
   if (lua_gettop(L) > 0) {
      int i = lua_toboolean(L,1);
      if (i==1)
         pilot_setFlag(cur_pilot, PILOT_COMBAT);
      else if (i==0)
         pilot_rmFlag(cur_pilot, PILOT_COMBAT);
   }
   else
      pilot_setFlag(cur_pilot, PILOT_COMBAT);

   return 0;
}

/**
 * @brief Sets the pilot's target.
 *
 *    @luaparam target Pilot to target.
 *    @luafunc settarget
 */
static int aiL_settarget( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   pilot_setTarget( cur_pilot, p->id );
   return 0;
}

/**
 * @brief Sets the pilot's asteroid target.
 *
 *    @luaparam int field Id of the field to target.
 *    @luaparam int ast Id of the asteroid to target.
 *    @luafunc setasterotarget
 */
static int aiL_setasterotarget( lua_State *L )
{
   const LuaAsteroid_t *la = luaL_checkasteroid(L,1);

   /* Set the target asteroid. */
   cur_pilot->nav_anchor = la->parent;
   cur_pilot->nav_asteroid = la->id;

   /* Untarget pilot. */
   cur_pilot->target = cur_pilot->id;
   cur_pilot->ptarget = NULL;

   return 0;
}

/**
 * @brief Gets the closest gatherable within a radius.
 *
 *    @luaparam float rad Radius to search in.
 *    @luareturn int i Id of the gatherable or nil if none found.
 *    @luafunc getgatherable
 */
static int aiL_getGatherable( lua_State *L )
{
   int i;
   double rad;

   if ((lua_gettop(L) < 1) || lua_isnil(L,1))
      rad = INFINITY;
   else
      rad = lua_tonumber(L,1);

   i = gatherable_getClosest( &cur_pilot->solid.pos, rad );

   if (i != -1)
      lua_pushnumber(L,i);
   else
      lua_pushnil(L);

   return 1;
}

/**
 * @brief Gets the pos and vel of a given gatherable.
 *
 *    @luaparam int id Id of the gatherable.
 *    @luareturn vec2 pos position of the gatherable.
 *    @luareturn vec2 vel velocity of the gatherable.
 *    @luafunc gatherablepos
 */
static int aiL_gatherablePos( lua_State *L )
{
   int i, did;
   vec2 pos, vel;

   i = lua_tointeger(L,1);

   did = gatherable_getPos( &pos, &vel, i );

   if (did == 0) /* No gatherable matching this ID. */
      return 0;

   lua_pushvector(L, pos);
   lua_pushvector(L, vel);

   return 2;
}

/**
 * @brief Sets the active weapon set, fires another weapon set or activate an outfit.
 *
 *    @luatparam number id ID of the weapon set to switch to or fire.
 *    @luatparam[opt=true] boolean type true to activate, false to deactivate.
 * @luafunc weapset
 */
static int aiL_weapSet( lua_State *L )
{
   int id, type;
   id = luaL_checkinteger(L,1);

   if (lua_gettop(L) > 1)
      type = lua_toboolean(L,2);
   else
      type = 1;

   /* weapset type is weapon or change */
   if (type)
      pilot_weapSetPress( cur_pilot, id, +1 );
   else
      pilot_weapSetPress( cur_pilot, id, -1 );
   return 0;
}

/**
 * @brief Does the pilot have cannons?
 *
 *    @luatreturn boolean True if the pilot has cannons.
 * @luafunc hascannons
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
 * @luafunc hasturrets
 */
static int aiL_hasturrets( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->nturrets > 0 );
   return 1;
}

/**
 * @brief Does the pilot have fighter bays?
 *
 *    @luatreturn boolean True if the pilot has fighter bays.
 * @luafunc hasfighterbays
 */
static int aiL_hasfighterbays( lua_State *L )
{
   lua_pushboolean( L, cur_pilot->nfighterbays > 0 );
   return 1;
}

/**
 * @brief Does the pilot have afterburners?
 *
 *    @luatreturn boolean True if the pilot has afterburners.
 * @luafunc hasafterburners
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
 *    @luafunc shoot
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
 *    @luatparam[opt=nil] Vec2 radius Distance to search for an enemy. If not specified tries to find the nearest enemy.
 *    @luatreturn Pilot|nil
 *    @luafunc getenemy
 */
static int aiL_getenemy( lua_State *L )
{
   if (lua_isnoneornil(L,1)) {
      unsigned int id = pilot_getNearestEnemy(cur_pilot);
      if (id==0) /* No enemy found */
         return 0;
      lua_pushpilot(L, id);
      return 1;
   }
   else {
      double range = luaL_checknumber(L,1);
      double r2 = pow2(range);
      unsigned int tp = 0;
      double d = 0.;
      int x, y, r;
      Pilot *const* pilot_stack = pilot_getAll();

      r = ceil(range);
      x = round(cur_pilot->solid.pos.x);
      y = round(cur_pilot->solid.pos.y);
      pilot_collideQueryIL( &ai_qtquery, x-r, y-r, x+r, y+r );
      for (int i=0; i<il_size(&ai_qtquery); i++ ) {
         const Pilot *p = pilot_stack[ il_get( &ai_qtquery, i, 0 ) ];
         double td;

         if (vec2_dist2(&p->solid.pos, &cur_pilot->solid.pos) > r2)
            continue;

         if (!pilot_validEnemy( cur_pilot, p ))
            continue;

         /* Check distance. */
         td = vec2_dist2(&p->solid.pos, &cur_pilot->solid.pos);
         if (!tp || (td < d)) {
            d  = td;
            tp = p->id;
         }
      }
      return tp;
   }
}

/**
 * @brief Sets the enemy hostile (basically notifies of an impending attack).
 *
 *    @luatparam Pilot target Pilot to set hostile.
 *    @luafunc hostile
 */
static int aiL_hostile( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   if (pilot_isWithPlayer(p))
      pilot_setHostile(cur_pilot);
   return 0;
}

/**
 * @brief Gets the range of a weapon.
 *
 *    @luatparam[opt] number id Optional parameter indicating id of weapon set to get range of, defaults to selected one.
 *    @luatparam[opt=-1] number level Level of weapon set to get range of.
 *    @luatreturn number The range of the weapon set.
 * @luafunc getweaprange
 */
static int aiL_getweaprange( lua_State *L )
{
   int id    = luaL_optinteger( L, 1, cur_pilot->active_set );
   int level = luaL_optinteger( L, 2, -1 );
   lua_pushnumber(L, pilot_weapSetRange( cur_pilot, id, level ) );
   return 1;
}

/**
 * @brief Gets the speed of a weapon.
 *
 *    @luatparam[opt] number id Optional parameter indicating id of weapon set to get speed of, defaults to selected one.
 *    @luatparam[opt=-1] number level Level of weapon set to get range of.
 *    @luatreturn number The range of the weapon set.
 * @luafunc getweapspeed
 */
static int aiL_getweapspeed( lua_State *L )
{
   int id    = luaL_optinteger( L, 1, cur_pilot->active_set );
   int level = luaL_optinteger( L, 2, -1 );
   lua_pushnumber(L, pilot_weapSetSpeed( cur_pilot, id, level ) );
   return 1;
}

/**
 * @brief Gets the ammo of a weapon.
 *
 *    @luatparam[opt] number id Optional parameter indicating id of weapon set to get ammo of, defaults to selected one.
 *    @luatparam[opt=-1] number level Level of weapon set to get range of.
 *    @luatreturn number The range of the weapon set.
 * @luafunc getweapammo
 */
static int aiL_getweapammo( lua_State *L )
{
   int id    = luaL_optinteger( L, 1, cur_pilot->active_set );
   int level = luaL_optinteger( L, 2, -1 );
   lua_pushnumber(L, pilot_weapSetAmmo( cur_pilot, id, level ) );
   return 1;
}

/**
 * @brief Checks to see if pilot can board the target.
 *
 *    @luatparam Pilot target Target to see if pilot can board.
 *    @luatreturn boolean true if pilot can board, false if it can't.
 * @luafunc canboard
 */
static int aiL_canboard( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);

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
 * @luafunc relsize
 */
static int aiL_relsize( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, pilot_relsize(cur_pilot, p));
   return 1;
}

/**
 * @brief Gets the relative damage output (total DPS) between the current pilot and the specified target.
 *
 *    @luatparam Pilot target The pilot whose DPS we will compare.
 *    @luatreturn number A number from 0 to 1 mapping the relative DPSes.
 * @luafunc reldps
 */
static int aiL_reldps( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, pilot_reldps(cur_pilot, p));
   return 1;
}

/**
 * @brief Gets the relative health (total shields and armour) between the current pilot and the specified target
 *
 *    @luatparam Pilot target The pilot whose health we will compare.
 *    @luatreturn number A number from 0 to 1 mapping the relative healths.
 *    @luafunc relhp
 */
static int aiL_relhp( lua_State *L )
{
   const Pilot *p = luaL_validpilot(L,1);
   lua_pushnumber(L, pilot_relhp(cur_pilot, p));
   return 1;
}

/**
 * @brief Attempts to board the pilot's target.
 *
 *    @luatreturn boolean true if was able to board the target.
 *    @luafunc board
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
 *    @luafunc refuel
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
 *    @luafunc settimer
 */
static int aiL_settimer( lua_State *L )
{
   int n = luaL_checkint(L,1);
   /* Set timer. */
   cur_pilot->timer[n] = luaL_optnumber(L,2,0.);
   return 0;
}

/**
 * @brief Checks a timer.
 *
 *    @luatparam number timer Timer number.
 *    @luatreturn boolean Whether time is up.
 *    @luafunc timeup
 */

static int aiL_timeup( lua_State *L )
{
   int n = luaL_checkint(L,1);
   lua_pushboolean(L, cur_pilot->timer[n] < 0.);
   return 1;
}

/**
 * @brief Set the seeker shoot indicator.
 *
 *    @luatparam boolean value to set the shoot indicator to.
 *    @luafunc set_shoot_indicator
 */
static int aiL_set_shoot_indicator( lua_State *L )
{
   cur_pilot->shoot_indicator = lua_toboolean(L,1);
   return 0;
}

/**
 * @brief Access the seeker shoot indicator (that is put to true each time a seeker is shot).
 *
 *    @luatreturn boolean true if the shoot_indicator is true.
 *    @luafunc set_shoot_indicator
 */
static int aiL_shoot_indicator( lua_State *L )
{
   lua_pushboolean(L, cur_pilot->shoot_indicator);
   return 1;
}

/**
 * @brief Sends a distress signal.
 *
 *    @luatparam string|nil msg Message to send or nil.
 *    @luafunc distress
 */
static int aiL_distress( lua_State *L )
{
   if (lua_isstring(L,1))
      snprintf( aiL_distressmsg, sizeof(aiL_distressmsg), "%s", lua_tostring(L,1) );
   else if (lua_isnoneornil(L,1))
      aiL_distressmsg[0] = '\0';
   else
      NLUA_INVALID_PARAMETER(L,1);

   /* Set flag because code isn't reentrant. */
   ai_setFlag(AI_DISTRESS);

   return 0;
}

/**
 * @brief Picks a pilot that will command the current pilot.
 *
 *    @luatreturn Pilot|nil
 *    @luafunc getBoss
 */
static int aiL_getBoss( lua_State *L )
{
   unsigned int id = pilot_getBoss( cur_pilot );

   if (id==0) /* No boss found */
      return 0;

   lua_pushpilot(L, id);

   return 1;
}

/**
 * @brief Sets the pilots credits. Only call in create().
 *
 *    @luatparam number num Number of credits.
 *    @luafunc setcredits
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
 *    @luafunc messages
 *    @luatreturn {{},...} Messages.
 */
static int aiL_messages( lua_State *L )
{
   lua_rawgeti(L, LUA_REGISTRYINDEX, cur_pilot->messages);
   lua_newtable(L);
   lua_rawseti(L, LUA_REGISTRYINDEX, cur_pilot->messages);
   return 1;
}

/**
 * @brief Tries to stealth or destealth the pilot.
 *
 *    @luatparam[opt=true] boolean enable Whether or not to try to stealth the pilot.
 *    @luatreturn boolean Whether or not the stealthing or destealthing succeeded.
 * @luafunc stealth
 */
static int aiL_stealth( lua_State *L )
{
   int b = 1;
   if (lua_gettop(L)>0)
      b = lua_toboolean(L,1);

   if (!b) {
      pilot_destealth( cur_pilot );
      lua_pushboolean(L,1); /* always succeeds */
      return 1;
   }

   lua_pushboolean(L, pilot_stealth( cur_pilot ));
   return 1;
}
/**
 * @}
 */
