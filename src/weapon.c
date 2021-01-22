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
#include "opengl.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "spfx.h"


#define weapon_isSmart(w)     (w->think != NULL) /**< Checks if the weapon w is smart. */

#define WEAPON_CHUNK_MAX      16384 /**< Maximum size to increase array with */
#define WEAPON_CHUNK_MIN      256 /**< Minimum size to increase array with */

/* Weapon status */
#define WEAPON_STATUS_OK         0 /**< Weapon is fine */
#define WEAPON_STATUS_JAMMED     1 /**< Got jammed */
#define WEAPON_STATUS_UNJAMMED   2 /**< Survived jamming */


/*
 * pilot stuff
 */
extern Pilot** pilot_stack;


/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   Solid *solid; /**< Actually has its own solid :) */
   unsigned int ID; /**< Only used for beam weapons. */

   int faction; /**< faction of pilot that shot it */
   unsigned int parent; /**< pilot that shot it */
   unsigned int target; /**< target to hit, only used by seeking things */
   const Outfit* outfit; /**< related outfit that fired it or whatnot */

   double real_vel; /**< Keeps track of the real velocity. */
   double dam_mod; /**< Damage modifier. */
   double dam_as_dis_mod; /**< Damage as disable modifier. */
   int voice; /**< Weapon's voice. */
   double exp_timer; /**< Explosion timer for beams. */
   double life; /**< Total life. */
   double timer; /**< mainly used to see when the weapon was fired */
   double anim; /**< Used for beam weapon graphics and others. */
   int sprite; /**< Used for spinning outfits. */
   PilotOutfitSlot *mount; /**< Used for beam weapons. */
   double falloff; /**< Point at which damage falls off. */
   double strength; /**< Calculated with falloff. */
   int sx; /**< Current X sprite to use. */
   int sy; /**< Current Y sprite to use. */

   /* position update and render */
   void (*update)(struct Weapon_*, const double, WeaponLayer); /**< Updates the weapon */
   void (*think)(struct Weapon_*, const double); /**< for the smart missiles */

   char status; /**< Weapon status - to check for jamming */
} Weapon;


/* behind player layer */
static Weapon** wbackLayer = NULL; /**< behind pilots */
/* behind player layer */
static Weapon** wfrontLayer = NULL; /**< in front of pilots, behind player */

/* Graphics. */
static gl_vbo  *weapon_vbo     = NULL; /**< Weapon VBO. */
static GLfloat *weapon_vboData = NULL; /**< Data of weapon VBO. */
static size_t weapon_vboSize   = 0; /**< Size of the VBO. */


/* Internal stuff. */
static unsigned int beam_idgen = 0; /**< Beam identifier generator. */


/*
 * Prototypes
 */
/* Creation. */
static double weapon_aimTurret( const Outfit *outfit, const Pilot *parent,
      const Pilot *pilot_target, const Vector2d *pos, const Vector2d *vel, double dir,
      double swivel, double time );
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent, double time );
static void weapon_createAmmo( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent, double time );
static Weapon* weapon_create( const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, const unsigned int target, double time );
/* Updating. */
static void weapon_render( Weapon* w, const double dt );
static void weapons_updateLayer( const double dt, const WeaponLayer layer );
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer );
/* Destruction. */
static void weapon_destroy( Weapon* w, WeaponLayer layer );
static void weapon_free( Weapon* w );
static void weapon_explodeLayer( WeaponLayer layer,
      double x, double y, double radius,
      const Pilot *parent, int mode );
/* Hitting. */
static int weapon_checkCanHit( Weapon* w, Pilot *p );
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos );
static void weapon_hitAst( Weapon* w, Asteroid* a, WeaponLayer layer, Vector2d* pos );
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      Vector2d pos[2], const double dt );
static void weapon_hitAstBeam( Weapon* w, Asteroid* a, WeaponLayer layer,
      Vector2d pos[2], const double dt );
/* think */
static void think_seeker( Weapon* w, const double dt );
static void think_beam( Weapon* w, const double dt );
/* externed */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape, double alpha );
/* movement. */
static void weapon_setThrust( Weapon *w, double thrust );
static void weapon_setTurn( Weapon *w, double turn );


void weapon_init (void)
{
   wfrontLayer = array_create(Weapon*);
   wbackLayer  = array_create(Weapon*);
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
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape, double alpha )
{
   int i, rc, p;
   double x, y;
   Weapon *wp;
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
   for (i=0; i<array_size(wbackLayer); i++) {
      wp = wbackLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player.p, wp->solid->pos.x, wp->solid->pos.y ))
         continue;

      /* Get radar position. */
      x = (wp->solid->pos.x - player.p->solid->pos.x) / res;
      y = (wp->solid->pos.y - player.p->solid->pos.y) / res;

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
   for (i=0; i<array_size(wfrontLayer); i++) {
      wp = wfrontLayer[i];

      /* Make sure is in range. */
      if (!pilot_inRange( player.p, wp->solid->pos.x, wp->solid->pos.y ))
         continue;

      /* Get radar position. */
      x = (wp->solid->pos.x - player.p->solid->pos.x) / res;
      y = (wp->solid->pos.y - player.p->solid->pos.y) / res;

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

      gl_beginSmoothProgram(gl_view_matrix);
      gl_vboActivateAttribOffset( weapon_vbo, shaders.smooth.vertex, 0, 2, GL_FLOAT, 0 );
      gl_vboActivateAttribOffset( weapon_vbo, shaders.smooth.vertex_color, offset * sizeof(GLfloat), 4, GL_FLOAT, 0 );
      glDrawArrays( GL_POINTS, 0, p );
      gl_endSmoothProgram();
   }
}


