/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file gatherable.c
 *
 * @brief Handles gatherable objects.
 */
/** @cond */
#include <stdio.h>
#include <stdint.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "gatherable.h"

#include "array.h"
#include "hook.h"
#include "player.h"

/* Gatherables */
#define GATHER_DIST 30. /**< Maximum distance a gatherable can be gathered. */

/* gatherables stack */
static Gatherable* gatherable_stack = NULL; /**< Contains the gatherable stuff floating around. */
static float noscoop_timer = 1.; /**< Timer for the "full cargo" message . */

int gatherable_load (void)
{
   gatherable_stack = array_create( Gatherable );
   return 0;
}

void gatherable_cleanup (void)
{
   array_free( gatherable_stack );
   gatherable_stack = NULL;
}

/**
 * @brief Initializes a gatherable object
 *
 *    @param com Type of commodity.
 *    @param pos Position.
 *    @param vel Velocity.
 *    @param lifeleng Duration in seconds.
 *    @param qtt Quantity to add.
 *    @param player_only Whether the gatherable can only be gathered by the player.
 */
int gatherable_init( const Commodity* com, const vec2 *pos, const vec2 *vel, double lifeleng, int qtt, unsigned int player_only )
{
   Gatherable *g = &array_grow( &gatherable_stack );
   g->type = com;
   g->pos = *pos;
   g->vel = *vel;
   g->timer = 0.;
   g->quantity = qtt;
   g->sx = RNG( 0, com->gfx_space->sx );
   g->sy = RNG( 0, com->gfx_space->sy );
   g->player_only = player_only;

   if (lifeleng < 0.)
      g->lifeleng = RNGF()*100. + 50.;
   else
      g->lifeleng = lifeleng;

   return g-gatherable_stack;
}

/**
 * @brief Updates all gatherable objects
 *
 *    @param dt Elapsed time.
 */
void gatherable_update( double dt )
{
   /* Update the timer for "full cargo" message. */
   noscoop_timer += dt;

   for (int i=0; i < array_size(gatherable_stack); i++) {
      Gatherable *g = &gatherable_stack[i];
      g->timer += dt;
      g->pos.x += dt*gatherable_stack[i].vel.x;
      g->pos.y += dt*gatherable_stack[i].vel.y;

      /* Remove the gatherable */
      if (g->timer > g->lifeleng) {
         array_erase( &gatherable_stack, g, g+1 );
         i--;
      }
   }
}

/**
 * @brief Frees all the gatherables
 */
void gatherable_free( void )
{
   array_erase( &gatherable_stack, array_begin(gatherable_stack), array_end(gatherable_stack) );
}

/**
 * @brief Renders all the gatherables
 */
void gatherable_render( void )
{
   for (int i=0; i < array_size(gatherable_stack); i++) {
      const Gatherable *gat = &gatherable_stack[i];
      gl_renderSprite( gat->type->gfx_space, gat->pos.x, gat->pos.y, gat->sx, gat->sy, NULL );
   }
}

/**
 * @brief Gets the closest gatherable from a given position, within a given radius
 *
 *    @param pos position.
 *    @param rad radius.
 *    @return The id of the closest gatherable, or -1 if none is found.
 */
int gatherable_getClosest( const vec2 *pos, double rad )
{
   int curg = -1;
   double mindist = INFINITY;

   for (int i=0; i < array_size(gatherable_stack); i++) {
      Gatherable *gat = &gatherable_stack[i];
      double curdist = vec2_dist(pos, &gat->pos);
      if ( (curdist<mindist) && (curdist<rad) ) {
         curg = i;
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
 *    @return flag 1->there exists a gatherable 0->elsewere.
 */
int gatherable_getPos( vec2* pos, vec2* vel, int id )
{
   Gatherable *gat;

   if ((id < 0) || (id > array_size(gatherable_stack)-1) ) {
      vectnull( pos );
      vectnull( vel );
      return 0;
   }

   gat = &gatherable_stack[id];
   *pos = gat->pos;
   *vel = gat->vel;

   return 1;
}

/**
 * @brief See if the pilot can gather anything
 *
 *    @param p Pilot to try to gather for.
 */
void gatherable_gather( Pilot *p )
{
   for (int i=0; i < array_size(gatherable_stack); i++) {
      Gatherable *gat = &gatherable_stack[i];

      /* Only player can gather player only stuff. */
      if (gat->player_only && !pilot_isPlayer(p))
         continue;

      if (vec2_dist( &p->solid.pos, &gat->pos ) < GATHER_DIST ) {
         /* Add cargo to pilot. */
         int q = pilot_cargoAdd( p, gat->type, gat->quantity, 0 );

         if (q>0) {
            if (pilot_isPlayer(p)) {
               HookParam hparam[3];
               player_message( n_("%d ton of %s gathered", "%d tons of %s gathered", q), q, _(gat->type->name) );

               /* Run hooks. */
               hparam[0].type    = HOOK_PARAM_STRING;
               hparam[0].u.str   = gat->type->name;
               hparam[1].type    = HOOK_PARAM_NUMBER;
               hparam[1].u.num   = q;
               hparam[2].type    = HOOK_PARAM_SENTINEL;
               hooks_runParam( "gather", hparam );
            }

            /* Remove the object from space. */
            array_erase( &gatherable_stack, gat, gat+1 );

            /* Test if there is still cargo space */
            if ((pilot_cargoFree(p) < 1) && (pilot_isPlayer(p)))
               player_message( _("No more cargo space available") );
         }
         else if ((pilot_isPlayer(p)) && (noscoop_timer > 2.)) {
            noscoop_timer = 0.;
            player_message( _("Cannot gather material: no more cargo space available") );
         }
      }
   }
}
