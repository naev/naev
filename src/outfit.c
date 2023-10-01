/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file outfit.c
 *
 * @brief Handles all the ship outfit specifics.
 *
 * These outfits allow you to modify ships or make them more powerful and are
 *  a fundamental part of the game.
 */
/** @cond */
#include <math.h>
#include <stdlib.h>
#include "SDL_thread.h"
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "outfit.h"

#include "array.h"
#include "conf.h"
#include "damagetype.h"
#include "log.h"
#include "mapData.h"
#include "ndata.h"
#include "nfile.h"
#include "nlua.h"
#include "nlua_gfx.h"
#include "nlua_pilotoutfit.h"
#include "nlua_outfit.h"
#include "nlua_camera.h"
#include "nlua_hook.h"
#include "nlua_munition.h"
#include "nstring.h"
#include "nstring.h"
#include "nxml.h"
#include "pilot.h"
#include "pilot_heat.h"
#include "ship.h"
#include "slots.h"
#include "spfx.h"
#include "start.h"
#include "unistd.h"

#define outfit_setProp(o,p)      ((o)->properties |= p) /**< Checks outfit property. */

#define XML_OUTFIT_TAG     "outfit"    /**< XML section identifier. */

#define OUTFIT_SHORTDESC_MAX  STRMAX_SHORT /**< Max length of the short description of the outfit. */

/*
 * the stack
 */
static Outfit* outfit_stack = NULL; /**< Stack of outfits. */
static char **license_stack = NULL; /**< Stack of available licenses. */

/*
 * Helper stuff for setting up short descriptions for outfits.
 */
#define SDESC_ADD( l, temp, txt, ... ) \
(l) += scnprintf( &(temp)->summary_raw[l], OUTFIT_SHORTDESC_MAX - (l), (txt), ## __VA_ARGS__ )

/*
 * Prototypes
 */
/* misc */
static OutfitType outfit_strToOutfitType( char *buf );
/* parsing */
static int outfit_loadDir( char *dir );
static int outfit_parseDamage( Damage *dmg, xmlNodePtr node );
static int outfit_parse( Outfit* temp, const char* file );
static void outfit_parseSBolt( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSBeam( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSFighterBay( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSMap( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSLocalMap( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSGUI( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSLicense( Outfit *temp, const xmlNodePtr parent );
static int outfit_loadPLG( Outfit *temp, const char *buf );
static int outfit_loadGFX( Outfit *temp, const xmlNodePtr node );
static void sdesc_miningRarity( int *l, Outfit *temp, int rarity );
/* Display */

typedef struct s_Outfitstat {
    const char *name;
    const char *unit;
    int color;
    int color_threshold;
    int hide_zero;
    int precision;
} t_os_stat;
typedef const t_os_stat os_opts;

/* Printing functions. */
static int os_printD( char *buf, int len, double value, const os_opts *opts );
static int os_printD_range( char *buffer, int i, double minValue, double maxValue, const t_os_stat *opts );
static int os_printD_rate( char *buffer, int i, double val, const t_os_stat *val_opts, int multiplier, double rate, const t_os_stat *rate_opts );

/* Helpers for different attributes. */
static os_opts darmour_opts = { N_("Armour Damage"), _UNIT_PERCENT, 1, 100, 0, 0 };
static os_opts dshield_opts = { N_("Shield Damage"), _UNIT_PERCENT, 1, 100, 0, 0 };
static os_opts dknockback_opts = { N_("Knockback"), _UNIT_PERCENT, 0, 0, 1, 0 };
static os_opts cpu_opts = { N_("CPU"), _UNIT_CPU, 1, 0, 1, 0 };
static os_opts mass_opts = { N_("Mass"), _UNIT_MASS, 0, 0, 1, 0 };
static os_opts penetration_opts = { N_("Penetration"), _UNIT_PERCENT, 0, 0, 1, 0 };
static os_opts damage_opts = {N_("Damage"), _UNIT_ENERGY, 0, 1, 1, 1 };
static os_opts dps_opts = {N_("Damage Rate"), _UNIT_POWER, 0, 0, 1, 1 };
static os_opts disable_opts = {N_("Disable"), _UNIT_ENERGY, 0, 1, 1, 1 };
static os_opts disable_rate_opts = {N_("Disable Rate"), _UNIT_POWER, 0, 0, 1, 1 };
static os_opts fire_rate_opts = {N_("Fire Rate"), _UNIT_PER_TIME, 0, 0, 0, 1 };
static os_opts energy_opts = { N_("Energy"), _UNIT_ENERGY, 0, 1, 1, 1 };
static os_opts power_opts = { N_("Power"), _UNIT_POWER, 0, 0, 1, 1 };
static os_opts range_opts = { N_("Range"), _UNIT_DISTANCE, 0, 0, 1, 0 };
static os_opts speed_opts = { N_("Speed"), _UNIT_SPEED, 0, 0, 1, 0 };
static os_opts heatup_opts = { N_("Overheat"), _UNIT_TIME, 0, 0, 1, 1 };
static os_opts dispersion_opts = { N_("Dispersion"), _UNIT_ANGLE, 0, 0, 1, 0 };
static os_opts swivel_opts = { N_("Swivel"), _UNIT_ANGLE, 0, 0, 1, 0 };
static os_opts tracking_opts = { N_("Tracking"), _UNIT_DISTANCE, 0, 0, 1, 0 };
static os_opts duration_opts = { N_("Duration"), _UNIT_TIME, 0, 0, 1, 1 };
static os_opts cooldown_opts = { N_("Cooldown"), _UNIT_TIME, 0, 0, 1, 1 };
static os_opts lockon_opts = { N_("Lock On"), _UNIT_TIME, 0, 0, 1, 0 };
static os_opts inflight_calib_opts = { N_("Inflight Calibration"), _UNIT_TIME, 0, 0, 1, 1 };
static os_opts initial_speed_opts = { N_("Launch Speed"), _UNIT_SPEED, 0, 0, 1, 0 };
static os_opts thrust_opts = { N_("Accel"), _UNIT_ACCEL, 0, 0, 1, 0 };
static os_opts max_speed_opts = { N_("Max Speed"), _UNIT_SPEED, 0, 0, 1, 0 };
static os_opts reload_opts = { N_("Reload Time"), _UNIT_TIME, 0, 0, 1, 1 };
static os_opts armour_opts = { N_("Armour"), _UNIT_ENERGY, 0, 0, 1, 1 };
static os_opts absorp_opts = { N_("Absorption"), _UNIT_PERCENT, 0, 0, 1, 1 };
static os_opts jam_res_opts = { N_("Jam Resistance"), _UNIT_PERCENT, 0, 0, 1, 0 };
static os_opts max_mass_opts = { N_("Max Effective Mass"), _UNIT_MASS, 0, 0, 1, 0 };
static os_opts rumble_opts = { N_("Rumble"), NULL, 0, 0, 1, 1 };
static os_opts shots_delay_opts = { N_("Shots Delay"), _UNIT_TIME, 0, 0, 1, 1 };

static int outfit_cmp( const void *p1, const void *p2 )
{
   const Outfit *o1, *o2;
   o1 = (const Outfit*) p1;
   o2 = (const Outfit*) p2;
   return strcmp( o1->name, o2->name );
}

/**
 * @brief Gets an outfit by name.
 *
 *    @param name Name to match.
 *    @return Outfit matching name or NULL if not found.
 */
const Outfit* outfit_get( const char* name )
{
   const Outfit *o = outfit_getW( name );
   if (o==NULL)
      WARN(_("Outfit '%s' not found in stack."), name);
   return o;
}

/**
 * @brief Gets an outfit by name without warning on no-find.
 *
 *    @param name Name to match.
 *    @return Outfit matching name or NULL if not found.
 */
const Outfit* outfit_getW( const char* name )
{
   const Outfit s = {.name = (char*)name };
   return bsearch( &s, outfit_stack, array_size(outfit_stack), sizeof(Outfit), outfit_cmp );
}

/**
 * @brief Gets the array (array.h) of all outfits.
 */
const Outfit* outfit_getAll (void)
{
   return outfit_stack;
}

/**
 * @brief Checks to see if an outfit exists matching name (case insensitive).
 */
const char *outfit_existsCase( const char* name )
{
   for (int i=0; i<array_size(outfit_stack); i++)
      if (strcasecmp(name,outfit_stack[i].name)==0)
         return outfit_stack[i].name;
   return NULL;
}

/**
 * @brief Does a fuzzy search of all the outfits. Searches translated names but returns internal names.
 */
char **outfit_searchFuzzyCase( const char* name, int *n )
{
   int len, nstack;
   char **names;

   /* Overallocate to maximum. */
   nstack = array_size(outfit_stack);
   names = malloc( sizeof(char*) * nstack );

   /* Do fuzzy search. */
   len = 0;
   for (int i=0; i<nstack; i++) {
      if (SDL_strcasestr( _(outfit_stack[i].name), name ) != NULL) {
         names[len] = outfit_stack[i].name;
         len++;
      }
   }

   /* Free if empty. */
   if (len == 0) {
      free(names);
      names = NULL;
   }

   *n = len;
   return names;
}

/**
 * @brief Function meant for use with C89, C99 algorithm qsort().
 *
 *    @param outfit1 First argument to compare.
 *    @param outfit2 Second argument to compare.
 *    @return -1 if first argument is inferior, +1 if it's superior, 0 if ties.
 */
int outfit_compareTech( const void *outfit1, const void *outfit2 )
{
   int ret;
   const Outfit *o1, *o2;

   /* Get outfits. */
   o1 = * (const Outfit**) outfit1;
   o2 = * (const Outfit**) outfit2;

   /* Compare slot type. */
   if (o1->slot.type < o2->slot.type)
      return +1;
   else if (o1->slot.type > o2->slot.type)
      return -1;

   /* Compare intrinsic types. */
   if (o1->type < o2->type)
      return -1;
   else if (o1->type > o2->type)
      return +1;

   /* Compare named types. */
   if ((o1->typename == NULL) && (o2->typename != NULL))
      return -1;
   else if ((o1->typename != NULL) && (o2->typename == NULL))
      return +1;
   else if ((o1->typename != NULL) && (o2->typename != NULL)) {
      ret = strcmp( o1->typename, o2->typename );
      if (ret != 0)
         return ret;
   }

   /* Compare slot sizes. */
   if (o1->slot.size < o2->slot.size)
      return +1;
   else if (o1->slot.size > o2->slot.size)
      return -1;

   /* Compare sort priority. */
   if (o1->priority < o2->priority)
      return +1;
   else if (o1->priority > o2->priority)
      return -1;

   /* Special prices are listed first. */
   if ((o1->lua_price != LUA_NOREF) && (o2->lua_price == LUA_NOREF))
      return -1;
   else if ((o1->lua_price == LUA_NOREF) && (o2->lua_price != LUA_NOREF))
      return +1;

   /* Compare price. */
   if (o1->price < o2->price)
      return +1;
   else if (o1->price > o2->price)
      return -1;

   /* It turns out they're the same. */
   return strcmp( o1->name, o2->name );
}

int outfit_filterWeapon( const Outfit *o )
{ return ((o->slot.type == OUTFIT_SLOT_WEAPON) && !sp_required( o->slot.spid )); }

int outfit_filterUtility( const Outfit *o )
{ return ((o->slot.type == OUTFIT_SLOT_UTILITY) && !sp_required( o->slot.spid )); }

int outfit_filterStructure( const Outfit *o )
{ return ((o->slot.type == OUTFIT_SLOT_STRUCTURE) && !sp_required( o->slot.spid )); }

int outfit_filterCore( const Outfit *o )
{ return sp_required( o->slot.spid ); }

int outfit_filterOther( const Outfit *o )
{
   return (!sp_required( o->slot.spid ) && ((o->slot.type == OUTFIT_SLOT_NULL)
         || (o->slot.type == OUTFIT_SLOT_NA)));
}

/**
 * @brief Gets the name of the slot type of an outfit.
 *
 *    @param o Outfit to get slot type of.
 *    @return The human readable name of the slot type.
 */
const char *outfit_slotName( const Outfit* o )
{
   return slotName( o->slot.type );
}

/**
 * @brief \see outfit_slotName
 */
const char *slotName( const OutfitSlotType type )
{
   switch (type) {
      case OUTFIT_SLOT_NULL:
         return "NULL";
      case OUTFIT_SLOT_NA:
         return gettext_noop("N/A");
      case OUTFIT_SLOT_STRUCTURE:
         return gettext_noop("Structure");
      case OUTFIT_SLOT_UTILITY:
         return gettext_noop("Utility");
      case OUTFIT_SLOT_WEAPON:
         return gettext_noop("Weapon");
      default:
         return gettext_noop("Unknown");
   }
}

/**
 * @brief Gets the slot size as a string.
 */
const char *slotSize( const OutfitSlotSize o )
{
   switch (o) {
      case OUTFIT_SLOT_SIZE_NA:
         return gettext_noop("N/A");
      case OUTFIT_SLOT_SIZE_LIGHT:
         return gettext_noop("Small");
      case OUTFIT_SLOT_SIZE_MEDIUM:
         return gettext_noop("Medium");
      case OUTFIT_SLOT_SIZE_HEAVY:
         return gettext_noop("Large");
      default:
         return gettext_noop("Unknown");
   }
}

/**
 * @brief Gets the name of the slot size of an outfit.
 *
 *    @param o Outfit to get slot size of.
 *    @return The human readable name of the slot size.
 */
const char *outfit_slotSize( const Outfit* o )
{
   return slotSize( o->slot.size );
}

/**
 * @brief Gets the slot size colour for an outfit slot.
 *
 *    @param os Outfit slot to get the slot size colour of.
 *    @return The slot size colour of the outfit slot.
 */
const glColour *outfit_slotSizeColour( const OutfitSlot* os )
{
   if (os->size == OUTFIT_SLOT_SIZE_HEAVY)
      return &cOutfitHeavy;
   else if (os->size == OUTFIT_SLOT_SIZE_MEDIUM)
      return &cOutfitMedium;
   else if (os->size == OUTFIT_SLOT_SIZE_LIGHT)
      return &cOutfitLight;
   return NULL;
}

/**
 * @brief Gets a font colour character that roughly matches an outfit slot size colour.
 *
 *    @param os Outfit slot to get the slot size font colour of.
 *    @return The slot size font colour of the outfit slot.
 */
char outfit_slotSizeColourFont( const OutfitSlot* os )
{
   if (os->size == OUTFIT_SLOT_SIZE_HEAVY)
      return 'r';
   else if (os->size == OUTFIT_SLOT_SIZE_MEDIUM)
      return 'b';
   else if (os->size == OUTFIT_SLOT_SIZE_LIGHT)
      return 'y';
   return '0';
}

/**
 * @brief Gets a font colour character that roughly matches an outfit slot type colour.
 *
 *    @param os Outfit slot to get the slot type font colour of.
 *    @return The slot type font colour of the outfit slot.
 */
char outfit_slotTypeColourFont( const OutfitSlot* os )
{
   if (os->type == OUTFIT_SLOT_WEAPON)
      return 'p';
   else if (os->type == OUTFIT_SLOT_UTILITY)
      return 'g';
   else if (os->type == OUTFIT_SLOT_STRUCTURE)
      return 'n';
   return '0';
}

/**
 * @brief Gets a brief name/class description suitable for the title section of an outfitter screen.
 *
 *    @param outfit Outfit to describe.
 *    @param[out] buf Output buffer.
 *    @param size Size of output buffer.
 *    @return Number of characters written.
 */
size_t outfit_getNameWithClass( const Outfit* outfit, char* buf, size_t size )
{
   size_t p = scnprintf( &buf[0], size, "%s", _(outfit->name) );
   if (outfit->slot.type != OUTFIT_SLOT_NA) {
      p += scnprintf( &buf[p], size-p, _("\n#%c%s #%c%s #0slot"),
            outfit_slotSizeColourFont( &outfit->slot ), _(outfit_slotSize( outfit )),
            outfit_slotTypeColourFont( &outfit->slot ), _(outfit_slotName( outfit )) );
   }
   return p;
}

/**
 * @brief Gets the outfit slot size from a human readable string.
 *
 *    @param s String representing an outfit slot size.
 *    @return Outfit slot size matching string.
 */
OutfitSlotSize outfit_toSlotSize( const char *s )
{
   if (s == NULL) {
      /*WARN( "(NULL) outfit slot size" );*/
      return OUTFIT_SLOT_SIZE_NA;
   }

   if (strcasecmp(s,"Large")==0)
      return OUTFIT_SLOT_SIZE_HEAVY;
   else if (strcasecmp(s,"Medium")==0)
      return OUTFIT_SLOT_SIZE_MEDIUM;
   else if (strcasecmp(s,"Small")==0)
      return OUTFIT_SLOT_SIZE_LIGHT;

   WARN(_("'%s' does not match any outfit slot sizes."), s);
   return OUTFIT_SLOT_SIZE_NA;
}

/**
 * @brief Checks if outfit is an active outfit.
 *    @param o Outfit to check.
 *    @return 1 if o is active.
 */
int outfit_isActive( const Outfit* o )
{
   if (outfit_isForward(o) || outfit_isTurret(o) || outfit_isLauncher(o) || outfit_isFighterBay(o))
      return 1;
   if (outfit_isMod(o) && o->u.mod.active)
      return 1;
   if (outfit_isAfterburner(o))
      return 1;
   return 0;
}

/**
 * @brief Checks if outfit can be toggled.
 *    @param o Outfit to check.
 *    @return 1 if o is active.
 */
int outfit_isToggleable( const Outfit *o )
{
   /* Must be active. */
   if (!outfit_isActive(o))
      return 0;

   /* Special case it is lua-based and not toggleable. */
   if (outfit_isMod(o) && (o->lua_env != LUA_NOREF) && (o->lua_ontoggle == LUA_NOREF))
      return 0;

   return 1;
}

/**
 * @brief Checks if outfit is a fixed mounted weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a weapon (beam/bolt).
 */
int outfit_isForward( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BOLT) ||
         (o->type==OUTFIT_TYPE_BEAM) );
}
/**
 * @brief Checks if outfit is bolt type weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a bolt type weapon.
 */
int outfit_isBolt( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BOLT) ||
         (o->type==OUTFIT_TYPE_TURRET_BOLT) );
}
/**
 * @brief Checks if outfit is a beam type weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a beam type weapon.
 */
int outfit_isBeam( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BEAM) ||
         (o->type==OUTFIT_TYPE_TURRET_BEAM) );
}
/**
 * @brief Checks if outfit is a weapon launcher.
 *    @param o Outfit to check.
 *    @return 1 if o is a weapon launcher.
 */
int outfit_isLauncher( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_LAUNCHER) ||
         (o->type==OUTFIT_TYPE_TURRET_LAUNCHER) );
}
/**
 * @brief Checks if outfit is a seeking weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a seeking weapon.
 */