/**
 * @brief Sets the weapon's thrust.
 */
static void weapon_setThrust( Weapon *w, double thrust )
{
   w->solid->thrust = thrust;
}


/**
 * @brief Sets the weapon's turn.
 */
static void weapon_setTurn( Weapon *w, double turn )
{
   w->solid->dir_vel = turn;
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
   Pilot *p;
   Vector2d v;
   double t, turn_max;
   double ewtrack;

   if (w->target == w->parent)
      return; /* no self shooting */

   p = pilot_get(w->target); /* no null pilot */
   if (p==NULL) {
      weapon_setThrust( w, 0. );
      weapon_setTurn( w, 0. );
      return;
   }

   ewtrack = pilot_ewWeaponTrack( pilot_get(w->parent), p, w->outfit->u.amm.resist );

   /* Handle by status. */
   switch (w->status) {

      case WEAPON_STATUS_OK: /* Check to see if can get jammed */
      /* Purpose fallthrough */
      case WEAPON_STATUS_UNJAMMED: /* Work as expected */

         /* Smart seekers take into account ship velocity. */
         if (w->outfit->u.amm.ai == AMMO_AI_SMART) {

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
         /* Other seekers are simplistic. */
         else {
            diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                  vect_angle(&w->solid->pos, &p->solid->pos));
         }

         /* Set turn. */
         turn_max = w->outfit->u.amm.turn * ewtrack;
         weapon_setTurn( w, CLAMP( -turn_max, turn_max,
                  10 * diff * w->outfit->u.amm.turn ));
         break;

      case WEAPON_STATUS_JAMMED: /* Continue doing whatever */
         /* Do nothing, dir_vel should be set already if needed */
         break;

      default:
         WARN(_("Unknown weapon status for '%s'"), w->outfit->name);
         break;
   }

   /* Limit speed here */
   w->real_vel = MIN( w->outfit->u.amm.speed, w->real_vel + w->outfit->u.amm.thrust*dt );
   vect_pset( &w->solid->vel, /* ewtrack * */ w->real_vel, w->solid->dir );

   /* Modulate max speed. */
   //w->solid->speed_max = w->outfit->u.amm.speed * ewtrack;
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
   AsteroidAnchor *field;
   Asteroid *ast;
   double diff;
   Vector2d v;

   /* Get pilot, if pilot is dead beam is destroyed. */
   p = pilot_get(w->parent);
   if (p == NULL) {
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
   w->solid->pos.x = p->solid->pos.x + v.x;
   w->solid->pos.y = p->solid->pos.y + v.y;

   /* Handle aiming. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BEAM:
         w->solid->dir = p->solid->dir;
         break;

      case OUTFIT_TYPE_TURRET_BEAM:
         /* Get target, if target is dead beam stops moving. Targeting
          * self is invalid so in that case we ignore the target.
          */
         t = (w->target != w->parent) ? pilot_get(w->target) : NULL;
         if (t == NULL) {
            if (p->nav_asteroid >= 0) {
               field = &cur_system->asteroids[p->nav_anchor];
               ast = &field->asteroids[p->nav_asteroid];

               diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                     vect_angle(&w->solid->pos, &ast->pos));
            }
            else
               diff = angle_diff(w->solid->dir, p->solid->dir);
         }
         else
            diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                  vect_angle(&w->solid->pos, &t->solid->pos));

         weapon_setTurn( w, CLAMP( -w->outfit->u.bem.turn, w->outfit->u.bem.turn,
                  10 * diff *  w->outfit->u.bem.turn ));
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
   Weapon *w;
   int i;
   int spfx;
   int s;
   Pilot *p;

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

   i = 0;
   while (i < array_size(wlayer)) {
      w = wlayer[i];

      switch (w->outfit->type) {

         /* most missiles behave the same */
         case OUTFIT_TYPE_AMMO:

            w->timer -= dt;
            if (w->timer < 0.) {
               spfx = -1;
               /* See if we need armour death sprite. */
               if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR))
                  spfx = outfit_spfxArmour(w->outfit);
               /* See if we need shield death sprite. */
               else if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_SHIELD))
                  spfx = outfit_spfxShield(w->outfit);
               /* Add death sprite if needed. */
               if (spfx != -1) {
                  spfx_add( spfx, w->solid->pos.x, w->solid->pos.y,
                        w->solid->vel.x, w->solid->vel.y,
                        SPFX_LAYER_BACK ); /* presume back. */
                  /* Add sound if explodes and has it. */
                  s = outfit_soundHit(w->outfit);
                  if (s != -1)
                     w->voice = sound_playPos(s,
                           w->solid->pos.x,
                           w->solid->pos.y,
                           w->solid->vel.x,
                           w->solid->vel.y);
               }
               weapon_destroy(w,layer);
               break;
            }
            break;

         case OUTFIT_TYPE_BOLT:
         case OUTFIT_TYPE_TURRET_BOLT:
            w->timer -= dt;
            if (w->timer < 0.) {
               spfx = -1;
               /* See if we need armour death sprite. */
               if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR))
                  spfx = outfit_spfxArmour(w->outfit);
               /* See if we need shield death sprite. */
               else if (outfit_isProp(w->outfit, OUTFIT_PROP_WEAP_BLOWUP_SHIELD))
                  spfx = outfit_spfxShield(w->outfit);
               /* Add death sprite if needed. */
               if (spfx != -1) {
                  spfx_add( spfx, w->solid->pos.x, w->solid->pos.y,
                        w->solid->vel.x, w->solid->vel.y,
                        SPFX_LAYER_BACK ); /* presume back. */
                  /* Add sound if explodes and has it. */
                  s = outfit_soundHit(w->outfit);
                  if (s != -1)
                     w->voice = sound_playPos(s,
                           w->solid->pos.x,
                           w->solid->pos.y,
                           w->solid->vel.x,
                           w->solid->vel.y);
               }
               weapon_destroy(w,layer);
               break;
            }
            else if (w->timer < w->falloff)
               w->strength = w->timer / w->falloff;
            break;

         /* Beam weapons handled a part. */
         case OUTFIT_TYPE_BEAM:
         case OUTFIT_TYPE_TURRET_BEAM:
            w->timer -= dt;
            if (w->timer < 0. || (w->outfit->u.bem.min_duration > 0. &&
                  w->mount->stimer < 0.)) {
               p = pilot_get(w->parent);
               if (p != NULL)
                  pilot_stopBeam(p, w->mount);
               weapon_destroy(w,layer);
               break;
            }
            /* We use the explosion timer to tell when we have to create explosions. */
            w->exp_timer -= dt;
            if (w->exp_timer < 0.) {
               if (w->exp_timer < -1.)
                  w->exp_timer = 0.100;
               else
                  w->exp_timer = -1.;
            }
            break;
         default:
            WARN(_("Weapon of type '%s' has no update implemented yet!"),
                  w->outfit->name);
            break;
      }

      /* Out of bounds, loop is over. */
      if (i >= array_size(wlayer))
         break;

      /* Only increment if weapon wasn't deleted. */
      if (w == wlayer[i]) {
         weapon_update(w,dt,layer);
         if ((i < array_size(wlayer)) && (w == wlayer[i]))
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
   int i;

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

   for (i=0; i<array_size(wlayer); i++)
      weapon_render( wlayer[i], dt );
}


