/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file weapon.c
 *
 * @brief Handles all the weapons in game.
 *
 * Weapons are what gets created when a pilot shoots.  They are based
 * on the outfit that created them.
 */


#include "weapon.h"

#include "naev.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "collision.h"
#include "spfx.h"
#include "opengl.h"
#include "explosion.h"
#include "gui.h"


#define weapon_isSmart(w)     (w->think != NULL) /**< Checks if the weapon w is smart. */

#define WEAPON_CHUNK          128 /**< Size to increase array with */

/* Weapon status */
#define WEAPON_STATUS_OK         0 /**< Weapon is fine */
#define WEAPON_STATUS_JAMMED     1 /**< Got jammed */
#define WEAPON_STATUS_UNJAMMED   2 /**< Survived jamming */


/*
 * pilot stuff
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;
/*
 * ai stuff
 */
extern void ai_attacked( Pilot* attacked, const unsigned int attacker ); /**< Triggers the "attacked" function in the ai */


/*
 * Weapon stuff.
 */
static int weapon_safety = 1; /**< Enables shooting friendlies. */


/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   Solid *solid; /**< Actually has its own solid :) */
   int ID; /**< Only used for beam weapons. */

   int faction; /**< faction of pilot that shot it */
   unsigned int parent; /**< pilot that shot it */
   unsigned int target; /**< target to hit, only used by seeking things */
   const Outfit* outfit; /**< related outfit that fired it or whatnot */

   int voice; /**< Weapon's voice. */
   double lockon; /**< some weapons have a lockon delay */
   double timer; /**< mainly used to see when the weapon was fired */
   double anim; /**< Used for beam weapon graphics and others. */
   int sprite; /**< Used for spinning outfits. */
   int mount; /**< Used for beam weapons. */

   /* position update and render */
   void (*update)(struct Weapon_*, const double, WeaponLayer); /**< Updates the weapon */
   void (*think)(struct Weapon_*, const double); /**< for the smart missiles */

   char status; /**< Weapon status - to check for jamming */
} Weapon;


/* behind pilot_nstack layer */
static Weapon** wbackLayer = NULL; /**< behind pilots */
static int nwbackLayer = 0; /**< number of elements */
static int mwbacklayer = 0; /**< alloced memory size */
/* behind player layer */
static Weapon** wfrontLayer = NULL; /**< infront of pilots, behind player */
static int nwfrontLayer = 0; /**< number of elements */
static int mwfrontLayer = 0; /**< alloced memory size */


/* Internal stuff. */
static int beam_idgen = 0; /**< Beam identifier generator. */


/*
 * Prototypes
 */
/* static */
static Weapon* weapon_create( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int parent, const unsigned int target );
static void weapon_render( Weapon* w, const double dt );
static void weapons_updateLayer( const double dt, const WeaponLayer layer );
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer );
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos );
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      Vector2d pos[2], const double dt );
static void weapon_destroy( Weapon* w, WeaponLayer layer );
static void weapon_free( Weapon* w );
static void weapon_explodeLayer( WeaponLayer layer,
      double x, double y, double radius,
      unsigned int parent, int mode );
static int weapon_checkCanHit( Weapon* w, Pilot *p );
/* think */
static void think_seeker( Weapon* w, const double dt );
static void think_beam( Weapon* w, const double dt );
/* externed */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape, double alpha );


#define PIXEL(x,y)      \
   if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
         (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y)) <= rc)))   \
   glVertex2i((x),(y)) /**< Sets a pixel if within range. */
/**
 * @brief Draws the minimap weapons (used in player.c).
 *
 *    @param res Minimap resolution.
 *    @param w Width of minimap.
 *    @param h Height of minimap.
 *    @param shape Shape of the minimap.
 *    @param alpha Alpha to draw points at.
 */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape, double alpha )
{
   int i, rc;
   double x, y;
   Weapon *wp;

   /* Begin the points. */
   glBegin(GL_POINTS);

   if (shape==RADAR_CIRCLE)
      rc = (int)(w*w);

   /* Draw the points for weapons on all layers. */
   for (i=0; i<nwbackLayer; i++) {
      wp = wbackLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player, wp->solid->pos.x, wp->solid->pos.y ))
         continue;

      /* Choose colour based on if it'll hit player. */
      if (outfit_isSeeker(wp->outfit) && (wp->target != PLAYER_ID))
         ACOLOUR(cNeutral, alpha);
      else if ((wp->target == PLAYER_ID) || !areAllies(FACTION_PLAYER, wp->faction))
         ACOLOUR(cHostile, alpha);
      else
         ACOLOUR(cNeutral, alpha);

      /* Put the pixel. */
      x = (wp->solid->pos.x - player->solid->pos.x) / res;
      y = (wp->solid->pos.y - player->solid->pos.y) / res;
      PIXEL(x,y);
   }
   for (i=0; i<nwfrontLayer; i++) {
      wp = wfrontLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player, wp->solid->pos.x, wp->solid->pos.y ))
         continue;

      /* Choose colour based on if it'll hit player. */
      if (outfit_isSeeker(wp->outfit) && (wp->target != PLAYER_ID))
         ACOLOUR(cNeutral, alpha);
      else if ((wp->target == PLAYER_ID) || !areAllies(FACTION_PLAYER, wp->faction))
         ACOLOUR(cHostile, alpha);
      else
         ACOLOUR(cNeutral, alpha);

      /* Put the pixel. */
      x = (wp->solid->pos.x - player->solid->pos.x) / res;
      y = (wp->solid->pos.y - player->solid->pos.y) / res;
      PIXEL(x,y);
   }

   /* End the points. */
   glEnd(); /* GL_POINTS */
}
#undef PIXEL