int outfit_isSeeker( const Outfit* o )
{
   if (((o->type==OUTFIT_TYPE_TURRET_LAUNCHER) ||
            (o->type==OUTFIT_TYPE_LAUNCHER)) &&
         (o->u.lau.ai > 0))
      return 1;
   return 0;
}
/**
 * @brief Checks if outfit is a turret class weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a turret class weapon.
 */
int outfit_isTurret( const Outfit* o )
{
   return ((o->type==OUTFIT_TYPE_TURRET_BOLT)   ||
         (o->type==OUTFIT_TYPE_TURRET_BEAM)     ||
         (o->type==OUTFIT_TYPE_TURRET_LAUNCHER));
}
/**
 * @brief Checks if outfit is a ship modification.
 *    @param o Outfit to check.
 *    @return 1 if o is a ship modification.
 */
int outfit_isMod( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_MODIFICATION);
}
/**
 * @brief Checks if outfit is an afterburner.
 *    @param o Outfit to check.
 *    @return 1 if o is an afterburner.
 */
int outfit_isAfterburner( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_AFTERBURNER);
}
/**
 * @brief Checks if outfit is a fighter bay.
 *    @param o Outfit to check.
 *    @return 1 if o is a jammer.
 */
int outfit_isFighterBay( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_FIGHTER_BAY);
}
/**
 * @brief Checks if outfit is a space map.
 *    @param o Outfit to check.
 *    @return 1 if o is a map.
 */
int outfit_isMap( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_MAP);
}
/**
 * @brief Checks if outfit is a local space map.
 *    @param o Outfit to check.
 *    @return 1 if o is a map.
 */
int outfit_isLocalMap( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_LOCALMAP);
}
/**
 * @brief Checks if outfit is a license.
 *    @param o Outfit to check.
 *    @return 1 if o is a license.
 */
int outfit_isLicense( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_LICENSE);
}
/**
 * @brief Checks if outfit is a GUI.
 *    @param o Outfit to check.
 *    @return 1 if o is a GUI.
 */
int outfit_isGUI( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_GUI);
}

/**
 * @brief Checks if outfit has the secondary flag set.
 *    @param o Outfit to check.
 *    @return 1 if o is a secondary weapon.
 */
int outfit_isSecondary( const Outfit* o )
{
   return (o->properties & OUTFIT_PROP_WEAP_SECONDARY) != 0;
}

/**
 * @brief Gets the outfit's graphic effect.
 *    @param o Outfit to get information from.
 */
const OutfitGFX* outfit_gfx( const Outfit* o )
{
   if (outfit_isBolt(o)) return &o->u.blt.gfx;
   else if (outfit_isLauncher(o)) return &o->u.lau.gfx;
   return NULL;
}
/**
 * @brief Gets the outfit's collision polygon.
 *    @param o Outfit to get information from.
 */
const CollPoly* outfit_plg( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.gfx.polygon;
   else if (outfit_isLauncher(o)) return o->u.lau.gfx.polygon;
   return NULL;
}
/**
 * @brief Gets the outfit's sound effect.
 *    @param o Outfit to get information from.
 */
int outfit_spfxArmour( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.spfx_armour;
   else if (outfit_isBeam(o)) return o->u.bem.spfx_armour;
   else if (outfit_isLauncher(o)) return o->u.lau.spfx_armour;
   return -1;
}
/**
 * @brief Gets the outfit's sound effect.
 *    @param o Outfit to get information from.
 */
int outfit_spfxShield( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.spfx_shield;
   else if (outfit_isBeam(o)) return o->u.bem.spfx_shield;
   else if (outfit_isLauncher(o)) return o->u.lau.spfx_shield;
   return -1;
}
/**
 * @brief Gets the outfit's damage.
 *    @param o Outfit to get information from.
 */
const Damage *outfit_damage( const Outfit* o )
{
   if (outfit_isBolt(o)) return &o->u.blt.dmg;
   else if (outfit_isBeam(o)) return &o->u.bem.dmg;
   else if (outfit_isLauncher(o)) return &o->u.lau.dmg;
   return NULL;
}
/**
 * @brief Gets the outfit's explosion radius.
 *    @param o Outfit to get information from.
 */
double outfit_radius( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.radius;
   else if (outfit_isLauncher(o)) return o->u.lau.radius;
   return 0.;
}
/**
 * @brief Gets the outfit's delay.
 *    @param o Outfit to get information from.
 */
double outfit_delay( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.delay;
   else if (outfit_isBeam(o)) return o->u.bem.delay;
   else if (outfit_isLauncher(o)) return o->u.lau.delay;
   else if (outfit_isFighterBay(o)) return o->u.bay.delay;
   return -1;
}
/**
 * @brief Gets the amount an outfit can hold.
 *    @param o Outfit to get information from.
 */
int outfit_amount( const Outfit* o )
{
   if (outfit_isLauncher(o)) return o->u.lau.amount;
   else if (outfit_isFighterBay(o)) return o->u.bay.amount;
   return -1;
}

/**
 * @brief Gets the outfit's energy usage.
 *    @param o Outfit to get information from.
 */
double outfit_energy( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.energy;
   else if (outfit_isBeam(o)) return o->u.bem.energy;
   else if (outfit_isLauncher(o)) return o->u.lau.energy;
   return -1.;
}
/**
 * @brief Gets the outfit's heat generation.
 *    @param o Outfit to get information from.
 */
double outfit_heat( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.heat;
   else if (outfit_isAfterburner(o)) return o->u.afb.heat;
   else if (outfit_isBeam(o)) return o->u.bem.heat;
   return -1;
}
/**
 * @brief Gets the outfit's cpu usage.
 *    @param o Outfit to get information from.
 */
double outfit_cpu( const Outfit* o )
{
   return o->cpu;
}
/**
 * @brief Gets the outfit's range.
 *    @param o Outfit to get information from.
 */
double outfit_range( const Outfit* o )
{
   if (outfit_isBolt(o))
      return o->u.blt.falloff + (o->u.blt.range - o->u.blt.falloff)/2.;
   else if (outfit_isBeam(o))
      return o->u.bem.range;
   else if (outfit_isLauncher(o)) {
      if (o->u.lau.thrust) {
         double speedinc;
         if (o->u.lau.speed > 0.) /* Ammo that don't start stopped don't have max speed. */
            speedinc = INFINITY;
         else
            speedinc = o->u.lau.speed_max - o->u.lau.speed;
         double at = speedinc / o->u.lau.thrust;
         if (at < o->u.lau.duration)
            return speedinc * (o->u.lau.duration - at / 2.) + o->u.lau.speed * o->u.lau.duration;

         /* Maximum speed will never be reached. */
         return pow2(o->u.lau.duration) * o->u.lau.thrust / 2. + o->u.lau.duration * o->u.lau.speed;
      }
      return o->u.lau.speed * o->u.lau.duration;
   }
   else if (outfit_isFighterBay(o))
      return INFINITY;
   return -1.;
}
/**
 * @brief Gets the outfit's speed.
 *    @param o Outfit to get information from.
 *    @return Outfit's speed.
 */
double outfit_speed( const Outfit* o )
{
   if (outfit_isBolt(o))
      return o->u.blt.speed;
   else if (outfit_isLauncher(o)) {
      if (o->u.lau.thrust == 0.)
         return o->u.lau.speed;
      else { /* Gets the average speed. */
         double t;
         if (o->u.lau.speed > 0.) /* Ammo that don't start stopped don't have max speed. */
            t = INFINITY;
         else
            t = (o->u.lau.speed_max - o->u.lau.speed) / o->u.lau.thrust; /* Time to reach max speed */

         /* Reaches max speed. */
         if (t < o->u.lau.duration)
            return (o->u.lau.thrust * t * t / 2. + (o->u.lau.speed_max - o->u.lau.speed) * (o->u.lau.duration - t)) / o->u.lau.duration + o->u.lau.speed;
         /* Doesn't reach max speed. */
         else
            return o->u.lau.thrust * o->u.lau.duration / 2. + o->u.lau.speed;
      }
   }
   return -1.;
}
/**
 * @brief Gets the outfit's animation spin.
 *    @param o Outfit to get information from.
 *    @return Outfit's animation spin.
 */
double outfit_spin( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.gfx.spin;
   else if (outfit_isLauncher(o)) return o->u.lau.gfx.spin;
   return -1.;
}
/**
 * @brief Gets the outfit's minimal tracking.
 *    @param o Outfit to get information from.
 *    @return Outfit's minimal tracking.
 */
double outfit_trackmin( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.trackmin;
   else if (outfit_isLauncher(o)) return o->u.lau.trackmin;
   else if (outfit_isBeam(o)) return 0.;
   return -1.;
}
/**
 * @brief Gets the outfit's minimal tracking.
 *    @param o Outfit to get information from.
 *    @return Outfit's minimal tracking.
 */
