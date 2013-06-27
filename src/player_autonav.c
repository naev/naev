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
#include <time.h>


extern double player_acc; /**< Player acceleration. */
extern int map_npath; /**< @todo remove */

static double tc_mod    = 1.; /**< Time compression modifier. */
static double tc_base   = 1.; /**< Base compression modifier. */
static double tc_down   = 0.; /**< Rate of decrement. */
static int tc_rampdown  = 0; /**< Ramping down time compression? */
static double lasts;
static double lasta;
static int slockons;
static double abort_mod = 1.;
static double autopause_timer = 0.; /**< Avoid autopause if the player just unpaused, and don't compress time right away */

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

   if ((player.p->nav_hyperspace == -1) && (player.p->nav_planet== -1))
      return;
   else if ((player.p->nav_planet != -1) && !player_getHypPreempt()) {
      player_autonavPnt( cur_system->planets[ player.p->nav_planet ]->name );
      return;
   }

   if (player.p->fuel < player.p->fuel_consumption) {
      player_message("\erNot enough fuel to jump for autonav.");
      return;
   }

   if (pilot_isFlag( player.p, PILOT_NOJUMP)) {
      player_message("\erHyperspace drive is offline.");
      return;
   }

   /* Cooldown and autonav are mutually-exclusive. */
   if ((pilot_isFlag(player.p, PILOT_COOLDOWN)) ||
         (pilot_isFlag(player.p, PILOT_COOLDOWN_BRAKE)))
      pilot_cooldownEnd(player.p, NULL);

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

      tc_base   = player_isFlag(PLAYER_DOUBLESPEED) ? 2. : 1.;
      tc_mod    = tc_base;
      if (conf.compression_mult >= 1.)
         player.tc_max = MIN( conf.compression_velocity / solid_maxspeed(player.p->solid, player.p->speed, player.p->thrust), conf.compression_mult );
      else
         player.tc_max = conf.compression_velocity /
               solid_maxspeed(player.p->solid, player.p->speed, player.p->thrust);

      /* Safe cap. */
      player.tc_max = MAX( 1., player.tc_max );
   }

   /* Sane values. */
   tc_rampdown  = 0;
   tc_down      = 0.;
   lasts        = player.p->shield / player.p->shield_max;
   lasta        = player.p->armour / player.p->armour_max;
   slockons     = player.p->lockons;
   if (player.autonav_timer <= 0.)
      abort_mod = 1.;

   /* Set flag and tc_mod just in case. */
   player_setFlag(PLAYER_AUTONAV);
   pause_setSpeed( tc_mod );
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
   player.autonavmsg = "position";
   vect_cset( &player.autonav_pos, x, y );
}


/**
 * @brief Starts autonav with a planet destination.
 */
void player_autonavPnt( char *name )
{
   Planet *p;

   p = planet_get( name );
   player_autonavSetup();
   player.autonav    = AUTONAV_PNT_APPROACH;
   player.autonavmsg = p->name;
   vect_cset( &player.autonav_pos, p->pos.x, p->pos.y );
}


/**
 * @brief Handles common time accel ramp-down for autonav to positions and planets.
 */
static void player_autonavRampdown( double d )
{
   double t, tint;
   double vel;

   vel   = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) );
   t     = d / vel * (1. - 0.075 * tc_base);
   tint  = 3. + 0.5*(3.*(tc_mod-tc_base));
   if (t < tint) {
      tc_rampdown = 1;
      tc_down     = (tc_mod-tc_base) / 3.;
   }
}


/**
 * @brief Aborts regular interstellar autonav, but not in-system autonav.
 *
 *    @param reason Human-readable string describing abort condition.
 */
void player_autonavAbortJump( const char *reason )
{
   /* No point if player is beyond aborting. */
   if ((player.p==NULL) || ((player.p != NULL) && pilot_isFlag(player.p, PILOT_HYPERSPACE)))
      return;

   if (!player_isFlag(PLAYER_AUTONAV) || ((player.autonav != AUTONAV_JUMP_APPROACH) &&
         (player.autonav != AUTONAV_JUMP_BRAKE)))
      return;

   /* It's definitely not in-system autonav. */
   player_autonavAbort(reason);
}


/**
 * @brief Aborts autonav.
 *
 *    @param reason Human-readable string describing abort condition.
 */
