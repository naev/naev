/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file hook.c
 *
 * @brief Handles hooks.
 *
 * Hooks have a major issue, they are sort of like a poor man's threading. This
 * means get all the issues related to threading. The main issues here are the
 * fact that the hooks can mess with the game state during the update and break
 * everything. The solution is to handle hooks either before the update stage
 * (input stage) or after update stage (render stage). This leaves the update
 * stage as sort of an atomic block that doesn't have to worry  about state
 * corruption.
 *
 * The flaw in this design is that it's still possible for hooks to bash other
 * hooks. Notably the player.teleport() is a very dangerous function as it'll
 * destroy the entire current Naev state which will most likely cause all the
 * other hooks to fail.
 *
 * Therefore we must tread carefully. Hooks are serious business.
 */
/** @cond */
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "hook.h"

#include "array.h"
#include "claim.h"
#include "event.h"
#include "log.h"
#include "menu.h"
#include "mission.h"
#include "nlua_commodity.h"
#include "nlua_evt.h"
#include "nlua_hook.h"
#include "nlua_outfit.h"
#include "nlua_pilot.h"
#include "nlua_ship.h"
#include "player.h"
#include "space.h"

/**
 * @brief Hook queue to delay execution.
 */
typedef struct HookQueue_s {
   struct HookQueue_s *next;                   /**< Next in linked list. */
   char               *stack;                  /**< Stack to run. */
   unsigned int        id;                     /**< Run specific hook. */
   HookParam           hparam[HOOK_MAX_PARAM]; /**< Parameters. */
} HookQueue_t;
static HookQueue_t *hook_queue = NULL; /**< The hook queue. */
static int     hook_atomic = 0; /**< Whether or not hooks should be queued. */
static ntime_t hook_time_accum = 0; /**< Time accumulator. */

/**
 * @brief Types of hook.
 */
typedef enum HookType_e {
   HOOK_TYPE_NULL,  /**< Invalid hook type. */
   HOOK_TYPE_MISN,  /**< Mission hook type. */
   HOOK_TYPE_EVENT, /**< Event hook type. */
   HOOK_TYPE_FUNC   /**< C function hook type. */
} HookType_t;

/**
 * @struct Hook
 *
 * @brief Internal representation of a hook.
 */
typedef struct Hook_ {
   struct Hook_ *next; /**< Linked list. */

   unsigned int id;      /**< unique id */
   char        *stack;   /**< stack it's a part of */
   int          created; /**< Hook has just been created. */
   int delete;           /**< indicates it should be deleted when possible */
   int ran_once; /**< Indicates if the hook already ran, useful when iterating.
                  */
   int once;     /**< Only run the hook once. */

   /* Timer information. */
   int    is_timer; /**< Whether or not is actually a timer. */
   double ms;       /**< Milliseconds left. */

   /* Date information. */
   int     is_date; /**< Whether or not it is a date hook. */
   ntime_t res;     /**< Resolution to display. */
   ntime_t acc;     /**< Accumulated resolution. */

   HookType_t type; /**< Type of hook. */
   union {
      struct {
         unsigned int parent; /**< mission it's connected to */
         char        *func;   /**< function it runs */
      } misn;                 /**< Mission Lua function. */
      struct {
         unsigned int parent; /**< Event it's connected to. */
         char        *func;   /**< Function it runs. */
      } event;                /**< Event Lua function. */
      struct {
         int ( *func )( void * );
         void *data;
      } func;
   } u; /**< Type specific data. */
} Hook;

/*
 * the stack
 */
static unsigned int hook_id           = 0;    /**< Unique hook id generator. */
static Hook        *hook_list         = NULL; /**< Stack of hooks. */
static int          hook_runningstack = 0;    /**< Check if stack is running. */
static int hook_loadingstack = 0; /**< Check if the hooks are being loaded. */

/*
 * prototypes
 */
