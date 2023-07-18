/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file weapon.c
 *
 * @brief Handles all the weapons in game.
 *
 * Weapons are what gets created when a pilot shoots. They are based
 * on the outfit that created them.
 */
/** @cond */
#include <math.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "weapon.h"

#include "array.h"
#include "ai.h"
#include "camera.h"
#include "collision.h"
#include "explosion.h"
#include "gui.h"
#include "log.h"
#include "nstring.h"
#include "nlua_pilot.h"
#include "nlua_vec2.h"
#include "nlua_outfit.h"
#include "opengl.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "spfx.h"
#include "intlist.h"

#define weapon_isSmart(w)     (w->think != NULL) /**< Checks if the weapon w is smart. */

/* Weapon status */
typedef enum WeaponStatus_ {
   WEAPON_STATUS_LOCKING,  /**< Weapon is locking on. */
   WEAPON_STATUS_OK,       /**< Weapon is fine */
   WEAPON_STATUS_UNJAMMED, /**< Survived jamming */
   WEAPON_STATUS_JAMMED,   /**< Got jammed */
   WEAPON_STATUS_JAMMED_SLOWED, /**< Jammed and is now slower. */
} WeaponStatus;

/* Weapon flags. */
#define WEAPON_FLAG_DESTROYED    1
#define weapon_isFlag(w,f)    ((w)->flags & (f))
#define weapon_setFlag(w,f)   ((w)->flags |= (f))
#define weapon_rmFlag(w,f)    ((w)->flags &= ~(f))

/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   unsigned int flags;  /**< Weapno flags. */
   Solid solid;         /**< Actually has its own solid :) */
   unsigned int ID;     /**< Only used for beam weapons. */

   int faction;         /**< faction of pilot that shot it */
   unsigned int parent; /**< pilot that shot it */
   unsigned int target; /**< target to hit, only used by seeking things */
   const Outfit* outfit; /**< related outfit that fired it or whatnot */

   double real_vel;     /**< Keeps track of the real velocity. */
   double dam_mod;      /**< Damage modifier. */
   double dam_as_dis_mod; /**< Damage as disable modifier. */
   int voice;           /**< Weapon's voice. */
   double timer2;       /**< Explosion timer for beams, and lockon for ammo. */
   double paramf;       /**< Arbitrary parameter for outfits. */
   double life;         /**< Total life. */
   double timer;        /**< mainly used to see when the weapon was fired */
   double anim;         /**< Used for beam weapon graphics and others. */
   GLfloat r;           /**< Unique random value . */
   int sprite;          /**< Used for spinning outfits. */
   PilotOutfitSlot *mount; /**< Used for beam weapons. */
   int lua_mem;         /**< Mem table, in case of a Pilot Outfit. */
   double falloff;      /**< Point at which damage falls off. Used to determine slowdown for smart seekers.  */
   double strength;     /**< Calculated with falloff. */
   int sx;              /**< Current X sprite to use. */
   int sy;              /**< Current Y sprite to use. */
   Trail_spfx *trail;   /**< Trail graphic if applicable, else NULL. */

   void (*think)(struct Weapon_*, double); /**< for the smart missiles */

   WeaponStatus status; /**< Weapon status - to check for jamming */
} Weapon;

/**
 * @brief Struct useful for generalization of weapno collisions.
 */
typedef struct WeaponCollision_ {
   const Weapon *w;        /**< Weapon doing the colliding. */
   const OutfitGFX *gfx;   /**< Graphics of the weapon if applicable. */
   int beam;               /**< Is the weapon a beam weapon? */
   double range;           /**< Range of the weapon (or size in the case of GFX). */
   const CollPoly *polygon;/**< Collision polygon of the weapon if applicable. */
} WeaponCollision;

/* Weapon layers. */
static Weapon** wbackLayer = NULL; /**< behind pilots */
static Weapon** wfrontLayer = NULL; /**< in front of pilots, behind player */

/* Graphics. */
static gl_vbo  *weapon_vbo     = NULL; /**< Weapon VBO. */
static GLfloat *weapon_vboData = NULL; /**< Data of weapon VBO. */
static size_t weapon_vboSize   = 0; /**< Size of the VBO. */

/* Internal stuff. */
static unsigned int beam_idgen = 0; /**< Beam identifier generator. */
static IntList weapon_qtquery; /**< For querying collisions. */
static IntList weapon_qtexp; /**< For querying collisions from explosions. */

/*
 * Prototypes
 */
/* Creation. */
static double weapon_aimTurret( const Outfit *outfit, const Pilot *parent,
      const Pilot *pilot_target, const vec2 *pos, const vec2 *vel, double dir,
      double swivel, double time );
static double weapon_aimTurretStatic( const vec2 *target_pos, const vec2 *pos, double dir, double swivel );
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      double dir, const vec2* pos, const vec2* vel, const Pilot* parent, double time, int aim );
static void weapon_createAmmo( Weapon *w, const Outfit* outfit, double T,
      double dir, const vec2* pos, const vec2* vel, const Pilot* parent, double time, int aim );
static Weapon* weapon_create( PilotOutfitSlot* po, const Outfit *ref,
      double T, double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, double time, int aim );
static double weapon_computeTimes( double rdir, double rx, double ry, double dvx, double dvy, double pxv,
      double vmin, double acc, double *tt );
/* Updating. */
static void weapon_render( Weapon* w, double dt );
static void weapons_updateLayerCollide( double dt, WeaponLayer layer );
static void weapons_updateLayer( double dt, const WeaponLayer layer );
static void weapon_updateCollide( Weapon* w, double dt, WeaponLayer layer );
static void weapon_update( Weapon* w, double dt );
static void weapon_sample_trail( Weapon* w );
/* Destruction. */
static void weapon_destroy( Weapon* w );
static void weapon_free( Weapon* w );
static void weapon_explodeLayer( WeaponLayer layer,
      double x, double y, double radius,
      const Pilot *parent, int mode );
static void weapons_purgeLayer( Weapon** layer );
/* Hitting. */
static int weapon_checkCanHit( const Weapon* w, const Pilot *p );
static void weapon_hit( Weapon *w, Pilot *ptarget, const vec2 *pos );
static void weapon_miss( Weapon *w );
static void weapon_hitAst( Weapon* w, Asteroid* a, WeaponLayer layer, const vec2* pos );
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      vec2 pos[2], double dt );
static void weapon_hitAstBeam( Weapon* w, Asteroid* a, WeaponLayer layer,
      vec2 pos[2], double dt );
static int weapon_testCollision( const WeaponCollision *wc, const glTexture *ctex,
   int csx, int csy, const vec2 *cpos, const CollPoly *cpol, vec2 crash[2] );
/* think */
static void think_seeker( Weapon* w, double dt );
static void think_beam( Weapon* w, double dt );
/* externed */
void weapon_minimap( double res, double w,
      double h, const RadarShape shape, double alpha );
/* movement. */
static void weapon_setThrust( Weapon *w, double thrust );
static void weapon_setTurn( Weapon *w, double turn );

/**
 * @brief Initializes the weapon stuff.
 */
void weapon_init (void)
{
   wfrontLayer = array_create(Weapon*);
   wbackLayer  = array_create(Weapon*);
   il_create( &weapon_qtquery, 1 );
   il_create( &weapon_qtexp, 1 );
}

/**
 * @brief Draws the minimap weapons (used in player.c).
 *
 *    @param res Minimap resolution.
 *    @param w Width of minimap.
 *    @param h Height of minimap.
 *    @param shape Shape of the minimap.
 *    @param alpha Alpha to draw points at.
 */
void weapon_minimap( double res, double w,
      double h, const RadarShape shape, double alpha )
{
   int rc, p;
   const glColour *c;
   GLsizei offset;
   Pilot *par;

   /* Get offset. */
   p = 0;
   offset = weapon_vboSize;

   if (shape==RADAR_CIRCLE)
      rc = (int)(w*w);
   else
      rc = 0;

   /* Draw the points for weapons on all layers. */
   for (int i=0; i<array_size(wbackLayer); i++) {
      double x, y;
      Weapon *wp = wbackLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player.p, wp->solid.pos.x, wp->solid.pos.y ))
         continue;

      /* Get radar position. */
      x = (wp->solid.pos.x - player.p->solid.pos.x) / res;
      y = (wp->solid.pos.y - player.p->solid.pos.y) / res;

      /* Make sure in range. */
      if (shape==RADAR_RECT && (ABS(x)>w/2. || ABS(y)>h/2.))
         continue;
      if (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y)) > rc))
         continue;

      /* Choose colour based on if it'll hit player. */
      if ((outfit_isSeeker(wp->outfit) && (wp->target != PLAYER_ID)) ||
            (wp->faction == FACTION_PLAYER))
         c = &cNeutral;
      else {
         if (wp->target == PLAYER_ID)
            c = &cHostile;
         else {
            par = pilot_get(wp->parent);
            if ((par!=NULL) && pilot_isHostile(par))
               c = &cHostile;
            else
               c = &cNeutral;
         }
      }

      /* Set the colour. */
      weapon_vboData[ offset + 4*p + 0 ] = c->r;
      weapon_vboData[ offset + 4*p + 1 ] = c->g;
      weapon_vboData[ offset + 4*p + 2 ] = c->b;
      weapon_vboData[ offset + 4*p + 3 ] = alpha;

      /* Put the pixel. */
      weapon_vboData[ 2*p + 0 ] = x;
      weapon_vboData[ 2*p + 1 ] = y;

      /* "Add" pixel. */
      p++;
   }
   for (int i=0; i<array_size(wfrontLayer); i++) {
      double x, y;
      Weapon *wp = wfrontLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player.p, wp->solid.pos.x, wp->solid.pos.y ))
         continue;

      /* Get radar position. */
      x = (wp->solid.pos.x - player.p->solid.pos.x) / res;
      y = (wp->solid.pos.y - player.p->solid.pos.y) / res;

      /* Make sure in range. */
      if (shape==RADAR_RECT && (ABS(x)>w/2. || ABS(y)>h/2.))
         continue;
      if (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y)) > rc))
         continue;

      /* Choose colour based on if it'll hit player. */
      if (outfit_isSeeker(wp->outfit) && (wp->target != PLAYER_ID))
         c = &cNeutral;
      else if ((wp->target == PLAYER_ID && wp->target != wp->parent) ||
            areEnemies(FACTION_PLAYER, wp->faction))
         c = &cHostile;
      else
         c = &cNeutral;

      /* Set the colour. */
      weapon_vboData[ offset + 4*p + 0 ] = c->r;
      weapon_vboData[ offset + 4*p + 1 ] = c->g;
      weapon_vboData[ offset + 4*p + 2 ] = c->b;
      weapon_vboData[ offset + 4*p + 3 ] = alpha;

      /* Put the pixel. */
      weapon_vboData[ 2*p + 0 ] = x;
      weapon_vboData[ 2*p + 1 ] = y;

      /* "Add" pixel. */
      p++;
   }

   /* Only render with something to draw. */
   if (p > 0) {
      /* Upload data changes. */
      gl_vboSubData( weapon_vbo, 0, sizeof(GLfloat) * 2*p, weapon_vboData );
      gl_vboSubData( weapon_vbo, offset * sizeof(GLfloat),
            sizeof(GLfloat) * 4*p, &weapon_vboData[offset] );

      glUseProgram(shaders.points.program);
      glEnableVertexAttribArray(shaders.points.vertex);
      glEnableVertexAttribArray(shaders.points.vertex_color);
      gl_uniformMat4(shaders.points.projection, &gl_view_matrix);
      gl_vboActivateAttribOffset( weapon_vbo, shaders.points.vertex, 0, 2, GL_FLOAT, 0 );
      gl_vboActivateAttribOffset( weapon_vbo, shaders.points.vertex_color, offset * sizeof(GLfloat), 4, GL_FLOAT, 0 );
      glDrawArrays( GL_POINTS, 0, p );
      glDisableVertexAttribArray(shaders.points.vertex);
      glDisableVertexAttribArray(shaders.points.vertex_color);
      glUseProgram(0);
      gl_checkErr();
   }
}

