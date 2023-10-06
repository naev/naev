/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "collision.h"
#include "commodity.h"
#include "opengl.h"
#include "shipstats.h"
#include "sound.h"
#include "spfx.h"
#include "nlua.h"

/*
 * properties
 */
#define outfit_isProp(o,p)          ((o)->properties & p) /**< Checks an outfit for property. */
/* property flags */
#define OUTFIT_PROP_UNIQUE             (1<<0)  /**< Unique item (can only have one). Not sellable.*/
#define OUTFIT_PROP_SHOOT_DRY          (1<<1)  /**< Weapon that doesn't actually create particles. Should be handled in ontoggle. */
#define OUTFIT_PROP_TEMPLATE           (1<<2)  /**< Outfit is meant to be used as a template for other outfits. Ignores warnings. */
#define OUTFIT_PROP_WEAP_SECONDARY     (1<<3) /**< Is a secondary weapon? */
#define OUTFIT_PROP_WEAP_SPIN          (1<<4) /**< Should weapon spin around? */
#define OUTFIT_PROP_WEAP_BLOWUP_ARMOUR (1<<5) /**< Weapon blows up (armour spfx)
                                                   when timer is up. */
#define OUTFIT_PROP_WEAP_BLOWUP_SHIELD (1<<6) /**< Weapon blows up (shield spfx)
                                                   when timer is up. */
#define OUTFIT_PROP_WEAP_FRIENDLYFIRE  (1<<8) /**< Weapon damages all ships when blowing up. */
#define OUTFIT_PROP_WEAP_POINTDEFENSE  (1<<9) /**< Weapon can hit ammunitions. */
#define OUTFIT_PROP_WEAP_MISS_SHIPS    (1<<10) /**< Weapon can not hit ships. */
#define OUTFIT_PROP_WEAP_MISS_ASTEROIDS (1<<11) /**< Weapon can not hit asteroids. */
#define OUTFIT_PROP_WEAP_MISS_EXPLODE  (1<<12) /**< The weapon particle blows up on miss. */

/* Outfit filter labels. [Doc comments are also translator notes and must precede the #define.] */
/** Color-coded abbreviation for "Weapon [outfit]", short enough to use as a tab/column title. */
#define OUTFIT_LABEL_WEAPON            N_("#p W ")
/** Color-coded abbreviation for "Utility [outfit]", short enough to use as a tab/column title. */
#define OUTFIT_LABEL_UTILITY           N_("#g U ")
/** Color-coded abbreviation for "Structure [outfit]", short enough to use as a tab/column title. */
#define OUTFIT_LABEL_STRUCTURE         N_("#n S ")
/** Color-coded abbreviation for "Core [outfit]", short enough to use as a tab/column title. */
#define OUTFIT_LABEL_CORE              N_("#oCore")

/*
 * Needed because some outfittypes call other outfits.
 */
struct Outfit_;

/**
 * @brief Different types of existing outfits.
 *
 * Outfits are organized by the order here
 *
 * @note If you modify this DON'T FORGET TO MODIFY outfit_getType too!!!
 */
typedef enum OutfitType_ {
   OUTFIT_TYPE_NULL,       /**< Null type. */
   OUTFIT_TYPE_BOLT,       /**< Fixed bolt cannon. */
   OUTFIT_TYPE_BEAM,       /**< Fixed beam cannon. */
   OUTFIT_TYPE_TURRET_BOLT,/**< Rotary bolt turret. */
   OUTFIT_TYPE_TURRET_BEAM,/**< Rotary beam turret. */
   OUTFIT_TYPE_LAUNCHER,   /**< Launcher. */
   OUTFIT_TYPE_TURRET_LAUNCHER, /**< Turret launcher. */
   OUTFIT_TYPE_MODIFICATION, /**< Modifies the ship base features. */
   OUTFIT_TYPE_AFTERBURNER,/**< Gives the ship afterburn capability. */
   OUTFIT_TYPE_FIGHTER_BAY,/**< Contains other ships. */
   OUTFIT_TYPE_MAP,        /**< Gives the player more knowledge about systems. */
   OUTFIT_TYPE_LOCALMAP,   /**< Gives the player more knowledge about the current system. */
   OUTFIT_TYPE_GUI,        /**< GUI for the player. */
   OUTFIT_TYPE_LICENSE,    /**< License that allows player to buy special stuff. */
   OUTFIT_TYPE_SENTINEL    /**< indicates last type */
} OutfitType;

