/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nxml.h"
#include "nlua.h"

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
   SS_TYPE_D_FUEL_MOD,         /**< Fuel capacity multiplier. */
   SS_TYPE_D_ARMOUR_MOD,       /**< Armour multiplier. */
   SS_TYPE_D_ARMOUR_REGEN_MOD, /**< Armour regeneration multiplier. */
   SS_TYPE_D_SHIELD_MOD,       /**< Shield multiplier. */
   SS_TYPE_D_SHIELD_REGEN_MOD, /**< Shield regeneration multiplier. */
   SS_TYPE_D_ENERGY_MOD,       /**< Energy multiplier. */
   SS_TYPE_D_ENERGY_REGEN_MOD, /**< Energy regeneration multiplier. */
   SS_TYPE_D_CPU_MOD,          /**< CPU multiplier. */
   SS_TYPE_D_COOLDOWN_MOD,     /**< Ability cooldown multiplier. */

   /* Freighter-type. */
   SS_TYPE_D_JUMP_DELAY,      /**< Modulates the time that passes during a hyperspace jump. */
   SS_TYPE_D_LAND_DELAY,      /**< Modulates the time that passes during landing. */
   SS_TYPE_D_CARGO_INERTIA,   /**< Modifies the effect of cargo_mass. */

   /* Electronic warfare. */
   SS_TYPE_D_EW_HIDE,         /**< Electronic warfare hide modifier. (affects ew_detection) */
   SS_TYPE_D_EW_EVADE,        /**< Electronic warfare evasion modifier. (affects ew_evasion) */
   SS_TYPE_D_EW_STEALTH,      /**< Electronic warfare stealth modifier. (affects ew_stealth) */
   SS_TYPE_D_EW_DETECT,       /**< Electronic warfare detection modifier. */
   SS_TYPE_D_EW_TRACK,        /**< Electronic warfare tracking modifier. */
   SS_TYPE_D_EW_JUMPDETECT,   /**< Electronic warfare jump point detection modifier. */
   SS_TYPE_D_EW_STEALTH_TIMER,/**< Electronic warfare stealth timer decrease speed. */
   SS_TYPE_D_EW_SCANNED_TIME, /**< Electronic warfare time it takes to get scanned. */

   /* Launchers. */
   SS_TYPE_D_LAUNCH_RATE,     /**< Launch rate for missiles. */
   SS_TYPE_D_LAUNCH_RANGE,    /**< Launch range for missiles. */
   SS_TYPE_D_LAUNCH_DAMAGE,   /**< Launch damage for missiles. */
   SS_TYPE_D_AMMO_CAPACITY,   /**< Capacity of launchers. */
   SS_TYPE_D_LAUNCH_LOCKON,   /**< Lock-on speed of launchers. */
   SS_TYPE_D_LAUNCH_CALIBRATION,/**< Calibration speed of launchers. */
   SS_TYPE_D_LAUNCH_RELOAD,   /**< Regeneration rate of launcher ammo. */

   /* Fighter Bays. */
   SS_TYPE_D_FBAY_DAMAGE,     /**< Fighter bay fighter damage bonus (all weapons). */
   SS_TYPE_D_FBAY_HEALTH,     /**< Fighter bay fighter health bonus (shield and armour). */
   SS_TYPE_D_FBAY_MOVEMENT,   /**< Fighter bay fighter movement bonus (turn, thrust, and speed). */
   SS_TYPE_D_FBAY_CAPACITY,   /**< Capacity of fighter bays. */
   SS_TYPE_D_FBAY_RATE,       /**< Launch rate for fighter bays. */
   SS_TYPE_D_FBAY_RELOAD,     /**< Regeneration rate of fighters. */

   /* Forward mounts. */
   SS_TYPE_D_FORWARD_HEAT,    /**< Heat generation for cannons. */
   SS_TYPE_D_FORWARD_DAMAGE,  /**< Damage done by cannons. */
   SS_TYPE_D_FORWARD_FIRERATE, /**< Firerate of cannons. */
   SS_TYPE_D_FORWARD_ENERGY,  /**< Energy usage of cannons. */
   SS_TYPE_D_FORWARD_DAMAGE_AS_DISABLE, /**< Damage converted to disable. */

   /* Turrets. */
   SS_TYPE_D_TURRET_HEAT,     /**< Heat generation for turrets. */
   SS_TYPE_D_TURRET_DAMAGE,   /**< Damage done by turrets. */
   SS_TYPE_D_TURRET_TRACKING, /**< Tracking of turrets. */
   SS_TYPE_D_TURRET_FIRERATE, /**< Firerate of turrets. */
   SS_TYPE_D_TURRET_ENERGY,   /**< Energy usage of turrets. */
   SS_TYPE_D_TURRET_DAMAGE_AS_DISABLE, /**< Damage converted to disable. */

   /* Misc. */
   SS_TYPE_D_HEAT_DISSIPATION, /**< Ship heat dissipation. */
   SS_TYPE_D_STRESS_DISSIPATION, /**< Ship stress dissipation. */
   SS_TYPE_D_CREW,            /**< Ship crew. */
   SS_TYPE_D_MASS,            /**< Ship mass. */
   SS_TYPE_D_ENGINE_LIMIT_REL, /**< Modifier for the ship's engine limit. */
   SS_TYPE_D_LOOT_MOD,        /**< Affects boarding rewards. */
   SS_TYPE_D_TIME_MOD,        /**< Time dilation modifier. */
   SS_TYPE_D_TIME_SPEEDUP,    /**< Makes the pilot operate at a higher dt. */
   SS_TYPE_D_COOLDOWN_TIME,   /**< Speeds up or slows down the cooldown time. */
   SS_TYPE_D_JUMP_DISTANCE,   /**< Modifies the distance from a jump point at which the pilot can jump. */
   SS_TYPE_D_JUMP_WARMUP,     /**< Modifies the time it takes to warm up to jump. */
   SS_TYPE_D_MINING_BONUS,    /**< Bonus when mining asteroids. */

   /*
    * A: Absolute double type data. Should be continuous.
    */
   /* Movement. */
   SS_TYPE_A_THRUST,          /**< Thrust modifier. */
   SS_TYPE_A_TURN,            /**< Turn modifier (in deg/s). */
   SS_TYPE_A_SPEED,           /**< Speed modifier. */
   /* Health. */
   SS_TYPE_A_ENERGY,          /**< Energy modifier. */
   SS_TYPE_A_ENERGY_REGEN,    /**< Energy regeneration modifier. */
   SS_TYPE_A_ENERGY_REGEN_MALUS,/**< Flat energy regeneration modifier (not multiplied). */
   SS_TYPE_A_ENERGY_LOSS,     /**< Flat energy modifier (not multiplied) and applied linearly. */
   SS_TYPE_A_SHIELD,          /**< Shield modifier. */
   SS_TYPE_A_SHIELD_REGEN,    /**< Shield regeneration modifier. */
   SS_TYPE_A_SHIELD_REGEN_MALUS,/**< Flat shield regeneration modifier (not multiplied). */
   SS_TYPE_A_ARMOUR,          /**< Armour modifier. */
   SS_TYPE_A_ARMOUR_REGEN,    /**< Armour regeneration modifier. */
   SS_TYPE_A_ARMOUR_REGEN_MALUS,/**< Flat armour regeneration modifier (not multiplied). */
   SS_TYPE_A_DAMAGE,          /**< Flat damage modifier (eats through shield first then armour like normal damage). */
   SS_TYPE_A_DISABLE,         /**< Flat disable modifier (acts like normal disable damage). */
   /* Misc. */
   SS_TYPE_A_CPU_MAX,         /**< Maximum CPU modifier. */
   SS_TYPE_A_ENGINE_LIMIT,    /**< Engine's mass limit. */
   SS_TYPE_A_FUEL_REGEN,      /**< Fuel regeneration. */
   SS_TYPE_A_ASTEROID_SCAN,   /**< DIstance at which ship can gather informations from asteroids. */
   SS_TYPE_A_NEBULA_VISIBILITY,/**< Bonus distance to nebula visibility. */

   /*
    * P: Absolute percent type datas. Should be continuous.
    */
   SS_TYPE_P_ABSORB,          /**< Damage absorption. */
   /* Nebula. */
   SS_TYPE_P_NEBULA_ABSORB,   /**< Nebula resistance. */
   SS_TYPE_P_JAMMING_CHANCE,  /**< Jamming chance for outfits. */

   /*
    * I: Integer type data. Should be continuous.
    */
   SS_TYPE_I_FUEL,            /**< Fuel bonus. */
   SS_TYPE_I_CARGO,           /**< Cargo bonus. */
   SS_TYPE_I_CREW,            /**< Crew bonus. */

   /*
    * B: Boolean type data. Should be continuous.
    */
   SS_TYPE_B_HIDDEN_JUMP_DETECT,/**< Hidden jump detection. */
   SS_TYPE_B_INSTANT_JUMP,    /**< Do not require brake or chargeup to jump. */
   SS_TYPE_B_REVERSE_THRUST,  /**< Ship slows down rather than turning on reverse. */

   /*
    * End of list.
    */
   SS_TYPE_SENTINEL           /**< Sentinel for end of types. */
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
 * Absolute Percentage:
 *  Absolute percentage values (get added together).
 *
 * Booleans:
 *  1 or 0 values wher 1 indicates property is set.
 */