/**
 * @brief Toggles the player's weapon safety.
 */
void weapon_toggleSafety (void)
{
   weapon_safety = !weapon_safety;

   if (weapon_safety)
      player_message( "Enabling weapon safety." );
   else
      player_message( "Disabling weapon safety." );
}


/**
 * @brief The AI of seeker missiles.
 *
 *    @param w Weapon to do the thinking.
 *    @param dt Current delta tick.
 */
static void think_seeker( Weapon* w, const double dt )
{
   double diff;
   double vel;
   Pilot *p;
   int effect;
   int spfx;
            Vector2d v;
            double t;

   if (w->target == w->parent) return; /* no self shooting */

   p = pilot_get(w->target); /* no null pilot_nstack */
   if (p==NULL) {
      w->solid->dir_vel = 0.; /* go straight */
      vectnull( &w->solid->force ); /* no force */
      return;
   }

   /* Only run if locked on. */
   if (w->lockon < 0.) {

      switch (w->status) {
         case WEAPON_STATUS_OK: /* Check to see if can get jammed */
            if ((p->jam_range != 0.) &&  /* Target has jammer and weapon is in range */
                  (vect_dist(&w->solid->pos,&p->solid->pos) < p->jam_range)) {

               /* Check to see if weapon gets jammed */
               if (RNGF() < p->jam_chance - w->outfit->u.amm.resist) {
                  w->status = WEAPON_STATUS_JAMMED;
                  /* Give it a nice random effect */
                  effect = RNG(0,4);
                  switch (effect) {
                     case 0: /* Blow up */
                        w->timer = -1.;
                        spfx = outfit_spfxArmour(w->outfit);
                        spfx_add( spfx, w->solid->pos.x, w->solid->pos.y,
                              w->solid->vel.x, w->solid->vel.y,
                              SPFX_LAYER_BACK ); /* presume back. */
                        break;
                     case 1: /* Stuck in left loop */
                        w->solid->dir_vel = w->outfit->u.amm.turn;
                        break;
                     case 2: /* Stuck in right loop */
                        w->solid->dir_vel = -w->outfit->u.amm.turn;
                        break;

                     default: /* Go straight */
                        w->solid->dir_vel = 0.;
                        return;
                  }
               }
               else /* Can't get jammed anymore */
                  w->status = WEAPON_STATUS_UNJAMMED;
            }

         /* Purpose fallthrough */
         case WEAPON_STATUS_UNJAMMED: /* Work as expected */

            /* Smart seekers take into account ship velocity. */
            if (w->outfit->type == OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO) {

               /* Calculate time to reach target. */
               vect_cset( &v, p->solid->pos.x - w->solid->pos.x,
                     p->solid->pos.y - w->solid->pos.y );
               t = vect_odist( &v ) / w->outfit->u.amm.speed;

               /* Calculate target's movement. */
               vect_cset( &v, v.x + t*(p->solid->vel.x - w->solid->vel.x),
                     v.y + t*(p->solid->vel.y - w->solid->vel.y) );

               /* Get the angle now. */
               diff = angle_diff(w->solid->dir, VANGLE(v) );
            }
            /* Other seekers are stupid. */
            else {
               diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                     vect_angle(&w->solid->pos, &p->solid->pos));
            }

            w->solid->dir_vel = 10 * diff *  w->outfit->u.amm.turn; /* Face pos */
            /* Check for under/overflows */
            if (w->solid->dir_vel > w->outfit->u.amm.turn)
               w->solid->dir_vel = w->outfit->u.amm.turn;
            else if (w->solid->dir_vel < -w->outfit->u.amm.turn)
               w->solid->dir_vel = -w->outfit->u.amm.turn;
            break;

         case WEAPON_STATUS_JAMMED: /* Continue doing whatever */
            /* Do nothing, dir_vel should be set already if needed */
            break;

         default:
            WARN("Unknown weapon status for '%s'", w->outfit->name);
            break;
      }
   }

   /* Limit speed here */
   vel = MIN(w->outfit->u.amm.speed, VMOD(w->solid->vel) + w->outfit->u.amm.thrust*dt);
   vect_pset( &w->solid->vel, vel, w->solid->dir );
   /*limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );*/
}


/**
 * @brief The pseudo-ai of the beam weapons.
 *
 *    @param w Weapon to do the thinking.
 *    @param dt Current delta tick.
 */
