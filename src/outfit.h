/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "collision.h"
#include "commodity.h"
#include "nlua.h"
#include "opengl_tex.h"
#include "physics.h"
#include "shipstats.h"
#include "sound.h"
#include "spfx.h"

/*
 * properties
 */
/* property flags */
#define OUTFIT_PROP_UNIQUE                                                     \
   ( 1 << 0 ) /**< Unique item (can only have one). Not sellable.*/
#define OUTFIT_PROP_SHOOT_DRY                                                  \
   ( 1 << 1 ) /**< Weapon that doesn't actually create particles. Should be    \
                 handled in ontoggle. */
#define OUTFIT_PROP_TEMPLATE                                                   \
   ( 1 << 2 ) /**< Outfit is meant to be used as a template for other outfits. \
                 Ignores warnings. */
#define OUTFIT_PROP_WEAP_SECONDARY ( 1 << 3 ) /**< Is a secondary weapon? */
#define OUTFIT_PROP_WEAP_SPIN ( 1 << 4 )      /**< Should weapon spin around? */
#define OUTFIT_PROP_WEAP_BLOWUP_ARMOUR                                         \
   ( 1 << 5 ) /**< Weapon blows up (armour spfx)                               \
                   when timer is up. */
#define OUTFIT_PROP_WEAP_BLOWUP_SHIELD                                         \
   ( 1 << 6 ) /**< Weapon blows up (shield spfx)                               \
                   when timer is up. */
#define OUTFIT_PROP_WEAP_FRIENDLYFIRE                                          \
   ( 1 << 8 ) /**< Weapon damages all ships when blowing up. */
#define OUTFIT_PROP_WEAP_POINTDEFENSE                                          \
   ( 1 << 9 ) /**< Weapon can hit ammunitions. */
#define OUTFIT_PROP_WEAP_MISS_SHIPS                                            \
   ( 1 << 10 ) /**< Weapon can not hit ships. */
#define OUTFIT_PROP_WEAP_MISS_ASTEROIDS                                        \
   ( 1 << 11 ) /**< Weapon can not hit asteroids. */
#define OUTFIT_PROP_WEAP_MISS_EXPLODE                                          \
   ( 1 << 12 ) /**< The weapon particle blows up on miss. */
#define OUTFIT_PROP_WEAP_ONLYHITTARGET                                         \
   ( 1 << 13 ) /**< The weapon can only hit the target (and asteroids or       \
                  whatever). */
#define OUTFIT_PROP_WEAP_COLLISION_OVERRIDE                                    \
   ( 1 << 14 ) /**< Weapon uses collision sphere irregardless of graphics.  */
#define OUTFIT_PROP_NEEDSGFX                                                   \
   ( 1 << 15 ) /**< The outfit needs to load graphics. */
#define OUTFIT_PROP_STEALTH_ON                                                 \
   ( 1 << 16 ) /**< The outfit does not get turned off by stealth. */
#define OUTFIT_PROP_HIDESTATS                                                  \
   ( 1 << 17 ) /**< Hide the outfit shipstats from Lua stuff in the            \
                  descriptions. */
// We use a core property obtained from tags for now as we need a way to
// distinguish them
// TODO remove when we use a more flexible / better tab system
#define OUTFIT_PROP_CORE ( 1 << 18 ) /**< Outfit is a core. */

/* Outfit filter labels. [Doc comments are also translator notes and must
 * precede the #define.] */
/** Colour-coded abbreviation for "Weapon [outfit]", short enough to use as a
 * tab/column title. */
#define OUTFIT_LABEL_WEAPON N_( "#pWeapon" )
/** Colour-coded abbreviation for "Utility [outfit]", short enough to use as a
 * tab/column title. */
#define OUTFIT_LABEL_UTILITY N_( "#gUtility" )
/** Colour-coded abbreviation for "Structure [outfit]", short enough to use as a
 * tab/column title. */
#define OUTFIT_LABEL_STRUCTURE N_( "#nStructural" )
/** Colour-coded abbreviation for "Core [outfit]", short enough to use as a
 * tab/column title. */
