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
#include "nstring.h"

#include "log.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "collision.h"
#include "spfx.h"
#include "opengl.h"
#include "explosion.h"
#include "gui.h"
#include "camera.h"
#include "ai.h"
#include "ai_extra.h"


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
extern int pilot_nstack;


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
   double jam_power; /**< Power being jammed by. */
   double dam_mod; /**< Damage modifier. */
   int voice; /**< Weapon's voice. */
   double exp_timer; /**< Explosion timer for beams. */
   double life; /**< Total life. */
   double timer; /**< mainly used to see when the weapon was fired */
   double anim; /**< Used for beam weapon graphics and others. */
   int sprite; /**< Used for spinning outfits. */
   const PilotOutfitSlot *mount; /**< Used for beam weapons. */
   double falloff; /**< Point at which damage falls off. */
   double strength; /**< Calculated with falloff. */
   int sx; /**< Current X sprite to use. */
   int sy; /**< Current Y sprite to use. */

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
static Weapon** wfrontLayer = NULL; /**< in front of pilots, behind player */
static int nwfrontLayer = 0; /**< number of elements */
static int mwfrontLayer = 0; /**< alloced memory size */

/* Graphics. */
static gl_vbo  *weapon_vbo     = NULL; /**< Weapon VBO. */
static GLfloat *weapon_vboData = NULL; /**< Data of weapon VBO. */
static int weapon_vboSize      = 0; /**< Size of the VBO. */


/* Internal stuff. */
static unsigned int beam_idgen = 0; /**< Beam identifier generator. */


/*
 * Prototypes
 */
