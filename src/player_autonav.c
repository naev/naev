/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player_autonav.c
 *
 * @brief Contains all the player autonav related stuff.
 */
/** @cond */
#include <time.h>

#include "naev.h"
/** @endcond */

#include "player.h"

#include "array.h"
#include "board.h"
#include "conf.h"
#include "map.h"
#include "pause.h"
#include "pilot.h"
#include "pilot_ew.h"
#include "player.h"
#include "sound.h"
#include "space.h"
#include "toolkit.h"

extern double player_acc; /**< Player acceleration. */

static double tc_mod    = 1.; /**< Time compression modifier. */
static double tc_base   = 1.; /**< Base compression modifier. */
static double tc_down   = 0.; /**< Rate of decrement. */
static int tc_rampdown  = 0; /**< Ramping down time compression? */
static double last_shield; /**< Player's last shield value. */
static double last_armour; /**< Player's last armour value. */
static int target_known = 0; /**< Is the target known? */

/*
 * Prototypes.
 */
static int player_autonavSetup (void);
static void player_autonav (void);
static int player_autonavApproach( const vec2 *pos, double *dist2, int count_target );
static void player_autonavFollow( const vec2 *pos, const vec2 *vel, const int follow, double *dist2 );
static int player_autonavApproachBoard( const vec2 *pos, const vec2 *vel, double *dist2, double sw );
static int player_autonavBrake (void);

/**
 * @brief Resets the game speed.
 */
void player_autonavResetSpeed (void)
{
   tc_mod = 1.;
   tc_rampdown = 0;
   player_resetSpeed();
}

/**
 * @brief Starts autonav.
 */
void player_autonavStart (void)
{
   StarSystem *dest;

   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   if ((player.p->nav_hyperspace == -1) && (player.p->nav_spob== -1))
      return;
   else if ((player.p->nav_spob != -1) && !player_getHypPreempt()) {
      player_autonavSpob( cur_system->spobs[ player.p->nav_spob ]->name, 0 );
      return;
   }

   if (player.p->fuel < player.p->fuel_consumption) {
      player_message(_("#rNot enough fuel to jump for autonav."));
      return;
   }

   if (pilot_isFlag( player.p, PILOT_NOJUMP)) {
      player_message(_("#rHyperspace drive is offline."));
      return;
   }

   if (player_autonavSetup())
      return;

   dest = map_getDestination(NULL);
   player_message(_("#oAutonav: travelling to %s."), (sys_isKnown(dest) ? _(dest->name) : _("Unknown")) );
   if (space_canHyperspace(player.p))
      player.autonav = AUTONAV_JUMP_BRAKE;
   else
      player.autonav = AUTONAV_JUMP_APPROACH;
}

/**
 * @brief Prepares the player to enter autonav.
 *
 *    @return 0 on success, -1 on failure (disabled, etc.)
 */
static int player_autonavSetup (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return -1;

   /* Autonav is mutually-exclusive with other autopilot methods. */
   player_restoreControl( PINPUT_AUTONAV, NULL );

   if (!player_isFlag(PLAYER_AUTONAV)) {
      tc_base   = player_dt_default() * player.speed;
      tc_mod    = tc_base;
      player.tc_max = conf.compression_velocity /
            solid_maxspeed(&player.p->solid, player.p->speed, player.p->thrust);
      if (conf.compression_mult >= 1.)
         player.tc_max = MIN( player.tc_max, conf.compression_mult );

      /* Safe cap. */
      player.tc_max = MAX( 1., player.tc_max );
   }

   /* Safe values. */
   free( player.autonavmsg );
   player.autonavmsg = NULL;
   tc_rampdown  = 0;
   tc_down      = 0.;
   last_shield  = player.p->shield / player.p->shield_max;
   last_armour  = player.p->armour / player.p->armour_max;

   /* Set flag and tc_mod just in case. */
   player_setFlag(PLAYER_AUTONAV);
   pause_setSpeed( tc_mod );
   sound_setSpeed( tc_mod );

   /* Make sure time acceleration starts immediately. */
   player.autonav_timer = 0.;

   return 0;
}

