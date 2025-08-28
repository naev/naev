/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file comm.c
 *
 * @brief For communicating with spobs/pilots.
 */
#include "comm.h"

#include "ai.h"
#include "array.h"
#include "escort.h"
#include "hook.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "pilot.h"
#include "player.h"

#define BUTTON_WIDTH 80  /**< Button width. */
#define BUTTON_HEIGHT 30 /**< Button height. */

#define GRAPHIC_WIDTH 256  /**< Width of graphic. */
#define GRAPHIC_HEIGHT 256 /**< Height of graphic. */

static Spob     *comm_spob      = NULL; /**< Spob currently talking to. */
static int       comm_commClose = 0;    /**< Close comm when done. */
static nlua_env *comm_env       = NULL; /**< Comm Lua env. */
static int       comm_open      = 0;

/*
 * Prototypes.
 */
/* Static. */
static const char *comm_getString( const Pilot *p, const char *str );

/**
 * @brief Check to see if the comm window is open.
 */
int comm_isOpen( void )
{
   return comm_open;
}

/**
 * @brief Queues a close command when possible.
 */
void comm_queueClose( void )
{
   comm_commClose = 1;
}

/**
 * @brief Opens the communication dialogue with a pilot.
 *
 *    @param pilot Pilot to communicate with.
 *    @return 0 on success.
 */
int comm_openPilot( unsigned int pilot )
{
   const char   *msg;
   char          c;
   Pilot        *p;
   Pilot *const *pltstk;
   AIMemory      oldmem;

   /* Get the pilot. */
   p = pilot_get( pilot );

   /* Make sure pilot exists. */
   if ( p == NULL )
      return -1;

   /* Make sure pilot in range. */
   if ( !pilot_isFlag( p, PILOT_HAILING ) &&
        pilot_inRangePilot( player.p, p, NULL ) <= 0 ) {
      player_message( _( "#rTarget is out of communications range" ) );
      return -1;
   }
   c = pilot_getFactionColourChar( p );

   /* Must not be jumping. */
   if ( pilot_isFlag( p, PILOT_HYPERSPACE ) ) {
      player_message( _( "#%c%s#r is jumping and can't respond" ), c, p->name );
      return 0;
   }

   /* Must not be disabled, unless hailing. */
   if ( !pilot_isFlag( p, PILOT_HAILING ) &&
        pilot_isFlag( p, PILOT_DISABLED ) ) {
      player_message( _( "#%c%s#r does not respond" ), c, p->name );
      return 0;
   }

   /* Set up for the comm_get* functions. */
   oldmem = ai_setPilot( p );

   /* Have pilot stop hailing. */
   pilot_rmFlag( p, PILOT_HAILING );

   /* Don't close automatically. */
   comm_commClose = 0;

   /* Run specific hail hooks on hailing pilot. */
   if ( pilot_canTarget( p ) ) {
      HookParam hparam[] = { { .type = HOOK_PARAM_PILOT, .u = { .lp = p->id } },
                             { .type = HOOK_PARAM_SENTINEL } };
      hooks_runParam( "hail", hparam );
      pilot_runHook( p, PILOT_HOOK_HAIL );
   }

   /* Check for player faction (escorts). Should be moved to the comm script
    * most likely. For now, we just run it after hooks if it hasn't been closed
    * already. */
   if ( !comm_commClose && p->faction == FACTION_PLAYER ) {
      escort_playerCommand( p );
      ai_unsetPilot( oldmem );
      return 0;
   }

   /* Check to see if pilot wants to communicate. */
   msg = comm_getString( p, "comm_no" );
   if ( msg != NULL ) {
      if ( comm_commClose == 0 ) {
         char col = pilot_getFactionColourChar( p );
         player_message( _( "#%c%s>#0 %s" ), col, p->name, msg );
      }
      ai_unsetPilot( oldmem );
      return 0;
   }

   /* Run generic hail hooks on all pilots. */
   pltstk = pilot_getAll();
   for ( int i = 0; i < array_size( pltstk ); i++ )
      ai_hail( pltstk[i] );

   /* Close window if necessary. */
   if ( comm_commClose ) {
      comm_spob      = NULL;
      comm_commClose = 0;
      ai_unsetPilot( oldmem );
      return 0;
   }

   /* Set up environment first time. */
   if ( comm_env == NULL ) {
      comm_env = nlua_newEnv( "comm" );
      nlua_loadStandard( comm_env );

      size_t bufsize;
      char  *buf = ndata_read( COMM_PATH, &bufsize );
      if ( nlua_dobufenv( comm_env, buf, bufsize, COMM_PATH ) != 0 ) {
         WARN( _( "Error loading file: %s\n"
                  "%s\n"
                  "Most likely Lua file has improper syntax, please check" ),
               COMM_PATH, lua_tostring( naevL, -1 ) );
         free( buf );
         ai_unsetPilot( oldmem );
         return -1;
      }
      free( buf );
   }

   comm_open = 1;

   /* Run Lua. */
   nlua_getenv( naevL, comm_env, "comm" );
   lua_pushpilot( naevL, p->id );
   if ( nlua_pcall( comm_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Comm: '%s'" ), lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }

   ai_unsetPilot( oldmem );
   comm_open = 0;

   return 0;
}

/**
 * @brief Opens a communication dialogue with a spob.
 *
 *    @param spob Spob to communicate with.
 *    @return 0 on success.
 */
int comm_openSpob( Spob *spob )
{
   /* Don't close automatically. */
   comm_commClose = 0;

   /* Run hail_spob hook. */
   HookParam hparam[] = {
      { .type = HOOK_PARAM_SPOB, .u = { .la = spob_index( spob ) } },
      { .type = HOOK_PARAM_SENTINEL } };
   hooks_runParam( "hail_spob", hparam );

   /* Close window if necessary. */
   if ( comm_commClose ) {
      comm_spob      = NULL;
      comm_commClose = 0;
      return 0;
   }

   /* Lua stuff. */
   if ( spob->lua_comm != LUA_NOREF ) {
      comm_open = 1;
      spob_luaInitMem( spob );
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, spob->lua_comm ); /* f */
      if ( nlua_pcall( spob->lua_env, 0, 1 ) ) {
         WARN( _( "Spob '%s' failed to run '%s':\n%s" ), spob->name, "comm",
               lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      } else {
         int commed = lua_toboolean( naevL, -1 );
         lua_pop( naevL, 1 );
         if ( commed ) {
            comm_open = 0;
            return 0;
         }
      }
      comm_open = 0;
   }

   player_message( _( "%s does not respond." ), spob_name( spob ) );
   return 0;
}

/**
 * @brief Gets a string from the pilot's memory.
 *
 * Valid targets are:
 *    - comm_no: message of communication failure.
 *    - bribe_no: unbribe message
 *    - bribe_prompt: bribe prompt
 *    - bribe_paid: paid message
 *
 *    @param p Pilot to get string from.
 *    @param str String to get.
 *    @return String matching string.
 */
static const char *comm_getString( const Pilot *p, const char *str )
{
   const char *ret;

   if ( p->ai == NULL )
      return NULL;

   /* Get memory table. */
   nlua_getenv( naevL, p->ai->env, "mem" );

   /* Get str message. */
   lua_getfield( naevL, -1, str );
   if ( !lua_isstring( naevL, -1 ) )
      ret = NULL;
   else
      ret = lua_tostring( naevL, -1 );
   lua_pop( naevL, 2 );

   return ret;
}
