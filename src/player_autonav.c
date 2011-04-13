/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file player_autonav.c
 *
 * @brief Contains all the player autonav related stuff.
 */


#include "player.h"

#include "naev.h"

#include "toolkit.h"
#include "pause.h"
#include "space.h"
#include "conf.h"


extern double player_acc; /**< Player acceleration. */
extern int map_npath; /**< @todo remove */

static double tc_mod    = 1.; /**< Time compression modifier. */
static double tc_max    = 1.; /**< Maximum time compression. */
static double tc_down   = 0.; /**< Rate of decrement. */
static int tc_rampdown  = 0; /**< Ramping down time compression? */
static double starts;
static double starta;
static double abort_mod = 1.;


/*
 * Prototypes.
 */
static void player_autonavSetup (void);
static void player_autonav (void);
static int player_autonavApproach( Vector2d *pos, double *dist2, int count_target );
static int player_autonavBrake (void);


/**
 * @brief Starts autonav.
 */
void player_autonavStart (void)
{
   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   if (player.p->nav_hyperspace == -1)
      return;

   if (player.p->fuel < HYPERSPACE_FUEL) {
      player_message("\erNot enough fuel to jump for autonav.");
      return;
   }

   if (pilot_isFlag( player.p, PILOT_NOJUMP)) {
      player_message("\erHyperspace drive is offline.");
      return;
   }

   if player_isFlag(PLAYER_AUTONAV) {
      player_autonavAbort(NULL);
      return;
   }
   player_autonavSetup();
   player.autonav = AUTONAV_JUMP_APPROACH;
}


/**
 * @brief Prepares the player to enter autonav.
 */
static void player_autonavSetup (void)
{
   player_message("\epAutonav initialized.");
   if (!player_isFlag(PLAYER_AUTONAV)) {
      tc_mod         = 1.;
      tc_max         = conf.compression_velocity / solid_maxspeed(player.p->solid, player.p->speed, player.p->thrust);
   }
   tc_rampdown    = 0;
   tc_down        = 0.;
   starts         = player.p->shield / player.p->shield_max;
   starta         = player.p->armour / player.p->armour_max;
   if (player.autonav_timer <= 0.)
      abort_mod = 1.;
   player_setFlag(PLAYER_AUTONAV);
}


/**
 * @brief Ends the autonav.
 */
void player_autonavEnd (void)
{
   player_rmFlag(PLAYER_AUTONAV);
   if (player_isFlag(PLAYER_DOUBLESPEED)) {
     tc_mod         = 2.;
     pause_setSpeed( 2. );
   } else {
     tc_mod         = 1.;
     pause_setSpeed( 1. );
   }
}


/**
 * @brief Starts autonav and closes the window.
 */
void player_autonavStartWindow( unsigned int wid, char *str)
{
   (void) str;
   player_autonavStart();
   window_destroy( wid );
}


/**
 * @brief Starts autonav with a local position destination.
 */
void player_autonavPos( double x, double y )
{
   player_autonavSetup();
   player.autonav    = AUTONAV_POS_APPROACH;
   vect_cset( &player.autonav_pos, x, y );
}


/**
 * @brief Aborts autonav.
 */
void player_autonavAbort( const char *reason )
{
   /* No point if player is beyond aborting. */
   if ((player.p==NULL) || ((player.p != NULL) && pilot_isFlag(player.p, PILOT_HYPERSPACE)))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (reason != NULL)
         player_message("\erAutonav aborted: %s!", reason);
      else
         player_message("\erAutonav aborted!");
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player.p);
         player_message("\epAborting hyperspace sequence.");
      }

      /* Reset time compression. */
      player_autonavEnd();
   }
}


/**
 * @brief Handles the autonavigation process for the player.
 */
static void player_autonav (void)
{
   JumpPoint *jp;
   int ret;
   double d, t, tint;
   double vel;

   switch (player.autonav) {
      case AUTONAV_JUMP_APPROACH:
         /* Target jump. */
         jp    = &cur_system->jumps[ player.p->nav_hyperspace ];
         ret   = player_autonavApproach( &jp->pos, &d, 0 );
         if (ret)
            player.autonav = AUTONAV_JUMP_BRAKE;
         else if (!tc_rampdown && (map_npath<=1)) {
            vel   = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) );
            t     = d / vel * 1.1;
            /* tint is the integral of the time in per time units.
             *
             * tc_mod
             *    ^
             *    |
             *    |\
             *    | \
             *    |  \___
             *    |
             *    +------> time
             *    0   3
             *
             * We decompose integral in a rectangle (3*1) and a triangle (3*(tc_mod-1.))/2.
             *  This is the "elapsed time" when linearly decreasing the tc_mod. Which we can
             *  use to calculate the actual "game time" that'll pass when decreasing the
             *  tc_mod to 1 during 3 seconds. This can be used then to compare when we want to
             *  start decrementing.
             */
            tint  = 3. + 0.5*(3.*(tc_mod-1.));
            if (t < tint) {
               tc_rampdown = 1;
               tc_down     = (tc_mod-1.) / 3.;
            }
         }
         break;

      case AUTONAV_JUMP_BRAKE:
         /* Target jump. */
         jp    = &cur_system->jumps[ player.p->nav_hyperspace ];
         ret   = player_autonavBrake();
         /* Try to jump or see if braked. */
         if (ret) {
            if (space_canHyperspace(player.p))
               player_jump();
            player.autonav = AUTONAV_JUMP_APPROACH;
         }

         /* See if should ramp down. */
         if (!tc_rampdown && (map_npath<=1)) {
            tc_rampdown = 1;
            tc_down     = (tc_mod-1.) / 3.;
         }
         break;
   
      case AUTONAV_POS_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret) {
            player_message( "\epAutonav arrived at position." );
            player_autonavEnd();
         }
         else if (!tc_rampdown) {
            vel   = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) );
            t     = d / vel * 0.925;
            tint  = 3. + 0.5*(3.*(tc_mod-1.));
            if (t < tint) {
               tc_rampdown = 1;
               tc_down     = (tc_mod-1.) / 3.;
            }
         }
         break;
   }
}


