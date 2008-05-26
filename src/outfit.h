/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef OUTFIT_H
#  define OUTFIT_H


#include "opengl.h"
#include "sound.h"


/*
 * properties
 */
#define outfit_isProp(o,p)          ((o)->properties & p)
/* property flags */
#define OUTFIT_PROP_WEAP_SECONDARY  (1<<0)


/*
 * all the different outfit types
 */
typedef enum OutfitType_ {
   OUTFIT_TYPE_NULL,
   OUTFIT_TYPE_BOLT,
   OUTFIT_TYPE_BEAM,
   OUTFIT_TYPE_MISSILE_DUMB,
   OUTFIT_TYPE_MISSILE_DUMB_AMMO,
   OUTFIT_TYPE_MISSILE_SEEK,
   OUTFIT_TYPE_MISSILE_SEEK_AMMO,
   OUTFIT_TYPE_MISSILE_SEEK_SMART,
   OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO,
   OUTFIT_TYPE_MISSILE_SWARM,
   OUTFIT_TYPE_MISSILE_SWARM_AMMO,
   OUTFIT_TYPE_MISSILE_SWARM_SMART,
   OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO,
   OUTFIT_TYPE_TURRET_BOLT,
   OUTFIT_TYPE_TURRET_BEAM,
   OUTFIT_TYPE_MODIFCATION,
   OUTFIT_TYPE_AFTERBURNER,
   OUTFIT_TYPE_MAP,
   OUTFIT_TYPE_SENTINEL /* indicates last type */
} OutfitType;

typedef enum DamageType_ {
   DAMAGE_TYPE_NULL,
   DAMAGE_TYPE_ENERGY,
   DAMAGE_TYPE_KINETIC,
   DAMAGE_TYPE_ION,
   DAMAGE_TYPE_RADIATION
} DamageType;

/*
 * an outfit, depends radically on the type
 */
typedef struct Outfit_ {
   char* name;

   /* general specs */
   int max;
   int tech;
   int mass;

   /* store stuff */
   unsigned int price;
   char* description;

   glTexture* gfx_store; /* store graphic */

   int properties; /* properties stored bitwise */

   /* Type dependent */
   OutfitType type;
   union {
      struct { /* bolt */
         unsigned int delay; /* delay between shots */
         double speed; /* how fast it goes (not applicable to beam) */
         double range; /* how far it goes */
         double accuracy; /* desviation accuracy */
         double energy; /* energy usage */
         DamageType dtype; /* damage type */
         double damage; /* damage */


         glTexture* gfx_space; /* graphic */
         ALuint sound; /* sound to play */
         int spfx; /* special effect on hit */
      } blt;
      struct { /* beam */
         double range; /* how far it goes */
         glColour colour; /* beam colour */
         double energy; /* energy it drains */
         double dtype; /* damage type */
         double damage; /* damage */
      } bem;
      struct { /* launcher */
         unsigned int delay; /* delay between shots */
         char *ammo; /* the ammo to use */
      } lau;
      struct { /* ammo */
         unsigned int duration; /* duration */
         double speed; /* maximum speed */
         double turn; /* turn velocity */
         double thrust; /* acceleration */
         double energy; /* energy usage */
         DamageType dtype; /* damage type */
         double damage; /* damage */

         glTexture* gfx_space; /* graphic */
         ALuint sound; /* sound to play */
         int spfx; /* special effect on hit */

         unsigned int lockon; /* time it takes to lock on the target */
      } amm;
      struct { /* modification */
         /* movement */
         double thrust, turn, speed;
         
         /* health */
         double armour, armour_regen;
         double shield, shield_regen;
         double energy, energy_regen;
         double fuel;

         /* misc */
         int cargo; /* cargo space to add */
      } mod;
      struct { /* afterburner */
         double rumble; /* percent of rumble */
         ALuint sound; /* sound of the afterburner */
         double thrust_perc, thrust_abs; /* percent and absolute thrust bonus */
         double speed_perc, speed_abs; /* percent and absolute speed bonus */
         double energy; /* energy usage while active */
      } afb;
      struct { /* map */
         double radius; /* amount of systems to add */
      } map;
   } u;
} Outfit;


/*
 * misc
 */
void outfit_calcDamage( double *dshield, double *darmour,
      DamageType dtype, double dmg );


/*
 * get
 */
Outfit* outfit_get( const char* name );
char** outfit_getTech( int *n, const int *tech, const int techmax );
/* outfit types */
int outfit_isWeapon( const Outfit* o );
int outfit_isLauncher( const Outfit* o );
int outfit_isAmmo( const Outfit* o );
int outfit_isTurret( const Outfit* o );
int outfit_isMod( const Outfit* o );
int outfit_isAfterburner( const Outfit* o );
int outfit_isMap( const Outfit* o );
const char* outfit_getType( const Outfit* o );
const char* outfit_getTypeBroad( const Outfit* o );

/*
 * get data from outfit
 */
glTexture* outfit_gfx( const Outfit* o );
int outfit_spfx( const Outfit* o );
double outfit_damage( const Outfit* o );
DamageType outfit_damageType( const Outfit* o );
int outfit_delay( const Outfit* o );
double outfit_energy( const Outfit* o );

/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


#endif /* OUTFIT_H */
