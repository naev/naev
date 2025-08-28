/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file gatherable.c
 *
 * @brief Handles gatherable objects.
 */
/** @cond */
#include "vec2.h"

#include "naev.h"
/** @endcond */

#include "gatherable.h"

#include "array.h"
#include "hook.h"
#include "player.h"
#include "rng.h"

/* Gatherables */
#define GATHER_DIST 30. /**< Maximum distance a gatherable can be gathered. */

/**
 * @struct Gatherable
 *
 * @brief Represents stuff that can be gathered.
 */
typedef struct Gatherable_ {
   const Commodity *type;        /**< Type of commodity. */
   vec2             pos;         /**< Position. */
   vec2             vel;         /**< Velocity. */
   double           timer;       /**< Timer to de-spawn the gatherable. */
   double           lifeleng;    /**< nb of seconds before de-spawn. */
   int              quantity;    /**< Quantity of material. */
   int              sx;          /**< X sprite to use. */
   int              sy;          /**< Y sprite to use. */
   int              player_only; /**< Can only be gathered by player. */
} Gatherable;

/* gatherables stack */
static Gatherable *gatherable_stack =
   NULL; /**< Contains the gatherable stuff floating around. */
static float   noscoop_timer = 1.; /**< Timer for the "full cargo" message . */
static IntList gather_qtquery;     /**< For collisions. */

/* Prototypes. */
static int gatherable_gather( Gatherable *got, Pilot *p );

/**
 * @brief Loads the gatherable system.
 *
 *    @return 0 on success.
 */
int gatherable_load( void )
{
   gatherable_stack = array_create( Gatherable );
   il_create( &gather_qtquery, 1 );
   return 0;
}

/**
 * @brief Cleans up after the gatherable system.
 */
void gatherable_cleanup( void )
{
   array_free( gatherable_stack );
   gatherable_stack = NULL;
   il_destroy( &gather_qtquery );
}

/**
 * @brief Initializes a gatherable object
 *
 *    @param com Type of commodity.
 *    @param pos Position.
 *    @param vel Velocity.
 *    @param lifeleng Duration in seconds.
 *    @param qtt Quantity to add.
 *    @param player_only Whether the gatherable can only be gathered by the
 * player.
 */
int gatherable_init( const Commodity *com, const vec2 *pos, const vec2 *vel,
                     double lifeleng, int qtt, unsigned int player_only )
{
   Gatherable *g = &array_grow( &gatherable_stack );
   memset( g, 0, sizeof( Gatherable ) );
   g->type        = com;
   g->pos         = *pos;
   g->vel         = *vel;
   g->timer       = 0.;
   g->quantity    = qtt;
   g->sx          = RNG( 0, tex_sx( com->gfx_space ) - 1 );
   g->sy          = RNG( 0, tex_sy( com->gfx_space ) - 1 );
   g->player_only = player_only;

   if ( lifeleng < 0. )
      g->lifeleng = RNGF() * 100. + 50.;
   else
      g->lifeleng = lifeleng;

   return g - gatherable_stack;
}

/**
 * @brief Updates all gatherable objects
 *
 *    @param dt Elapsed time.
 */
void gatherable_update( double dt )
{
   Pilot *const *pilot_stack = pilot_getAll();

   /* Update the timer for "full cargo" message. */
   noscoop_timer += dt;

   for ( int i = array_size( gatherable_stack ) - 1; i >= 0; i-- ) {
      Gatherable *g = &gatherable_stack[i];
      const int   r = ceil( GATHER_DIST );
      int         x, y;

      g->timer += dt;
      g->pos.x += dt * g->vel.x;
      g->pos.y += dt * g->vel.y;

      /* Remove the gatherable */
      if ( g->timer > g->lifeleng ) {
         array_erase( &gatherable_stack, g, g + 1 );
         continue;
      }

      /* Only player can gather player only stuff. */
      if ( g->player_only ) {
         if ( ( player.p != NULL ) &&
              vec2_dist2( &player.p->solid.pos, &g->pos ) <=
                 pow2( GATHER_DIST ) )
            gatherable_gather( g, player.p );
         continue;
      }

      /* Check if can be picked up. */
      x = round( g->pos.x );
      y = round( g->pos.y );
      pilot_collideQueryIL( &gather_qtquery, x - r, y - r, x + r, y + r );
      for ( int j = 0; j < il_size( &gather_qtquery ); j++ ) {
         Pilot *p = pilot_stack[il_get( &gather_qtquery, j, 0 )];

         /* Fighters don't pick up stuff. */
         if ( pilot_isFlag( p, PILOT_CARRIED ) )
            continue;

         /* See if in distance. */
         if ( vec2_dist2( &p->solid.pos, &g->pos ) > pow2( GATHER_DIST ) )
            continue;

         /* Try to pick up. */
         if ( gatherable_gather( g, p ) )
            break;
      }
   }
}

