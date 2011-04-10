/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef OUTFIT_H
#  define OUTFIT_H


#include "opengl.h"
#include "sound.h"
#include "economy.h"


/*
 * properties
 */
#define outfit_isProp(o,p)          ((o)->properties & p) /**< Checks an outfit for property. */
/* property flags */
#define OUTFIT_PROP_WEAP_SECONDARY     (1<<0) /**< Is a secondary weapon? */
#define OUTFIT_PROP_WEAP_SPIN          (1<<1) /**< Should weapon spin around? */
#define OUTFIT_PROP_WEAP_BLOWUP_ARMOUR (1<<2) /**< Weapon blows up (armour spfx)
                                                   when timer is up. */
#define OUTFIT_PROP_WEAP_BLOWUP_SHIELD (1<<3) /**< Weapon blows up (shield spfx)
                                                   when timer is up. */


/**
 * @brief Represents ship statistics, properties ship can use.
 *
 * These values for outfits/ships are in percent, so 25 would be +25%,
 *  -25 would be -25% and so on.
 *
 * However for pilots these are normalized and centered around 1 so they are
 *  in the [0:2] range, with 1. being default. This value then modulates the
 *  stat's base value.
 *
 * Example:
 *  0.7 would lower by 30% the base value.
 *  1.2 would increase by 20% the base value.
 */
typedef struct ShipStats_ {
#if 0
   /* Corvette type. */
   double afterburner_energy; /**< Energy used by afterburner. */

   /* Carrier type. */
   double fighterbay_cpu; /**< CPU usage by fighter bays. */
   double fighterbay_rate; /**< Launch rate of fighter bay. */
#endif

   /* Freighter-type. */
   double jump_delay; /**< Modulates the jump delay. */
   double jump_range; /**< Distance from a jump point it can initiate jump from. */
   double cargo_inertia; /**< Lowers the effect of cargo mass. */

   /* Scout type. */
   double jam_range; /**< Range of jammer effect. */
   double ew_hide;   /**< Electronic warfare hide modifier. */
   double ew_detect; /**< Electronic warfare detection modifier. */

   /* Military type. */
   double heat_dissipation; /**< Global ship dissipation. */

   /* Bomber type. */
   double launch_rate; /**< Fire rate of launchers. */ /* TODO */
   double launch_range; /**< Range of launchers. */ /* TODO */
   double jam_counter; /**< Ability to avoid jamming. */ /* TODO */
   double ammo_capacity; /**< Capacity of launchers. */ /* TODO */

   /* Fighter/Corvette type. */
   double heat_forward; /**< Heat of forward mounts. */
   double damage_forward; /**< Damage of forward mounts. */
   double firerate_forward; /**< Rate of fire of forward mounts. */
   double energy_forward; /**< Consumption rate of forward mounts. */

   /* Destroyer/Cruiser type. */
   double heat_turret; /**< Heat of turrets. */
   double damage_turret; /**< Damage of turrets. */
   double firerate_turret; /**< Rate of fire of turrets. */
   double energy_turret; /**< Consumption rate of turrets. */

   /* Misc. */
   double nebula_damage; /**< Resistance of nebula. */
} ShipStats;


/*
 * Needed because some outfittypes call other outfits.
 */
struct Outfit_;


/**
 * @brief Different types of existing outfits.
 *
 * Outfits are organized by the order here
 */
typedef enum OutfitType_ {
   OUTFIT_TYPE_NULL, /**< Null type. */
   OUTFIT_TYPE_BOLT, /**< Fixed bolt cannon. */
   OUTFIT_TYPE_BEAM, /**< Fixed beam cannon. */
   OUTFIT_TYPE_TURRET_BOLT, /**< Rotary bolt turret. */
   OUTFIT_TYPE_TURRET_BEAM, /**< Rotary beam turret. */
   OUTFIT_TYPE_LAUNCHER, /**< Launcher. */
   OUTFIT_TYPE_AMMO, /**< Launcher ammo. */
   OUTFIT_TYPE_TURRET_LAUNCHER, /**< Turret launcher. */
   OUTFIT_TYPE_TURRET_AMMO, /**< Turrent launcher ammo. */
   OUTFIT_TYPE_MODIFCATION, /**< Modifies the ship base features. */
   OUTFIT_TYPE_AFTERBURNER, /**< Gives the ship afterburn capability. */
   OUTFIT_TYPE_JAMMER, /**< Used to nullify seeker missiles. */
   OUTFIT_TYPE_FIGHTER_BAY, /**< Contains other ships. */
   OUTFIT_TYPE_FIGHTER, /**< Ship contained in FIGHTER_BAY. */
   OUTFIT_TYPE_MAP, /**< Gives the player more knowledge about systems. */
   OUTFIT_TYPE_GUI, /**< GUI for the player. */
   OUTFIT_TYPE_LICENSE, /**< License that allows player to buy special stuff. */
   OUTFIT_TYPE_SENTINEL /**< indicates last type */
} OutfitType;