/**
 * @brief Outfit slot types.
 */
typedef enum OutfitSlotType_ {
   OUTFIT_SLOT_NULL,       /**< Invalid slot type. */
   OUTFIT_SLOT_NA,         /**< Slot type not applicable. */
   OUTFIT_SLOT_STRUCTURE,  /**< Low energy slot. */
   OUTFIT_SLOT_UTILITY,    /**< Medium energy slot. */
   OUTFIT_SLOT_WEAPON      /**< High energy slot. */
} OutfitSlotType;

/**
 * @brief Outfit slot sizes.
 */
typedef enum OutfitSlotSize_ {
   OUTFIT_SLOT_SIZE_NA,    /**< Not applicable slot size. */
   OUTFIT_SLOT_SIZE_LIGHT, /**< Light slot size. */
   OUTFIT_SLOT_SIZE_MEDIUM,/**< Medium slot size. */
   OUTFIT_SLOT_SIZE_HEAVY  /**< Heavy slot size. */
} OutfitSlotSize;

/**
 * @brief Ammo AI types.
 */
typedef enum OutfitAmmoAI_ {
   AMMO_AI_UNGUIDED, /**< No AI. */
   AMMO_AI_SEEK,     /**< Aims at the target. */
   AMMO_AI_SMART     /**< Aims at the target, correcting for relative velocity. */
} OutfitAmmoAI;

/**
 * @brief Pilot slot that can contain outfits.
 */
typedef struct OutfitSlot_ {
   unsigned int spid;   /**< Slot property ID. */
   int exclusive;       /**< Outfit must go exclusively into the slot. */
   OutfitSlotType type; /**< Type of outfit slot. */
   OutfitSlotSize size; /**< Size of the outfit. */
} OutfitSlot;

/**
 * @brief
 */
typedef struct OutfitGFX_ {
   glTexture* tex;      /**< Graphic in case of texture. */
   glTexture* tex_end;  /**< End texture if applicable. */
   CollPoly *polygon;   /**< Collision polygon. */
   double spin;         /**< Graphic spin rate. */
   GLuint program;      /**< Shader program. */
   GLuint vertex;       /**< Vertex info. */
   GLuint projection;   /**< Projection matrix. */
   GLuint dimensions;   /**< Dimensions of the rendered object. */
   GLuint u_r;          /**< Random value uniform. */
   GLuint u_time;       /**< Elapsed time uniform. */
   GLuint u_fade;       /**< Fade factor uniform. */
   double size;         /**< Size to render at. */
   double col_size;     /**< Size of the collision object. */
} OutfitGFX;

/**
 * @brief Core damage that an outfit does.
 */
typedef struct Damage_ {
   int type;            /**< Type of damage. */
   double penetration;  /**< Penetration the damage has [0:1], with 1 being 100%. */
   double damage;       /**< Amount of damage, this counts towards killing the ship. */
   double disable;      /**< Amount of disable damage, this counts towards disabling the ship. */
} Damage;

/**
 * @brief Represents the particular properties of a bolt weapon.
 */