/**
 * @brief Handles approaching a position with autonav.
 *
 *    @param pos Position to go to.
 *    @return 1 on completion.
 */
static int player_autonavApproach( Vector2d *pos, double *dist2, int count_target )
{
   double d, time, vel, dist;

   /* Only accelerate if facing move dir. */
   d = pilot_face( player.p, vect_angle( &player.p->solid->pos, pos ) );
   if (FABS(d) < MIN_DIR_ERR) {
      if (player_acc < 1.)
         player_accel( 1. );
   }
   else if (player_acc > 0.)
      player_accelOver();

   /* Get current time to reach target. */
   time  = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) ) /
      (player.p->thrust / player.p->solid->mass);

   /* Get velocity. */
   vel   = MIN( player.p->speed, VMOD(player.p->solid->vel) );

   /* Get distance. */
   dist  = vel*(time+1.1*M_PI/player.p->turn) -
      0.5*(player.p->thrust/player.p->solid->mass)*time*time;

   /* Output distance^2 */
   d        = vect_dist( pos, &player.p->solid->pos );
   dist     = d - dist;
   if (count_target)
      *dist2   = dist;
   else
      *dist2   = d;

   /* See if should start braking. */
   if (dist < 0.) {
      player_accelOver();
      return 1;
   }
   return 0;
}


/**
 * @brief Handles the autonav braking.
 *
 *    @return 1 on completion.
 */
static int player_autonavBrake (void)
{
   double d;

   /* Braking procedure. */
   d = pilot_face( player.p, VANGLE(player.p->solid->vel) + M_PI );
   if (FABS(d) < MIN_DIR_ERR) {
      if (player_acc < 1.)
         player_accel( 1. );
   }
   else if (player_acc > 0.)
      player_accelOver();

   if (VMOD(player.p->solid->vel) < MIN_VEL_ERR) {
      player_accelOver();
      return 1;
   }
   return 0;
}

/**
 * @brief Checks whether the player should abort autonav due to damage or missile locks.
 *
 *    @return 1 if autonav should be aborted.
 */
int player_shouldAbortAutonav( int damaged )
{
   double failpc = conf.autonav_abort * abort_mod;
   double shield = player.p->shield / player.p->shield_max;
   double armour = player.p->armour / player.p->armour_max;

   if (failpc >= 1. && player.p->lockons > 0)
      player_autonavAbort("Missile Lockon Detected");
   else if (failpc >= 1. && (shield < 1. && shield < starts) && damaged)
      player_autonavAbort("Sustaining damage");
   else if (failpc > 0. && (shield < failpc && shield < starts) && damaged)
      player_autonavAbort("Shield below damage threshold");
   else if (armour < starta && damaged)
      player_autonavAbort("Sustaining armour damage");

   if (!player_isFlag(PLAYER_AUTONAV)) {
      if (player.autonav_timer > 0.)
         abort_mod = MAX( 0., abort_mod - .2 );
      else
         abort_mod = .8;
      player.autonav_timer = 30.;
      return 1;
   }
   return 0;
}


/**
 * @brief Handles autonav thinking.
 *
 *    @param pplayer Player doing the thinking.
 */
void player_thinkAutonav( Pilot *pplayer, double dt )
{
   if (player.autonav_timer > 0.)
      player.autonav_timer -= dt;
   if (player_shouldAbortAutonav(0))
      return;
   if ((player.autonav == AUTONAV_JUMP_APPROACH) ||
         (player.autonav == AUTONAV_JUMP_BRAKE)) {
      /* If we're already at the target. */
      if (player.p->nav_hyperspace == -1)
         player_autonavAbort("Target changed to current system");

      /* Need fuel. */
      else if (pplayer->fuel < HYPERSPACE_FUEL)
         player_autonavAbort("Not enough fuel for autonav to continue");

      else
         player_autonav();
   }

   /* Keep on moving. */
   else
      player_autonav();
}


/**
 * @brief Updates the player's autonav.
 *
 *    @param dt Current delta tick (should be real delta tick, not game delta tick).
 */
void player_updateAutonav( double dt )
{
   /* Must be autonaving. */
   if (!player_isFlag(PLAYER_AUTONAV) || (paused))
      return;

   /* Ramping down. */
   if (tc_rampdown) {
      if (tc_mod != 1.) {
         tc_mod = MAX( 1., tc_mod-tc_down*dt );
         pause_setSpeed( tc_mod );
      }
      return;
   }

   /* We'll update the time compression here. */
   if (tc_mod == tc_max)
      return;
   else
      tc_mod += 0.2 * dt * (tc_max-1.);
   /* Avoid going over. */ 
   if (tc_mod > tc_max)
      tc_mod = tc_max;
   pause_setSpeed( tc_mod );
}