/**
 * @brief Different types of damage.
 */
typedef enum DamageType_ {
   DAMAGE_TYPE_NULL, /**< NULL */
   DAMAGE_TYPE_ENERGY, /**< Energy-based weapons. */
   DAMAGE_TYPE_KINETIC, /**< Physic impact weapons. */
   DAMAGE_TYPE_ION, /**< Ion-based weapons. */
   DAMAGE_TYPE_RADIATION, /**< Radioactive weapons. */
   DAMAGE_TYPE_EMP /**< Electromagnetic pulse weapons. */
} DamageType;


/**
 * @brief Outfit slot types.
 */
typedef enum OutfitSlotType_ {
   OUTFIT_SLOT_NULL, /**< Invalid slot type. */
   OUTFIT_SLOT_NA, /**< Slot type not applicable. */
   OUTFIT_SLOT_STRUCTURE, /**< Low energy slot. */
   OUTFIT_SLOT_UTILITY, /**< Medium energy slot. */
   OUTFIT_SLOT_WEAPON /**< High energy slot. */
} OutfitSlotType;


/**
 * @brief Outfit slot sizes.
 */
typedef enum OutfitSlotSize_ {
   OUTFIT_SLOT_SIZE_NA, /**< Not applicable slot size. */
   OUTFIT_SLOT_SIZE_LIGHT, /**< Light slot size. */
   OUTFIT_SLOT_SIZE_MEDIUM, /**< Medium slot size. */
   OUTFIT_SLOT_SIZE_HEAVY /**< Heavy slot size. */
} OutfitSlotSize;


/**
 * @brief Pilot slot that can contain outfits.
 */
typedef struct OutfitSlot_ {
   OutfitSlotType type; /**< Type of outfit slot. */
   OutfitSlotSize size; /**< Size of the outfit. */
} OutfitSlot;



/**
 * @brief Represents the particular properties of a bolt weapon.
 */
typedef struct OutfitBoltData_ {
   double delay; /**< delay between shots */
   double speed; /**< how fast it goes (not applicable to beam) */
   double range; /**< how far it goes */
   double falloff; /**< Point at which damage falls off. */
   double ew_lockon; /**< Electronic warfare lockon parameter. */
   double energy; /**< energy usage */
   double cpu; /**< CPU usage. */
   DamageType dtype; /**< damage type */
   double damage; /**< damage */
   double penetration; /**< Weapon penetration [0:1] with 1 being 100%. */
   double heatup; /**< How long it should take for the weapon to heat up (approx). */
   double heat; /**< Heat per shot. */
   double track; /**< Ewarfare to track. */
   double swivel; /**< Amount of swivel (semiarc in radians of deviation the weapon can correct. */

   /* Sound and graphics. */
   glTexture* gfx_space; /**< Normal graphic. */
   glTexture* gfx_end; /**< End graphic with modified hue. */
   double spin; /**< Graphic spin rate. */
   int sound; /**< sound to play */
   int sound_hit; /**< Sound to play on hit. */
   int spfx_armour; /**< special effect on hit */
   int spfx_shield; /**< special effect on hit */
} OutfitBoltData;

/**
 * @brief Represents the particular properties of a beam weapon.
 */
typedef struct OutfitBeamData_ {
   /* Time stuff. */
   double delay; /**< Delay between usage. */
   double warmup; /**< How long beam takes to warm up. */
   double duration; /**< How long the beam lasts active. */

   /* Beam properties. */
   double range; /**< how far it goes */
   double turn; /**< How fast it can turn. Only for turrets, in rad/s. */
   double energy; /**< Amount of energy it drains (per second). */
   double cpu; /**< CPU usage. */
   DamageType dtype; /**< Damage type. */
   double damage; /**< Damage amount. */
   double penetration; /**< Weapon penetration [0:1] with 1 being 100%. */
   double heatup; /**< How long it should take for the weapon to heat up (approx). */

   /* Graphics and sound. */
   glTexture *gfx; /**< Base texture. */
   int spfx_armour; /**< special effect on hit */
   int spfx_shield; /**< special effect on hit */
   int sound_warmup; /**< Sound to play when warming up. @todo use. */
   int sound; /**< Sound to play. */
   int sound_off; /**< Sound to play when turning off. */
} OutfitBeamData;