/**
 * @brief Sets the weapon's thrust.
 */
static void weapon_setThrust( Weapon *w, double thrust )
{
   w->solid.thrust = thrust;
}

/**
 * @brief Sets the weapon's turn.
 */
static void weapon_setTurn( Weapon *w, double turn )
{
   w->solid.dir_vel = turn;
}

/**
 * @brief The AI of seeker missiles.
 *
 *    @param w Weapon to do the thinking.
 *    @param dt Current delta tick.
 */
static void think_seeker( Weapon* w, double dt )
{
   double diff;
   Pilot *p;
   vec2 v;
   double turn_max, d, jc, speed_mod;

   if (w->target == w->parent)
      return; /* no self shooting */

   p = pilot_get(w->target); /* no null pilot */
   if (p==NULL) {
      weapon_setThrust( w, 0. );
      weapon_setTurn( w, 0. );
      return;
   }

   //ewtrack = pilot_ewWeaponTrack( pilot_get(w->parent), p, w->outfit->u.lau.resist );

   /* Handle by status. */
   switch (w->status) {
      case WEAPON_STATUS_LOCKING: /* Check to see if we can get a lock on. */
         w->timer2 -= dt;
         if (w->timer2 >= 0.)
            weapon_setThrust( w, w->outfit->u.lau.thrust * w->outfit->mass );
         else
            w->status = WEAPON_STATUS_OK; /* Weapon locked on. */
         /* Can't get jammed while locking on. */
         break;

      case WEAPON_STATUS_OK: /* Check to see if can get jammed */
         jc = p->stats.jam_chance - w->outfit->u.lau.resist;
         if (jc > 0.) {
            /* Roll based on distance. */
            d = vec2_dist( &p->solid.pos, &w->solid.pos );
            if (d < w->r * p->ew_evasion) {
               if (RNGF() < jc) {
                  double r = RNGF();
                  if (r < 0.3) {
                     w->timer = -1.; /* Should blow up. */
                     w->status = WEAPON_STATUS_JAMMED;
                  }
                  else if (r < 0.6) {
                     w->status = WEAPON_STATUS_JAMMED;
                     weapon_setTurn( w, w->outfit->u.lau.turn * ((RNGF()>0.5)?-1.0:1.0) );
                  }
                  else if (r < 0.8) {
                     w->status = WEAPON_STATUS_JAMMED;
                     weapon_setTurn( w, 0. );
                     weapon_setThrust( w, w->outfit->u.lau.thrust * w->outfit->mass );
                  }
                  else {
                     w->status = WEAPON_STATUS_JAMMED_SLOWED;
                     w->falloff = RNGF()*0.5;
                  }
                  break;
               }
               else
                  w->status = WEAPON_STATUS_UNJAMMED;
            }
         }
         FALLTHROUGH;

      case WEAPON_STATUS_JAMMED_SLOWED: /* Slowed down. */
      case WEAPON_STATUS_UNJAMMED: /* Work as expected */
         turn_max = w->outfit->u.lau.turn;// * ewtrack;

         /* Smart seekers take into account ship velocity. */
         if (w->outfit->u.lau.ai == AMMO_AI_SMART) {
            /*
              The control interval is short enough compared to the maximum turn rate,
              so we can use a bang-bang control.
            */
            vec2_csetmin( &v, p->solid.pos.x - w->solid.pos.x,
                  p->solid.pos.y - w->solid.pos.y );

#define QUADRATURE(ref, v) ((v).x * (-(ref).y) + (v).y * (ref).x)
            if (vec2_dot(&v, &w->solid.vel) < 0) {
               /*
                 The target's behind the weapon.
                 Make U-turn.
               */
               if (QUADRATURE(w->solid.vel, v) > 0)
                  weapon_setTurn( w, turn_max );
               else
                  weapon_setTurn( w, -turn_max );
            }
            else {
               vec2 r_vel;
               vec2_csetmin( &r_vel, p->solid.vel.x - w->solid.vel.x,
                     p->solid.vel.y - w->solid.vel.y);
               if (vec2_dot(&r_vel, &w->solid.vel) > 0) {
                  /*
                    The target is going away.
                    Run parallel to the target.
                  */
                  if (QUADRATURE(w->solid.vel, p->solid.vel) > 0)
                     weapon_setTurn( w, turn_max );
                  else
                     weapon_setTurn( w, -turn_max );
               }
               else {
                  /*
                    Match the horizontal speed of the missile to the target's.
                    (cf. Proportional navigation)
                    It assumes that the approaching speed is a positive number.
                  */
                  if (QUADRATURE(r_vel, v) < 0)
                     weapon_setTurn( w, turn_max );
                  else
                     weapon_setTurn( w, -turn_max );
               }
            }
#undef QUADRATURE
         }
         /* Other seekers are simplistic. */
         else {
            diff = angle_diff(w->solid.dir, /* Get angle to target pos */
                  vec2_angle(&w->solid.pos, &p->solid.pos));
            weapon_setTurn( w, CLAMP( -turn_max, turn_max,
                  10 * diff * w->outfit->u.lau.turn ));
         }
         break;

      case WEAPON_STATUS_JAMMED: /* Continue doing whatever */
         /* Do nothing, dir_vel should be set already if needed */
         break;

      default:
         WARN(_("Unknown weapon status for '%s'"), w->outfit->name);
         break;
   }

   /* Slow off based on falloff. */
   speed_mod = (w->status==WEAPON_STATUS_JAMMED_SLOWED) ? w->falloff : 1.;

   /* Limit speed here */
   w->real_vel = MIN( speed_mod * w->outfit->u.lau.speed_max, w->real_vel + w->outfit->u.lau.thrust*dt );
   vec2_pset( &w->solid.vel, /* ewtrack * */ w->real_vel, w->solid.dir );

   /* Modulate max speed. */
   //w->solid.speed_max = w->outfit->u.lau.speed * ewtrack;
}

/**
 * @brief The pseudo-ai of the beam weapons.
 *
 *    @param w Weapon to do the thinking.
 *    @param dt Current delta tick.
 */
static void think_beam( Weapon* w, double dt )
{
   Pilot *p, *t;
   AsteroidAnchor *field;
   Asteroid *ast;
   double diff, mod;
   vec2 v;
   PilotOutfitSlot *slot;
   unsigned int turn_off;

   slot = w->mount;

   /* Get pilot, if pilot is dead beam is destroyed. */
   p = pilot_get(w->parent);
   if (p == NULL) {
      w->timer = -1.; /* Hack to make it get destroyed next update. */
      return;
   }

   /* Check if pilot has enough energy left to keep beam active. */
   mod = (w->outfit->type == OUTFIT_TYPE_BEAM) ? p->stats.fwd_energy : p->stats.tur_energy;
   p->energy -= mod * dt*w->outfit->u.bem.energy;
   pilot_heatAddSlotTime( p, slot, dt );
   if (p->energy < 0.) {
      p->energy = 0.;
      w->timer = -1;
      return;
   }

   /* Get the targets. */
   if (p->nav_asteroid != -1) {
      field = &cur_system->asteroids[p->nav_anchor];
      ast = &field->asteroids[p->nav_asteroid];
   }
   else
      ast = NULL;
   t = (w->target != w->parent) ? pilot_get(w->target) : NULL;

   /* Check the beam is still in range. */
   if (slot->inrange) {
      turn_off = 1;
      if (t != NULL) {
         if (vec2_dist( &p->solid.pos, &t->solid.pos ) <= slot->outfit->u.bem.range)
            turn_off = 0;
      }
      if (ast != NULL) {
         if (vec2_dist( &p->solid.pos, &ast->pos ) <= slot->outfit->u.bem.range)
            turn_off = 0;
      }

      /* Attempt to turn the beam off. */
      if (turn_off) {
         if (slot->outfit->u.bem.min_duration > 0.) {
            slot->stimer = slot->outfit->u.bem.min_duration -
                  (slot->outfit->u.bem.duration - slot->timer);
            if (slot->stimer > 0.)
               turn_off = 0;
         }
      }
      if (turn_off) {
         w->timer = -1;
      }
   }

   /* Use mount position. */
   pilot_getMount( p, slot, &v );
   w->solid.pos.x = p->solid.pos.x + v.x;
   w->solid.pos.y = p->solid.pos.y + v.y;

   /* Handle aiming at the target. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BEAM:
         if (w->outfit->u.bem.swivel > 0.)
            w->solid.dir = weapon_aimTurret( w->outfit, p, t, &w->solid.pos, &p->solid.vel, p->solid.dir, w->outfit->u.bem.swivel, 0. );
         else
            w->solid.dir = p->solid.dir;
         break;

      case OUTFIT_TYPE_TURRET_BEAM:
         /* If target is dead beam stops moving. Targeting
          * self is invalid so in that case we ignore the target.
          */
         t = (w->target != w->parent) ? pilot_get(w->target) : NULL;
         if (t == NULL) {
            if (ast != NULL) {
               diff = angle_diff(w->solid.dir, /* Get angle to target pos */
                     vec2_angle(&w->solid.pos, &ast->pos));
            }
            else
               diff = angle_diff(w->solid.dir, p->solid.dir);
         }
         else
            diff = angle_diff(w->solid.dir, /* Get angle to target pos */
                  vec2_angle(&w->solid.pos, &t->solid.pos));

         weapon_setTurn( w, CLAMP( -w->outfit->u.bem.turn, w->outfit->u.bem.turn,
                  10 * diff *  w->outfit->u.bem.turn ));
         break;

      default:
         return;
   }
}

