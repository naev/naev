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

#include <math.h>
#include <malloc.h>
#include <string.h>

#include "naev.h"
#include "log.h"
#include "rng.h"
#include "pilot.h"
#include "player.h"
#include "collision.h"
#include "spfx.h"
#include "opengl.h"


#define weapon_isSmart(w)     (w->think != NULL) /**< Checks if the weapon w is smart. */

#define WEAPON_CHUNK          128 /**< Size to increase array with */

/* Weapon status */
#define WEAPON_STATUS_OK         0 /**< Weapon is fine */
#define WEAPON_STATUS_JAMMED     1 /**< Got jammed */
#define WEAPON_STATUS_UNJAMMED   2 /**< Survived jamming */

/*
 * opengl stuff.
 */
extern Vector2d *gl_camera;
extern double gui_xoff, gui_yoff;
/*
 * pilot stuff
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;
/*
 * player stuff
 */
extern unsigned int player_target;
/*
 * ai stuff
 */
extern void ai_attacked( Pilot* attacked, const unsigned int attacker );


/**
 * @struct Weapon
 *
 * @brief In-game representation of a weapon.
 */
typedef struct Weapon_ {
   Solid *solid; /**< Actually has its own solid :) */

   unsigned int faction; /**< faction of pilot that shot it */
   unsigned int parent; /**< pilot that shot it */
   unsigned int target; /**< target to hit, only used by seeking things */
   const Outfit* outfit; /**< related outfit that fired it or whatnot */

   double lockon; /**< some weapons have a lockon delay */
   double timer; /**< mainly used to see when the weapon was fired */

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


/*
 * Prototypes
 */
/* static */
static Weapon* weapon_create( const Outfit* outfit,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const unsigned int parent, const unsigned int target );
static void weapon_render( const Weapon* w );
static void weapons_updateLayer( const double dt, const WeaponLayer layer );
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer );
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos );
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      Vector2d pos[2], const double dt );
static void weapon_destroy( Weapon* w, WeaponLayer layer );
static void weapon_free( Weapon* w );
/* think */
static void think_seeker( Weapon* w, const double dt );
static void think_beam( Weapon* w, const double dt );
/* externed */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape );


#define PIXEL(x,y)      \
   if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
         (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y))<rc)))   \
   glVertex2i((x),(y)) /**< Sets a pixel if within range. */
/**
 * @fn void weapon_minimap( const double res, const double w,
 *       const double h, const RadarShape shape )
 *
 * @brief Draws the minimap weapons (used in player.c).
 *
 *    @param res Minimap resolution.
 *    @param w Width of minimap.
 *    @param h Height of minimap.
 *    @param shape Shape of the minimap.
 */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape )
{
   int i, rc;
   double x, y;

   if (shape==RADAR_CIRCLE) rc = (int)(w*w);

   for (i=0; i<nwbackLayer; i++) {
      x = (wbackLayer[i]->solid->pos.x - player->solid->pos.x) / res;
      y = (wbackLayer[i]->solid->pos.y - player->solid->pos.y) / res;
      PIXEL(x,y);
   }
   for (i=0; i<nwfrontLayer; i++) {
      x = (wfrontLayer[i]->solid->pos.x - player->solid->pos.x) / res;
      y = (wfrontLayer[i]->solid->pos.y - player->solid->pos.y) / res;
      PIXEL(x,y);
   }
}
#undef PIXEL


