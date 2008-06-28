/*
 * See Licensing and Copyright notice in naev.h
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


#define weapon_isSmart(w)     (w->think != NULL)

#define VOICE_PRIORITY_BOLT   10 /* default */
#define VOICE_PRIORITY_AMMO   8  /* higher */
#define VOICE_PRIORITY_BEAM   6  /* even higher */

#define WEAPON_CHUNK          128 /* Size to increase array with */

/* Weapon status */
#define WEAPON_STATUS_OK         0 /* weapon is fine */
#define WEAPON_STATUS_JAMMED     1 /* Got jammed */
#define WEAPON_STATUS_UNJAMMED   2 /* Survived jamming */


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


typedef struct Weapon_ {
   Solid* solid; /* actually has its own solid :) */

   unsigned int faction; /* faction of pilot that shot it */
   unsigned int parent; /* pilot that shot it */
   unsigned int target; /* target to hit, only used by seeking things */
   const Outfit* outfit; /* related outfit that fired it or whatnot */

   double lockon; /* some weapons have a lockon delay */
   double timer; /* mainly used to see when the weapon was fired */

   alVoice* voice; /* virtual voice */

   /* position update and render */
   void (*update)(struct Weapon_*, const double, WeaponLayer);
   void (*think)(struct Weapon_*, const double); /* for the smart missiles */

   char status; /* Weapon status - to check for jamming */
} Weapon;


/* behind pilot_nstack layer */
static Weapon** wbackLayer = NULL; /* behind pilot_nstack */
static int nwbackLayer = 0; /* number of elements */
static int mwbacklayer = 0; /* alloced memory size */
/* behind player layer */
static Weapon** wfrontLayer = NULL; /* infront of pilot_nstack, behind player */
static int nwfrontLayer = 0; /* number of elements */
static int mwfrontLayer = 0; /* alloced memory size */


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
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer );
static void weapon_destroy( Weapon* w, WeaponLayer layer );
static void weapon_free( Weapon* w );
/* think */
static void think_seeker( Weapon* w, const double dt );
/*static void think_smart( Weapon* w, const double dt );*/
/* externed */
void weapon_minimap( const double res, const double w,
      const double h, const RadarShape shape );


/*
 * draws the minimap weapons (used in player.c)
 */
#define PIXEL(x,y)      \
   if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
         (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y))<rc)))   \
   glVertex2i((x),(y))
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


/*
 * seeker brain, you get what you pay for :)
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


/*
 * smart seeker brain, much better at homing
 */
#if 0
static void think_smart( Weapon* w, const double dt )
{
   Vector2d sv, tv;
   double t;

   if (w->target == w->parent) return; /* no self shooting */

   Pilot* p = pilot_get(w->target); /* no null pilot_nstack */
   if (p==NULL) {
      limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
      return;
   }

   /* ammo isn't locked on yet */
   if (w->lockon < 0.) {

      vect_cset( &tv, VX(p->solid->pos) + dt*VX(p->solid->vel),
            VY(p->solid->pos) + dt*VY(p->solid->vel));
      vect_cset( &sv, VX(w->solid->pos) + dt*VX(w->solid->vel),
            VY(w->solid->pos) + dt*VY(w->solid->vel));
      t = -angle_diff(w->solid->dir, vect_angle(&tv, &sv));

      w->solid->dir_vel = t * w->outfit->u.amm.turn; /* face the target */

      if (w->solid->dir_vel > w->outfit->u.amm.turn)
         w->solid->dir_vel = w->outfit->u.amm.turn;
      else if (w->solid->dir_vel < -w->outfit->u.amm.turn)
         w->solid->dir_vel = -w->outfit->u.amm.turn;
   }
   vect_pset( &w->solid->vel, w->outfit->u.amm.speed, w->solid->dir );

   limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
}
#endif


/*
 * updates all the weapon layers
 */
void weapons_update( const double dt )
{
   weapons_updateLayer(dt,WEAPON_LAYER_BG);
   weapons_updateLayer(dt,WEAPON_LAYER_FG);
}


/*
 * updates all the weapons in the layer
 */
static void weapons_updateLayer( const double dt, const WeaponLayer layer )
{
   Weapon** wlayer;
   int* nlayer;

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

   int i;
   Weapon* w;
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
            wlayer[i]->timer -= dt;
            if (wlayer[i]->timer < 0.) {
               weapon_destroy(wlayer[i],layer);
               continue;
            }
            break;

         default:
            break;
      }
      weapon_update(wlayer[i],dt,layer);

      /* if the weapon has been deleted we have to hold back one */
      if (w != wlayer[i]) i--;
   }
}


/*
 * renders all the weapons
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


/*
 * renders the weapon
 */
