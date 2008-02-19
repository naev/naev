/*
 * See Licensing and Copyright notice in naev.h
 */



#include "pilot.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "xml.h"

#include "naev.h"
#include "log.h"
#include "weapon.h"
#include "pack.h"
#include "spfx.h"
#include "rng.h"


#define XML_ID          "Fleets"  /* XML section identifier */
#define XML_FLEET       "fleet"

#define FLEET_DATA      "dat/fleet.xml"


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = PLAYER_ID;


/* id for special mission cargo */
static unsigned int mission_cargo_id = 0;


/* stack of pilots */
Pilot** pilot_stack = NULL; /* not static, used in player.c, weapon.c, pause.c and ai.c */
int pilots = 0; /* same */
static int mpilots = 0;

/*
 * stuff from player.c
 */
extern Pilot* player;
extern unsigned int player_crating;

/* stack of fleets */
static Fleet* fleet_stack = NULL;
static int nfleets = 0;


/*
 * prototyes
 */
/* external */
/* ai.c */
extern void ai_destroy( Pilot* p );
extern void ai_think( Pilot* pilot );
extern void ai_create( Pilot* pilot );
/* player.c */
extern void player_think( Pilot* pilot );
extern void player_brokeHyperspace (void);
extern double player_faceHyperspace (void);
extern void player_dead (void);
extern void player_destroyed (void);
extern int gui_load( const char *name );
/* internal */
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w, const unsigned int t );
static void pilot_update( Pilot* pilot, const double dt );
static void pilot_hyperspace( Pilot* pilot );
void pilot_render( Pilot* pilot ); /* externed in player.c */
static void pilot_calcStats( Pilot* pilot );
void pilot_free( Pilot* p );
static Fleet* fleet_parse( const xmlNodePtr parent );
static void pilot_dead( Pilot* p );
static int pilot_oquantity( Pilot* p, PilotOutfit* w );


/*
 * gets the next pilot based on id
 */
unsigned int pilot_getNext( const unsigned int id )
{
   /* binary search */
   int l,m,h;
   l = 0;
   h = pilots-1;
   while (l <= h) {
      m = (l+h)/2;
      if (pilot_stack[m]->id > id) h = m-1;
      else if (pilot_stack[m]->id < id) l = m+1;
      else break;
   }

   if (m == (pilots-1)) return PLAYER_ID;
   else return pilot_stack[m+1]->id;
}


/*
 * gets the nearest enemy to the pilot
 */
unsigned int pilot_getNearest( const Pilot* p )
{
   unsigned int tp;
   int i;
   double d, td;
   for (tp=0,d=0.,i=0; i<pilots; i++)
      if (areEnemies(p->faction, pilot_stack[i]->faction)) {
         td = vect_dist(&pilot_stack[i]->solid->pos, &p->solid->pos);
         if (!pilot_isDisabled(pilot_stack[i]) && ((!tp) || (td < d))) {
            d = td;
            tp = pilot_stack[i]->id;
         }
      }
   return tp;
}


/*
 * gets the nearest hostile enemy to the player
 */
unsigned pilot_getHostile (void)
{
   unsigned int tp;
   int i;                                                                 
   double d, td;
   for (tp=PLAYER_ID,d=0.,i=0; i<pilots; i++)
      if (pilot_isFlag(pilot_stack[i],PILOT_HOSTILE)) {
         td = vect_dist(&pilot_stack[i]->solid->pos, &player->solid->pos);
         if (!pilot_isDisabled(pilot_stack[i]) && ((tp==PLAYER_ID) || (td < d))) {
            d = td;
            tp = pilot_stack[i]->id;
         }
      }
   return tp;
}


/*
 * pulls a pilot out of the pilot_stack based on id
 */
Pilot* pilot_get( const unsigned int id )
{
   if (id==PLAYER_ID) return player; /* special case player */
   
   /* binary */
   int l,m,h;
   l = 0;
   h = pilots-1;
   while (l <= h) {
      m = (l+h)/2;
      if (pilot_stack[m]->id > id) h = m-1;
      else if (pilot_stack[m]->id < id) l = m+1;
      else return pilot_stack[m];
   }
   return NULL;
}


/*
 * tries to turn the pilot to face dir
 */
double pilot_face( Pilot* p, const float dir )
{
   double diff, turn;
   
   diff = angle_diff( p->solid->dir, dir );

   turn = -10.*diff;
   if (turn > 1.) turn = 1.;
   else if (turn < -1.) turn = -1.;

   p->solid->dir_vel = 0.;
   if (turn)
      p->solid->dir_vel -= p->turn * turn;

   return diff;
}


/*
 * returns the quantity of a pilot outfit
 */