static void weapon_renderBeam( Weapon* w, const double dt ) {
   double x, y, z, cx, cy, gx, gy;
   glTexture *gfx;
   gl_Matrix4 projection, tex_mat;

   /* Load GLSL program */
   glUseProgram(shaders.beam.program);

   gfx = outfit_gfx(w->outfit);

   /* Zoom. */
   z = cam_getZoom();

   /* Position. */
   cam_getPos( &cx, &cy );
   gui_getOffset( &gx, &gy );
   x = (w->solid->pos.x - cx)*z + gx;
   y = (w->solid->pos.y - cy)*z + gy;

   projection = gl_Matrix4_Translate( gl_view_matrix, SCREEN_W/2.+x, SCREEN_H/2.+y, 0. );
   projection = gl_Matrix4_Rotate2d( projection, w->solid->dir );
   projection = gl_Matrix4_Scale( projection, w->outfit->u.bem.range*z,gfx->sh * z, 1 );

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, gfx->texture);

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.beam.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.beam.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set the texture. */
   tex_mat = gl_Matrix4_Identity();
   tex_mat = gl_Matrix4_Translate(tex_mat, w->anim, 0, 0);
   tex_mat = gl_Matrix4_Scale(tex_mat, w->outfit->u.bem.range / gfx->sw, 1, 1);

   /* Set shader uniforms. */
   gl_Matrix4_Uniform(shaders.beam.projection, projection);
   gl_Matrix4_Uniform(shaders.beam.tex_mat, tex_mat);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.beam.vertex );
   glUseProgram(0);

   /* anything failed? */
   gl_checkErr();

   /* Do the beam movement. */
   gfx = outfit_gfx(w->outfit);
   w->anim -= 5. * dt;
   if (w->anim <= -gfx->sw)
      w->anim += gfx->sw;
}


/**
 * @brief Renders an individual weapon.
 *
 *    @param w Weapon to render.
 *    @param dt Current delta tick.
 */