/* Execution. */
static int  hooks_executeParam( const char *stack, const HookParam *param );
static void hooks_updateDateExecute( ntime_t change );
/* intern */
static void         hook_rmRaw( Hook *h );
static void         hooks_purgeList( void );
static Hook        *hook_get( unsigned int id );
static unsigned int hook_genID( void );
static Hook        *hook_new( HookType_t type, const char *stack );
static int          hook_parseParam( const HookParam *param );
static int  hook_runMisn( Hook *hook, const HookParam *param, int claims );
static int  hook_runEvent( Hook *hook, const HookParam *param, int claims );
static int  hook_run( Hook *hook, const HookParam *param, int claims );
static void hook_free( Hook *h );
static int  hook_needSave( Hook *h );
static int  hook_parse( xmlNodePtr base );
/* externed */
int hook_save( xmlTextWriterPtr writer );
int hook_load( xmlNodePtr parent );
/* Misc. */
static Mission *hook_getMission( Hook *hook );

/**
 * Adds a hook to the queue.
 */
static int hq_add( HookQueue_t *hq )
{
   HookQueue_t *c;

   /* Set as head. */
   if ( hook_queue == NULL ) {
      hook_queue = hq;
      return 0;
   }

   /* Find tail. */
   for ( c = hook_queue; c->next != NULL; c = c->next )
      ;
   c->next = hq;
   return 0;
}

/**
 * @brief Frees a queued hook.
 */
static void hq_free( HookQueue_t *hq )
{
   free( hq->stack );
   free( hq );
}

/**
 * @brief Clears the queued hooks.
 */
static void hq_clear( void )
{
   HookQueue_t *hq;
   while ( hook_queue != NULL ) {
      hq         = hook_queue;
      hook_queue = hq->next;
      hq_free( hq );
   }
}

/**
 * @brief Starts the hook exclusion zone, this makes hooks queue until exclusion
 * is done.
 */
void hook_exclusionStart( void )
{
   hook_atomic = 1;
}

/**
 * @brief Ends exclusion zone and runs all the queued hooks.
 */
void hook_exclusionEnd( double dt )
{
   HookQueue_t *hq;
   ntime_t      temp;
   hook_atomic = 0;

   /* Handle hook queue. */
   while ( hook_queue != NULL ) {
      /* Move hook down. */
      hq         = hook_queue;
      hook_queue = hq->next;

      /* Execute. */
      hooks_executeParam( hq->stack, hq->hparam );

      /* Clean up. */
      hq_free( hq );
   }

   /* Update timer hooks. */
   hooks_update( dt );

   /* Run assorted hooks. */
   if ( player.p !=
        NULL ) /* All these hooks rely on player actually existing. */
      player_runHooks();

   /* Time hooks. */
   temp            = hook_time_accum;
   hook_time_accum = 0;
   hooks_updateDateExecute( temp );

   /* Purge the dead. */
   hooks_purgeList();
}

/**
 * @brief Parses hook parameters.
 *
 *    @param param Parameters to process.
 *    @return Parameters found.
 */