/**
 * @brief Frees all the gatherables
 */
void gatherable_free( void )
{
   array_erase( &gatherable_stack, array_begin( gatherable_stack ),
                array_end( gatherable_stack ) );
}

/**
 * @brief Renders all the gatherables
 */
void gatherable_render( void )
{
   for ( int i = 0; i < array_size( gatherable_stack ); i++ ) {
      const Gatherable *gat = &gatherable_stack[i];
      gl_renderSprite( gat->type->gfx_space, gat->pos.x, gat->pos.y, gat->sx,
                       gat->sy, NULL );
   }
}

/**
 * @brief Gets the closest gatherable from a given position, within a given
 * radius
 *
 *    @param pos position.
 *    @param rad radius.
 *    @return The id of the closest gatherable, or -1 if none is found.
 */
int gatherable_getClosest( const vec2 *pos, double rad )
{
   int    curg    = -1;
   double mindist = INFINITY;

   for ( int i = 0; i < array_size( gatherable_stack ); i++ ) {
      Gatherable *gat     = &gatherable_stack[i];
      double      curdist = vec2_dist( pos, &gat->pos );
      if ( ( curdist < mindist ) && ( curdist < rad ) ) {
         curg    = i;
         mindist = curdist;
      }
   }
   return curg;
}

/**
 * @brief Returns the position and velocity of a gatherable
 *
 *    @param pos pointer to the position.
 *    @param vel pointer to the velocity.
 *    @param id Id of the gatherable in the stack.
 *    @return flag 1->there exists a gatherable 0->elsewhere.
 */
int gatherable_getPos( vec2 *pos, vec2 *vel, int id )
{
   Gatherable *gat;

   if ( ( id < 0 ) || ( id > array_size( gatherable_stack ) - 1 ) ) {
      vectnull( pos );
      vectnull( vel );
      return 0;
   }

   gat  = &gatherable_stack[id];
   *pos = gat->pos;
   *vel = gat->vel;

   return 1;
}

/**
 * @brief See if the pilot can gather anything
 *
 *    @param gat Gatherable to try to gather.
 *    @param p Pilot to try to gather for.
 */
static int gatherable_gather( Gatherable *gat, Pilot *p )
{
   int q;

   /* Must not be dead. */
   if ( pilot_isFlag( p, PILOT_DELETE ) || pilot_isFlag( p, PILOT_DEAD ) )
      return 0;

   /* Must not be hidden nor invisible. */
   if ( pilot_isFlag( p, PILOT_HIDE ) || pilot_isFlag( p, PILOT_INVISIBLE ) )
      return 0;

   /* Disabled pilots can't pick up stuff. */
   if ( pilot_isDisabled( p ) )
      return 0;

   /* Try to add cargo to pilot. */
   q = pilot_cargoAdd( p, gat->type, gat->quantity, 0 );

   if ( q > 0 ) {
      if ( pilot_isPlayer( p ) ) {
         HookParam hparam[3];
         player_message(
            n_( "%d ton of %s gathered", "%d tons of %s gathered", q ), q,
            _( gat->type->name ) );

         /* Run hooks. */
         hparam[0].type        = HOOK_PARAM_COMMODITY;
         hparam[0].u.commodity = (Commodity *)gat->type; /* TODO not cast. */
         hparam[1].type        = HOOK_PARAM_NUMBER;
         hparam[1].u.num       = q;
         hparam[2].type        = HOOK_PARAM_SENTINEL;
         hooks_runParam( "gather", hparam );
      }

      /* Remove the object from space. */
      array_erase( &gatherable_stack, gat, gat + 1 );

      /* Test if there is still cargo space */
      if ( ( pilot_cargoFree( p ) < 1 ) && ( pilot_isPlayer( p ) ) )
         player_message( _( "No more cargo space available" ) );
      return 1;
   } else if ( ( pilot_isPlayer( p ) ) && ( noscoop_timer > 2. ) ) {
      noscoop_timer = 0.;
      player_message(
         _( "Cannot gather material: no more cargo space available" ) );
   }
   return 0;
}