double outfit_trackmax( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.trackmax;
   else if (outfit_isLauncher(o)) return o->u.lau.trackmax;
   else if (outfit_isBeam(o)) return 1.;
   return -1.;
}
/**
 * @brief Gets the maximum rarity the outfit can mine up to.
 *    @param o Outfit to get information from.
 *    @return Outfit's maximum mining rarity.
 */
int outfit_miningRarity( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.mining_rarity;
   else if (outfit_isLauncher(o)) return o->u.lau.mining_rarity;
   else if (outfit_isBeam(o)) return o->u.bem.mining_rarity;
   return -1;
}
/**
 * @brief Gets the outfit's sound.
 *    @param o Outfit to get sound from.
 *    @return Outfit's sound.
 */
int outfit_sound( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.sound;
   else if (outfit_isLauncher(o)) return o->u.lau.sound;
   return -1.;
}
/**
 * @brief Gets the outfit's hit sound.
 *    @param o Outfit to get hit sound from.
 *    @return Outfit's hit sound.
 */
int outfit_soundHit( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.sound_hit;
   else if (outfit_isLauncher(o)) return o->u.lau.sound_hit;
   return -1.;
}
/**
 * @brief Gets the outfit's ammunition mass.
 *    @param o Outfit to get ammunition mass from.
 *    @return Outfit's ammunition's mass.
 */
double outfit_ammoMass( const Outfit *o )
{
   if (outfit_isLauncher(o)) return o->u.lau.ammo_mass;
   else if (outfit_isFighterBay(o)) return o->u.bay.ship_mass;
   return -1.;
}
/**
 * @brief Gets the outfit's duration.
 *    @param o Outfit to get the duration of.
 *    @return Outfit's duration.
 */
double outfit_duration( const Outfit* o )
{
   if (outfit_isMod(o)) { if (o->u.mod.active) return o->u.mod.duration; }
   else if (outfit_isAfterburner(o)) return INFINITY;
   else if (outfit_isBolt(o)) return (o->u.blt.range / o->u.blt.speed);
   else if (outfit_isBeam(o)) return o->u.bem.duration;
   else if (outfit_isLauncher(o)) return o->u.lau.duration;
   else if (outfit_isFighterBay(o)) return INFINITY;
   return -1.;
}
/**
 * @brief Gets the outfit's cooldown.
 *    @param o Outfit to get the cooldown of.
 *    @return Outfit's cooldown.
 */
double outfit_cooldown( const Outfit* o )
{
   if (outfit_isMod(o)) { if (o->u.mod.active) return o->u.mod.cooldown; }
   else if (outfit_isAfterburner(o)) return 0.;
   return -1.;
}

/**
 * @brief Gets the outfit's specific type.
 *
 *    @param o Outfit to get specific type from.
 *    @return The specific type in human readable form (English).
 */
const char* outfit_getType( const Outfit* o )
{
   const char* outfit_typename[] = {
      N_("NULL"),
      N_("Bolt Cannon"),
      N_("Beam Cannon"),
      N_("Bolt Turret"),
      N_("Beam Turret"),
      N_("Launcher"),
      N_("Turret Launcher"),
      N_("Ship Modification"),
      N_("Afterburner"),
      N_("Fighter Bay"),
      N_("Star Map"),
      N_("Local Map"),
      N_("GUI"),
      N_("License"),
   };

   /* Name override. */
   if (o->typename != NULL)
      return o->typename;
   return outfit_typename[o->type];
}

/**
 * @brief Gets the outfit's broad type.
 *
 *    @param o Outfit to get the type of.
 *    @return The outfit's broad type in human readable form.
 */
const char* outfit_getTypeBroad( const Outfit* o )
{
   if (outfit_isBolt(o))            return N_("Bolt Weapon");
   else if (outfit_isBeam(o))       return N_("Beam Weapon");
   else if (outfit_isLauncher(o))   return N_("Launcher");
   //else if (outfit_isTurret(o))     return N_("Turret");
   else if (outfit_isMod(o))        return N_("Modification");
   else if (outfit_isAfterburner(o)) return N_("Afterburner");
   else if (outfit_isFighterBay(o)) return N_("Fighter Bay");
   else if (outfit_isMap(o))        return N_("Map");
   else if (outfit_isLocalMap(o))   return N_("Local Map");
   else if (outfit_isGUI(o))        return N_("GUI");
   else if (outfit_isLicense(o))    return N_("License");
   else                             return N_("Unknown");
}

/**
 * @brief Gets a human-readable string describing an ammo outfit's AI.
 *    @param o Ammo outfit.
 *    @return Name of the outfit's AI.
 */
const char* outfit_getAmmoAI( const Outfit *o )
{
   const char *ai_type[] = {
      N_("Unguided"),
      N_("Seek"),
      N_("Smart")
   };

   if (!outfit_isLauncher(o)) {
      WARN(_("Outfit '%s' is not a launcher outfit"), o->name);
      return NULL;
   }

   return ai_type[o->u.lau.ai];
}

/**
 * @brief Gets the description of an outfit.
 *
 * Subsequent calls will change the memory, strdup if necessary.
 *
 *    @param o Outfit to get description of.
 *    @return Description of the outfit.
 */
const char* outfit_description( const Outfit *o )
{
   return pilot_outfitDescription( NULL, o );
}

/**
 * @brief Gets the summary of an outfit.
 *
 * Subsequent calls will change the memory, strdup if necessary.
 *
 *    @param o Outfit to get summary of.
 *    @param withname Whether or not to include the outfit name.
 *    @return Summary of the outfit.
 */
const char* outfit_summary( const Outfit *o, int withname )
{
   return pilot_outfitSummary( NULL, o, withname );
}

/**
 * @brief Checks to see if an outfit fits a slot.
 *
 *    @param o Outfit to see if fits in a slot.
 *    @param s Slot to see if outfit fits in.
 *    @return 1 if outfit fits the slot, 0 otherwise.
 */
int outfit_fitsSlot( const Outfit* o, const OutfitSlot* s )
{
   const OutfitSlot *os = &o->slot;

   /* Outfit must have valid slot type. */
   if ((os->type == OUTFIT_SLOT_NULL) ||
      (os->type == OUTFIT_SLOT_NA))
      return 0;

   /* Outfit type must match outfit slot. */
   if (os->type != s->type)
      return 0;

   /* It doesn't fit. */
   if (os->size > s->size)
      return 0;

   /* Must match slot property. */
   if (os->spid != 0)
      if (s->spid != os->spid)
         return 0;

   /* Exclusive only match property. */
   if (s->exclusive)
      if (s->spid != os->spid)
         return 0;

   /* Must have valid slot size. */
   /*
   if (os->size == OUTFIT_SLOT_SIZE_NA)
      return 0;
   */

   /* It meets all criteria. */
   return 1;
}

/**
 * @brief Checks to see if an outfit fits a slot type (ignoring size).
 *
 *    @param o Outfit to see if fits in a slot.
 *    @param s Slot to see if outfit fits in.
 *    @return 1 if outfit fits the slot, 0 otherwise.
 */
int outfit_fitsSlotType( const Outfit* o, const OutfitSlot* s )
{
   const OutfitSlot *os = &o->slot;

   /* Outfit must have valid slot type. */
   if ((os->type == OUTFIT_SLOT_NULL) ||
      (os->type == OUTFIT_SLOT_NA))
      return 0;

   /* Outfit type must match outfit slot. */
   if (os->type != s->type)
      return 0;

   /* It meets all criteria. */
   return 1;
}

/**
 * @brief Frees an outfit slot.
 *
 *    @param s Slot to free.
 */
void outfit_freeSlot( OutfitSlot* s )
{
   (void) s;
}

#define O_CMP(s,t) \
if (strcasecmp(buf,(s))==0) return t /**< Define to help with outfit_strToOutfitType. */
/**
 * @brief Gets the outfit type from a human readable string.
 *
 *    @param buf String to extract outfit type from.
 *    @return Outfit type stored in buf.
 */
static OutfitType outfit_strToOutfitType( char *buf )
{
   O_CMP("bolt",           OUTFIT_TYPE_BOLT);
   O_CMP("beam",           OUTFIT_TYPE_BEAM);
   O_CMP("turret bolt",    OUTFIT_TYPE_TURRET_BOLT);
   O_CMP("turret beam",    OUTFIT_TYPE_TURRET_BEAM);
   O_CMP("launcher",       OUTFIT_TYPE_LAUNCHER);
   O_CMP("turret launcher",OUTFIT_TYPE_TURRET_LAUNCHER);
   O_CMP("modification",   OUTFIT_TYPE_MODIFICATION);
   O_CMP("afterburner",    OUTFIT_TYPE_AFTERBURNER);
   O_CMP("fighter bay",    OUTFIT_TYPE_FIGHTER_BAY);
   O_CMP("map",            OUTFIT_TYPE_MAP);
   O_CMP("localmap",       OUTFIT_TYPE_LOCALMAP);
   O_CMP("license",        OUTFIT_TYPE_LICENSE);
   O_CMP("gui",            OUTFIT_TYPE_GUI);

   WARN(_("Invalid outfit type: '%s'"),buf);
   return  OUTFIT_TYPE_NULL;
}
#undef O_CMP

/**
 * @brief Adds a small blurb about rarity mining.
 */
static void sdesc_miningRarity( int *l, Outfit *temp, int rarity )
{
   if (rarity == 0)
      return;
   if (rarity == 2)
      SDESC_ADD( *l, temp, "\n#g%s#0", _("Can mine uncommon and rare minerals") );
   else
      SDESC_ADD( *l, temp, "\n#g%s#0", _("Can mine uncommon minerals") );
}

/**
 * @brief Parses a damage node.
 *
 * Example damage node would be:
 * @code
 * <damage type="kinetic">10</damage>
 * @endcode
 *
 *    @param[out] dmg Stores the damage here.
 *    @param[in] node Node to parse damage from.
 *    @return 0 on success.
 */
static int outfit_parseDamage( Damage *dmg, xmlNodePtr node )
{
   xmlNodePtr cur;

   /* Defaults. */
   dmg->type         = dtype_get( start_dtype_default() );
   dmg->damage       = 0.;
   dmg->penetration  = 0.;
   dmg->disable      = 0.;

   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      /* Core properties. */
      xmlr_float( cur, "penetrate", dmg->penetration );
      xmlr_float( cur, "physical",  dmg->damage );
      xmlr_float( cur, "disable",   dmg->disable );

      /* Get type */
      if (xml_isNode(cur,"type")) {
         char *buf   = xml_get( cur );
         dmg->type   = dtype_get(buf);
         if (dmg->type < 0) { /* Case damage type in outfit.xml that isn't in damagetype.xml */
            dmg->type = 0;
            WARN(_("Unknown damage type '%s'"), buf);
         }
      }
      else WARN(_("Damage has unknown node '%s'"), cur->name);

   } while (xml_nextNode(cur));

   /* Normalize. */
   dmg->penetration /= 100.;

   return 0;
}

/**
 * @brief Loads the collision polygon for a bolt outfit.
 *
 *    @param temp Outfit to load into.
 *    @param buf Name of the file.
 */
