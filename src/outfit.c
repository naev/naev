/*
 * See Licensing and Copyright notice in naev.h
 */



#include "outfit.h"

#include <math.h>
#include <string.h>

#include "xml.h"
#include "SDL_thread.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "spfx.h"


#define outfit_setProp(o,p)      ((o)->properties |= p)


#define XML_OUTFIT_ID      "Outfits"   /* XML section identifier */
#define XML_OUTFIT_TAG     "outfit"

#define OUTFIT_DATA  "dat/outfit.xml"
#define OUTFIT_GFX   "gfx/outfit/"


extern SDL_mutex* sound_lock; /* from sound.c */

/*
 * the stack
 */
static Outfit* outfit_stack = NULL;
static int outfits = 0;


/*
 * Prototypes
 */
/* misc */
static DamageType outfit_strToDamageType( char *buf );
/* parsing */
static int outfit_parseDamage( DamageType *dtype, double *dmg, xmlNodePtr node );
static Outfit* outfit_parse( const xmlNodePtr parent );
static void outfit_parseSWeapon( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent );


/*
 * returns the outfit
 */
Outfit* outfit_get( const char* name )
{
   int i;
   for (i=0; i<outfits; i++)
      if (strcmp(name,outfit_stack[i].name)==0)
         return &outfit_stack[i];
   return NULL;
}


/*
 * returns all the outfits
 */
char** outfit_getTech( int *n, const int *tech, const int techmax )
{
   int i, j;
   char **outfitnames = malloc(sizeof(Outfit*) * outfits);

   *n = 0;
   for (i=0; i < outfits; i++)
      if (outfit_stack[i].tech <= tech[0]) {
         outfitnames[*n] = strdup(outfit_stack[i].name);
         (*n)++;
      }
      else {
         for(j=0; j<techmax; j++)
            if (tech[j] ==outfit_stack[i].tech) {
               outfitnames[*n] = strdup(outfit_stack[i].name);
               (*n)++;
            }
      }

   /* actual size is bigger, but it'll just get freed :) */
   return outfitnames;
}


void outfit_calcDamage( double *dshield, double *darmour,
      DamageType dtype, double dmg )
{
   switch (dtype) {
      case DAMAGE_TYPE_ENERGY:
         (*dshield) = dmg*1.1;
         (*darmour) = dmg*0.7;
         break;
      case DAMAGE_TYPE_KINETIC:
         (*dshield) = dmg*0.8;
         (*darmour) = dmg*1.2;
         break;

      default:
         (*dshield) = (*darmour) = 0.;
         break;
   }
}


/*
 * return 1 if o is a weapon (beam/bolt)
 */
int outfit_isWeapon( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_BOLT)      ||
         (o->type==OUTFIT_TYPE_BEAM) );
}
/*
 * return 1 if o is a launcher
 */
int outfit_isLauncher( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_MISSILE_DUMB) ||
         (o->type==OUTFIT_TYPE_MISSILE_SEEK)    ||
         (o->type==OUTFIT_TYPE_MISSILE_SEEK_SMART) ||
         (o->type==OUTFIT_TYPE_MISSILE_SWARM)   ||
         (o->type==OUTFIT_TYPE_MISSILE_SWARM_SMART) );
}
/*
 * return 1 if o is weapon ammunition
 */
int outfit_isAmmo( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_MISSILE_DUMB_AMMO)  ||
         (o->type==OUTFIT_TYPE_MISSILE_SEEK_AMMO)     ||
         (o->type==OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO) ||
         (o->type==OUTFIT_TYPE_MISSILE_SWARM_AMMO)    ||
         (o->type==OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO) );
}
/*
 * return 1 if o is a turret
 */
int outfit_isTurret( const Outfit* o )
{
   return ( (o->type==OUTFIT_TYPE_TURRET_BOLT) ||
         (o->type==OUTFIT_TYPE_TURRET_BEAM) );
}


/*
 * returns 1 if o is a modification
 */
int outfit_isMod( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_MODIFCATION);
}


/*
 * returns 1 if o is an afterburner
 */
int outfit_isAfterburner( const Outfit* o )
{
   return (o->type==OUTFIT_TYPE_AFTERBURNER);
}


/*
 * gets the outfit's gfx
 */
glTexture* outfit_gfx( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.gfx_space;
   else if (outfit_isAmmo(o)) return o->u.amm.gfx_space;
   else if (outfit_isTurret(o)) return o->u.blt.gfx_space;
   return NULL;
}
/*
 * gets the outfit's spfx is applicable
 */