static void think_beam( Weapon* w, const double dt )
{
   (void)dt;
   Pilot *p, *t;
   double diff;
   Vector2d v;

   /* Get pilot, if pilot is dead beam is destroyed. */
   p = pilot_get(w->parent);
   if (p==NULL) {
      w->timer = -1.; /* Hack to make it get destroyed next update. */
      return;
   }

   /* Check if pilot has enough energy left to keep beam active. */
   p->energy -= dt*w->outfit->u.bem.energy;
   if (p->energy < 0.) {
      p->energy = 0.;
      w->timer = -1;
      return;
   }

   /* Use mount position. */
   pilot_getMount( p, w->mount, &v );
   w->solid->pos.x = v.x;
   w->solid->pos.y = v.y;

   /* Handle aiming. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BEAM:
         w->solid->dir = p->solid->dir;
         break;

      case OUTFIT_TYPE_TURRET_BEAM:
         /* Get target, if target is dead beam stops moving. */
         t = pilot_get(w->target);
         if (t==NULL) {
            w->solid->dir_vel = 0.;
            return;
         }

         if (w->target == w->parent) /* Invalid target, tries to follow shooter. */
            diff = angle_diff(w->solid->dir, p->solid->dir);
         else
            diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                  vect_angle(&w->solid->pos, &t->solid->pos));
         w->solid->dir_vel = 10 * diff *  w->outfit->u.bem.turn; /* Face pos */
         /* Check for under/overflows */
         if (w->solid->dir_vel > w->outfit->u.bem.turn)
            w->solid->dir_vel = w->outfit->u.bem.turn;
         else if (w->solid->dir_vel < -w->outfit->u.bem.turn)
            w->solid->dir_vel = -w->outfit->u.bem.turn;
         break;

      default:
         return;
   }
}


/**
 * @brief Updates all the weapon layers.
 *
 *    @param dt Current delta tick.
 */
void weapons_update( const double dt )
{
   weapons_updateLayer(dt,WEAPON_LAYER_BG);
   weapons_updateLayer(dt,WEAPON_LAYER_FG);
}


/**
 * @brief Updates all the weapons in the layer.
 *
 *    @param dt Current delta tick.
 *    @param layer Layer to update.
 */
static void weapons_updateLayer( const double dt, const WeaponLayer layer )
{
   Weapon **wlayer;
   int *nlayer;
   Weapon *w;
   int i;

   /* Choose layer. */
   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         nlayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         nlayer = &nwfrontLayer;
         break;

      default:
         WARN("Unknown weapon layer!");
   }

   i = 0;
   while (i < *nlayer) {
      w = wlayer[i];
      switch (w->outfit->type) {

         /* most missiles behave the same */
         case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
         case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         case OUTFIT_TYPE_MISSILE_SWARM_AMMO:
         case OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO:
            if (w->lockon > 0.) /* decrement lockon */
               w->lockon -= dt;
            /* purpose fallthrough */

         /* bolts too */
         case OUTFIT_TYPE_TURRET_DUMB_AMMO:
         case OUTFIT_TYPE_MISSILE_DUMB_AMMO: /* Dumb missiles are like bolts */
            limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
         case OUTFIT_TYPE_BOLT:
         case OUTFIT_TYPE_TURRET_BOLT:
            w->timer -= dt;
            if (w->timer < 0.) {
               weapon_destroy(w,layer);
               break;
            }
            break;

         /* Beam weapons handled a part. */
         case OUTFIT_TYPE_BEAM:
         case OUTFIT_TYPE_TURRET_BEAM:
            w->timer -= dt;
            if (w->timer < 0.) {
               weapon_destroy(w,layer);
               break;
            }
            /* We use the lockon to tell when we have to create explosions. */
            w->lockon -= dt;
            if (w->lockon < 0.) {
               if (w->lockon < -1.)
                  w->lockon = 0.100;
               else
                  w->lockon = -1.;
            }
            break;
         default:
            WARN("Weapon of type '%s' has no update implemented yet!",
                  w->outfit->name);
            break;
      }

      /* Out of bounds, loop is over. */
      if (i >= *nlayer)
         break;

      /* Only increment if weapon wasn't deleted. */
      if (w == wlayer[i]) {
         weapon_update(w,dt,layer);
         if ((i < *nlayer) && (w == wlayer[i]))
            i++;
      }
   }
}


/**
 * @brief Renders all the weapons in a layer.
 *
 *    @param layer Layer to render.
 *    @param dt Current delta tick.
 */
void weapons_render( const WeaponLayer layer, const double dt )
{
   Weapon** wlayer;
   int* nlayer;
   int i;

   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         nlayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         nlayer = &nwfrontLayer;
         break;

      default:
         WARN("Unknown weapon layer!");
   }

   for (i=0; i<(*nlayer); i++)
      weapon_render( wlayer[i], dt );
}


/**
 * @brief Renders an individual weapon.
 *
 *    @param w Weapon to render.
 *    @param dt Current delta tick.
 */