/* Creation. */
static double weapon_aimTurret( Weapon *w, const Outfit *outfit, const Pilot *parent,
      const Pilot *pilot_target, const Vector2d *pos, const Vector2d *vel, double dir,
      double swivel );
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent );
static void weapon_createAmmo( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent );
static Weapon* weapon_create( const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, const unsigned int target );
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
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
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
   for (i=0; i<nwbackLayer; i++) {
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
   for (i=0; i<nwfrontLayer; i++) {
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

      /* Activate VBO. */
      gl_vboActivateOffset( weapon_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
      gl_vboActivateOffset( weapon_vbo, GL_COLOR_ARRAY, offset * sizeof(GLfloat),
            4, GL_FLOAT, 0 );

      /* Render VBO. */
      glDrawArrays( GL_POINTS, 0, p );

      /* Disable VBO. */
      gl_vboDeactivate();
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

   if (w->target == w->parent)
      return; /* no self shooting */

   p = pilot_get(w->target); /* no null pilot_nstack */
   if (p==NULL) {
      weapon_setThrust( w, 0. );
      weapon_setTurn( w, 0. );
      return;
   }

   /* Handle by status. */
   switch (w->status) {

      case WEAPON_STATUS_OK: /* Check to see if can get jammed */
      /* Purpose fallthrough */
      case WEAPON_STATUS_UNJAMMED: /* Work as expected */

         /* Smart seekers take into account ship velocity. */
         if (w->outfit->u.amm.ai == 2) {

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

         /* Set turn. */
         turn_max = w->outfit->u.amm.turn * (1. - w->jam_power);
         weapon_setTurn( w, CLAMP( -turn_max, turn_max,
                  10 * diff * w->outfit->u.amm.turn ));
         break;

      case WEAPON_STATUS_JAMMED: /* Continue doing whatever */
         /* Do nothing, dir_vel should be set already if needed */
         break;

      default:
         WARN("Unknown weapon status for '%s'", w->outfit->name);
         break;
   }

   /* Limit speed here */
   w->real_vel = MIN( w->outfit->u.amm.speed, w->real_vel + w->outfit->u.amm.thrust*dt );
   vect_pset( &w->solid->vel, (1. - w->jam_power) * w->real_vel, w->solid->dir );

   /* Modulate max speed. */
   //w->solid->speed_max = w->outfit->u.amm.speed * (1. - w->jam_power);
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
   w->solid->pos.x = p->solid->pos.x + v.x;
   w->solid->pos.y = p->solid->pos.y + v.y;

   /* Handle aiming. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BEAM:
         w->solid->dir = p->solid->dir;
         break;

      case OUTFIT_TYPE_TURRET_BEAM:
         /* Get target, if target is dead beam stops moving. */
         t = pilot_get(w->target);
         if (t==NULL) {
            weapon_setTurn( w, 0. );
            return;
         }

         if (w->target == w->parent) /* Invalid target, tries to follow shooter. */
            diff = angle_diff(w->solid->dir, p->solid->dir);
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
   int *nlayer;
   Weapon *w;
   int i, j, k;
   int spfx;
   int s;
   Pilot *p;
   Outfit *o;

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
         return;
   }

   /** @TODO optimize me plz. */
   /* Reset jam power. */
   for (k=0; k < *nlayer; k++) {
      w = wlayer[k];
      if (!outfit_isSeeker( w->outfit ))
         continue;
      w->jam_power = 0.;
   }
   /* Iterate over all pilots. */
   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      /* Must be jamming. */
      if (!p->jamming)
         continue;

      /* Iterate over outfits to find jammers. */
      for (j=0; j<p->noutfits; j++) {
         o    = p->outfits[j]->outfit;
         if (o==NULL)
            continue;
         /* Must be on. */
         if (p->outfits[j]->state != PILOT_OUTFIT_ON)
            continue;
         /* Must be a jammer. */
         if (!outfit_isJammer(o))
            continue;
    
         /* Apply jamming. */
         for (k=0; k < *nlayer; k++) {
            w = wlayer[k];
            if (!outfit_isSeeker( w->outfit ))
               continue;

            /* Must be in range. */
            if (o->u.jam.range2 < vect_dist2( &w->solid->pos, &p->solid->pos ))
               continue;

            /* We only consider the strongest jammer. */
            w->jam_power = CLAMP( 0., 1., MAX( w->jam_power, (o->u.jam.power - w->outfit->u.amm.resist) ) );
         }
      }
   }

   i = 0;
   while (i < *nlayer) {
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
            if (w->timer < 0.) {
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
         return;
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
   double x,y, cx,cy, gx,gy;
   glTexture *gfx;
   double z;
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
         gfx = outfit_gfx(w->outfit);

         /* Zoom. */
         z = cam_getZoom();

         /* Position. */
         cam_getPos( &cx, &cy );
         gui_getOffset( &gx, &gy );
         x = (w->solid->pos.x - cx)*z + gx;
         y = (w->solid->pos.y - cy)*z + gy;

         /* Set up the matrix. */
         glPushMatrix();
            glTranslated( SCREEN_W/2.+x, SCREEN_H/2.+y, 0. );
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
            glVertex2d( -gfx->sh/2.*z, 0. );

            glTexCoord2d( w->anim, 1. );
            glVertex2d( +gfx->sh/2.*z, 0. );

            /* Full strength. */
            COLOUR(cWhite);

            glTexCoord2d( w->anim + 10. / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2.*z, 10.*z );

            glTexCoord2d( w->anim + 10. / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2.*z, 10.*z );

            glTexCoord2d( w->anim + 0.8*w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2.*z, 0.8*w->outfit->u.bem.range*z );

            glTexCoord2d( w->anim + 0.8*w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2.*z, 0.8*w->outfit->u.bem.range*z );

            /* Fades out. */
            ACOLOUR(cWhite, 0.);

            glTexCoord2d( w->anim + w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2.*z, w->outfit->u.bem.range*z );

            glTexCoord2d( w->anim + w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2.*z, w->outfit->u.bem.range*z );
         glEnd(); /* GL_QUAD_STRIP */

         /* Do the beam movement. */
         w->anim -= 5. * dt;
         if (w->anim <= -gfx->sw)
            w->anim += gfx->sw;

         /* Clean up. */
         glDisable(GL_TEXTURE_2D);
         glShadeModel(GL_FLAT);
         glPopMatrix();
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
      if (pilot_isFlag(p, PILOT_HOSTILE))
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
      if (parent != NULL) {
         if (pilot_isFlag(parent, PILOT_BRIBED))
            return 0;
         if (pilot_isFlag(parent, PILOT_HOSTILE))
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
   int i, b, psx,psy;
   glTexture *gfx;
   Vector2d crash[2];
   Pilot *p;

   /* Get the sprite direction to speed up calculations. */
   b     = outfit_isBeam(w->outfit);
   if (!b) {
      gfx = outfit_gfx(w->outfit);
      gl_getSpriteFromDir( &w->sx, &w->sy, gfx, w->solid->dir );
   }
   else
      gfx = NULL;

   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      /* Beam weapons have special collisions. */
      if (b) {
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
               (w->status == WEAPON_STATUS_OK) &&
               weapon_checkCanHit(w,p) &&
               CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
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
               CollideSprite( gfx, w->sx, w->sy, &w->solid->pos,
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
         ai_attacked( p, shooter->id );

         /* Set as hostile. */
         pilot_setHostile(p);
         pilot_rmFlag( p, PILOT_BRIBED );
         pilot_rmFlag( p, PILOT_FRIENDLY );
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
   int s, spfx;
   double damage;
   WeaponLayer spfx_layer;
   Damage dmg;
   const Damage *odmg;

   /* Get general details. */
   odmg              = outfit_damage( w->outfit );
   parent            = pilot_get( w->parent );
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

   /* Have pilot take damage and get real damage done. */
   damage = pilot_hit( p, w->solid, w->parent, &dmg );

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
   damage = pilot_hit( p, w->solid, w->parent, &dmg );

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
 * @brief Gets the aim position of a turret weapon.
 *
 *    @param w Weapon aiming.
 *    @param outfit Weapon outfit.
 *    @param parent Parent of the weapon.
 *    @param pilot_target Target of the weapon.
 *    @param pos Position of the turret.
 *    @param vel Velocity of the turret.
 *    @param dir Direction facing parent ship and turret.
 */
static double weapon_aimTurret( Weapon *w, const Outfit *outfit, const Pilot *parent,
      const Pilot *pilot_target, const Vector2d *pos, const Vector2d *vel, double dir,
      double swivel )
{
   Vector2d approach_vector, relative_location;
   double rdir, lead_angle;
   double speed, radial_speed;
   double x, y, t, dist;
   double off;

   if (pilot_target == NULL)
      rdir        = dir;
   else {
      /* Get the distance */
      dist = vect_dist( pos, &pilot_target->solid->pos );
      vect_cset( &relative_location, VX(pilot_target->solid->pos) - VX(parent->solid->pos),
            VY(pilot_target->solid->pos) - VY(parent->solid->pos) );

      /* Aim. */
      if (dist > outfit->u.blt.range*1.2) {
         x = pilot_target->solid->pos.x - pos->x;
         y = pilot_target->solid->pos.y - pos->y;
      }
      else {
         /* Try to predict where the enemy will be. */
         /* determine the radial, or approach speed */
         vect_cset( &approach_vector, VX(parent->solid->vel) - VX(pilot_target->solid->vel),
               VY(parent->solid->vel) - VY(pilot_target->solid->vel) );

         radial_speed = vect_dot( &approach_vector, &relative_location );
         radial_speed = radial_speed / VMOD(relative_location);

         speed = w->outfit->u.blt.speed;

         /* Time for shots to reach that distance */
         /* if the target is not hittable (ie, fleeing faster than our shots can fly), just face the target */
         if((speed+radial_speed) > 0)
            t = dist / (speed + radial_speed);
         else
            t = 0;

         /* Position is calculated on where it should be */
         x = (pilot_target->solid->pos.x + pilot_target->solid->vel.x*t)
            - (pos->x + vel->x*t);
         y = (pilot_target->solid->pos.y + pilot_target->solid->vel.y*t)
            - (pos->y + vel->y*t);
      }

      /* Set angle to face. */
      rdir = ANGLE(x, y);

      /* Lead angle is determined from ewarfare. */
      lead_angle = M_PI*pilot_ewWeaponTrack( parent, pilot_target, outfit->u.blt.track );

      /*only do this if the lead angle is implemented; save compute cycled on fixed weapons*/
      if (lead_angle && fabs( angle_diff(ANGLE(x, y), VANGLE(relative_location)) ) > lead_angle) {
         /* the target is moving too fast for the turret to keep up */
         if (ANGLE(x, y) < VANGLE(relative_location))
            rdir = angle_diff(lead_angle, VANGLE(relative_location));
         else
            rdir = angle_diff(-1*lead_angle, VANGLE(relative_location));
      }

      /* Calculate bounds. */
      off = angle_diff( rdir, dir );
      if (fabs(off) > swivel) {
         if (off > 0.)
            rdir = dir - swivel;
         else
            rdir = dir + swivel;
      }
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
 */
static void weapon_createBolt( Weapon *w, const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent )
{
   Vector2d v;
   double mass, rdir;
   Pilot *pilot_target;
   double acc;
   glTexture *gfx;

   /* Only difference is the direction of fire */
   if ((w->parent!=w->target) && (w->target != 0)) { /* Must have valid target */
      pilot_target = pilot_get(w->target);
      rdir = weapon_aimTurret( w, outfit, parent, pilot_target, pos, vel, dir, outfit->u.blt.swivel );
   }
   else /* fire straight */
      rdir = dir;

   /* Calculate accuracy. */
   acc =  HEAT_WORST_ACCURACY * pilot_heatAccuracyMod( T );

   /* Stat modifiers. */
   if (outfit->type == OUTFIT_TYPE_TURRET_BOLT)
      w->dam_mod *= parent->stats.tur_damage;
   else
      w->dam_mod *= parent->stats.fwd_damage;

   /* Calculate direction. */
   rdir += RNG_2SIGMA() * acc;
   if (rdir < 0.)
      rdir += 2.*M_PI;
   else if (rdir >= 2.*M_PI)
      rdir -= 2.*M_PI;

   mass = 1; /* Lasers are presumed to have unitary mass */
   vectcpy( &v, vel );
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
 */
static void weapon_createAmmo( Weapon *w, const Outfit* launcher, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel, const Pilot* parent )
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
      rdir = weapon_aimTurret( w, ammo, parent, pilot_target, pos, vel, dir, M_PI );
   }
   else
      rdir = dir;

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
   vectcpy( &v, vel );
   if (ammo->u.amm.thrust == 0.)
      vect_cadd( &v, cos(rdir) * w->outfit->u.amm.speed,
            sin(rdir) * w->outfit->u.amm.speed );
   w->real_vel = VMOD(v);

   /* Set up ammo details. */
   mass        = w->outfit->mass;
   w->timer    = ammo->u.amm.duration;
   w->solid    = solid_create( mass, rdir, pos, &v, SOLID_UPDATE_RK4 );
   if (w->outfit->u.amm.thrust != 0.) {
      weapon_setThrust( w, w->outfit->u.amm.thrust * mass );
      w->solid->speed_max = w->outfit->u.amm.speed; /* Limit speed, we only care if it has thrust. */
   }

   /* Handle seekers. */
   if (w->outfit->u.amm.ai > 0) {
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
 *    @return A pointer to the newly created weapon.
 */
static Weapon* weapon_create( const Outfit* outfit, double T,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const Pilot* parent, const unsigned int target )
{
   double mass, rdir;
   Pilot *pilot_target;
   Weapon* w;

   /* Create basic features */
   w           = calloc( 1, sizeof(Weapon) );
   w->dam_mod  = 1.; /* Default of 100% damage. */
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

   switch (outfit->type) {

      /* Bolts treated together */
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         weapon_createBolt( w, outfit, T, dir, pos, vel, parent );
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
         weapon_createAmmo( w, outfit, T, dir, pos, vel, parent );
         break;

      /* just dump it where the player is */
      default:
         WARN("Weapon of type '%s' has no create implemented yet!",
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
 */
void weapon_add( const Outfit* outfit, const double T, const double dir,
      const Vector2d* pos, const Vector2d* vel,
      const Pilot *parent, unsigned int target )
{
   WeaponLayer layer;
   Weapon *w;
   Weapon **curLayer;
   int *mLayer, *nLayer;
   GLsizei size;

   if (!outfit_isBolt(outfit) &&
         !outfit_isLauncher(outfit)) {
      ERR("Trying to create a Weapon from a non-Weapon type Outfit");
      return;
   }

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w     = weapon_create( outfit, T, dir, pos, vel, parent, target );

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

   if (*mLayer > *nLayer) /* more memory alloced than needed */
      curLayer[(*nLayer)++] = w;
   else { /* need to allocate more memory */
      if ((*mLayer) == 0)
         (*mLayer) = WEAPON_CHUNK_MIN;
      else
         (*mLayer) += MIN( (*mLayer), WEAPON_CHUNK_MAX );

      switch (layer) {
         case WEAPON_LAYER_BG:
            curLayer   = wbackLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
         case WEAPON_LAYER_FG:
            curLayer   = wfrontLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
      }
      curLayer[(*nLayer)++] = w;

      /* Grow the vertex stuff. */
      weapon_vboSize = mwfrontLayer + mwbacklayer;
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
      const PilotOutfitSlot *mount )
{
   WeaponLayer layer;
   Weapon *w;
   Weapon **curLayer;
   int *mLayer, *nLayer;
   GLsizei size;

   if (!outfit_isBeam(outfit)) {
      ERR("Trying to create a Beam Weapon from a non-beam outfit.");
      return -1;
   }

   layer = (parent->id==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   w = weapon_create( outfit, 0., dir, pos, vel, parent, target );
   w->ID = ++beam_idgen;
   w->mount = mount;
   w->exp_timer = 0.;

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

   if (*mLayer > *nLayer) /* more memory alloced than needed */
      curLayer[(*nLayer)++] = w;
   else { /* need to allocate more memory */
      if ((*mLayer) == 0)
         (*mLayer) = WEAPON_CHUNK_MIN;
      else
         (*mLayer) += MIN( (*mLayer), WEAPON_CHUNK_MAX );

      switch (layer) {
         case WEAPON_LAYER_BG:
            curLayer = wbackLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
         case WEAPON_LAYER_FG:
            curLayer = wfrontLayer = realloc(curLayer, (*mLayer)*sizeof(Weapon*));
            break;
      }
      curLayer[(*nLayer)++] = w;

      /* Grow the vertex stuff. */
      weapon_vboSize = mwfrontLayer + mwbacklayer;
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
   int *nLayer;

   layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;

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
         return;
   }

   for (i=0; (wlayer[i] != w) && (i < *nlayer); i++); /* get to the current position */
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

   /* Destroy front layer. */
   if (wbackLayer != NULL) {
      free(wbackLayer);
      wbackLayer  = NULL;
      mwbacklayer = 0;
   }

   /* Destroy back layer. */
   if (wfrontLayer != NULL) {
      free(wfrontLayer);
      wfrontLayer  = NULL;
      mwfrontLayer = 0;
   }

   /* Destroy VBO. */
   if (weapon_vbo != NULL) {
      free( weapon_vboData );
      weapon_vboData = NULL;
      gl_vboDestroy( weapon_vbo );
      weapon_vbo = NULL;
   }
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