/**
 * @brief Ends the autonav.
 */
void player_autonavEnd (void)
{
   player_rmFlag(PLAYER_AUTONAV);
   player_autonavResetSpeed();
   free( player.autonavmsg );
   player.autonavmsg = NULL;
   /* Get rid of acceleration. */
   player_accelOver();
}

/**
 * @brief Starts autonav and closes the window.
 */
void player_autonavStartWindow( unsigned int wid, const char *str )
{
   (void) str;
   player_hyperspacePreempt( 1 );
   player_autonavStart();
   window_destroy( wid );
}

/**
 * @brief Starts autonav with a local position destination.
 */
void player_autonavPos( double x, double y )
{
   if (player_autonavSetup())
      return;

   player_message(_("#oAutonav: heading to target position."));
   player.autonav    = AUTONAV_POS_APPROACH;
   player.autonavmsg = strdup( _("position" ));
   player.autonavcol = '0';
   vec2_cset( &player.autonav_pos, x, y );
}

/**
 * @brief Starts autonav with a spob destination.
 */
void player_autonavSpob( const char *name, int tryland )
{
   Spob *p;
   vec2 pos;
   double a;

   if (player_autonavSetup())
      return;
   p = spob_get( name );
   player.autonavmsg = strdup( spob_name(p) );
   player.autonavcol = spob_getColourChar( p );
   pos = p->pos;

   /* Don't target center, but position offset in the direction of the player. */
   a = ANGLE( player.p->solid.pos.x - pos.x,
              player.p->solid.pos.y - pos.y );
   vec2_padd( &pos, 0.6 * p->radius, a );
   player.autonav_pos = pos;

   if (tryland) {
      player.autonav = AUTONAV_SPOB_LAND_APPROACH;
      player_message(_("#oAutonav: landing on #%c%s#0."), player.autonavcol, player.autonavmsg );
   }
   else {
      player.autonav = AUTONAV_SPOB_APPROACH;
      player_message(_("#oAutonav: approaching #%c%s#0."), player.autonavcol, player.autonavmsg );
   }
}

/**
 * @brief Starts autonav with a pilot to follow.
 */
void player_autonavPil( unsigned int p )
{
   Pilot *pilot = pilot_get( p );
   int inrange  = pilot_inRangePilot( player.p, pilot, NULL );
   if (player_autonavSetup() || !inrange)
      return;

   target_known = (inrange > 0);
   player_message(_("#oAutonav: following %s."), (target_known) ? pilot->name : _("Unknown") );
   player.autonav    = AUTONAV_PLT_FOLLOW;
   player.autonavmsg = strdup( pilot->name );
   player.autonavcol = '0';
}

/**
 * @brief Starts autonav with a pilot to board.
 */
void player_autonavBoard( unsigned int p )
{
   Pilot *pilot = pilot_get( p );
   int inrange  = pilot_inRangePilot( player.p, pilot, NULL );
   if (player_autonavSetup() || !inrange)
      return;

   /* Detected fuzzy, can't board. */
   if (inrange!=1) {
      player_autonavPil( p );
      return;
   }

   player_message(_("#oAutonav: boarding %s."), pilot->name );
   player.autonav    = AUTONAV_PLT_BOARD_APPROACH;
   player.autonavmsg = strdup( pilot->name );
   player.autonavcol = '0';
}

/**
 * @brief Handles common time accel ramp-down for autonav to positions and spobs.
 */