#define OUTFIT_LABEL_CORE N_( "#oCore" )

/**
 * @brief Different types of existing outfits.
 *
 * Outfits are organized by the order here
 *
 * @note If you modify this DON'T FORGET TO MODIFY outfit_getType too!!!
 */
typedef enum OutfitType_ {
   OUTFIT_TYPE_NULL,            /**< Null type. */
   OUTFIT_TYPE_BOLT,            /**< Fixed bolt cannon. */
   OUTFIT_TYPE_BEAM,            /**< Fixed beam cannon. */
   OUTFIT_TYPE_TURRET_BOLT,     /**< Rotary bolt turret. */
   OUTFIT_TYPE_TURRET_BEAM,     /**< Rotary beam turret. */
   OUTFIT_TYPE_LAUNCHER,        /**< Launcher. */
   OUTFIT_TYPE_TURRET_LAUNCHER, /**< Turret launcher. */
   OUTFIT_TYPE_MODIFICATION,    /**< Modifies the ship base features. */
   OUTFIT_TYPE_AFTERBURNER,     /**< Gives the ship afterburn capability. */
   OUTFIT_TYPE_FIGHTER_BAY,     /**< Contains other ships. */
   OUTFIT_TYPE_MAP,      /**< Gives the player more knowledge about systems. */
   OUTFIT_TYPE_LOCALMAP, /**< Gives the player more knowledge about the current
                            system. */
   OUTFIT_TYPE_GUI,      /**< GUI for the player. */
   OUTFIT_TYPE_LICENSE, /**< License that allows player to buy special stuff. */
   OUTFIT_TYPE_SENTINEL /**< indicates last type */
} OutfitType;

/**
 * @brief Outfit slot types.
 */
typedef enum OutfitSlotType_ {
   OUTFIT_SLOT_NULL,      /**< Invalid slot type. */
   OUTFIT_SLOT_NA,        /**< Slot type not applicable. */
   OUTFIT_SLOT_INTRINSIC, /**< Internal outfits that don't use slots. */
   OUTFIT_SLOT_STRUCTURE, /**< Low energy slot. */
   OUTFIT_SLOT_UTILITY,   /**< Medium energy slot. */
   OUTFIT_SLOT_WEAPON     /**< High energy slot. */
} OutfitSlotType;

/**
 * @brief Outfit slot sizes.
 */
typedef enum OutfitSlotSize_ {
   OUTFIT_SLOT_SIZE_NA,     /**< Not applicable slot size. */
   OUTFIT_SLOT_SIZE_LIGHT,  /**< Light slot size. */
   OUTFIT_SLOT_SIZE_MEDIUM, /**< Medium slot size. */
   OUTFIT_SLOT_SIZE_HEAVY   /**< Heavy slot size. */
} OutfitSlotSize;

/**
 * @brief Ammo AI types.
 */
typedef enum OutfitAmmoAI_ {
   AMMO_AI_UNGUIDED, /**< No AI. */
   AMMO_AI_SEEK,     /**< Aims at the target. */
   AMMO_AI_SMART /**< Aims at the target, correcting for relative velocity. */
} OutfitAmmoAI;

/**
 * @brief Pilot slot that can contain outfits.
 */
typedef struct OutfitSlot_ {
   unsigned int   spid;      /**< Slot property ID. */
   int            exclusive; /**< Outfit must go exclusively into the slot. */
   OutfitSlotType type;      /**< Type of outfit slot. */
   OutfitSlotSize size;      /**< Size of the outfit. */
} OutfitSlot;

/**
 * @brief
 */
