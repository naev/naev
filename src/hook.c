/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file hook.c
 *
 * @brief Handles hooks.
 *
 * Currently only used in the mission system.
 */


#include "hook.h"

#include "naev.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "nxml.h"
#include "player.h"
#include "event.h"
#include "claim.h"
#include "nlua_pilot.h"
#include "nlua_hook.h"
#include "mission.h"
#include "space.h"


#define HOOK_CHUNK   32 /**< Size to grow by when out of space */


/**
 * @brief Types of hook.
 */
typedef enum HookType_e {
   HOOK_TYPE_NULL, /**< Invalid hook type. */
   HOOK_TYPE_MISN, /**< Mission hook type. */
   HOOK_TYPE_EVENT, /**< Event hook type. */
   HOOK_TYPE_FUNC /**< C function hook type. */
} HookType_t;


/**
 * @struct Hook
 *
 * @brief Internal representation of a hook.
 */
typedef struct Hook_ {
   unsigned int id; /**< unique id */
   char *stack; /**< stack it's a part of */
   int delete; /**< indicates it should be deleted when possible */
   int ran_once; /**< Indicates if the hook already ran, useful when iterating. */

   /* Timer information. */
   int is_timer; /**< Whether or not is actually a timer. */
   double ms; /**< Miliseconds left. */

   /* Date information. */
   int is_date; /**< Whether or not it is a date hook. */
   ntime_t res; /**< Resolution to display. */
   ntime_t acc; /**< Accumulated resolution. */

   HookType_t type; /**< Type of hook. */
   union {
      struct {
         unsigned int parent; /**< mission it's connected to */
         char *func; /**< function it runs */
      } misn; /**< Mission Lua function. */
      struct {
         unsigned int parent; /**< Event it's connected to. */
         char *func; /**< Function it runs. */
      } event; /**< Event Lua function. */
      struct {
         int (*func)( void *data ); /**< C function to run. */
         void *data; /**< Data to pass to C function. */
      } func; /**< Normal C function hook. */
   } u; /**< Type specific data. */
} Hook;


/*
 * the stack
 */
static unsigned int hook_id   = 0; /**< Unique hook id generator. */
static Hook* hook_stack       = NULL; /**< Stack of hooks. */
static int hook_mstack        = 0; /**< Size of hook memory. */
static int hook_nstack        = 0; /**< Number of hooks currently used. */
static int hook_runningstack  = 0; /**< Check if stack is running. */
static int hook_loadingstack  = 0; /**< Check if the hooks are being loaded. */


/*
 * prototypes
 */
/* intern */
static Hook* hook_get( unsigned int id );
static unsigned int hook_genID (void);
static Hook* hook_new( HookType_t type, const char *stack );
static int hook_parseParam( lua_State *L, HookParam *param );
static int hook_runMisn( Hook *hook, HookParam *param, int claims );
static int hook_runEvent( Hook *hook, HookParam *param, int claims );
static int hook_runFunc( Hook *hook );
static int hook_run( Hook *hook, HookParam *param, int claims );
static void hook_free( Hook *h );
static int hook_needSave( Hook *h );
static int hook_parse( xmlNodePtr base );
/* externed */
int hook_save( xmlTextWriterPtr writer );
int hook_load( xmlNodePtr parent );


/**
 * @brief Parses hook parameters.
 *
 *    @param L Lua state to feed parameters into.
 *    @param param Parameters to process.
 *    @return Parameters found.
 */
static int hook_parseParam( lua_State *L, HookParam *param )
{
   int n;

   if (param == NULL)
      return 0;

   n = 0;
   while (param[n].type != HOOK_PARAM_SENTINAL) {
      switch (param[n].type) {
         case HOOK_PARAM_NIL:
            lua_pushnil( L );
            break;
         case HOOK_PARAM_PILOT:
            lua_pushpilot( L, param[n].u.lp );
            break;

         default:
            WARN( "Unknown Lua parameter type." );
            lua_pushnil( L );
            break;
      }
      n++;
   }

   return n;
}