typedef struct ShipStats_ {
   /* Movement. */
   double speed;              /**< Speed modifier. */
   double turn;               /**< Turn modifier. */
   double thrust;             /**< Thrust modifier. */
   double speed_mod;          /**< Speed multiplier. */
   double turn_mod;           /**< Turn multiplier. */
   double thrust_mod;         /**< Thrust multiplier. */

   /* Health. */
   double energy;             /**< Energy modifier. */
   double energy_regen;       /**< Energy regeneration modifier. */
   double energy_mod;         /**< Energy multiplier. */
   double energy_regen_mod;   /**< Energy regeneration multiplier. */
   double energy_regen_malus; /**< Energy usage (flat). */
   double energy_loss;        /**< Energy modifier (flat and linear). */
   double shield;             /**< Shield modifier. */
   double shield_regen;       /**< Shield regeneration modifier. */
   double shield_mod;         /**< Shield multiplier. */
   double shield_regen_mod;   /**< Shield regeneration multiplier. */
   double shield_regen_malus; /**< Shield usage (flat). */
   double armour;             /**< Armour modifier. */
   double armour_regen;       /**< Armour regeneration modifier. */
   double armour_mod;         /**< Armour multiplier. */
   double armour_regen_mod;   /**< Armour regeneration multiplier. */
   double armour_regen_malus; /**< Armour regeneration (flat). */
   double damage;             /**< Damage over time. */
   double disable;            /**< Disable over time. */

   /* General */
   double cargo_mod;          /**< Cargo space multiplier. */
   double fuel_mod;           /**< Fuel capacity multiplier. */
   double cpu_mod;            /**< CPU multiplier. */
   double cpu_max;            /**< CPU modifier. */
   double absorb;             /**< Flat damage absorption. */
   double cooldown_mod;       /**< Ability cooldown mod. */

   /* Freighter-type. */
   double jump_delay;      /**< Modulates the time that passes during a hyperspace jump. */
   double land_delay;      /**< Modulates the time that passes during landing. */
   double cargo_inertia;   /**< Lowers the effect of cargo mass. */

   /* Stealth. */
   double ew_hide;         /**< Electronic warfare hide modifier. */
   double ew_evade;
   double ew_stealth;
   double ew_detect;       /**< Electronic warfare detection modifier. */
   double ew_track;
   double ew_jump_detect;  /**< Electronic warfare jump point detection modifier. */
   double ew_stealth_timer; /**< Stealth timer decrease speed. */
   double ew_scanned_time; /**< Time to scan. */

   /* Military type. */
   double heat_dissipation; /**< Global ship dissipation. */
   double stress_dissipation; /**< Global stress dissipation. */
   double crew_mod;        /**< Relative crew modification. */
   double mass_mod;        /**< Relative mass modification. */

   /* Launchers. */
   double launch_rate;     /**< Fire rate of launchers. */
   double launch_range;    /**< Range of launchers. */
   double launch_damage;   /**< Damage of launchers. */
   double ammo_capacity;   /**< Capacity of launchers. */
   double launch_lockon;   /**< Lock on speed of launchers. */
   double launch_calibration;/**< Calibration speed of launchers. */
   double launch_reload;   /**< Reload rate of launchers. */

   /* Fighter bays. */
   double fbay_damage;     /**< Fighter bay fighter damage (all weapons). */
   double fbay_health;     /**< Fighter bay fighter health (armour and shield). */
   double fbay_movement;   /**< Fighter bay fighter movement (thrust, turn, and speed). */
   double fbay_capacity;   /**< Capacity of fighter bays. */
   double fbay_rate;       /**< Launch rate of fighter bays. */
   double fbay_reload;     /**< Reload rate of fighters. */

   /* Fighter/Corvette type. */
   double fwd_heat;        /**< Heat of forward mounts. */
   double fwd_damage;      /**< Damage of forward mounts. */
   double fwd_firerate;    /**< Rate of fire of forward mounts. */
   double fwd_energy;      /**< Consumption rate of forward mounts. */
   double fwd_dam_as_dis;  /**< Damage as disable for forward mounts. */

   /* Destroyer/Cruiser type. */
   double tur_heat;        /**< Heat of turrets. */
   double tur_damage;      /**< Damage of turrets. */
   double tur_tracking;    /**< Tracking of turrets. */
   double tur_firerate;    /**< Rate of fire of turrets. */
   double tur_energy;      /**< Consumption rate of turrets. */
   double tur_dam_as_dis;  /**< Damage as disable for turrets. */

   /* Jamming. */
   double jam_chance;      /**< Jamming chance. */

   /* Engine limits. */
   double engine_limit_rel; /**< Engine limit modifier. */
   double engine_limit;     /**< Engine limit. */

   /* Misc. */
   double nebu_absorb;     /**< Shield nebula resistance. */
   double nebu_visibility; /**< Nebula visibility. */
   int misc_instant_jump;  /**< Do not require brake or chargeup to jump. */
   int misc_reverse_thrust;/**< Slows down the ship instead of turning it around. */
   double asteroid_scan;   /**< Distance at which asteroids can be scanned. */
   double mining_bonus;    /**< Bonus when mining asteroids. */
   int misc_hidden_jump_detect; /**< Degree of hidden jump detection. */
   int fuel;               /**< Maximum fuel modifier. */
   double fuel_regen;      /**< Absolute fuel regeneration. */
   int cargo;              /**< Maximum cargo modifier. */
   int crew;               /**< Crew modifier. */
   double loot_mod;        /**< Boarding loot reward bonus. */
   double time_mod;        /**< Time dilation modifier. */
   double time_speedup;    /**< Makes the pilot operate at higher speeds. */
   double cooldown_time;   /**< Modifies cooldown time. */
   double jump_distance;   /**< Modifies how far the pilot can jump from the jump point. */
   double jump_warmup;     /**< Modifies the time that is necessary to jump. */
} ShipStats;