typedef struct OutfitGFX_ {
   glTexture *tex;        /**< Graphic in case of texture. */
   glTexture *tex_end;    /**< End texture if applicable. */
   CollPoly   polygon;    /**< Collision polygon. */
   double     spin;       /**< Graphic spin rate. */
   GLuint     program;    /**< Shader program. */
   GLuint     vertex;     /**< Vertex info. */
   GLuint     projection; /**< Projection matrix. */
   GLuint     dimensions; /**< Dimensions of the rendered object. */
   GLuint     u_r;        /**< Random value uniform. */
   GLuint     u_time;     /**< Elapsed time uniform. */
   GLuint     u_fade;     /**< Fade factor uniform. */
   double     size;       /**< Size to render at. */
   double     col_size;   /**< Size of the collision object. */
} OutfitGFX;

/**
 * @brief Core damage that an outfit does.
 */
typedef struct Damage_ {
   int type; /**< Type of damage. */
   double
      penetration; /**< Penetration the damage has [0:1], with 1 being 100%. */
   double
      damage; /**< Amount of damage, this counts towards killing the ship. */
   double disable; /**< Amount of disable damage, this counts towards disabling
                      the ship. */
} Damage;

/**
 * @brief Represents the particular properties of a bolt weapon.
 */
typedef struct OutfitBoltData_ {
   double delay;   /**< Delay between shots */
   double speed;   /**< How fast it goes. */
   double range;   /**< How far it goes. */
   double falloff; /**< Point at which damage falls off. */
   double energy;  /**< Energy usage */
   Damage dmg;     /**< Damage done. */
   double radius;  /**< Explosion radius .*/

   double trackmin;   /**< Ewarfare minimal tracking. */
   double trackmax;   /**< Ewarfare maximal (optimal) tracking. */
   double swivel;     /**< Amount of swivel (semiarc in radians of deviation the
                         weapon can correct). */
   double dispersion; /**< Angle amount to spread particles around. */
   double speed_dispersion; /**< Dispersion, but for speed. */
   int    shots;            /**< Number of particles shot when fired. */
   int    mining_rarity;    /**< Maximum mining rarity the weapon can mine. */

   /* Sound and graphics. */
   OutfitGFX    gfx;         /**< Rendering information. */
   const Sound *sound;       /**< Sound to play on shoot.*/
   const Sound *sound_hit;   /**< Sound to play on hit. */
   int          spfx_armour; /**< special effect on hit. */
   int          spfx_shield; /**< special effect on hit. */
} OutfitBoltData;

typedef struct BeamShader {
   GLuint program;
   GLuint vertex;
   GLuint projection;
   GLuint colour;
   GLuint dt;
   GLuint r;
   GLuint dimensions;
} BeamShader;

/**
 * @brief Represents the particular properties of a beam weapon.
 */
typedef struct OutfitBeamData_ {
   /* Time stuff. */
   double delay;     /**< Delay between usage. */
   double warmup;    /**< How long beam takes to warm up. */
   double duration;  /**< How long the beam lasts active. */
   double min_delay; /**< Minimum delay between firing the  beam. */

   /* Beam properties. */
   double range;      /**< how far it goes */
   double turn;       /**< How fast it can turn. Only for turrets, in rad/s. */
   double energy;     /**< Amount of energy it drains (per second). */
   Damage dmg;        /**< Damage done. */
   double swivel;     /**< Amount of swivel (semiarc in radians of deviation the
                         weapon can correct). */
   int mining_rarity; /**< Maximum mining rarity the weapon can mine. */

   /* Graphics and sound. */
   glColour     colour; /**< Colour to use for the shader. */
   GLfloat      width;  /**< Width of the beam. */
   BeamShader   shader;
   int          spfx_armour;  /**< special effect on hit */
   int          spfx_shield;  /**< special effect on hit */
   const Sound *sound_warmup; /**< Sound to play when warming up. @todo use. */
   const Sound *sound;        /**< Sound to play. */
   const Sound *sound_off;    /**< Sound to play when turning off. */
} OutfitBeamData;

/**
 * @brief Represents a particular missile launcher.
 *
 * The properties of the weapon are highly dependent on the ammunition.
 */