/**
 * @brief Runs a mission hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_runMisn( Hook *hook, HookParam *param, int claims )
{
   int i;
   unsigned int id;
   Mission* misn;
   lua_State *L;
   int n;

   /* Simplicity. */
   id = hook->id;

   /* Make sure it's valid. */
   if (hook->u.misn.parent == 0) {
      WARN("Trying to run hook with inexistant parent: deleting");
      hook->delete = 1; /* so we delete it */
      return -1;
   }

   /* Locate the mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id == hook->u.misn.parent)
         break;
   if (i>=MISSION_MAX) {
      WARN("Trying to run hook with parent not in player mission stack: deleting");
      hook->delete = 1; /* so we delete it */
      return -1;
   }
   misn = &player_missions[i];

   /* Make sure it's claimed. */
   if ((claims > 0) && (claim_testSys(misn->claims, cur_system->id) != claims))
      return 0;

   /* Set up hook parameters. */
   L = misn_runStart( misn, hook->u.misn.func );
   n = hook_parseParam( L, param );

   /* Add hook parameters. */
   hookL_getarg( L, id );
   n++;

   /* Run mission code. */
   hook->ran_once = 1;
   if (misn_runFunc( misn, hook->u.misn.func, n ) < 0) { /* error has occured */
      WARN("Hook [%s] '%d' -> '%s' failed", hook->stack,
            hook->id, hook->u.misn.func);
      return -1;
   }
   return 0;
}


/**
 * @brief Runs a Event function hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_runEvent( Hook *hook, HookParam *param, int claims )
{
   int ret;
   unsigned int id;
   lua_State *L;
   int n;

   /* Must match claims. */
   if ((claims > 0) && (event_testClaims( hook->u.event.parent, cur_system->id ) != claims))
      return 0;

   /* Simplicity. */
   id = hook->id;

   /* Set up hook parameters. */
   L = event_runStart( hook->u.event.parent, hook->u.event.func );
   if (L == NULL) {
      WARN("Hook [%s] '%d' -> '%s' failed, event does not exist. Deleting hook.", hook->stack,
            hook->id, hook->u.event.func);
      hook->delete = 1; /* Set for deletion. */
      return -1;
   }
   n = hook_parseParam( L, param );

   /* Add hook parameters. */
   hookL_getarg( L, id );
   n++;

   /* Run the hook. */
   ret = event_runFunc( hook->u.event.parent, hook->u.event.func, n );
   hook->ran_once = 1;
   if (ret < 0) {
      hook_rm( id );
      WARN("Hook [%s] '%d' -> '%s' failed", hook->stack,
            hook->id, hook->u.event.func);
      return -1;
   }
   return 0;
}


/**
 * @brief Runs a C function hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_runFunc( Hook *hook )
{
   int ret, id;
   id = hook->id;
   hook->ran_once = 1;
   ret = hook->u.func.func( hook->u.func.data );
   if (ret != 0) {
      hook_rm( id );
      return -1;
   }
   return 0;
}


/**
 * @brief Runs a hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_run( Hook *hook, HookParam *param, int claims )
{
   if (hook->delete)
      return 0; /* hook should be deleted not run */

   switch (hook->type) {
      case HOOK_TYPE_MISN:
         return hook_runMisn(hook, param, claims);

      case HOOK_TYPE_EVENT:
         return hook_runEvent(hook, param, claims);

      case HOOK_TYPE_FUNC:
         return hook_runFunc(hook);

      default:
         WARN("Invalid hook type '%d', deleting.", hook->type);
         hook->delete = 1;
         return -1;
   }
}


/**
 * @brief Generates a new hook id.
 *
 *    @return New hook id.
 */
static unsigned int hook_genID (void)
{
   unsigned int id;
   int i;
   id = ++hook_id; /* default id, not safe if loading */

   /* If not loading we can just return. */
   if (!hook_loadingstack)
      return id;

   /* Must check ids for collisions. */
   for (i=0; i<hook_nstack; i++)
      if (id == hook_stack[i].id) /* Check for collision. */
         return hook_genID(); /* recursively try again */

   return id;
}


/**
 * @brief Generates and allocates a new hook.
 *
 *    @param stack Stack to which the new hook belongs.
 *    @return The newly allocated hook.
 */
static Hook* hook_new( HookType_t type, const char *stack )
{
   Hook *new_hook;

   /* if memory must grow */
   if (hook_nstack+1 > hook_mstack) {
      hook_mstack += HOOK_CHUNK;
      hook_stack   = realloc(hook_stack, hook_mstack*sizeof(Hook));
   }

   /* Get and create new hook. */
   new_hook          = &hook_stack[hook_nstack];
   memset( new_hook, 0, sizeof(Hook) );

   /* Fill out generic details. */
   new_hook->type    = type;
   new_hook->id      = hook_genID();
   new_hook->stack   = strdup(stack);

   /* Increment stack size. */
   hook_nstack++;

   return new_hook;
}