static void weapon_render( Weapon* w, const double dt )
{
   int sx, sy;
   double x,y, cx,cy, gx,gy;
   glTexture *gfx;

   switch (w->outfit->type) {
      /* Weapons that use sprites. */
      case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
      case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
      case OUTFIT_TYPE_MISSILE_SWARM_AMMO:
      case OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO:
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
      case OUTFIT_TYPE_MISSILE_DUMB_AMMO:
      case OUTFIT_TYPE_TURRET_DUMB_AMMO:
         gfx = outfit_gfx(w->outfit);

         /* Outfit spins around. */
         if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_SPIN)) {
            /* Check timer. */
            w->anim -= dt;
            if (w->anim < 0.) {
               w->anim = outfit_spin(w->outfit);

               /* Increment sprite. */
               w->sprite++;
               if (w->sprite >= gfx->sx*gfx->sy)
                  w->sprite = 0;
            }

            /* Render. */
            gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y,
                  w->sprite % (int)gfx->sx, w->sprite / (int)gfx->sx, NULL );
         }
         /* Outfit faces direction. */
         else {
            gl_getSpriteFromDir( &sx, &sy, gfx, w->solid->dir );
            gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y, sx, sy, NULL );
         }
         break;

      /* Beam weapons. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         gfx = outfit_gfx(w->outfit);

         /* Position. */
         gl_cameraGet( &cx, &cy );
         gui_getOffset( &gx, &gy );
         x = w->solid->pos.x - cx + gx;
         y = w->solid->pos.y - cy + gy;

         /* Set up the matrix. */
         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
            glTranslated( x, y, 0. );
            glRotated( 270. + w->solid->dir / M_PI * 180., 0., 0., 1. );

         /* Preparatives. */
         glEnable(GL_TEXTURE_2D);
         glBindTexture( GL_TEXTURE_2D, gfx->texture);
         glShadeModel(GL_SMOOTH);

         /* Actual rendering. */
         glBegin(GL_QUAD_STRIP);

            /* Start faded. */
            ACOLOUR(cWhite, 0.);

            glTexCoord2d( w->anim, 0. );
            glVertex2d( -gfx->sh/2., 0. );

            glTexCoord2d( w->anim, 1. );
            glVertex2d( +gfx->sh/2., 0. );

            /* Full strength. */
            COLOUR(cWhite);

            glTexCoord2d( w->anim + 10. / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2., 10. );

            glTexCoord2d( w->anim + 10. / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2., 10. );

            glTexCoord2d( w->anim + 0.8*w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2., 0.8*w->outfit->u.bem.range );

            glTexCoord2d( w->anim + 0.8*w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2., 0.8*w->outfit->u.bem.range );

            /* Fades out. */
            ACOLOUR(cWhite, 0.);

            glTexCoord2d( w->anim + w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2., w->outfit->u.bem.range );

            glTexCoord2d( w->anim + w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2., w->outfit->u.bem.range );
         glEnd(); /* GL_QUAD_STRIP */

         /* Do the beam movement. */
         w->anim -= 5. * dt;
         if (w->anim <= -gfx->sw)
            w->anim += gfx->sw;

         /* Clean up. */
         glDisable(GL_TEXTURE_2D);
         glShadeModel(GL_FLAT);
         glPopMatrix(); /* GL_PROJECTION */
         gl_checkErr();
         break;

      default:
         WARN("Weapon of type '%s' has no render implemented yet!",
               w->outfit->name);
         break;
   }
}


/**
 * @brief Checks to see if the weapon can hit the pilot.
 *
 *    @param w Weapon to check if hits pilot.
 *    @param p Pilot to check if is hit by weapon.
 *    @return 1 if can be hit, 0 if can't.
 */
static int weapon_checkCanHit( Weapon* w, Pilot *p )
{
   Pilot *parent;

   /* Can never hit same faction. */
   if (p->faction == w->faction)
      return 0;

   /* Go "through" dead pilots. */
   if (pilot_isFlag(p, PILOT_DEAD))
      return 0;

   /* Player behaves differently. */
   if (w->faction == FACTION_PLAYER) {

      /* Always hit without safety. */
      if (!weapon_safety)
         return 1;

      /* Always hit target. */
      else if (w->target == p->id)
         return 1;

      /* Always hit hostiles. */
      else if (pilot_isFlag(p, PILOT_HOSTILE))
         return 1;

      /* Always hit unbribed enemies. */
      else if (!pilot_isFlag(p, PILOT_BRIBED) &&
            areEnemies(w->faction, p->faction))
        return 1;

      /* Miss rest - can be neutral/ally. */
      else
         return 0;
   }

   /* Let hostiles hit player. */
   if (p->faction == FACTION_PLAYER) {
      parent = pilot_get(w->parent);
      if ((parent != NULL) && pilot_isFlag(parent, PILOT_HOSTILE))
         return 1;
   }

   /* Hit non-allies. */
   if (areEnemies(w->faction, p->faction))
      return 1;

   return 0;
}


