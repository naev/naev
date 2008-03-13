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


#define weapon_isSmart(w)     (w->think)

#define VOICE_PRIORITY_BOLT   10 /* default */
#define VOICE_PRIORITY_AMMO   8  /* higher */


#define WEAPON_CHUNK    32


/*
 * pilot stuff
 */
extern Pilot** pilot_stack;
extern int pilots;
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

   unsigned int parent; /* pilot that shot it */
   unsigned int target; /* target to hit, only used by seeking things */
   const Outfit* outfit; /* related outfit that fired it or whatnot */

   unsigned int timer; /* mainly used to see when the weapon was fired */

   alVoice* voice; /* virtual voice */

   /* position update and render */
   void (*update)(struct Weapon_*, const double, WeaponLayer);
   void (*think)(struct Weapon_*, const double); /* for the smart missiles */

   double pid_last;
   double pid_int;
} Weapon;


/* behind pilots layer */
static Weapon** wbackLayer = NULL; /* behind pilots */
static int nwbackLayer = 0; /* number of elements */
static int mwbacklayer = 0; /* alloced memory size */
/* behind player layer */
static Weapon** wfrontLayer = NULL; /* infront of pilots, behind player */
static int nwfrontLayer = 0; /* number of elements */
static int mwfrontLayer = 0; /* alloced memory size */


/*
 * Prototypes
 */
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
static void think_smart( Weapon* w, const double dt );


/*
 * draws the minimap weapons (used in player.c)
 */
#define PIXEL(x,y)      \
   if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
         (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y))<rc)))   \
   glVertex2i((x),(y))
void weapon_minimap( const double res, const double w, const double h,
      const RadarShape shape )
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
 * pauses the weapon system
 */
void weapons_pause (void)
{
   int i;
   unsigned int t = SDL_GetTicks();

   /* adjust layer's time */
   for (i=0; i<nwbackLayer; i++)
      wbackLayer[i]->timer -= t;
   for (i=0; i<nwfrontLayer; i++)
      wfrontLayer[i]->timer -= t;
}
void weapons_unpause (void)
{
   int i;                            
   unsigned int t = SDL_GetTicks();

   /* adjust layer's time */
   for (i=0; i<nwbackLayer; i++) 
      wbackLayer[i]->timer += t;
   for (i=0; i<nwfrontLayer; i++) 
      wfrontLayer[i]->timer += t;
}
void weapons_delay( unsigned int delay )
{
   int i;
   for (i=0; i<nwbackLayer; i++)
      wbackLayer[i]->timer += delay;
   for (i=0; i<nwfrontLayer; i++)
      wfrontLayer[i]->timer += delay;
}


/*
 * seeker brain, you get what you pay for :)
 */
static void think_seeker( Weapon* w, const double dt )
{
   double diff;

   if (w->target == w->parent) return; /* no self shooting */

   Pilot* p = pilot_get(w->target); /* no null pilots */
   if (p==NULL) {
      limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
      return;
   }

   /* ammo isn't locked on yet */
   if (SDL_GetTicks() > (w->timer + w->outfit->u.amm.lockon)) {
   
      diff = angle_diff(w->solid->dir,
            vect_angle(&w->solid->pos, &p->solid->pos));
      w->solid->dir_vel = 10 * diff *  w->outfit->u.amm.turn; /* face the target */
      if (w->solid->dir_vel > w->outfit->u.amm.turn)
         w->solid->dir_vel = w->outfit->u.amm.turn;
      else if (w->solid->dir_vel < -w->outfit->u.amm.turn)
         w->solid->dir_vel = -w->outfit->u.amm.turn;
   }

   vect_pset( &w->solid->force, w->outfit->u.amm.thrust, w->solid->dir );

   limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
}
/*
 * smart seeker brain, much better at homing
 */
