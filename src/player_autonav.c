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
#include "player.h"
#include "pilot.h"
#include "pilot_ew.h"
#include "space.h"
#include "sound.h"
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

/*
 * Prototypes.
 */
static int player_autonavSetup (void);
static void player_autonav (void);
static int player_autonavApproach( const Vector2d *pos, double *dist2, int count_target );
static int player_autonavBrake (void);


/**
 * @brief Resets the game speed.
 */
void player_autonavResetSpeed (void)
{
   if (player_isFlag(PLAYER_DOUBLESPEED)) {
      tc_mod         = 2. * player.p->ship->dt_default;
      pause_setSpeed( tc_mod );
      sound_setSpeed( 2 );
   } else {
      tc_mod         = player.p->ship->dt_default;
      pause_setSpeed( tc_mod );
      sound_setSpeed( 1 );
   }
   tc_rampdown = 0;
}


/**
 * @brief Starts autonav.
 */
void player_autonavStart (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   if ((player.p->nav_hyperspace == -1) && (player.p->nav_planet== -1))
      return;
   else if ((player.p->nav_planet != -1) && !player_getHypPreempt()) {
      player_autonavPnt( cur_system->planets[ player.p->nav_planet ]->name );
      return;
   }

   if (player.p->fuel < player.p->fuel_consumption) {
      player_message(_("\arNot enough fuel to jump for autonav."));
      return;
   }

   if (pilot_isFlag( player.p, PILOT_NOJUMP)) {
      player_message(_("\arHyperspace drive is offline."));
      return;
   }

   if (!player_autonavSetup())
      return;

   player.autonav = AUTONAV_JUMP_APPROACH;
}


/**
 * @brief Prepares the player to enter autonav.
 *
 *    @return 1 on success, 0 on failure (disabled, etc.)
 */