/**
 * @brief Updates an individual weapon.
 *
 *    @param w Weapon to update.
 *    @param dt Current delta tick.
 *    @param layer Layer to which the weapon belongs.
 */
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer )
{
   int i, wsx,wsy, psx,psy;
   glTexture *gfx;
   Vector2d crash[2];
   Pilot *p;

   /* Get the sprite direction to speed up calculations. */
   if (!outfit_isBeam(w->outfit)) {
      gfx = outfit_gfx(w->outfit);
      gl_getSpriteFromDir( &wsx, &wsy, gfx, w->solid->dir );
   }

   for (i=0; i<pilot_nstack; i++) {

      p = pilot_stack[i];

      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      /* Beam weapons have special collisions. */
      if (outfit_isBeam(w->outfit)) {
         /* Check for collision. */
         if (weapon_checkCanHit(w,p) &&
               CollideLineSprite( &w->solid->pos, w->solid->dir,
                     w->outfit->u.bem.range,
                     p->ship->gfx_space, psx, psy,
                     &p->solid->pos,
                     crash)) {
            weapon_hitBeam( w, p, layer, crash, dt );
            /* No return because beam can still think, it's not
             * destroyed like the other weapons.*/
         }
      }
      /* smart weapons only collide with their target */
      else if (weapon_isSmart(w)) {

         if ((pilot_stack[i]->id == w->target) &&
               weapon_checkCanHit(w,p) &&
               CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                     p->ship->gfx_space, psx, psy,
                     &p->solid->pos,
                     &crash[0] )) {
            weapon_hit( w, p, layer, &crash[0] );
            return; /* Weapon is destroyed. */
         }
      }
      /* dumb weapons hit anything not of the same faction */
      else {
         if (weapon_checkCanHit(w,p) &&
               CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                     p->ship->gfx_space, psx, psy,
                     &p->solid->pos,
                     &crash[0] )) {
            weapon_hit( w, p, layer, &crash[0] );
            return; /* Weapon is destroyed. */
         }
      }
   }

   /* smart weapons also get to think their next move */
   if (weapon_isSmart(w))
      (*w->think)(w,dt);

   /* Update the solid position. */
   (*w->solid->update)(w->solid, dt);

   /* Update the sound. */
   sound_updatePos(w->voice, w->solid->pos.x, w->solid->pos.y);
}


/**
 * @brief Informs the AI if needed that it's been hit.
 *
 *    @param p Pilot being hit.
 *    @param shooter Pilot that shot.
 *    @param dmg Damage done to p.
 */
static void weapon_hitAI( Pilot *p, Pilot *shooter, double dmg )
{
   /* Must be a valid shooter. */
   if (shooter == NULL)
      return;
     
   /* Player is handled differently. */
   if (shooter->faction == FACTION_PLAYER) {

      /* Increment damage done to by player. */
      p->player_damage += dmg / (p->shield_max + p->armour_max);

      /* If damage is over threshold, inform pilot or if is targetted. */
      if ((p->player_damage > PILOT_HOSTILE_THRESHOLD) ||
            (shooter->target==p->id)) {
         /* Inform attacked. */
         ai_attacked( p, shooter->id );

         /* Set as hostile. */
         pilot_setHostile(p);
         pilot_rmFlag( p, PILOT_BRIBED );
      }
   }
   /* Otherwise just inform of being attacked. */
   else
      ai_attacked( p, shooter->id );
}


/**
 * @brief Weapon hit the pilot.
 *
 *    @param w Weapon involved in the collision.
 *    @param p Pilot that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 */
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos )
{
   Pilot *parent;
   int spfx;
   double damage;
   DamageType dtype;
   WeaponLayer spfx_layer;

   /* Choose spfx. */
   if (p->shield > 0.)
      spfx = outfit_spfxShield(w->outfit);
   else
      spfx = outfit_spfxArmour(w->outfit);

   /* Get the layer. */
   spfx_layer = (p==player) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;

   /* Get general details. */
   parent = pilot_get(w->parent);
   damage = outfit_damage(w->outfit);
   dtype = outfit_damageType(w->outfit);

   /* Add sprite, layer depends on whether player shot or not. */
   spfx_add( spfx, pos->x, pos->y,
         VX(p->solid->vel), VY(p->solid->vel), spfx_layer );

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, w->solid, w->parent, dtype, damage );

   /* Inform AI that it's been hit. */
   weapon_hitAI( p, parent, damage );

   /* no need for the weapon particle anymore */
   weapon_destroy(w,layer);
}


/**
 * @brief Weapon hit the pilot.
 *
 *    @param w Weapon involved in the collision.
 *    @param p Pilot that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 *    @param dt Current delta tick.
 */
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      Vector2d pos[2], const double dt )
{
   (void) layer;
   Pilot *parent;
   int spfx;
   double damage;
   DamageType dtype;
   WeaponLayer spfx_layer;

   /* Choose spfx. */
   if (p->shield > 0.)
      spfx = outfit_spfxShield(w->outfit);
   else
      spfx = outfit_spfxArmour(w->outfit);

   /* Get the layer. */
   spfx_layer = (p==player) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;

   /* Get general details. */
   parent = pilot_get(w->parent);
   damage = outfit_damage(w->outfit) * dt;
   dtype  = outfit_damageType(w->outfit);

   /* Add sprite, layer depends on whether player shot or not. */
   if (w->lockon == -1.) {
      spfx_add( spfx, pos[0].x, pos[0].y,
            VX(p->solid->vel), VY(p->solid->vel), spfx_layer );
      spfx_add( spfx, pos[1].x, pos[1].y,
            VX(p->solid->vel), VY(p->solid->vel), spfx_layer );
         w->lockon = -2;
   }

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, w->solid, w->parent, dtype, damage );

   /* Inform AI that it's been hit. */
   weapon_hitAI( p, parent, damage );
}