typedef struct OutfitBoltData_ {
   double delay;     /**< Delay between shots */
   double speed;     /**< How fast it goes. */
   double range;     /**< How far it goes. */
   double falloff;   /**< Point at which damage falls off. */
   double energy;    /**< Energy usage */
   Damage dmg;       /**< Damage done. */
   double radius;    /**< Explosion radius .*/

   double heatup;    /**< How long it should take for the weapon to heat up (approx). */
   double heat;      /**< Heat per shot. */
   double trackmin;  /**< Ewarfare minimal tracking. */
   double trackmax;  /**< Ewarfare maximal (optimal) tracking. */
   double swivel;    /**< Amount of swivel (semiarc in radians of deviation the weapon can correct). */
   double dispersion;/**< Angle amount to spread particles around independent of heat. */
   double speed_dispersion;/**< Dispersion, but for speed. */
   int shots;        /**< Number of particles shot when fired. */
   int mining_rarity;/**< Maximum mining rarity the weapon can mine. */

   /* Sound and graphics. */
   OutfitGFX gfx;    /**< Rendering information. */
   int sound;        /**< Sound to play on shoot.*/
   int sound_hit;    /**< Sound to play on hit. */
   int spfx_armour;  /**< special effect on hit. */
   int spfx_shield;  /**< special effect on hit. */
} OutfitBoltData;

/**
 * @brief Represents the particular properties of a beam weapon.
 */
typedef struct OutfitBeamData_ {
   /* Time stuff. */
   double delay;     /**< Delay between usage. */
   double warmup;    /**< How long beam takes to warm up. */
   double duration;  /**< How long the beam lasts active. */
   double min_duration; /**< Minimum duration the beam can be fired for. */

   /* Beam properties. */
   double range;     /**< how far it goes */
   double turn;      /**< How fast it can turn. Only for turrets, in rad/s. */
   double energy;    /**< Amount of energy it drains (per second). */
   Damage dmg;       /**< Damage done. */
   double heatup;    /**< How long it should take for the weapon to heat up (approx). */
   double heat;      /**< Heat per second. */
   double swivel;    /**< Amount of swivel (semiarc in radians of deviation the weapon can correct). */
   int mining_rarity;/**< Maximum mining rarity the weapon can mine. */

   /* Graphics and sound. */
   glColour colour;  /**< Color to use for the shader. */
   GLfloat width;    /**< Width of the beam. */
   GLuint shader;    /**< Shader subroutine to use. */
   int spfx_armour;  /**< special effect on hit */
   int spfx_shield;  /**< special effect on hit */
   int sound_warmup; /**< Sound to play when warming up. @todo use. */
   int sound;        /**< Sound to play. */
   int sound_off;    /**< Sound to play when turning off. */
} OutfitBeamData;

/**
 * @brief Represents a particular missile launcher.
 *
 * The properties of the weapon are highly dependent on the ammunition.
 */
typedef struct OutfitLauncherData_ {
   double delay;     /**< Delay between shots. */
   int amount;       /**< Amount of ammo it can store. */
   double reload_time; /**< Time it takes to reload 1 ammo. */

   /* Lock-on information. */
   double lockon;    /**< Time it takes to lock on the target */
   double iflockon;  /**< Time it takes to lock on properly after launch. */
   double trackmin;  /**< Ewarfare minimal tracking. */
   double trackmax;  /**< Ewarfare maximal (optimal) tracking. */
   double arc;       /**< Semi-angle of the arc which it will lock on in. */
   double swivel;    /**< Amount of swivel (semiarc in radians of deviation the weapon can correct when launched). */
   double dispersion;/**< Angle amount to spread particles around independent of heat. */
   double speed_dispersion;/**< Dispersion, but for speed. */
   int shots;        /**< Number of particles shot when fired. */
   int mining_rarity;/**< Maximum mining rarity the weapon can mine. */

   double ammo_mass; /**< How heavy it is. */
   double duration;  /**< How long the ammo lives. */
   double resist;    /**< Lowers chance of jamming by this amount */
   OutfitAmmoAI ai;  /**< Smartness of ammo. */

   double speed;     /**< Initial speed. */
   double speed_max; /**< Maximum speed. Defaults to speed if not set. */
   double turn;      /**< Turn velocity in rad/s. */
   double accel;     /**< Acceleration */
   double energy;    /**< Energy usage */
   Damage dmg;       /**< Damage done. */
   double radius;    /**< Explosion radius. */

   /* Health stuff. */
   double armour;    /**< Core health. */
   double dmg_absorb; /**< Damage absorption. */

   OutfitGFX gfx;    /**< Rendering information. */
   int sound;        /**< sound to play */
   int sound_hit;    /**< Sound to play on hit. */
   int spfx_armour;  /**< special effect on hit */
   int spfx_shield;  /**< special effect on hit */
   const TrailSpec* trail_spec; /**< Trail style if applicable, else NULL. */
   double trail_x_offset; /**< Offset x. */
} OutfitLauncherData;