static void player_autonavRampdown( double d )
{
   double t, tint;
   double vel;

   vel   = MIN( 1.5*player.p->speed, VMOD(player.p->solid.vel) );
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

   /* Not under autonav. */
   if (!player_isFlag(PLAYER_AUTONAV))
      return;

   /* Cooldown (handled later) may be script-initiated and we don't
    * want to make it player-abortable while under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (reason != NULL)
         player_message(_("#rAutonav: aborted due to '%s'!"), reason);
      else
         player_message(_("#rAutonav: aborted!"));
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player.p);
         player_message(_("#oAutonav: aborting hyperspace sequence."));
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
   Pilot *p;
   int ret, map_npath, inrange;
   double d, t, tint;
   double vel;
   vec2 pos;

   (void) map_getDestination( &map_npath );

   switch (player.autonav) {
      case AUTONAV_JUMP_APPROACH:
         /* Target jump. */
         jp    = &cur_system->jumps[ player.p->nav_hyperspace ];
         /* Don't target center, but position offset in the direction of the player. */
         pos = jp->pos;
         t = ANGLE( player.p->solid.pos.x - jp->pos.x,
                    player.p->solid.pos.y - jp->pos.y );
         d = space_jumpDistance( player.p, jp );
         vec2_padd( &pos, MAX( 0.8*d, d-30. ), t );
         ret = player_autonavApproach( &pos, &d, 0 );
         if (ret)
            player.autonav = AUTONAV_JUMP_BRAKE;
         else if (!tc_rampdown && (map_npath<=1)) {
            vel   = MIN( 1.5*player.p->speed, VMOD(player.p->solid.vel) );
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
            player_acc = player.p->solid.thrust / player.p->thrust;
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
            player_message( _("#oAutonav: arrived at position.") );
            player_autonavEnd();
         }
         else if (!tc_rampdown)
            player_autonavRampdown(d);
         break;

      case AUTONAV_SPOB_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret) {
            player_message( _("#oAutonav: arrived at #%c%s#0."),
                  player.autonavcol,
                  player.autonavmsg );
            player_autonavEnd();
         }
         else if (!tc_rampdown)
            player_autonavRampdown(d);
         break;

      case AUTONAV_SPOB_LAND_APPROACH:
         ret = player_autonavApproach( &player.autonav_pos, &d, 1 );
         if (ret)
            player.autonav = AUTONAV_SPOB_LAND_BRAKE;
         else if (!tc_rampdown)
            player_autonavRampdown(d);
         break;

      case AUTONAV_SPOB_LAND_BRAKE:
         ret = player_autonavBrake();

         /* Try to land. */
         if (ret) {
            if (player_land(0) == PLAYER_LAND_DENIED)
               player_autonavAbort(NULL);
            else
               player.autonav = AUTONAV_SPOB_LAND_APPROACH;
         }

         /* See if should ramp down. */
         if (!tc_rampdown) {
            tc_rampdown = 1;
            tc_down     = (tc_mod-tc_base) / 3.;
         }
         break;

      case AUTONAV_PLT_FOLLOW:
         p = pilot_getTarget( player.p );

         if (p != NULL) {
            inrange = pilot_inRangePilot( player.p, p, NULL );
            target_known = (inrange > 0);
         }
         else {
            inrange = 0;
            target_known = 1; /* Suddenly disappeared but in range. */
         }

         if (!inrange) {
            /* TODO : handle the different reasons: pilot is too far, jumped, landed or died. */
            player_message( _("#oAutonav: following target %s has been lost."),
                              (target_known) ? player.autonavmsg : _("Unknown") );
            player_accel( 0. );
            player_autonavEnd();
            break;
         }

         ret = (pilot_isDisabled(p) || pilot_isFlag(p,PILOT_BOARDABLE));
         player_autonavFollow( &p->solid.pos, &p->solid.vel, !ret, &d );
         if (ret && (!tc_rampdown))
            player_autonavRampdown(d);
         break;

      case AUTONAV_PLT_BOARD_APPROACH:
         p = pilot_getTarget( player.p );

         if (p != NULL) {
            inrange = pilot_inRangePilot( player.p, p, NULL );
            target_known = (inrange > 0);
         }
         else {
            inrange = 0;
            target_known = 1; /* Suddenly disappeared but in range. */
         }

         if (!inrange) {
            /* TODO : handle the different reasons: pilot is too far, jumped, landed or died. */
            player_message( _("#oAutonav: boarding target %s has been lost."),
                              (target_known) ? player.autonavmsg : _("Unknown") );
            player_accel( 0. );
            player_autonavEnd();
            break;
         }

         ret = player_autonavApproachBoard( &p->solid.pos, &p->solid.vel, &d, p->ship->gfx_space->sw );
         if (!tc_rampdown)
            player_autonavRampdown(d);

         /* Try to board. */
         if (ret) {
            ret = player_tryBoard(0);
            if (ret == PLAYER_BOARD_OK)
               player_autonavEnd();
            else if (!(ret == PLAYER_BOARD_RETRY))
               player_autonavAbort(NULL); /* Not allowed to board, and it's not because of your position: abort. */
         }
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
static int player_autonavApproach( const vec2 *pos, double *dist2, int count_target )
{
   double d, t, vel, dist;

   /* Only accelerate if facing move dir. */
   d = pilot_face( player.p, vec2_angle( &player.p->solid.pos, pos ) );
   if (FABS(d) < MIN_DIR_ERR) {
      if (player_acc < 1.)
         player_accel( 1. );
   }
   else if (player_acc > 0.)
      player_accelOver();

   /* Get current time to reach target. */
   t  = MIN( 1.5*player.p->speed, VMOD(player.p->solid.vel) ) /
      (player.p->thrust / player.p->solid.mass);

   /* Get velocity. */
   vel   = MIN( player.p->speed, VMOD(player.p->solid.vel) );

   /* Get distance. */
   dist  = vel*(t+1.1*M_PI/player.p->turn) -
      0.5*(player.p->thrust/player.p->solid.mass)*t*t;

   /* Output distance^2 */
   d        = vec2_dist( pos, &player.p->solid.pos );
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
 * @brief Handles following a moving point with autonav (PD controller).
 *
 *    @param[in] pos Position to go to.
 *    @param[in] vel Velocity of the target.
 *    @param[in] follow Whether to follow, or arrive at
 *    @param[out] dist2 Distance left to target.
 */
static void player_autonavFollow( const vec2 *pos, const vec2 *vel, const int follow, double *dist2 )
{
   double angle, radius, d, timeFactor;
   vec2 dir, point;

   /* timeFactor is a time constant of the ship, used to heuristically determine the ratio Kd/Kp. */
   timeFactor = M_PI/player.p->turn + player.p->speed/player.p->thrust*player.p->solid.mass;

   /* Define the control coefficients.
      Maybe radius could be adjustable by the player. */
   const double Kp = 10.;
   const double Kd = MAX( 5., 10.84*timeFactor-10.82 );
   radius = 100.;

   /* Find a point behind the target at a distance of radius unless stationary, or not following. */
   if (!follow || (vel->x == 0 && vel->y == 0))
      radius = 0.;
   angle = M_PI + vel->angle;
   vec2_cset( &point, pos->x + radius * cos(angle),
              pos->y + radius * sin(angle) );

   vec2_cset( &dir, (point.x - player.p->solid.pos.x) * Kp +
         (vel->x - player.p->solid.vel.x) *Kd,
         (point.y - player.p->solid.pos.y) * Kp +
         (vel->y - player.p->solid.vel.y) *Kd );

   d = pilot_face( player.p, VANGLE(dir) );

   if ((FABS(d) < MIN_DIR_ERR) && (VMOD(dir) > 300.))
      player_accel( 1. );
   else
      player_accel( 0. );

   /* If aiming exactly at the point, should say when approaching. */
   if (!follow)
      *dist2 = vec2_dist( pos, &player.p->solid.pos );
}

static int player_autonavApproachBoard( const vec2 *pos, const vec2 *vel, double *dist2, double sw )
{
   double d, timeFactor;
   vec2 dir;

   /* timeFactor is a time constant of the ship, used to heuristically determine the ratio Kd/Kp. */
   timeFactor = M_PI/player.p->turn + player.p->speed/player.p->thrust*player.p->solid.mass;

   /* Define the control coefficients. */
   const double Kp = 10.;
   const double Kd = MAX( 5., 10.84*timeFactor-10.82 );

   vec2_cset( &dir, (pos->x - player.p->solid.pos.x) * Kp +
         (vel->x - player.p->solid.vel.x) *Kd,
         (pos->y - player.p->solid.pos.y) * Kp +
         (vel->y - player.p->solid.vel.y) *Kd );

   d = pilot_face( player.p, VANGLE(dir) );

   if ((FABS(d) < MIN_DIR_ERR) && (VMOD(dir) > 300.))
      player_accel( 1. );
   else
      player_accel( 0. );

   /* Distance for TC-rampdown. */
   *dist2 = vec2_dist( pos, &player.p->solid.pos );

   /* Check if velocity and position allow to board. */
   if (*dist2 > sw * PILOT_SIZE_APPROX)
      return 0;
   if (vec2_dist2( &player.p->solid.vel, vel ) > pow2(MAX_HYPERSPACE_VEL))
      return 0;
   return 1;
}

/**
 * @brief Handles the autonav braking.
 *
 *    @return 1 on completion.
 */
static int player_autonavBrake (void)
{
   int ret;
   if ((player.autonav == AUTONAV_JUMP_BRAKE) && (player.p->nav_hyperspace != -1)) {
      vec2 pos;
      JumpPoint *jp  = &cur_system->jumps[ player.p->nav_hyperspace ];

      pilot_brakeDist( player.p, &pos );
      if (vec2_dist2( &pos, &jp->pos ) > pow2(jp->radius))
         ret = pilot_interceptPos( player.p, jp->pos.x, jp->pos.y );
      else
         ret = pilot_brake( player.p );
   }
   else
      ret = pilot_brake(player.p);

   player_acc = player.p->solid.thrust / player.p->thrust;

   return ret;
}

/**
 * @brief Checks whether the speed should be reset due to damage or missile locks.
 *
 *    @return 1 if the speed should be reset.
 */
int player_autonavShouldResetSpeed (void)
{
   double shield, armour;
   int will_reset, lowhealth;
   double reset_dist, reset_shield;

   if (!player_isFlag(PLAYER_AUTONAV))
      return 0;

   /* Stop on lockons. */
   if (player.p->lockons > 0) {
      player.autonav_timer = MAX( player.autonav_timer, 0. );
      player_autonavResetSpeed();
      return 1;
   }

   /* Helper variables. */
   reset_dist     = conf.autonav_reset_dist;
   reset_shield   = conf.autonav_reset_shield;
   will_reset     = 0;
   /* Current ship health. */
   shield = player.p->shield / player.p->shield_max;
   armour = player.p->armour / player.p->armour_max;
   lowhealth = ((shield < last_shield && shield < reset_shield) || armour < last_armour);

   /* Only care when low health or stopping on distance. */
   if ((reset_dist > 0.) || lowhealth) {
      double rdist2     = pow2(reset_dist);
      Pilot *const*pstk = pilot_getAll();
      for (int i=0; i<array_size(pstk); i++) {
         double d2;
         Pilot *p = pstk[i];

         if (p->id == PLAYER_ID)
            continue;

         if (pilot_isDisabled(p))
            continue;

         if (!pilot_isHostile( p ))
            continue;

         if (pilot_isFlag(p,PILOT_STEALTH))
            continue;

         if (!pilot_canTarget( p ))
            continue;

         if (pilot_inRangePilot( player.p, pstk[i], &d2 )!=1)
            continue;

         if (lowhealth) {
            will_reset = 1;
            player.autonav_timer = MAX( player.autonav_timer, 2. );
            break;
         }
         if (reset_dist > 0.) {
            if (d2 < rdist2) {
               will_reset = 1;
               player.autonav_timer = MAX( player.autonav_timer, 0. );
               break;
            }
         }
         else
            break;
      }
   }

   /* Remember last helath status. */
   last_shield = shield;
   last_armour = armour;

   if (will_reset || (player.autonav_timer > 0.)) {
      player_autonavResetSpeed();
      return 1;
   }
   return 0;
}

/**
 * @brief Handles autonav thinking.
 *
 *    @param pplayer Player doing the thinking.
 *    @param dt Current delta tick.
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

   if (paused || (player.p==NULL) ||
         pilot_isFlag(player.p, PILOT_DEAD) ||
         player_isFlag( PLAYER_CINEMATICS ))
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
      sound_setSpeed( tc_mod / player_dt_default() );
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
         sound_setSpeed( tc_mod / player_dt_default() );
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
   sound_setSpeed( tc_mod / player_dt_default() );
}