static int hook_parseParam( const HookParam *param )
{
   int n;

   if ( param == NULL )
      return 0;

   n = 0;
   while ( param[n].type != HOOK_PARAM_SENTINEL ) {
      switch ( param[n].type ) {
      case HOOK_PARAM_NIL:
         lua_pushnil( naevL );
         break;
      case HOOK_PARAM_NUMBER:
         lua_pushnumber( naevL, param[n].u.num );
         break;
      case HOOK_PARAM_STRING:
         lua_pushstring( naevL, param[n].u.str );
         break;
      case HOOK_PARAM_BOOL:
         lua_pushboolean( naevL, param[n].u.b );
         break;
      case HOOK_PARAM_PILOT:
         lua_pushpilot( naevL, param[n].u.lp );
         break;
      case HOOK_PARAM_SHIP:
         lua_pushship( naevL, param[n].u.ship );
         break;
      case HOOK_PARAM_OUTFIT:
         lua_pushoutfit( naevL, param[n].u.outfit );
         break;
      case HOOK_PARAM_COMMODITY:
         lua_pushcommodity( naevL, param[n].u.commodity );
         break;
      case HOOK_PARAM_FACTION:
         lua_pushfaction( naevL, param[n].u.lf );
         break;
      case HOOK_PARAM_SSYS:
         lua_pushsystem( naevL, param[n].u.ls );
         break;
      case HOOK_PARAM_SPOB:
         lua_pushspob( naevL, param[n].u.la );
         break;
      case HOOK_PARAM_JUMP:
         lua_pushjump( naevL, param[n].u.lj );
         break;
      case HOOK_PARAM_REF:
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, param[n].u.ref );
         break;

      default:
         WARN( _( "Unknown Lua parameter type." ) );
         lua_pushnil( naevL );
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
 *    @param param Parameters to pass.
 *    @param claims Whether the hook is contingent on the mission/event claiming
 * the current system.
 *    @return 0 on success.
 */
static int hook_runMisn( Hook *hook, const HookParam *param, int claims )
{
   unsigned int id;
   Mission     *misn;
   int          n;

   /* Simplicity. */
   id = hook->id;

   /* Make sure it's valid. */
   if ( hook->u.misn.parent == 0 ) {
      WARN( _( "Trying to run hook with nonexistent parent: deleting" ) );
      hook->delete = 1; /* so we delete it */
      return -1;
   }

   /* Locate the mission */
   misn = hook_getMission( hook );
   if ( misn == NULL ) {
      WARN( _( "Trying to run hook with parent not in player mission stack: "
               "deleting" ) );
      hook->delete = 1; /* so we delete it. */
      return -1;
   }

   /* Make sure it's claimed. */
   if ( ( claims > 0 ) &&
        ( claim_testSys( misn->claims, cur_system->id ) != claims ) )
      return 1;

   /* Delete if only supposed to run once. */
   if ( hook->once )
      hook_rmRaw( hook );

   /* Set up hook parameters. */
   misn_runStart( misn, hook->u.misn.func );
   n = hook_parseParam( param );

   /* Add hook parameters. */
   n += hookL_getarg( id );

   /* Run mission code. */
   hook->ran_once = 1;
   if ( misn_runFunc( misn, hook->u.misn.func, n ) <
        0 ) { /* error has occurred */
      WARN( _( "Hook [%s] '%d' -> '%s' failed" ), hook->stack, hook->id,
            hook->u.misn.func );
      /* Don't remove hooks on failure, or it can lead to stuck missions (like
       * rehab). */
      // hook_rmRaw( hook );
      return -1;
   }
   return 0;
}

/**
 * @brief Runs a Event function hook.
 *
 *    @param hook Hook to run.
 *    @param param Parameters to pass.
 *    @param claims Whether the hook is contingent on the mission/event claiming
 * the current system.
 *    @return 0 on success.
 */
static int hook_runEvent( Hook *hook, const HookParam *param, int claims )
{
   int ret, n, id;

   /* Simplicity. */
   id = hook->id;

   /* Must match claims. */
   if ( ( claims > 0 ) &&
        ( event_testClaims( hook->u.event.parent, cur_system->id ) != claims ) )
      return 1;

   /* Delete if only supposed to run once. */
   if ( hook->once )
      hook_rmRaw( hook );

   /* Set up hook parameters. */
   if ( !event_exists( hook->u.event.parent ) ) {
      WARN( _( "Hook [%s] '%d' -> '%s' failed, event does not exist. Deleting "
               "hook." ),
            hook->stack, id, hook->u.event.func );
      hook->delete = 1; /* Set for deletion. */
      return -1;
   }

   event_runStart( hook->u.event.parent, hook->u.event.func );
   n = hook_parseParam( param );

   /* Add hook parameters. */
   n += hookL_getarg( id );

   /* Run the hook. */
   ret = event_runFunc( hook->u.event.parent, hook->u.event.func, n );
   hook->ran_once = 1;
   if ( ret < 0 ) {
      WARN( _( "Hook [%s] '%d' -> '%s' failed" ), hook->stack, hook->id,
            hook->u.event.func );
      /* Don't remove hooks on failure, or it can lead to stuck missions (like
       * rehab). */
      // hook_rmRaw( hook );
      return -1;
   }
   return 0;
}

/**
 * @brief Runs a hook.
 *
 *    @param hook Hook to run.
 *    @param param Parameters to pass.
 *    @param claims Whether the hook is contingent on the mission/event claiming
 * the current system.
 *    @return 0 on success.
 */
static int hook_run( Hook *hook, const HookParam *param, int claims )
{
   int ret;

   /* Do not run if pending deletion. */
   if ( hook->delete )
      return 0;

   /* Don't run anything when main menu is open. */
   if ( menu_isOpen( MENU_MAIN ) )
      return 0;

   switch ( hook->type ) {
   case HOOK_TYPE_MISN:
      ret = hook_runMisn( hook, param, claims );
      break;

   case HOOK_TYPE_EVENT:
      ret = hook_runEvent( hook, param, claims );
      break;

   case HOOK_TYPE_FUNC:
      /* We have to remove the hook first, so it doesn't get run again.
       * Note that the function will not do any checks nor has arguments, since
       * it is C-side. */
      if ( hook->once )
         hook->delete = 1;
      ret = hook->u.func.func( hook->u.func.data );
      break;

   default:
      WARN( _( "Invalid hook type '%d', deleting." ), hook->type );
      hook->delete = 1;
      return -1;
   }

   return ret;
}

/**
 * @brief Generates a new hook id.
 *
 *    @return New hook id.
 */
static unsigned int hook_genID( void )
{
   unsigned int id = ++hook_id; /* default id, not safe if loading */

   /* If not loading we can just return. */
   if ( !hook_loadingstack )
      return id;

   /* Must check ids for collisions. */
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( id == h->id )      /* Check for collision. */
         return hook_genID(); /* recursively try again */

   return id;
}

/**
 * @brief Generates and allocates a new hook.
 *
 *    @param type Type of hook to create.
 *    @param stack Stack to which the new hook belongs.
 *    @return The newly allocated hook.
 */
static Hook *hook_new( HookType_t type, const char *stack )
{
   /* Get and create new hook. */
   Hook *new_hook = calloc( 1, sizeof( Hook ) );
   if ( hook_list == NULL )
      hook_list = new_hook;
   else {
      /* Put at front, O(1). */
      new_hook->next = hook_list;
      hook_list      = new_hook;
   }

   /* Fill out generic details. */
   new_hook->type    = type;
   new_hook->id      = hook_genID();
   new_hook->stack   = strdup( stack );
   new_hook->created = 1;

   /** @TODO fix this hack. */
   if ( strcmp( stack, "safe" ) == 0 )
      new_hook->once = 1;

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
unsigned int hook_addMisn( unsigned int parent, const char *func,
                           const char *stack )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_MISN, stack );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup( func );

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
unsigned int hook_addEvent( unsigned int parent, const char *func,
                            const char *stack )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_EVENT, stack );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup( func );

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
unsigned int hook_addTimerMisn( unsigned int parent, const char *func,
                                double ms )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_MISN, "timer" );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup( func );

   /* Timer information. */
   new_hook->is_timer = 1;
   new_hook->ms       = ms;

   return new_hook->id;
}