static int outfit_loadPLG( Outfit *temp, const char *buf )
{
   char *file;
   OutfitGFX *gfx;
   CollPoly *polygon;
   xmlDocPtr doc;
   xmlNodePtr node, cur;

   if (outfit_isLauncher(temp))
      gfx = &temp->u.lau.gfx;
   else if (outfit_isBolt(temp))
      gfx = &temp->u.blt.gfx;
   else {
      WARN(_("Trying to load polygon for non-compatible outfit '%s'!"),temp->name);
      return -1;
   }

   SDL_asprintf( &file, "%s%s.xml", OUTFIT_POLYGON_PATH, buf );

   /* See if the file does exist. */
   if (!PHYSFS_exists(file)) {
      WARN(_("%s xml collision polygon does not exist!\n \
               Please use the script 'polygon_from_sprite.py' \
that can be found in Naev's artwork repo."), file);
      free(file);
      return 0;
   }

   /* Load the XML. */
   doc  = xml_parsePhysFS( file );

   if (doc == NULL) {
      free(file);
      return 0;
   }

   node = doc->xmlChildrenNode; /* First polygon node */
   if (node == NULL) {
      xmlFreeDoc(doc);
      WARN(_("Malformed %s file: does not contain elements"), file);
      free(file);
      return 0;
   }

   free(file);

   do { /* load the polygon data */
      if (xml_isNode(node,"polygons")) {
         cur = node->children;
         gfx->polygon = array_create_size( CollPoly, 36 );
         do {
            if (xml_isNode(cur,"polygon")) {
               polygon = &array_grow( &gfx->polygon );
               LoadPolygon( polygon, cur );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   return 0;
}

/**
 * @brief Loads the graphics for an outfit.
 */
static int outfit_loadGFX( Outfit *temp, const xmlNodePtr node )
{
   char *type;
   OutfitGFX *gfx;

   if (outfit_isLauncher(temp))
      gfx = &temp->u.lau.gfx;
   else if (outfit_isBolt(temp))
      gfx = &temp->u.blt.gfx;
   else {
      WARN(_("Trying to load graphics for non-compatible outfit '%s'!"),temp->name);
      return -1;
   }

   /* Comomn properties. */
   xmlr_attr_float(node, "spin", gfx->spin);
   if (gfx->spin != 0.)
      outfit_setProp( temp, OUTFIT_PROP_WEAP_SPIN );

   /* Split by type. */
   xmlr_attr_strd(node,"type",type);
   if ((type != NULL) && (strcmp(type,"shader")==0)) {
      char *vertex;

      xmlr_attr_strd(node,"vertex",vertex);
      if (vertex == NULL)
         vertex = strdup("project_pos.vert");
      gfx->program   = gl_program_vert_frag( vertex, xml_get(node), NULL );
      free( vertex );
      gfx->vertex    = glGetAttribLocation(  gfx->program, "vertex" );
      gfx->projection= glGetUniformLocation( gfx->program, "projection" );
      gfx->dimensions= glGetUniformLocation( gfx->program, "dimensions" );
      gfx->u_r       = glGetUniformLocation( gfx->program, "u_r" );
      gfx->u_time    = glGetUniformLocation( gfx->program, "u_time" );
      gfx->u_fade    = glGetUniformLocation( gfx->program, "u_fade" );

      xmlr_attr_float_def(node,"size",gfx->size,-1.);
      if (gfx->size < 0.)
         WARN(_("Outfit '%s' has GFX shader but no 'size' set!"),temp->name);

      xmlr_attr_float_def(node,"col_size",gfx->col_size,gfx->size*0.8);

      free(type);
      return 0;
   }
   else if (type != NULL) {
      WARN(_("Outfit '%s' has unknown gfx type '%s'!"), temp->name, type);
      free(type);
      return -1;
   }

   /* Load normal graphics. */
   gfx->tex = xml_parseTexture( node, OUTFIT_GFX_PATH"space/%s", 6, 6,
         OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS );
   /* Load the collision polygon. */
   char *buf = xml_get(node);
   outfit_loadPLG( temp, buf );
   gfx->size = (gfx->tex->sw + gfx->tex->sh)*0.5;

   /* Validity check: there must be 1 polygon per sprite. */
   if (array_size(gfx->polygon) != 36) {
      WARN(_("Outfit '%s': the number of collision polygons is wrong.\n \
               npolygon = %i and sx*sy = %i"),
               temp->name, array_size(gfx->polygon), 36);
   }

   return 0;
}

/**
 * @brief Parses the specific area for a bolt weapon and loads it into Outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSBolt( Outfit* temp, const xmlNodePtr parent )
{
   ShipStatList *ll;
   xmlNodePtr node;
   double C, area;
   double dshield, darmour, dknockback;
   int l;

   /* Defaults */
   temp->u.blt.spfx_armour    = -1;
   temp->u.blt.spfx_shield    = -1;
   temp->u.blt.sound          = -1;
   temp->u.blt.sound_hit      = -1;
   temp->u.blt.falloff        = -1.;
   temp->u.blt.trackmin       = -1.;
   temp->u.blt.trackmax       = -1.;
   temp->u.blt.shots          = 1;

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      xmlr_float(node,"speed",temp->u.blt.speed);
      xmlr_float(node,"delay",temp->u.blt.delay);
      xmlr_float(node,"energy",temp->u.blt.energy);
      xmlr_float(node,"heatup",temp->u.blt.heatup);
      xmlr_float(node,"trackmin",temp->u.blt.trackmin);
      xmlr_float(node,"trackmax",temp->u.blt.trackmax);
      xmlr_float(node,"swivel",temp->u.blt.swivel);
      xmlr_float(node,"dispersion",temp->u.blt.dispersion);
      xmlr_float(node,"speed_dispersion",temp->u.blt.speed_dispersion);
      xmlr_int(node,"shots",temp->u.blt.shots);
      xmlr_int(node,"mining_rarity",temp->u.blt.mining_rarity);
      xmlr_strd(node,"lua",temp->lua_file);
      if (xml_isNode(node,"radius")) {
         char *buf;
         temp->u.blt.radius = xml_getFloat(node);
         xmlr_attr_strd(node,"friendlyfire",buf);
         if (buf != NULL) {
            outfit_setProp(temp, OUTFIT_PROP_WEAP_FRIENDLYFIRE);
            free(buf);
         }
         continue;
      }
      if (xml_isNode(node,"pointdefense")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_POINTDEFENSE);
         continue;
      }
      if (xml_isNode(node,"miss_ships")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_SHIPS);
         continue;
      }
      if (xml_isNode(node,"miss_asteroids")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_ASTEROIDS);
         continue;
      }
      if (xml_isNode(node,"miss_explode")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_EXPLODE);
         continue;
      }
      if (xml_isNode(node,"range")) {
         char *buf;
         xmlr_attr_strd(node,"blowup",buf);
         if (buf != NULL) {
            if (strcmp(buf,"armour")==0)
               outfit_setProp(temp, OUTFIT_PROP_WEAP_BLOWUP_SHIELD);
            else if (strcmp(buf,"shield")==0)
               outfit_setProp(temp, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR);
            else
               WARN(_("Outfit '%s' has invalid blowup property: '%s'"),
                     temp->name, buf );
            free(buf);
         }
         temp->u.blt.range = xml_getFloat(node);
         continue;
      }
      xmlr_float(node,"falloff",temp->u.blt.falloff);

      /* Graphics. */
      if (xml_isNode(node,"gfx")) {
         outfit_loadGFX( temp, node );
         continue;
      }
      if (xml_isNode(node,"gfx_end")) {
         temp->u.blt.gfx.tex_end = xml_parseTexture( node,
               OUTFIT_GFX_PATH"space/%s", 6, 6,
               OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS );
         continue;
      }

      /* Special effects. */
      if (xml_isNode(node,"spfx_shield")) {
         temp->u.blt.spfx_shield = spfx_get(xml_get(node));
         if (temp->u.blt.spfx_shield < 0)
            WARN(_("Outfit '%s' has unknown spfx_shield '%s'!"),temp->name,xml_get(node));
         continue;
      }
      if (xml_isNode(node,"spfx_armour")) {
         temp->u.blt.spfx_armour = spfx_get(xml_get(node));
         if (temp->u.blt.spfx_armour < 0)
            WARN(_("Outfit '%s' has unknown spfx_armour '%s'!"),temp->name,xml_get(node));
         continue;
      }

      /* Misc. */
      if (xml_isNode(node,"sound")) {
         temp->u.blt.sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound_hit")) {
         temp->u.blt.sound_hit = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"damage")) {
         outfit_parseDamage( &temp->u.blt.dmg, node );
         continue;
      }

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* If not defined assume maximum. */
   if (temp->u.blt.falloff < 0.)
      temp->u.blt.falloff = temp->u.blt.range;

   /* Post processing. */
   temp->u.blt.swivel  *= M_PI/180.;
   temp->u.blt.dispersion *= M_PI/180.;
   if (outfit_isTurret(temp))
      temp->u.blt.swivel = M_PI;
   /*
    *         dT Mthermal - Qweap
    * Hweap = ----------------------
    *                tweap
    */
   C = pilot_heatCalcOutfitC(temp);
   area = pilot_heatCalcOutfitArea(temp);
   temp->u.blt.heat     = ((800.-CONST_SPACE_STAR_TEMP)*C +
            STEEL_HEAT_CONDUCTIVITY * ((800-CONST_SPACE_STAR_TEMP) * area)) /
         temp->u.blt.heatup * temp->u.blt.delay;

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD( l, temp, p_("outfitstats","%s [%s]"), _(outfit_getType(temp)),
         _(dtype_damageTypeToStr(temp->u.blt.dmg.type)) );
   dtype_raw( temp->u.blt.dmg.type, &dshield, &darmour, &dknockback );
   // new_opts(name, unit, color, threshold, hidezero, precision)
   l = os_printD( temp->summary_raw, l, darmour*100., &darmour_opts );
   l = os_printD( temp->summary_raw, l, dshield*100., &dshield_opts );
   l = os_printD( temp->summary_raw, l, dknockback*100., &dknockback_opts );
   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass, &mass_opts );
   /* Higher level stats. */
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.blt.dmg.damage, &damage_opts, temp->u.blt.shots,
         (double)temp->u.blt.shots * temp->u.blt.dmg.damage / temp->u.blt.delay, &dps_opts );
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.blt.dmg.disable, &disable_opts, temp->u.blt.shots,
         (double) temp->u.blt.shots * temp->u.blt.dmg.disable / temp->u.blt.delay, &disable_rate_opts );
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.blt.energy, &energy_opts, 1,
         (double)temp->u.blt.energy / temp->u.blt.delay, &power_opts );
   /* Standard stats. */
   l = os_printD( temp->summary_raw, l, temp->u.blt.dmg.penetration * 100., &penetration_opts );
   l = os_printD( temp->summary_raw, l, 1./temp->u.blt.delay, &fire_rate_opts );
   if (temp->u.blt.radius > 0.) {
      char radius[STRMAX_SHORT];
      snprintf(radius, sizeof(radius), outfit_isProp(temp, OUTFIT_PROP_WEAP_FRIENDLYFIRE) ? p_("friendlyfire","#r!! %s !!#0") : "%s", _("Hit radius"));
      t_os_stat radius_opts = {
         .name = radius,
         .unit = _UNIT_DISTANCE,
         .color = 0,
         .color_threshold = 0,
         .hide_zero = 1,
         .precision = 0,
      };
      l = os_printD( temp->summary_raw, l, temp->u.blt.radius, &radius_opts );
   }
   l = os_printD( temp->summary_raw, l, temp->u.blt.range, &range_opts );
   l = os_printD( temp->summary_raw, l, temp->u.blt.speed, &speed_opts );
   l = os_printD( temp->summary_raw, l, temp->u.blt.heatup, &heatup_opts );
   l = os_printD( temp->summary_raw, l, temp->u.blt.dispersion*180./M_PI, &dispersion_opts );
   if (!outfit_isTurret(temp))
      l = os_printD( temp->summary_raw, l, temp->u.blt.swivel*180./M_PI, &swivel_opts );
   l = os_printD_range( temp->summary_raw, l, temp->u.blt.trackmin, temp->u.blt.trackmax, &tracking_opts );
   sdesc_miningRarity( &l, temp, temp->u.blt.mining_rarity );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   //MELEMENT(temp->u.blt.gfx_space==NULL,"gfx");
   MELEMENT(temp->u.blt.spfx_shield==-1,"spfx_shield");
   MELEMENT(temp->u.blt.spfx_armour==-1,"spfx_armour");
   MELEMENT((sound_disabled!=0) && (temp->u.blt.sound<0),"sound");
   MELEMENT(temp->mass==0.,"mass");
   MELEMENT(temp->u.blt.delay==0,"delay");
   MELEMENT(temp->u.blt.speed==0,"speed");
   MELEMENT(temp->u.blt.range==0,"range");
   MELEMENT(temp->u.blt.dmg.damage==0,"damage");
   MELEMENT(temp->u.blt.energy==0.,"energy");
   //MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.blt.falloff > temp->u.blt.range,"falloff");
   MELEMENT(temp->u.blt.heatup==0.,"heatup");
   MELEMENT(((temp->u.blt.swivel > 0.) || outfit_isTurret(temp)) && (temp->u.blt.trackmin<0.),"trackmin");
   MELEMENT(((temp->u.blt.swivel > 0.) || outfit_isTurret(temp)) && (temp->u.blt.trackmax<0.),"trackmax");
#undef MELEMENT
}

/**
 * @brief Parses the beam weapon specifics of an outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSBeam( Outfit* temp, const xmlNodePtr parent )
{
   ShipStatList *ll;
   int l;
   xmlNodePtr node;
   double C, area;
   double dshield, darmour, dknockback;
   char *shader;

   /* Defaults. */
   temp->u.bem.spfx_armour    = -1;
   temp->u.bem.spfx_shield    = -1;
   temp->u.bem.sound_warmup   = -1;
   temp->u.bem.sound          = -1;
   temp->u.bem.sound_off      = -1;

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      xmlr_float(node,"range",temp->u.bem.range);
      xmlr_float(node,"turn",temp->u.bem.turn);
      xmlr_float(node,"energy",temp->u.bem.energy);
      xmlr_float(node,"delay",temp->u.bem.delay);
      xmlr_float(node,"warmup",temp->u.bem.warmup);
      xmlr_float(node,"heatup",temp->u.bem.heatup);
      xmlr_float(node,"swivel",temp->u.bem.swivel);
      xmlr_int(node,"mining_rarity",temp->u.bem.mining_rarity);
      xmlr_strd(node,"lua",temp->lua_file);
      if (xml_isNode(node,"pointdefense")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_POINTDEFENSE);
         continue;
      }
      if (xml_isNode(node,"miss_ships")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_SHIPS);
         continue;
      }
      if (xml_isNode(node,"miss_asteroids")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_ASTEROIDS);
         continue;
      }

      if (xml_isNode(node, "duration")) {
         xmlr_attr_float(node, "min", temp->u.bem.min_duration);
         temp->u.bem.duration = xml_getFloat(node);
         continue;
      }

      if (xml_isNode(node,"damage")) {
         outfit_parseDamage( &temp->u.bem.dmg, node );
         continue;
      }

      /* Graphic stuff. */
      if (xml_isNode(node,"shader")) {
         xmlr_attr_float(node, "r", temp->u.bem.colour.r);
         xmlr_attr_float(node, "g", temp->u.bem.colour.g);
         xmlr_attr_float(node, "b", temp->u.bem.colour.b);
         xmlr_attr_float(node, "a", temp->u.bem.colour.a);
         xmlr_attr_float(node, "width", temp->u.bem.width);
         col_gammaToLinear( &temp->u.bem.colour );
         shader = xml_get(node);
         if (gl_has( OPENGL_SUBROUTINES )) {
            temp->u.bem.shader = glGetSubroutineIndex( shaders.beam.program, GL_FRAGMENT_SHADER, shader );
            if (temp->u.bem.shader == GL_INVALID_INDEX)
               WARN("Beam outfit '%s' has unknown shader function '%s'", temp->name, shader);
         }
         continue;
      }
      if (xml_isNode(node,"spfx_armour")) {
         temp->u.bem.spfx_armour = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"spfx_shield")) {
         temp->u.bem.spfx_shield = spfx_get(xml_get(node));
         continue;
      }

      /* Sound stuff. */
      if (xml_isNode(node,"sound_warmup")) {
         temp->u.bem.sound_warmup = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound")) {
         temp->u.bem.sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound_off")) {
         temp->u.bem.sound_off = sound_get( xml_get(node) );
         continue;
      }

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post processing. */
   temp->u.bem.swivel  *= M_PI/180.;
   temp->u.bem.turn    *= M_PI/180.; /* Convert to rad/s. */
   C = pilot_heatCalcOutfitC(temp);
   area = pilot_heatCalcOutfitArea(temp);
   temp->u.bem.heat     = ((800.-CONST_SPACE_STAR_TEMP)*C +
            STEEL_HEAT_CONDUCTIVITY * ((800-CONST_SPACE_STAR_TEMP) * area)) /
         temp->u.bem.heatup * (temp->u.bem.delay+temp->u.bem.warmup+temp->u.bem.duration) / temp->u.bem.delay;

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD(  l, temp, "%s [%s]", _(outfit_getType(temp)),_(dtype_damageTypeToStr(temp->u.bem.dmg.type)) );
   dtype_raw( temp->u.bem.dmg.type, &dshield, &darmour, &dknockback );
   l = os_printD( temp->summary_raw, l, darmour*100., &darmour_opts );
   l = os_printD( temp->summary_raw, l, dshield*100., &dshield_opts );
   l = os_printD( temp->summary_raw, l, dknockback*100., &dknockback_opts );
   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass, &mass_opts );
   /* Higher level stats. */
   l = os_printD( temp->summary_raw, l, temp->u.bem.dmg.damage, &dps_opts ); /* TODO display DPS also? */
   l = os_printD( temp->summary_raw, l, temp->u.bem.dmg.disable, &disable_rate_opts );
   l = os_printD( temp->summary_raw, l, temp->u.bem.energy, &power_opts );
   /* Standard stats. */
   l = os_printD( temp->summary_raw, l, temp->u.bem.dmg.penetration * 100, &penetration_opts );
   l = os_printD_range( temp->summary_raw, l,
                 temp->u.bem.min_duration,
                 temp->u.bem.duration,
                 &duration_opts );
   l = os_printD( temp->summary_raw, l, temp->u.bem.delay, &cooldown_opts );
   l = os_printD( temp->summary_raw, l, temp->u.bem.range, &range_opts );
   l = os_printD( temp->summary_raw, l, temp->u.bem.heatup, &heatup_opts );
   if (!outfit_isTurret(temp))
      l = os_printD( temp->summary_raw, l, temp->u.bem.swivel*180./M_PI, &swivel_opts );
   sdesc_miningRarity( &l, temp, temp->u.bem.mining_rarity );