/**
 * @brief Purges unnecessary weapons.
 */
void weapons_updatePurge (void)
{
   /* Actually purge and remove weapons. */
   weapons_purgeLayer( wbackLayer );
   weapons_purgeLayer( wfrontLayer );
}

/**
 * @brief Handles weapon collisions.
 */
void weapons_updateCollide( double dt )
{
   weapons_updateLayerCollide(dt,WEAPON_LAYER_BG);
   weapons_updateLayerCollide(dt,WEAPON_LAYER_FG);
}

/**
 * @brief Updates all the weapon layers.
 *
 *    @param dt Current delta tick.
 */
void weapons_update( double dt )
{
   /* When updating, just mark weapons for deletion. */
   weapons_updateLayer(dt,WEAPON_LAYER_BG);
   weapons_updateLayer(dt,WEAPON_LAYER_FG);
}

/**
 * @brief Updates all the weapons in the layer.
 *
 *    @param dt Current delta tick.
 *    @param layer Layer to update.
 */
static void weapons_updateLayerCollide( double dt, WeaponLayer layer )
{
   Weapon **wlayer;

   /* Choose layer. */
   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         break;

      default:
         WARN(_("Unknown weapon layer!"));
         return;
   }

   for (int i=0; i<array_size(wlayer); i++) {
      Weapon *w = wlayer[i];

      /* Ignore destroyed wapons. */
      if (weapon_isFlag(w, WEAPON_FLAG_DESTROYED))
         continue;

      /* Handle types. */
      switch (w->outfit->type) {

         /* most missiles behave the same */
         case OUTFIT_TYPE_LAUNCHER:
         case OUTFIT_TYPE_TURRET_LAUNCHER:
            w->timer -= dt;
            if (w->timer < 0.) {
               int spfx = -1;
               /* See if we need armour death sprite. */
               if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR))
                  spfx = outfit_spfxArmour(w->outfit);
               /* See if we need shield death sprite. */
               else if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_SHIELD))
                  spfx = outfit_spfxShield(w->outfit);
               /* Add death sprite if needed. */
               if (spfx != -1) {
                  int s;
                  spfx_add( spfx, w->solid.pos.x, w->solid.pos.y,
                        w->solid.vel.x, w->solid.vel.y,
                        SPFX_LAYER_MIDDLE ); /* presume middle. */
                  /* Add sound if explodes and has it. */
                  s = outfit_soundHit(w->outfit);
                  if (s != -1)
                     w->voice = sound_playPos(s,
                           w->solid.pos.x,
                           w->solid.pos.y,
                           w->solid.vel.x,
                           w->solid.vel.y);
               }
               weapon_miss(w);
               break;
            }
            break;

         case OUTFIT_TYPE_BOLT:
         case OUTFIT_TYPE_TURRET_BOLT:
            w->timer -= dt;
            if (w->timer < 0.) {
               int spfx = -1;
               /* See if we need armour death sprite. */
               if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR))
                  spfx = outfit_spfxArmour(w->outfit);
               /* See if we need shield death sprite. */
               else if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_SHIELD))
                  spfx = outfit_spfxShield(w->outfit);
               /* Add death sprite if needed. */
               if (spfx != -1) {
                  int s;
                  spfx_add( spfx, w->solid.pos.x, w->solid.pos.y,
                        w->solid.vel.x, w->solid.vel.y,
                        SPFX_LAYER_MIDDLE ); /* presume middle. */
                  /* Add sound if explodes and has it. */
                  s = outfit_soundHit(w->outfit);
                  if (s != -1)
                     w->voice = sound_playPos(s,
                           w->solid.pos.x,
                           w->solid.pos.y,
                           w->solid.vel.x,
                           w->solid.vel.y);
               }
               weapon_miss(w);
               break;
            }
            else if (w->timer < w->falloff)
               w->strength = w->timer / w->falloff;
            break;

         /* Beam weapons handled a part. */
         case OUTFIT_TYPE_BEAM:
         case OUTFIT_TYPE_TURRET_BEAM:
            /* Beams don't have inherent accuracy, so we use the
             * heatAccuracyMod to modulate duration. */
            w->timer -= dt / (1.-pilot_heatAccuracyMod(w->mount->heat_T));
            if (w->timer < 0. || (w->outfit->u.bem.min_duration > 0. &&
                  w->mount->stimer < 0.)) {
               Pilot *p = pilot_get(w->parent);
               if (p != NULL)
                  pilot_stopBeam(p, w->mount);
               weapon_miss(w);
               break;
            }
            /* We use the explosion timer to tell when we have to create explosions. */
            w->timer2 -= dt;
            if (w->timer2 < 0.) {
               if (w->timer2 < -1.)
                  w->timer2 = 0.100;
               else
                  w->timer2 = -1.;
            }
            break;
         default:
            WARN(_("Weapon of type '%s' has no update implemented yet!"),
                  w->outfit->name);
            break;
      }

      /* Only increment if weapon wasn't destroyed. */
      if (!weapon_isFlag(w, WEAPON_FLAG_DESTROYED))
         weapon_updateCollide(w,dt,layer);
   }
}

/**
 * @brief Purges weapons marked for deletion.
 *
 *    @param layer Layer to purge weapons from.
 */
static void weapons_purgeLayer( Weapon** layer )
{
   for (int i=0; i<array_size(layer); i++) {
      if (weapon_isFlag(layer[i],WEAPON_FLAG_DESTROYED)) {
         weapon_free(layer[i]);
         array_erase( &layer, &layer[i], &layer[i+1] );
         i--;
      }
   }
}

/**
 * @brief Updates all the weapons in the layer.
 *
 *    @param dt Current delta tick.
 *    @param layer Layer to update.
 */
static void weapons_updateLayer( double dt, WeaponLayer layer )
{
   Weapon **wlayer;

   /* Choose layer. */
   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         break;

      default:
         WARN(_("Unknown weapon layer!"));
         return;
   }

   for (int i=0; i<array_size(wlayer); i++) {
      Weapon *w = wlayer[i];
      /* Only increment if weapon wasn't destroyed. */
      if (!weapon_isFlag(w, WEAPON_FLAG_DESTROYED))
         weapon_update( w, dt );
   }
}

/**
 * @brief Renders all the weapons in a layer.
 *
 *    @param layer Layer to render.
 *    @param dt Current delta tick.
 */
void weapons_render( const WeaponLayer layer, double dt )
{
   Weapon** wlayer;

   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         break;

      default:
         WARN(_("Unknown weapon layer!"));
         return;
   }

   for (int i=0; i<array_size(wlayer); i++)
      weapon_render( wlayer[i], dt );
}

static void weapon_renderBeam( Weapon* w, double dt )
{
   double x, y, z;
   mat4 projection;

   /* Animation. */
   w->anim += dt;

   /* Load GLSL program */
   glUseProgram(shaders.beam.program);

   /* Zoom. */
   z = cam_getZoom();

   /* Position. */
   gl_gameToScreenCoords( &x, &y, w->solid.pos.x, w->solid.pos.y );

   projection = gl_view_matrix;
   mat4_translate( &projection, x, y, 0. );
   mat4_rotate2d( &projection, w->solid.dir );
   mat4_scale( &projection, w->outfit->u.bem.range*z,w->outfit->u.bem.width * z, 1. );
   mat4_translate( &projection, 0., -0.5, 0. );

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.beam.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.beam.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformMat4(shaders.beam.projection, &projection);
   gl_uniformColor(shaders.beam.color, &w->outfit->u.bem.colour);
   glUniform2f(shaders.beam.dimensions, w->outfit->u.bem.range, w->outfit->u.bem.width);
   glUniform1f(shaders.beam.dt, w->anim);
   glUniform1f(shaders.beam.r, w->r);

   /* Set the subroutine. */
   if (gl_has( OPENGL_SUBROUTINES ))
      glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &w->outfit->u.bem.shader );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.beam.vertex );
   glUseProgram(0);

   /* anything failed? */
   gl_checkErr();
}

/**
 * @brief Renders an individual weapon.
 *
 *    @param w Weapon to render.
 *    @param dt Current delta tick.
 */