/**
 * @brief Represents a particular missile launcher.
 *
 * The properties of the weapon are highly dependent on the ammunition.
 */
typedef struct OutfitLauncherData_ {
   double delay; /**< Delay between shots. */
   double cpu; /**< CPU usage. */
   char *ammo_name; /**< Name of the ammo to use. */
   struct Outfit_ *ammo; /**< Ammo to use. */
   int amount; /**< Amount of ammo it can store. */
} OutfitLauncherData;

/**
 * @brief Represents ammunition for a launcher.
 */
typedef struct OutfitAmmoData_ {
   double duration; /**< How long the ammo lives. */
   double lockon; /**< time it takes to lock on the target */
   double ew_lockon; /**< Electronic warfare lockon parameter. */
   double resist; /**< lowers chance of jamming by this amount */
   int ai; /**< Smartness of ammo. */

   double speed; /**< maximum speed */
   double turn; /**< turn velocity in rad/s. */
   double thrust; /**< acceleration */
   double energy; /**< energy usage */
   DamageType dtype; /**< damage type */
   double damage; /**< damage */
   double penetration; /**< Weapon penetration [0:1] with 1 being 100%. */

   glTexture* gfx_space; /**< graphic */
   double spin; /**< Graphic spin rate. */
   int sound; /**< sound to play */
   int sound_hit; /**< Sound to play on hit. */
   int spfx_armour; /**< special effect on hit */
   int spfx_shield; /**< special effect on hit */
} OutfitAmmoData;

/**
 * @brief Represents a ship modification.
 *
 * These modify the ship's basic properties when equipped on a pilot.
 */
typedef struct OutfitModificationData_ {
   /* movement */
   double thrust; /**< Maximum thrust modifier. */
   double thrust_rel; /**< Relative thrust modifier. */
   double turn; /**< Maximum turn modifier. */
   double turn_rel; /**< Relative turn modifier. */
   double speed; /**< Maximum speed modifier. */
   double speed_rel; /**< Relative speed modifier. */

   /* health */
   double armour; /**< Maximum armour modifier. */
   double armour_rel; /**< Relative to armour base modifier. */
   double armour_regen; /**< Armour regeneration modifier. */
   double shield; /**< Maximum shield modifier. */
   double shield_rel; /**< Relative to shield base modifier. */
   double shield_regen; /**< Shield regeneration modifier. */
   double energy; /**< Maximum energy modifier. */
   double energy_rel; /**< Relative to energy base modifier. */
   double energy_regen; /**< Energy regeneration modifier. */
   double cpu; /**< CPU modifier. */

   /* misc */
   double cargo; /**< Cargo space modifier. */
   double crew_rel; /**< Relative crew modification. */
   double mass_rel; /**< Relative mass modification. */
   double fuel; /**< Maximum fuel modifier. */

   /* stats */
   ShipStats stats; /**< Stats. */
} OutfitModificationData;

/**
 * @brief Represents an afterburner.
 */
typedef struct OutfitAfterburnerData_ {
   double rumble; /**< percent of rumble */
   int sound; /**< sound of the afterburner */
   double thrust; /**< % of thrust increase based on ship base. */
   double speed; /**< % of speed to increase based on ship base. */
   double energy; /**< Energy usage while active */
   double cpu; /**< CPU usage. */
   double mass_limit; /**< Limit at which effectivity starts to drop. */
} OutfitAfterburnerData;

/**
 * @brief Represents a fighter bay.
 */
typedef struct OutfitFighterBayData_ {
   char *ammo_name; /**< Name fo the ships to use as ammo. */
   struct Outfit_ *ammo; /**< Ships to use as ammo. */
   double delay; /**< Delay between launches. */
   double cpu; /**< CPU usage. */
   int amount; /**< Amount of ammo it can store. */
} OutfitFighterBayData;

/**
 * @brief Represents a fighter for a fighter bay.
 */
typedef struct OutfitFighterData_ {
   char *ship; /**< Ship to use for fighter. */
   int sound; /**< Sound to make when launching. */
} OutfitFighterData;

/**
 * @brief Represents a map, is not actually stored on a ship but put into the nav system.
 *
 * Basically just marks an amount of systems when the player buys it as known.
 */
typedef struct OutfitMapData_ {
   double radius; /**< Number of jumps to add all systems within. */
} OutfitMapData;

/**
 * @brief Represents a jammer.
 */
