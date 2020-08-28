/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIPSTATS_H
#  define SHIPSTATS_H


#include "nxml.h"


/**
 * @brief Lists all the possible types.
 *
 * Syntax:
 *    SS_TYPE_#1_#2
 *
 * #1 is D for double, A for absolute double, I for integer or B for boolean.
 * #2 is the name.
 */
typedef enum ShipStatsType_ {
   SS_TYPE_NIL,               /**< Invalid type. */

   /*
    * D: Double type data. Should be continuous.
    */
   /* General. */
   SS_TYPE_D_SPEED_MOD,        /**< Speed multiplier. */
   SS_TYPE_D_TURN_MOD,         /**< Turn multiplier. */
   SS_TYPE_D_THRUST_MOD,       /**< Acceleration multiplier. */
   SS_TYPE_D_CARGO_MOD,        /**< Cargo space multiplier. */
   SS_TYPE_D_ARMOUR_MOD,       /**< Armour multiplier. */
   SS_TYPE_D_ARMOUR_REGEN_MOD, /**< Armour regeneration multiplier. */
   SS_TYPE_D_SHIELD_MOD,       /**< Shield multiplier. */
   SS_TYPE_D_SHIELD_REGEN_MOD, /**< Shield regeneration multiplier. */
   SS_TYPE_D_ENERGY_MOD,       /**< Energy multiplier. */
   SS_TYPE_D_ENERGY_REGEN_MOD, /**< Energy regeneration multiplier. */
   SS_TYPE_D_CPU_MOD,          /**< CPU multiplier. */

   /* Freighter-type. */
   SS_TYPE_D_JUMP_DELAY,      /**< Modulates the time that passes during a hyperspace jump. */
   SS_TYPE_D_CARGO_INERTIA,   /**< Modifies the effect of cargo_mass. */

   /* Stealth. */
   SS_TYPE_D_EW_HIDE,         /**< Electronic warfare hide modifier. */
   SS_TYPE_D_EW_DETECT,       /**< Electronic warfare detection modifier. */
   SS_TYPE_D_EW_JUMPDETECT,   /**< Electronic warfare jump point detection modifier. */

   /* Launchers. */
   SS_TYPE_D_LAUNCH_RATE,     /**< Launch rate for missiles. */
   SS_TYPE_D_LAUNCH_RANGE,    /**< Launch range for missiles. */
   SS_TYPE_D_LAUNCH_DAMAGE,   /**< Launch damage for missiles. */
   SS_TYPE_D_AMMO_CAPACITY,   /**< Capacity of launchers. */     /* TODO */
   SS_TYPE_D_LAUNCH_LOCKON,   /**< Lockon speed of launchers. */ /* TODO */

   /* Forward mounts. */
   SS_TYPE_D_FORWARD_HEAT,    /**< Heat generation for cannons. */
   SS_TYPE_D_FORWARD_DAMAGE,  /**< Damage done by cannons. */
   SS_TYPE_D_FORWARD_FIRERATE, /**< Firerate of cannons. */
   SS_TYPE_D_FORWARD_ENERGY,  /**< Energy usage of cannons. */

   /* Turrets. */
   SS_TYPE_D_TURRET_HEAT,     /**< Heat generation for turrets. */
   SS_TYPE_D_TURRET_DAMAGE,   /**< Damage done by turrets. */
   SS_TYPE_D_TURRET_TRACKING, /**< Tracking of turrets. */
   SS_TYPE_D_TURRET_FIRERATE, /**< Firerate of turrets. */
   SS_TYPE_D_TURRET_ENERGY,   /**< Energy usage of turrets. */

   /* Nebula. */
   SS_TYPE_D_NEBULA_ABSORB_SHIELD, /**< Shield nebula resistance. */
   SS_TYPE_D_NEBULA_ABSORB_ARMOUR, /**< Armour nebula resistance. */

   /* Misc. */
   SS_TYPE_D_HEAT_DISSIPATION, /**< Ship heat dissipation. */
   SS_TYPE_D_STRESS_DISSIPATION, /**< Ship stress dissipation. */
   SS_TYPE_D_CREW,            /**< Ship crew. */
   SS_TYPE_D_MASS,            /**< Ship mass. */
   SS_TYPE_D_ENGINE_LIMIT_REL, /**< Modifier for the ship's engine limit. */

   /*
    * A: Absolute double type data. Should be continuous.
    */
   SS_TYPE_A_ENERGY_FLAT,       /**< Flat energy modifier (not multiplied). */
   SS_TYPE_A_ENERGY_REGEN_FLAT, /**< Flat energy regeneration modifier (not multiplied). */
   SS_TYPE_A_SHIELD_FLAT,       /**< Flat shield modifier (not multiplied). */
   SS_TYPE_A_SHIELD_REGEN_FLAT, /**< Flat shield regeneration modifier (not multiplied). */
   SS_TYPE_A_ARMOUR_FLAT,       /**< Flat armour modifier (not multiplied). */
   SS_TYPE_A_ARMOUR_REGEN_FLAT, /**< Flat armour regeneration modifier (not multiplied). */
   SS_TYPE_A_CPU_MAX,           /**< Maximum CPU modifier. */
   SS_TYPE_A_ENGINE_LIMIT,      /**< Engine's mass limit. */

   /*
    * I: Integer type data. Should be continuous.
    */
   SS_TYPE_I_HIDDEN_JUMP_DETECT, /**< Hidden jump detection. */

   /*
    * B: Boolean type data. Should be continuous.
    */
   SS_TYPE_B_INSTANT_JUMP, /**< Do not require brake or chargeup to jump. */
   SS_TYPE_B_REVERSE_THRUST, /**< Ship slows down rather than turning on reverse. */
   SS_TYPE_B_ASTEROID_SCAN, /**< Ship can gather informations from asteroids. */

   /*
    * End of list.
    */
   SS_TYPE_SENTINEL          /**< Sentinel for end of types. */
} ShipStatsType;