static int pilot_oquantity( Pilot* p, PilotOutfit* w )
{
   return (outfit_isAmmo(w->outfit) && p->secondary) ?
      p->secondary->quantity : w->quantity ;
}


/*
 * gets pilot's free weapon space
 */
int pilot_freeSpace( Pilot* p )
{
   int i,s;
   
   s = p->ship->cap_weapon;
   for (i=0; i<p->noutfits; i++)
      s -= p->outfits[i].quantity * p->outfits[i].outfit->mass;
   
   return s;
}



/*
 * makes the pilot shoot
 *
 * @param p the pilot which is shooting
 * @param secondary whether they are shooting secondary weapons or primary weapons
 */
void pilot_shoot( Pilot* p, const unsigned int target, const int secondary )
{
   int i;

   if (!p->outfits) return; /* no outfits */

   if (!secondary) { /* primary weapons */

      for (i=0; i<p->noutfits; i++) /* cycles through outfits to find primary weapons */
         if (!outfit_isProp(p->outfits[i].outfit,OUTFIT_PROP_WEAP_SECONDARY))
            pilot_shootWeapon( p, &p->outfits[i], target );
   }
   else { /* secondary weapon */

      if (!p->secondary) return; /* no secondary weapon */
      pilot_shootWeapon( p, p->secondary, target );

   }
}
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w, const unsigned int t )
{
   int quantity, delay;
   
   /* will segfault when trying to launch with 0 ammo otherwise */
   quantity = pilot_oquantity(p,w);
   delay = outfit_delay(w->outfit);
   
   /* check to see if weapon is ready */
   if ((SDL_GetTicks() - w->timer) < (unsigned int)(delay / quantity)) return;

   /*
    * regular weapons
    */
   if (outfit_isWeapon(w->outfit) || (outfit_isTurret(w->outfit))) {
      
      /* different weapons, different behaviours */
      switch (w->outfit->type) {
         case OUTFIT_TYPE_TURRET_BOLT:
         case OUTFIT_TYPE_BOLT:

            /* enough energy? */
            if (outfit_energy(w->outfit) > p->energy) return;

            p->energy -= outfit_energy(w->outfit);
            weapon_add( w->outfit, p->solid->dir,
                  &p->solid->pos, &p->solid->vel, p->id, t );

            /* can't shoot it for a bit */      
            w->timer = SDL_GetTicks();
            break;

         default:
            break;
      }

   }

   /*
    * missile launchers
    *
    * @must be a secondary weapon
    * @shooter can't be the target - sanity check for the player
    */
   else if (outfit_isLauncher(w->outfit) && (w==p->secondary) && (p->id!=t)) {
      if (p->ammo && (p->ammo->quantity > 0)) {

         /* enough energy? */
         if (outfit_energy(w->outfit) > p->energy) return;
   
         p->energy -= outfit_energy(w->outfit);
         weapon_add( p->ammo->outfit, p->solid->dir,
               &p->solid->pos, &p->solid->vel, p->id, t );

         w->timer = SDL_GetTicks(); /* can't shoot it for a bit */
         p->ammo->quantity -= 1; /* we just shot it */
      }
   }
}


/*
 * damages the pilot
 */
void pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const double damage_shield, const double damage_armour )
{
   double dam_mod;

   if (p->shield-damage_shield > 0.) {
      p->shield -= damage_shield;
      dam_mod = damage_shield/p->shield_max;
   }
   else if (p->shield > 0.) { /* shields can take part of the blow */
      p->armour -= p->shield/damage_shield*damage_armour;
      p->shield = 0.;
      dam_mod = (damage_shield+damage_armour) / (p->shield_max+p->armour_max);
   }
   else if (p->armour-damage_armour > 0.) {
      p->armour -= damage_armour;
      dam_mod = damage_armour/p->armour_max;
   }
   else { /* officially dead */

      p->armour = 0.;
      dam_mod = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
         pilot_dead(p);

         /* adjust the combat rating based on pilot mass */
         if (shooter==PLAYER_ID) player_crating += MAX( 1, p->ship->mass/50 );
      }
   }

   /* knock back effect is dependent on both damage and mass of the weapon 
    * should probably get turned into a partial conservative collision */
   vect_cadd( &p->solid->vel,
         w->vel.x * (dam_mod/6. + w->mass/p->solid->mass/6.),
         w->vel.y * (dam_mod/6. + w->mass/p->solid->mass/6.) );
}