/**
 * @brief Adds a new event type hook timer.
 *
 *    @param parent Hook event parent.
 *    @param func Function to run when hook is triggered.
 *    @param ms Milliseconds to wait.
 *    @return The new hook identifier.
 */
unsigned int hook_addTimerEvt( unsigned int parent, const char *func,
                               double ms )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_EVENT, "timer" );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup( func );

   /* Timer information. */
   new_hook->is_timer = 1;
   new_hook->ms       = ms;

   return new_hook->id;
}

/**
 * @brief Adds a function hook to be run.
 */
unsigned int hook_addTimerFunc( int ( *func )( void * ), void *data, double ms )
{
   Hook *new_hook = hook_new( HOOK_TYPE_FUNC, "timer" );

   /* Function special stuff. */
   new_hook->u.func.func = func;
   new_hook->u.func.data = data;

   /* Timer information. */
   new_hook->is_timer = 1;
   new_hook->ms       = ms;

   return new_hook->id;
}

/**
 * @brief Adds a function hook to be run.
 */
unsigned int hook_addFunc( int ( *func )( void * ), void *data,
                           const char *stack )
{
   Hook *new_hook = hook_new( HOOK_TYPE_FUNC, stack );

   /* Function special stuff. */
   new_hook->u.func.func = func;
   new_hook->u.func.data = data;

   return new_hook->id;
}

/**
 * @brief Purges the list of deletable hooks.
 */
static void hooks_purgeList( void )
{
   Hook *h, *hl;

   /* Do not run while stack is being run. */
   if ( hook_runningstack )
      return;

   /* Second pass to delete. */
   hl = NULL;
   h  = hook_list;
   while ( h != NULL ) {
      /* Find valid timer hooks. */
      if ( h->delete ) {

         if ( hl == NULL )
            hook_list = h->next;
         else
            hl->next = h->next;

         /* Free. */
         h->next = NULL;
         hook_free( h );

         /* Last. */
         h = hl;
      }

      hl = h;
      if ( h == NULL )
         h = hook_list;
      else
         h = h->next;
   }
}