int outfit_spfx( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.spfx;
   else if (outfit_isAmmo(o)) return o->u.amm.spfx;
   else if (outfit_isTurret(o)) return o->u.blt.spfx;
   return -1;
}
double outfit_damage( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.damage;
   else if (outfit_isAmmo(o)) return o->u.amm.damage;
   else if (outfit_isTurret(o)) return o->u.blt.damage;
   return -1.;
}
DamageType outfit_damageType( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.dtype;
   else if (outfit_isAmmo(o)) return o->u.amm.dtype;
   else if (outfit_isTurret(o)) return o->u.blt.dtype;
   return DAMAGE_TYPE_NULL;
}
int outfit_delay( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.delay;
   else if (outfit_isLauncher(o)) return o->u.lau.delay;
   else if (outfit_isTurret(o)) return o->u.blt.delay;
   return -1;
}
double outfit_energy( const Outfit* o )
{
   if (outfit_isWeapon(o)) return o->u.blt.energy;
   else if (outfit_isAmmo(o)) return o->u.amm.energy;
   else if (outfit_isTurret(o)) return o->u.blt.energy;
   return -1.;
}



/*
 * returns the associated name
 */
const char* outfit_typename[] = { 
      "NULL",
      "Bolt Cannon",
      "Beam Cannon",
      "Dumb Missile",
      "Dumb Missile Ammunition",
      "Seeker Missile",
      "Seeker Missile Ammunition",
      "Smart Seeker Missile",
      "Smart Seeker Missile Ammunition",
      "Swarm Missile",
      "Swarm Missile Ammunition Pack",
      "Smart Swarm Missile",
      "Smart Swarm Missile Ammunition Pack",
      "Bolt Turret",
      "Beam Turret",
      "Ship Modification",
      "Afterburner"
};
const char* outfit_getType( const Outfit* o )
{
   return outfit_typename[o->type];
}


/*
 * returns the broad outfit type
 */
const char* outfit_typenamebroad[] = { "NULL",
      "Weapon",
      "Launcher",
      "Ammo",
      "Turret",
      "Modification",
      "Afterburner"
};
const char* outfit_getTypeBroad( const Outfit* o )
{
   int i = 0;
   if (outfit_isWeapon(o)) i = 1;
   else if (outfit_isLauncher(o)) i = 2;
   else if (outfit_isAmmo(o)) i = 3;
   else if (outfit_isTurret(o)) i = 4;
   else if (outfit_isMod(o)) i = 5;
   else if (outfit_isAfterburner(o)) i = 6;

   return outfit_typenamebroad[i];
}


/*
 * returns the damage type from an str
 */
static DamageType outfit_strToDamageType( char *buf )
{
   if (strcmp(buf,"energy")==0) return DAMAGE_TYPE_ENERGY;
   else if (strcmp(buf,"kinetic")==0) return DAMAGE_TYPE_KINETIC;

   WARN("Invalid damage type: '%s'", buf);
   return DAMAGE_TYPE_NULL;
}


/*
 * parses a damage node: <damage type="kinetic">10</damage>
 */
static int outfit_parseDamage( DamageType *dtype, double *dmg, xmlNodePtr node )
{
   char *buf;

   if (xml_isNode(node,"damage")) {
   xmlr_attr(node,"type",buf);
   (*dtype) = outfit_strToDamageType(buf);
   if (buf) free(buf);
   (*dmg) = xml_getFloat(node);

      return 0;
   }

   (*dtype) = DAMAGE_TYPE_NULL;
   (*dmg) = 0;
   WARN("Trying to parse non-damage node as damage node!");
   return 1;
}


/*
 * parses the specific area for a weapon and loads it into Outfit
 */
static void outfit_parseSWeapon( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   char str[PATH_MAX] = "\0";

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      xmlr_float(node,"speed",temp->u.blt.speed);
      xmlr_float(node,"delay",temp->u.blt.delay);
      xmlr_float(node,"range",temp->u.blt.range);
      xmlr_float(node,"accuracy",temp->u.blt.accuracy);
      xmlr_float(node,"energy",temp->u.blt.energy);

      if (xml_isNode(node,"gfx")) {
         snprintf( str, strlen(xml_get(node))+sizeof(OUTFIT_GFX)+10,
               OUTFIT_GFX"space/%s.png", xml_get(node));
         temp->u.blt.gfx_space = gl_newSprite(str, 6, 6);
      }
      else if (xml_isNode(node,"spfx"))
         temp->u.blt.spfx = spfx_get(xml_get(node));
      else if (xml_isNode(node,"sound"))
         temp->u.blt.sound = sound_get( xml_get(node) );
      else if (xml_isNode(node,"damage"))
         outfit_parseDamage( &temp->u.blt.dtype, &temp->u.blt.damage, node );
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN("Outfit '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->u.blt.gfx_space==NULL,"gfx");
   MELEMENT((sound_lock!=NULL) && (temp->u.blt.sound==0),"sound");
   MELEMENT(temp->u.blt.delay==0,"delay");
   MELEMENT(temp->u.blt.speed==0,"speed");
   MELEMENT(temp->u.blt.range==0,"range");
   MELEMENT(temp->u.blt.accuracy==0,"accuracy");
   MELEMENT(temp->u.blt.damage==0,"damage");
#undef MELEMENT
}


/*
 * parses the specific area for a launcher and loads it into Outfit
 */
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   node  = parent->xmlChildrenNode;


   do { /* load all the data */
      if (xml_isNode(node,"delay")) temp->u.lau.delay = xml_getInt(node);
      else if (xml_isNode(node,"ammo")) temp->u.lau.ammo = strdup(xml_get(node));
   } while (xml_nextNode(node));

#define MELEMENT(o,s)      if (o) WARN("Outfit '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->u.lau.ammo==NULL,"ammo");
   MELEMENT(temp->u.lau.delay==0,"delay");
#undef MELEMENT
}