void pilot_dead( Pilot* p )
{
   /* basically just set timers */
   if (p->id==PLAYER_ID) player_dead();
   p->timer[0] = SDL_GetTicks(); /* no need for AI anymore */
   p->ptimer = p->timer[0] + 1000 +
      (unsigned int)sqrt(10*p->armour_max*p->shield_max);
   p->timer[1] = p->timer[0]; /* explosion timer */

   /* flag cleanup - fixes some issues */
   if (pilot_isFlag(p,PILOT_HYP_PREP)) pilot_rmFlag(p,PILOT_HYP_PREP);
   if (pilot_isFlag(p,PILOT_HYP_BEGIN)) pilot_rmFlag(p,PILOT_HYP_BEGIN);
   if (pilot_isFlag(p,PILOT_HYPERSPACE)) pilot_rmFlag(p,PILOT_HYPERSPACE);

   /* PILOT R OFFICIALLY DEADZ0R */
   pilot_setFlag(p,PILOT_DEAD);
}


/*
 * sets the pilot's secondary weapon based on it's name
 */
void pilot_setSecondary( Pilot* p, const char* secondary )
{
   int i;

   if (secondary == NULL) {
      p->secondary = NULL;
      p->ammo = NULL;
      return;
   }

   for (i=0; i<p->noutfits; i++) {
      if (strcmp(secondary, p->outfits[i].outfit->name)==0) {
         p->secondary = &p->outfits[i];
         pilot_setAmmo( p );
         return;
      }
   }

   WARN("attempted to set pilot '%s' secondary weapon to non-existing '%s'",
         p->name, secondary );
   p->secondary = NULL;
   p->ammo = NULL;
}


/*
 * sets the pilot's ammo based on their secondary weapon
 */
void pilot_setAmmo( Pilot* p )
{
   int i;
   char *name;

   if ((p->secondary == NULL) || !outfit_isLauncher(p->secondary->outfit)) {
      p->ammo = NULL;
      return;
   }

   name = p->secondary->outfit->u.lau.ammo;

   for (i=0; i<p->noutfits; i++)
      if (strcmp(p->outfits[i].outfit->name,name)==0) {
         p->ammo = p->outfits + i;
         return;
      }

   p->ammo = NULL;
}


/*
 * renders the pilot
 */
void pilot_render( Pilot* p )
{
   gl_blitSprite( p->ship->gfx_space,
         p->solid->pos.x, p->solid->pos.y,
         p->tsx, p->tsy, NULL );
}


/*
 * updates the Pilot
 */
static void pilot_update( Pilot* pilot, const double dt )
{
   unsigned int t, l;
   double a, px,py, vx,vy;

   if (pilot_isFlag(pilot,PILOT_DEAD)) {
      t = SDL_GetTicks();

      if (t > pilot->ptimer) {
         if (pilot->id==PLAYER_ID)
            player_destroyed();
         pilot_setFlag(pilot,PILOT_DELETE); /* will get deleted next frame */
         return;
      }
      
      if (!pilot_isFlag(pilot,PILOT_EXPLODED) && (t > pilot->ptimer - 200)) {
         spfx_add( spfx_get("ExpL"), 
               VX(pilot->solid->pos), VY(pilot->solid->pos),
               VX(pilot->solid->vel), VY(pilot->solid->vel), SPFX_LAYER_BACK );
         pilot_setFlag(pilot,PILOT_EXPLODED);
      }
      else if (t > pilot->timer[1]) {
         pilot->timer[1] = t + (unsigned int)(100
               *(double)(pilot->ptimer - pilot->timer[1])
               /(double)(pilot->ptimer - pilot->timer[0]));

         /* random position on ship */
         a = RNGF()*2.*M_PI;
         px = VX(pilot->solid->pos) +  cos(a)*RNGF()*pilot->ship->gfx_space->sw/2.;
         py = VY(pilot->solid->pos) +  sin(a)*RNGF()*pilot->ship->gfx_space->sh/2.;
         vx = VX(pilot->solid->vel);
         vy = VY(pilot->solid->vel);

         /* set explosions */
         l = (pilot->id==PLAYER_ID) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;
         if (RNGF() > 0.8) spfx_add( spfx_get("ExpM"), px, py, vx, vy, l );
         else spfx_add( spfx_get("ExpS"), px, py, vx, vy, l );
      }
   }
   else if (pilot->armour <= 0.) /* PWNED */
         pilot_dead(pilot);

   /* purpose fallthrough to get the movement like disabled */
   if ((pilot != player) && 
         (pilot->armour < PILOT_DISABLED_ARMOR*pilot->armour_max)) { /* disabled */
      pilot_setFlag(pilot,PILOT_DISABLED);
      vect_pset( &pilot->solid->vel, /* slowly brake */
         VMOD(pilot->solid->vel) * (1. - dt*0.10),
         VANGLE(pilot->solid->vel) );
      vectnull( &pilot->solid->force );
      pilot->solid->dir_vel = 0.; /* no more turning */

      /* update the solid */
      pilot->solid->update( pilot->solid, dt );
      gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
            pilot->ship->gfx_space, pilot->solid->dir );
      return;
   }

   /* still alive */
   else if (pilot->armour < pilot->armour_max) {
      pilot->armour += pilot->armour_regen * dt;
      pilot->energy += pilot->energy_regen * dt;
   }
   else {
      pilot->shield += pilot->shield_regen * dt;
      pilot->energy += pilot->energy_regen * dt;
   }

   if (pilot->armour > pilot->armour_max) pilot->armour = pilot->armour_max;
   if (pilot->shield > pilot->shield_max) pilot->shield = pilot->shield_max;
   if (pilot->energy > pilot->energy_max) pilot->energy = pilot->energy_max;

   /* update the solid */
   (*pilot->solid->update)( pilot->solid, dt );
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,  
         pilot->ship->gfx_space, pilot->solid->dir );

   if (!pilot_isFlag(pilot, PILOT_HYPERSPACE)) { /* limit the speed */
      
      if (pilot_isFlag(pilot, PILOT_AFTERBURNER) && /* must have enough energy left */
               (player->energy > pilot->afterburner->outfit->u.afb.energy * dt)) {
         limit_speed( &pilot->solid->vel,
               pilot->speed * pilot->afterburner->outfit->u.afb.speed_perc + 
               pilot->afterburner->outfit->u.afb.speed_abs, dt );
         pilot->energy -= pilot->afterburner->outfit->u.afb.energy * dt;
      }
      else
         limit_speed( &pilot->solid->vel, pilot->speed, dt );
   }
}