/**
 * @fn static void think_seeker( Weapon* w, const double dt )
 *
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

   if (w->target == w->parent) return; /* no self shooting */

   p = pilot_get(w->target); /* no null pilot_nstack */
   if (p==NULL) {
      w->solid->dir_vel = 0.; /* go straight */
      vectnull( &w->solid->force ); /* no force */
      return;
   }

   /* ammo isn't locked on yet */
   if (SDL_GetTicks() > (w->outfit->u.amm.lockon)) {

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
            diff = angle_diff(w->solid->dir, /* Get angle to target pos */
                  vect_angle(&w->solid->pos, &p->solid->pos));
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
 * @fn static void think_beam( Weapon* w, const double dt )
 *
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

   /* Update beam position to match pilot. */
   w->solid->pos.x = p->solid->pos.x;
   w->solid->pos.y = p->solid->pos.y;

   /* Get target, if target is dead beam stops moving. */
   t = pilot_get(w->target);
   if (t==NULL) {
      w->solid->dir_vel = 0.;
      return;
   }

   /* Handle aiming. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BEAM:
         w->solid->dir = p->solid->dir;
         break;

      case OUTFIT_TYPE_TURRET_BEAM:
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
 * @fn void weapons_update( const double dt )
 *
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
 * @fn static void weapons_updateLayer( const double dt, const WeaponLayer layer )
 *
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

   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         nlayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         nlayer = &nwfrontLayer;
         break;
   }

   for (i=0; i<(*nlayer); i++) {
      w = wlayer[i];
      switch (wlayer[i]->outfit->type) {

         /* most missiles behave the same */
         case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
         case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         case OUTFIT_TYPE_MISSILE_SWARM_AMMO:
         case OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO:
            if (wlayer[i]->lockon > 0.) /* decrement lockon */
               wlayer[i]->lockon -= dt;
            /* purpose fallthrough */

         /* bolts too */
         case OUTFIT_TYPE_BOLT:
         case OUTFIT_TYPE_TURRET_BOLT:
         case OUTFIT_TYPE_MISSILE_DUMB_AMMO: /* Dumb missiles are like bolts */
            wlayer[i]->timer -= dt;
            if (wlayer[i]->timer < 0.) {
               weapon_destroy(wlayer[i],layer);
               continue;
            }
            break;

         /* Beam weapons handled a part. */
         case OUTFIT_TYPE_BEAM:
         case OUTFIT_TYPE_TURRET_BEAM:
            wlayer[i]->timer -= dt;
            if (wlayer[i]->timer < 0.) {
               weapon_destroy(wlayer[i],layer);
               continue;
            }
            /* We use the lockon to tell when we have to create explosions. */
            wlayer[i]->lockon -= dt;
            if (wlayer[i]->lockon < 0.) {
               if (wlayer[i]->lockon < -1.)
                  wlayer[i]->lockon = 0.100;
               else
                  wlayer[i]->lockon = -1.;
            }
            break;
         default:
            WARN("Weapon of type '%s' has no update implemented yet!",
                  w->outfit->name);
            break;
      }
      weapon_update(wlayer[i],dt,layer);

      /* if the weapon has been deleted we have to hold back one */
      if (w != wlayer[i]) i--;
   }
}


/**
 * @fn void weapons_render( const WeaponLayer layer )
 *
 * @brief Renders all the weapons in a layer.
 *
 *    @param layer Layer to render.
 */
void weapons_render( const WeaponLayer layer )
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
   }
   
   for (i=0; i<(*nlayer); i++)
      weapon_render( wlayer[i] );
}


/**
 * @fn static void weapon_render( const Weapon* w )
 *
 * @brief Renders an individual weapon.
 *
 *    @param w Weapon to render.
 */
static void weapon_render( const Weapon* w )
{
   int sx, sy;
   double x,y;
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
         gfx = outfit_gfx(w->outfit);
         /* get the sprite corresponding to the direction facing */
         gl_getSpriteFromDir( &sx, &sy, gfx, w->solid->dir );
         gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y, sx, sy, NULL );
         break;

      /* Beam weapons. */
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
         gfx = outfit_gfx(w->outfit);
         x = w->solid->pos.x - VX(*gl_camera) + gui_xoff;
         y = w->solid->pos.y - VY(*gl_camera) + gui_yoff;

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
            COLOUR(cWhite);

            /* Full strength. */
            glTexCoord2d( 0., 0. );
            glVertex2d( -gfx->sh/2., 0. );

            glTexCoord2d( 0., 1. );
            glVertex2d( +gfx->sh/2., 0. );

            glTexCoord2d( 0.8*w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2., 0.8*w->outfit->u.bem.range );

            glTexCoord2d( 0.8*w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2., 0.8*w->outfit->u.bem.range );

            /* Fades out. */
            ACOLOUR(cWhite, 0.);

            glTexCoord2d( w->outfit->u.bem.range / gfx->sw, 0. );
            glVertex2d( -gfx->sh/2., w->outfit->u.bem.range );

            glTexCoord2d( w->outfit->u.bem.range / gfx->sw, 1. );
            glVertex2d( +gfx->sh/2., w->outfit->u.bem.range );
         glEnd(); /* GL_QUAD_STRIP */

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
 * @fn static void weapon_update( Weapon* w, const double dt, WeaponLayer layer )
 *
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

   for (i=0; i<pilot_nstack; i++) {

      /* check for player to exists */
      if ((i==0) && (player==NULL)) continue;

      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      /* Beam weapons have special collisions. */
      if (outfit_isBeam(w->outfit)) {
         if (!areAllies(w->faction,pilot_stack[i]->faction) &&
               CollideLineSprite( &w->solid->pos, w->solid->dir,
                     w->outfit->u.bem.range,
                     pilot_stack[i]->ship->gfx_space, psx, psy,
                     &pilot_stack[i]->solid->pos,
                     crash)) {
            weapon_hitBeam( w, pilot_stack[i], layer, crash, dt );
            /* No return because beam can still think, it's not
             * destroyed like the other weapons.*/
         }
      }
      /* smart weapons only collide with their target */
      else if (weapon_isSmart(w)) {

         gfx = outfit_gfx(w->outfit);
         gl_getSpriteFromDir( &wsx, &wsy, gfx, w->solid->dir );
         if ((pilot_stack[i]->id == w->target) &&
               CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                     pilot_stack[i]->ship->gfx_space, psx, psy,
                     &pilot_stack[i]->solid->pos,
                     &crash[0] )) {
            weapon_hit( w, pilot_stack[i], layer, &crash[0] );
            return; /* Weapon is destroyed. */
         }
      }
      /* dump weapons hit anything not of the same faction */
      else if (!weapon_isSmart(w)) {

         gfx = outfit_gfx(w->outfit);
         gl_getSpriteFromDir( &wsx, &wsy, gfx, w->solid->dir );
         if (!areAllies(w->faction,pilot_stack[i]->faction) &&
               CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                     pilot_stack[i]->ship->gfx_space, psx, psy,
                     &pilot_stack[i]->solid->pos,
                     &crash[0] )) {
            weapon_hit( w, pilot_stack[i], layer, &crash[0] );
            return; /* Weapon is destroyed. */
         }
      }
   }

   /* smart weapons also get to think their next move */
   if (weapon_isSmart(w)) (*w->think)(w,dt);
 
   (*w->solid->update)(w->solid, dt);
}