typedef struct OutfitJammerData_ {
   double range; /**< Range it starts to do effect */
   double chance; /**< Chance of it nullifying the missile */
   double energy; /**< Energy it uses to run */
   double cpu; /**< CPU usage. */
} OutfitJammerData;

/**
 * @brief Represents a GUI.
 */
typedef struct OutfitGUIData_ {
   char *gui; /**< Name of the GUI file. */
} OutfitGUIData;

/**
 * @brief A ship outfit, depends radically on the type.
 */
typedef struct Outfit_ {
   char *name; /**< Name of the outfit. */
   char *typename; /**< Overrides the base type. */

   /* general specs */
   OutfitSlot slot; /**< Slot the outfit fits into. */
   char *license; /**< Licenses needed to buy it. */
   double mass; /**< How much weapon capacity is needed. */

   /* store stuff */
   credits_t price; /**< Base sell price. */
   char *description; /**< Store description. */
   char *desc_short; /**< Short outfit description. */

   glTexture* gfx_store; /**< Store graphic. */

   unsigned int properties; /**< Properties stored bitwise. */

   /* Type dependent */
   OutfitType type; /**< Type of the outfit. */
   union {
      OutfitBoltData blt; /**< BOLT */
      OutfitBeamData bem; /**< BEAM */
      OutfitLauncherData lau; /**< MISSILE */
      OutfitAmmoData amm; /**< AMMO */
      OutfitModificationData mod; /**< MODIFICATION */
      OutfitAfterburnerData afb; /**< AFTERBURNER */
      OutfitJammerData jam; /**< JAMMER */
      OutfitFighterBayData bay; /**< FIGHTER_BAY */
      OutfitFighterData fig; /**< FIGHTER */
      OutfitMapData map; /**< MAP */
      OutfitGUIData gui; /**< GUI */
   } u; /**< Holds the type-based outfit data. */
} Outfit;


/*
 * misc
 */
void outfit_calcDamage( double *dshield, double *darmour, double *knockback,
      DamageType dtype, double dmg );


/*
 * get
 */
Outfit* outfit_get( const char* name );
Outfit* outfit_getW( const char* name );
Outfit* outfit_getAll( int *n );
int outfit_compareTech( const void *outfit1, const void *outfit2 );
/* outfit types */
int outfit_isActive( const Outfit* o );
int outfit_isForward( const Outfit* o );
int outfit_isBolt( const Outfit* o );
int outfit_isBeam( const Outfit* o );
int outfit_isLauncher( const Outfit* o );
int outfit_isAmmo( const Outfit* o );
int outfit_isSeeker( const Outfit* o );
int outfit_isTurret( const Outfit* o );
int outfit_isMod( const Outfit* o );
int outfit_isAfterburner( const Outfit* o );
int outfit_isJammer( const Outfit* o );
int outfit_isFighterBay( const Outfit* o );
int outfit_isFighter( const Outfit* o );
int outfit_isMap( const Outfit* o );
int outfit_isGUI( const Outfit* o );
int outfit_isLicense( const Outfit* o );
const char* outfit_getType( const Outfit* o );
const char* outfit_getTypeBroad( const Outfit* o );

/*
 * Search.
 */
const char *outfit_existsCase( const char* name );
char **outfit_searchFuzzyCase( const char* name, int *n );

/*
 * get data from outfit
 */
const char *outfit_slotName( const Outfit* o );
const char *outfit_slotSize( const Outfit* o );
glColour *outfit_slotSizeColour( const OutfitSlot* os );
OutfitSlotSize outfit_toSlotSize( const char *s );
glTexture* outfit_gfx( const Outfit* o );
int outfit_spfxArmour( const Outfit* o );
int outfit_spfxShield( const Outfit* o );
double outfit_damage( const Outfit* o );
double outfit_penetration( const Outfit* o );
DamageType outfit_damageType( const Outfit* o );
double outfit_delay( const Outfit* o );
Outfit* outfit_ammo( const Outfit* o );
int outfit_amount( const Outfit* o );
double outfit_energy( const Outfit* o );
double outfit_heat( const Outfit* o );
double outfit_cpu( const Outfit* o );
double outfit_range( const Outfit* o );
double outfit_speed( const Outfit* o );
double outfit_spin( const Outfit* o );
int outfit_sound( const Outfit* o );
int outfit_soundHit( const Outfit* o );

/*
 * loading/freeing outfit stack
 */
int outfit_load (void);
void outfit_free (void);


/*
 * Misc.
 */
const char *outfit_damageTypeToStr( DamageType dmg );
int outfit_fitsSlot( const Outfit* o, const OutfitSlot* s );


#endif /* OUTFIT_H */