/**
 * @brief Represents a ship modification.
 *
 * These modify the ship's basic properties when equipped on a pilot.
 */
typedef struct OutfitModificationData_ {
   /* Active information (if applicable). */
   int active;       /**< Outfit is active. */
   double duration;  /**< Time the active outfit stays on (in seconds). */
   double cooldown;  /**< Time the active outfit stays off after it's duration (in seconds). */

   /* All the modifiers are based on the outfit's ship stats, nothing here but active stuff. */
} OutfitModificationData;

/**
 * @brief Represents an afterburner.
 */
typedef struct OutfitAfterburnerData_ {
   /* Internal properties. */
   double rumble;    /**< Percent of rumble */
   int sound_on;     /**< Sound of the afterburner turning on */
   int sound;        /**< Sound of the afterburner being on */
   int sound_off;    /**< Sound of the afterburner turning off */
   double accel;     /**< Percent of accel increase based on ship base. */
   double speed;     /**< Percent of speed to increase based on ship base. */
   double energy;    /**< Energy usage while active */
   double mass_limit;/**< Limit at which effectiveness starts to drop. */
   double heatup;    /**< How long it takes for the afterburner to overheat. */
   double heat;      /**< Heat per second. */
   double heat_cap;  /**< Temperature at which the outfit overheats (K). */
   double heat_base; /**< Temperature at which the outfit BEGINS to overheat(K). */
} OutfitAfterburnerData;

/**
 * @brief Represents a fighter bay.
 */
typedef struct OutfitFighterBayData_ {
   char *ship;       /**< Name of the ships to use as ammo. */
   double ship_mass; /**< Mass of a fighter. */
   const struct Outfit_ *ammo; /**< Ships to use as ammo. */
   double delay;     /**< Delay between launches. */
   int amount;       /**< Amount of ammo it can store. */
   double reload_time;/**< Time it takes to reload 1 ammo. */
   int sound;        /**< Sound to use when launching. */
} OutfitFighterBayData;

/* Forward declaration */
struct OutfitMapData_s;
typedef struct OutfitMapData_s OutfitMapData_t;

/**
 * @brief Represents a local map.
 */
typedef struct OutfitLocalMapData_ {
   double jump_detect;  /**< Ability to detect jumps. */
   double spob_detect; /**< Ability to detect spobs. */
} OutfitLocalMapData;

/**
 * @brief Represents a GUI.
 */
typedef struct OutfitGUIData_ {
   char *gui;  /**< Name of the GUI file. */
} OutfitGUIData;

/**
 * @brief Represents a license.
 */
typedef struct OutfitLicenseData_ {
   char *provides; /**< License provided by this license (defaults to name if NULL). */
} OutfitLicenseData;

/**
 * @brief A ship outfit, depends radically on the type.
 */