typedef struct OutfitLauncherData_ {
   double delay;       /**< Delay between shots. */
   int    amount;      /**< Amount of ammo it can store. */
   double reload_time; /**< Time it takes to reload 1 ammo. */

   /* Lock-on information. */
   double lockon;     /**< Time it takes to lock on the target */
   double iflockon;   /**< Time it takes to lock on properly after launch. */
   double trackmin;   /**< Ewarfare minimal tracking. */
   double trackmax;   /**< Ewarfare maximal (optimal) tracking. */
   double arc;        /**< Semi-angle of the arc which it will lock on in. */
   double swivel;     /**< Amount of swivel (semiarc in radians of deviation the
                         weapon can correct when launched). */
   double dispersion; /**< Angle amount to spread particles around. */
   double speed_dispersion; /**< Dispersion, but for speed. */
   int    shots;            /**< Number of particles shot when fired. */
   int    mining_rarity;    /**< Maximum mining rarity the weapon can mine. */

   double       ammo_mass; /**< How heavy it is. */
   double       duration;  /**< How long the ammo lives. */
   double       resist;    /**< Lowers chance of jamming by this amount */
   OutfitAmmoAI ai;        /**< Smartness of ammo. */

   double speed;     /**< Initial speed. */
   double speed_max; /**< Maximum speed. Defaults to speed if not set. */
   double turn;      /**< Turn velocity in rad/s. */
   double accel;     /**< Acceleration */
   double energy;    /**< Energy usage */
   Damage dmg;       /**< Damage done. */
   double radius;    /**< Explosion radius. */

   /* Health stuff. */
   double armour;     /**< Core health. */
   double dmg_absorb; /**< Damage absorption. */

   OutfitGFX        gfx;         /**< Rendering information. */
   const Sound     *sound;       /**< sound to play */
   const Sound     *sound_hit;   /**< Sound to play on hit. */
   int              spfx_armour; /**< special effect on hit */
   int              spfx_shield; /**< special effect on hit */
   const TrailSpec *trail_spec;  /**< Trail style if applicable, else NULL. */
   double           trail_x_offset; /**< Offset x. */
} OutfitLauncherData;

/**
 * @brief Represents a ship modification.
 *
 * These modify the ship's basic properties when equipped on a pilot.
 */
typedef struct OutfitModificationData_ {
   /* Active information (if applicable). */
   int    active;   /**< Outfit is active. */
   double duration; /**< Time the active outfit stays on (in seconds). */
   double cooldown; /**< Time the active outfit stays off after its duration
                       (in seconds). */

   /* All the modifiers are based on the outfit's ship stats, nothing here but
    * active stuff. */
} OutfitModificationData;

/**
 * @brief Represents an afterburner.
 */
typedef struct OutfitAfterburnerData_ {
   /* Internal properties. */
   double       rumble;    /**< Percent of rumble */
   const Sound *sound_on;  /**< Sound of the afterburner turning on */
   const Sound *sound;     /**< Sound of the afterburner being on */
   const Sound *sound_off; /**< Sound of the afterburner turning off */
   double       accel;     /**< Percent of accel increase based on ship base. */
   double       speed;  /**< Percent of speed to increase based on ship base. */
   double       energy; /**< Energy usage while active */
   double       mass_limit; /**< Limit at which effectiveness starts to drop. */
} OutfitAfterburnerData;

struct Ship; /* Bit of a horrible hack to allow us to avoid circular
                 definitions. */
/**
 * @brief Represents a fighter bay.
 */
typedef struct OutfitFighterBayData_ {
   char              *shipname;    /**< Name of the ships to use as ammo. */
   const struct Ship *ship;        /**< Ship to use as ammo. */
   double             ship_mass;   /**< Mass of a fighter. */
   double             delay;       /**< Delay between launches. */
   int                amount;      /**< Amount of ammo it can store. */
   double             reload_time; /**< Time it takes to reload 1 ammo. */
   const Sound       *sound;       /**< Sound to use when launching. */
} OutfitFighterBayData;

/* Forward declaration */
struct OutfitMapData_s;
typedef struct OutfitMapData_s OutfitMapData_t;

/**
 * @brief Represents a local map.
 */