static void weapon_render( Weapon* w, double dt )
{
   const OutfitGFX *gfx;
   double x, y, st;
   glColour col, c = { .r=1., .g=1., .b=1. };

   /* Don't render destroyed weapons. */
   if (weapon_isFlag(w,WEAPON_FLAG_DESTROYED))
      return;

   switch (w->outfit->type) {
      /* Weapons that use sprites. */
      case OUTFIT_TYPE_LAUNCHER:
      case OUTFIT_TYPE_TURRET_LAUNCHER:
         if (w->status == WEAPON_STATUS_LOCKING) {
            double r, z;
            z = cam_getZoom();
            gl_gameToScreenCoords( &x, &y, w->solid.pos.x, w->solid.pos.y );
            r = w->outfit->u.lau.gfx.size * z * 0.75; /* Assume square. */

            st = 1. - w->timer2 / w->paramf;
            col_blend( &col, &cYellow, &cRed, st );
            col.a = 0.5;

            glUseProgram( shaders.iflockon.program );
            glUniform1f( shaders.iflockon.paramf, st );
            gl_renderShader( x, y, r, r, r, &shaders.iflockon, &col, 1 );
         }
         FALLTHROUGH;
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         gfx = outfit_gfx(w->outfit);

         /* Alpha based on strength. */
         c.a = w->strength;

         /* Outfit spins around. */
         if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_SPIN)) {

            /* Render. */
            if (gfx->tex != NULL) {
               const glTexture *tex = gfx->tex;

               /* Check timer. */
               w->anim -= dt;
               if (w->anim < 0.) {
                  w->anim = outfit_spin(w->outfit);

                  /* Increment sprite. */
                  w->sprite++;
                  if (w->sprite >= tex->sx*tex->sy)
                     w->sprite = 0;
               }

               if (gfx->tex_end != NULL)
                  gl_renderSpriteInterpolate( tex, gfx->tex_end,
                        w->timer / w->life,
                        w->solid.pos.x, w->solid.pos.y,
                        w->sprite % (int)tex->sx, w->sprite / (int)tex->sx, &c );
               else
                  gl_renderSprite( tex, w->solid.pos.x, w->solid.pos.y,
                        w->sprite % (int)tex->sx, w->sprite / (int)tex->sx, &c );
            }
         }
         /* Outfit faces direction. */
         else {
            /* Render. */
            if (gfx->tex != NULL) {
               const glTexture *tex = gfx->tex;
               if (gfx->tex_end != NULL)
                  gl_renderSpriteInterpolate( tex, gfx->tex_end,
                        w->timer / w->life,
                        w->solid.pos.x, w->solid.pos.y, w->sx, w->sy, &c );
               else
                  gl_renderSprite( tex, w->solid.pos.x, w->solid.pos.y, w->sx, w->sy, &c );
            }
            else {
               double r, z;

               /* Translate coords. */
               z = cam_getZoom();
               gl_gameToScreenCoords( &x, &y, w->solid.pos.x, w->solid.pos.y );

               /* Scaled sprite dimensions. */
               r = gfx->size*z;

               /* Check if inbounds */
               if ((x < -r) || (x > SCREEN_W+r) ||
                     (y < -r) || (y > SCREEN_H+r))
                  return;

               mat4 projection = gl_view_matrix;
               mat4_translate( &projection, x, y, 0. );
               mat4_rotate2d( &projection, w->solid.dir );
               mat4_scale( &projection, r, r, 1. );

               glUseProgram( gfx->program );
               glUniform2f( gfx->dimensions, r, r );
               glUniform1f( gfx->u_r, w->r );
               glUniform1f( gfx->u_time, w->life-w->timer );
               glUniform1f( gfx->u_fade, w->strength );
               gl_uniformMat4( gfx->projection, &projection );

               glEnableVertexAttribArray( gfx->vertex );
               gl_vboActivateAttribOffset( gl_circleVBO, gfx->vertex, 0, 2, GL_FLOAT, 0 );

               glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

               glDisableVertexAttribArray(gfx->vertex);
               glUseProgram(0);
               gl_checkErr();
            }
         }
         break;

      /* Beam weapons. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         weapon_renderBeam(w, dt);
         break;

      default:
         WARN(_("Weapon of type '%s' has no render implemented yet!"),
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
static int weapon_checkCanHit( const Weapon* w, const Pilot *p )
{
   /* Can't hit invincible stuff. */
   if (pilot_isFlag(p, PILOT_INVINCIBLE))
      return 0;

   /* Can't hit hidden stuff. */
   if (pilot_isFlag(p, PILOT_HIDE))
      return 0;

   /* Must not be landing nor taking off. */
   if (pilot_isFlag(p, PILOT_LANDING) ||
         pilot_isFlag(p, PILOT_TAKEOFF))
      return 0;

   /* Go "through" dead pilots. */
   if (pilot_isFlag(p, PILOT_DEAD))
      return 0;

   /* Player can not hit special pilots. */
   if ((w->faction == FACTION_PLAYER) &&
         pilot_isFlag(p, PILOT_INVINC_PLAYER))
      return 0;

   /* Always hit target. */
   if (w->target == p->id)
      return 1;

   /* Can never hit same faction, unless explicitly targetted (see above). */
   if (p->faction == w->faction)
      return 0;

   /* Player behaves differently. */
   if (w->faction == FACTION_PLAYER) {

      /* Always hit hostiles. */
      if (pilot_isHostile(p))
         return 1;

      /* Miss rest - can be neutral/ally. */
      else
         return 0;
   }

   /* Let hostiles hit player. */
   if (p->faction == FACTION_PLAYER) {
      const Pilot *parent = pilot_get(w->parent);
      if (parent != NULL) {
         if (pilot_isHostile(parent))
            return 1;
      }
   }

   /* Hit non-allies. */
   if (areEnemies(w->faction, p->faction))
      return 1;

   return 0;
}

/**
 * @brief Tests to see if a weapon collides with a ship.
 *
 *    @param wc Weapon collision data.
 *    @param ctex Collision target texture.
 *    @param csx Collision target texture x sprite.
 *    @param csy Collision target texture y sprite.
 *    @param cpos Collision target pos.
 *    @param cpol Collision target collision polygon (NULL if none).
 *    @param[out] crash Crash location, which is only set if collision is detected.
 *    @return Number of collisions detected (0 to 2)
 */
static int weapon_testCollision( const WeaponCollision *wc, const glTexture *ctex,
   int csx, int csy, const vec2 *cpos, const CollPoly *cpol, vec2 crash[2] )
{
   const Weapon *w = wc->w;
   if (wc->beam) {
      if (cpol!=NULL) {
         int k = ctex->sx * csy + csx;
         return CollideLinePolygon( &w->solid.pos, w->solid.dir,
               wc->range, &cpol[k], cpos, crash);
      }
      else {
         return CollideLineSprite( &w->solid.pos, w->solid.dir,
               wc->range, ctex, csx, csy, cpos, crash);
      }
   }
   else {
      /* Case full polygon on polygon collision. */
      if ((wc->polygon!=NULL) && (cpol!=NULL)) {
         int k = ctex->sx * csy + csx;
         return CollidePolygon( &cpol[k], cpos, wc->polygon, &w->solid.pos, crash );
      }
      /* Case texture on texture collision. */
      else if ((wc->gfx != NULL) && (wc->gfx->tex!=NULL)) {
         return CollideSprite( wc->gfx->tex, w->sx, w->sy, &w->solid.pos,
                  ctex, csx, csy, cpos, crash );
      }
      /* Fallback using simple spherical collision. */
      else if (cpol != NULL) {
         return CollideCirclePolygon( &w->solid.pos, wc->range, cpol, cpos, crash );
      }
      /* TODO case no polygon and circle collision. */
      else
         assert( "TODO implement case no ship polygon and circle collision"==NULL );
      return 0;
   }
}

/**
 * @brief Updates an individual weapon.
 *
 *    @param w Weapon to update.
 *    @param dt Current delta tick.
 *    @param layer Layer to which the weapon belongs.
 */
static void weapon_updateCollide( Weapon* w, double dt, WeaponLayer layer )
{
   vec2 crash[2];
   WeaponCollision wc;
   Pilot *const* pilot_stack = pilot_getAll();
   int x1, y1, x2, y2;

   /* Get the sprite direction to speed up calculations. */
   wc.w = w;
   wc.beam = outfit_isBeam(w->outfit);
   if (!wc.beam) {
      int x, y, w2, h2;
      wc.gfx = outfit_gfx(w->outfit);
      if (wc.gfx->tex != NULL) {
         const CollPoly *plg;
         int n;
         gl_getSpriteFromDir( &w->sx, &w->sy, wc.gfx->tex, w->solid.dir );
         n = wc.gfx->tex->sx * w->sy + w->sx;
         plg = outfit_plg(w->outfit);
         wc.polygon = &plg[n];
         wc.range = wc.gfx->size; /* Range is set to size in this case. */
      }
      else {
         wc.polygon = NULL;
         wc.range = wc.gfx->col_size;
      }

      /* Determine quadtree location. */
      x = round(w->solid.pos.x);
      y = round(w->solid.pos.y);
      w2 = ceil(wc.range * 0.5);
      h2 = ceil(wc.range * 0.5);
      x1 = x-w2;
      y1 = y-h2;
      x2 = x+w2;
      y2 = y+h2;
   }
   else {
      Pilot *p = pilot_get( w->parent );
      /* Beams have to update properties as necessary. */
      if (p != NULL) {
         /* Beams need to update their properties online. */
         if (w->outfit->type == OUTFIT_TYPE_BEAM) {
            w->dam_mod        = p->stats.fwd_damage;
            w->dam_as_dis_mod = p->stats.fwd_dam_as_dis-1.;
         }
         else {
            w->dam_mod        = p->stats.tur_damage;
            w->dam_as_dis_mod = p->stats.tur_dam_as_dis-1.;
         }
         w->dam_as_dis_mod = CLAMP( 0., 1., w->dam_as_dis_mod );
      }
      wc.gfx = NULL;
      wc.polygon = NULL;
      wc.range = w->outfit->u.bem.range; /* Set beam range. */

      /* Determine quadtree location. */
      x1 = round(w->solid.pos.x);
      y1 = round(w->solid.pos.y);
      x2 = x1 + ceil( w->outfit->u.bem.range * cos(w->solid.dir) );
      y2 = y1 + ceil( w->outfit->u.bem.range * sin(w->solid.dir) );
      if (x1 > x2) {
         int t = x1;
         x1 = x2;
         x2 = t;
      }
      if (y1 > y2) {
         int t = y1;
         y1 = y2;
         y2 = t;
      }
   }

   /* Get what collides. */
   pilot_collideQueryIL( &weapon_qtquery, x1, y1, x2, y2 );
   for (int i=0; i<il_size(&weapon_qtquery); i++) {
      Pilot *p = pilot_stack[ il_get( &weapon_qtquery, i, 0 ) ];

      /* Ignore pilots being deleted. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      /* Ignore if parent is self. */
      if (w->parent == p->id)
         continue; /* pilot is self */

      /* Smart weapons only collide with their target */
      if (weapon_isSmart(w)) {
         int isjammed = ((w->status == WEAPON_STATUS_JAMMED) || (w->status == WEAPON_STATUS_JAMMED_SLOWED));
         if (!isjammed && (p->id != w->target))
            continue;
      }

      /* Check to see if it can hit. */
      if (!weapon_checkCanHit(w,p))
         continue;

      /* Test if hit. */
      if (!weapon_testCollision( &wc, p->ship->gfx_space, p->tsx, p->tsy,
            &p->solid.pos, p->ship->polygon, crash ))
         continue;

      /* Handle the hit. */
      if (wc.beam)
         weapon_hitBeam( w, p, layer, crash, dt );
         /* No return because beam can still think, it's not
         * destroyed like the other weapons.*/
      else {
         weapon_hit( w, p, crash );
         return; /* Weapon is destroyed. */
      }
   }

   /* Collide with asteroids*/
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];

      /* Early in-range check with the asteroid field. */
      if (vec2_dist2( &w->solid.pos, &ast->pos ) >
            pow2( ast->radius + ast->margin + wc.range ))
         continue;

      /* Quadtree collisions. */
      asteroid_collideQueryIL( ast, &weapon_qtquery, x1, y1, x2, y2 );
      for (int j=0; j<il_size(&weapon_qtquery); j++) {
         Asteroid *a = &ast->asteroids[ il_get( &weapon_qtquery, j, 0 ) ];
         int coll;
         if (a->state != ASTEROID_FG)
            continue;

         /* In-range check with the actual asteroid. */
         /* This is advantageous because we are going to rotate the polygon afterwards. */
         if (vec2_dist2( &w->solid.pos, &a->pos ) > pow2( wc.range ))
            continue;

         if (a->polygon->npt!=0) {
            CollPoly rpoly;
            RotatePolygon( &rpoly, a->polygon, (float) a->ang );
            coll = weapon_testCollision( &wc, a->gfx, 0, 0, &a->pos, &rpoly, crash );
            free(rpoly.x);
            free(rpoly.y);
         }
         else
            coll = weapon_testCollision( &wc, a->gfx, 0, 0, &a->pos, NULL, crash );

         /* Missed. */
         if (!coll)
            continue;

         /* Handle the hit. */
         if (wc.beam)
            weapon_hitAstBeam( w, a, layer, crash, dt );
         else {
            weapon_hitAst( w, a, layer, crash );
            return; /* Weapon is destroyed. */
         }
      }
   }
}