void player_autonavAbort( const char *reason )
{
   /* No point if player is beyond aborting. */
   if ((player.p==NULL) || ((player.p != NULL) && pilot_isFlag(player.p, PILOT_HYPERSPACE)))
      return;

   /* Cooldown (handled later) may be script-initiated and we don't
    * want to make it player-abortable while under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (conf.autonav_pause && reason) {
         /* Keep it from re-pausing before you can react */
		 if (autopause_timer > 0) return;
         player_message("\erGame paused: %s!", reason);

         if (player_isFlag(PLAYER_DOUBLESPEED)) {
           tc_mod         = 2.;
           pause_setSpeed( 2. );
         } else {
           tc_mod         = 1.;
           pause_setSpeed( 1. );
         }

         autopause_timer = 2.;
         pause_game();
		 return;
      }
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
   else if (pilot_isFlag(player.p, PILOT_COOLDOWN_BRAKE))
      pilot_cooldownEnd(player.p, NULL);
   else if (pilot_isFlag(player.p, PILOT_COOLDOWN))
      pilot_cooldownEnd(player.p, reason);
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
            t     = d / vel * (1.2 - .1 * tc_base);
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
            tint  = 3. + 0.5*(3.*(tc_mod-tc_base));
            if (t < tint) {
               tc_rampdown = 1;
               tc_down     = (tc_mod-tc_base) / 3.;
            }
         }
         break;

      case AUTONAV_JUMP_BRAKE:
         /* Target jump. */
         ret   = (player.p->stats.misc_instant_jump ? 1 : player_autonavBrake());
         /* Try to jump or see if braked. */
         if (ret) {
            if (space_canHyperspace(player.p))
               player_jump();
            player.autonav = AUTONAV_JUMP_APPROACH;
         }

         /* See if should ramp down. */
         if (!tc_rampdown && (map_npath<=1)) {
            tc_rampdown = 1;
            tc_down     = (tc_mod-tc_base) / 3.;
         }
         break;

      case AUTONAV_POS_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret) {
            player_message( "\epAutonav arrived at position." );
            player_autonavEnd();
         }
         else if (!tc_rampdown)
            player_autonavRampdown(d);
         break;
      case AUTONAV_PNT_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret) {
            player_message( "\epAutonav arrived at \e%c%s\e\0.",
                  planet_getColourChar( planet_get(player.autonavmsg) ),
                  player.autonavmsg );
            player_autonavEnd();
         }
         else if (!tc_rampdown)
            player_autonavRampdown(d);
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
   double d, t, vel, dist;

   /* Only accelerate if facing move dir. */
   d = pilot_face( player.p, vect_angle( &player.p->solid->pos, pos ) );
   if (FABS(d) < MIN_DIR_ERR) {
      if (player_acc < 1.)
         player_accel( 1. );
   }
   else if (player_acc > 0.)
      player_accelOver();

   /* Get current time to reach target. */
   t  = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) ) /
      (player.p->thrust / player.p->solid->mass);

   /* Get velocity. */
   vel   = MIN( player.p->speed, VMOD(player.p->solid->vel) );

   /* Get distance. */
   dist  = vel*(t+1.1*M_PI/player.p->turn) -
      0.5*(player.p->thrust/player.p->solid->mass)*t*t;

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
   int ret;

   ret = pilot_brake(player.p);
   player_acc = player.p->solid->thrust / player.p->thrust;

   return ret;
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
   char *reason = NULL;

   if (!player_isFlag(PLAYER_AUTONAV))
      return 0;

   if (failpc >= 1. && !slockons && player.p->lockons > 0)
      reason = "Missile Lockon Detected";
   else if (failpc >= 1. && (shield < 1. && shield < lasts) && damaged)
      reason = "Sustaining damage";
   else if (failpc > 0. && (shield < failpc && shield < lasts) && damaged)
      reason = "Shield below damage threshold";
   else if (armour < lasta && damaged)
      reason = "Sustaining armour damage";

   lasts = player.p->shield / player.p->shield_max;
   lasta = player.p->armour / player.p->armour_max;

   if (reason) {
      player_autonavAbort(reason);
      if (player.autonav_timer > 0.)
         abort_mod = MIN( MAX( 0., abort_mod - .25 ), (int)(shield * 4) * .25 );
      else
         abort_mod = MIN( 0.75, (int)(shield * 4) * .25 );
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
      else if (pplayer->fuel < pplayer->fuel_consumption)
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
   const double dis_dead = 5.0;
   const double dis_mod  = 0.5;
   const double dis_max  = 4.0;
   const double dis_ramp = 6.0;

   if (paused || (player.p==NULL))
      return;

   /* We handle disabling here. */
   if (pilot_isFlag(player.p, PILOT_DISABLED)) {
      /* It is somewhat like:
       *        /------------\        4x
       *       /              \
       * -----/                \----- 1x
       *
       * <---><-><----------><-><--->
       *   5   6     X        6   5    Real time
       *   5   15    X        15  5    Game time
       *
       * For triangles we have to add the rectangle and triangle areas.
       */
      /* 5 second deadtime. */
      if (player.p->dtimer_accum < dis_dead)
         tc_mod = tc_base;
      else {
         /* Ramp down. */
         if (player.p->dtimer - player.p->dtimer_accum < dis_dead + (dis_max-tc_base)*dis_ramp/2 + tc_base*dis_ramp)
            tc_mod = MAX( tc_base, tc_mod - dis_mod*dt );
         /* Normal. */
         else
            tc_mod = MIN( dis_max, tc_mod + dis_mod*dt );
      }
      pause_setSpeed( tc_mod );
      return;
   }

   /* Must be autonaving. */
   if (!player_isFlag(PLAYER_AUTONAV))
      return;

   /* Ramping down. */
   if (tc_rampdown) {
      if (tc_mod != tc_base) {
         tc_mod = MAX( tc_base, tc_mod-tc_down*dt );
         pause_setSpeed( tc_mod );
      }
      return;
   }

   /* We'll update the time compression here. */
   if (autopause_timer > 0) {
      /* Don't start time acceleration right away.  Let the player react. */
      autopause_timer -= dt;
	  return;
   }
   if (tc_mod == player.tc_max)
      return;
   else
      tc_mod += 0.2 * dt * (player.tc_max-tc_base);
   /* Avoid going over. */
   if (tc_mod > player.tc_max)
      tc_mod = player.tc_max;
   pause_setSpeed( tc_mod );
}