/**
 * @brief Represents relative ship statistics as a linked list.
 *
 * Doubles:
 *  These values are relative so something like -0.15 would be -15%.
 *
 * Absolute and Integers:
 *  These values are just absolute values.
 *
 * Booleans:
 *  Can only be 1.
 */
typedef struct ShipStatList_ {
   struct ShipStatList_ *next; /**< Next pointer. */

   int target;          /**< Whether or not it affects the target. */
   ShipStatsType type;  /**< Type of stat. */
   union {
      double d;         /**< Floating point data. */
      int    i;         /**< Integer data. */
   } d; /**< Stat data. */
} ShipStatList;


/**
 * @brief Represents ship statistics, properties ship can use.
 *
 * Doubles:
 *  These are normalized and centered around 1 so they are in the [0:2]
 *  range, with 1. being default. This value then modulates the stat's base
 *  value.
 *  Example:
 *   0.7 would lower by 30% the base value.
 *   1.2 would increase by 20% the base value.
 *
 * Absolute and Integers:
 *  Absolute values in whatever units it's meant to use.
 *
 * Booleans:
 *  1 or 0 values wher 1 indicates property is set.
 */
typedef struct ShipStats_ {
#if 0
   /* Corvette type. */
   double afterburner_energy; /**< Energy used by afterburner. */

   /* Carrier type. */
   double fighterbay_cpu; /**< CPU usage by fighter bays. */
   double fighterbay_rate; /**< Launch rate of fighter bay. */
#endif

   /* General */
   double speed_mod;          /**< Speed multiplier. */
   double turn_mod;           /**< Turn multiplier. */
   double thrust_mod;         /**< Thrust multiplier. */
   double cargo_mod;          /**< Cargo space multiplier. */
   double armour_mod;         /**< Armour multiplier. */
   double armour_regen_mod;   /**< Armour regeneration multiplier. */
   double armour_flat;        /**< Armour modifier (flat). */
   double armour_damage;      /**< Armour regeneration (flat). */
   double shield_mod;         /**< Shield multiplier. */
   double shield_regen_mod;   /**< Shield regeneration multiplier. */
   double shield_flat;        /**< Shield modifier (flat). */
   double shield_usage;       /**< Shield usage (flat). */
   double energy_mod;         /**< Energy multiplier. */
   double energy_regen_mod;   /**< Energy regeneration multiplier. */
   double energy_flat;        /**< Energy modifier (flat). */
   double energy_usage;       /**< Energy usage (flat). */
   double cpu_mod;            /**< CPU multiplier. */
   double cpu_max;            /**< CPU modifier. */

   /* Freighter-type. */
   double jump_delay;      /**< Modulates the time that passes during a hyperspace jump. */
   double cargo_inertia;   /**< Lowers the effect of cargo mass. */

   /* Stealth. */
   double ew_hide;         /**< Electronic warfare hide modifier. */
   double ew_detect;       /**< Electronic warfare detection modifier. */
   double ew_jump_detect;  /**< Electronic warfare jump point detection modifier. */

   /* Military type. */
   double heat_dissipation; /**< Global ship dissipation. */
   double stress_dissipation; /**< Global stress dissipation. */
   double crew_mod;        /**< Relative crew modification. */
   double mass_mod;        /**< Relative mass modification. */

   /* Launchers. */
   double launch_rate;     /**< Fire rate of launchers. */
   double launch_range;    /**< Range of launchers. */
   double ammo_capacity;   /**< Capacity of launchers. */
   double launch_lockon;   /**< Lock on speed of launchers. */

   /* Fighter/Corvette type. */
   double fwd_heat;        /**< Heat of forward mounts. */
   double fwd_damage;      /**< Damage of forward mounts. */
   double fwd_firerate;    /**< Rate of fire of forward mounts. */
   double fwd_energy;      /**< Consumption rate of forward mounts. */

   /* Destroyer/Cruiser type. */
   double tur_heat;        /**< Heat of turrets. */
   double tur_damage;      /**< Damage of turrets. */
   double tur_tracking;    /**< Tracking of turrets. */
   double tur_firerate;    /**< Rate of fire of turrets. */
   double tur_energy;      /**< Consumption rate of turrets. */

   /* Engine limits. */
   double engine_limit_rel; /**< Engine limit modifier. */
   double engine_limit;     /**< Engine limit. */

   /* Misc. */
   double nebu_absorb_shield; /**< Shield nebula resistance. */
   double nebu_absorb_armour; /**< Armour nebula resistance. */
   int misc_instant_jump;    /**< Do not require brake or chargeup to jump. */
   int misc_reverse_thrust;  /**< Slows down the ship instead of turning it around. */
   int misc_asteroid_scan;   /**< Able to scan asteroids. */
   int misc_hidden_jump_detect; /**< Degree of hidden jump detection. */
} ShipStats;


/*
 * Safety.
 */
int ss_check (void);

/*
 * Loading.
 */
ShipStatList* ss_listFromXML( xmlNodePtr node );
void ss_free( ShipStatList *ll );

/*
 * Manipulation
 */
int ss_statsInit( ShipStats *stats );
int ss_statsModSingle( ShipStats *stats, const ShipStatList* list, const ShipStats *amount );
int ss_statsModFromList( ShipStats *stats, const ShipStatList* list, const ShipStats *amount );

/*
 * Lookup.
 */
const char* ss_nameFromType( ShipStatsType type );
size_t ss_offsetFromType( ShipStatsType type );
ShipStatsType ss_typeFromName( const char *name );
int ss_statsListDesc( const ShipStatList *ll, char *buf, int len, int newline );
int ss_statsDesc( const ShipStats *s, char *buf, int len, int newline );
int ss_csv( const ShipStats *s, char *buf, int len );


#endif /* SHIPSTATS_H */