typedef struct OutfitLocalMapData_ {
   double jump_detect; /**< Ability to detect jumps. */
   double spob_detect; /**< Ability to detect spobs. */
   int range; /**< Distance at which applied. 0 only applies to current system.
               */
} OutfitLocalMapData;

/**
 * @brief Represents a GUI.
 */
typedef struct OutfitGUIData_ {
   char *gui; /**< Name of the GUI file. */
} OutfitGUIData;

/**
 * @brief Represents a license.
 */
typedef struct OutfitLicenseData_ {
   char *provides; /**< License provided by this license (defaults to name if
                      NULL). */
} OutfitLicenseData;

/**
 * @brief A ship outfit, depends radically on the type.
 */
typedef struct Outfit {
   char *name;      /**< Name of the outfit. */
   char *display;   /**< Display name of the outfit. */
   char *typename;  /**< Overrides the base type. */
   char *shortname; /**< Shorter version of the name for GUI and such. */
   int   rarity;    /**< Rarity of the outfit. */
   char *filename;  /**< File data was loaded from. */

   /* General specs */
   OutfitSlot   slot;       /**< Slot the outfit fits into. */
   unsigned int spid_extra; /**< Can also fit this slot. */
   char        *license;    /**< Licenses needed to buy it. */
   char        *cond;       /**< Conditional Lua string. */
   char        *condstr; /**< Human readable description of the conditional. */
   double       mass;    /**< How much weapon capacity is needed. */
   double       cpu;     /**< CPU usage. */
   char        *limit; /**< Name to limit to one per ship (ignored if NULL). */
   int         *illegalto;  /**< Factions this outfit is illegal to. */
   char       **illegaltoS; /**< Temporary buffer to set up illegality. */

   /* Store stuff */
   credits_t price;       /**< Base sell price. */
   char     *desc_raw;    /**< Base store description. */
   char     *summary_raw; /**< Short outfit summary (stored translated). */
   char     *desc_extra;  /**< Extra description string (if static). */
   int       priority;    /**< Sort priority, highest first. */

   char       *gfx_store_path; /**< Store graphic path. */
   glTexture  *gfx_store;      /**< Store graphic. */
   glTexture **gfx_overlays;   /**< Array (array.h): Store overlay graphics. */

   unsigned int properties; /**< Properties stored bitwise. */

   /* Stats. */
   ShipStatList *stats; /**< Stat list. */

   /* Tags. */
   char **tags; /**< Outfit tags. */

   /* Lua function references. Set to LUA_NOREF if not used. */
   char *lua_file;   /**< Lua File. */
   char *lua_inline; /**< Inline Lua. */
   nlua_env *
      lua_env; /**< Lua environment. Shared for each outfit to allow globals. */
   int lua_descextra; /**< Run to get the extra description status. */
   int lua_onadd; /**< Run when added to a pilot or player adds this outfit. */
   int lua_onremove; /**< Run when removed to a pilot or when player removes
                                                this outfit. */
   int lua_onoutfitchange; /**< Run when any outfit is changed on the ship (not
                                                             just this one. */
   int lua_init;           /**< Run when pilot enters a system. */
   int lua_cleanup;        /**< Run when the pilot is erased. */
   int lua_update;         /**< Run periodically. */
   int lua_ontoggle;       /**< Run when toggled. */
   int lua_onshoot;        /**< Run when shooting. */
   int lua_onhit;          /**< Run when pilot takes damage. */
   int lua_outofenergy;    /**< Run when the pilot runs out of energy. */
   int lua_onshootany;     /**< Run when pilot shoots ANY weapon. */
   int lua_onstealth;      /**< Run when pilot toggles stealth. */
   int lua_onscanned;    /**< Run when the pilot is scanned by another pilot. */
   int lua_onscan;       /**< Run when the pilot scans another pilot. */
   int lua_cooldown;     /**< Run when cooldown is started or stopped. */
   int lua_land;         /**< Run when the player lands. */
   int lua_takeoff;      /**< Run when the player takes off. */
   int lua_jumpin;       /**< Run when the player jumps in. */
   int lua_board;        /**< Run when the player boards a ship. */
   int lua_keydoubletap; /**< Run when a key is double tapped. */
   int lua_keyrelease;   /**< Run when a key is released. */
   int lua_message;      /**< Run when an outfit receives a message via Lua. */
   int lua_ondeath;      /**< Run when pilot is killed. */
   int lua_onanyimpact;  /**< Run when any weapon hits the enemy. */
   /* Weapons only. */
   int lua_onimpact; /**< Run when weapon hits the enemy. */
   int lua_onmiss;   /**< Run when weapon particle expires. */
   /* Independent of slots and pilots. */
   int lua_price; /**< Determines the "cost" string and whether or not the
                     player can buy or sell the outfit when available. */
   int lua_buy;   /**< Run when the outfit is boughten. */
   int lua_sell;  /**< Run when the outfit is sold. */

   /* Type dependent */
   OutfitType type; /**< Type of the outfit. */
   union {
      OutfitBoltData         blt;  /**< BOLT */
      OutfitBeamData         bem;  /**< BEAM */
      OutfitLauncherData     lau;  /**< LAUNCHER */
      OutfitModificationData mod;  /**< MODIFICATION */
      OutfitAfterburnerData  afb;  /**< AFTERBURNER */
      OutfitFighterBayData   bay;  /**< FIGHTER_BAY */
      OutfitMapData_t       *map;  /**< MAP */
      OutfitLocalMapData     lmap; /**< LOCALMAP */
      OutfitGUIData          gui;  /**< GUI */
      OutfitLicenseData      lic;  /**< LICENSE. */
   } u;                            /**< Holds the type-based outfit data. */
} Outfit;