/**
 * @brief Adds a new mission type hook.
 *
 *    @param parent Hook mission parent.
 *    @param func Function to run when hook is triggered.
 *    @param stack Stack hook belongs to.
 *    @return The new hook identifier.
 */
unsigned int hook_addMisn( unsigned int parent, const char *func, const char *stack )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_MISN, stack );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup(func);

   return new_hook->id;
}


/**
 * @brief Adds a new event type hook.
 *
 *    @param parent Hook event parent.
 *    @param func Function to run when hook is triggered.
 *    @param stack Stack hook belongs to.
 *    @return The new hook identifier.
 */
unsigned int hook_addEvent( unsigned int parent, const char *func, const char *stack )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_EVENT, stack );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup(func);

   return new_hook->id;
}


/**
 * @brief Adds a new mission type hook timer hook.
 *
 *    @param parent Hook mission parent.
 *    @param func Function to run when hook is triggered.
 *    @param ms Milliseconds to wait
 *    @return The new hook identifier.
 */
unsigned int hook_addTimerMisn( unsigned int parent, const char *func, double ms )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_MISN, "timer" );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup(func);

   /* Timer information. */
   new_hook->is_timer      = 1;
   new_hook->ms            = ms;

   return new_hook->id;
}


/**
 * @brief Adds a new event type hook timer.
 *
 *    @param parent Hook event parent.
 *    @param func Function to run when hook is triggered.
 *    @param ms Miliseconds to wait.
 *    @return The new hook identifier.
 */
unsigned int hook_addTimerEvt( unsigned int parent, const char *func, double ms )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_EVENT, "timer" );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup(func);

   /* Timer information. */
   new_hook->is_timer      = 1;
   new_hook->ms            = ms;

   return new_hook->id;
}



void hooks_updateDate( ntime_t change )
{
   int i, j;
   Hook *h;

   /* Don't update without player. */
   if ((player.p == NULL) || player_isFlag(PLAYER_CREATING))
      return;

   hook_runningstack = 1; /* running hooks */
   for (j=1; j>=0; j--) {
      for (i=0; i<hook_nstack; i++) {
         /* Find valid timer hooks. */
         h = &hook_stack[i];
         if (h->is_date == 0)
            continue;

         /* Decrement timer and check to see if should run. */
         if (j==0)
            h->acc += change;
         if (h->acc < h->res)
            continue;

         /* Run the timer hook. */
         hook_run( h, NULL, j );
         /* Date hooks are not deleted. */

         if (j==1)
            h->acc -= h->res;
      }
   }
   hook_runningstack = 0; /* not running hooks anymore */

   /* Second pass to delete. */
   for (i=0; i<hook_nstack; i++) {
      /* Find valid timer hooks. */
      h = &hook_stack[i];
      if (h->delete)
         if (hook_rm(h->id)==1)
            i--;
   }
}


unsigned int hook_addDateMisn( unsigned int parent, const char *func, ntime_t resolution )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_MISN, "date" );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup(func);

   /* Timer information. */
   new_hook->is_date       = 1;
   new_hook->res           = resolution;
   new_hook->acc           = 0;

   return new_hook->id;
}


unsigned int hook_addDateEvt( unsigned int parent, const char *func, ntime_t resolution )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_EVENT, "date" );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup(func);

   /* Timer information. */
   new_hook->is_date       = 1;
   new_hook->res           = resolution;
   new_hook->acc           = 0;

   return new_hook->id;
}


/**
 * @brief Updates all the hook timer related stuff.
 */
void hooks_update( double dt )
{
   int i, j;
   Hook *h;

   /* Don't update without player. */
   if ((player.p == NULL) || player_isFlag(PLAYER_CREATING))
      return;

   hook_runningstack = 1; /* running hooks */
   for (j=1; j>=0; j--) {
      for (i=0; i<hook_nstack; i++) {
         /* Find valid timer hooks. */
         h = &hook_stack[i];
         if (h->is_timer == 0)
            continue;

         /* Decrement timer and check to see if should run. */
         if (j==0)
            h->ms -= dt;
         if (h->ms > 0.)
            continue;

         /* Run the timer hook. */
         hook_run( h, NULL, j );
         h->delete = 1; /* Mark for deletion. */
      }
   }
   hook_runningstack = 0; /* not running hooks anymore */

   /* Second pass to delete. */
   for (i=0; i<hook_nstack; i++) {
      /* Find valid timer hooks. */
      h = &hook_stack[i];
      if (h->delete)
         if (hook_rm(h->id)==1)
            i--;
   }
}