/*
 * pilot is actually getting ready or in hyperspace
 */
static void pilot_hyperspace( Pilot* p )
{
   double diff;

   if (pilot_isFlag(p, PILOT_HYPERSPACE)) { /* pilot is actually in hyperspace */

      if (SDL_GetTicks() > p->ptimer) {
         if (p == player) { /* player just broke hyperspace */
            player_brokeHyperspace();
         }
         else
            pilot_setFlag(p, PILOT_DELETE); /* set flag to delete pilot */
         return;
      }

      vect_pset( &p->solid->force, p->thrust * 3., p->solid->dir );
   }
   else if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {

      if (SDL_GetTicks() > p->ptimer) { /* engines ready */
         p->ptimer = SDL_GetTicks() + HYPERSPACE_FLY_DELAY;
         pilot_setFlag(p, PILOT_HYPERSPACE);
      }
   }
   else { /* pilot is getting ready for hyperspace */

      if (VMOD(p->solid->vel) > MIN_VEL_ERR) {
         diff = pilot_face( p, VANGLE(p->solid->vel) + M_PI );

         if (ABS(diff) < MAX_DIR_ERR) /* brake */
            vect_pset( &p->solid->force, p->thrust, p->solid->dir );

      }
      else {

         vectnull( &p->solid->force ); /* stop accel */


         /* player should actually face the system he's headed to */
         if (p==player) diff = player_faceHyperspace();
         else diff = pilot_face( p, VANGLE(p->solid->pos) );

         if (ABS(diff) < MAX_DIR_ERR) { /* we can now prepare the jump */
            p->solid->dir_vel = 0.;
            p->ptimer = SDL_GetTicks() + HYPERSPACE_ENGINE_DELAY;
            pilot_setFlag(p, PILOT_HYP_BEGIN);
         }
      }
   }
}


/*
 * adds an outfit to the pilot
 */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, int quantity )
{
   int i, q;
   char *s;

   q = quantity;

   for (i=0; i<pilot->noutfits; i++)
      if (strcmp(outfit->name, pilot->outfits[i].outfit->name)==0) {
         pilot->outfits[i].quantity += quantity;
         /* can't be over max */
         if (pilot->outfits[i].quantity > outfit->max) {
            q -= pilot->outfits[i].quantity - outfit->max;
            pilot->outfits[i].quantity = outfit->max;
         }
         pilot_calcStats(pilot);
         return q;
      }

   s = (pilot->secondary) ? pilot->secondary->outfit->name : NULL;
   pilot->outfits = realloc(pilot->outfits, (pilot->noutfits+1)*sizeof(PilotOutfit));
   pilot->outfits[pilot->noutfits].outfit = outfit;
   pilot->outfits[pilot->noutfits].quantity = quantity;
   /* can't be over max */
   if (pilot->outfits[pilot->noutfits].quantity > outfit->max) {
      q -= pilot->outfits[pilot->noutfits].quantity - outfit->max;
      pilot->outfits[i].quantity = outfit->max;
   }
   pilot->outfits[pilot->noutfits].timer = 0;
   (pilot->noutfits)++;

   if (outfit_isTurret(outfit)) /* used to speed up AI */
      pilot_setFlag(pilot, PILOT_HASTURRET);

   /* hack due to realloc possibility */
   pilot_setSecondary( pilot, s );

   pilot_calcStats(pilot);
   return q;
}