/**
 * @brief Updates the time to see if it should be updated.
 */
void hooks_updateDate( ntime_t change )
{
   if ( change > 0 )
      hook_time_accum += change;
}

/**
 * @brief Updates date hooks and runs them if necessary.
 */
static void hooks_updateDateExecute( ntime_t change )
{
   /* Don't update without player. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_CREATING ) )
      return;

   /* Clear creation flags. */
   for ( Hook *h = hook_list; h != NULL; h = h->next ) {
      h->created = 0;
      if ( h->is_date )
         h->ran_once = 0;
   }

   hook_runningstack++; /* running hooks */
   for ( int j = 1; j >= 0; j-- ) {
      for ( Hook *h = hook_list; h != NULL; h = h->next ) {
         /* Not be deleting. */
         if ( h->delete )
            continue;
         /* Find valid date hooks. */
         if ( h->is_date == 0 )
            continue;
         /* Don't update newly created hooks. */
         if ( h->created != 0 )
            continue;

         /* Decrement timer and check to see if should run. */
         if ( j == 1 )
            h->acc += change;
         if ( h->acc < h->res )
            continue;

         /* Time is modified at the end. */
         if ( j == 0 )
            h->acc %= h->res; /* We'll skip all buggers. */
         if ( h->ran_once )
            continue;

         /* Run the date hook. */
         hook_run( h, NULL, j );
         /* Date hooks are not deleted. */
      }
   }
   hook_runningstack--; /* not running hooks anymore */

   /* Second pass to delete. */
   hooks_purgeList();
}

unsigned int hook_addDateMisn( unsigned int parent, const char *func,
                               ntime_t resolution )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_MISN, "date" );

   /* Put mission specific details. */
   new_hook->u.misn.parent = parent;
   new_hook->u.misn.func   = strdup( func );

   /* Timer information. */
   new_hook->is_date = 1;
   new_hook->res     = resolution;
   new_hook->acc     = 0;

   return new_hook->id;
}

unsigned int hook_addDateEvt( unsigned int parent, const char *func,
                              ntime_t resolution )
{
   /* Create the new hook. */
   Hook *new_hook = hook_new( HOOK_TYPE_EVENT, "date" );

   /* Put event specific details. */
   new_hook->u.event.parent = parent;
   new_hook->u.event.func   = strdup( func );

   /* Timer information. */
   new_hook->is_date = 1;
   new_hook->res     = resolution;
   new_hook->acc     = 0;

   return new_hook->id;
}

/**
 * @brief Updates all the hook timer related stuff.
 */
void hooks_update( double dt )
{
   Hook *h;

   /* Don't update without player. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_CREATING ) ||
        player_isFlag( PLAYER_DESTROYED ) )
      return;

   /* Clear creation flags. */
   for ( h = hook_list; h != NULL; h = h->next )
      h->created = 0;

   hook_runningstack++; /* running hooks */
   for ( int j = 1; j >= 0; j-- ) {
      for ( h = hook_list; h != NULL; h = h->next ) {
         /* Not be deleting. */
         if ( h->delete )
            continue;
         /* Find valid timer hooks. */
         if ( h->is_timer == 0 )
            continue;
         /* Don't update newly created hooks. */
         if ( h->created != 0 )
            continue;

         /* Decrement timer and check to see if should run. */
         if ( j == 1 )
            h->ms -= dt;
         if ( h->ms > 0. )
            continue;

         /* Run the timer hook. */
         hook_run( h, NULL, j );
         if ( h->ran_once ) /* Remove when run. */
            hook_rmRaw( h );
      }
   }
   hook_runningstack--; /* not running hooks anymore */

   /* Second pass to delete. */
   hooks_purgeList();
}

/**
 * @brief Gets the mission of a hook.
 */
static Mission *hook_getMission( Hook *hook )
{
   for ( int i = 0; i < array_size( player_missions ); i++ )
      if ( player_missions[i]->id == hook->u.misn.parent )
         return player_missions[i];

   return NULL;
}

/**
 * @brief Removes a hook.
 *
 *    @param id Identifier of the hook to remove.
 */
void hook_rm( unsigned int id )
{
   Hook *h = hook_get( id );
   if ( h == NULL )
      return;

   hook_rmRaw( h );
}