static int player_autonavSetup (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return 0;

   /* Autonav is mutually-exclusive with other autopilot methods. */
   player_restoreControl( PINPUT_AUTONAV, NULL );

   player_message(_("\apAutonav initialized."));
   if (!player_isFlag(PLAYER_AUTONAV)) {

      tc_base   = player.p->ship->dt_default * (player_isFlag(PLAYER_DOUBLESPEED) ? 2. : 1.);
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

   /* Set flag and tc_mod just in case. */
   player_setFlag(PLAYER_AUTONAV);
   pause_setSpeed( tc_mod );
   sound_setSpeed( tc_mod / player.p->ship->dt_default );

   /* Make sure time acceleration starts immediately. */
   player.autonav_timer = 0.;

   return 1;
}


/**
 * @brief Ends the autonav.
 */
void player_autonavEnd (void)
{
   player_rmFlag(PLAYER_AUTONAV);
   player_autonavResetSpeed();
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
   if (!player_autonavSetup())
      return;

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
   if (!player_autonavSetup())
      return;

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
   if ((player.p==NULL) || pilot_isFlag(player.p, PILOT_HYPERSPACE))
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
   if ((player.p==NULL) || pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;

   /* Cooldown (handled later) may be script-initiated and we don't
    * want to make it player-abortable while under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (reason != NULL)
         player_message(_("\arAutonav aborted: %s!"), reason);
      else
         player_message(_("\arAutonav aborted!"));
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player.p);
         player_message(_("\apAborting hyperspace sequence."));
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
         jp    = &cur_system->jumps[ player.p->nav_hyperspace ];
         if (player.p->stats.misc_instant_jump) {
            ret = pilot_interceptPos( player.p, jp->pos.x, jp->pos.y );
            if (!ret && space_canHyperspace(player.p))
               ret = 1;
            player_acc = player.p->solid->thrust / player.p->thrust;
         }
         else
            ret = player_autonavBrake();

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
            player_message( _("\apAutonav arrived at position.") );
            player_autonavEnd();
         }
         else if (!tc_rampdown)
            player_autonavRampdown(d);
         break;
      case AUTONAV_PNT_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret) {
            player_message( _("\apAutonav arrived at \a%c%s\a\0."),
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
 *    @param[in] pos Position to go to.
 *    @param[out] dist2 Square distance left to target.
 *    @param count_target If 1 it subtracts the braking distance from dist2. Otherwise it returns the full distance.
 *    @return 1 on completion.
 */
static int player_autonavApproach( const Vector2d *pos, double *dist2, int count_target )
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
   JumpPoint *jp;
   Vector2d pos;

   if ((player.autonav == AUTONAV_JUMP_BRAKE) && (player.p->nav_hyperspace != -1)) {
      jp  = &cur_system->jumps[ player.p->nav_hyperspace ];

      pilot_brakeDist( player.p, &pos );
      if (vect_dist2( &pos, &jp->pos ) > pow2(jp->radius))
         ret = pilot_interceptPos( player.p, jp->pos.x, jp->pos.y );
      else
         ret = pilot_brake( player.p );
   }
   else
      ret = pilot_brake(player.p);

   player_acc = player.p->solid->thrust / player.p->thrust;

   return ret;
}

/**
 * @brief Checks whether the speed should be reset due to damage or missile locks.
 *
 *    @return 1 if the speed should be reset.
 */
int player_autonavShouldResetSpeed (void)
{
   double failpc, shield, armour;
   int i, n;
   Pilot **pstk;
   int hostiles, will_reset;

   if (!player_isFlag(PLAYER_AUTONAV))
      return 0;

   hostiles   = 0;
   will_reset = 0;

   failpc = conf.autonav_reset_speed;
   shield = player.p->shield / player.p->shield_max;
   armour = player.p->armour / player.p->armour_max;

   pstk = pilot_getAll( &n );
   for (i=0; i<n; i++) {
      if ((pstk[i]->id != PLAYER_ID) && pilot_isHostile( pstk[i] ) &&
            pilot_inRangePilot( player.p, pstk[i] ) &&
            !pilot_isDisabled( pstk[i] ) &&
            !pilot_isFlag( pstk[i], PILOT_BRIBED )) {
         hostiles = 1;
         break;
      }
   }

   if (hostiles) {
      if (failpc > .995) {
         will_reset = 1;
         player.autonav_timer = MAX( player.autonav_timer, 0. );
      }
      else if ((shield < lasts && shield < failpc) || armour < lasta) {
         will_reset = 1;
         player.autonav_timer = MAX( player.autonav_timer, 2. );
      }
   }

   lasts = shield;
   lasta = armour;

   if (will_reset || (player.autonav_timer > 0)) {
      player_autonavResetSpeed();
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

   player_autonavShouldResetSpeed();
   if ((player.autonav == AUTONAV_JUMP_APPROACH) ||
         (player.autonav == AUTONAV_JUMP_BRAKE)) {
      /* If we're already at the target. */
      if (player.p->nav_hyperspace == -1)
         player_autonavAbort(_("Target changed to current system"));

      /* Need fuel. */
      else if (pplayer->fuel < pplayer->fuel_consumption)
         player_autonavAbort(_("Not enough fuel for autonav to continue"));

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

   if (paused || (player.p==NULL) || pilot_isFlag(player.p, PILOT_DEAD))
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
      sound_setSpeed( tc_mod / player.p->ship->dt_default );
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
         sound_setSpeed( tc_mod / player.p->ship->dt_default );
      }
      return;
   }

   /* We'll update the time compression here. */
   if (tc_mod == player.tc_max)
      return;
   else
      tc_mod += 0.2 * dt * (player.tc_max-tc_base);
   /* Avoid going over. */
   if (tc_mod > player.tc_max)
      tc_mod = player.tc_max;
   pause_setSpeed( tc_mod );
   sound_setSpeed( tc_mod / player.p->ship->dt_default );
}