/*
 * removes an outfit from the pilot
 */
int pilot_rmOutfit( Pilot* pilot, Outfit* outfit, int quantity )
{
   int i, q;
   char* s;

   q = quantity;
   for (i=0; i<pilot->noutfits; i++)
      if (strcmp(outfit->name, pilot->outfits[i].outfit->name)==0) {
         pilot->outfits[i].quantity -= quantity;
         if (pilot->outfits[i].quantity <= 0) {

            /* we didn't actually remove the full amount */
            q += pilot->outfits[i].quantity;

            /* hack in case it reallocs - can happen even when shrinking */
            s = (pilot->secondary) ? pilot->secondary->outfit->name : NULL;
            /* clear it if it's the afterburner */
            if (&pilot->outfits[i] == pilot->afterburner)
               pilot->afterburner = NULL;

            /* remove the outfit */
            memmove( pilot->outfits+i, pilot->outfits+i+1,
                  sizeof(PilotOutfit) * (pilot->noutfits-i-1) );
            pilot->noutfits--;
            pilot->outfits = realloc( pilot->outfits,
                  sizeof(PilotOutfit) * (pilot->noutfits) );

            pilot_setSecondary( pilot, s );
         }
         return q;
      }
   WARN("Failure attempting to remove %d '%s' from pilot '%s'",
         quantity, outfit->name, pilot->name );
   return 0;
}


/*
 * returns all the outfits in nice text form
 */
char* pilot_getOutfits( Pilot* pilot )
{
   int i;
   char buf[64], *str;

   str = malloc(sizeof(char)*1024);
   buf[0] = '\0';
   /* first outfit */
   if (pilot->noutfits>0)
      snprintf( str, 1024, "%dx %s",
            pilot->outfits[0].quantity, pilot->outfits[0].outfit->name );
   /* rest of the outfits */
   for (i=1; i<pilot->noutfits; i++) {
      snprintf( buf, 64, ", %dx %s",
            pilot->outfits[i].quantity, pilot->outfits[i].outfit->name );
      strcat( str, buf );
   }

   return str;
}


/*
 * recalculates the pilot's stats based on his outfits
 */
static void pilot_calcStats( Pilot* pilot )
{
   int i;
   double q;
   Outfit* o;
   double ac, sc, ec; /* temporary health coeficients to set */
   /*
    * set up the basic stuff
    */
   /* movement */
   pilot->thrust = pilot->ship->thrust;
   pilot->turn = pilot->ship->turn;
   pilot->speed = pilot->ship->speed;
   /* health */
   ac = pilot->armour / pilot->armour_max;
   sc = pilot->shield / pilot->shield_max;
   ec = pilot->energy / pilot->energy_max;
   pilot->armour_max = pilot->ship->armour;
   pilot->shield_max = pilot->ship->shield;
   pilot->energy_max = pilot->ship->energy;
   pilot->armour_regen = pilot->ship->armour_regen;
   pilot->shield_regen = pilot->ship->shield_regen;
   pilot->energy_regen = pilot->ship->energy_regen;

   /*
    * now add outfit changes
    */
   for (i=0; i<pilot->noutfits; i++) {
      if (outfit_isMod(pilot->outfits[i].outfit)) {
         q = (double) pilot->outfits[i].quantity;
         o = pilot->outfits[i].outfit;

         /* movement */
         pilot->thrust += o->u.mod.thrust * q;
         pilot->turn += o->u.mod.turn * q;
         pilot->speed += o->u.mod.speed * q;
         /* health */
         pilot->armour_max += o->u.mod.armour * q;
         pilot->armour_regen += o->u.mod.armour_regen * q;
         pilot->shield_max += o->u.mod.shield * q;
         pilot->shield_regen += o->u.mod.shield_regen * q;
         pilot->energy_max += o->u.mod.energy * q;
         pilot->energy_regen += o->u.mod.energy_regen * q;
      }
      else if (outfit_isAfterburner(pilot->outfits[i].outfit)) { /* set afterburner */
         pilot->afterburner = &pilot->outfits[i];
      }
   }

   /* give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;
}


/*
 * pilot free cargo space
 */
int pilot_freeCargo( Pilot* p )
{
   return p->cargo_free;
}


/*
 * tries to add quantity of cargo to pilot, returns quantity actually added
 */
int pilot_addCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   int i, q;

   q = quantity;
   for (i=0; i<pilot->ncommodities; i++)
      if (!pilot->commodities[i].id && (pilot->commodities[i].commodity == cargo)) {
         if (pilot->cargo_free < quantity)
            q = pilot->cargo_free;
         pilot->commodities[i].quantity += q;
         pilot->cargo_free -= q;
         return q;
      }

   /* must add another one */
   pilot->commodities = realloc( pilot->commodities,
         sizeof(PilotCommodity) * (pilot->ncommodities+1));
   pilot->commodities[ pilot->ncommodities ].commodity = cargo;
   if (pilot->cargo_free < quantity)
      q = pilot->cargo_free;
   pilot->commodities[ pilot->ncommodities ].id = 0;
   pilot->commodities[ pilot->ncommodities ].quantity = q;
   pilot->cargo_free -= q;
   pilot->ncommodities++;

   return q;
}