/**
 * @brief Creates a new weapon.
 *
 *    @param outfit Outfit which spawned the weapon.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Shooter ID.
 *    @param target Target ID of the shooter.
 *    @return A pointer to the newly created weapon.
 */
static Weapon* weapon_create( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int parent, const unsigned int target )
{
   Vector2d v;
   double mass, rdir;
   Pilot *pilot_target;
   double x,y, t, dist;
   Weapon* w;

   /* Create basic features */
   w = malloc(sizeof(Weapon));
   memset(w, 0, sizeof(Weapon));
   w->faction = pilot_get(parent)->faction; /* non-changeable */
   w->parent = parent; /* non-changeable */
   w->target = target; /* non-changeable */
   w->outfit = outfit; /* non-changeable */
   w->update = weapon_update;
   w->status = WEAPON_STATUS_OK;

   switch (outfit->type) {

      /* Bolts treated together */
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         /* Only difference is the direction of fire */
         if ((outfit->type == OUTFIT_TYPE_TURRET_BOLT) && (w->parent!=w->target) &&
               (w->target != 0)) { /* Must have valid target */

            pilot_target = pilot_get(w->target);
            if (pilot_target == NULL)
               rdir = dir;

            else {
               /* Get the distance */
               dist = vect_dist( pos, &pilot_target->solid->pos );

               /* Aim. */
               if (dist > outfit->u.blt.range*1.2) {
                  x = pilot_target->solid->pos.x - pos->x;
                  y = pilot_target->solid->pos.y - pos->y;
               }
               else {
                  /* Try to predict where the enemy will be. */
                  /* Time for shots to reach that distance */
                  t = dist / w->outfit->u.blt.speed;

                  /* Position is calculated on where it should be */
                  x = (pilot_target->solid->pos.x + pilot_target->solid->vel.x*t)
                     - (pos->x + vel->x*t);
                  y = (pilot_target->solid->pos.y + pilot_target->solid->vel.y*t)
                     - (pos->y + vel->y*t);
               }

               /* Set angle to face. */
               rdir = ANGLE(x, y);
            }
         }
         else /* fire straight */
            rdir = dir;

         rdir += RNG_2SIGMA() * outfit->u.blt.accuracy/2. * 1./180.*M_PI;
         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;

         mass = 1; /* Lasers are presumed to have unitary mass */
         vectcpy( &v, vel );
         vect_cadd( &v, outfit->u.blt.speed*cos(rdir), outfit->u.blt.speed*sin(rdir));
         w->timer = outfit->u.blt.range/outfit->u.blt.speed;
         w->solid = solid_create( mass, rdir, pos, &v );
         w->voice = sound_playPos(w->outfit->u.blt.sound,
               w->solid->pos.x + w->solid->vel.x,
               w->solid->pos.y + w->solid->vel.y);
         break;

      /* Beam weapons are treated together. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         if ((outfit->type == OUTFIT_TYPE_TURRET_BEAM) && (w->parent!=w->target)) {
            pilot_target = pilot_get(target);
            rdir = (pilot_target == NULL) ? dir :
                  vect_angle(pos, &pilot_target->solid->pos);
         }
         else
            rdir = dir;
         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;
         mass = 1.; /**< Needs a mass. */
         w->solid = solid_create( mass, rdir, pos, NULL );
         w->think = think_beam;
         w->timer = outfit->u.bem.duration;
         w->voice = sound_playPos(w->outfit->u.bem.sound,
               w->solid->pos.x + vel->x,
               w->solid->pos.y + vel->y);
         break;

      /* Treat seekers together. */
      case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
      case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         mass = w->outfit->mass;

         rdir = dir;
         if (outfit->u.amm.accuracy != 0.) {
            rdir += RNG_2SIGMA() * outfit->u.amm.accuracy/2. * 1./180.*M_PI;
         }
         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;

         w->lockon = outfit->u.amm.lockon;
         w->timer = outfit->u.amm.duration;
         w->solid = solid_create( mass, dir, pos, vel );

         /* if they are seeking a pilot, increment lockon counter */
         pilot_target = pilot_get(target);
         if (pilot_target != NULL)
            pilot_target->lockons++;

         /* only diff is AI */
         w->think = think_seeker; /* AI is the same atm. */
         /*if (outfit->type == OUTFIT_TYPE_MISSILE_SEEK_AMMO)
            w->think = think_seeker;
         else if (outfit->type == OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO)
            w->think = think_smart;*/
         w->voice = sound_playPos(w->outfit->u.amm.sound,
               w->solid->pos.x + w->solid->vel.x,
               w->solid->pos.y + w->solid->vel.y);
         break;

      /* Dumb missiles and turrets. */
      case OUTFIT_TYPE_TURRET_DUMB_AMMO:
      case OUTFIT_TYPE_MISSILE_DUMB_AMMO:
         if (w->outfit->type == OUTFIT_TYPE_TURRET_DUMB_AMMO) {
            pilot_target = pilot_get(w->target);
            if (pilot_target == NULL)
               rdir = dir;

            else {
               /* Get the distance */
               dist = vect_dist( pos, &pilot_target->solid->pos );

               /* Aim. */
               /* Try to predict where the enemy will be. */
               /* Time for shots to reach that distance */
               t = dist / w->outfit->u.amm.speed;

               /* Position is calculated on where it should be */
               x = (pilot_target->solid->pos.x + pilot_target->solid->vel.x*t)
                  - (pos->x + vel->x*t);
               y = (pilot_target->solid->pos.y + pilot_target->solid->vel.y*t)
                  - (pos->y + vel->y*t);

               /* Set angle to face. */
               rdir = ANGLE(x, y);
            }
         }
         else {
            rdir = dir;
         }
         if (outfit->u.amm.accuracy != 0.) {
            rdir += RNG_2SIGMA() * outfit->u.amm.accuracy/2. * 1./180.*M_PI;
            if ((rdir > 2.*M_PI) || (rdir < 0.))
               rdir = fmod(rdir, 2.*M_PI);
         }
         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;

         /* If thrust is 0. we assume it starts out at speed. */
         vectcpy( &v, vel );
         if (outfit->u.amm.thrust == 0.)
            vect_cadd( &v, cos(rdir) * w->outfit->u.amm.speed,
                  sin(rdir) * w->outfit->u.amm.speed );

         mass = w->outfit->mass;
         w->timer = outfit->u.amm.duration;
         w->solid = solid_create( mass, rdir, pos, &v );
         if (w->outfit->u.amm.thrust != 0.)
            vect_pset( &w->solid->force, w->outfit->u.amm.thrust * mass, rdir );
         w->voice = sound_playPos(w->outfit->u.amm.sound,
               w->solid->pos.x + w->solid->vel.x,
               w->solid->pos.y + w->solid->vel.y);
         break;


      /* just dump it where the player is */
      default:
         WARN("Weapon of type '%s' has no create implemented yet!",
               w->outfit->name);
         w->solid = solid_create( mass, dir, pos, vel );
         break;
   }

   return w;
}