/**
 * @fn static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos )
 *
 * @brief Weapon hit the pilot.
 *
 *    @param w Weapon involved in the collision.
 *    @param p Pilot that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 */
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer, Vector2d* pos )
{
   /* inform the ai it has been attacked, useless if player */
   if (!pilot_isPlayer(p)) {
      if ((player_target == p->id) || (RNGF() < 0.33)) { /* 33% chance */
         if ((w->parent == PLAYER_ID) &&
               (!pilot_isFlag(p,PILOT_HOSTILE) || (RNGF() < 0.5))) { /* 50% chance */
            faction_modPlayer( p->faction, -1 ); /* slowly lower faction */
            pilot_setFlag( p, PILOT_HOSTILE);
         }
         ai_attacked( p, w->parent );
      }
      spfx_add( outfit_spfx(w->outfit), pos->x, pos->y,
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_BACK );
   }
   else
      spfx_add( outfit_spfx(w->outfit), pos->x, pos->y,
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_FRONT );

   /* inform the ship that it should take some damage */
   pilot_hit( p, w->solid, w->parent, 
         outfit_damageType(w->outfit), outfit_damage(w->outfit) );
   /* no need for the weapon particle anymore */
   weapon_destroy(w,layer);
}


/**
 * @fn static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
 *       Vector2d pos[2], const double dt )
 *
 * @brief Weapon hit the pilot.
 *
 *    @param w Weapon involved in the collision.
 *    @param p Pilot that got hit.
 *    @param layer Layer to which the weapon belongs.
 *    @param pos Position of the hit.
 */
static void weapon_hitBeam( Weapon* w, Pilot* p, WeaponLayer layer,
      Vector2d pos[2], const double dt )
{
   (void) layer;

   /* inform the ai it has been attacked, useless if player */
   if (!pilot_isPlayer(p)) {
      if ((player_target == p->id) || (RNGF()*dt < 0.70)) { /* 70% chance per second */
         if ((w->parent == PLAYER_ID) &&
               (!pilot_isFlag(p,PILOT_HOSTILE) || (RNGF() < 0.5))) { /* 50% chance */
            faction_modPlayer( p->faction, -1 ); /* slowly lower faction */
            pilot_setFlag( p, PILOT_HOSTILE);
         }
         ai_attacked( p, w->parent );
      }

      if (w->lockon == -1.) { /* Code to signal create explosions. */
         spfx_add( outfit_spfx(w->outfit), pos[0].x, pos[0].y,
               VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_BACK );
         spfx_add( outfit_spfx(w->outfit), pos[1].x, pos[1].y,
               VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_BACK );
      }
   }
   else if (w->lockon == -1.) {
      spfx_add( outfit_spfx(w->outfit), pos[0].x, pos[0].y,
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_FRONT );
      spfx_add( outfit_spfx(w->outfit), pos[1].x, pos[1].y,
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_FRONT );
   }

   /* inform the ship that it should take some damage */
   pilot_hit( p, w->solid, w->parent, 
         outfit_damageType(w->outfit), outfit_damage(w->outfit)*dt );
}