/*
 * adds special mission cargo, can't sell it and such
 */
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   int q;
   q = quantity;

   pilot->commodities = realloc( pilot->commodities,
         sizeof(PilotCommodity) * (pilot->ncommodities+1));
   pilot->commodities[ pilot->ncommodities ].commodity = cargo;
   if (pilot->cargo_free < quantity)
      q = pilot->cargo_free;
   pilot->commodities[ pilot->ncommodities ].id = ++mission_cargo_id;
   pilot->commodities[ pilot->ncommodities ].quantity = q;                
   pilot->cargo_free -= q;
   pilot->ncommodities++;

   return pilot->commodities[ pilot->ncommodities-1 ].id;
}
int pilot_rmMissionCargo( Pilot* pilot, unsigned int cargo_id )
{
   int i;

   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].id == cargo_id)
         break;
   
   if (i>=pilot->ncommodities)
      return 1;

   /* remove cargo */
   pilot->cargo_free += pilot->commodities[i].quantity;
   memmove( pilot->commodities+i, pilot->commodities+i+1,
         sizeof(PilotCommodity) * (pilot->ncommodities-i-1) );
   pilot->ncommodities--;
   pilot->commodities = realloc( pilot->commodities,
         sizeof(PilotCommodity) * pilot->ncommodities );

   return 0;
}



/*
 * tries to get rid of quantity cargo from pilot, returns quantity actually removed
 */
int pilot_rmCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   int i, q;

   q = quantity;
   for (i=0; i<pilot->ncommodities; i++)
      if (!pilot->commodities[i].id && (pilot->commodities[i].commodity == cargo)) {
         if (quantity >= pilot->commodities[i].quantity) {
            q = pilot->commodities[i].quantity;

            /* remove cargo */
            memmove( pilot->commodities+i, pilot->commodities+i+1,
                  sizeof(PilotCommodity) * (pilot->ncommodities-i-1) );
            pilot->ncommodities--;
            pilot->commodities = realloc( pilot->commodities,
                  sizeof(PilotCommodity) * pilot->ncommodities );
         }
         else
            pilot->commodities[i].quantity -= q;
         pilot->cargo_free += q;
         return q;
      }
   return 0;
}


/*
 * Initialize pilot
 *
 * @ ship : ship pilot will be flying
 * @ name : pilot's name, if NULL ship's name will be used
 * @ dir : initial direction to face (radians)
 * @ vel : initial velocity
 * @ pos : initial position
 * @ flags : used for tweaking the pilot
 */
void pilot_init( Pilot* pilot, Ship* ship, char* name, int faction, AI_Profile* ai,
      const double dir, const Vector2d* pos, const Vector2d* vel, const int flags )
{
   if (flags & PILOT_PLAYER) /* player is ID 0 */
      pilot->id = PLAYER_ID;
   else
      pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? ship->name : name );

   /* faction */
   pilot->faction = faction;

   /* AI */
   pilot->ai = ai;
   pilot->tcontrol = 0;
   pilot->flags = 0;

   /* solid */
   pilot->solid = solid_create(ship->mass, dir, pos, vel);

   /* initially idle */
   pilot->task = NULL;

   /* outfits */
   pilot->outfits = NULL;
   pilot->secondary = NULL;
   pilot->ammo = NULL;
   pilot->afterburner = NULL;
   ShipOutfit* so;
   if (ship->outfit) {
      pilot->noutfits = 0;
      for (so=ship->outfit; so; so=so->next) {
         pilot->outfits = realloc(pilot->outfits, (pilot->noutfits+1)*sizeof(PilotOutfit));
         pilot->outfits[pilot->noutfits].outfit = so->data;
         pilot->outfits[pilot->noutfits].quantity = so->quantity;
         pilot->outfits[pilot->noutfits].timer = 0;
         (pilot->noutfits)++;
         if (outfit_isTurret(so->data)) /* used to speed up AI */
            pilot_setFlag(pilot, PILOT_HASTURRET);
      }
   }

   /* set the pilot stats based on his ship and outfits */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot_calcStats(pilot);

   /* cargo */
   pilot->credits = 0;
   pilot->commodities = NULL;
   pilot->ncommodities = 0;
   pilot->cargo_free = pilot->ship->cap_cargo;

   /* set flags and functions */
   if (flags & PILOT_PLAYER) {
      pilot->think = player_think; /* players don't need to think! :P */
      pilot->render = NULL; /* render will get called from player_think */
      pilot_setFlag(pilot,PILOT_PLAYER); /* it is a player! */
      player = pilot;
      gui_load( pilot->ship->gui ); /* load the gui */
   }
   else {
      pilot->think = ai_think;
      pilot->render = pilot_render;
      ai_create(pilot); /* will run the create function in the ai */
   }

   /* all update the same way */
   pilot->update = pilot_update;
}