static void weapon_render( Weapon* w, const double dt )
{
   glTexture *gfx;
   glColour c = { .r=1., .g=1., .b=1. };

   switch (w->outfit->type) {
      /* Weapons that use sprites. */
      case OUTFIT_TYPE_AMMO:
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         gfx = outfit_gfx(w->outfit);

         /* Alpha based on strength. */
         c.a = w->strength;

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
            if (outfit_isBolt(w->outfit) && w->outfit->u.blt.gfx_end)
               gl_blitSpriteInterpolate( gfx, w->outfit->u.blt.gfx_end,
                     w->timer / w->life,
                     w->solid->pos.x, w->solid->pos.y,
                     w->sprite % (int)gfx->sx, w->sprite / (int)gfx->sx, &c );
            else
               gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y,
                     w->sprite % (int)gfx->sx, w->sprite / (int)gfx->sx, &c );
         }
         /* Outfit faces direction. */
         else {
            if (outfit_isBolt(w->outfit) && w->outfit->u.blt.gfx_end)
               gl_blitSpriteInterpolate( gfx, w->outfit->u.blt.gfx_end,
                     w->timer / w->life,
                     w->solid->pos.x, w->solid->pos.y, w->sx, w->sy, &c );
            else
               gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y, w->sx, w->sy, &c );
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
static int weapon_checkCanHit( Weapon* w, Pilot *p )
{
   Pilot *parent;

   /* Can't hit invincible stuff. */
   if (pilot_isFlag(p, PILOT_INVINCIBLE))
      return 0;

   /* Can't hit invisible stuff. */
   if (pilot_isFlag(p, PILOT_INVISIBLE))
      return 0;

   /* Can never hit same faction. */
   if (p->faction == w->faction)
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
      parent = pilot_get(w->parent);
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
 * @brief Updates an individual weapon.
 *
 *    @param w Weapon to update.
 *    @param dt Current delta tick.
 *    @param layer Layer to which the weapon belongs.
 */
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer )
{
   int i, j, b, psx, psy, k, n;
   unsigned int coll, usePoly=1;
   glTexture *gfx;
   CollPoly *plg, *polygon;
   Vector2d crash[2];
   Pilot *p;
   AsteroidAnchor *ast;
   Asteroid *a;
   AsteroidType *at;

   gfx = NULL;
   polygon = NULL;

   /* Get the sprite direction to speed up calculations. */
   b     = outfit_isBeam(w->outfit);
   if (!b) {
      gfx = outfit_gfx(w->outfit);
      gl_getSpriteFromDir( &w->sx, &w->sy, gfx, w->solid->dir );
      n = gfx->sx * w->sy + w->sx;
      plg = outfit_plg(w->outfit);
      polygon = &plg[n];

      /* See if the outfit has a collision polygon. */
      if (outfit_isBolt(w->outfit)) {
         if (w->outfit->u.blt.npolygon == 0)
            usePoly = 0;
      }
      else if (outfit_isAmmo(w->outfit)) {
         if (w->outfit->u.amm.npolygon == 0)
            usePoly = 0;
      }
   }

   for (i=0; i<array_size(pilot_stack); i++) {
      p = pilot_stack[i];

      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      /* See if the ship has a collision polygon. */
      if (p->ship->npolygon == 0)
         usePoly = 0;

      /* Beam weapons have special collisions. */
      if (b) {
         /* Check for collision. */
         if (weapon_checkCanHit(w,p)) {
            if (usePoly) {
               k = p->ship->gfx_space->sx * psy + psx;
               coll = CollideLinePolygon( &w->solid->pos, w->solid->dir,
                     w->outfit->u.bem.range, &p->ship->polygon[k],
                     &p->solid->pos, crash);
            }
            else {
               coll = CollideLineSprite( &w->solid->pos, w->solid->dir,
                     w->outfit->u.bem.range, p->ship->gfx_space, psx, psy,
                     &p->solid->pos, crash);
            }
            if (coll)
               weapon_hitBeam( w, p, layer, crash, dt );
               /* No return because beam can still think, it's not
                * destroyed like the other weapons.*/
         }
      }
      /* smart weapons only collide with their target */
      else if (weapon_isSmart(w)) {

         if ( (pilot_stack[i]->id == w->target) &&
               (w->status == WEAPON_STATUS_OK) &&
               weapon_checkCanHit(w,p) ) {
            if (usePoly) {
               k = p->ship->gfx_space->sx * psy + psx;
               coll = CollidePolygon( &p->ship->polygon[k], &p->solid->pos,
                        polygon, &w->solid->pos, &crash[0] );
            }
            else {
               coll = CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
                        p->ship->gfx_space, psx, psy,
                        &p->solid->pos, &crash[0] );
            }
            if (coll) {
               weapon_hit( w, p, layer, &crash[0] );
               return; /* Weapon is destroyed. */
            }
         }
      }
      /* unguided weapons hit anything not of the same faction */
      else {
         if (weapon_checkCanHit(w,p)) {
            if (usePoly) {
               k = p->ship->gfx_space->sx * psy + psx;
               coll = CollidePolygon( &p->ship->polygon[k], &p->solid->pos,
                        polygon, &w->solid->pos, &crash[0] );
            }
            else {
               coll = CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
                        p->ship->gfx_space, psx, psy,
                        &p->solid->pos, &crash[0] );
            }

            if (coll) {
            weapon_hit( w, p, layer, &crash[0] );
            return; /* Weapon is destroyed. */
            }
         }
      }
   }

   /* Collide with asteroids*/
   if (outfit_isAmmo(w->outfit)) {
      for (i=0; i<cur_system->nasteroids; i++) {
         ast = &cur_system->asteroids[i];
         for (j=0; j<ast->nb; j++) {
            a = &ast->asteroids[j];
            at = space_getType ( a->type );
            if ( (a->appearing == ASTEROID_VISIBLE) &&
                  CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
                        at->gfxs[a->gfxID], 0, 0, &a->pos,
                        &crash[0] ) ) {
               weapon_hitAst( w, a, layer, &crash[0] );
               return; /* Weapon is destroyed. */
            }
         }
      }
   }
   else if (outfit_isBolt(w->outfit)) {
      for (i=0; i<cur_system->nasteroids; i++) {
         ast = &cur_system->asteroids[i];
         for (j=0; j<ast->nb; j++) {
            a = &ast->asteroids[j];
            at = space_getType ( a->type );
            if ( (a->appearing == ASTEROID_VISIBLE) &&
                  CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
                        at->gfxs[a->gfxID], 0, 0, &a->pos,
                        &crash[0] ) ) {
               weapon_hitAst( w, a, layer, &crash[0] );
               return; /* Weapon is destroyed. */
            }
         }
      }
   }
   else if (b) { /* Beam */
      for (i=0; i<cur_system->nasteroids; i++) {
         ast = &cur_system->asteroids[i];
         for (j=0; j<ast->nb; j++) {
            a = &ast->asteroids[j];
            at = space_getType ( a->type );
            if ( (a->appearing == ASTEROID_VISIBLE) &&
                  CollideLineSprite( &w->solid->pos, w->solid->dir,
                        w->outfit->u.bem.range,
                        at->gfxs[a->gfxID], 0, 0, &a->pos,
                        crash ) ) {
               weapon_hitAstBeam( w, a, layer, crash, dt );
               /* No return because beam can still think, it's not
                * destroyed like the other weapons.*/
            }
         }
      }
   }

   /* smart weapons also get to think their next move */
   if (weapon_isSmart(w))
      (*w->think)(w,dt);

   /* Update the solid position. */
   (*w->solid->update)(w->solid, dt);

   /* Update the sound. */
   sound_updatePos(w->voice, w->solid->pos.x, w->solid->pos.y,
         w->solid->vel.x, w->solid->vel.y);
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
   int i;
   double d;

   /* Must be a valid shooter. */
   if (shooter == NULL)
      return;

   /* Must not be disabled. */
   if (pilot_isDisabled(p))
      return;

   /* Player is handled differently. */
   if (shooter->faction == FACTION_PLAYER) {

      /* Increment damage done to by player. */
      p->player_damage += dmg / (p->shield_max + p->armour_max);

      /* If damage is over threshold, inform pilot or if is targeted. */
      if ((p->player_damage > PILOT_HOSTILE_THRESHOLD) ||
            (shooter->target==p->id)) {
         /* Inform attacked. */
         ai_attacked( p, shooter->id, dmg );

         /* Trigger a pseudo-distress that incurs no faction loss. */
         for (i=0; i<array_size(pilot_stack); i++) {
            /* Skip if unsuitable. */
            if ((pilot_stack[i]->ai == NULL) || (pilot_stack[i]->id == p->id) ||
                  (pilot_isFlag(pilot_stack[i], PILOT_DEAD)) ||
                  (pilot_isFlag(pilot_stack[i], PILOT_DELETE)))
               continue;

            /*
             * Pilots within a radius of 1500 (in a zero-interference system)
             * will immediately notice hostile actions.
             */
            d = vect_dist2( &p->solid->pos, &pilot_stack[i]->solid->pos );
            if (d > (pilot_sensorRange() * 0.04 )) /* 0.2^2 */
               continue;

            /* Send AI the distress signal. */
            ai_getDistress( pilot_stack[i], p, shooter );
         }

         /* Set as hostile. */
         pilot_setHostile(p);
      }
   }
   /* Otherwise just inform of being attacked. */
   else
      ai_attacked( p, shooter->id, dmg );
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
   int s, spfx;
   double damage;
   WeaponLayer spfx_layer;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   parent            = pilot_get( w->parent );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage * (1.-w->dam_as_dis_mod) );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = MAX( 0., odmg->disable + dmg.damage * w->dam_as_dis_mod );

   /* Play sound if they have it. */
   s = outfit_soundHit(w->outfit);
   if (s != -1)
      w->voice = sound_playPos( s,
            w->solid->pos.x,
            w->solid->pos.y,
            w->solid->vel.x,
            w->solid->vel.y);

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, w->solid, w->parent, &dmg, 1 );

   /* Get the layer. */
   spfx_layer = (p==player.p) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;
   /* Choose spfx. */
   if (p->shield > 0.)
      spfx = outfit_spfxShield(w->outfit);
   else
      spfx = outfit_spfxArmour(w->outfit);
   /* Add sprite, layer depends on whether player shot or not. */
   spfx_add( spfx, pos->x, pos->y,
         VX(p->solid->vel), VY(p->solid->vel), spfx_layer );

   /* Inform AI that it's been hit. */
   weapon_hitAI( p, parent, damage );

   /* no need for the weapon particle anymore */
   weapon_destroy(w,layer);
}