/**
 * @brief Creates a new weapon.
 *
 *    @param outfit Outfit which spawns the weapon.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Pilot ID of the shooter.
 *    @param target Target ID that is getting shot.
 */
void weapon_add( const Outfit* outfit, const double dir,
      const Vector2d* pos, const Vector2d* vel,
      unsigned int parent, unsigned int target )
{
   WeaponLayer layer;
   Weapon *w;
   Weapon **curLayer;
   int *mLayer, *nLayer;

   if (!outfit_isBolt(outfit) &&
         !outfit_isAmmo(outfit)) {
      ERR("Trying to create a Weapon from a non-Weapon type Outfit");
      return;
   }

   layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w = weapon_create( outfit, dir, pos, vel, parent, target );

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         nLayer = &nwbackLayer;
         mLayer = &mwbacklayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         nLayer = &nwfrontLayer;
         mLayer = &mwfrontLayer;
         break;

      default:
         WARN("Unknown weapon layer!");
   }

   if (*mLayer > *nLayer) /* more memory alloced then needed */
      curLayer[(*nLayer)++] = w;
   else { /* need to allocate more memory */
      switch (layer) {
         case WEAPON_LAYER_BG:
            (*mLayer) += WEAPON_CHUNK;
            curLayer = wbackLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
         case WEAPON_LAYER_FG:
            (*mLayer) += WEAPON_CHUNK;
            curLayer = wfrontLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
      }
      curLayer[(*nLayer)++] = w;
   }
}


/**
 * @brief Starts a beam weaapon.
 *
 *    @param outfit Outfit which spawns the weapon.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Pilot ID of the shooter.
 *    @param target Target ID that is getting shot.
 *    @param mount Mount on the ship.
 *    @return The identifier of the beam weapon.
 *
 * @sa beam_end
 */
int beam_start( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int parent, const unsigned int target,
      const int mount )
{
   WeaponLayer layer;
   Weapon *w;
   Weapon **curLayer;
   int *mLayer, *nLayer;

   if (!outfit_isBeam(outfit)) {
      ERR("Trying to create a Beam Weapon from a non-beam outfit.");
      return -1;
   }

   layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w = weapon_create( outfit, dir, pos, vel, parent, target );
   w->ID = ++beam_idgen;
   w->mount = mount;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         nLayer = &nwbackLayer;
         mLayer = &mwbacklayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         nLayer = &nwfrontLayer;
         mLayer = &mwfrontLayer;
         break;

      default:
         ERR("Invalid WEAPON_LAYER specified");
         return -1;
   }

   if (*mLayer > *nLayer) /* more memory alloced then needed */
      curLayer[(*nLayer)++] = w;
   else { /* need to allocate more memory */
      switch (layer) {
         case WEAPON_LAYER_BG:
            (*mLayer) += WEAPON_CHUNK;
            curLayer = wbackLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
         case WEAPON_LAYER_FG:
            (*mLayer) += WEAPON_CHUNK;
            curLayer = wfrontLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
      }
      curLayer[(*nLayer)++] = w;
   }

   return w->ID;
}