typedef struct Outfit_ {
   char *name;       /**< Name of the outfit. */
   char *typename;   /**< Overrides the base type. */
   int rarity;       /**< Rarity of the outfit. */
   char *filename;   /**< File data was loaded from. */

   /* General specs */
   OutfitSlot slot;  /**< Slot the outfit fits into. */
   char *license;    /**< Licenses needed to buy it. */
   char *cond;       /**< Conditional Lua string. */
   char *condstr;    /**< Human readable description of the conditional. */
   double mass;      /**< How much weapon capacity is needed. */
   double cpu;       /**< CPU usage. */
   char *limit;      /**< Name to limit to one per ship (ignored if NULL). */
   int *illegalto;   /**< Factions this outfit is illegal to. */
   char **illegaltoS;/**< Temporary buffer to set up illegality. */

   /* Store stuff */
   credits_t price;  /**< Base sell price. */
   char *desc_raw;   /**< Base store description. */
   char *summary_raw;/**< Short outfit summary (stored translated). */
   char *desc_extra; /**< Extra description string (if static). */
   int priority;     /**< Sort priority, highest first. */

   glTexture *gfx_store;   /**< Store graphic. */
   glTexture **gfx_overlays;/**< Array (array.h): Store overlay graphics. */

   unsigned int properties;/**< Properties stored bitwise. */
   unsigned int group;     /**< Weapon group to use when autoweap is enabled. */

   /* Stats. */
   ShipStatList *stats; /**< Stat list. */

   /* Tags. */
   char **tags;      /**< Outfit tags. */

   /* Lua function references. Set to LUA_NOREF if not used. */
   char *lua_file;   /**< Lua File. */
   nlua_env lua_env; /**< Lua environment. Shared for each outfit to allow globals. */
   int lua_descextra;/**< Run to get the extra description status. */
   int lua_onadd;    /**< Run when added to a pilot or player swaps to this ship. */
   int lua_onremove; /**< Run when removed to a pilot or when player swaps away from this ship. */
   int lua_init;     /**< Run when pilot enters a system. */
   int lua_cleanup;  /**< Run when the pilot is erased. */
   int lua_update;   /**< Run periodically. */
   int lua_ontoggle; /**< Run when toggled. */
   int lua_onhit;    /**< Run when pilot takes damage. */
   int lua_outofenergy;/**< Run when the pilot runs out of energy. */
   int lua_onshoot;  /**< Run when pilot shoots. */
   int lua_onstealth;/**< Run when pilot toggles stealth. */
   int lua_onscanned;/**< Run when the pilot is scanned by another pilot. */
   int lua_onscan;   /**< Run when the pilot scans another pilot. */
   int lua_cooldown; /**< Run when cooldown is started or stopped. */
   int lua_land;     /**< Run when the player lands. */
   int lua_takeoff;  /**< Run when the player takes off. */
   int lua_jumpin;   /**< Run when the player jumps in. */
   int lua_board;    /**< Run when the player boards a ship. */
   int lua_keydoubletap; /**< Run when a key is double tapped. */
   int lua_keyrelease; /**< Run when a key is released. */
   /* Weapons only. */
   int lua_onimpact; /**< Run when weapon hits the enemy. */
   int lua_onmiss;   /**< Run when weapon particle expires. */
   /* Independent of slots and pilots. */
   int lua_price;    /**< Determines the "cost" string and whether or not the player can buy or sell the outfit when available. */
   int lua_buy;      /**< Run when the outfit is boughten. */
   int lua_sell;     /**< Run when the outfit is sold. */

   /* Type dependent */
   OutfitType type; /**< Type of the outfit. */
   union {
      OutfitBoltData blt;         /**< BOLT */
      OutfitBeamData bem;         /**< BEAM */
      OutfitLauncherData lau;     /**< MISSILE */
      OutfitModificationData mod; /**< MODIFICATION */
      OutfitAfterburnerData afb;  /**< AFTERBURNER */
      OutfitFighterBayData bay;   /**< FIGHTER_BAY */
      OutfitMapData_t *map;       /**< MAP */
      OutfitLocalMapData lmap;    /**< LOCALMAP */
      OutfitGUIData gui;          /**< GUI */
      OutfitLicenseData lic;      /**< LICENSE. */
   } u; /**< Holds the type-based outfit data. */
} Outfit;

/*
 * Access stuff.
 */