/**
 * @fn static Weapon* weapon_create( const Outfit* outfit,
 *                    const double dir, const Vector2d* pos, const Vector2d* vel,
 *                    const unsigned int parent, const unsigned int target )
 *
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
   w = MALLOC_ONE(Weapon);
   w->faction = pilot_get(parent)->faction; /* non-changeable */
   w->parent = parent; /* non-changeable */
   w->target = target; /* non-changeable */
   w->outfit = outfit; /* non-changeable */
   w->update = weapon_update;
   w->think = NULL;
   w->status = WEAPON_STATUS_OK;

   switch (outfit->type) {

      /* Bolts treated together */
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_TURRET_BOLT:
         /* Only difference is the direction of fire */
         if ((outfit->type == OUTFIT_TYPE_TURRET_BOLT) && (w->parent!=w->target) &&
               (w->target != 0)) { /* Must have valid target */
            pilot_target = pilot_get(w->target);

            /* Get the distance */
            dist = vect_dist( pos, &pilot_target->solid->pos );

            /* Time for shots to reach that distance */
            t = dist / w->outfit->u.blt.speed;

            /* Position is calculated on where it should be */
            x = (pilot_target->solid->pos.x + pilot_target->solid->vel.x*t)
                  - (pos->x + vel->x*t);
            y = (pilot_target->solid->pos.y + pilot_target->solid->vel.y*t)
                  - (pos->y + vel->y*t);
            vect_cset( &v, x, y );

            rdir = VANGLE(v);
         }
         else /* fire straight */
            rdir = dir;

         rdir += NormalInverse( RNGF()*0.9 + 0.05 ) /* Get rid of extreme values */
               * outfit->u.blt.accuracy/2. * 1./180.*M_PI;
         if ((rdir > 2.*M_PI) || (rdir < 0.)) rdir = fmod(rdir, 2.*M_PI);

         mass = 1; /* Lasers are presumed to have unitary mass */
         vectcpy( &v, vel );
         vect_cadd( &v, outfit->u.blt.speed*cos(rdir), outfit->u.blt.speed*sin(rdir));
         w->timer = outfit->u.blt.range/outfit->u.blt.speed;
         w->solid = solid_create( mass, rdir, pos, &v );
         sound_playPos(w->outfit->u.blt.sound, 
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
         mass = 1.;
         w->solid = solid_create( mass, rdir, pos, NULL );
         w->think = think_beam;
         w->timer = outfit->u.bem.duration;
         break;

      /* Treat seekers together. */
      case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
      case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         mass = w->outfit->mass;
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
         sound_playPos(w->outfit->u.amm.sound,
               w->solid->pos.x + w->solid->vel.x,
               w->solid->pos.y + w->solid->vel.y);
         break;

      /* Dumb missiles are a mixture of missile and bolt */
      case OUTFIT_TYPE_MISSILE_DUMB_AMMO:
         mass = w->outfit->mass;
         w->timer = outfit->u.amm.duration;
         w->solid = solid_create( mass, dir, pos, vel );
         vect_pset( &w->solid->force, w->outfit->u.amm.thrust, dir );
         w->think = NULL; /* No AI */

         sound_playPos(w->outfit->u.amm.sound,
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
 * @fn void weapon_add( const Outfit* outfit, const double dir,
 *                      const Vector2d* pos, const Vector2d* vel,
 *                      unsigned int parent, unsigned int target )
 *
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

   if (!outfit_isWeapon(outfit) && 
         !outfit_isAmmo(outfit) && 
         !outfit_isTurret(outfit)) {
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
         ERR("Invalid WEAPON_LAYER specified");
         return;
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
 * @fn static void weapon_destroy( Weapon* w, WeaponLayer layer )
 *
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

   switch (layer) {
      case WEAPON_LAYER_BG:
         wlayer = wbackLayer;
         nlayer = &nwbackLayer;
         break;
      case WEAPON_LAYER_FG:
         wlayer = wfrontLayer;
         nlayer = &nwfrontLayer;
         break;
   }

   for (i=0; wlayer[i] != w; i++); /* get to the current position */
   weapon_free(wlayer[i]);
   wlayer[i] = NULL;
   (*nlayer)--;

   for ( ; i < (*nlayer); i++)
      wlayer[i] = wlayer[i+1];
}


/**
 * @fn static void weapon_free( Weapon* w )
 *
 * @brief Frees the weapon.
 *
 *    @param w Weapon to free.
 */
static void weapon_free( Weapon* w )
{
   solid_free(w->solid);
   free(w);
}

/**
 * @fn void weapon_clear (void)
 *
 * @brief Clears all the weapons, does NOT free the layers.
 */
void weapon_clear (void)
{
   int i;
   for (i=0; i < nwbackLayer; i++)
      weapon_free(wbackLayer[i]);
   nwbackLayer = 0;
   for (i=0; i < nwfrontLayer; i++)
      weapon_free(wfrontLayer[i]);                                          
   nwfrontLayer = 0;
}

/**
 * @fn void weapon_exit (void)
 *
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