/*
 * Safety.
 */
int ss_check (void);

/*
 * Loading.
 */
ShipStatList* ss_listFromXML( xmlNodePtr node );
int ss_listToXML( xmlTextWriterPtr writer, const ShipStatList *ll );
int ss_sort( ShipStatList **ll );
void ss_free( ShipStatList *ll );

/*
 * Manipulation
 */
int ss_statsInit( ShipStats *stats );
int ss_statsMerge( ShipStats *dest, const ShipStats *src );
int ss_statsMergeSingle( ShipStats *stats, const ShipStatList *list );
int ss_statsMergeSingleScale( ShipStats *stats, const ShipStatList *list, double scale );
int ss_statsMergeFromList( ShipStats *stats, const ShipStatList *list );
int ss_statsMergeFromListScale( ShipStats *stats, const ShipStatList *list, double scale );

/*
 * Lookup.
 */
const char* ss_nameFromType( ShipStatsType type );
size_t ss_offsetFromType( ShipStatsType type );
ShipStatsType ss_typeFromName( const char *name );
int ss_statsListDesc( const ShipStatList *ll, char *buf, int len, int newline );
int ss_statsDesc( const ShipStats *s, char *buf, int len, int newline );

/*
 * Manipulation.
 */
int ss_statsSet( ShipStats *s, const char *name, double value, int overwrite );
double ss_statsGet( const ShipStats *s, const char *name );
int ss_statsGetLua( lua_State *L, const ShipStats *s, const char *name, int internal );
int ss_statsGetLuaTable( lua_State *L, const ShipStats *s, int internal );