/*
 * Access stuff.
 */
int            outfit_gfxStoreLoaded( const Outfit *o );
int            outfit_gfxStoreLoadNeeded( void );
int            outfit_gfxStoreLoad( Outfit *o );
const Outfit  *outfit_get( const char *name );
const Outfit  *outfit_getW( const char *name );
const Outfit **outfit_getAll( void );
Outfit        *outfit_getAll_rust( void );
int            outfit_compareTech( const void *outfit1, const void *outfit2 );
int            outfit_isProp( const Outfit *o, unsigned int prop );
void           outfit_setProp( Outfit *o, unsigned int prop );
void           outfit_rmProp( Outfit *o, unsigned int prop );
double         outfit_mass( const Outfit *o );
double         outfit_massAmmo( const Outfit *o );
const char    *outfit_license( const Outfit *o );
credits_t      outfit_price( const Outfit *o );
const char    *outfit_getPrice( const Outfit *outfit, unsigned int q,
                                credits_t *price, int *canbuy, int *cansell,
                                char **player_has, const char **reason );
/* outfit types */
int            outfit_isActive( const Outfit *o );
int            outfit_isToggleable( const Outfit *o );
int            outfit_isWeapon( const Outfit *o );
int            outfit_isForward( const Outfit *o );
int            outfit_isBolt( const Outfit *o );
int            outfit_isBeam( const Outfit *o );
int            outfit_isLauncher( const Outfit *o );
int            outfit_isAmmo( const Outfit *o );
int            outfit_isSeeker( const Outfit *o );
int            outfit_isTurret( const Outfit *o );
int            outfit_isMod( const Outfit *o );
int            outfit_isAfterburner( const Outfit *o );
int            outfit_isFighterBay( const Outfit *o );
int            outfit_isMap( const Outfit *o );
int            outfit_isLocalMap( const Outfit *o );
int            outfit_isGUI( const Outfit *o );
int            outfit_isLicense( const Outfit *o );
int            outfit_isSecondary( const Outfit *o );
const char    *outfit_getType( const Outfit *o );
const char    *outfit_getTypeBroad( const Outfit *o );
const char    *outfit_getAmmoAI( const Outfit *o );
const char    *outfit_description( const Outfit *o );
const char    *outfit_summary( const Outfit *o, int withname );
const char    *outfit_rawname( const Outfit *o );
const char    *outfit_name( const Outfit *o );
const char    *outfit_shortname( const Outfit *o );
const char    *outfit_cond( const Outfit *o );
const char    *outfit_condstr( const Outfit *o );
const char    *outfit_limit( const Outfit *o );
OutfitType     outfit_type( const Outfit *o );
OutfitSlotType outfit_slotType( const Outfit *o );
OutfitSlotSize outfit_slotSize( const Outfit *o );
int            outfit_slotProperty( const Outfit *o );
int            outfit_slotPropertyExtra( const Outfit *o );
int            outfit_slotExclusive( const Outfit *o );
int            outfit_rarity( const Outfit *o );
char          *outfit_descExtra( const Outfit *o );
char          *outfit_descRaw( const Outfit *o );
char          *outfit_summaryRaw( const Outfit *o );
/* Lua stuff. */
nlua_env *outfit_luaEnv( const Outfit *o );
int       outfit_luaDescextra( const Outfit *o );
int       outfit_luaOnadd( const Outfit *o );
int       outfit_luaOnremove( const Outfit *o );
int       outfit_luaOnoutfitchange( const Outfit *o );
int       outfit_luaInit( const Outfit *o );
int       outfit_luaCleanup( const Outfit *o );
int       outfit_luaUpdate( const Outfit *o );
int       outfit_luaOntoggle( const Outfit *o );
int       outfit_luaOnshoot( const Outfit *o );
int       outfit_luaOnhit( const Outfit *o );
int       outfit_luaOutofenergy( const Outfit *o );
int       outfit_luaOnshootany( const Outfit *o );
int       outfit_luaOnstealth( const Outfit *o );
int       outfit_luaOnscanned( const Outfit *o );
int       outfit_luaOnscan( const Outfit *o );
int       outfit_luaCooldown( const Outfit *o );
int       outfit_luaLand( const Outfit *o );
int       outfit_luaTakeoff( const Outfit *o );
int       outfit_luaJumpin( const Outfit *o );
int       outfit_luaBoard( const Outfit *o );
int       outfit_luaKeydoubletap( const Outfit *o );
int       outfit_luaKeyrelease( const Outfit *o );
int       outfit_luaMessage( const Outfit *o );
int       outfit_luaOndeath( const Outfit *o );
int       outfit_luaOnanyimpact( const Outfit *o );
int       outfit_luaOnImpact( const Outfit *o );
int       outfit_luaOnMiss( const Outfit *o );
int       outfit_luaPrice( const Outfit *o );
int       outfit_luaBuy( const Outfit *o );
int       outfit_luaSell( const Outfit *o );