#define MELEMENT(o,s) \
if (o) WARN( _("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.bem.width==0.,"shader width");
   MELEMENT(temp->u.bem.spfx_shield==-1,"spfx_shield");
   MELEMENT(temp->u.bem.spfx_armour==-1,"spfx_armour");
   MELEMENT((sound_disabled!=0) && (temp->u.bem.warmup > 0.) && (temp->u.bem.sound<0),"sound_warmup");
   MELEMENT((sound_disabled!=0) && (temp->u.bem.sound<0),"sound");
   MELEMENT((sound_disabled!=0) && (temp->u.bem.sound_off<0),"sound_off");
   MELEMENT(temp->u.bem.delay==0,"delay");
   MELEMENT(temp->u.bem.duration==0,"duration");
   MELEMENT(temp->u.bem.min_duration < 0,"duration");
   MELEMENT(temp->u.bem.range==0,"range");
   MELEMENT((temp->type!=OUTFIT_TYPE_BEAM) && (temp->u.bem.turn==0),"turn");
   MELEMENT(temp->u.bem.energy==0.,"energy");
   MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.bem.dmg.damage==0,"damage");
   MELEMENT(temp->u.bem.heatup==0.,"heatup");
#undef MELEMENT
}

/**
 * @brief Parses the specific area for a launcher and loads it into Outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent )
{
   ShipStatList *ll;
   xmlNodePtr node;
   double dshield, darmour, dknockback;
   int l;

   temp->u.lau.trackmin    = -1.;
   temp->u.lau.trackmax    = -1.;
   temp->u.lau.spfx_armour = -1;
   temp->u.lau.spfx_shield = -1;
   temp->u.lau.sound       = -1;
   temp->u.lau.sound_hit   = -1;
   temp->u.lau.trail_spec  = NULL;
   temp->u.lau.ai          = -1;
   temp->u.lau.speed_max   = -1.;
   temp->u.lau.shots       = 1;

   node  = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      xmlr_float(node,"delay",temp->u.lau.delay);
      xmlr_int(node,"amount",temp->u.lau.amount);
      xmlr_float(node,"reload_time",temp->u.lau.reload_time);
      xmlr_float(node,"trackmin",temp->u.lau.trackmin);
      xmlr_float(node,"trackmax",temp->u.lau.trackmax);
      xmlr_float(node,"lockon",temp->u.lau.lockon);
      xmlr_float(node,"iflockon",temp->u.lau.iflockon);
      xmlr_float(node,"swivel",temp->u.lau.swivel);
      xmlr_float(node,"dispersion",temp->u.lau.dispersion);
      xmlr_float(node,"speed_dispersion",temp->u.lau.speed_dispersion);
      xmlr_float(node,"armour",temp->u.lau.armour);
      xmlr_float(node,"absorb",temp->u.lau.dmg_absorb);
      xmlr_int(node,"shots",temp->u.lau.shots);
      xmlr_int(node,"mining_rarity",temp->u.lau.mining_rarity);
      xmlr_strd(node,"lua",temp->lua_file);
      if (xml_isNode(node,"radius")) {
         char *buf;
         temp->u.lau.radius = xml_getFloat(node);
         xmlr_attr_strd(node,"friendlyfire",buf);
         if (buf != NULL) {
            outfit_setProp(temp, OUTFIT_PROP_WEAP_FRIENDLYFIRE);
            free(buf);
         }
         continue;
      }
      if (xml_isNode(node,"pointdefense")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_POINTDEFENSE);
         continue;
      }
      if (xml_isNode(node,"miss_ships")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_SHIPS);
         continue;
      }
      if (xml_isNode(node,"miss_asteroids")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_ASTEROIDS);
         continue;
      }
      if (xml_isNode(node,"miss_explode")) {
         outfit_setProp(temp, OUTFIT_PROP_WEAP_MISS_EXPLODE);
         continue;
      }

      if (!outfit_isTurret(temp))
         xmlr_float(node,"arc",temp->u.lau.arc); /* This is in semi-arc like swivel. */

      /* Ammo stuff. */
      /* Basic */
      if (xml_isNode(node,"duration")) {
         char *buf;
         xmlr_attr_strd(node,"blowup",buf);
         if (buf != NULL) {
            if (strcmp(buf,"armour")==0)
               outfit_setProp(temp, OUTFIT_PROP_WEAP_BLOWUP_SHIELD);
            else if (strcmp(buf,"shield")==0)
               outfit_setProp(temp, OUTFIT_PROP_WEAP_BLOWUP_ARMOUR);
            else
               WARN(_("Outfit '%s' has invalid blowup property: '%s'"),
                     temp->name, buf );
            free(buf);
         }
         temp->u.lau.duration = xml_getFloat(node);
         continue;
      }
      xmlr_float(node,"resist",temp->u.lau.resist);
      /* Movement */
      xmlr_float(node,"thrust",temp->u.lau.thrust);
      xmlr_float(node,"turn",temp->u.lau.turn);
      xmlr_float(node,"speed",temp->u.lau.speed);
      xmlr_float(node,"speed_max",temp->u.lau.speed_max);
      xmlr_float(node,"energy",temp->u.lau.energy);
      xmlr_float(node,"ammo_mass",temp->u.lau.ammo_mass);
      if (xml_isNode(node,"gfx")) {
         outfit_loadGFX( temp, node );
         continue;
      }
      if (xml_isNode(node,"spfx_armour")) {
         temp->u.lau.spfx_armour = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"spfx_shield")) {
         temp->u.lau.spfx_shield = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"sound")) {
         temp->u.lau.sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound_hit")) {
         temp->u.lau.sound_hit = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"damage")) {
         outfit_parseDamage( &temp->u.lau.dmg, node );
         continue;
      }
      if (xml_isNode(node,"trail_generator")) {
         xmlr_attr_float( node, "x", temp->u.lau.trail_x_offset );
         char *buf = xml_get(node);
         if (buf == NULL)
            buf = "default";
         temp->u.lau.trail_spec = trailSpec_get( buf );
         continue;
      }
      if (xml_isNode(node,"ai")) {
         char *buf = xml_get(node);
         if (buf != NULL) {
            if (strcmp(buf,"unguided")==0)
               temp->u.lau.ai = AMMO_AI_UNGUIDED;
            else if (strcmp(buf,"seek")==0)
               temp->u.lau.ai = AMMO_AI_SEEK;
            else if (strcmp(buf,"smart")==0)
               temp->u.lau.ai = AMMO_AI_SMART;
            else
               WARN(_("Ammo '%s' has unknown ai type '%s'."), temp->name, buf);
         }
         continue;
      }

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post processing. */
   if (!outfit_isProp(temp,OUTFIT_PROP_TEMPLATE)) {
      temp->mass -= temp->u.lau.ammo_mass*temp->u.lau.amount;
      if (temp->mass < 0.)
         WARN(_("Launcher outfit '%s' has negative mass when subtracting ammo mass!"), temp->name);
   }
   temp->u.lau.dmg_absorb /= 100.;
   temp->u.lau.swivel *= M_PI/180.;
   temp->u.lau.arc *= M_PI/180.;
   /* Note that arc will be 0. for turrets. */
   temp->u.lau.dispersion *= M_PI/180.;
   temp->u.lau.turn *= M_PI/180.; /* Convert to rad/s. */
   if (temp->u.lau.speed_max < 0.)
      temp->u.lau.speed_max = temp->u.lau.speed;
   else if (temp->u.lau.speed > 0. && temp->u.lau.thrust > 0.) /* Condition for not taking max_speed into account. */
      WARN(_("Max speed of ammo '%s' will be ignored."), temp->name);
   temp->u.lau.resist /= 100.;

   /* Short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD(  l, temp, "%s [%s]", _(outfit_getType(temp)),
         _(dtype_damageTypeToStr(temp->u.lau.dmg.type)) );
   dtype_raw( temp->u.lau.dmg.type, &dshield, &darmour, &dknockback );
   l = os_printD( temp->summary_raw, l, darmour*100., &darmour_opts );
   l = os_printD( temp->summary_raw, l, dshield*100., &dshield_opts );
   l = os_printD( temp->summary_raw, l, dknockback*100., &dknockback_opts );
   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass+temp->u.lau.ammo_mass*temp->u.lau.amount, &mass_opts ); /* Include ammo. */
   /* Higher level stats. */
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.lau.dmg.damage, &damage_opts, temp->u.lau.shots,
         temp->u.lau.dmg.damage * (double)temp->u.lau.shots / temp->u.lau.delay, &dps_opts );
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.lau.dmg.disable, &disable_opts, temp->u.lau.shots,
         temp->u.lau.dmg.disable * (double)temp->u.lau.shots / temp->u.lau.delay, &disable_rate_opts );
   l = os_printD_rate( temp->summary_raw, l,
         temp->u.lau.energy, &energy_opts, 1,
         temp->u.lau.delay * temp->u.lau.energy, &power_opts );
   /* Standard stats. */
   l = os_printD( temp->summary_raw, l, temp->u.lau.dmg.penetration*100., &penetration_opts );
   if (outfit_isSeeker(temp)) {
      l = os_printD( temp->summary_raw, l, temp->u.lau.lockon, &lockon_opts );
      l = os_printD( temp->summary_raw, l, temp->u.lau.iflockon, &inflight_calib_opts );
      l = os_printD_range( temp->summary_raw, l, temp->u.lau.trackmin, temp->u.lau.trackmax, &tracking_opts );
   }
   else {
      SDESC_ADD(  l, temp, "\n%s", _("No Seeking") );
      if (outfit_isTurret(temp) || temp->u.lau.swivel > 0.) {
         l = os_printD_range( temp->summary_raw, l, temp->u.lau.trackmin, temp->u.lau.trackmax, &tracking_opts );
         l = os_printD( temp->summary_raw, l, temp->u.lau.swivel*180./M_PI, &swivel_opts );
      }
   }

   SDESC_ADD(  l, temp, _("\n  Holds %d ammo"), temp->u.lau.amount );
   if (temp->u.lau.radius > 0.) {
      char radius[STRMAX_SHORT];
      snprintf(radius, sizeof(radius), outfit_isProp(temp, OUTFIT_PROP_WEAP_FRIENDLYFIRE) ? p_("friendlyfire","#r!! %s !!#0") : "%s", _("Hit radius"));
      t_os_stat radius_opts = {
         .name = radius,
         .unit = _UNIT_DISTANCE,
         .color = 0,
         .color_threshold = 0,
         .hide_zero = 1,
         .precision = 0,
      };
      l = os_printD( temp->summary_raw, l, temp->u.lau.radius, &radius_opts );
   }
   l = os_printD( temp->summary_raw, l, 1. / temp->u.lau.delay, &fire_rate_opts);
   l = os_printD( temp->summary_raw, l, outfit_range(temp), &range_opts );
   //l = os_printD( temp->summary_raw, l, temp->u.lau.duration, &duration_opts );

   if (temp->u.lau.thrust > 0.) {
      if (temp->u.lau.speed > 0.)
         l = os_printD( temp->summary_raw, l, temp->u.lau.speed, &initial_speed_opts );
      l = os_printD( temp->summary_raw, l, temp->u.lau.thrust, &thrust_opts );
   }
   else
      l = os_printD( temp->summary_raw, l, temp->u.lau.speed, &initial_speed_opts );
   if (!(temp->u.lau.thrust > 0. && temp->u.lau.speed > 0.))
      l = os_printD( temp->summary_raw, l, temp->u.lau.speed_max, &max_speed_opts );
   l = os_printD( temp->summary_raw, l, temp->u.lau.reload_time, &reload_opts );
   l = os_printD( temp->summary_raw, l, temp->u.lau.armour, &armour_opts );
   l = os_printD( temp->summary_raw, l, temp->u.lau.dmg_absorb*100., &absorp_opts );
   l = os_printD( temp->summary_raw, l, temp->u.lau.resist*100., &jam_res_opts );
   sdesc_miningRarity( &l, temp, temp->u.lau.mining_rarity );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.lau.delay==0.,"delay");
   //MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.lau.amount==0.,"amount");
   MELEMENT(temp->u.lau.reload_time==0.,"reload_time");
   MELEMENT(temp->u.lau.ammo_mass==0.,"mass");
   //MELEMENT(!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.gfx_space==NULL,"gfx");
   MELEMENT(!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.spfx_shield==-1,"spfx_shield");
   MELEMENT(!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.spfx_armour==-1,"spfx_armour");
   MELEMENT((sound_disabled!=0) && (temp->u.lau.sound<0),"sound");
   /* MELEMENT(temp->u.lau.thrust==0,"thrust"); */
   /* Unguided missiles don't need everything */
   if (outfit_isSeeker(temp)) {
      MELEMENT(temp->u.lau.turn==0,"turn");
      MELEMENT(temp->u.lau.trackmin<0,"trackmin");
      MELEMENT(temp->u.lau.trackmax<0,"trackmax");
      MELEMENT(temp->u.lau.lockon<0,"lockon");
      MELEMENT(!outfit_isTurret(temp) && (temp->u.lau.arc==0.),"arc");
   }
   MELEMENT(!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.speed_max==0,"speed_max");
   MELEMENT(!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.duration==0,"duration");
   MELEMENT(temp->u.lau.dmg.damage==0,"damage");
   /*MELEMENT(temp->u.lau.energy==0.,"energy");*/