/**
 * @brief Updates an individual weapon.
 *
 *    @param w Weapon to update.
 *    @param dt Current delta tick.
 */
static void weapon_update( Weapon* w, double dt )
{
   /* Smart weapons also get to think their next move */
   if (weapon_isSmart(w))
      (*w->think)( w, dt );

   /* Update the solid position. */
   (*w->solid.update)( &w->solid, dt );

   /* Update the sound. */
   sound_updatePos(w->voice, w->solid.pos.x, w->solid.pos.y,
         w->solid.vel.x, w->solid.vel.y);

   /* Update the trail. */
   if (w->trail != NULL)
      weapon_sample_trail( w );
}

/**
 * @brief Updates the animated trail for a weapon.
 */
static void weapon_sample_trail( Weapon* w )
{
   double a, dx, dy;
   TrailMode mode;

   if (!space_needsEffects())
      return;

   /* Compute the engine offset. */
   a  = w->solid.dir;
   dx = w->outfit->u.lau.trail_x_offset * cos(a);
   dy = w->outfit->u.lau.trail_x_offset * sin(a);

   /* Set the colour. */
   if ((w->outfit->u.lau.ai == AMMO_AI_UNGUIDED) ||
        w->solid.vel.x*w->solid.vel.x + w->solid.vel.y*w->solid.vel.y + 1.
        < w->solid.speed_max*w->solid.speed_max)
      mode = MODE_AFTERBURN;
   else if (w->solid.dir_vel != 0.)
      mode = MODE_GLOW;
   else
      mode = MODE_IDLE;

   spfx_trail_sample( w->trail, w->solid.pos.x + dx, w->solid.pos.y + dy*M_SQRT1_2, mode, 0 );
}

/**
 * @brief Informs the AI if needed that it's been hit.
 *
 *    @param p Pilot being hit.
 *    @param shooter Pilot that shot.
 *    @param dmg Damage done to p.
 */
void weapon_hitAI( Pilot *p, const Pilot *shooter, double dmg )
{
   /* Must be a valid shooter. */
   if (shooter == NULL)
      return;

   /* Only care about actual damage. */
   if (dmg <= 0.)
      return;

   /* Must not be disabled. */
   if (pilot_isDisabled(p))
      return;

   /* Must not be deleting. */
   if (pilot_isFlag(p, PILOT_DELETE) || pilot_isFlag(p, PILOT_DEAD) || pilot_isFlag( p, PILOT_HIDE ))
      return;

   /* Player is handled differently. */
   if (shooter->faction == FACTION_PLAYER) {
      /* Increment damage done to by player. */
      p->player_damage += dmg / (p->shield_max + p->armour_max);

      /* If damage is over threshold, inform pilot or if is targeted. */
      if ((p->player_damage > PILOT_HOSTILE_THRESHOLD) ||
            (shooter->target==p->id)) {
         /* Inform attacked. */
         pilot_setHostile(p);
         ai_attacked( p, shooter->id, dmg );
      }
   }
   /* Otherwise just inform of being attacked. */
   else
      ai_attacked( p, shooter->id, dmg );
}

static void weapon_hitExplode( Weapon *w, const Damage *dmg, const vec2 *pos, double radius )
{
   Pilot *const* pilot_stack = pilot_getAll();
   int x, y, r, x1, y1, x2, y2;
   Pilot *parent = pilot_get( w->parent );
   WeaponCollision wc;

   /* Circle explosion. */
   wc.w     = w;
   wc.gfx   = NULL;
   wc.beam  = 0;
   wc.range = radius;
   wc.polygon = NULL;

   /* Set up coordinates. */
   x = round(pos->x);
   y = round(pos->y);
   r = ceil(radius);
   x1 = x-r;
   y1 = y-r;
   x2 = x+r;
   y2 = y+r;

   /* Test pilots. */
   pilot_collideQueryIL( &weapon_qtexp, x1, y1, x2, y2 );
   for (int i=0; i<il_size(&weapon_qtexp); i++) {
      vec2 crash[2];
      double damage;
      Pilot *p = pilot_stack[ il_get( &weapon_qtexp, i, 0 ) ];

      /* Ignore pilots being deleted. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      if (!outfit_isProp( w->outfit, OUTFIT_PROP_WEAP_FRIENDLYFIRE )) {
         /* Ignore if parent is self. */
         if (w->parent == p->id)
            continue; /* pilot is self */

         /* Check to see if it can hit. */
         if (!weapon_checkCanHit( w, p ))
            continue;
      }

      /* Test if hit. */
      if (!weapon_testCollision( &wc, p->ship->gfx_space, p->tsx, p->tsy,
               &p->solid.pos, p->ship->polygon, crash ))
         continue;

      /* Have pilot take damage and get real damage done. */
      damage = pilot_hit( p, &w->solid, parent, dmg, w->outfit, w->lua_mem, 1 );
      /* Inform AI that it's been hit. */
      weapon_hitAI( p, parent, damage );
   }

   /* Test asteroids. */
   double mining_bonus = (parent != NULL) ? parent->stats.mining_bonus : 1.;
   int mining_rarity = outfit_miningRarity( w->outfit );
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];

      /* Early in-range check with the asteroid field. */
      if (vec2_dist2( &w->solid.pos, &ast->pos ) >
            pow2( ast->radius + ast->margin + wc.range ))
         continue;

      /* Quadtree collisions. */
      asteroid_collideQueryIL( ast, &weapon_qtquery, x1, y1, x2, y2 );
      for (int j=0; j<il_size(&weapon_qtquery); j++) {
         Asteroid *a = &ast->asteroids[ il_get( &weapon_qtquery, j, 0 ) ];
         vec2 crash[2];
         int coll;
         if (a->state != ASTEROID_FG)
            continue;

         if (a->polygon->npt!=0) {
            CollPoly rpoly;
            RotatePolygon( &rpoly, a->polygon, (float) a->ang );
            coll = weapon_testCollision( &wc, a->gfx, 0, 0, &a->pos, &rpoly, crash );
            free(rpoly.x);
            free(rpoly.y);
         }
         else
            coll = weapon_testCollision( &wc, a->gfx, 0, 0, &a->pos, NULL, crash );

         /* Missed. */
         if (!coll)
            continue;

         asteroid_hit( a, dmg, mining_rarity, mining_bonus );
      }
   }
}

/**
 * @brief Weapon hit the pilot.
 *
 *    @param w Weapon involved in the collision.
 *    @param ptarget Pilot that got hit.
 *    @param pos Position of the hit.
 */
static void weapon_hit( Weapon *w, Pilot *ptarget, const vec2 *pos )
{
   int s, spfx;
   double damage, radius;
   WeaponLayer spfx_layer;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   damage            = w->dam_mod * w->strength * odmg->damage;
   radius            = outfit_radius( w->outfit );
   dmg.damage        = MAX( 0., damage * (1.-w->dam_as_dis_mod) );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = MAX( 0., w->dam_mod * w->strength * odmg->disable + damage * w->dam_as_dis_mod );

   /* Play sound if they have it. */
   s = outfit_soundHit(w->outfit);
   if (s != -1)
      w->voice = sound_playPos( s,
            w->solid.pos.x, w->solid.pos.y,
            w->solid.vel.x, w->solid.vel.y );

   if (radius > 0.) {
      weapon_hitExplode( w, &dmg, pos, radius );
   }
   else {
      Pilot *parent = pilot_get( w->parent );
      /* Have pilot take damage and get real damage done. */
      damage = pilot_hit( ptarget, &w->solid, parent, &dmg, w->outfit, w->lua_mem, 1 );
      /* Inform AI that it's been hit. */
      weapon_hitAI( ptarget, parent, damage );
   }

   /* Get the layer. */
   spfx_layer = (ptarget==player.p) ? SPFX_LAYER_FRONT : SPFX_LAYER_MIDDLE;
   /* Choose spfx. */
   if (ptarget->shield > 0.)
      spfx = outfit_spfxShield(w->outfit);
   else
      spfx = outfit_spfxArmour(w->outfit);
   /* Add sprite, layer depends on whether player shot or not. */
   spfx_add( spfx, pos->x, pos->y,
         VX(ptarget->solid.vel), VY(ptarget->solid.vel), spfx_layer );

   /* no need for the weapon particle anymore */
   weapon_destroy(w);
}

/**
 * @brief Weapon missed and is due to be destroyed.
 *
 *    @param w Weapon that missed.
 */
static void weapon_miss( Weapon *w )
{
   /* On hit weapon effects. */
   if (w->outfit->lua_onmiss != LUA_NOREF) {
      Pilot *parent = pilot_get( w->parent );

      lua_rawgeti(naevL, LUA_REGISTRYINDEX, w->lua_mem); /* mem */
      nlua_setenv(naevL, w->outfit->lua_env, "mem"); /* */

      /* Set up the function: onmiss() */
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, w->outfit->lua_onmiss); /* f */
      lua_pushpilot(naevL, (parent==NULL) ? 0 : parent->id);
      lua_pushvector(naevL, w->solid.pos);
      lua_pushvector(naevL, w->solid.vel);
      lua_pushoutfit(naevL, w->outfit);
      if (nlua_pcall( w->outfit->lua_env, 4, 0 )) {   /* */
         WARN( _("Outfit '%s' -> '%s':\n%s"), w->outfit->name, "onmiss", lua_tostring(naevL,-1) );
         lua_pop(naevL, 1);
      }
   }

   weapon_destroy(w);
}