static void weapon_render( const Weapon* w )
{
   int sx, sy;
   glTexture *gfx;

   gfx = outfit_gfx(w->outfit);

   /* get the sprite corresponding to the direction facing */
   gl_getSpriteFromDir( &sx, &sy, gfx, w->solid->dir );

   gl_blitSprite( gfx, w->solid->pos.x, w->solid->pos.y, sx, sy, NULL );
}


/*
 * updates the weapon
 */
static void weapon_update( Weapon* w, const double dt, WeaponLayer layer )
{
   int i, wsx,wsy, psx,psy;
   glTexture *gfx;

   gfx = outfit_gfx(w->outfit);
   gl_getSpriteFromDir( &wsx, &wsy, gfx, w->solid->dir );

   for (i=0; i<pilot_nstack; i++) {

      /* check for player to exists */
      if ((i==0) && (player==NULL)) continue;

      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      /* smart weapons only collide with their target */
      if ( (weapon_isSmart(w)) && (pilot_stack[i]->id == w->target) &&
            CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                  pilot_stack[i]->ship->gfx_space, psx, psy, &pilot_stack[i]->solid->pos)) {

         weapon_hit( w, pilot_stack[i], layer );
         return;
      }
      /* dump weapons hit anything not of the same faction */
      if ( !weapon_isSmart(w) &&
            !areAllies(w->faction,pilot_stack[i]->faction) &&
            CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                  pilot_stack[i]->ship->gfx_space, psx, psy, &pilot_stack[i]->solid->pos)) {

         weapon_hit( w, pilot_stack[i], layer );
         return;
      }
   }

   /* smart weapons also get to think their next move */
   if (weapon_isSmart(w)) (*w->think)(w,dt);
   
   (*w->solid->update)(w->solid, dt);

   /* update the sound */
   if (w->voice)
      voice_update( w->voice, w->solid->pos.x, w->solid->pos.y,
            w->solid->vel.x, w->solid->vel.y );
}


/*
 * weapon hit the player
 */
static void weapon_hit( Weapon* w, Pilot* p, WeaponLayer layer )
{
   /* inform the ai it has been attacked, useless if player */
   if (!pilot_isPlayer(p)) {
      if ((player_target == p->id) || (RNG(0,2) == 0)) { /* 33% chance */
         if ((w->parent == PLAYER_ID) &&
               (!pilot_isFlag(p,PILOT_HOSTILE) || (RNG(0,1) == 0))) { /* 50% chance */
            faction_modPlayer( p->faction, -1 ); /* slowly lower faction */
            pilot_setFlag( p, PILOT_HOSTILE);
         }
         ai_attacked( p, w->parent );
      }
      spfx_add( outfit_spfx(w->outfit),
            VX(w->solid->pos), VY(w->solid->pos),
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_BACK );
   }
   else
      spfx_add( outfit_spfx(w->outfit),
            VX(w->solid->pos), VY(w->solid->pos),
            VX(p->solid->vel), VY(p->solid->vel), SPFX_LAYER_FRONT );

   /* inform the ship that it should take some damage */
   pilot_hit( p, w->solid, w->parent, 
         outfit_damageType(w->outfit), outfit_damage(w->outfit) );
   /* no need for the weapon particle anymore */
   weapon_destroy(w,layer);
}


/*
 * creates a new weapon
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
         w->voice = sound_addVoice( VOICE_PRIORITY_BOLT,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.blt.sound, 0 );
         break;


      /* Treat seekers together */
      case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
      case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         mass = w->outfit->mass;
         w->lockon = outfit->u.amm.lockon;
         w->timer = outfit->u.amm.duration;
         w->solid = solid_create( mass, dir, pos, vel );
         w->voice = sound_addVoice( VOICE_PRIORITY_AMMO,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.amm.sound, 0 );

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
         break;


      /* just dump it where the player is */
      default:
         w->voice = NULL;
         w->solid = solid_create( mass, dir, pos, vel );
         break;
   }

   return w;
}


/*
 * adds a new weapon
 */
void weapon_add( const Outfit* outfit, const double dir,
      const Vector2d* pos, const Vector2d* vel,
      unsigned int parent, unsigned int target )
{
   if (!outfit_isWeapon(outfit) && 
         !outfit_isAmmo(outfit) && 
         !outfit_isTurret(outfit)) {
      ERR("Trying to create a Weapon from a non-Weapon type Outfit");
      return;
   }

   WeaponLayer layer = (parent==PLAYER_ID) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG;
   Weapon* w = weapon_create( outfit, dir, pos, vel, parent, target );

   /* set the proper layer */
   Weapon** curLayer = NULL;
   int *mLayer = NULL;
   int *nLayer = NULL;
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


/*
 * destroys the weapon
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


/*
 * clears the weapon
 */
static void weapon_free( Weapon* w )
{
   sound_delVoice( w->voice );
   solid_free(w->solid);
   free(w);
}

/*
 * clears all the weapons, does NOT free the layers
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

void weapon_exit (void)
{
   weapon_clear();
   free(wbackLayer);
   free(wfrontLayer);
}