/*
 * Search.
 */
const char *outfit_existsCase( const char *name );
char      **outfit_searchFuzzyCase( const char *name, int *n );

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
const char     *outfit_slotName( const Outfit *o );
const char     *slotName( const OutfitSlotType o );
const char     *outfit_slotSizeName( const Outfit *o );
const char     *slotSize( const OutfitSlotSize o );
const glColour *outfit_slotSizeColour( OutfitSlotSize size );
char            outfit_slotSizeColourFont( OutfitSlotSize size );
char            outfit_slotTypeColourFont( OutfitSlotType type );
size_t outfit_getNameWithClass( const Outfit *outfit, char *buf, size_t size );
OutfitSlotSize      outfit_toSlotSize( const char *s );
const OutfitGFX    *outfit_gfx( const Outfit *o );
const glTexture    *outfit_gfxStore( const Outfit *o );
const glTexture   **outfit_gfxOverlays( const Outfit *o );
const CollPoly     *outfit_plg( const Outfit *o );
const ShipStatList *outfit_stats( const Outfit *o );
int                 outfit_spfxArmour( const Outfit *o );
int                 outfit_spfxShield( const Outfit *o );
const Damage       *outfit_damage( const Outfit *o );
double              outfit_radius( const Outfit *o );
double              outfit_delay( const Outfit *o );
int                 outfit_amount( const Outfit *o );
int                 outfit_reloadTime( const Outfit *o );
double              outfit_energy( const Outfit *o );
double              outfit_cpu( const Outfit *o );
double              outfit_rangeRaw( const Outfit *o );
double              outfit_range( const Outfit *o );
double              outfit_speed( const Outfit *o );
double              outfit_width( const Outfit *o );
const glColour     *outfit_colour( const Outfit *o );
double              outfit_falloff( const Outfit *o );
double              outfit_turn( const Outfit *o );
double              outfit_swivel( const Outfit *o );
double              outfit_spin( const Outfit *o );
double              outfit_trackmin( const Outfit *o );
double              outfit_trackmax( const Outfit *o );
int                 outfit_miningRarity( const Outfit *o );
const Sound        *outfit_sound( const Outfit *o );
const Sound        *outfit_soundHit( const Outfit *o );
const Sound        *outfit_soundOff( const Outfit *o );
double              outfit_ammoMass( const Outfit *o );
int                 outfit_shots( const Outfit *o );
double              outfit_dispersion( const Outfit *o );
double              outfit_speed_dispersion( const Outfit *o );
const char         *outfit_gui( const Outfit *o );
const char         *outfit_licenseProvides( const Outfit *o );
double              outfit_lmapJumpDetect( const Outfit *o );
double              outfit_lmapSpobDetect( const Outfit *o );
int                 outfit_lmapRange( const Outfit *o );
double              outfit_afterburnerMassLimit( const Outfit *o );
double              outfit_afterburnerSpeed( const Outfit *o );
double              outfit_afterburnerAccel( const Outfit *o );
const Sound        *outfit_afterburnerSound( const Outfit *o );
const Sound        *outfit_afterburnerSoundOn( const Outfit *o );
const Sound        *outfit_afterburnerSoundOff( const Outfit *o );
double              outfit_afterburnerRumble( const Outfit *o );
double              outfit_launcherSpeed( const Outfit *o );
double              outfit_launcherSpeedMax( const Outfit *o );
double              outfit_launcherTurn( const Outfit *o );
double              outfit_launcherAccel( const Outfit *o );
double              outfit_launcherResist( const Outfit *o );
OutfitAmmoAI        outfit_launcherAI( const Outfit *o );
double              outfit_launcherArc( const Outfit *o );
double              outfit_launcherDuration( const Outfit *o );
double              outfit_launcherArmour( const Outfit *o );
double              outfit_launcherAbsorb( const Outfit *o );
double              outfit_launcherLockon( const Outfit *o );
double              outfit_launcherIFLockon( const Outfit *o );
const OutfitGFX    *outfit_launcherGFX( const Outfit *o );
const TrailSpec    *outfit_launcherTrailSpec( const Outfit *o );
double              outfit_launcherTrailOffset( const Outfit *o );
const struct Ship  *outfit_bayShip( const Outfit *o );
void                outfit_renderBeam( const Outfit *beam, const Solid *solid,
                                       double range_mod, double dt, double r );
double              outfit_beamMinDelay( const Outfit *o );
double              outfit_beamWarmup( const Outfit *o );
double              outfit_boltSpeed( const Outfit *o );

/* Active outfits. */
double outfit_duration( const Outfit *o );
double outfit_cooldown( const Outfit *o );

/*
 * Loading and freeing outfit stack.
 */
int  outfit_load( void );
int  outfit_loadPost( void );
int  outfit_mapParse( void );
void outfit_free( void );

/*
 * Misc.
 */
int        outfit_fitsSlot( const Outfit *o, const OutfitSlot *s );
int        outfit_fitsSlotType( const Outfit *o, const OutfitSlot *s );
void       outfit_freeSlot( OutfitSlot *s );
glTexture *rarity_texture( int rarity );
int        outfit_checkIllegal( const Outfit *o, int fct );
int       *outfit_illegalTo( const Outfit *o );
int        outfit_licenseExists( const char *name );
char     **outfit_tags( const Outfit *o );