/**
 * @brief Weapon hit an asteroid.
 *
 *    @param w Weapon involved in the collision.
 *    @param a Asteroid that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 */
static void weapon_hitAst( Weapon* w, Asteroid* a, WeaponLayer layer, const vec2* pos )
{
   int s, spfx;
   Damage dmg;
   const Damage *odmg;
   double radius;
   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   radius            = outfit_radius( w->outfit );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = odmg->disable;

   /* Play sound if they have it. */
   s = outfit_soundHit(w->outfit);
   if (s != -1)
      w->voice = sound_playPos( s,
            w->solid.pos.x, w->solid.pos.y,
            w->solid.vel.x, w->solid.vel.y );

   /* Add the spfx */
   spfx = outfit_spfxArmour(w->outfit);
   spfx_add( spfx, pos->x, pos->y,VX(a->vel), VY(a->vel), layer );

   if (radius > 0.) {
      weapon_hitExplode( w, &dmg, pos, radius );
   }
   else {
      Pilot *parent = pilot_get( w->parent );
      double mining_bonus = (parent != NULL) ? parent->stats.mining_bonus : 1.;
      asteroid_hit( a, &dmg, outfit_miningRarity(w->outfit), mining_bonus );
   }

   weapon_destroy(w);
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
      vec2 pos[2], double dt )
{
   (void) layer;
   Pilot *parent;
   int spfx;
   double damage;
   WeaponLayer spfx_layer;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   parent            = pilot_get( w->parent );
   damage            = w->dam_mod * w->strength * odmg->damage * dt ;
   dmg.damage        = MAX( 0., damage * (1.-w->dam_as_dis_mod) );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = MAX( 0., w->dam_mod * w->strength * odmg->disable * dt + damage * w->dam_as_dis_mod );

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, &w->solid, parent, &dmg, w->outfit, w->lua_mem, 1 );

   /* Add sprite, layer depends on whether player shot or not. */
   if (w->timer2 == -1.) {
      /* Get the layer. */
      spfx_layer = (p==player.p) ? SPFX_LAYER_FRONT : SPFX_LAYER_MIDDLE;

      /* Choose spfx. */
      if (p->shield > 0.)
         spfx = outfit_spfxShield(w->outfit);
      else
         spfx = outfit_spfxArmour(w->outfit);

      /* Add graphic. */
      spfx_add( spfx, pos[0].x, pos[0].y,
            VX(p->solid.vel), VY(p->solid.vel), spfx_layer );
      spfx_add( spfx, pos[1].x, pos[1].y,
            VX(p->solid.vel), VY(p->solid.vel), spfx_layer );
      w->timer2 = -2.;

      /* Inform AI that it's been hit, to not saturate ai Lua with messages. */
      weapon_hitAI( p, parent, damage );
   }
}

/**
 * @brief Weapon hit an asteroid.
 *
 *    @param w Weapon involved in the collision.
 *    @param a Asteroid that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 *    @param dt Current delta tick.
 */
static void weapon_hitAstBeam( Weapon* w, Asteroid* a, WeaponLayer layer,
      vec2 pos[2], double dt )
{
   (void) layer;
   Damage dmg;
   const Damage *odmg;
   Pilot *parent;
   double mining_bonus;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage * dt );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = odmg->disable * dt;

   parent = pilot_get( w->parent );
   mining_bonus = (parent != NULL) ? parent->stats.mining_bonus : 1.;
   asteroid_hit( a, &dmg, outfit_miningRarity(w->outfit), mining_bonus );

   /* Add sprite. */
   if (w->timer2 == -1.) {
      int spfx = outfit_spfxArmour(w->outfit);

      /* Add graphic. */
      spfx_add( spfx, pos[0].x, pos[0].y,
            VX(a->vel), VY(a->vel), SPFX_LAYER_MIDDLE );
      spfx_add( spfx, pos[1].x, pos[1].y,
            VX(a->vel), VY(a->vel), SPFX_LAYER_MIDDLE );
      w->timer2 = -2.;
   }
}

/**
 * @brief Gets the aim position of a turret weapon.
 *
 *    @param outfit Weapon outfit.
 *    @param parent Parent of the weapon.
 *    @param pilot_target Target of the weapon.
 *    @param pos Position of the turret.
 *    @param vel Velocity of the turret.
 *    @param dir Direction facing parent ship and turret.
 *    @param time Expected flight time.
 *    @param swivel Maximum angle between weapon and straight ahead.
 */
static double weapon_aimTurret( const Outfit *outfit, const Pilot *parent,
      const Pilot *pilot_target, const vec2 *pos, const vec2 *vel, double dir,
      double swivel, double time )
{
   const vec2 *target_pos, *target_vel;
   double rx, ry, x, y, t, lead, rdir, off;

   if (pilot_target != NULL) {
      target_pos = &pilot_target->solid.pos;
      target_vel = &pilot_target->solid.vel;
   }
   else {
      if (parent->nav_asteroid < 0)
         return dir;

      AsteroidAnchor *field = &cur_system->asteroids[parent->nav_anchor];
      Asteroid *ast = &field->asteroids[parent->nav_asteroid];
      target_pos = &ast->pos;
      target_vel = &ast->vel;
   }

   /* Get the vector : shooter -> target */
   rx = target_pos->x - pos->x;
   ry = target_pos->y - pos->y;

   /* Try to predict where the enemy will be. */
   t = time;
   if (t == INFINITY)  /*Postprocess (t = INFINITY means target is not hittable)*/
      t = 0.;

   /* Position is calculated on where it should be */
   x = (target_pos->x + target_vel->x*t) - (pos->x + vel->x*t);
   y = (target_pos->y + target_vel->y*t) - (pos->y + vel->y*t);

   /* Compute both the angles we want. */
   if (pilot_target != NULL) {
      /* Lead angle is determined from ewarfare. */
      double trackmin = outfit_trackmin(outfit);
      double trackmax = outfit_trackmax(outfit);
      lead     = pilot_ewWeaponTrack( parent, pilot_target, trackmin, trackmax );
      x        = lead * x + (1.-lead) * rx;
      y        = lead * y + (1.-lead) * ry;
   }
   else
      lead     = 1.;
   rdir     = ANGLE(x,y);

   /* For unguided rockets: use a FD quasi-Newton algorithm to aim better. */
   if (outfit_isLauncher(outfit) && outfit->u.lau.thrust > 0.) {
      double vmin  = outfit->u.lau.speed;

      if (vmin > 0.) {
         /* Get various details. */
         double tt, ddir, dtdd, acc, pxv, ang, dvx, dvy;
         int niter = 5;
         acc = outfit->u.lau.thrust;

         /* Get the relative velocity. */
         dvx = lead * (target_vel->x - vel->x);
         dvy = lead * (target_vel->y - vel->y);

         /* Cross product between position and vel. */
         /* For having a better conditionning, ddir is adapted to the angular difference. */
         pxv = rx*dvy - ry*dvx;
         ang = atan2( pxv, rx*dvx+ry*dvy ); /* Angle between position and velocity. */
         if (fabs(ang + M_PI) < fabs(ang))
            ang += M_PI; /* Periodicity tricks. */
         else if (fabs(ang - M_PI) < fabs(ang))
            ang -= M_PI;
         ddir = -ang/1000.;

         /* Iterate to correct the initial guess rdir. */
         /* We compute more precisely ta and tt. */
         /* (times for the ammo and the target to get to intersection point) */
         /* The aim is to nullify ta-tt. */
         if (fabs(ang) > 1e-7) { /* No need to iterate if it's already nearly aligned. */
            for (int i=0; i<niter; i++) {
               double d  = weapon_computeTimes( rdir, rx, ry, dvx, dvy, pxv, vmin, acc, &tt );
               double dd = weapon_computeTimes( rdir+ddir, rx, ry, dvx, dvy, pxv, vmin, acc, &tt );

               /* Manage an exception (tt<0), and regular stopping condition. */
               /* TODO: this stopping criterion is too restrictive. */
               /* (for example when pos and vel are nearly aligned). */
               if (tt < 0. || fabs(d) < 5.)
                  break;

               dtdd = (dd-d)/ddir; /* Derivative of the criterion wrt. rdir. */
               rdir = rdir - d/dtdd; /* Update. */
            }
         }
      }
   }

   /* Calculate bounds. */
   off = angle_diff( rdir, dir );
   if (FABS(off) > swivel) {
      if (off > 0.)
         rdir = dir - swivel;
      else
         rdir = dir + swivel;
   }

   return rdir;
}

/**
 * @brief Gets the aim position of a turret weapon.
 *
 *    @param target_pos Target of the weapon.
 *    @param pos Position of the turret.
 *    @param dir Direction facing parent ship and turret.
 *    @param swivel Maximum angle between weapon and straight ahead.
 */
static double weapon_aimTurretStatic( const vec2 *target_pos, const vec2 *pos, double dir, double swivel )
{
   double rx, ry, rdir, off;
   /* Get the vector : shooter -> target */
   rx = target_pos->x - pos->x;
   ry = target_pos->y - pos->y;
   rdir = ANGLE(rx,ry);

   /* Calculate bounds. */
   off = angle_diff( rdir, dir );
   if (FABS(off) > swivel) {
      if (off > 0.)
         rdir = dir - swivel;
      else
         rdir = dir + swivel;
   }

   return rdir;
}

/**
 * @brief Computes precisely interception times for propelled weapons (rockets).
 *
 *    @param rdir tried shooting angle.
 *    @param rx Relative position x.
 *    @param ry Relative position y.
 *    @param dvx Relative velocity x.
 *    @param dvy Relative velocity y.
 *    @param pxv Cross product between relative position and relative velocity
 *    @param vmin min ammo velocity.
 *    @param acc ammo acceleration.
 *    @param[out] tt Time for the target to reach the interception point.
 */
static double weapon_computeTimes( double rdir, double rx, double ry, double dvx, double dvy, double pxv,
      double vmin, double acc, double *tt )
{
   double l, dxv, dxp, ct, st, d;

   /* Trigonometry. */
   ct = cos(rdir); st = sin(rdir);

   /* Two extra cross products. */
   dxv = ct*dvy - st*dvx;
   dxp = ct*ry  - st*rx;

   /* Compute criterion. */
   *tt = -dxp/dxv; /* Time to interception for target. Because triangle aera. */
   l = pxv/dxv; /* Length to interception for shooter. Because triangle aera. */
   d = .5*acc*(*tt)*(*tt) + vmin*(*tt); /* Estimate how far the projectile went. */

   return (d-l); /* Criterion is distance of projectile to intersection when target is there. */
}