#undef MELEMENT
   if (!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.speed==0. && temp->u.lau.thrust==0.)
      WARN(_("Outfit '%s' has no speed nor thrust set!"), temp->name);
   if (!outfit_isProp(temp,OUTFIT_PROP_SHOOT_DRY)&&temp->u.lau.iflockon >= temp->u.lau.duration)
      WARN(_("Outfit '%s' has longer 'iflockon' than ammo 'duration'"), temp->name);
}

/**
 * @brief Parses the modification tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent )
{
   int l;
   xmlNodePtr node = parent->children;

   do { /* load all the data */
      ShipStatList *ll;

      xml_onlyNodes(node);
      xmlr_strd(node,"lua",temp->lua_file);

      if (xml_isNode(node,"active")) {
         xmlr_attr_float(node, "cooldown", temp->u.mod.cooldown);
         temp->u.mod.active   = 1;
         temp->u.mod.duration = xml_getFloat(node);

         /* Infinity if no duration specified. */
         if (temp->u.mod.duration == 0)
            temp->u.mod.duration = INFINITY;

         continue;
      }

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }

      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD( l, temp, "%s", _(outfit_getType(temp)) );
   if (temp->u.mod.active || temp->lua_ontoggle != LUA_NOREF)
      SDESC_ADD( l, temp, "\n#r%s#0", _("Activated Outfit") );
   if (temp->u.mod.active && temp->u.mod.cooldown > 0.)
      l = os_printD( temp->summary_raw, l, temp->u.mod.cooldown, &cooldown_opts );
   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass, &mass_opts );
}

/**
 * @brief Parses the afterburner tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent )
{
   double C, area;
   size_t l;
   xmlNodePtr node = parent->children;

   /* Defaults. */
   temp->u.afb.sound = -1;
   temp->u.afb.sound_on = -1;
   temp->u.afb.sound_off = -1;

   /* must be >= 1. */
   temp->u.afb.thrust = 1.;
   temp->u.afb.speed  = 1.;

   do { /* parse the data */
      ShipStatList *ll;

      xml_onlyNodes(node);
      xmlr_float(node,"rumble",temp->u.afb.rumble);
      xmlr_strd(node,"lua",temp->lua_file);
      if (xml_isNode(node,"sound_on")) {
         temp->u.afb.sound_on = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound")) {
         temp->u.afb.sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound_off")) {
         temp->u.afb.sound_off = sound_get( xml_get(node) );
         continue;
      }
      xmlr_float(node,"thrust",temp->u.afb.thrust);
      xmlr_float(node,"speed",temp->u.afb.speed);
      xmlr_float(node,"energy",temp->u.afb.energy);
      xmlr_float(node,"mass_limit",temp->u.afb.mass_limit);
      xmlr_float(node,"heatup",temp->u.afb.heatup);
      xmlr_float(node,"heat_cap",temp->u.afb.heat_cap);
      xmlr_float(node,"heat_base",temp->u.afb.heat_base);

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD( l, temp, "%s", _(outfit_getType(temp)) );

   SDESC_ADD( l, temp, "\n#r%s#0", _("Activated Outfit") );

   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass, &mass_opts );
   l = os_printD( temp->summary_raw, l, temp->u.afb.mass_limit, &max_mass_opts );
   SDESC_ADD( l, temp, "\n%s", _("Only one can be equipped") );
   l = os_printD( temp->summary_raw, l, temp->u.afb.thrust+100., &thrust_opts );
   l = os_printD( temp->summary_raw, l, temp->u.afb.speed+100., &max_speed_opts );
   l = os_printD( temp->summary_raw, l, temp->u.afb.energy, &power_opts );
   l = os_printD( temp->summary_raw, l, temp->u.afb.rumble, &rumble_opts );

   /* Post processing. */
   temp->u.afb.thrust /= 100.;
   temp->u.afb.speed  /= 100.;
   C = pilot_heatCalcOutfitC(temp);
   area = pilot_heatCalcOutfitArea(temp);
   temp->u.afb.heat    = ((800.-CONST_SPACE_STAR_TEMP)*C +
            STEEL_HEAT_CONDUCTIVITY * ((800-CONST_SPACE_STAR_TEMP) * area)) /
         temp->u.afb.heatup;

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.afb.thrust==0.,"thrust");
   MELEMENT(temp->u.afb.speed==0.,"speed");
   MELEMENT(temp->u.afb.energy==0.,"energy");
   //MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.afb.mass_limit==0.,"mass_limit");
   MELEMENT(temp->u.afb.heatup==0.,"heatup");
#undef MELEMENT
}

/**
 * @brief Parses the fighter bay tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSFighterBay( Outfit *temp, const xmlNodePtr parent )
{
   size_t l;
   xmlNodePtr node = parent->children;

   do {
      ShipStatList *ll;

      xml_onlyNodes(node);
      xmlr_float(node,"delay",temp->u.bay.delay);
      xmlr_float(node,"reload_time",temp->u.bay.reload_time);
      xmlr_strd(node,"ship",temp->u.bay.ship);
      xmlr_float(node,"ship_mass",temp->u.bay.ship_mass);
      xmlr_int(node,"amount",temp->u.bay.amount);
      xmlr_strd(node,"lua",temp->lua_file);

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post-processing. */
   temp->mass -= temp->u.bay.ship_mass*temp->u.bay.amount;
   if (temp->mass < 0.)
      WARN(_("Fighter bay outfit '%s' has negative mass when subtracting ship mass!"), temp->name);

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD( l, temp, "%s", _(outfit_getType(temp)) );
   l = os_printD( temp->summary_raw, l, temp->cpu, &cpu_opts );
   l = os_printD( temp->summary_raw, l, temp->mass+temp->u.bay.ship_mass*temp->u.bay.amount, &mass_opts );
   SDESC_ADD( l, temp, _("\n  Holds %d ships"), temp->u.bay.amount );
   l = os_printD( temp->summary_raw, l, temp->u.bay.delay, &shots_delay_opts );
   l = os_printD( temp->summary_raw, l, temp->u.bay.reload_time, &reload_opts );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.bay.ship==NULL,"ship");
   MELEMENT(temp->u.bay.ship_mass<=0.,"ship_mass");
   MELEMENT(temp->u.bay.delay==0,"delay");
   MELEMENT(temp->u.bay.reload_time==0.,"reload_time");
   MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.bay.amount==0,"amount");
#undef MELEMENT
}

/**
 * @brief Parses the map tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSMap( Outfit *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   char *buf;

   node = parent->children;

   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   temp->u.map->systems = array_create(StarSystem*);
   temp->u.map->spobs   = array_create(Spob*);
   temp->u.map->jumps   = array_create(JumpPoint*);

   do {
      xml_onlyNodes(node);

      if (xml_isNode(node,"sys")) {
         xmlr_attr_strd(node,"name",buf);
         StarSystem *sys = system_get(buf);
         if (sys == NULL) {
            WARN(_("Map '%s' has invalid system '%s'"), temp->name, buf);
            free(buf);
            continue;
         }

         free(buf);
         array_push_back( &temp->u.map->systems, sys );

         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);

            if (xml_isNode(cur,"spob")) {
               buf = xml_get(cur);
               Spob *spob = (buf != NULL) ? spob_get(buf) : NULL;
               if ((buf != NULL) && (spob != NULL))
                  array_push_back( &temp->u.map->spobs, spob );
               else
                  WARN(_("Map '%s' has invalid spob '%s'"), temp->name, buf);
            }
            else if (xml_isNode(cur,"jump")) {
               JumpPoint *jump;
               buf = xml_get(cur);
               if ((buf != NULL) && ((jump = jump_get(xml_get(cur),
                     temp->u.map->systems[array_size(temp->u.map->systems)-1] )) != NULL))
                  array_push_back( &temp->u.map->jumps, jump );
               else
                  WARN(_("Map '%s' has invalid jump point '%s'"), temp->name, buf);
            }
            else
               WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, cur->name);
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"short_desc")) {
         temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
         snprintf( temp->summary_raw, OUTFIT_SHORTDESC_MAX, "%s", xml_get(node) );
      }
      else if (xml_isNode(node,"all")) { /* Add everything to the map */
         StarSystem *system_stack = system_getAll();
         for (int i=0;i<array_size(system_stack);i++) {
            StarSystem *ss = &system_stack[i];
            array_push_back( &temp->u.map->systems, ss );
            for (int j=0;j<array_size(system_stack[i].spobs);j++)
               array_push_back( &temp->u.map->spobs, ss->spobs[j] );
            for (int j=0;j<array_size(system_stack[i].jumps);j++)
               array_push_back( &temp->u.map->jumps, &ss->jumps[j] );
         }
      }
      else
         WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   array_shrink( &temp->u.map->systems );
   array_shrink( &temp->u.map->spobs  );
   array_shrink( &temp->u.map->jumps   );

   if (temp->summary_raw == NULL) {
      /* Set short description based on type. */
      temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
      snprintf( temp->summary_raw, OUTFIT_SHORTDESC_MAX,
            "%s", _(outfit_getType(temp)) );
   }

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s)
/**< Define to help check for data errors. */
   MELEMENT(temp->mass!=0.,"cpu");
   MELEMENT(temp->cpu!=0.,"cpu");
#undef MELEMENT
}

/**
 * @brief Parses the map tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSLocalMap( Outfit *temp, const xmlNodePtr parent )
{
   xmlNodePtr node = parent->children;

   temp->slot.type   = OUTFIT_SLOT_NA;
   temp->slot.size   = OUTFIT_SLOT_SIZE_NA;

   do {
      xml_onlyNodes(node);
      xmlr_float(node,"spob_detect",temp->u.lmap.spob_detect);
      xmlr_float(node,"jump_detect",temp->u.lmap.jump_detect);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   temp->u.lmap.spob_detect = pow2( temp->u.lmap.spob_detect );
   temp->u.lmap.jump_detect  = pow2( temp->u.lmap.jump_detect );

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->summary_raw, OUTFIT_SHORTDESC_MAX,
         "%s",
         _(outfit_getType(temp)) );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s)
/**< Define to help check for data errors. */
   MELEMENT(temp->mass!=0.,"cpu");
   MELEMENT(temp->cpu!=0.,"cpu");
#undef MELEMENT
}

/**
 * @brief Parses the GUI tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSGUI( Outfit *temp, const xmlNodePtr parent )
{
   xmlNodePtr node = parent->children;

   temp->slot.type   = OUTFIT_SLOT_NA;
   temp->slot.size   = OUTFIT_SLOT_SIZE_NA;

   do {
      xml_onlyNodes(node);
      xmlr_strd(node,"gui",temp->u.gui.gui);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->summary_raw, OUTFIT_SHORTDESC_MAX,
         _("GUI (Graphical User Interface)") );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s)
/**< Define to help check for data errors. */
   MELEMENT(temp->u.gui.gui==NULL,"gui");
   MELEMENT(temp->mass!=0.,"cpu");
   MELEMENT(temp->cpu!=0.,"cpu");
#undef MELEMENT
}

/**
 * @brief Parses the license tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSLicense( Outfit *temp, const xmlNodePtr parent )
{
   xmlNodePtr node = parent->children;

   temp->slot.type   = OUTFIT_SLOT_NA;
   temp->slot.size   = OUTFIT_SLOT_SIZE_NA;

   do {
      xml_onlyNodes(node);
      xmlr_strd(node,"provides",temp->u.lic.provides);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   if (temp->u.lic.provides==NULL)
      temp->u.lic.provides = strdup( temp->name );

   if (license_stack == NULL)
      license_stack = array_create( char* );
   array_push_back( &license_stack, temp->u.lic.provides );

   /* Set short description. */
   temp->summary_raw = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->summary_raw, OUTFIT_SHORTDESC_MAX,
         "%s",
         _(outfit_getType(temp)) );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s)