/**
 * @brief Adds a new C function type hook.
 *
 *    @param func Function to hook.  Parameter is the data passed.  Function
 *           should return 0 if hook should stay or 1 if it should be deleted.
 *    @param data Data to pass to the hooked function.
 *    @param stack Stack to which the hook belongs.
 *    @return The new hook identifier.
 */
unsigned hook_addFunc( int (*func)(void*), void* data, const char *stack )
{
   Hook *new_hook;

   /* Create the new hook. */
   new_hook = hook_new( HOOK_TYPE_FUNC, stack );

   /* Put mission specific details. */
   new_hook->u.func.func = func;
   new_hook->u.func.data = data;

   return new_hook->id;
}


/**
 * @brief Removes a hook.
 *
 *    @param id Identifier of the hook to remove.
 *    @return 1 if hook was removed, 2 if hook was scheduled for removal and
 *            0 if it wasn't removed.
 */
int hook_rm( unsigned int id )
{
   int l,m,h,f;

   /* Remove from all the pilots. */
   pilots_rmHook( id );

   /* Binary search. */
   f = 0;
   l = 0;
   h = hook_nstack-1;
   while (l <= h) {
      m = (l+h)/2;
      if (hook_stack[m].id > id) h = m-1;
      else if (hook_stack[m].id < id) l = m+1;
      else {
         f = 1;
         break;
      }
   }

   /* Check if hook was found. */
   if (f == 0)
      return 0;

   /* Mark to delete, but do not delete yet, hooks are running. */
   if (hook_runningstack) {
      hook_stack[m].delete = 1;
      return 2;
   }

   /* Free the hook. */
   hook_free( &hook_stack[m] );

   /* Last hook, just clip the stack. */
   if (m == (hook_nstack-1)) {
      hook_nstack--;
      return 1;
   }

   /* Move it! */
   memmove( &hook_stack[m], &hook_stack[m+1], sizeof(Hook) * (hook_nstack-m-1) );
   hook_nstack--;
   return 1;
}


/**
 * @brief Removes all hooks belonging to parent mission.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmMisnParent( unsigned int parent )
{
   int i;

   for (i=0; i<hook_nstack; i++)
      if ((hook_stack[i].type==HOOK_TYPE_MISN) &&
            (parent == hook_stack[i].u.misn.parent)) {
         /* Only decrement if hook was actually removed. */
         if (hook_rm( hook_stack[i].id ) == 1)
            i--;
      }
}


/**
 * @brief Removes all hooks belonging to parent event.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmEventParent( unsigned int parent )
{
   int i;

   for (i=0; i<hook_nstack; i++)
      if ((hook_stack[i].type==HOOK_TYPE_EVENT) &&
            (parent == hook_stack[i].u.event.parent)) {
         /* Only decrement if hook was actually removed. */
         if (hook_rm( hook_stack[i].id ) == 1)
            i--;
      }
}


/**
 * @brief Checks to see how many hooks there are with the same mission parent.
 *
 *    @param parent ID of the parent.
 *    @return Number of children hooks the parent has.
 */
int hook_hasMisnParent( unsigned int parent )
{
   int i, num;
   num = 0;
   for (i=0; i<hook_nstack; i++)
      if ((hook_stack[i].type==HOOK_TYPE_MISN) &&
            (parent == hook_stack[i].u.misn.parent))
         num++;
   return num;
}


/**
 * @brief Checks to see how many hooks there are with the same event parent.
 *
 *    @param parent ID of the parent.
 *    @return Number of children hooks the parent has.
 */
int hook_hasEventParent( unsigned int parent )
{
   int i, num;
   num = 0;
   for (i=0; i<hook_nstack; i++)
      if ((hook_stack[i].type==HOOK_TYPE_EVENT) &&
            (parent == hook_stack[i].u.event.parent))
         num++;
   return num;
}


/**
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @return 0 on success.
 */