/**
 * @brief Creates the bolt specific properties of a weapon.
 *
 *    @param w Weapon to create bolt specific properties of.
 *    @param outfit Outfit which spawned the weapon.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the slot (absolute).
 *    @param vel Velocity of the slot (absolute).
 *    @param parent Shooter.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim.
 */
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      double dir, const vec2* pos, const vec2* vel, const Pilot* parent, double time, int aim )
{
   vec2 v;
   double mass, rdir, acc, m;
   Pilot *pilot_target;
   const OutfitGFX *gfx;

   /* Only difference is the direction of fire */
   if ((w->parent!=w->target) && (w->target != 0)) /* Must have valid target */
      pilot_target = pilot_get(w->target);
   else /* fire straight or at asteroid */
      pilot_target = NULL;

   if (aim)
      rdir = weapon_aimTurret( outfit, parent, pilot_target, pos, vel, dir, outfit->u.blt.swivel, time );
   else {
      if (pilot_isPlayer(parent) && (SDL_ShowCursor(SDL_QUERY)==SDL_ENABLE)) {
         double x, y;
         vec2 target;
         gl_screenToGameCoords( &x, &y, player.mousex, player.mousey );
         vec2_csetmin( &target, x, y );
         rdir = weapon_aimTurretStatic( &target, pos, dir, outfit->u.blt.swivel );
      }
      else
         rdir = dir;
   }

   /* Disperse as necessary. */
   if (outfit->u.blt.dispersion > 0.)
      rdir += RNG_1SIGMA() * outfit->u.blt.dispersion;

   /* Calculate accuracy. */
   acc =  HEAT_WORST_ACCURACY * pilot_heatAccuracyMod( T );

   /* Stat modifiers. */
   if (outfit->type == OUTFIT_TYPE_TURRET_BOLT) {
      w->dam_mod *= parent->stats.tur_damage;
      /* dam_as_dis is computed as multiplier, must be corrected. */
      w->dam_as_dis_mod = parent->stats.tur_dam_as_dis-1.;
   }
   else {
      w->dam_mod *= parent->stats.fwd_damage;
      /* dam_as_dis is computed as multiplier, must be corrected. */
      w->dam_as_dis_mod = parent->stats.fwd_dam_as_dis-1.;
   }
   /* Clamping, but might not actually be necessary if weird things want to be done. */
   w->dam_as_dis_mod = CLAMP( 0., 1., w->dam_as_dis_mod );

   /* Calculate direction. */
   rdir += RNG_2SIGMA() * acc;
   if (rdir < 0.)
      rdir += 2.*M_PI;
   else if (rdir >= 2.*M_PI)
      rdir -= 2.*M_PI;

   mass = 1.; /* Lasers are presumed to have unitary mass, just like the real world. */
   v = *vel;
   m = outfit->u.blt.speed;
   if (outfit->u.blt.speed_dispersion > 0.)
      m += RNG_1SIGMA() * outfit->u.blt.speed_dispersion;
   vec2_cadd( &v, m*cos(rdir), m*sin(rdir));
   w->timer = outfit->u.blt.range / outfit->u.blt.speed;
   w->falloff = w->timer - outfit->u.blt.falloff / outfit->u.blt.speed;
   solid_init( &w->solid, mass, rdir, pos, &v, SOLID_UPDATE_EULER );
   w->voice = sound_playPos( w->outfit->u.blt.sound,
         w->solid.pos.x, w->solid.pos.y,
         w->solid.vel.x, w->solid.vel.y );

   /* Set facing direction. */
   gfx = outfit_gfx( w->outfit );
   if (gfx->tex != NULL)
      gl_getSpriteFromDir( &w->sx, &w->sy, gfx->tex, w->solid.dir );
}

/**
 * @brief Creates the ammo specific properties of a weapon.
 *
 *    @param w Weapon to create ammo specific properties of.
 *    @param outfit Outfit which spawned the weapon.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the slot (absolute).
 *    @param vel Velocity of the slot (absolute).
 *    @param parent Shooter.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim.
 */
static void weapon_createAmmo( Weapon *w, const Outfit* outfit, double T,
      double dir, const vec2* pos, const vec2* vel, const Pilot* parent, double time, int aim )
{
   (void) T;
   vec2 v;
   double mass, rdir, m;
   Pilot *pilot_target;
   const OutfitGFX *gfx;

   /* Only difference is the direction of fire */
   if ((w->parent!=w->target) && (w->target != 0)) /* Must have valid target */
      pilot_target = pilot_get(w->target);
   else /* fire straight or at asteroid */
      pilot_target = NULL;

   if (aim) {
      if (outfit->type == OUTFIT_TYPE_TURRET_LAUNCHER)
         rdir = weapon_aimTurret( outfit, parent, pilot_target, pos, vel, dir, M_PI, time );
      else if (outfit->u.lau.swivel > 0.)
         rdir = weapon_aimTurret( outfit, parent, pilot_target, pos, vel, dir, outfit->u.lau.swivel, time );
      else
         rdir = dir;
   }
   else {
      if ((outfit->u.lau.swivel > 0.) && pilot_isPlayer(parent) && (SDL_ShowCursor(SDL_QUERY)==SDL_ENABLE)) {
         double x, y;
         vec2 target;
         gl_screenToGameCoords( &x, &y, player.mousex, player.mousey );
         vec2_csetmin( &target, x, y );
         rdir = weapon_aimTurretStatic( &target, pos, dir, outfit->u.blt.swivel );
      }
      else
         rdir = dir;
   }

   /* Disperse as necessary. */
   if (outfit->u.blt.dispersion > 0.)
      rdir += RNG_1SIGMA() * outfit->u.lau.dispersion;

   /* Make sure angle is in range. */
   while (rdir < 0.)
      rdir += 2.*M_PI;
   while (rdir >= 2.*M_PI)
      rdir -= 2.*M_PI;

   /* Launcher damage. */
   w->dam_mod *= parent->stats.launch_damage;

   /* If thrust is 0. we assume it starts out at speed. */
   v = *vel;
   m = outfit->u.lau.speed;
   if (outfit->u.lau.speed_dispersion > 0.)
      m += RNG_1SIGMA() * outfit->u.lau.speed_dispersion;
   vec2_cadd( &v, m * cos(rdir), m * sin(rdir) );
   w->real_vel = VMOD(v);

   /* Set up ammo details. */
   mass        = w->outfit->u.lau.ammo_mass;
   w->timer    = w->outfit->u.lau.duration * parent->stats.launch_range;
   solid_init( &w->solid, mass, rdir, pos, &v, SOLID_UPDATE_EULER );
   if (w->outfit->u.lau.thrust > 0.) {
      weapon_setThrust( w, w->outfit->u.lau.thrust * mass );
      /* Limit speed, we only relativize in the case it has thrust + initial speed. */
      w->solid.speed_max = w->outfit->u.lau.speed_max;
      if (w->outfit->u.lau.speed > 0.)
         w->solid.speed_max = -1; /* No limit. */
   }

   /* Handle seekers. */
   if (w->outfit->u.lau.ai != AMMO_AI_UNGUIDED) {
      w->timer2   = outfit->u.lau.iflockon * parent->stats.launch_calibration;
      w->paramf   = outfit->u.lau.iflockon * parent->stats.launch_calibration;
      w->status   = (w->timer2 > 0.) ? WEAPON_STATUS_LOCKING : WEAPON_STATUS_OK;

      w->think = think_seeker; /* AI is the same atm. */
      w->r     = RNGF(); /* Used for jamming. */

      /* If they are seeking a pilot, increment lockon counter. */
      if (pilot_target == NULL)
         pilot_target = pilot_get(w->target);
      if (pilot_target != NULL)
         pilot_target->lockons++;
   }
   else
      w->status = WEAPON_STATUS_OK;

   /* Play sound. */
   w->voice = sound_playPos( w->outfit->u.lau.sound,
         w->solid.pos.x, w->solid.pos.y,
         w->solid.vel.x, w->solid.vel.y );

   /* Set facing direction. */
   gfx = outfit_gfx( w->outfit );
   if (gfx->tex != NULL)
      gl_getSpriteFromDir( &w->sx, &w->sy, gfx->tex, w->solid.dir );

   /* Set up trails. */
   if (w->outfit->u.lau.trail_spec != NULL)
      w->trail = spfx_trail_create( w->outfit->u.lau.trail_spec );
}

/**
 * @brief Creates a new weapon.
 *
 *    @param po Outfit slot which spawned the weapon.
 *    @param ref Reference outfit to use, does not have to be the outfit in the slot, but will default to it if set to NULL.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the slot (absolute).
 *    @param vel Velocity of the slot (absolute).
 *    @param parent Shooter.
 *    @param target Target ID of the shooter.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim.
 *    @return A pointer to the newly created weapon.
 */