/**
 * @brief Removes a hook.
 */
static void hook_rmRaw( Hook *h )
{
   h->delete = 1;
   hookL_unsetarg( h->id );
}

/**
 * @brief Removes all hooks belonging to parent mission.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmMisnParent( unsigned int parent )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( ( h->type == HOOK_TYPE_MISN ) && ( parent == h->u.misn.parent ) )
         h->delete = 1;
}

/**
 * @brief Removes all hooks belonging to parent event.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmEventParent( unsigned int parent )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( ( h->type == HOOK_TYPE_EVENT ) && ( parent == h->u.event.parent ) )
         h->delete = 1;
}

/**
 * @brief Checks to see how many hooks there are with the same mission parent.
 *
 *    @param parent ID of the parent.
 *    @return Number of children hooks the parent has.
 */
int hook_hasMisnParent( unsigned int parent )
{
   int num = 0;
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( ( h->type == HOOK_TYPE_MISN ) && ( parent == h->u.misn.parent ) )
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
   int num = 0;
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( ( h->type == HOOK_TYPE_EVENT ) && ( parent == h->u.event.parent ) )
         num++;

   return num;
}

static int hooks_executeParam( const char *stack, const HookParam *param )
{
   int run;

   /* Don't update if player is dead. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_DESTROYED ) )
      return 0;

   /* Reset the current stack's ran and creation flags. */
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( strcmp( stack, h->stack ) == 0 ) {
         h->ran_once = 0;
         h->created  = 0;
      }

   run = 0;
   hook_runningstack++; /* running hooks */
   for ( int j = 1; j >= 0; j-- ) {
      for ( Hook *h = hook_list; h != NULL; h = h->next ) {
         /* Should be deleted. */
         if ( h->delete )
            continue;
         /* Don't run again. */
         if ( h->ran_once )
            continue;
         /* Don't update newly created hooks. */
         if ( h->created != 0 )
            continue;
         /* Doesn't match stack. */
         if ( strcmp( stack, h->stack ) != 0 )
            continue;

         /* Run hook. */
         hook_run( h, param, j );
         run++;

         /* If hook_cleanup was run, hook_list will be NULL */
         if ( hook_list == NULL )
            break;
      }
      if ( hook_list == NULL )
         break;
   }
   hook_runningstack--; /* not running hooks anymore */

   /* Free reference parameters. */
   if ( param != NULL ) {
      int n = 0;
      while ( param[n].type != HOOK_PARAM_SENTINEL ) {
         switch ( param[n].type ) {
         case HOOK_PARAM_REF:
            luaL_unref( naevL, LUA_REGISTRYINDEX, param[n].u.ref );
            break;

         default:
            break;
         }
         n++;
      }
   }

   /* Check claims. */
   if ( run )
      claim_activateAll();

   return run;
}

/**
 * @brief Runs all the hooks of stack in the next frame. Does not trigger right
 * away.
 *
 *    @param stack Stack to run.
 *    @param param Parameters to pass.
 *    @return 0 on success.
 */
int hooks_runParamDeferred( const char *stack, const HookParam *param )
{
   int          i;
   HookQueue_t *hq;

   /* Don't update if player is dead. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_DESTROYED ) )
      return 0;

   hq        = calloc( 1, sizeof( HookQueue_t ) );
   hq->stack = strdup( stack );
   i         = 0;
   if ( param != NULL ) {
      for ( ; param[i].type != HOOK_PARAM_SENTINEL; i++ )
         hq->hparam[i] = param[i];
   }
#ifdef DEBUGGING
   if ( i >= HOOK_MAX_PARAM )
      WARN( _( "HOOK_MAX_PARAM is set too low (%d), need at least %d!" ),
            HOOK_MAX_PARAM, i );
#endif /* DEBUGGING */
   hq->hparam[i].type = HOOK_PARAM_SENTINEL;
   hq_add( hq );
   return 0;
}

/**
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @param param Parameters to pass.
 *    @return 0 on success.
 */
int hooks_runParam( const char *stack, const HookParam *param )
{
   /* Don't update if player is dead. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_DESTROYED ) )
      return 0;

   /* Not time to run hooks, so queue them. */
   if ( hook_atomic )
      return hooks_runParamDeferred( stack, param );

   /* Execute. */
   return hooks_executeParam( stack, param );
}