int hooks_runParam( const char* stack, HookParam *param )
{
   int i, j;
   int run;
   Hook *h;

   /* Don't update if player is dead. */
   if ((player.p == NULL) || player_isFlag(PLAYER_DESTROYED))
      return 0;

   /* Mark hooks as unrun. */
   for (i=0; i<hook_nstack; i++)
      hook_stack[i].ran_once = 0;

   run = 0;
   hook_runningstack = 1; /* running hooks */
   for (j=1; j>=0; j--) {
      for (i=0; i<hook_nstack; i++) {
         h = &hook_stack[i];
         /* Should be deleted. */
         if (h->delete)
            continue;
         /* Doesn't match stack. */
         if (strcmp(stack, h->stack) != 0)
            continue;
         if (h->ran_once)
            continue;
         hook_run( h, param, j );
         run++;
      }
   }
   hook_runningstack = 0; /* not running hooks anymore */

   for (i=0; i<hook_nstack; i++) {
      if (hook_stack[i].delete) { /* Delete any that need deleting */
         hook_rm( hook_stack[i].id );
         i--;
      }
   }

   /* Check claims. */
   if (run)
      claim_activateAll();

   return run;
}


/**
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @return 0 on success.
 */
int hooks_run( const char* stack )
{
   return hooks_runParam( stack, NULL );
}


/**
 * @brief Gets a hook by ID.
 */
static Hook* hook_get( unsigned int id )
{
   int i;
   for (i=0; i<hook_nstack; i++)
      if (hook_stack[i].id == id)
         return &hook_stack[i];
   return NULL;
}


/**
 * @brief Runs a single hook by id.
 *
 *    @param id Identifier of the hook to run.
 *    @return The ID of the hook or 0 if it got deleted.
 */
int hook_runIDparam( unsigned int id, HookParam *param )
{
   Hook *h;

   /* Don't update if player is dead. */
   if ((player.p == NULL) || player_isFlag(PLAYER_DESTROYED))
      return 0;

   /* Try to find the hook and run it. */
   h = hook_get( id );
   if (h == NULL) {
      WARN("Attempting to run hook of id '%d' which is not in the stack", id);
      return -1;
   }
   hook_run( h, param, -1 );

   return 0;
}


/**
 * @brief Runs a single hook by id.
 *
 *    @param id Identifier of the hook to run.
 *    @return The ID of the hook or 0 if it got deleted.
 */
int hook_runID( unsigned int id )
{
   return hook_runIDparam( id, 0 );
}


/**
 * @brief Frees a hook.
 *
 *    @param h Hook to free.
 */
static void hook_free( Hook *h )
{
   /* Generic freeing. */
   if (h->stack != NULL)
      free(h->stack);

   switch (h->type) {
      case HOOK_TYPE_MISN:
         if (h->u.misn.func != NULL)
            free(h->u.misn.func);
         break;

      case HOOK_TYPE_EVENT:
         if (h->u.event.func != NULL)
            free(h->u.event.func);
         break;

      default:
         break;
   }
}


/**
 * @brief Gets rid of all current hooks.
 */
void hook_cleanup (void)
{
   int i;

   for (i=0; i<hook_nstack; i++)
      hook_free( &hook_stack[i] );
   free( hook_stack );
   /* sane defaults just in case */
   hook_stack  = NULL;
   hook_nstack = 0;
   hook_mstack = 0;
}


/**
 * @brief Checks if a hook needs to be saved.
 *
 *    @param h Hook to check if it should be saved.
 *    @return 1 if hook should be saved.
 */
static int hook_needSave( Hook *h )
{
   int i;
   char *nosave[] = {
         "p_death", "p_board", "p_disable", "p_jump", "p_attacked", "p_idle", /* pilot hooks */
         "timer", /* timers */
         "end" };

   /* Impossible to save functions. */
   if (h->type == HOOK_TYPE_FUNC)
      return 0;

   /* Events must need saving. */
   if ((h->type == HOOK_TYPE_EVENT) && !event_save(h->u.event.parent))
      return 0;

   /* Make sure it's in the proper stack. */
   for (i=0; strcmp(nosave[i],"end") != 0; i++)
      if (strcmp(nosave[i],h->stack)==0) return 0;

   return 1;
}