/*
 * parses the specific area for a weapon and loads it into Outfit
 */
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   node = parent->xmlChildrenNode;

   char str[PATH_MAX] = "\0";

   do { /* load all the data */
      xmlr_float(node,"thrust",temp->u.amm.thrust);
      xmlr_float(node,"turn",temp->u.amm.turn);
      xmlr_float(node,"speed",temp->u.amm.speed);
      xmlr_float(node,"energy",temp->u.amm.energy);
      if (xml_isNode(node,"duration"))
         temp->u.amm.duration = (unsigned int)1000.*xml_getFloat(node);
      else if (xml_isNode(node,"lockon"))
         temp->u.amm.lockon = (unsigned int)1000.*xml_getFloat(node);
      else if (xml_isNode(node,"gfx")) {
         snprintf( str, strlen(xml_get(node))+sizeof(OUTFIT_GFX)+10,
               OUTFIT_GFX"space/%s.png", xml_get(node));
         temp->u.amm.gfx_space = gl_newSprite(str, 6, 6);
      }
      else if (xml_isNode(node,"spfx"))
         temp->u.amm.spfx = spfx_get(xml_get(node));
      else if (xml_isNode(node,"sound"))
         temp->u.amm.sound = sound_get( xml_get(node) );
      else if (xml_isNode(node,"damage"))
         outfit_parseDamage( &temp->u.amm.dtype, &temp->u.amm.damage, node );
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN("Outfit '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->u.amm.gfx_space==NULL,"gfx");
   MELEMENT((sound_lock!=NULL) && (temp->u.amm.sound==0),"sound");
   MELEMENT(temp->u.amm.thrust==0,"thrust");
   MELEMENT(temp->u.amm.turn==0,"turn");
   MELEMENT(temp->u.amm.speed==0,"speed");
   MELEMENT(temp->u.amm.duration==0,"duration");
   MELEMENT(temp->u.amm.lockon==0,"lockon");
   MELEMENT(temp->u.amm.damage==0,"damage");
#undef MELEMENT
}


/*
 * parses the modification tidbits of the outfit
 */
static void outfit_parseSMod( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   node = parent->children;

   do { /* load all the data */
      /* movement */
      xmlr_float(node,"thrust",temp->u.mod.thrust);
      xmlr_float(node,"turn",temp->u.mod.turn);
      xmlr_float(node,"speed",temp->u.mod.speed);
      /* health */
      xmlr_float(node,"armour",temp->u.mod.armour);
      xmlr_float(node,"shield",temp->u.mod.shield);
      xmlr_float(node,"energy",temp->u.mod.energy);
      xmlr_float(node,"fuel",temp->u.mod.fuel);
      if (xml_isNode(node,"armour_regen"))
         temp->u.mod.armour_regen = xml_getFloat(node)/60.0;
      else if (xml_isNode(node,"shield_regen"))
         temp->u.mod.shield_regen = xml_getFloat(node)/60.0;
      else if (xml_isNode(node,"energy_regen"))
         temp->u.mod.energy_regen = xml_getFloat(node)/60.0;
      /* misc */
      xmlr_int(node,"cargo",temp->u.mod.cargo);
   } while (xml_nextNode(node));
}


/*
 * parses the afterburner tidbits of the outfit
 */
static void outfit_parseSAfterburner( Outfit* temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   node = parent->children;

   /* must be >= 1. */
   temp->u.afb.thrust_perc = 1.;
   temp->u.afb.speed_perc = 1.;
   
   do { /* parse the data */
      xmlr_float(node,"rumble",temp->u.afb.rumble);
      if (xml_isNode(node,"thrust_perc"))
         temp->u.afb.thrust_perc = 1. + xml_getFloat(node)/100.;
      xmlr_float(node,"thrust_abs",temp->u.afb.thrust_abs);
      if (xml_isNode(node,"speed_perc"))
         temp->u.afb.speed_perc = 1. + xml_getFloat(node)/100.;
      xmlr_float(node,"speed_abs",temp->u.afb.speed_abs);
      xmlr_float(node,"energy",temp->u.afb.energy);
   } while (xml_nextNode(node));
}


/*
 * parses and returns Outfit from parent node
 */
static Outfit* outfit_parse( const xmlNodePtr parent )
{
   Outfit* temp = CALLOC_ONE(Outfit);
   xmlNodePtr cur, node;
   char *prop;
   char str[PATH_MAX] = "\0";

   temp->name = xml_nodeProp(parent,"name"); /* already mallocs */
   if (temp->name == NULL) WARN("Outfit in "OUTFIT_DATA" has invalid or no name");

   node = parent->xmlChildrenNode;

   do { /* load all the data */
      if (xml_isNode(node,"general")) {
         cur = node->children;
         do {
            xmlr_int(cur,"max",temp->max);
            xmlr_int(cur,"tech",temp->tech);
            xmlr_int(cur,"mass",temp->mass);
            xmlr_int(cur,"price",temp->price);
            xmlr_strd(cur,"description",temp->description);
            if (xml_isNode(cur,"gfx_store")) {
               snprintf( str, strlen(xml_get(cur))+sizeof(OUTFIT_GFX)+10,
                     OUTFIT_GFX"store/%s.png", xml_get(cur));
               temp->gfx_store = gl_newImage(str);
            }

         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"specific")) { /* has to be processed seperately */

         /* get the type */
         prop = xml_nodeProp(node,"type");
         if (prop == NULL)
            ERR("Outfit '%s' element 'specific' missing property 'type'",temp->name);
         temp->type = atoi(prop);
         free(prop);

         /* is secondary weapon? */
         prop = xml_nodeProp(node,"secondary");
         if (prop != NULL) {
            if ((int)atoi(prop)) outfit_setProp(temp, OUTFIT_PROP_WEAP_SECONDARY);
            free(prop);
         }

         if (temp->type==OUTFIT_TYPE_NULL)
            WARN("Outfit '%s' is of type NONE", temp->name);
         else if (outfit_isWeapon(temp))
            outfit_parseSWeapon( temp, node );
         else if (outfit_isLauncher(temp))
            outfit_parseSLauncher( temp, node );
         else if (outfit_isAmmo(temp))
            outfit_parseSAmmo( temp, node );
         else if (outfit_isTurret(temp))
            outfit_parseSWeapon( temp, node );
         else if (outfit_isMod(temp))
            outfit_parseSMod( temp, node );
         else if (outfit_isAfterburner(temp))
            outfit_parseSAfterburner( temp, node );
      }
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN("Outfit '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->max==0,"max");
   MELEMENT(temp->tech==0,"tech");
   MELEMENT(temp->gfx_store==NULL,"gfx_store");
   /*MELEMENT(temp->mass==0,"mass"); Not really needed */
   MELEMENT(temp->type==0,"type");
   MELEMENT(temp->price==0,"price");
   MELEMENT(temp->description==NULL,"description");
#undef MELEMENT

   return temp;
}


/*
 * loads all the outfits into the outfit_stack
 */
int outfit_load (void)
{
   uint32_t bufsize;
   char *buf = pack_readfile( DATA, OUTFIT_DATA, &bufsize );

   Outfit *temp;

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_OUTFIT_ID)) {
      ERR("Malformed '"OUTFIT_DATA"' file: missing root element '"XML_OUTFIT_ID"'");
      return -1;
   }        

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"OUTFIT_DATA"' file: does not contain elements");
      return -1;
   }        

   do {
      if (xml_isNode(node,XML_OUTFIT_TAG)) {

         temp = outfit_parse(node);               
         outfit_stack = realloc(outfit_stack, sizeof(Outfit)*(++outfits));
         memcpy(outfit_stack+outfits-1, temp, sizeof(Outfit));
         free(temp);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Outfit%s", outfits, (outfits==1) ? "" : "s" );

   return 0;
}


/*
 * frees the outfit stack
 */
void outfit_free (void)
{
   int i;
   for (i=0; i < outfits; i++) {
      /* free graphics */
      if (outfit_gfx(&outfit_stack[i]))
         gl_freeTexture(outfit_gfx(&outfit_stack[i]));

      /* strings */
      if (outfit_isLauncher(&outfit_stack[i]) && outfit_stack[i].u.lau.ammo)
         free(outfit_stack[i].u.lau.ammo);
      if (outfit_stack[i].description)
         free(outfit_stack[i].description);
      if (outfit_stack[i].gfx_store)
         gl_freeTexture(outfit_stack[i].gfx_store);
      free(outfit_stack[i].name);
   }
   free(outfit_stack);
}