/**
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @return 0 on success.
 */
int hooks_run( const char *stack )
{
   return hooks_runParam( stack, NULL );
}

/**
 * @brief Gets a hook by ID.
 */
static Hook *hook_get( unsigned int id )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      if ( h->id == id )
         return h;

   return NULL;
}

/**
 * @brief Gets the lua env for a hook.
 */
nlua_env *hook_env( unsigned int hook )
{
   Mission *misn;

   Hook *h = hook_get( hook );
   if ( h == NULL )
      return NULL;

   switch ( h->type ) {
   case HOOK_TYPE_MISN:
      misn = hook_getMission( h );
      if ( misn != NULL )
         return misn->env;
      break;
   case HOOK_TYPE_EVENT:
      return event_getEnv( h->u.event.parent );
   default:
      break;
   }

   return NULL;
}

/**
 * @brief Runs a single hook by id.
 *
 *    @param id Identifier of the hook to run.
 *    @param param Parameters to process.
 *    @return The ID of the hook or 0 if it got deleted.
 */
int hook_runIDparam( unsigned int id, const HookParam *param )
{
   Hook *h;

   /* Don't update if player is dead. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_DESTROYED ) )
      return 0;

   /* Try to find the hook and run it. */
   h = hook_get( id );
   if ( h == NULL ) {
      WARN( _( "Attempting to run hook of id '%d' which is not in the stack" ),
            id );
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
   /* Remove from all the pilots. */
   pilots_rmHook( h->id );

   /* Generic freeing. */
   free( h->stack );

   /* Free type specific. */
   switch ( h->type ) {
   case HOOK_TYPE_MISN:
      free( h->u.misn.func );
      break;

   case HOOK_TYPE_EVENT:
      free( h->u.event.func );
      break;

   default:
      break;
   }

   free( h );
}

/**
 * @brief Gets rid of all current hooks.
 */
void hook_cleanup( void )
{
   Hook *h;

   /* Clear queued hooks. */
   hq_clear();

   h = hook_list;
   while ( h != NULL ) {
      Hook *hn = h->next;
      hook_free( h );
      h = hn;
   }
   /* safe defaults just in case */
   hook_list = NULL;
}

/**
 * @brief Clears the hooks.
 */
void hook_clear( void )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next ) {
      if ( h->delete )
         continue;
      hook_rmRaw( h );
   }
}

/**
 * @brief Clears the timer hooks for a mission.
 */
void hook_clearMissionTimers( unsigned int parent )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next ) {
      if ( !h->is_timer )
         continue;
      if ( ( h->type == HOOK_TYPE_MISN ) && ( parent == h->u.misn.parent ) )
         h->delete = 1;
   }
}

/**
 * @brief Clears the timer hooks for an event.
 */
void hook_clearEventTimers( unsigned int parent )
{
   for ( Hook *h = hook_list; h != NULL; h = h->next ) {
      if ( !h->is_timer )
         continue;
      if ( ( h->type == HOOK_TYPE_EVENT ) && ( parent == h->u.event.parent ) )
         h->delete = 1;
   }
}

/**
 * @brief Checks if a hook needs to be saved.
 *
 *    @param h Hook to check if it should be saved.
 *    @return 1 if hook should be saved.
 */