/**
 * @brief Saves all the hooks.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int hook_save( xmlTextWriterPtr writer )
{
   int i;
   Hook *h;

   xmlw_startElem(writer,"hooks");
   for (i=0; i<hook_nstack; i++) {
      h = &hook_stack[i];

      if (!hook_needSave(h))
         continue; /* no need to save it */

      xmlw_startElem(writer,"hook");

      switch (h->type) {
         case HOOK_TYPE_MISN:
            xmlw_attr(writer,"type","misn"); /* Save attribute. */
            xmlw_elem(writer,"parent","%u",h->u.misn.parent);
            xmlw_elem(writer,"func","%s",h->u.misn.func);
            break;

         case HOOK_TYPE_EVENT:
            xmlw_attr(writer,"type","event"); /* Save attribute. */
            xmlw_elem(writer,"parent","%u",h->u.event.parent);
            xmlw_elem(writer,"func","%s",h->u.event.func);
            break;

         default:
            WARN("Something has gone screwy here...");
            break;
      }

      /* Generic information. */
      xmlw_elem(writer,"id","%u",h->id);
      xmlw_elem(writer,"stack","%s",h->stack);

      /* Store additional date information. */
      if (h->is_date)
         xmlw_elem(writer,"resolution","%"PRId64,h->res);

      xmlw_endElem(writer); /* "hook" */
   }
   xmlw_endElem(writer); /* "hooks" */

   return 0;
}


/**
 * @brief Loads hooks for a player.
 *
 *    @param parent Parent xml node containing the hooks.
 *    @return 0 on success.
 */
int hook_load( xmlNodePtr parent )
{
   xmlNodePtr node;
   int i;

   hook_cleanup();

   /* We're loading. */
   hook_loadingstack = 1;

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"hooks"))
         hook_parse(node);
   } while (xml_nextNode(node));

   /* Done loading. */
   hook_loadingstack = 0;

   /* Set ID gen to highest hook. */
   for (i=0; i<hook_nstack; i++)
      hook_id = MAX( hook_stack[i].id, hook_id );

   return 0;
}


/**
 * @brief Parses an individual hook.
 *
 *    @param base Parent xml node of the hook.
 *    @return 0 on success.
 */
static int hook_parse( xmlNodePtr base )
{
   xmlNodePtr node, cur;
   char *func, *stack, *stype;
   unsigned int parent, id, new_id;
   HookType_t type;
   Hook *h;
   int is_date;
   ntime_t res;

   /* Defaults. */
   is_date = 0;

   node = base->xmlChildrenNode;
   do {
      if (xml_isNode(node,"hook")) {
         id       = 0;
         parent   = 0;
         func     = NULL;
         stack    = NULL;

         /* Handle the type. */
         xmlr_attr(node,"type",stype);
         /* Default to mission for old saves. */
         if (stype == NULL)
            type = HOOK_TYPE_MISN;
         /* Mission type. */
         else if (strcmp(stype,"misn")==0) {
            type = HOOK_TYPE_MISN;
            free(stype);
         }
         /* Event type. */
         else if (strcmp(stype,"event")==0) {
            type = HOOK_TYPE_EVENT;
            free(stype);
         }
         /* Unknown. */
         else {
            WARN("Hook of unknown type '%s' found, skipping.", stype);
            type = HOOK_TYPE_NULL;
            free(stype);
            continue;
         }

         /* Handle the data. */
         cur = node->xmlChildrenNode;
         do {
            xml_onlyNodes(cur);

            /* ID. */
            xmlr_long(cur,"id",id);

            /* Generic. */
            xmlr_str(cur,"stack",stack);

            /* Type specific. */
            if ((type == HOOK_TYPE_MISN) || (type == HOOK_TYPE_EVENT)) {
               xmlr_uint(cur,"parent",parent);
               xmlr_str(cur,"func",func);
            }

            /* Check for date hook. */
            if (xml_isNode( cur, "resolution" )) {
               is_date = 1;
               res = xml_getLong( cur );
               continue;
            }

            WARN("Save has unknown hook node '%s'.", cur->name);
         } while (xml_nextNode(cur));

         /* Check for validity. */
         if ((parent == 0) || (func == NULL) || (stack == NULL)) {
            WARN("Invalid hook.");
            continue;
         }

         /* Create the hook. */
         if (type == HOOK_TYPE_MISN) {
            new_id = hook_addMisn( parent, func, stack );
         }
         if (type == HOOK_TYPE_EVENT) {
            new_id = hook_addEvent( parent, func, stack );
         }

         /* Set the id. */
         if (id != 0) {
            h = hook_get( new_id );
            h->id = id;
         }

         /* Additional info. */
         if (is_date) {
            h->is_date = 1;
            h->res = res;
         }
      }
   } while (xml_nextNode(node));

   return 0;
}