const Outfit* outfit_get( const char* name );
const Outfit* outfit_getW( const char* name );
const Outfit* outfit_getAll (void);
int outfit_compareTech( const void *outfit1, const void *outfit2 );
/* outfit types */
int outfit_isActive( const Outfit *o );
int outfit_isToggleable( const Outfit *o );
int outfit_isWeapon( const Outfit *o );
int outfit_isForward( const Outfit *o );
int outfit_isBolt( const Outfit *o );
int outfit_isBeam( const Outfit *o );
int outfit_isLauncher( const Outfit *o );
int outfit_isAmmo( const Outfit *o );
int outfit_isSeeker( const Outfit *o );
int outfit_isTurret( const Outfit *o );
int outfit_isMod( const Outfit *o );
int outfit_isAfterburner( const Outfit *o );
int outfit_isFighterBay( const Outfit *o );
int outfit_isMap( const Outfit *o );
int outfit_isLocalMap( const Outfit *o );
int outfit_isGUI( const Outfit *o );
int outfit_isLicense( const Outfit *o );
int outfit_isSecondary( const Outfit *o );
const char* outfit_getType( const Outfit *o );
const char* outfit_getTypeBroad( const Outfit *o );
const char* outfit_getAmmoAI( const Outfit *o );
const char* outfit_description( const Outfit *o );
const char* outfit_summary( const Outfit *o, int withname );

/*
 * Search.
 */
const char *outfit_existsCase( const char* name );
char **outfit_searchFuzzyCase( const char* name, int *n );

/*
 * Filter.
 */
int outfit_filterWeapon( const Outfit *o );
int outfit_filterUtility( const Outfit *o );
int outfit_filterStructure( const Outfit *o );
int outfit_filterCore( const Outfit *o );
int outfit_filterOther( const Outfit *o );

/*
 * Get data from outfits.
 */
const char *outfit_slotName( const Outfit* o );
const char *slotName( const OutfitSlotType o );
const char *outfit_slotSize( const Outfit* o );
const char *slotSize( const OutfitSlotSize o );
const glColour *outfit_slotSizeColour( const OutfitSlot* os );
char outfit_slotSizeColourFont( const OutfitSlot* os );
char outfit_slotTypeColourFont( const OutfitSlot* os );
size_t outfit_getNameWithClass( const Outfit* outfit, char* buf, size_t size );
OutfitSlotSize outfit_toSlotSize( const char *s );
const OutfitGFX* outfit_gfx( const Outfit* o );
const CollPoly* outfit_plg( const Outfit* o );
int outfit_spfxArmour( const Outfit* o );
int outfit_spfxShield( const Outfit* o );
const Damage *outfit_damage( const Outfit* o );
double outfit_radius( const Outfit* o );
double outfit_delay( const Outfit* o );
int outfit_amount( const Outfit* o );
double outfit_energy( const Outfit* o );
double outfit_heat( const Outfit* o );
double outfit_cpu( const Outfit* o );
double outfit_range( const Outfit* o );
double outfit_speed( const Outfit* o );
double outfit_spin( const Outfit* o );
double outfit_trackmin( const Outfit* o );
double outfit_trackmax( const Outfit* o );
int outfit_miningRarity( const Outfit* o );
int outfit_sound( const Outfit* o );
int outfit_soundHit( const Outfit* o );
double outfit_ammoMass( const Outfit *o );
/* Active outfits. */
double outfit_duration( const Outfit* o );
double outfit_cooldown( const Outfit* o );

/*
 * Loading and freeing outfit stack.
 */
int outfit_load (void);
int outfit_loadPost (void);
int outfit_mapParse(void);
void outfit_free (void);

/*
 * Misc.
 */
int outfit_fitsSlot( const Outfit* o, const OutfitSlot* s );
int outfit_fitsSlotType( const Outfit* o, const OutfitSlot* s );
void outfit_freeSlot( OutfitSlot* s );
glTexture* rarity_texture( int rarity );
int outfit_checkIllegal( const Outfit* o, int fct );
int outfit_licenseExists( const char *name );