static Weapon* weapon_create( PilotOutfitSlot* po, const Outfit *ref,
      double T, double dir, const vec2* pos, const vec2* vel,
      const Pilot* parent, const unsigned int target, double time, int aim )
{
   double mass, rdir;
   Pilot *pilot_target;
   AsteroidAnchor *field;
   Asteroid *ast;
   Weapon* w;
   const Outfit *outfit = (ref==NULL) ? po->outfit : ref;

   /* Create basic features */
   w           = calloc( 1, sizeof(Weapon) );
   w->dam_mod  = 1.; /* Default of 100% damage. */
   w->dam_as_dis_mod = 0.; /* Default of 0% damage to disable. */
   w->faction  = parent->faction; /* non-changeable */
   w->parent   = parent->id; /* non-changeable */
   w->target   = target; /* non-changeable */
   w->lua_mem  = LUA_NOREF;
   if (po != NULL && po->lua_mem != LUA_NOREF) {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->lua_mem); /* mem */
      w->lua_mem = luaL_ref( naevL, LUA_REGISTRYINDEX );
   }
   w->outfit   = outfit; /* non-changeable */
   w->strength = 1.;
   w->r        = RNGF(); /* Set unique value. */

   /* Inform the target. */
   if (!(outfit_isBeam(w->outfit))) {
      pilot_target = pilot_get(target);
      if (pilot_target != NULL)
         pilot_target->projectiles++;
   }

   switch (outfit->type) {

      /* Bolts treated together */
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         weapon_createBolt( w, outfit, T, dir, pos, vel, parent, time, aim );
         break;

      /* Beam weapons are treated together. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         rdir = dir;
         if (aim && (outfit->type == OUTFIT_TYPE_TURRET_BEAM)) {
            pilot_target = pilot_get(target);
            if ((w->parent != w->target) && (pilot_target != NULL))
               rdir = vec2_angle(pos, &pilot_target->solid.pos);
            else if (parent->nav_asteroid >= 0) {
               field = &cur_system->asteroids[parent->nav_anchor];
               ast = &field->asteroids[parent->nav_asteroid];
               rdir = vec2_angle(pos, &ast->pos);
            }
         }

         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;
         mass = 1.; /**< Needs a mass. */
         solid_init( &w->solid, mass, rdir, pos, vel, SOLID_UPDATE_EULER );
         w->think = think_beam;
         w->timer = outfit->u.bem.duration;
         w->voice = sound_playPos( w->outfit->u.bem.sound,
               w->solid.pos.x, w->solid.pos.y,
               w->solid.vel.x, w->solid.vel.y );

         if (outfit->type == OUTFIT_TYPE_BEAM) {
            w->dam_mod       *= parent->stats.fwd_damage;
            w->dam_as_dis_mod = parent->stats.fwd_dam_as_dis-1.;
         }
         else {
            w->dam_mod       *= parent->stats.tur_damage;
            w->dam_as_dis_mod = parent->stats.tur_dam_as_dis-1.;
         }
         w->dam_as_dis_mod = CLAMP( 0., 1., w->dam_as_dis_mod );

         break;

      /* Treat seekers together. */
      case OUTFIT_TYPE_LAUNCHER:
      case OUTFIT_TYPE_TURRET_LAUNCHER:
         weapon_createAmmo( w, outfit, T, dir, pos, vel, parent, time, aim );
         break;

      /* just dump it where the player is */
      default:
         WARN(_("Weapon of type '%s' has no create implemented yet!"),
               w->outfit->name);
         solid_init( &w->solid, 1., dir, pos, vel, SOLID_UPDATE_EULER );
         break;
   }

   /* Set life to timer. */
   w->life = w->timer;

   return w;
}

/**
 * @brief Creates a new weapon.
 *
 *    @param po Outfit slot which spawns the weapon.
 *    @param ref Reference outfit to use for computing damage and properties.
 *    @param T Temperature of the shooter.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the slot (absolute).
 *    @param vel Velocity of the slot (absolute).
 *    @param parent Pilot ID of the shooter.
 *    @param target Target ID that is getting shot.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim.
 */
void weapon_add( PilotOutfitSlot *po, const Outfit *ref,
      double T, double dir,
      const vec2* pos, const vec2* vel,
      const Pilot *parent, unsigned int target, double time, int aim )
{
   WeaponLayer layer;
   Weapon *w, **m;
   size_t bufsize;
   const Outfit *o = (ref==NULL) ? po->outfit : ref;

#if DEBUGGING
   if (!outfit_isBolt(o) &&
         !outfit_isLauncher(o)) {
      ERR(_("Trying to create a Weapon from a non-Weapon type Outfit"));
      return;
   }
#endif /* DEBUGGING */

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w     = weapon_create( po, ref, T, dir, pos, vel, parent, target, time, aim );

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         m = &array_grow(&wbackLayer);
         break;
      case WEAPON_LAYER_FG:
         m = &array_grow(&wfrontLayer);
         break;

      default:
         WARN(_("Unknown weapon layer!"));
         return;
   }
   *m = w;

   /* Grow the vertex stuff if needed. */
   bufsize = array_reserved(wfrontLayer) + array_reserved(wbackLayer);
   if (bufsize != weapon_vboSize) {
      GLsizei size;
      weapon_vboSize = bufsize;
      size = sizeof(GLfloat) * (2+4) * weapon_vboSize;
      weapon_vboData = realloc( weapon_vboData, size );
      if (weapon_vbo == NULL)
         weapon_vbo = gl_vboCreateStream( size, NULL );
      gl_vboData( weapon_vbo, size, weapon_vboData );
   }
}

/**
 * @brief Starts a beam weapon.
 *
 *    @param po Outfit slot which spawns the weapon.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the slot (absolute).
 *    @param vel Velocity of the slot (absolute).
 *    @param parent Pilot shooter.
 *    @param target Target ID that is getting shot.
 *    @param aim Whether or not to aim.
 *    @return The identifier of the beam weapon.
 *
 * @sa beam_end
 */
unsigned int beam_start( PilotOutfitSlot *po,
      double dir, const vec2* pos, const vec2* vel,
      const Pilot *parent, const unsigned int target, int aim )
{
   WeaponLayer layer;
   Weapon *w, **m;
   GLsizei size;
   size_t bufsize;

   if (!outfit_isBeam(po->outfit)) {
      ERR(_("Trying to create a Beam Weapon from a non-beam outfit."));
      return -1;
   }

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w = weapon_create( po, NULL, 0., dir, pos, vel, parent, target, 0., aim );
   w->ID = ++beam_idgen;
   w->mount = po;
   w->timer2 = 0.;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         m = &array_grow(&wbackLayer);
         break;
      case WEAPON_LAYER_FG:
         m = &array_grow(&wfrontLayer);
         break;

      default:
         ERR(_("Invalid WEAPON_LAYER specified"));
         return -1;
   }
   *m = w;

   /* Grow the vertex stuff if needed. */
   bufsize = array_reserved(wfrontLayer) + array_reserved(wbackLayer);
   if (bufsize != weapon_vboSize) {
      weapon_vboSize = bufsize;
      size = sizeof(GLfloat) * (2+4) * weapon_vboSize;
      weapon_vboData = realloc( weapon_vboData, size );
      if (weapon_vbo == NULL)
         weapon_vbo = gl_vboCreateStream( size, NULL );
      gl_vboData( weapon_vbo, size, weapon_vboData );
   }

   return w->ID;
}

/**
 * @brief Ends a beam weapon.
 *
 *    @param parent ID of the parent of the beam.
 *    @param beam ID of the beam to destroy.
 */
void beam_end( const unsigned int parent, unsigned int beam )
{
   WeaponLayer layer;
   Weapon **curLayer;

   layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         break;

      default:
         ERR(_("Invalid WEAPON_LAYER specified"));
         return;
   }

#if DEBUGGING
   if (beam==0) {
      WARN(_("Trying to remove beam with ID 0!"));
      return;
   }
#endif /* DEBUGGING */

   /* Now try to destroy the beam. */
   for (int i=0; i<array_size(curLayer); i++) {
      if (curLayer[i]->ID == beam) { /* Found it. */
         weapon_miss(curLayer[i]);
         break;
      }
   }
}

/**
 * @brief Destroys a weapon.
 *
 *    @param w Weapon to destroy.
 */
static void weapon_destroy( Weapon* w )
{
   /* Just mark for removal. */
   weapon_setFlag( w, WEAPON_FLAG_DESTROYED );
}

/**
 * @brief Frees the weapon.
 *
 *    @param w Weapon to free.
 */
static void weapon_free( Weapon* w )
{
   Pilot *pilot_target = pilot_get( w->target );

   /* Stop playing sound if beam weapon. */
   if (outfit_isBeam(w->outfit)) {
      sound_stop( w->voice );
      sound_playPos(w->outfit->u.bem.sound_off,
            w->solid.pos.x, w->solid.pos.y,
            w->solid.vel.x, w->solid.vel.y );
   }
   else {
      /* Decrement target lockons if needed */
      if (pilot_target != NULL) {
         pilot_target->projectiles--;
         if (outfit_isSeeker(w->outfit))
            pilot_target->lockons--;
      }
   }

   /* Free the solid. */
   //solid_free(w->solid);

   /* Free the trail, if any. */
   spfx_trail_remove(w->trail);

   /* Free the Lua ref, if any. */
   luaL_unref( naevL, LUA_REGISTRYINDEX, w->lua_mem );

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
   /* Don't forget to stop the sounds. */
   for (int i=0; i < array_size(wbackLayer); i++) {
      sound_stop(wbackLayer[i]->voice);
      weapon_free(wbackLayer[i]);
   }
   array_erase( &wbackLayer, array_begin(wbackLayer), array_end(wbackLayer) );
   for (int i=0; i < array_size(wfrontLayer); i++) {
      sound_stop(wfrontLayer[i]->voice);
      weapon_free(wfrontLayer[i]);
   }
   array_erase( &wfrontLayer, array_begin(wfrontLayer), array_end(wfrontLayer) );
}

/**
 * @brief Destroys all the weapons and frees it all.
 */
void weapon_exit (void)
{
   weapon_clear();

   /* Destroy front layer. */
   array_free(wbackLayer);

   /* Destroy back layer. */
   array_free(wfrontLayer);

   /* Destroy VBO. */
   free( weapon_vboData );
   weapon_vboData = NULL;
   gl_vboDestroy( weapon_vbo );
   weapon_vbo = NULL;

   /* Clean up the queries. */
   il_destroy( &weapon_qtquery );
   il_destroy( &weapon_qtexp );
}

/**
 * @brief Clears possible exploded weapons.
 */
void weapon_explode( double x, double y, double radius,
      int dtype, double damage,
      const Pilot *parent, int mode )
{
   (void) dtype;
   (void) damage;
   weapon_explodeLayer( WEAPON_LAYER_FG, x, y, radius, parent, mode );
   weapon_explodeLayer( WEAPON_LAYER_BG, x, y, radius, parent, mode );
}

/**
 * @brief Explodes all the things on a layer.
 */
static void weapon_explodeLayer( WeaponLayer layer,
      double x, double y, double radius,
      const Pilot *parent, int mode )
{
   (void)parent;
   Weapon **curLayer;
   double rad2;

   /* set the proper layer */
   switch (layer) {
      case WEAPON_LAYER_BG:
         curLayer = wbackLayer;
         break;
      case WEAPON_LAYER_FG:
         curLayer = wfrontLayer;
         break;

      default:
         ERR(_("Invalid WEAPON_LAYER specified"));
         return;
   }

   rad2 = radius*radius;

   /* Now try to destroy the weapons affected. */
   for (int i=0; i<array_size(curLayer); i++) {
      Weapon *w = curLayer[i];
      if (((mode & EXPL_MODE_MISSILE) && outfit_isLauncher(w->outfit)) ||
            ((mode & EXPL_MODE_BOLT) && outfit_isBolt(w->outfit))) {

         double dist = pow2(w->solid.pos.x - x) + pow2(w->solid.pos.y - y);

         if (dist < rad2)
            weapon_destroy( w );
      }
   }
}