/**
 * @brief Ends a beam weapon.
 *
 *    @param parent ID of the parent of the beam.
 *    @param beam ID of the beam to destroy.
 */
void beam_end( const unsigned int parent, int beam )
{
   int i;
   WeaponLayer layer;
   Weapon **curLayer;
   int *mLayer, *nLayer;

   layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         nLayer = &nwbackLayer;
         mLayer = &mwbacklayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         nLayer = &nwfrontLayer;
         mLayer = &mwfrontLayer;
         break;

      default:
         ERR("Invalid WEAPON_LAYER specified");
         return;
   }


   /* Now try to destroy the beam. */
   for (i=0; i<*nLayer; i++) {
      if (curLayer[i]->ID == beam) { /* Found it. */
         weapon_destroy(curLayer[i], layer);
         break;
      }
   }
}


/**
 * @brief Destroys a weapon.
 *
 *    @param w Weapon to destroy.
 *    @param layer Layer to which the weapon belongs.
 */
static void weapon_destroy( Weapon* w, WeaponLayer layer )
{
   int i;
   Weapon** wlayer;
   int *nlayer;
   Pilot *pilot_target;

   /* Decrement target lockons if needed */
   if (outfit_isSeeker(w->outfit)) {
      pilot_target = pilot_get( w->target );
      if (pilot_target != NULL)
         pilot_target->lockons--;
   }

   /* Stop playing sound if beam weapon. */
   if (outfit_isBeam(w->outfit)) {
      sound_stop( w->voice );
      sound_playPos(w->outfit->u.bem.sound_off,
            w->solid->pos.x,
            w->solid->pos.y);
   }

   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         nlayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         nlayer = &nwfrontLayer;
         break;

      default:
         WARN("Unknown weapon layer!");
   }

   for (i=0; (wlayer[i] != w) && (i < *nlayer); i++); /* get to the curent position */
   if (i >= *nlayer) {
      WARN("Trying to destroy weapon not found in stack!");
      return;
   }

   weapon_free(wlayer[i]);
   wlayer[i] = NULL;
   (*nlayer)--;

   for ( ; i < (*nlayer); i++)
      wlayer[i] = wlayer[i+1];
}


/**
 * @brief Frees the weapon.
 *
 *    @param w Weapon to free.
 */
static void weapon_free( Weapon* w )
{
   solid_free(w->solid);

#ifdef DEBUGGING
   memset(w, 0, sizeof(Weapon));
#endif /* DEBUGGING */

   free(w);
}

/**
 * @brief Clears all the weapons, does NOT free the layers.
 */
void weapon_clear (void)
{
   int i;
   /* Don't forget to stop the sounds. */
   for (i=0; i < nwbackLayer; i++) {
      sound_stop(wbackLayer[i]->voice);
      weapon_free(wbackLayer[i]);
   }
   nwbackLayer = 0;
   for (i=0; i < nwfrontLayer; i++) {
      sound_stop(wfrontLayer[i]->voice);
      weapon_free(wfrontLayer[i]);
   }
   nwfrontLayer = 0;
}

/**
 * @brief Destroys all the weapons and frees it all.
 */
void weapon_exit (void)
{
   weapon_clear();

   if (wbackLayer != NULL) {
      free(wbackLayer);
      wbackLayer = NULL;
      mwbacklayer = 0;
   }

   if (wfrontLayer != NULL) {
      free(wfrontLayer);
      wfrontLayer = NULL;
      mwfrontLayer = 0;
   }
}


/**
 * @brief Clears possible exploded weapons.
 */
void weapon_explode( double x, double y, double radius,
      DamageType dtype, double damage,
      unsigned int parent, int mode )
{
   (void)dtype;
   (void)damage;
   weapon_explodeLayer( WEAPON_LAYER_FG, x, y, radius, parent, mode );
   weapon_explodeLayer( WEAPON_LAYER_BG, x, y, radius, parent, mode );
}


static void weapon_explodeLayer( WeaponLayer layer,
      double x, double y, double radius,
      unsigned int parent, int mode )
{
   (void)parent;
   int i;
   Weapon **curLayer;
   int *nLayer;
   double dist, rad2;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         nLayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         nLayer = &nwfrontLayer;
         break;

      default:
         ERR("Invalid WEAPON_LAYER specified");
         return;
   }

   rad2 = radius*radius;

   /* Now try to destroy the weapons affected. */
   for (i=0; i<*nLayer; i++) {
      if (((mode & EXPL_MODE_MISSILE) && outfit_isAmmo(curLayer[i]->outfit)) ||
            ((mode & EXPL_MODE_BOLT) && outfit_isBolt(curLayer[i]->outfit))) {

         dist = pow2(curLayer[i]->solid->pos.x - x) +
               pow2(curLayer[i]->solid->pos.y - y);

         if (dist < rad2) {
            weapon_destroy(curLayer[i], layer);
            i--;
         }
      }
   }
}