static void think_smart( Weapon* w, const double dt )
{
   Vector2d sv, tv;
   double t;

   if (w->target == w->parent) return; /* no self shooting */

   Pilot* p = pilot_get(w->target); /* no null pilots */
   if (p==NULL) {
      limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
      return;
   }

   /* ammo isn't locked on yet */
   if (SDL_GetTicks() > (w->timer + w->outfit->u.amm.lockon)) {

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
   vect_pset( &w->solid->force, w->outfit->u.amm.thrust, w->solid->dir );

   limit_speed( &w->solid->vel, w->outfit->u.amm.speed, dt );
}


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
            if (SDL_GetTicks() > (wlayer[i]->timer + wlayer[i]->outfit->u.amm.duration)) {
               weapon_destroy(wlayer[i],layer);
               continue;
            }
            break;

         case OUTFIT_TYPE_BOLT: /* check to see if exceeded distance */
         case OUTFIT_TYPE_TURRET_BOLT:
            if (SDL_GetTicks() > wlayer[i]->timer) {
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

   for (i=0; i<pilots; i++) {
      psx = pilot_stack[i]->tsx;
      psy = pilot_stack[i]->tsy;

      if (w->parent == pilot_stack[i]->id) continue; /* pilot is self */

      if ( (weapon_isSmart(w)) && (pilot_stack[i]->id == w->target) &&
            CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                  pilot_stack[i]->ship->gfx_space, psx, psy, &pilot_stack[i]->solid->pos)) {

         weapon_hit( w, pilot_stack[i], layer );
         return;
      }
      else if ( !weapon_isSmart(w) &&
            !areAllies(pilot_get(w->parent)->faction,pilot_stack[i]->faction) &&
            CollideSprite( gfx, wsx, wsy, &w->solid->pos,
                  pilot_stack[i]->ship->gfx_space, psx, psy, &pilot_stack[i]->solid->pos)) {

         weapon_hit( w, pilot_stack[i], layer );
         return;
      }
   }

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
   /* inform the ai it has been attacked, useless if  player */
   if (!pilot_isPlayer(p)) {
      if ((player_target == p->id) || (RNG(0,2) == 0)) {
         if ((w->parent == PLAYER_ID) &&
               (!pilot_isFlag(p,PILOT_HOSTILE) || (RNG(0,2) == 0))) {
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
         outfit_dmgShield(w->outfit), outfit_dmgShield(w->outfit) );
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
   double mass = 1; /* presume lasers have a mass of 1 */
   double rdir = dir; /* real direction (accuracy) */
   Weapon* w = MALLOC_ONE(Weapon);
   w->parent = parent; /* non-changeable */
   w->target = target; /* non-changeable */
   w->outfit = outfit; /* non-changeable */
   w->update = weapon_update;
   w->timer = SDL_GetTicks();
   w->think = NULL;

   switch (outfit->type) {
      case OUTFIT_TYPE_BOLT: /* needs "accuracy" and speed based on player */
         rdir += RNG(-outfit->u.blt.accuracy/2.,
               outfit->u.blt.accuracy/2.)/180.*M_PI;
         if ((rdir > 2.*M_PI) || (rdir < 0.)) rdir = fmod(rdir, 2.*M_PI);
         vectcpy( &v, vel );
         vect_cadd( &v, outfit->u.blt.speed*cos(rdir), outfit->u.blt.speed*sin(rdir));
         w->timer += 1000*(unsigned int)outfit->u.blt.range/outfit->u.blt.speed;
         w->solid = solid_create( mass, rdir, pos, &v );
         w->voice = sound_addVoice( VOICE_PRIORITY_BOLT,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.blt.sound, 0 );
         break;

      case OUTFIT_TYPE_MISSILE_SEEK_AMMO:
         mass = w->outfit->mass;
         w->solid = solid_create( mass, dir, pos, vel );
         w->think = think_seeker; /* eet's a seeker */
         w->voice = sound_addVoice( VOICE_PRIORITY_AMMO,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.amm.sound, 0 );
         break;

      case OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO:
         mass = w->outfit->mass;
         w->solid = solid_create( mass, dir, pos, vel );
         w->think = think_smart; /* smartass */
         w->voice = sound_addVoice( VOICE_PRIORITY_AMMO,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.amm.sound, 0 );
         break;

      case OUTFIT_TYPE_TURRET_BOLT:
         if (w->parent!=w->target)
            rdir = vect_angle(pos,&pilot_get(w->target)->solid->pos);
         rdir += RNG(-outfit->u.blt.accuracy/2.,
               outfit->u.blt.accuracy/2.)/180.*M_PI;
         if ((rdir > 2.*M_PI) || (rdir < 0.)) rdir = fmod(rdir, 2.*M_PI);
         vectcpy( &v, vel );
         vect_cadd( &v, outfit->u.blt.speed*cos(rdir), outfit->u.blt.speed*sin(rdir));
         w->timer += 1000*(unsigned int)outfit->u.blt.range/outfit->u.blt.speed;
         w->solid = solid_create( mass, rdir, pos, &v );
         w->voice = sound_addVoice( VOICE_PRIORITY_BOLT,
               w->solid->pos.x, w->solid->pos.y,
               w->solid->vel.x, w->solid->vel.y,  w->outfit->u.blt.sound, 0 );
         break;




      default: /* just dump it where the player is */
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