/*
 * Creates a new pilot
 *
 * see pilot_init for parameters
 *
 * returns pilot's id
 */
unsigned int pilot_create( Ship* ship, char* name, int faction, AI_Profile* ai,
      const double dir, const Vector2d* pos, const Vector2d* vel, const int flags )
{
   Pilot **tp, *dyn;
   
   dyn = MALLOC_ONE(Pilot);
   if (dyn == NULL) {
      WARN("Unable to allocate memory");
      return 0;;
   }
   pilot_init( dyn, ship, name, faction, ai, dir, pos, vel, flags );

   if (flags & PILOT_PLAYER) { /* player */
      if (!pilot_stack) {
         pilot_stack = MALLOC_ONE(Pilot*);
         pilots = 1;
         mpilots = 1;
      }
      pilot_stack[0] = dyn;
   }
   else { /* add to the stack */

      pilots++; /* there's a new pilot */

      if (pilots >= mpilots) { /* needs to grow */
         mpilots += 20; /* grow 20 at a time */
         tp = pilot_stack;
         pilot_stack = realloc( pilot_stack, mpilots*sizeof(Pilot*) );
         if ((pilot_stack != tp) && player) /* take into account possible mem move */
            player = pilot_stack[0];
      }

      pilot_stack[pilots-1] = dyn;
   }

   return dyn->id;
}


/*
 * copies src pilot to dest
 */
Pilot* pilot_copy( Pilot* src )
{
   Pilot *dest = malloc(sizeof(Pilot));

   memcpy( dest, src, sizeof(Pilot) );
   if (src->name) dest->name = strdup(src->name);

   /* solid */
   dest->solid = malloc(sizeof(Solid));
   memcpy( dest->solid, src->solid, sizeof(Solid) );

   /* copy outfits */
   dest->outfits = malloc( sizeof(PilotOutfit)*src->noutfits );
   memcpy( dest->outfits, src->outfits,
         sizeof(PilotOutfit)*src->noutfits );
   dest->secondary = NULL;
   dest->ammo = NULL;
   dest->afterburner = NULL;

   /* copy commodities */
   dest->commodities = malloc( sizeof(PilotCommodity)*src->ncommodities );
   memcpy( dest->commodities, src->commodities,
         sizeof(PilotCommodity)*src->ncommodities);

   /* ai is not copied */
   dest->task = NULL;

   /* will set afterburner and correct stats */
   pilot_calcStats( dest );

   return dest;
}


/*
 * frees and cleans up a pilot
 */
void pilot_free( Pilot* p )
{
   if (player==p) player = NULL;
   solid_free(p->solid);
   if (p->outfits) free(p->outfits);
   free(p->name);
   if (p->commodities) free(p->commodities);
   ai_destroy(p);
   free(p);
}


/*
 * destroys pilot from stack
 */
void pilot_destroy(Pilot* p)
{
   int i;

   for (i=0; i < pilots; i++)
      if (pilot_stack[i]==p)
         break;
   pilots--;

   while (i < pilots) {
      pilot_stack[i] = pilot_stack[i+1];
      i++;
   }

   pilot_free(p);
}


/*
 * frees the pilots
 */
void pilots_free (void)
{
   int i;
   if (player) pilot_free(player);
   for (i=1; i < pilots; i++)
      pilot_free(pilot_stack[i]);
   free(pilot_stack);
   pilot_stack = NULL;
   pilots = 0;
}


/*
 * cleans up the pilots - leaves the player
 */
void pilots_clean (void)
{
   int i;
   for (i=1; i < pilots; i++)
      pilot_free(pilot_stack[i]);
   pilots = 1;
}


/*
 * updates all the pilots
 */