/**< Define to help check for data errors. */
   MELEMENT(temp->mass!=0.,"cpu");
   MELEMENT(temp->cpu!=0.,"cpu");
#undef MELEMENT
}

/**
 * @brief Parses and returns Outfit from parent node.

 *    @param temp Outfit to load into.
 *    @param file Path to the XML file (relative to base directory).
 *    @return 0 on success.
 */
static int outfit_parse( Outfit* temp, const char* file )
{
   xmlNodePtr node, parent;
   char *prop, *desc_extra;
   const char *cprop;
   int group, l;

   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first outfit node */
   if (parent == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), file);
      return -1;
   }

   /* Clear data. */
   memset( temp, 0, sizeof(Outfit) );
   temp->filename = strdup( file );
   desc_extra = NULL;

   /* Defaults. */
   temp->lua_env        = LUA_NOREF;
   temp->lua_descextra  = LUA_NOREF;
   temp->lua_onadd      = LUA_NOREF;
   temp->lua_onremove   = LUA_NOREF;
   temp->lua_init       = LUA_NOREF;
   temp->lua_cleanup    = LUA_NOREF;
   temp->lua_update     = LUA_NOREF;
   temp->lua_ontoggle   = LUA_NOREF;
   temp->lua_onhit      = LUA_NOREF;
   temp->lua_outofenergy = LUA_NOREF;
   temp->lua_onshoot    = LUA_NOREF;
   temp->lua_onstealth  = LUA_NOREF;
   temp->lua_onscanned  = LUA_NOREF;
   temp->lua_onscan     = LUA_NOREF;
   temp->lua_cooldown   = LUA_NOREF;
   temp->lua_land       = LUA_NOREF;
   temp->lua_takeoff    = LUA_NOREF;
   temp->lua_jumpin     = LUA_NOREF;
   temp->lua_onimpact   = LUA_NOREF;
   temp->lua_onmiss     = LUA_NOREF;
   temp->lua_price      = LUA_NOREF;
   temp->lua_buy        = LUA_NOREF;
   temp->lua_sell       = LUA_NOREF;

   xmlr_attr_strd(parent,"name",temp->name);
   if (temp->name == NULL)
      WARN(_("Outfit '%s' has invalid or no name"), file);

   node = parent->xmlChildrenNode;

   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"general")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_int(cur,"rarity",temp->rarity);
            xmlr_strd(cur,"license",temp->license);
            xmlr_strd(cur,"cond",temp->cond);
            xmlr_strd(cur,"condstr",temp->condstr);
            xmlr_float(cur,"mass",temp->mass);
            xmlr_float(cur,"cpu",temp->cpu);
            xmlr_long(cur,"price",temp->price);
            xmlr_strd(cur,"limit",temp->limit);
            xmlr_strd(cur,"description",temp->desc_raw);
            xmlr_strd(cur,"desc_extra",desc_extra);
            xmlr_strd(cur,"typename",temp->typename);
            xmlr_int(cur,"priority",temp->priority);
            if (xml_isNode(cur,"unique")) {
               outfit_setProp(temp, OUTFIT_PROP_UNIQUE);
               continue;
            }
            if (xml_isNode(cur,"shoot_dry")) {
               outfit_setProp(temp, OUTFIT_PROP_SHOOT_DRY);
               continue;
            }
            if (xml_isNode(cur,"template")) {
               outfit_setProp(temp, OUTFIT_PROP_TEMPLATE);
               continue;
            }
            else if (xml_isNode(cur,"gfx_store")) {
               temp->gfx_store = xml_parseTexture( cur,
                     OUTFIT_GFX_PATH"store/%s", 1, 1, OPENGL_TEX_MIPMAPS );
               continue;
            }
            else if (xml_isNode(cur,"gfx_overlays")) {
               xmlNodePtr ccur = cur->children;
               temp->gfx_overlays = array_create_size( glTexture*, 2 );
               do {
                  xml_onlyNodes(ccur);
                  if (xml_isNode(ccur,"gfx_overlay"))
                     array_push_back( &temp->gfx_overlays,
                           xml_parseTexture( ccur, OVERLAY_GFX_PATH"%s", 1, 1, OPENGL_TEX_MIPMAPS ) );
               } while (xml_nextNode(ccur));
               continue;
            }
            else if (xml_isNode(cur,"slot")) {
               cprop = xml_get(cur);
               if (cprop == NULL)
                  WARN(_("Outfit '%s' has an slot type invalid."), temp->name);
               else if (strcmp(cprop,"structure") == 0)
                  temp->slot.type = OUTFIT_SLOT_STRUCTURE;
               else if (strcmp(cprop,"utility") == 0)
                  temp->slot.type = OUTFIT_SLOT_UTILITY;
               else if (strcmp(cprop,"weapon") == 0)
                  temp->slot.type = OUTFIT_SLOT_WEAPON;
               else if (strcmp(cprop,"intrinsic") == 0)
                  temp->slot.type = OUTFIT_SLOT_NA;
               else if (strcmp(cprop,"none") == 0)
                  temp->slot.type = OUTFIT_SLOT_NA;
               else
                  WARN(_("Outfit '%s' has unknown slot type '%s'."), temp->name, cprop);

               /* Property. */
               xmlr_attr_strd( cur, "prop", prop );
               if (prop != NULL)
                  temp->slot.spid = sp_get( prop );
               free( prop );
               continue;
            }
            else if (xml_isNode(cur,"size")) {
               temp->slot.size = outfit_toSlotSize( xml_get(cur) );
               continue;
            }
            else if (xml_isNode(cur, "illegalto")) {
               xmlNodePtr ccur = cur->xmlChildrenNode;
               temp->illegaltoS = array_create( char* );
               do {
                  xml_onlyNodes(ccur);
                  if (xml_isNode(ccur, "faction")) {
                     const char *s = xml_get(ccur);
                     if (s==NULL)
                        WARN(_("Empty faction string for outfit '%s' legality!"), temp->name);
                     else
                        array_push_back( &temp->illegaltoS, strdup(s) );
                  }
               } while (xml_nextNode(ccur));
               if (array_size(temp->illegaltoS) <= 0 )
                  WARN(_("Outfit '%s' has no factions defined in <illegalto> block!"), temp->name);
               continue;
            }
            WARN(_("Outfit '%s' has unknown general node '%s'"),temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"stats")) {
         xmlNodePtr cur = node->children;
         do {
            ShipStatList *ll;
            xml_onlyNodes(cur);
            /* Stats. */
            ll = ss_listFromXML( cur );
            if (ll != NULL) {
               ll->next    = temp->stats;
               temp->stats = ll;
               continue;
            }
            WARN(_("Outfit '%s' has unknown node '%s'"), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      /* Parse tags. */
      if (xml_isNode(node, "tags")) {
         xmlNodePtr cur = node->children;
         temp->tags = array_create( char* );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "tag")) {
               char *tmp = xml_get(cur);
               if (tmp != NULL)
                  array_push_back( &temp->tags, strdup(tmp) );
               continue;
            }
            WARN(_("Outfit '%s' has unknown node in tags '%s'."), temp->name, cur->name );
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"specific")) { /* has to be processed separately */
         /* get the type */
         xmlr_attr_strd(node, "type", prop);
         if (prop == NULL)
            ERR(_("Outfit '%s' element 'specific' missing property 'type'"),temp->name);
         temp->type = outfit_strToOutfitType(prop);
         free(prop);

         /* is secondary weapon? */
         xmlr_attr_strd(node, "secondary", prop);
         if (prop != NULL) {
            if ((int)atoi(prop))
               outfit_setProp(temp, OUTFIT_PROP_WEAP_SECONDARY);
            free(prop);
         }

         /* Check for manually-defined group. */
         xmlr_attr_int_def(node, "group", group, -1);
         if (group != -1) {
            if (group > PILOT_WEAPON_SETS || group < 1) {
               WARN(_("Outfit '%s' has group '%d', should be in the 1-%d range"),
                     temp->name, group, PILOT_WEAPON_SETS);
            }

            temp->group = CLAMP(0, 9, group);
         }

         /*
          * Parse type.
          */
         if (temp->type==OUTFIT_TYPE_NULL)
            WARN(_("Outfit '%s' is of type NONE"), temp->name);
         else if (outfit_isBolt(temp))
            outfit_parseSBolt( temp, node );
         else if (outfit_isBeam(temp))
            outfit_parseSBeam( temp, node );
         else if (outfit_isLauncher(temp))
            outfit_parseSLauncher( temp, node );
         else if (outfit_isMod(temp))
            outfit_parseSMod( temp, node );
         else if (outfit_isAfterburner(temp))
            outfit_parseSAfterburner( temp, node );
         else if (outfit_isFighterBay(temp))
            outfit_parseSFighterBay( temp, node );
         else if (outfit_isMap(temp)) {
            temp->u.map = malloc( sizeof(OutfitMapData_t) ); /**< deal with maps after the universe is loaded */
            temp->slot.type         = OUTFIT_SLOT_NA;
            temp->slot.size         = OUTFIT_SLOT_SIZE_NA;
         }
         else if (outfit_isLocalMap(temp))
            outfit_parseSLocalMap( temp, node );
         else if (outfit_isGUI(temp))
            outfit_parseSGUI( temp, node );
         else if (outfit_isLicense(temp))
            outfit_parseSLicense( temp, node );

         /* Sort stats. */
         ss_sort( &temp->stats );

         /* We add the ship stats to the description here. */
         if (temp->summary_raw != NULL) {
            l = strlen(temp->summary_raw);
            ss_statsListDesc( temp->stats, &temp->summary_raw[l], OUTFIT_SHORTDESC_MAX-l, 1 );
            /* Add extra description task if available. */
            if (desc_extra != NULL) {
               l = strlen(temp->summary_raw);
               snprintf( &temp->summary_raw[l], OUTFIT_SHORTDESC_MAX-l, "\n%s", _(desc_extra) );
               free( desc_extra );
               desc_extra = NULL;
            }
         }

         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN( _("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->name==NULL,"name");
   if (!outfit_isProp(temp,OUTFIT_PROP_TEMPLATE)) {
      MELEMENT(temp->slot.type==OUTFIT_SLOT_NULL,"slot");
      MELEMENT((temp->slot.type!=OUTFIT_SLOT_NA) && (temp->slot.size==OUTFIT_SLOT_SIZE_NA),"size");
      MELEMENT(temp->gfx_store==NULL,"gfx_store");
      MELEMENT(temp->desc_raw==NULL,"description");
   }
   /*MELEMENT(temp->mass==0,"mass"); Not really needed */
   MELEMENT(temp->type==0,"type");
   /*MELEMENT(temp->price==0,"price");*/
   MELEMENT((temp->cond!=NULL) && (temp->condstr==NULL), "condstr");
   MELEMENT((temp->cond==NULL) && (temp->condstr!=NULL), "cond");
#undef MELEMENT

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Loads all the files in a directory.
 *
 *    @param dir Directory to load files from.
 *    @return 0 on success.
 */
static int outfit_loadDir( char *dir )
{
   char **outfit_files = ndata_listRecursive( dir );
   for (int i=0; i < array_size( outfit_files ); i++) {
      if (ndata_matchExt( outfit_files[i], "xml" )) {
         Outfit o;
         int ret = outfit_parse( &o, outfit_files[i] );
         if (ret == 0)
            array_push_back( &outfit_stack, o );

         /* Render if necessary. */
         naev_renderLoadscreen();
      }
      free( outfit_files[i] );
   }
   array_free( outfit_files );

   return 0;
}

/**
 * @brief Loads all the outfits.
 *
 *    @return 0 on success.
 */
int outfit_load (void)
{
   int noutfits;
   Uint32 time = SDL_GetTicks();

   /* First pass, Loads up all outfits, without filling ammunition and the likes. */
   outfit_stack = array_create(Outfit);
   outfit_loadDir( OUTFIT_DATA_PATH );
   array_shrink( &outfit_stack );
   noutfits = array_size(outfit_stack);
   /* Sort up licenses. */
   qsort( outfit_stack, noutfits, sizeof(Outfit), outfit_cmp );
   if (license_stack != NULL)
      qsort( license_stack, array_size(license_stack), sizeof(char*), strsort );

#if DEBUGGING
   for (int i=1; i<noutfits; i++)
      if (strcmp( outfit_stack[i-1].name, outfit_stack[i].name )==0)
         WARN(_("Duplicated outfit name '%s' detected!"), outfit_stack[i].name);
#endif /* DEBUGGING */

   /* Second pass. */
   for (int i=0; i<noutfits; i++) {
      Outfit *o = &outfit_stack[i];
      if (o->lua_file==NULL)
         continue;

      nlua_env env;
      size_t sz;
      char *dat = ndata_read( o->lua_file, &sz );
      if (dat==NULL) {
         WARN(_("Outfit '%s' failed to read Lua '%s'!"), o->name, o->lua_file );
         continue;
      }

      env = nlua_newEnv();
      o->lua_env = env;
      /* TODO limit libraries here. */
      nlua_loadStandard( env );
      nlua_loadGFX( env );
      nlua_loadPilotOutfit( env );
      nlua_loadCamera( env );
      nlua_loadMunition( env );

      /* Run code. */
      if (nlua_dobufenv( env, dat, sz, o->lua_file ) != 0) {
         WARN(_("Outfit '%s' Lua error:\n%s"), o->name, lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         nlua_freeEnv( o->lua_env );
         free( dat );
         o->lua_env = LUA_NOREF;
         continue;
      }
      free( dat );

      /* Check functions as necessary. */
      o->lua_descextra  = nlua_refenvtype( env, "descextra",LUA_TFUNCTION );
      o->lua_onadd      = nlua_refenvtype( env, "onadd",    LUA_TFUNCTION );
      o->lua_onremove   = nlua_refenvtype( env, "onremove", LUA_TFUNCTION );
      o->lua_init       = nlua_refenvtype( env, "init",     LUA_TFUNCTION );
      o->lua_cleanup    = nlua_refenvtype( env, "cleanup",  LUA_TFUNCTION );
      o->lua_update     = nlua_refenvtype( env, "update",   LUA_TFUNCTION );
      o->lua_ontoggle   = nlua_refenvtype( env, "ontoggle", LUA_TFUNCTION );
      o->lua_onhit      = nlua_refenvtype( env, "onhit",    LUA_TFUNCTION );
      o->lua_outofenergy= nlua_refenvtype( env, "outofenergy",LUA_TFUNCTION );
      o->lua_onshoot    = nlua_refenvtype( env, "onshoot",  LUA_TFUNCTION );
      o->lua_onstealth  = nlua_refenvtype( env, "onstealth",LUA_TFUNCTION );
      o->lua_onscanned  = nlua_refenvtype( env, "onscanned",LUA_TFUNCTION );
      o->lua_onscan     = nlua_refenvtype( env, "onscan",   LUA_TFUNCTION );
      o->lua_cooldown   = nlua_refenvtype( env, "cooldown", LUA_TFUNCTION );
      o->lua_land       = nlua_refenvtype( env, "land",     LUA_TFUNCTION );
      o->lua_takeoff    = nlua_refenvtype( env, "takeoff",  LUA_TFUNCTION );
      o->lua_jumpin     = nlua_refenvtype( env, "jumpin",   LUA_TFUNCTION );
      o->lua_onimpact   = nlua_refenvtype( env, "onimpact", LUA_TFUNCTION );
      o->lua_onmiss     = nlua_refenvtype( env, "onmiss",   LUA_TFUNCTION );
      o->lua_price      = nlua_refenvtype( env, "price",    LUA_TFUNCTION );
      o->lua_buy        = nlua_refenvtype( env, "buy",      LUA_TFUNCTION );
      o->lua_sell       = nlua_refenvtype( env, "sell",     LUA_TFUNCTION );

      if (outfit_isMod(o)) {
         nlua_getenv( naevL, env, "notactive" );
         o->u.mod.active = 1;
         if (lua_toboolean(naevL,-1)) {
            o->u.mod.active = 0;
            if (o->lua_ontoggle != LUA_NOREF)
               WARN(_("Outfit '%s' has 'ontoggle' Lua function defined, but is set as 'notactive'!"),o->name);
         }
         lua_pop(naevL,1);
      }
   }

#ifdef DEBUGGING
   char **outfit_names = malloc( noutfits * sizeof(char*) );
   int start;

   for (int i=0; i<noutfits; i++)
      outfit_names[i] = outfit_stack[i].name;

   qsort( outfit_names, noutfits, sizeof(char*), strsort );
   for (int i=0; i<(noutfits - 1); i++) {
      start = i;
      while (strcmp(outfit_names[i], outfit_names[i+1]) == 0)
         i++;

      if (i == start)
         continue;

      WARN( n_( "Name collision! %d outfit is named '%s'", "Name collision! %d outfits are named '%s'",
                      i + 1 - start ),
            i + 1 - start, outfit_names[ start ] );
   }
   free(outfit_names);
#endif /* DEBUGGING */

   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d Outfit in %.3f s", "Loaded %d Outfits in %.3f s", noutfits ), noutfits, time/1000. );
   }
   else
      DEBUG( n_( "Loaded %d Outfit", "Loaded %d Outfits", noutfits ), noutfits );

   return 0;
}

/**
 * @brief Loads all the outfits legality.
 *
 *    @return 0 on success.
 */
int outfit_loadPost (void)
{
   for (int i=0; i<array_size(outfit_stack); i++) {
      Outfit *o = &outfit_stack[i];

      if (outfit_isFighterBay(o) && (o->u.bay.ship!=NULL) && ship_get(o->u.bay.ship)==NULL)
         WARN(_("Fighter Bay Outfit '%s' has not found ship '%s'!"),o->name,o->u.bay.ship);

      /* Add illegality as necessary. */
      if (array_size(o->illegaltoS) > 0) {
         o->illegalto = array_create_size( int, array_size(o->illegaltoS) );
         for (int j=0; j<array_size(o->illegaltoS); j++) {
            int f = faction_get( o->illegaltoS[j] );
            array_push_back( &o->illegalto, f );
            free( o->illegaltoS[j] );
         }
         array_free( o->illegaltoS );
         o->illegaltoS = NULL;

         int l = strlen( o->summary_raw );
         SDESC_ADD( l, o, "\n#r%s#0", _("Illegal to:") );
         for (int j=0; j<array_size(o->illegalto); j++)
            SDESC_ADD( l, o, _("\n#r- %s#0"), _(faction_name(o->illegalto[j])) );
      }

      /* Handle initializing module stuff. */
      if (o->lua_env != LUA_NOREF) {
         nlua_getenv( naevL, o->lua_env, "onload" );
         if (lua_isnil(naevL,-1))
            lua_pop(naevL,1);
         else {
            lua_pushoutfit( naevL, o );
            if (nlua_pcall( o->lua_env, 1, 0 )) {
               WARN(_("Outfit '%s' lua load error -> 'load':\n%s"), o->name, lua_tostring(naevL,-1));
               lua_pop(naevL,1);
            }
         }
      }

      /* Make sure licenses are valid. */
      if ((o->license!=NULL) && !outfit_licenseExists( o->license ))
         WARN(_("Outfit '%s' has inexistent license requirement '%s'!"), o->name, o->license);
   }

   return 0;
}

/**
 * @brief Parses all the maps.
 *
 */
int outfit_mapParse (void)
{
   for (int i=0; i<array_size(outfit_stack); i++) {
      xmlDocPtr doc;
      xmlNodePtr node, cur;
      Outfit *o = &outfit_stack[i];

      if (!outfit_isMap(o))
         continue;

      doc = xml_parsePhysFS( o->filename );
      if (doc == NULL)
         continue;

      node = doc->xmlChildrenNode; /* first system node */
      if (node == NULL) {
         WARN( _("Malformed '%s' file: does not contain elements"), o->filename );
         xmlFreeDoc(doc);
         continue;
      }

      cur = node->xmlChildrenNode;
      do { /* load all the data */
         /* Only handle nodes. */
         xml_onlyNodes(cur);

         if (xml_isNode(cur,"specific"))
            outfit_parseSMap(o, cur);

      } while (xml_nextNode(cur));

      /* Clean up. */
      xmlFreeDoc(doc);
   }

   return 0;
}

/**
 * Gets the texture associated to the rarity of an outfit/ship.
 */
glTexture* rarity_texture( int rarity )
{
   char s[PATH_MAX];
   snprintf( s, sizeof(s), OVERLAY_GFX_PATH"rarity_%d.webp", rarity );
   return gl_newImage( s, OPENGL_TEX_MIPMAPS );
}

/**
 * @brief Checks illegality of an outfit to a faction.
 */
int outfit_checkIllegal( const Outfit* o, int fct )
{
   for (int i=0; i<array_size(o->illegalto); i++) {
      if (o->illegalto[i] == fct)
         return 1;
   }
   return 0;
}

/**
 * @brief Checks to see if a license exists.
 */
int outfit_licenseExists( const char *name )
{
   if (license_stack==NULL)
      return 0;
   char *lic = bsearch( &name, license_stack, array_size(license_stack), sizeof(char*), strsort );
   return (lic!=NULL);
}

/**
 * @brief Frees the outfit stack.
 */
void outfit_free (void)
{
   for (int i=0; i < array_size(outfit_stack); i++) {
      Outfit *o = &outfit_stack[i];

      free( o->filename );

      /* Free graphics */
      const OutfitGFX *gfx = outfit_gfx(o);
      if (gfx != NULL) {
         gl_freeTexture( gfx->tex );
         gl_freeTexture( gfx->tex_end );
         for (int j=0; j<array_size(gfx->polygon); j++) {
            free(gfx->polygon[j].x);
            free(gfx->polygon[j].y);
         }
         array_free(gfx->polygon);
         glDeleteProgram(gfx->program);
      }

      /* Free slot. */
      outfit_freeSlot( &o->slot );

      /* Free stats. */
      ss_free( o->stats );

      /* Free illegality. */
      array_free( o->illegalto );

      if (outfit_isFighterBay(o))
         free(o->u.bay.ship);
      else if (outfit_isGUI(o))
         free(o->u.gui.gui);
      else if (outfit_isLicense(o))
         free(o->u.lic.provides);
      else if (outfit_isMap(o)) {
         array_free( o->u.map->systems );
         array_free( o->u.map->spobs );
         array_free( o->u.map->jumps );
         free( o->u.map );
      }

      /* Lua. */
      nlua_freeEnv( o->lua_env );
      o->lua_env = LUA_NOREF;
      free(o->lua_file);

      /* strings */
      free(o->typename);
      free(o->desc_raw);
      free(o->limit);
      free(o->summary_raw);
      free(o->license);
      free(o->cond);
      free(o->condstr);
      free(o->name);
      gl_freeTexture(o->gfx_store);
      for (int j=0; j<array_size(o->gfx_overlays); j++)
         gl_freeTexture(o->gfx_overlays[j]);
      array_free(o->gfx_overlays);

      /* Free tags. */
      for (int j=0; j<array_size(o->tags); j++)
         free(o->tags[j]);
      array_free(o->tags);
   }

   array_free(outfit_stack);
   array_free(license_stack);
}

/**
 * @brief Writes an outfit statistic to a buffer.
 *
 *    @param buffer Buffer to write to.
 *    @param i Position to write at.
 *    @param value Value to write.
 *    @param opts Format to use to write value.
 *    @return Up until where was written in the buffer.
 */
static int os_printD( char *buffer, int i, double value, const t_os_stat *opts )
{
   const int MAXLEN = OUTFIT_SHORTDESC_MAX-i;

   if (opts->hide_zero && fabs(value) < 1e-2)
       return i;

   i += scnprintf(buffer + i, MAXLEN, "\n");
   if (opts->color)
      i += scnprintf(buffer + i, MAXLEN,
                     value > opts->color_threshold ? "#g" :
                     value < opts->color_threshold ? "#r" : "");
   /* The brochure of the International System of Units declares in chapter 5: "a space separates the number and the symbol %". The ISO 31-0 standard also specifies a space, and the TeX typesetting system encourages using one. */
   i += scnprintf(buffer + i, MAXLEN, p_("outfitstats", "%s: %s %s"), _(opts->name), num2strU( value, opts->precision ), opts->unit ? _(opts->unit) : "" );
   if (opts->color)
      i += scnprintf(buffer + i, MAXLEN, "#0");
   return i;
}

/**
 * @brief Writes an outfit statistic representing a range between two values to a buffer.
 *
 *    @param buffer Buffer to write to.
 *    @param i Position to write at.
 *    @param minValue Lower value bound.
 *    @param maxValue Upper value bound.
 *    @param opts Format to use to write value.
 *    @return Up until where was written in the buffer.
 */
static int os_printD_range( char *buffer, int i, double minValue, double maxValue, const t_os_stat *opts )
{
   const int MAXLEN = OUTFIT_SHORTDESC_MAX-i;
   char buf1[NUM2STRLEN], buf2[NUM2STRLEN];

   if (opts->hide_zero && fabs(maxValue) < 1e-2)
       return i;

   num2str( buf1, minValue, opts->precision );
   num2str( buf2, maxValue, opts->precision );

   i += scnprintf(buffer + i, MAXLEN, "\n");
   if (opts->color)
      i += scnprintf(buffer + i, MAXLEN,
                     maxValue > opts->color_threshold ? "#g" :
                     maxValue < opts->color_threshold ? "#r" : "");
   i += scnprintf(buffer + i, MAXLEN, p_("outfitstats", "%s: %s %s - %s %s"), _(opts->name),
         buf1, _(opts->unit), buf2, _(opts->unit) );
   if (opts->color)
      i += scnprintf(buffer + i, MAXLEN, "#0");
   return i;
}

/**
 * @brief Writes an outfit statistic representing a "per unit" value and rate of change value.
 *
 *    @param buffer Buffer to write to.
 *    @param i Position to write at.
 *    @param val Main "per unit value".
 *    @param val_opts Format used to write val.
 *    @param multiplier Multiplication value for val, or <=1 to disable.
 *    @param rate Rate of change value.
 *    @param rate_opts Format to use to write rate.
 *    @return Up until where was written in the buffer.
 */
static int os_printD_rate( char *buffer, int i, double val, const t_os_stat *val_opts, int multiplier, double rate, const t_os_stat *rate_opts )
{
   const int MAXLEN = OUTFIT_SHORTDESC_MAX-i;
   char mult[128];
   char buf1[NUM2STRLEN], buf2[NUM2STRLEN];

   if (val_opts->hide_zero && fabs(val) < 1e-2)
       return i;

   i += scnprintf(buffer + i, MAXLEN, "\n");
   if (val_opts->color)
      i += scnprintf(buffer + i, MAXLEN,
                     val > val_opts->color_threshold ? "#g" :
                     val < val_opts->color_threshold ? "#r" : "");

   if (multiplier > 1)
      snprintf( mult, sizeof(mult), p_("multiplier", " x %d"), multiplier );
   else
      mult[0] = '\0';

   num2str( buf1, val, val_opts->precision );
   num2str( buf2, rate, rate_opts->precision );

   i += scnprintf(buffer + i, MAXLEN, p_("outfitstats", "%s: %s%s %s [%s %s]"), _(val_opts->name),
                  buf1, mult, _(val_opts->unit), buf2, _(rate_opts->unit) );
   if (val_opts->color)
      i += scnprintf(buffer + i, MAXLEN, "#0");
   return i;
}