static int hook_needSave( Hook *h )
{
   const char *nosave[] = { "p_death", "p_board",    "p_disable",
                            "p_jump",  "p_attacked", "p_idle", /* pilot hooks */
                            "timer",                           /* timers */
                            "end" };

   /* Impossible to save functions. */
   if ( h->type == HOOK_TYPE_FUNC )
      return 0;

   /* Events must need saving. */
   if ( ( h->type == HOOK_TYPE_EVENT ) && !event_save( h->u.event.parent ) )
      return 0;

   /* Must not be pending deletion. */
   if ( h->delete )
      return 0;

   /* Make sure it's in the proper stack. */
   for ( int i = 0; strcmp( nosave[i], "end" ) != 0; i++ )
      if ( strcmp( nosave[i], h->stack ) == 0 )
         return 0;

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
   xmlw_startElem( writer, "hooks" );
   for ( Hook *h = hook_list; h != NULL; h = h->next ) {

      if ( !hook_needSave( h ) )
         continue; /* no need to save it */

      xmlw_startElem( writer, "hook" );

      switch ( h->type ) {
      case HOOK_TYPE_MISN:
         xmlw_attr( writer, "type", "misn" ); /* Save attribute. */
         xmlw_elem( writer, "parent", "%u", h->u.misn.parent );
         xmlw_elem( writer, "func", "%s", h->u.misn.func );
         break;

      case HOOK_TYPE_EVENT:
         xmlw_attr( writer, "type", "event" ); /* Save attribute. */
         xmlw_elem( writer, "parent", "%u", h->u.event.parent );
         xmlw_elem( writer, "func", "%s", h->u.event.func );
         break;

      default:
         WARN( _( "Something has gone screwy here..." ) );
         break;
      }

      /* Generic information. */
      xmlw_elem( writer, "id", "%u", h->id );
      xmlw_elem( writer, "stack", "%s", h->stack );

      /* Store additional date information. */
      if ( h->is_date )
         xmlw_elem( writer, "resolution", "%" PRId64, h->res );

      xmlw_endElem( writer ); /* "hook" */
   }
   xmlw_endElem( writer ); /* "hooks" */

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

   /* We're loading. */
   hook_loadingstack = 1;

   node = parent->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "hooks" ) )
         hook_parse( node );
   } while ( xml_nextNode( node ) );

   /* Done loading. */
   hook_loadingstack = 0;

   /* Set ID gen to highest hook. */
   for ( Hook *h = hook_list; h != NULL; h = h->next )
      hook_id = MAX( h->id, hook_id );

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
   xmlNodePtr   node, cur;
   char        *func, *stack, *stype;
   unsigned int parent, id, new_id;
   HookType_t   type;
   Hook        *h;
   int          is_date;
   ntime_t      res = 0;

   node = base->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "hook" ) ) {
         id      = 0;
         parent  = 0;
         func    = NULL;
         stack   = NULL;
         is_date = 0;

         /* Handle the type. */
         xmlr_attr_strd( node, "type", stype );
         /* Default to mission for old saves. */
         if ( stype == NULL )
            type = HOOK_TYPE_MISN;
         /* Mission type. */
         else if ( strcmp( stype, "misn" ) == 0 ) {
            type = HOOK_TYPE_MISN;
            free( stype );
         }
         /* Event type. */
         else if ( strcmp( stype, "event" ) == 0 ) {
            type = HOOK_TYPE_EVENT;
            free( stype );
         }
         /* Unknown. */
         else {
            WARN( _( "Hook of unknown type '%s' found, skipping." ), stype );
            free( stype );
            continue;
         }

         /* Handle the data. */
         cur = node->xmlChildrenNode;
         do {
            xml_onlyNodes( cur );

            /* ID. */
            xmlr_long( cur, "id", id );

            /* Generic. */
            xmlr_str( cur, "stack", stack );

            /* Type specific. */
            if ( ( type == HOOK_TYPE_MISN ) || ( type == HOOK_TYPE_EVENT ) ) {
               xmlr_uint( cur, "parent", parent );
               xmlr_str( cur, "func", func );
            }

            /* Check for date hook. */
            if ( xml_isNode( cur, "resolution" ) ) {
               is_date = 1;
               res     = xml_getLong( cur );
               continue;
            }

            // cppcheck-suppress nullPointerRedundantCheck
            WARN( _( "Save has unknown hook node '%s'." ), cur->name );
         } while ( xml_nextNode( cur ) );

         /* Check for validity. */
         if ( ( parent == 0 ) || ( func == NULL ) || ( stack == NULL ) ) {
            WARN( _( "Invalid hook." ) );
            continue;
         }

         /* Create the hook. */
         switch ( type ) {
         case HOOK_TYPE_MISN:
            new_id = hook_addMisn( parent, func, stack );
            break;
         case HOOK_TYPE_EVENT:
            new_id = hook_addEvent( parent, func, stack );
            break;
         default:
            WARN( _( "Save has unsupported hook type." ) );
            continue;
         }

         /* Set the id. */
         if ( id != 0 ) {
            h     = hook_get( new_id );
            h->id = id;

            /* Additional info. */
            if ( is_date ) {
               h->is_date = 1;
               h->res     = res;
            }
         }
      }
   } while ( xml_nextNode( node ) );

   return 0;
}