void pilots_update( double dt )
{
   int i;
   for ( i=0; i < pilots; i++ ) {
      if (pilot_stack[i]->think && /* think */
            !pilot_isDisabled(pilot_stack[i])) {

         /* hyperspace gets special treatment */
         if (pilot_isFlag(pilot_stack[i], PILOT_HYP_PREP))
            pilot_hyperspace(pilot_stack[i]);
         else
            pilot_stack[i]->think(pilot_stack[i]);

      }
      if (pilot_stack[i]->update) { /* update */
         if (pilot_isFlag(pilot_stack[i], PILOT_DELETE))
            pilot_destroy(pilot_stack[i]);
         else
            pilot_stack[i]->update( pilot_stack[i], dt );
      }
   }
}


/*
 * renders all the pilots
 */
void pilots_render (void)
{
   int i;
   for (i=1; i<pilots; i++) /* skip player */
      if (pilot_stack[i]->render) /* render */
         pilot_stack[i]->render(pilot_stack[i]);
}


/* returns the fleet based on name */
Fleet* fleet_get( const char* name )
{
   int i;
   for (i=0; i<nfleets; i++)
      if (strcmp(name, fleet_stack[i].name)==0)
         return fleet_stack+i;
   return NULL;
}


/* parses the fleet node */
static Fleet* fleet_parse( const xmlNodePtr parent )
{
   xmlNodePtr cur, node;
   FleetPilot* pilot;
   char* c;
   node  = parent->xmlChildrenNode;

   Fleet* temp = CALLOC_ONE(Fleet);
   temp->faction = -1;

   temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name"); /* already mallocs */
   if (temp->name == NULL) WARN("Fleet in "FLEET_DATA" has invalid or no name");

   do { /* load all the data */
      if (strcmp((char*)node->name,"faction")==0)
         temp->faction = faction_get((char*)node->children->content);
      else if (strcmp((char*)node->name,"ai")==0)
         temp->ai = ai_getProfile((char*)node->children->content);
      else if (strcmp((char*)node->name,"pilots")==0) {
         cur = node->children;     
         do {
            if (strcmp((char*)cur->name,"pilot")==0) {
               temp->npilots++; /* pilot count */
               pilot = MALLOC_ONE(FleetPilot);

               /* name is not obligatory, will only override ship name */
               c = (char*)xmlGetProp(cur,(xmlChar*)"name"); /* mallocs */
               pilot->name = c; /* no need to free here though */

               pilot->ship = ship_get((char*)cur->children->content);
               if (pilot->ship == NULL)
                  WARN("Pilot %s in Fleet %s has null ship", pilot->name, temp->name);

               c = (char*)xmlGetProp(cur,(xmlChar*)"chance"); /* mallocs */
               pilot->chance = atoi(c);
               if (pilot->chance == 0)
                  WARN("Pilot %s in Fleet %s has 0%% chance of appearing",
                     pilot->name, temp->name );
               if (c) free(c); /* free the external malloc */

               /* memory silliness */
               temp->pilots = realloc(temp->pilots, sizeof(FleetPilot)*temp->npilots);
               memcpy(temp->pilots+(temp->npilots-1), pilot, sizeof(FleetPilot));
               free(pilot);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

#define MELEMENT(o,s)      if (o) WARN("Fleet '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->ai==NULL,"ai");
   MELEMENT(temp->faction==-1,"faction");
   MELEMENT(temp->pilots==NULL,"pilots");
#undef MELEMENT

   return temp;
}


/* loads the fleets */
int fleet_load (void)
{
   uint32_t bufsize;
   char *buf = pack_readfile(DATA, FLEET_DATA, &bufsize);

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   Fleet* temp = NULL;

   node = doc->xmlChildrenNode; /* Ships node */
   if (strcmp((char*)node->name,XML_ID)) {
      ERR("Malformed "FLEET_DATA" file: missing root element '"XML_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first ship node */
   if (node == NULL) {
      ERR("Malformed "FLEET_DATA" file: does not contain elements");
      return -1;
   }

   do {  
      if (node->type ==XML_NODE_START &&         
            strcmp((char*)node->name,XML_FLEET)==0) {
         temp = fleet_parse(node);
         fleet_stack = realloc(fleet_stack, sizeof(Fleet)*(++nfleets));
         memcpy(fleet_stack+nfleets-1, temp, sizeof(Fleet));
         free(temp);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Fleet%s", nfleets, (nfleets==1) ? "" : "s" );

   return 0;
}


/* frees the fleets */
void fleet_free (void)
{
   int i,j;
   if (fleet_stack != NULL) {
      for (i=0; i<nfleets; i++) {
         for (j=0; j<fleet_stack[i].npilots; j++)
            if (fleet_stack[i].pilots[j].name)
               free(fleet_stack[i].pilots[j].name);
         free(fleet_stack[i].name);
         free(fleet_stack[i].pilots);
      }
      free(fleet_stack);
      fleet_stack = NULL;
   }
   nfleets = 0;
}