/**
 * @brief Weapon hit an asteroid.
 *
 *    @param w Weapon involved in the collision.
 *    @param a Asteroid that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 */
static void weapon_hitAst( Weapon* w, Asteroid* a, WeaponLayer layer, Vector2d* pos )
{
   int s, spfx;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = odmg->disable;

   /* Play sound if they have it. */
   s = outfit_soundHit(w->outfit);
   if (s != -1)
      w->voice = sound_playPos( s,
            w->solid->pos.x,
            w->solid->pos.y,
            w->solid->vel.x,
            w->solid->vel.y);

   /* Add the spfx */
   spfx = outfit_spfxShield(w->outfit);
   spfx_add( spfx, pos->x, pos->y,VX(a->vel), VY(a->vel), layer );

   weapon_destroy(w,layer);
   asteroid_hit( a, &dmg );
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
   WeaponLayer spfx_layer;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   parent            = pilot_get( w->parent );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage * dt );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = odmg->disable * dt;

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, w->solid, w->parent, &dmg, 1 );

   /* Add sprite, layer depends on whether player shot or not. */
   if (w->exp_timer == -1.) {
      /* Get the layer. */
      spfx_layer = (p==player.p) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;

      /* Choose spfx. */
      if (p->shield > 0.)
         spfx = outfit_spfxShield(w->outfit);
      else
         spfx = outfit_spfxArmour(w->outfit);

      /* Add graphic. */
      spfx_add( spfx, pos[0].x, pos[0].y,
            VX(p->solid->vel), VY(p->solid->vel), spfx_layer );
      spfx_add( spfx, pos[1].x, pos[1].y,
            VX(p->solid->vel), VY(p->solid->vel), spfx_layer );
      w->exp_timer = -2;

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
      Vector2d pos[2], const double dt )
{
   (void) layer;
   int spfx;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   dmg.damage        = MAX( 0., w->dam_mod * w->strength * odmg->damage * dt );
   dmg.penetration   = odmg->penetration;
   dmg.type          = odmg->type;
   dmg.disable       = odmg->disable * dt;

   asteroid_hit( a, &dmg );

   /* Add sprite. */
   if (w->exp_timer == -1.) {
      spfx = outfit_spfxArmour(w->outfit);

      /* Add graphic. */
      spfx_add( spfx, pos[0].x, pos[0].y,
            VX(a->vel), VY(a->vel), SPFX_LAYER_BACK );
      spfx_add( spfx, pos[1].x, pos[1].y,
            VX(a->vel), VY(a->vel), SPFX_LAYER_BACK );
      w->exp_timer = -2;
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
      const Pilot *pilot_target, const Vector2d *pos, const Vector2d *vel, double dir,
      double swivel, double time )
{
   AsteroidAnchor *field;
   Asteroid *ast;
   Vector2d target_pos;
   Vector2d target_vel;
   Vector2d relative_location;
   double rdir, lead_angle;
   double x, y, t;
   double off;

   if (pilot_target != NULL) {
      target_pos = pilot_target->solid->pos;
      target_vel = pilot_target->solid->vel;
   }
   else {
      if (parent->nav_asteroid < 0)
         return dir;

      field = &cur_system->asteroids[parent->nav_anchor];
      ast = &field->asteroids[parent->nav_asteroid];
      target_pos = ast->pos;
      target_vel = ast->vel;
   }

   /* Get the vector : shooter -> target*/
   vect_cset( &relative_location, VX(target_pos) - VX(parent->solid->pos),
         VY(target_pos) - VY(parent->solid->pos) );

   /* Try to predict where the enemy will be. */
   t = time;
   if (t == INFINITY)  /*Postprocess (t = INFINITY means target is not hittable)*/
      t = 0.;

   /* Position is calculated on where it should be */
   x = (target_pos.x + target_vel.x*t) - (pos->x + vel->x*t);
   y = (target_pos.y + target_vel.y*t) - (pos->y + vel->y*t);

   /* Set angle to face. */
   rdir = ANGLE(x, y);

   if (pilot_target != NULL) {
      /* Lead angle is determined from ewarfare. */
      lead_angle = M_PI*pilot_ewWeaponTrack( parent, pilot_target, outfit->u.blt.track );

      /*only do this if the lead angle is implemented; save compute cycled on fixed weapons*/
      if (lead_angle && FABS( angle_diff(ANGLE(x, y), VANGLE(relative_location)) ) > lead_angle) {
         /* the target is moving too fast for the turret to keep up */
         if (ANGLE(x, y) < VANGLE(relative_location))
            rdir = angle_diff(lead_angle, VANGLE(relative_location));
         else
            rdir = angle_diff(-1*lead_angle, VANGLE(relative_location));
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
 * @brief Creates the bolt specific properties of a weapon.
 *
 *    @param w Weapon to create bolt specific properties of.
 *    @param outfit Outfit which spawned the weapon.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Shooter.
 *    @param time Expected flight time.
 */
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent, double time )
{
   Vector2d v;
   double mass, rdir;
   Pilot *pilot_target;
   double acc;
   glTexture *gfx;

   /* Only difference is the direction of fire */
   if ((w->parent!=w->target) && (w->target != 0)) /* Must have valid target */
      pilot_target = pilot_get(w->target);
   else /* fire straight or at asteroid */
      pilot_target = NULL;

   rdir = weapon_aimTurret( outfit, parent, pilot_target, pos, vel, dir, outfit->u.blt.swivel, time );

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

   mass = 1; /* Lasers are presumed to have unitary mass */
   v = *vel;
   vect_cadd( &v, outfit->u.blt.speed*cos(rdir), outfit->u.blt.speed*sin(rdir));
   w->timer = outfit->u.blt.range / outfit->u.blt.speed;
   w->falloff = w->timer - outfit->u.blt.falloff / outfit->u.blt.speed;
   w->solid = solid_create( mass, rdir, pos, &v, SOLID_UPDATE_EULER );
   w->voice = sound_playPos( w->outfit->u.blt.sound,
         w->solid->pos.x,
         w->solid->pos.y,
         w->solid->vel.x,
         w->solid->vel.y);

   /* Set facing direction. */
   gfx = outfit_gfx( w->outfit );
   gl_getSpriteFromDir( &w->sx, &w->sy, gfx, w->solid->dir );
}


/**
 * @brief Creates the ammo specific properties of a weapon.
 *
 *    @param w Weapon to create ammo specific properties of.
 *    @param launcher Outfit which spawned the weapon.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Shooter.
 *    @param time Expected flight time.
 */
static void weapon_createAmmo( Weapon *w, const Outfit* launcher, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent, double time )
{
   (void) T;
   Vector2d v;
   double mass, rdir;
   Pilot *pilot_target;
   glTexture *gfx;
   Outfit* ammo;

   pilot_target = NULL;
   ammo = launcher->u.lau.ammo;
   if (w->outfit->type == OUTFIT_TYPE_AMMO &&
            launcher->type == OUTFIT_TYPE_TURRET_LAUNCHER) {
      pilot_target = pilot_get(w->target);
      rdir = weapon_aimTurret( ammo, parent, pilot_target, pos, vel, dir, M_PI, time );
   }
   else
      rdir = dir;

   /* Launcher damage. */
   w->dam_mod *= parent->stats.launch_damage;
   /*if (ammo->u.amm.accuracy != 0.) {
      rdir += RNG_2SIGMA() * ammo->u.amm.accuracy/2. * 1./180.*M_PI;
      if ((rdir > 2.*M_PI) || (rdir < 0.))
         rdir = fmod(rdir, 2.*M_PI);
   }*/
   if (rdir < 0.)
      rdir += 2.*M_PI;
   else if (rdir >= 2.*M_PI)
      rdir -= 2.*M_PI;

   /* If thrust is 0. we assume it starts out at speed. */
   v = *vel;
   if (ammo->u.amm.thrust == 0.)
      vect_cadd( &v, cos(rdir) * w->outfit->u.amm.speed,
            sin(rdir) * w->outfit->u.amm.speed );
   w->real_vel = VMOD(v);

   /* Set up ammo details. */
   mass        = w->outfit->mass;
   w->timer    = ammo->u.amm.duration * parent->stats.launch_range;
   w->solid    = solid_create( mass, rdir, pos, &v, SOLID_UPDATE_RK4 );
   if (w->outfit->u.amm.thrust != 0.) {
      weapon_setThrust( w, w->outfit->u.amm.thrust * mass );
      w->solid->speed_max = w->outfit->u.amm.speed; /* Limit speed, we only care if it has thrust. */
   }

   /* Handle seekers. */
   if (w->outfit->u.amm.ai != AMMO_AI_UNGUIDED) {
      w->think = think_seeker; /* AI is the same atm. */

      /* If they are seeking a pilot, increment lockon counter. */
      if (pilot_target == NULL)
         pilot_target = pilot_get(w->target);
      if (pilot_target != NULL)
         pilot_target->lockons++;
   }

   /* Play sound. */
   w->voice    = sound_playPos(w->outfit->u.amm.sound,
         w->solid->pos.x,
         w->solid->pos.y,
         w->solid->vel.x,
         w->solid->vel.y);

   /* Set facing direction. */
   gfx = outfit_gfx( w->outfit );
   gl_getSpriteFromDir( &w->sx, &w->sy, gfx, w->solid->dir );
}


/**
 * @brief Creates a new weapon.
 *
 *    @param outfit Outfit which spawned the weapon.
 *    @param T temperature of the shooter.
 *    @param dir Direction the shooter is facing.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Shooter.
 *    @param target Target ID of the shooter.
 *    @param time Expected flight time.
 *    @return A pointer to the newly created weapon.
 */
static Weapon* weapon_create( const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot* parent, const unsigned int target, double time )
{
   double mass, rdir;
   Pilot *pilot_target;
   AsteroidAnchor *field;
   Asteroid *ast;
   
   Weapon* w;

   /* Create basic features */
   w           = calloc( 1, sizeof(Weapon) );
   w->dam_mod  = 1.; /* Default of 100% damage. */
   w->dam_as_dis_mod = 0.; /* Default of 0% damage to disable. */
   w->faction  = parent->faction; /* non-changeable */
   w->parent   = parent->id; /* non-changeable */
   w->target   = target; /* non-changeable */
   if (outfit_isLauncher(outfit))
      w->outfit   = outfit->u.lau.ammo; /* non-changeable */
   else
      w->outfit   = outfit; /* non-changeable */
   w->update   = weapon_update;
   w->status   = WEAPON_STATUS_OK;
   w->strength = 1.;

   /* Inform the target. */
   pilot_target = pilot_get(target);
   if (pilot_target != NULL)
      pilot_target->projectiles++;

   switch (outfit->type) {

      /* Bolts treated together */
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         weapon_createBolt( w, outfit, T, dir, pos, vel, parent, time );
         break;

      /* Beam weapons are treated together. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         rdir = dir;
         if (outfit->type == OUTFIT_TYPE_TURRET_BEAM) {
            pilot_target = pilot_get(target);
            if ((w->parent != w->target) && (pilot_target != NULL))
               rdir = vect_angle(pos, &pilot_target->solid->pos);
            else if (parent->nav_asteroid >= 0) {
               field = &cur_system->asteroids[parent->nav_anchor];
               ast = &field->asteroids[parent->nav_asteroid];
               rdir = vect_angle(pos, &ast->pos);
            }
         }

         if (rdir < 0.)
            rdir += 2.*M_PI;
         else if (rdir >= 2.*M_PI)
            rdir -= 2.*M_PI;
         mass = 1.; /**< Needs a mass. */
         w->solid = solid_create( mass, rdir, pos, vel, SOLID_UPDATE_EULER );
         w->think = think_beam;
         w->timer = outfit->u.bem.duration;
         w->voice = sound_playPos( w->outfit->u.bem.sound,
               w->solid->pos.x,
               w->solid->pos.y,
               w->solid->vel.x,
               w->solid->vel.y);
         break;

      /* Treat seekers together. */
      case OUTFIT_TYPE_LAUNCHER:
      case OUTFIT_TYPE_TURRET_LAUNCHER:
         weapon_createAmmo( w, outfit, T, dir, pos, vel, parent, time );
         break;

      /* just dump it where the player is */
      default:
         WARN(_("Weapon of type '%s' has no create implemented yet!"),
               w->outfit->name);
         w->solid = solid_create( 1., dir, pos, vel, SOLID_UPDATE_EULER );
         break;
   }

   /* Set life to timer. */
   w->life = w->timer;

   return w;
}


/**
 * @brief Creates a new weapon.
 *
 *    @param outfit Outfit which spawns the weapon.
 *    @param T Temperature of the shooter.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Pilot ID of the shooter.
 *    @param target Target ID that is getting shot.
 *    @param time Expected flight time.
 */
void weapon_add( const Outfit* outfit, const double T, const double dir,
      const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, unsigned int target, double time )
{
   WeaponLayer layer;
   Weapon *w, **m;
   GLsizei size;
   size_t bufsize;

   if (!outfit_isBolt(outfit) &&
         !outfit_isLauncher(outfit)) {
      ERR(_("Trying to create a Weapon from a non-Weapon type Outfit"));
      return;
   }

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w     = weapon_create( outfit, T, dir, pos, vel, parent, target, time );

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
 *    @param outfit Outfit which spawns the weapon.
 *    @param dir Direction of the shooter.
 *    @param pos Position of the shooter.
 *    @param vel Velocity of the shooter.
 *    @param parent Pilot shooter.
 *    @param target Target ID that is getting shot.
 *    @param mount Mount on the ship.
 *    @return The identifier of the beam weapon.
 *
 * @sa beam_end
 */
unsigned int beam_start( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, const unsigned int target,
      PilotOutfitSlot *mount )
{
   WeaponLayer layer;
   Weapon *w, **m;
   GLsizei size;
   size_t bufsize;

   if (!outfit_isBeam(outfit)) {
      ERR(_("Trying to create a Beam Weapon from a non-beam outfit."));
      return -1;
   }

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w = weapon_create( outfit, 0., dir, pos, vel, parent, target, 0. );
   w->ID = ++beam_idgen;
   w->mount = mount;
   w->exp_timer = 0.;

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
   int i;
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


   /* Now try to destroy the beam. */
   for (i=0; i<array_size(curLayer); i++) {
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

   for (i=0; (wlayer[i] != w) && (i < array_size(wlayer)); i++); /* get to the current position */
   if (i >= array_size(wlayer)) {
      WARN(_("Trying to destroy weapon not found in stack!"));
      return;
   }

   weapon_free(wlayer[i]);
   array_erase( &wlayer, &wlayer[i], &wlayer[i+1] );
}


/**
 * @brief Frees the weapon.
 *
 *    @param w Weapon to free.
 */
static void weapon_free( Weapon* w )
{
   Pilot *pilot_target;

   pilot_target = pilot_get( w->target );

   /* Decrement target lockons if needed */
   if (pilot_target != NULL) {
      pilot_target->projectiles--;
      if (outfit_isSeeker(w->outfit))
         pilot_target->lockons--;
      }

   /* Stop playing sound if beam weapon. */
   if (outfit_isBeam(w->outfit)) {
      sound_stop( w->voice );
      sound_playPos(w->outfit->u.bem.sound_off,
            w->solid->pos.x,
            w->solid->pos.y,
            w->solid->vel.x,
            w->solid->vel.y);
   }

   /* Free the solid. */
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
   for (i=0; i < array_size(wbackLayer); i++) {
      sound_stop(wbackLayer[i]->voice);
      weapon_free(wbackLayer[i]);
   }
   array_erase( &wbackLayer, array_begin(wbackLayer), array_end(wbackLayer) );
   for (i=0; i < array_size(wfrontLayer); i++) {
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
}


/**
 * @brief Clears possible exploded weapons.
 */
void weapon_explode( double x, double y, double radius,
      int dtype, double damage,
      const Pilot *parent, int mode )
{
   (void)dtype;
   (void)damage;
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
   int i;
   Weapon **curLayer;
   double dist, rad2;

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
   for (i=0; i<array_size(curLayer); i++) {
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


