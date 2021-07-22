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
#include "nstring.h"
#include "nstring.h"
#include "nxml.h"
#include "pilot.h"
#include "pilot_heat.h"
#include "ship.h"
#include "slots.h"
#include "spfx.h"
#include "unistd.h"


#define outfit_setProp(o,p)      ((o)->properties |= p) /**< Checks outfit property. */

#define XML_OUTFIT_TAG     "outfit"    /**< XML section identifier. */

#define OUTFIT_SHORTDESC_MAX  1024 /**< Max length of the short description of the outfit. */


/*
 * the stack
 */
static Outfit* outfit_stack = NULL; /**< Stack of outfits. */


/*
 * Helper stuff for setting up short descriptions for outfits.
 */
#define SDESC_ADD( l, temp, txt, args... ) \
(l) += scnprintf( &(temp)->desc_short[l], OUTFIT_SHORTDESC_MAX-(l), (txt), ## args )
#define SDESC_COND( l, temp, txt, val, args... ) \
if (fabs(val) > 1e-5) \
   (l) += scnprintf( &(temp)->desc_short[l], OUTFIT_SHORTDESC_MAX-(l), (txt), (val), ## args )


/*
 * Prototypes
 */
/* misc */
static OutfitType outfit_strToOutfitType( char *buf );
static int outfit_setDefaultSize( Outfit *o );
static void outfit_launcherDesc( Outfit* o );
/* parsing */
static int outfit_loadDir( char *dir );
static int outfit_parseDamage( Damage *dmg, xmlNodePtr node );
static int outfit_parse( Outfit* temp, const char* file );
static void outfit_parseSBolt( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSBeam( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSFighterBay( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSFighter( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSMap( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSLocalMap( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSGUI( Outfit *temp, const xmlNodePtr parent );
static void outfit_parseSLicense( Outfit *temp, const xmlNodePtr parent );
static int outfit_loadPLG( Outfit *temp, const char *buf, unsigned int bolt );


/**
 * @brief Gets an outfit by name.
 *
 *    @param name Name to match.
 *    @return Outfit matching name or NULL if not found.
 */
Outfit* outfit_get( const char* name )
{
   int i;

   for (i=0; i<array_size(outfit_stack); i++)
      if (strcmp(name,outfit_stack[i].name)==0)
         return &outfit_stack[i];

   WARN(_("Outfit '%s' not found in stack."), name);
   return NULL;
}


/**
 * @brief Gets an outfit by name without warning on no-find.
 *
 *    @param name Name to match.
 *    @return Outfit matching name or NULL if not found.
 */
Outfit* outfit_getW( const char* name )
{
   int i;
   for (i=0; i<array_size(outfit_stack); i++)
      if (strcmp(name,outfit_stack[i].name)==0)
         return &outfit_stack[i];
   return NULL;
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
   int i;
   for (i=0; i<array_size(outfit_stack); i++)
      if (strcasecmp(name,outfit_stack[i].name)==0)
         return outfit_stack[i].name;
   return NULL;
}


/**
 * @brief Does a fuzzy search of all the outfits. Searches translated names but returns internal names.
 */
char **outfit_searchFuzzyCase( const char* name, int *n )
{
   int i, len, nstack;
   char **names;

   /* Overallocate to maximum. */
   nstack = array_size(outfit_stack);
   names = malloc( sizeof(char*) * nstack );

   /* Do fuzzy search. */
   len = 0;
   for (i=0; i<nstack; i++) {
      if (strcasestr( _(outfit_stack[i].name), name ) != NULL) {
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
   switch( o ) {
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
 * @brief Sets the outfit slot size from default outfit properties.
 */
static int outfit_setDefaultSize( Outfit *o )
{
   if (o->mass <= 10.)
      o->slot.size = OUTFIT_SLOT_SIZE_LIGHT;
   else if (o->mass <= 30.)
      o->slot.size = OUTFIT_SLOT_SIZE_MEDIUM;
   else
      o->slot.size = OUTFIT_SLOT_SIZE_HEAVY;
   WARN(_("Outfit '%s' has implicit slot size, setting to '%s'."),o->name,outfit_slotSize(o));
   return 0;
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
   if (outfit_isMod(o) && (o->u.mod.active || o->u.mod.lua_env != LUA_NOREF))
      return 1;
   if (outfit_isAfterburner(o))
      return 1;
   return 0;
}


/**
 * @brief Checks if outfit is a fixed mounted weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a weapon (beam/bolt).
 */
int outfit_isForward( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BOLT)      ||
         (o->type==OUTFIT_TYPE_BEAM) );
}
/**
 * @brief Checks if outfit is bolt type weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a bolt type weapon.
 */
int outfit_isBolt( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BOLT)      ||
         (o->type==OUTFIT_TYPE_TURRET_BOLT) );
}
/**
 * @brief Checks if outfit is a beam type weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a beam type weapon.
 */
int outfit_isBeam( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BEAM)      ||
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
 * @brief Checks if outfit is ammo for a launcher.
 *    @param o Outfit to check.
 *    @return 1 if o is ammo.
 */
int outfit_isAmmo( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_AMMO);
}
/**
 * @brief Checks if outfit is a seeking weapon.
 *    @param o Outfit to check.
 *    @return 1 if o is a seeking weapon.
 */
int outfit_isSeeker( const Outfit* o )
{
   if ((o->type==OUTFIT_TYPE_AMMO) && (o->u.amm.ai > 0))
      return 1;
   if (((o->type==OUTFIT_TYPE_TURRET_LAUNCHER) ||
            (o->type==OUTFIT_TYPE_LAUNCHER)) &&
         (o->u.lau.ammo->u.amm.ai > 0))
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
   return ( (o->type==OUTFIT_TYPE_TURRET_BOLT)  ||
         (o->type==OUTFIT_TYPE_TURRET_BEAM)     ||
         (o->type==OUTFIT_TYPE_TURRET_LAUNCHER) );
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
 * @brief Checks if outfit is a fighter.
 *    @param o Outfit to check.
 *    @return 1 if o is a Fighter.
 */
int outfit_isFighter( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_FIGHTER);
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
glTexture* outfit_gfx( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.gfx_space;
   else if (outfit_isAmmo(o)) return o->u.amm.gfx_space;
   return NULL;
}
/**
 * @brief Gets the outfit's collision polygon.
 *    @param o Outfit to get information from.
 */
CollPoly* outfit_plg( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.polygon;
   else if (outfit_isAmmo(o)) return o->u.amm.polygon;
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
   else if (outfit_isAmmo(o)) return o->u.amm.spfx_armour;
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
   else if (outfit_isAmmo(o)) return o->u.amm.spfx_shield;
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
   else if (outfit_isAmmo(o)) return &o->u.amm.dmg;
   return NULL;
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
 * @brief Gets the outfit's ammo.
 *    @param o Outfit to get information from.
 */
Outfit* outfit_ammo( const Outfit* o )
{
   if (outfit_isLauncher(o)) return o->u.lau.ammo;
   else if (outfit_isFighterBay(o)) return o->u.bay.ammo;
   return NULL;
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
   else if (outfit_isAmmo(o)) return o->u.amm.energy;
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
   Outfit *amm;
   double at;

   if (outfit_isBolt(o)) return o->u.blt.falloff + (o->u.blt.range - o->u.blt.falloff)/2.;
   else if (outfit_isBeam(o)) return o->u.bem.range;
   else if (outfit_isAmmo(o)) {
      if (o->u.amm.thrust) {
         at = o->u.amm.speed / o->u.amm.thrust;
         if (at < o->u.amm.duration)
            return o->u.amm.speed * (o->u.amm.duration - at / 2.);

         /* Maximum speed will never be reached. */
         return pow2(o->u.amm.duration) * o->u.amm.thrust / 2.;
      }

      return o->u.amm.speed * o->u.amm.duration;
   }
   else if (outfit_isLauncher(o)) {
      amm = outfit_ammo(o);
      if (amm != NULL)
         return outfit_range(amm);
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
   Outfit *amm;
   double t;
   if (outfit_isBolt(o)) return o->u.blt.speed;
   else if (outfit_isAmmo(o)) {
      if (o->u.amm.thrust == 0)
         return o->u.amm.speed;
      else {     /*Gets the average speed*/
         t = o->u.amm.speed / o->u.amm.thrust; /*time to reach max speed*/
         if (t < o->u.amm.duration)
            return ( o->u.amm.thrust * t * t / 2 + o->u.amm.speed * ( o->u.amm.duration - t ) ) / o->u.amm.duration;
         else return o->u.amm.thrust * o->u.amm.duration/2;
      }
   }
   else if (outfit_isLauncher(o)) {
      amm = outfit_ammo(o);
      if (amm != NULL)
         return outfit_speed(amm);
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
   if (outfit_isBolt(o)) return o->u.blt.spin;
   else if (outfit_isAmmo(o)) return o->u.amm.spin;
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
 * @brief Gets the outfit's sound.
 *    @param o Outfit to get sound from.
 *    @return Outfit's sound.
 */
int outfit_sound( const Outfit* o )
{
   if (outfit_isBolt(o)) return o->u.blt.sound;
   else if (outfit_isAmmo(o)) return o->u.amm.sound;
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
   else if (outfit_isAmmo(o)) return o->u.amm.sound_hit;
   return -1.;
}
/**
 * @brief Gets the outfit's duration.
 *    @param o Outfit to get the duration of.
 *    @return Outfit's duration.
 */
double outfit_duration( const Outfit* o )
{
   Outfit *amm;
   if (outfit_isMod(o)) { if (o->u.mod.active) return o->u.mod.duration; }
   else if (outfit_isAfterburner(o)) return INFINITY;
   else if (outfit_isBolt(o)) return (o->u.blt.range / o->u.blt.speed);
   else if (outfit_isBeam(o)) return o->u.bem.duration;
   else if (outfit_isAmmo(o)) return o->u.amm.duration;
   else if (outfit_isLauncher(o)) {
      amm = outfit_ammo(o);
      if (amm != NULL)
         return outfit_duration(amm);
   }
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
         N_("Ammunition"),
         N_("Turret Launcher"),
         N_("Ship Modification"),
         N_("Afterburner"),
         N_("Fighter Bay"),
         N_("Fighter"),
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
   if (outfit_isBolt(o))            return gettext_noop("Bolt Weapon");
   else if (outfit_isBeam(o))       return gettext_noop("Beam Weapon");
   else if (outfit_isLauncher(o))   return gettext_noop("Launcher");
   else if (outfit_isAmmo(o))       return gettext_noop("Ammo");
   //else if (outfit_isTurret(o))     return gettext_noop("Turret");
   else if (outfit_isMod(o))        return gettext_noop("Modification");
   else if (outfit_isAfterburner(o)) return gettext_noop("Afterburner");
   else if (outfit_isFighterBay(o)) return gettext_noop("Fighter Bay");
   else if (outfit_isFighter(o))    return gettext_noop("Fighter");
   else if (outfit_isMap(o))        return gettext_noop("Map");
   else if (outfit_isLocalMap(o))   return gettext_noop("Local Map");
   else if (outfit_isGUI(o))        return gettext_noop("GUI");
   else if (outfit_isLicense(o))    return gettext_noop("License");
   else                             return gettext_noop("Unknown");
}


/**
 * @brief Gets a human-readable string describing an ammo outfit's AI.
 *    @param o Ammo outfit.
 *    @return Name of the outfit's AI.
 */
const char* outfit_getAmmoAI( const Outfit *o )
{
   const char *ai_type[] = {
      gettext_noop("Unguided"),
      gettext_noop("Seek"),
      gettext_noop("Smart")
   };

   if (!outfit_isAmmo(o)) {
      WARN(_("Outfit '%s' is not an ammo outfit"), o->name);
      return NULL;
   }

   return ai_type[o->u.amm.ai];
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
   const OutfitSlot *os;
   os = &o->slot;

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
   O_CMP("ammo",           OUTFIT_TYPE_AMMO);
   O_CMP("turret launcher",OUTFIT_TYPE_TURRET_LAUNCHER);
   O_CMP("modification",   OUTFIT_TYPE_MODIFICATION);
   O_CMP("afterburner",    OUTFIT_TYPE_AFTERBURNER);
   O_CMP("fighter bay",    OUTFIT_TYPE_FIGHTER_BAY);
   O_CMP("fighter",        OUTFIT_TYPE_FIGHTER);
   O_CMP("map",            OUTFIT_TYPE_MAP);
   O_CMP("localmap",       OUTFIT_TYPE_LOCALMAP);
   O_CMP("license",        OUTFIT_TYPE_LICENSE);
   O_CMP("gui",            OUTFIT_TYPE_GUI);

   WARN(_("Invalid outfit type: '%s'"),buf);
   return  OUTFIT_TYPE_NULL;
}
#undef O_CMP


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
   char *buf;
   xmlNodePtr cur;

   /* Defaults. */
   dmg->type         = dtype_get("normal");
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
         buf         = xml_get( cur );
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
 *    @param bolt 1 if outfit is a Bolt, 0 if it is an Ammo
 */
static int outfit_loadPLG( Outfit *temp, const char *buf, unsigned int bolt )
{
   char *file;
   CollPoly *polygon;
   xmlDocPtr doc;
   xmlNodePtr node, cur;

   asprintf( &file, "%s%s.xml", OUTFIT_POLYGON_PATH, buf );

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

   if (bolt) {
      do { /* load the polygon data */
         if (xml_isNode(node,"polygons")) {
            cur = node->children;
            temp->u.blt.polygon = array_create_size( CollPoly, 36 );
            do {
               if (xml_isNode(cur,"polygon")) {
                  polygon = &array_grow( &temp->u.blt.polygon );
                  LoadPolygon( polygon, cur );
               }
            } while (xml_nextNode(cur));
         }
      } while (xml_nextNode(node));
   }
   else {
      do { /* Second case: outfit is an ammo */
         if (xml_isNode(node,"polygons")) {
            cur = node->children;
            temp->u.amm.polygon = array_create_size( CollPoly, 36 );
            do {
               if (xml_isNode(cur,"polygon")) {
                  polygon = &array_grow( &temp->u.amm.polygon );
                  LoadPolygon( polygon, cur );
               }
            } while (xml_nextNode(cur));
         }
      } while (xml_nextNode(node));
   }

   xmlFreeDoc(doc);
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
   char *buf;
   double C, area;
   int l;

   /* Defaults */
   temp->u.blt.spfx_armour    = -1;
   temp->u.blt.spfx_shield    = -1;
   temp->u.blt.sound          = -1;
   temp->u.blt.sound_hit      = -1;
   temp->u.blt.falloff        = -1.;
   temp->u.blt.trackmin       = -1.;
   temp->u.blt.trackmax       = -1.;

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
      if (xml_isNode(node,"range")) {
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
         temp->u.blt.gfx_space = xml_parseTexture( node,
               OUTFIT_GFX_PATH"space/%s", 6, 6,
               OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS );
         xmlr_attr_strd(node, "spin", buf);
         if (buf != NULL) {
            outfit_setProp( temp, OUTFIT_PROP_WEAP_SPIN );
            temp->u.blt.spin = atof( buf );
            free(buf);
         }
         /* Load the collision polygon. */
         buf = xml_get(node);
         outfit_loadPLG( temp, buf, 1 );

         /* Validity check: there must be 1 polygon per sprite. */
         if (array_size(temp->u.blt.polygon) != 36) {
            WARN(_("Outfit '%s': the number of collision polygons is wrong.\n \
                    npolygon = %i and sx*sy = %i"),
                    temp->name, array_size(temp->u.blt.polygon), 36);
         }
         continue;
      }
      if (xml_isNode(node,"gfx_end")) {
         temp->u.blt.gfx_end = xml_parseTexture( node,
               OUTFIT_GFX_PATH"space/%s", 6, 6,
               OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS );
         continue;
      }

      /* Special effects. */
      if (xml_isNode(node,"spfx_shield")) {
         temp->u.blt.spfx_shield = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"spfx_armour")) {
         temp->u.blt.spfx_armour = spfx_get(xml_get(node));
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

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD(  l, temp, _("%s [%s]"), _(outfit_getType(temp)),
         _(dtype_damageTypeToStr(temp->u.blt.dmg.type)) );
   SDESC_COND( l, temp, _("\n%.0f CPU"), temp->cpu );
   SDESC_ADD(  l, temp, _("\n%.0f%% Penetration"), temp->u.blt.dmg.penetration*100. );
   SDESC_COND( l, temp, _("\n%.2f DPS [%.0f Damage]"),
         1./temp->u.blt.delay * temp->u.blt.dmg.damage, temp->u.blt.dmg.damage );
   SDESC_COND( l, temp, _("\n%.2f Disable/s [%.0f Disable]"),
         1./temp->u.blt.delay * temp->u.blt.dmg.disable, temp->u.blt.dmg.disable );
   SDESC_ADD(  l, temp, _("\n%.1f Shots Per Second"), 1./temp->u.blt.delay );
   SDESC_COND( l, temp, _("\n%.1f EPS [%.0f Energy]"),
         1./temp->u.blt.delay * temp->u.blt.energy, temp->u.blt.energy );
   SDESC_ADD(  l, temp, _("\n%s Range"), num2strU( temp->u.blt.range, 0 ) );
   SDESC_COND( l, temp, _("\n%.1f second heat up"), temp->u.blt.heatup);
   if (!outfit_isTurret(temp))
      SDESC_ADD(  l, temp, _("\n%.1f Degree Swivel"), temp->u.blt.swivel*180./M_PI );
   SDESC_ADD(  l, temp, _("\n%s Optimal Tracking"), num2strU( temp->u.blt.trackmax, 0 ) );
   SDESC_ADD(  l, temp, _("\n%s Minimal Tracking"), num2strU( temp->u.blt.trackmin, 0 ) );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.blt.gfx_space==NULL,"gfx");
   MELEMENT(temp->u.blt.spfx_shield==-1,"spfx_shield");
   MELEMENT(temp->u.blt.spfx_armour==-1,"spfx_armour");
   MELEMENT((sound_disabled!=0) && (temp->u.blt.sound<0),"sound");
   MELEMENT(temp->mass==0.,"mass");
   MELEMENT(temp->u.blt.delay==0,"delay");
   MELEMENT(temp->u.blt.speed==0,"speed");
   MELEMENT(temp->u.blt.range==0,"range");
   MELEMENT(temp->u.blt.dmg.damage==0,"damage");
   MELEMENT(temp->u.blt.energy==0.,"energy");
   MELEMENT(temp->cpu==0.,"cpu");
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
   char *shader;

   /* Defaults. */
   temp->u.bem.spfx_armour = -1;
   temp->u.bem.spfx_shield = -1;
   temp->u.bem.sound_warmup = -1;
   temp->u.bem.sound = -1;
   temp->u.bem.sound_off = -1;

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

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD(  l, temp, "%s", _(outfit_getType(temp)) );
   SDESC_COND( l, temp, _("\n%.0f CPU"), temp->cpu );
   SDESC_ADD(  l, temp, _("\n%.0f%% Penetration"), temp->u.bem.dmg.penetration*100 );
   SDESC_ADD(  l, temp, _("\n%.2f DPS [%s]"),
         temp->u.bem.dmg.damage * temp->u.bem.duration / (temp->u.bem.duration + temp->u.bem.delay),
         _(dtype_damageTypeToStr(temp->u.bem.dmg.type) ) );
   SDESC_COND( l, temp, _("\n%.0f Disable/s"),  temp->u.bem.dmg.disable );
   SDESC_COND( l, temp, _("\n%.1f EPS"),        temp->u.bem.energy );
   SDESC_ADD(  l, temp, _("\n%.1f Duration"),   temp->u.bem.duration );
   SDESC_ADD(  l, temp, _(" (%.1f minimum)"),   temp->u.bem.min_duration );
   SDESC_ADD(  l, temp, _("\n%.1f Cooldown"),   temp->u.bem.duration );
   SDESC_ADD(  l, temp, _("\n%s Range"),        num2strU(temp->u.bem.range,0) );
   SDESC_COND( l, temp, _("\n%.1f second heat up"),temp->u.bem.heatup );
   if (!outfit_isTurret(temp))
      SDESC_ADD(  l, temp, _("\n%.1f Degree Swivel"), temp->u.bem.swivel*180./M_PI );

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

   temp->u.lau.trackmin = -1.;
   temp->u.lau.trackmax = -1.;

   node  = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      xmlr_float(node,"delay",temp->u.lau.delay);
      xmlr_strd(node,"ammo",temp->u.lau.ammo_name);
      xmlr_int(node,"amount",temp->u.lau.amount);
      xmlr_int(node,"reload_time",temp->u.lau.reload_time);
      xmlr_float(node,"trackmin",temp->u.lau.trackmin);
      xmlr_float(node,"trackmax",temp->u.lau.trackmax);
      xmlr_float(node,"lockon",temp->u.lau.lockon);
      xmlr_float(node,"swivel",temp->u.lau.swivel);
      if (!outfit_isTurret(temp))
         xmlr_float(node,"arc",temp->u.lau.arc); /* This is in semi-arc like swivel. */

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
   temp->u.lau.swivel *= M_PI/180.;
   temp->u.lau.arc *= M_PI/180.;
   /* Note that arc will be 0. for turrets. */

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.lau.ammo_name==NULL,"ammo");
   MELEMENT(temp->u.lau.delay==0.,"delay");
   MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.lau.amount==0.,"amount");
   MELEMENT(temp->u.lau.reload_time==0.,"reload_time");
#undef MELEMENT
}


/**
 * @brief Parses the specific area for a weapon and loads it into Outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   char *buf;

   node = parent->xmlChildrenNode;

   /* Defaults. */
   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;
   temp->u.amm.spfx_armour = -1;
   temp->u.amm.spfx_shield = -1;
   temp->u.amm.sound       = -1;
   temp->u.amm.sound_hit   = -1;
   temp->u.amm.trail_spec  = NULL;
   temp->u.amm.ai          = -1;

   do { /* load all the data */
      xml_onlyNodes(node);
      /* Basic */
      if (xml_isNode(node,"duration")) {
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
         temp->u.amm.duration = xml_getFloat(node);
         continue;
      }
      xmlr_float(node,"resist",temp->u.amm.resist);
      /* Movement */
      xmlr_float(node,"thrust",temp->u.amm.thrust);
      xmlr_float(node,"turn",temp->u.amm.turn);
      xmlr_float(node,"speed",temp->u.amm.speed);
      xmlr_float(node,"energy",temp->u.amm.energy);
      if (xml_isNode(node,"gfx")) {
         temp->u.amm.gfx_space = xml_parseTexture( node,
               OUTFIT_GFX_PATH"space/%s", 6, 6,
               OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS );
         xmlr_attr_float(node, "spin", temp->u.amm.spin);
         if (temp->u.amm.spin != 0)
            outfit_setProp( temp, OUTFIT_PROP_WEAP_SPIN );
         /* Load the collision polygon. */
         buf = xml_get(node);
         outfit_loadPLG( temp, buf, 0 );

         /* Validity check: there must be 1 polygon per sprite. */
         if (array_size(temp->u.amm.polygon) != 36) {
            WARN(_("Outfit '%s': the number of collision polygons is wrong.\n \
                    npolygon = %i and sx*sy = %i"),
                    temp->name, array_size(temp->u.amm.polygon), 36);
         }
         continue;
      }
      if (xml_isNode(node,"spfx_armour")) {
         temp->u.amm.spfx_armour = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"spfx_shield")) {
         temp->u.amm.spfx_shield = spfx_get(xml_get(node));
         continue;
      }
      if (xml_isNode(node,"sound")) {
         temp->u.amm.sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"sound_hit")) {
         temp->u.amm.sound_hit = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"damage")) {
         outfit_parseDamage( &temp->u.amm.dmg, node );
         continue;
      }
      if (xml_isNode(node,"trail_generator")) {
         xmlr_attr_float( node, "x", temp->u.amm.trail_x_offset );
         buf = xml_get(node);
         if (buf == NULL)
            buf = "default";
         temp->u.amm.trail_spec = trailSpec_get( buf );
         continue;
      }
      if (xml_isNode(node,"ai")) {
         buf = xml_get(node);
         if (buf != NULL) {
            if (strcmp(buf,"unguided")==0)
               temp->u.amm.ai = AMMO_AI_UNGUIDED;
            else if (strcmp(buf,"seek")==0)
               temp->u.amm.ai = AMMO_AI_SEEK;
            else if (strcmp(buf,"smart")==0)
               temp->u.amm.ai = AMMO_AI_SMART;
            else
               WARN(_("Ammo '%s' has unknown ai type '%s'."), temp->name, buf);
         }
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post-processing */
   temp->u.amm.turn *= M_PI/180.; /* Convert to rad/s. */

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   temp->desc_short[0]='\0';

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->mass==0.,"mass");
   MELEMENT(temp->u.amm.gfx_space==NULL,"gfx");
   MELEMENT(temp->u.amm.spfx_shield==-1,"spfx_shield");
   MELEMENT(temp->u.amm.spfx_armour==-1,"spfx_armour");
   MELEMENT((sound_disabled!=0) && (temp->u.amm.sound<0),"sound");
   /* MELEMENT(temp->u.amm.thrust==0,"thrust"); */
   /* Unguided missiles don't need everything */
   if (outfit_isSeeker(temp)) {
      MELEMENT(temp->u.amm.turn==0,"turn");
   }
   MELEMENT(temp->u.amm.speed==0,"speed");
   MELEMENT(temp->u.amm.duration==0,"duration");
   MELEMENT(temp->u.amm.dmg.damage==0,"damage");
   /*MELEMENT(temp->u.amm.energy==0.,"energy");*/
   MELEMENT(temp->cpu!=0.,"cpu");
#undef MELEMENT
}


/**
 * @brief Parses the modification tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent )
{
   int i;
   xmlNodePtr node;
   ShipStatList *ll;
   node = parent->children;

   /* Defaults. */
   temp->u.mod.lua_env = LUA_NOREF;
   temp->u.mod.lua_init = LUA_NOREF;
   temp->u.mod.lua_cleanup = LUA_NOREF;
   temp->u.mod.lua_update = LUA_NOREF;
   temp->u.mod.lua_ontoggle = LUA_NOREF;
   temp->u.mod.lua_onhit = LUA_NOREF;
   temp->u.mod.lua_outofenergy = LUA_NOREF;
   temp->u.mod.lua_cooldown = LUA_NOREF;

   do { /* load all the data */
      xml_onlyNodes(node);
      if (xml_isNode(node,"active")) {
         xmlr_attr_float(node, "cooldown", temp->u.mod.cooldown);
         temp->u.mod.active   = 1;
         temp->u.mod.duration = xml_getFloat(node);

         /* Infinity if no duration specified. */
         if (temp->u.mod.duration == 0)
            temp->u.mod.duration = INFINITY;

         continue;
      }
      /* Lua stuff. */
      if (xml_isNode(node,"lua")) {
         nlua_env env;
         size_t sz;
         char *dat = ndata_read( xml_get(node), &sz );
         if (dat==NULL) {
            WARN(_("Outfit '%s' failed to read Lua '%s'!"), temp->name, xml_get(node) );
            continue;
         }

         env = nlua_newEnv(1);
         temp->u.mod.lua_env = env;
         /* TODO limit libraries here. */
         nlua_loadStandard( env );
         nlua_loadGFX( env );
         nlua_loadPilotOutfit( env );

         /* Run code. */
         if (nlua_dobufenv( env, dat, sz, xml_get(node) ) != 0) {
            WARN(_("Outfit '%s' Lua error:\n%s"), temp->name, lua_tostring(naevL,-1));
            lua_pop(naevL,1);
            nlua_freeEnv( temp->u.mod.lua_env );
            free( dat );
            temp->u.mod.lua_env = LUA_NOREF;
            continue;
         }
         free( dat );

         /* Check functions as necessary. */
         temp->u.mod.lua_init = nlua_refenvtype( env, "init", LUA_TFUNCTION );
         temp->u.mod.lua_cleanup = nlua_refenvtype( env, "cleanup", LUA_TFUNCTION );
         temp->u.mod.lua_update = nlua_refenvtype( env, "update", LUA_TFUNCTION );
         temp->u.mod.lua_ontoggle = nlua_refenvtype( env, "ontoggle", LUA_TFUNCTION );
         temp->u.mod.lua_onhit = nlua_refenvtype( env, "onhit", LUA_TFUNCTION );
         temp->u.mod.lua_outofenergy = nlua_refenvtype( env, "outofenergy", LUA_TFUNCTION );
         temp->u.mod.lua_cooldown = nlua_refenvtype( env, "cooldown", LUA_TFUNCTION );
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

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   i = scnprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
         "%s"
         "%s",
         _(outfit_getType(temp)),
         (temp->u.mod.active || temp->u.mod.lua_ontoggle != LUA_NOREF) ? _("\n#rActivated Outfit#0") : "" );

#define DESC_ADD(x, s) \
if ((x) != 0) \
   do { \
      i += scnprintf( &temp->desc_short[i], OUTFIT_SHORTDESC_MAX-i, "\n#%c", ((x)>0)?'g':'r' ); \
      i += scnprintf( &temp->desc_short[i], OUTFIT_SHORTDESC_MAX-i, s, x ); \
      i += scnprintf( &temp->desc_short[i], OUTFIT_SHORTDESC_MAX-i, "#0" ); \
   } while(0)
   DESC_ADD( temp->cpu,                _("%+.0f CPU") );
#undef DESC_ADD
}


/**
 * @brief Parses the afterburner tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent )
{
   ShipStatList *ll;
   xmlNodePtr node;
   node = parent->children;
   double C, area;

   /* Defaults. */
   temp->u.afb.sound = -1;
   temp->u.afb.sound_on = -1;
   temp->u.afb.sound_off = -1;

   /* must be >= 1. */
   temp->u.afb.thrust = 1.;
   temp->u.afb.speed  = 1.;

   do { /* parse the data */
      xml_onlyNodes(node);
      xmlr_float(node,"rumble",temp->u.afb.rumble);
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
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
         _("%s\n"
         "#rActivated Outfit#0\n"
         "%.0f CPU\n"
         "Only one can be equipped\n"
         "%.0f Maximum Effective Mass\n"
         "%.0f%% Thrust\n"
         "%.0f%% Maximum Speed\n"
         "%.1f EPS\n"
         "%.1f Rumble"),
         _(outfit_getType(temp)),
         temp->cpu,
         temp->u.afb.mass_limit,
         temp->u.afb.thrust + 100.,
         temp->u.afb.speed + 100.,
         temp->u.afb.energy,
         temp->u.afb.rumble );

   /* Post processing. */
   temp->u.afb.thrust /= 100.;
   temp->u.afb.speed  /= 100.;
   C = pilot_heatCalcOutfitC(temp);
   area = pilot_heatCalcOutfitArea(temp);
   temp->u.afb.heat    = ((800.-CONST_SPACE_STAR_TEMP)*C +
            STEEL_HEAT_CONDUCTIVITY * ((800-CONST_SPACE_STAR_TEMP) * area)) /
         temp->u.afb.heatup;

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

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
   ShipStatList *ll;
   xmlNodePtr node;
   node = parent->children;

   do {
      xml_onlyNodes(node);
      xmlr_int(node,"delay",temp->u.bay.delay);
      xmlr_int(node,"reload_time",temp->u.bay.reload_time);
      xmlr_strd(node,"ammo",temp->u.bay.ammo_name);
      xmlr_int(node,"amount",temp->u.bay.amount);

      /* Stats. */
      ll = ss_listFromXML( node );
      if (ll != NULL) {
         ll->next    = temp->stats;
         temp->stats = ll;
         continue;
      }
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set default outfit size if necessary. */
   if (temp->slot.size == OUTFIT_SLOT_SIZE_NA)
      outfit_setDefaultSize( temp );

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
         _("%s\n"
         "%.0f CPU\n"
         "%.1f Seconds Per Launch\n"
         "Holds %d %s\n"
         "%.1f Seconds to Reload"),
         _(outfit_getType(temp)),
         temp->cpu, temp->u.bay.delay,
         temp->u.bay.amount, _(temp->u.bay.ammo_name),
         temp->u.bay.reload_time);

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
   MELEMENT(temp->u.bay.delay==0,"delay");
   MELEMENT(temp->u.bay.reload_time==0.,"reload_time");
   MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->u.bay.ammo_name==NULL,"ammo");
   MELEMENT(temp->u.bay.amount==0,"amount");
#undef MELEMENT
}

/**
 * @brief Parses the fighter tidbits of the outfit.
 *
 *    @param temp Outfit to finish loading.
 *    @param parent Outfit's parent node.
 */
static void outfit_parseSFighter( Outfit *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   node = parent->children;

   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   do {
      xml_onlyNodes(node);
      xmlr_strd(node,"ship",temp->u.fig.ship);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
         "%s",
         _(outfit_getType(temp)) );

#define MELEMENT(o,s) \
if (o) WARN(_("Outfit '%s' missing/invalid '%s' element"), temp->name, s)
/**< Define to help check for data errors. */
   MELEMENT(temp->u.fig.ship==NULL,"ship");
   MELEMENT(temp->cpu!=0.,"cpu");
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
   int i, j;
   xmlNodePtr node, cur;
   char *buf;
   StarSystem *sys, *system_stack;
   Planet *asset;
   JumpPoint *jump;

   node = parent->children;

   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   temp->u.map->systems = array_create(StarSystem*);
   temp->u.map->assets  = array_create(Planet*);
   temp->u.map->jumps   = array_create(JumpPoint*);

   do {
      xml_onlyNodes(node);

      if (xml_isNode(node,"sys")) {
         xmlr_attr_strd(node,"name",buf);
         sys = system_get(buf);
         if (sys != NULL) {
            free(buf);
            array_grow( &temp->u.map->systems ) = sys;

            cur = node->children;

            do {
               xml_onlyNodes(cur);

               if (xml_isNode(cur,"asset")) {
                  buf = xml_get(cur);
                  if ((buf != NULL) && ((asset = planet_get(buf)) != NULL))
                     array_grow( &temp->u.map->assets ) = asset;
                  else
                     WARN(_("Map '%s' has invalid asset '%s'"), temp->name, buf);
               }
               else if (xml_isNode(cur,"jump")) {
                  buf = xml_get(cur);
                  if ((buf != NULL) && ((jump = jump_get(xml_get(cur),
                        temp->u.map->systems[array_size(temp->u.map->systems)-1] )) != NULL))
                     array_grow( &temp->u.map->jumps ) = jump;
                  else
                     WARN(_("Map '%s' has invalid jump point '%s'"), temp->name, buf);
               }
               else
                  WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, cur->name);
            } while (xml_nextNode(cur));
         }
         else {
            WARN(_("Map '%s' has invalid system '%s'"), temp->name, buf);
            free(buf);
         }
      }
      else if (xml_isNode(node,"short_desc")) {
         temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
         snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX, "%s", xml_get(node) );
      }
      else if (xml_isNode(node,"all")) { /* Add everything to the map */
         system_stack = system_getAll();
         for (i=0;i<array_size(system_stack);i++) {
            array_grow( &temp->u.map->systems ) = &system_stack[i];
            for (j=0;j<array_size(system_stack[i].planets);j++)
               array_grow( &temp->u.map->assets ) = system_stack[i].planets[j];
            for (j=0;j<array_size(system_stack[i].jumps);j++)
               array_grow( &temp->u.map->jumps ) = &system_stack[i].jumps[j];
         }
      }
      else
         WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   array_shrink( &temp->u.map->systems );
   array_shrink( &temp->u.map->assets  );
   array_shrink( &temp->u.map->jumps   );

   if (temp->desc_short == NULL) {
      /* Set short description based on type. */
      temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
      snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
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
   xmlNodePtr node;
   node = parent->children;

   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   do {
      xml_onlyNodes(node);
      xmlr_float(node,"asset_detect",temp->u.lmap.asset_detect);
      xmlr_float(node,"jump_detect",temp->u.lmap.jump_detect);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   temp->u.lmap.asset_detect = pow2( temp->u.lmap.asset_detect );
   temp->u.lmap.jump_detect  = pow2( temp->u.lmap.jump_detect );

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
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
   xmlNodePtr node;

   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   node = parent->children;

   do {
      xml_onlyNodes(node);
      xmlr_strd(node,"gui",temp->u.gui.gui);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
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
   temp->slot.type         = OUTFIT_SLOT_NA;
   temp->slot.size         = OUTFIT_SLOT_SIZE_NA;

   xmlNodePtr node;
   node = parent->children;

   do {
      xml_onlyNodes(node);
      WARN(_("Outfit '%s' has unknown node '%s'"),temp->name, node->name);
   } while (xml_nextNode(node));

   /* Set short description. */
   temp->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   snprintf( temp->desc_short, OUTFIT_SHORTDESC_MAX,
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
   xmlNodePtr cur, ccur, node, parent;
   char *prop, *desc_extra;
   const char *cprop;
   int group, l;
   ShipStatList *ll;

   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first system node */
   if (parent == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), OUTFIT_DATA_PATH );
      return -1;
   }

   /* Clear data. */
   memset( temp, 0, sizeof(Outfit) );
   desc_extra = NULL;

   xmlr_attr_strd(parent,"name",temp->name);
   if (temp->name == NULL)
      WARN(_("Outfit in %s has invalid or no name"), OUTFIT_DATA_PATH);

   node = parent->xmlChildrenNode;

   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"general")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_int(cur,"rarity",temp->rarity);
            xmlr_strd(cur,"license",temp->license);
            xmlr_float(cur,"mass",temp->mass);
            xmlr_float(cur,"cpu",temp->cpu);
            xmlr_long(cur,"price",temp->price);
            xmlr_strd(cur,"limit",temp->limit);
            xmlr_strd(cur,"description",temp->description);
            xmlr_strd(cur,"desc_extra",desc_extra);
            xmlr_strd(cur,"typename",temp->typename);
            xmlr_int(cur,"priority",temp->priority);
            if (xml_isNode(cur,"unique")) {
               outfit_setProp(temp, OUTFIT_PROP_UNIQUE);
               continue;
            }
            else if (xml_isNode(cur,"gfx_store")) {
               temp->gfx_store = xml_parseTexture( cur,
                     OUTFIT_GFX_PATH"store/%s", 1, 1, OPENGL_TEX_MIPMAPS );
               continue;
            }
            else if (xml_isNode(cur,"gfx_overlays")) {
               ccur = cur->children;
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
            WARN(_("Outfit '%s' has unknown general node '%s'"),temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"stats")) {
         cur = node->children;
         do {
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

            temp->group = CLAMP(0, 9, group - 1);
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
         else if (outfit_isAmmo(temp))
            outfit_parseSAmmo( temp, node );
         else if (outfit_isMod(temp))
            outfit_parseSMod( temp, node );
         else if (outfit_isAfterburner(temp))
            outfit_parseSAfterburner( temp, node );
         else if (outfit_isFighterBay(temp))
            outfit_parseSFighterBay( temp, node );
         else if (outfit_isFighter(temp))
            outfit_parseSFighter( temp, node );
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

         /* We add the ship stats to the description here. */
         if (temp->desc_short) {
            l = strlen(temp->desc_short);
            ss_statsListDesc( temp->stats, &temp->desc_short[l], OUTFIT_SHORTDESC_MAX-l, 1 );
            /* Add extra description task if available. */
            if (desc_extra != NULL) {
               l = strlen(temp->desc_short);
               snprintf( &temp->desc_short[l], OUTFIT_SHORTDESC_MAX-l, "\n%s", desc_extra );
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
   MELEMENT(temp->slot.type==OUTFIT_SLOT_NULL,"slot");
   MELEMENT((temp->slot.type!=OUTFIT_SLOT_NA) && (temp->slot.size==OUTFIT_SLOT_SIZE_NA),"size");
   MELEMENT(temp->gfx_store==NULL,"gfx_store");
   /*MELEMENT(temp->mass==0,"mass"); Not really needed */
   MELEMENT(temp->type==0,"type");
   /*MELEMENT(temp->price==0,"price");*/
   MELEMENT(temp->description==NULL,"description");
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
   int i, n, ret;
   char **outfit_files;

   outfit_files = ndata_listRecursive( dir );
   for ( i = 0; i < array_size( outfit_files ); i++ ) {
      if (!ndata_matchExt( outfit_files[i], "xml" ))
         continue;

      ret = outfit_parse( &array_grow(&outfit_stack), outfit_files[i] );
      if (ret < 0) {
         n = array_size(outfit_stack);
         array_erase( &outfit_stack, &outfit_stack[n-1], &outfit_stack[n] );
      }
      free( outfit_files[i] );
   }
   array_free( outfit_files );

   /* Reduce size. */
   array_shrink( &outfit_stack );

   return 0;
}

/**
 * @brief Loads all the outfits.
 *
 *    @return 0 on success.
 */
int outfit_load (void)
{
   int i, noutfits;
   Outfit *o;

   /* First pass, loads up ammunition. */
   outfit_stack = array_create(Outfit);
   outfit_loadDir( OUTFIT_DATA_PATH );
   array_shrink(&outfit_stack);
   noutfits = array_size(outfit_stack);

   /* Second pass, sets up ammunition relationships. */
   for (i=0; i<noutfits; i++) {
      o = &outfit_stack[i];
      if (outfit_isLauncher(&outfit_stack[i])) {
         o->u.lau.ammo = outfit_get( o->u.lau.ammo_name );
         if (outfit_isSeeker(o) && /* Smart seekers. */
               (o->u.lau.ammo->u.amm.ai)) {
            if (o->u.lau.trackmin < 0.)
               WARN(_("Outfit '%s' missing/invalid 'trackmin' element"), o->name);
            if (o->u.lau.trackmax < 0.)
               WARN(_("Outfit '%s' missing/invalid 'trackmax' element"), o->name);
            if (o->u.lau.lockon == 0.)
               WARN(_("Outfit '%s' missing/invalid 'lockon' element"), o->name);
            if (!outfit_isTurret(o) && (o->u.lau.arc == 0.))
               WARN(_("Outfit '%s' missing/invalid 'arc' element"), o->name);
         }

         outfit_launcherDesc(o);
      }
      else if (outfit_isFighterBay(&outfit_stack[i]))
         o->u.bay.ammo = outfit_get( o->u.bay.ammo_name );
   }

#ifdef DEBUGGING
   char **outfit_names = malloc( noutfits * sizeof(char*) );
   int start;

   for (i=0; i<noutfits; i++)
      outfit_names[i] = outfit_stack[i].name;

   qsort( outfit_names, noutfits, sizeof(char*), strsort );
   for (i=0; i<(noutfits - 1); i++) {
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
#endif

   DEBUG( n_( "Loaded %d Outfit", "Loaded %d Outfits", noutfits ), noutfits );

   return 0;
}


/**
 * @brief Parses all the maps.
 *
 */
int outfit_mapParse (void)
{
   Outfit *o;
   size_t i;
   xmlNodePtr node, cur;
   xmlDocPtr doc;
   char **map_files;
   char *file, *n;

   map_files = PHYSFS_enumerateFiles( MAP_DATA_PATH );
   for (i=0; map_files[i]!=NULL; i++) {
      asprintf( &file, "%s%s", MAP_DATA_PATH, map_files[i] );

      doc = xml_parsePhysFS( file );
      if (doc == NULL) {
         WARN(_("%s file is invalid xml!"), file);
         free(file);
         continue;
      }

      node = doc->xmlChildrenNode; /* first system node */
      if (node == NULL) {
         WARN( _("Malformed '%s' file: does not contain elements"), OUTFIT_DATA_PATH );
         free(file);
         xmlFreeDoc(doc);
         continue;
      }

      xmlr_attr_strd( node, "name", n );
      o = outfit_get( n );
      free(n);
      if (!outfit_isMap(o)) { /* If its not a map, we don't care. */
         free(file);
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
      free(file);
      xmlFreeDoc(doc);
   }

   /* Clean up. */
   PHYSFS_freeList( map_files );

   return 0;
}


/**
 * @brief Generates short descs for launchers, including ammo info.
 *
 *    @param o Launcher.
 */
static void outfit_launcherDesc( Outfit* o )
{
   int l;
   Outfit *a; /* Launcher's ammo. */

   if (o->desc_short != NULL) {
      WARN(_("Outfit '%s' already has a short description"), o->name);
      return;
   }

   a = o->u.lau.ammo;

   o->desc_short = malloc( OUTFIT_SHORTDESC_MAX );
   l = 0;
   SDESC_ADD(  l, o, _("%s [%s]"), _(outfit_getType(o)),
         _(dtype_damageTypeToStr(a->u.amm.dmg.type)) );
   SDESC_COND( l, o, _("\n%.0f CPU"), o->cpu );

   if (outfit_isSeeker(o)) {
      SDESC_ADD(  l, o, _("\n%.1f Second Lock-on"), o->u.lau.lockon );
      SDESC_ADD(  l, o, _("\n%s Optimal Tracking"), num2strU( o->u.lau.trackmax, 0 ) );
      SDESC_ADD(  l, o, _("\n%s Minimal Tracking"), num2strU( o->u.lau.trackmin, 0 ) );
   }
   else {
      SDESC_ADD(  l, o, _("\nNo Seeking") );
      if (outfit_isTurret(o) || o->u.lau.swivel > 0.) {
         SDESC_ADD(  l, o, _("\n%s Optimal Tracking"), num2strU( o->u.lau.trackmax, 0 ) );
         SDESC_ADD(  l, o, _("\n%s Minimal Tracking"), num2strU( o->u.lau.trackmin, 0 ) );
         SDESC_COND( l, o, _("\n%.1f Degree Swivel"), o->u.lau.swivel*180./M_PI );
      }
   }

   SDESC_ADD(  l, o, _("\nHolds %d %s:"), o->u.lau.amount, _(o->u.lau.ammo_name) );
   SDESC_ADD(  l, o, _("\n%.0f%% Penetration"), a->u.amm.dmg.penetration * 100. );
   SDESC_COND( l, o, _("\n%.2f DPS [%.0f Damage]"),
         1. / o->u.lau.delay * a->u.amm.dmg.damage, a->u.amm.dmg.damage );
   SDESC_COND( l, o, _("\n%.1f Disable/s [%.0f Disable]"),
         1. / o->u.lau.delay * a->u.amm.dmg.disable, a->u.amm.dmg.disable );
   SDESC_ADD(  l, o, _("\n%.1f Shots Per Second"), 1. / o->u.lau.delay );
   SDESC_ADD(  l, o, _("\n%s Range [%.1f duration]"), num2strU( outfit_range(a), 0 ), a->u.amm.duration );
   SDESC_ADD(  l, o, _("\n%.0f Maximum Speed"), a->u.amm.speed );
   SDESC_ADD(  l, o, _("\n%.1f Seconds to Reload"), o->u.lau.reload_time );
   SDESC_COND( l, o, _("\n%.1f EPS [%.0f Energy]"), o->u.lau.delay * a->u.amm.energy, a->u.amm.energy );
   SDESC_COND( l, o, _("\n%.1f%% Jam Resistance"), (1. - 0.5 / a->u.amm.resist) * 100.);
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
 * @brief Frees the outfit stack.
 */
void outfit_free (void)
{
   int i, j;
   Outfit *o;

   for (i=0; i < array_size(outfit_stack); i++) {
      o = &outfit_stack[i];

      /* Free graphics */
      gl_freeTexture(outfit_gfx(o));

      /* Free slot. */
      outfit_freeSlot( &outfit_stack[i].slot );

      /* Free stats. */
      ss_free( o->stats );

      if (outfit_isAmmo(o)) {
         /* Free collision polygons. */
         for (j=0; j<array_size(o->u.amm.polygon); j++) {
            free(o->u.amm.polygon[j].x);
            free(o->u.amm.polygon[j].y);
         }
         array_free(o->u.amm.polygon);
      }
      /* Type specific. */
      if (outfit_isBolt(o)) {
         gl_freeTexture(o->u.blt.gfx_end);
         /* Free collision polygons. */
         for (j=0; j<array_size(o->u.blt.polygon); j++) {
            free(o->u.blt.polygon[j].x);
            free(o->u.blt.polygon[j].y);
         }
         array_free(o->u.blt.polygon);
      }
      if (outfit_isLauncher(o))
         free(o->u.lau.ammo_name);
      if (outfit_isFighterBay(o))
         free(o->u.bay.ammo_name);
      if (outfit_isFighter(o))
         free(o->u.fig.ship);
      if (outfit_isGUI(o))
         free(o->u.gui.gui);
      if (outfit_isMap(o)) {
         array_free( o->u.map->systems );
         array_free( o->u.map->assets );
         array_free( o->u.map->jumps );
         free( o->u.map );
      }
      if (outfit_isMod(o)) {
         if (o->u.mod.lua_env != LUA_NOREF)
            nlua_freeEnv( o->u.mod.lua_env );
         o->u.mod.lua_env = LUA_NOREF;
      }

      /* strings */
      free(o->typename);
      free(o->description);
      free(o->limit);
      free(o->desc_short);
      free(o->license);
      free(o->name);
      gl_freeTexture(o->gfx_store);
      for (j=0; j<array_size(o->gfx_overlays); j++)
         gl_freeTexture(o->gfx_overlays[j]);
      array_free(o->gfx_overlays);
   }

   array_free(outfit_stack);
}

