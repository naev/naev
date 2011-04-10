/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ship.c
 *
 * @brief Handles the ship details.
 */


#include "ship.h"

#include "naev.h"

#include <string.h>
#include <limits.h>

#include "nxml.h"

#include "log.h"
#include "ndata.h"
#include "toolkit.h"
#include "array.h"
#include "conf.h"
#include "npng.h"
#include "colour.h"


#define XML_ID    "Ships"  /**< XML document identifier */
#define XML_SHIP  "ship" /**< XML individual ship identifier. */

#define SHIP_DATA    "dat/ship.xml" /**< XML file containing ships. */
#define SHIP_GFX     "gfx/ship/" /**< Location of ship graphics. */
#define SHIP_EXT     ".png" /**< Ship graphics extension format. */
#define SHIP_ENGINE  "_engine" /**< Engine graphic extension. */
#define SHIP_TARGET  "_target" /**< Target graphic extension. */
#define SHIP_COMM    "_comm" /**< Communicatio graphic extension. */

#define VIEW_WIDTH   300 /**< Ship view window width. */
#define VIEW_HEIGHT  300 /**< Ship view window height. */

#define BUTTON_WIDTH  80 /**< Button width in ship view window. */
#define BUTTON_HEIGHT 30 /**< Button height in ship view window. */

#define CHUNK_SIZE    32 /**< Rate at which to allocate memory. */

#define STATS_DESC_MAX 128 /**< Maximum length for statistics description. */


static Ship* ship_stack = NULL; /**< Stack of ships available in the game. */


/*
 * Prototypes
 */
static int ship_loadGFX( Ship *temp, char *buf, int sx, int sy );
static int ship_parse( Ship *temp, xmlNodePtr parent );


/**
 * @brief Gets a ship based on its name.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
Ship* ship_get( const char* name )
{
   Ship *temp;
   int i;

   temp = ship_stack;
   for (i=0; i < array_size(ship_stack); i++)
      if (strcmp(temp[i].name, name)==0)
         return &temp[i];

   WARN("Ship %s does not exist", name);
   return NULL;
}


/**
 * @brief Gets a ship based on its name without warning.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
Ship* ship_getW( const char* name )
{
   Ship *temp;
   int i;

   temp = ship_stack;
   for (i=0; i < array_size(ship_stack); i++)
      if (strcmp(temp[i].name, name)==0)
         return &temp[i];

   return NULL;
}


/**
 * @brief Checks to see if an ship exists matching name (case insensitive).
 */
const char *ship_existsCase( const char* name )
{
   int i;
   for (i=0; i<array_size(ship_stack); i++)
      if (strcasecmp(name,ship_stack[i].name)==0)
         return ship_stack[i].name;
   return NULL;
}




/**
 * @brief Gets all the ships.
 */
Ship* ship_getAll( int *n )
{
   *n = array_size(ship_stack);
   return ship_stack;
}


/**
 * @brief Comparison function for qsort().
 */
int ship_compareTech( const void *arg1, const void *arg2 )
{
   const Ship *s1, *s2;

   /* Get ships. */
   s1 = * (const Ship**) arg1;
   s2 = * (const Ship**) arg2;

   /* Compare class. */
   if (s1->class < s2->class)
      return -1;
   else if (s1->class > s2->class)
      return +1;

   /* Compare price. */
   if (s1->price < s2->price)
      return -1;
   else if (s1->price > s2->price)
      return +1;

   /* Same. */
   return strcmp( s1->name, s2->name );
}


/**
 * @brief Gets the ship's class name in human readable form.
 *
 *    @param s Ship to get the class name from.
 *    @return The human readable class name.
 */
char* ship_class( Ship* s )
{
   switch (s->class) {
      case SHIP_CLASS_NULL:
         return "NULL";

      /* Civilian. */
      case SHIP_CLASS_YACHT:
         return "Yacht";
      case SHIP_CLASS_LUXURY_YACHT:
         return "Luxury Yacht";
      case SHIP_CLASS_CRUISE_SHIP:
         return "Cruise Ship";

      /* Merchant. */
      case SHIP_CLASS_COURIER:
         return "Courier";
      case SHIP_CLASS_ARMOURED_TRANSPORT:
         return "Armoured Transport";
      case SHIP_CLASS_FREIGHTER:
         return "Freighter";
      case SHIP_CLASS_BULK_CARRIER:
         return "Bulk Carrier";

      /* Military. */
      case SHIP_CLASS_SCOUT:
         return "Scout";
      case SHIP_CLASS_FIGHTER:
         return "Fighter";
      case SHIP_CLASS_BOMBER:
         return "Bomber";
      case SHIP_CLASS_CORVETTE:
         return "Corvette";
      case SHIP_CLASS_DESTROYER:
         return "Destroyer";
      case SHIP_CLASS_CRUISER:
         return "Cruiser";
      case SHIP_CLASS_CARRIER:
         return "Carrier";

      /* Robotic. */
      case SHIP_CLASS_DRONE:
         return "Drone";
      case SHIP_CLASS_HEAVY_DRONE:
         return "Heavy Drone";
      case SHIP_CLASS_MOTHERSHIP:
         return "Mothership";

      /* Unknown. */
      default:
         return "Unknown";
   }
}


/**
 * @brief Gets the machine ship class identifier from a human readable string.
 *
 *    @param str String to extract ship class identifier from.
 */
ShipClass ship_classFromString( char* str )
{
   /* Civilian */
   if (strcmp(str,"Yacht")==0)
      return SHIP_CLASS_YACHT;
   else if (strcmp(str,"Luxury Yacht")==0)
      return SHIP_CLASS_LUXURY_YACHT;
   else if (strcmp(str,"Cruise Ship")==0)
      return SHIP_CLASS_CRUISE_SHIP;

   /* Merchant. */
   else if (strcmp(str,"Courier")==0)
      return SHIP_CLASS_COURIER;
   else if (strcmp(str,"Freighter")==0)
      return SHIP_CLASS_FREIGHTER;
   else if (strcmp(str,"Armoured Transport")==0)
      return SHIP_CLASS_ARMOURED_TRANSPORT;
   else if (strcmp(str,"Bulk Carrier")==0)
      return SHIP_CLASS_BULK_CARRIER;

   /* Military */
   else if (strcmp(str,"Scout")==0)
      return SHIP_CLASS_SCOUT;
   else if (strcmp(str,"Fighter")==0)
      return SHIP_CLASS_FIGHTER;
   else if (strcmp(str,"Bomber")==0)
      return SHIP_CLASS_BOMBER;
   else if (strcmp(str,"Corvette")==0)
      return SHIP_CLASS_CORVETTE;
   else if (strcmp(str,"Destroyer")==0)
      return SHIP_CLASS_DESTROYER;
   else if (strcmp(str,"Cruiser")==0)
      return SHIP_CLASS_CRUISER;
   else if (strcmp(str,"Carrier")==0)
      return SHIP_CLASS_CARRIER;

   /* Robotic */
   else if (strcmp(str,"Drone")==0)
      return SHIP_CLASS_DRONE;
   else if (strcmp(str,"Heavy Drone")==0)
      return SHIP_CLASS_HEAVY_DRONE;
   else if (strcmp(str,"Mothership")==0)
      return SHIP_CLASS_MOTHERSHIP;

  /* Unknown */
  return SHIP_CLASS_NULL;
}


/**
 * @brief Gets the ship's base price (no outfits).
 */
credits_t ship_basePrice( Ship* s )
{
   credits_t price;

   /* Get ship base price. */
   price = s->price;

   if (price < 0) {
      WARN("Negative ship base price!");
      price = 0;
   }

   return price;
}


/**
 * @brief Loads the ship's comm graphic.
 *
 * Must be freed afterwards.
 */
glTexture* ship_loadCommGFX( Ship* s )
{
   return gl_newImage( s->gfx_comm, 0 );
}


/**
 * @brief Parses a single ship stat from a parent node if it is one.
 *
 *    @return 0 on success, 1 if not a node.
 */
int ship_statsParseSingle( ShipStats *s, xmlNodePtr node )
{
   /* Freighter. */
   xmlr_floatR(node,"jump_delay",s->jump_delay);
   xmlr_floatR(node,"jump_range",s->jump_range);
   xmlr_floatR(node,"cargo_inertia",s->cargo_inertia);
   /* Scout. */
   xmlr_floatR(node,"jam_range",s->jam_range);
   xmlr_floatR(node,"ew_hide",s->ew_hide);
   xmlr_floatR(node,"ew_detect",s->ew_detect);
   /* Military. */
   xmlr_floatR(node,"heat_dissipation",s->heat_dissipation);
   /* Bomber. */
   xmlr_floatR(node,"launch_rate",s->launch_rate);
   xmlr_floatR(node,"launch_range",s->launch_range);
   xmlr_floatR(node,"jam_counter",s->jam_counter);
   xmlr_floatR(node,"ammo_capacity",s->ammo_capacity);
   /* Fighter. */
   xmlr_floatR(node,"heat_forward",s->heat_forward);
   xmlr_floatR(node,"damage_forward",s->damage_forward);
   xmlr_floatR(node,"firerate_forward",s->firerate_forward);
   xmlr_floatR(node,"energy_forward",s->energy_forward);
   /* Cruiser. */
   xmlr_floatR(node,"heat_turret",s->heat_turret);
   xmlr_floatR(node,"damage_turret",s->damage_turret);
   xmlr_floatR(node,"firerate_turret",s->firerate_turret);
   xmlr_floatR(node,"energy_turret",s->energy_turret);
   /* Misc. */
   xmlr_floatR(node,"nebula_damage",s->nebula_damage);
   return 1;
}


/**
 * @brief Writes the ship statistics description.
 *
 *    @param s Ship stats to use.
 *    @param buf Buffer to write to.
 *    @param len Space left in the buffer.
 *    @param newline Add a newline at start.
 *    @param pilot Stats come from a pilot.
 *    @return Number of characters written.
 */
int ship_statsDesc( ShipStats *s, char *buf, int len, int newline, int pilot )
{
   int i;

   /* Set stat text. */
   i = 0;
#define DESC_ADD(x, s) \
   if ((pilot && (x!=1.)) || (!pilot && (x!=0.))) \
      i += snprintf( &buf[i], len-i, \
            "%s%+.0f%% "s, (!newline&&(i==0)) ? "" : "\n", \
            (pilot) ? (x-1.)*100. : x );
   /* Freighter Stuff. */
   DESC_ADD(s->jump_delay,"Jump Time");
   DESC_ADD(s->jump_range,"Jump Range");
   DESC_ADD(s->cargo_inertia,"Cargo Inertia");
   /* Scout Stuff. */
   DESC_ADD(s->jam_range,"Jam Range");
   DESC_ADD(s->ew_detect,"Detection");
   DESC_ADD(s->ew_hide,"Cloaking");
   /* Military Stuff. */
   DESC_ADD(s->heat_dissipation,"Heat Dissipation");
   /* Bomber STuff. */
   DESC_ADD(s->launch_rate,"Launch Rate");
   DESC_ADD(s->launch_range,"Launch Range");
   DESC_ADD(s->jam_counter,"Jam Countermeasures");
   DESC_ADD(s->ammo_capacity,"Ammo Capacity");
   /* Fighter Stuff. */
   DESC_ADD(s->heat_forward,"Heat (Cannon)");
   DESC_ADD(s->damage_forward,"Damage (Cannon)");
   DESC_ADD(s->firerate_forward,"Fire Rate (Cannon)");
   DESC_ADD(s->energy_forward,"Energy Usage (Cannon)");
   /* Cruiser Stuff. */
   DESC_ADD(s->heat_turret,"Heat (Turret)");
   DESC_ADD(s->damage_turret,"Damage (Turret)");
   DESC_ADD(s->firerate_turret,"Fire Rate (Turret)");
   DESC_ADD(s->energy_turret,"Energy Usage (Turret)");
   /* Misc. */
   DESC_ADD(s->nebula_damage,"Nebula damage.");
#undef DESC_ADD

   return i;
}


/**
 * @brief Generates a target graphic for a ship.
 */
static int ship_genTargetGFX( Ship *temp, SDL_Surface *surface, int sx, int sy )
{
   SDL_Surface *gfx, *gfx_store;
   int potw, poth, potw_store, poth_store;
   int x, y, sw, sh;
   SDL_Rect rtemp, dstrect;
#if 0 /* Required for scanlines. */
   int i, j;
   uint32_t *pix;
   double r, g, b, a;
   double h, s, v;
#endif
   char buf[PATH_MAX];
#if ! SDL_VERSION_ATLEAST(1,3,0)
   Uint32 saved_flags;
   Uint8 saved_alpha;
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

   /* Get sprite size. */
   sw = temp->gfx_space->w / sx;
   sh = temp->gfx_space->h / sy;

   /* POT size. */
   if (gl_needPOT()) {
      potw = gl_pot( sw );
      poth = gl_pot( sh );
      potw_store = gl_pot( SHIP_TARGET_W );
      poth_store = gl_pot( SHIP_TARGET_H );
   }
   else {
      potw = sw;
      poth = sh;
      potw_store = SHIP_TARGET_W;
      poth_store = SHIP_TARGET_H;
   }

   /* Create the surface. */
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* create the temp POT surface */
   gfx = SDL_CreateRGBSurface( 0, potw, poth,
         surface->format->BytesPerPixel*8, RGBAMASK );
   gfx_store = SDL_CreateRGBSurface( 0, potw_store, poth_store,
         surface->format->BytesPerPixel*8, RGBAMASK );
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   saved_flags = surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
   saved_alpha = surface->format->alpha;
   if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
      SDL_SetAlpha( surface, 0, SDL_ALPHA_OPAQUE );
      SDL_SetColorKey( surface, 0, surface->format->colorkey );
   }

   /* create the temp POT surface */
   gfx = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
         potw, poth, surface->format->BytesPerPixel*8, RGBAMASK );
   gfx_store = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
         potw_store, poth_store, surface->format->BytesPerPixel*8, RGBAMASK );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   if (gfx == NULL) {
      WARN( "Unable to create ship '%s' targetting surface.", temp->name );
      return -1;
   }

   /* Copy over for target. */
   gl_getSpriteFromDir( &x, &y, temp->gfx_space, M_PI* 5./4. );
   rtemp.x = sw * x;
   rtemp.y = sh * (temp->gfx_space->sy-y-1);
   rtemp.w = sw;
   rtemp.h = sh;
   dstrect.x = 0;
   dstrect.y = 0;
   dstrect.w = rtemp.w;
   dstrect.h = rtemp.h;
   SDL_BlitSurface( surface, &rtemp, gfx, &dstrect );

   /* Copy over for store. */
   dstrect.x = (SHIP_TARGET_W - sw) / 2;
   dstrect.y = (SHIP_TARGET_H - sh) / 2;
   dstrect.w = rtemp.w;
   dstrect.h = rtemp.h;
   SDL_BlitSurface( surface, &rtemp, gfx_store, &dstrect );

#if ! SDL_VERSION_ATLEAST(1,3,0)
   /* set saved alpha */
   if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
      SDL_SetAlpha( surface, 0, 0 );
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

   /* Load the store surface. */
   snprintf( buf, sizeof(buf), "%s_gfx_store.png", temp->name );
   temp->gfx_store = gl_loadImagePad( buf, gfx_store, 0, SHIP_TARGET_W, SHIP_TARGET_H, 1, 1, 1 );

#if 0 /* Disabled for now due to issues with larger sprites. */
   /* Some filtering. */
   for (j=0; j<sh; j++) {
      for (i=0; i<sw; i++) {
         pix   = (uint32_t*) ((uint8_t*) gfx->pixels + j*gfx->pitch + i*gfx->format->BytesPerPixel);
         r     = ((double) (*pix & RMASK)) / (double) RMASK;
         g     = ((double) (*pix & GMASK)) / (double) GMASK;
         b     = ((double) (*pix & BMASK)) / (double) BMASK;
         a     = ((double) (*pix & AMASK)) / (double) AMASK;
         if (j%2) /* Add scanlines. */
            a *= 0.5;

         /* Convert to HSV. */
         col_rgb2hsv( &h, &s, &v, r, g, b );

         h  = 0.0;
         s  = 1.0;
         v *= 1.5;

         /* Convert back to RGB. */
         col_hsv2rgb( &r, &g, &b, h, s, v );

         /* Convert to pixel. */
         *pix =   ((uint32_t) (r*RMASK) & RMASK) |
                  ((uint32_t) (g*GMASK) & GMASK) |
                  ((uint32_t) (b*BMASK) & BMASK) |
                  ((uint32_t) (a*AMASK) & AMASK);
      }
   }
#endif

   /* Load the surface. */
   snprintf( buf, sizeof(buf), "%s_gfx_target.png", temp->name );
   temp->gfx_target = gl_loadImagePad( buf, gfx, 0, sw, sh, 1, 1, 1 );

   return 0;
}


/**
 * @brief Loads the graphics for a ship.
 *
 *    @param temp Ship to load into.
 *    @param buf Name of the texture to work with.
 */
static int ship_loadGFX( Ship *temp, char *buf, int sx, int sy )
{
   char base[PATH_MAX], str[PATH_MAX];
   int i;
   png_uint_32 w, h;
   SDL_RWops *rw;
   npng_t *npng;
   SDL_Surface *surface;

   /* Get base path. */
   for (i=0; i<PATH_MAX; i++) {
      if ((buf[i] == '\0') || (buf[i] == '_')) {
         base[i] = '\0';
         break;
      }
      base[i] = buf[i];
   }
   if (i>=PATH_MAX) {
      WARN("Failed to get base path of '%s'.", buf);
      return -1;
   }

   /* Load the space sprite. */
   snprintf( str, PATH_MAX, SHIP_GFX"%s/%s"SHIP_EXT, base, buf );
   rw    = ndata_rwops( str );
   npng  = npng_open( rw );
   npng_dim( npng, &w, &h );
   surface = npng_readSurface( npng, gl_needPOT(), 1 );

   /* Load the texture. */
   temp->gfx_space = gl_loadImagePad( str, surface,
         OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS,
         w, h, sx, sy, 0 );

   /* Create the target graphic. */
   ship_genTargetGFX( temp, surface, sx, sy );

   /* Free stuff. */
   npng_close( npng );
   SDL_RWclose( rw );
   SDL_FreeSurface( surface );

   /* Load the engine sprite .*/
   if (conf.engineglow && conf.interpolate) {
      snprintf( str, PATH_MAX, SHIP_GFX"%s/%s"SHIP_ENGINE SHIP_EXT, base, buf );
      temp->gfx_engine = gl_newSprite( str, sx, sy, OPENGL_TEX_MIPMAPS );
      if (temp->gfx_engine == NULL)
         WARN("Ship '%s' does not have an engine sprite (%s).", temp->name, str );
   }

   /* Calculate mount angle. */
   temp->mangle  = 2.*M_PI;
   temp->mangle /= temp->gfx_space->sx * temp->gfx_space->sy;

   /* Get the comm graphic for future loading. */
   snprintf( str, PATH_MAX, SHIP_GFX"%s/%s"SHIP_COMM SHIP_EXT, base, buf );
   temp->gfx_comm = strdup(str);

   return 0;
}


/**
 * @brief Extracts the ingame ship from an XML node.
 *
 *    @param temp Ship to load data into.
 *    @param parent Node to get ship from.
 *    @return 0 on success.
 */
static int ship_parse( Ship *temp, xmlNodePtr parent )
{
   int i;
   xmlNodePtr cur, node;
   char str[PATH_MAX];
   int sx, sy;
   char *stmp, *buf;
   int l, m, h;
   OutfitSlotSize base_size;

   /* Clear memory. */
   memset( temp, 0, sizeof(Ship) );

   /* Defaults. */
   str[0] = '\0';
   temp->thrust = -1;
   temp->speed  = -1;

   /* Get name. */
   xmlr_attr(parent,"name",temp->name);
   if (temp->name == NULL)
      WARN("Ship in "SHIP_DATA" has invalid or no name");

   /* Load data. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"GFX")) {

         /* Get base graphic name. */
         buf = xml_get(node);
         if (buf==NULL) {
            WARN("Ship '%s': GFX element is NULL", temp->name);
            continue;
         }

         /* Get sprite size. */
         xmlr_attr(node, "sx", stmp );
         if (stmp != NULL) {
            sx = atoi(stmp);
            free(stmp);
         }
         else
            sx = 8;
         xmlr_attr(node, "sy", stmp );
         if (stmp != NULL) {
            sy = atoi(stmp);
            free(stmp);
         }
         else
            sy = 8;

         /* Load the graphics. */
         ship_loadGFX( temp, buf, sx, sy );

         continue;
      }

      xmlr_strd(node,"GUI",temp->gui);
      if (xml_isNode(node,"sound")) {
         temp->sound = sound_get( xml_get(node) );
         continue;
      }
      xmlr_strd(node,"base_type",temp->base_type);
      if (xml_isNode(node,"class")) {
         temp->class = ship_classFromString( xml_get(node) );
         continue;
      }
      xmlr_long(node,"price",temp->price);
      xmlr_strd(node,"license",temp->license);
      xmlr_strd(node,"fabricator",temp->fabricator);
      xmlr_strd(node,"description",temp->description);
      if (xml_isNode(node,"movement")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_float(cur,"thrust",temp->thrust);
            xmlr_float(cur,"turn",temp->turn);
            xmlr_float(cur,"speed",temp->speed);
            /* All the xmlr_ stuff have continue cases. */
            WARN("Ship '%s' has unknown movement node '%s'.", temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"health")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_float(cur,"absorb",temp->dmg_absorb);
            xmlr_float(cur,"armour",temp->armour);
            xmlr_float(cur,"armour_regen",temp->armour_regen);
            xmlr_float(cur,"shield",temp->shield);
            xmlr_float(cur,"shield_regen",temp->shield_regen);
            xmlr_float(cur,"energy",temp->energy);
            xmlr_float(cur,"energy_regen",temp->energy_regen);
            /* All the xmlr_ stuff have continue cases. */
            WARN("Ship '%s' has unknown health node '%s'.", temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"characteristics")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_int(cur,"crew",temp->crew);
            xmlr_float(cur,"mass",temp->mass);
            xmlr_float(cur,"cpu",temp->cpu);
            xmlr_int(cur,"fuel",temp->fuel);
            xmlr_float(cur,"cap_cargo",temp->cap_cargo);
            /* All the xmlr_ stuff have continue cases. */
            WARN("Ship '%s' has unknown characteristic node '%s'.", temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"slots")) {
         /* First pass, get number of mounts. */
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"structure"))
               temp->outfit_nstructure++;
            else if (xml_isNode(cur,"utility"))
               temp->outfit_nutility++;
            else if (xml_isNode(cur,"weapon"))
               temp->outfit_nweapon++;
            else
               WARN("Ship '%s' has unknown slot node '%s'.", temp->name, cur->name);
         } while (xml_nextNode(cur));
         /* Allocate the space. */
         temp->outfit_structure     = calloc( temp->outfit_nstructure, sizeof(ShipOutfitSlot) );
         temp->outfit_utility  = calloc( temp->outfit_nutility, sizeof(ShipOutfitSlot) );
         temp->outfit_weapon    = calloc( temp->outfit_nweapon, sizeof(ShipOutfitSlot) );
         /* Second pass, initialize the mounts. */
         l = m = h = 0;
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"structure")) {
               temp->outfit_structure[l].slot.type = OUTFIT_SLOT_STRUCTURE;
               temp->outfit_structure[l].slot.size = outfit_toSlotSize( xml_get(cur) );
               /*if (temp->outfit_structure[l].size == OUTFIT_SLOT_SIZE_NA)
                  WARN("Ship '%s' has invalid slot size '%s'", temp->name, xml_get(cur) );*/
               l++;
            }
            if (xml_isNode(cur,"utility")) {
               temp->outfit_utility[m].slot.type = OUTFIT_SLOT_UTILITY;
               temp->outfit_utility[m].slot.size = outfit_toSlotSize( xml_get(cur) );
               /*if (temp->outfit_utility[m].size == OUTFIT_SLOT_SIZE_NA)
                  WARN("Ship '%s' has invalid slot size '%s'", temp->name, xml_get(cur) );*/
               m++;
            }
            if (xml_isNode(cur,"weapon")) {
               temp->outfit_weapon[h].slot.type = OUTFIT_SLOT_WEAPON;
               temp->outfit_weapon[h].slot.size = outfit_toSlotSize( xml_get(cur) );
               /*if (temp->outfit_weapon[h].size == OUTFIT_SLOT_SIZE_NA)
                  WARN("Ship '%s' has invalid slot size '%s'", temp->name, xml_get(cur) );*/
               /* Get mount point. */
               xmlr_attr(cur,"x",stmp);
               if (stmp!=NULL) {
                  temp->outfit_weapon[h].mount.x = atof(stmp);
                  free(stmp);
               }
               else
                  WARN("Ship '%s' missing 'x' element of 'weapon' slot.",temp->name);
               xmlr_attr(cur,"y",stmp);
               if (stmp!=NULL) {
                  temp->outfit_weapon[h].mount.y = atof(stmp);
                  /* Since we measure in pixels, we have to modify it so it
                   *  doesn't get corrected by the ortho correction. */
                  temp->outfit_weapon[h].mount.y *= M_SQRT2;
                  free(stmp);
               }
               else
                  WARN("Ship '%s' missing 'y' element of 'weapon' slot.",temp->name);
               xmlr_attr(cur,"h",stmp);
               if (stmp!=NULL) {
                  temp->outfit_weapon[h].mount.h = atof(stmp);
                  free(stmp);
               }
               else
                  WARN("Ship '%s' missing 'h' element of 'weapon' slot.",temp->name);
               /* Increment h. */
               h++;
            }
         } while (xml_nextNode(cur));
         continue;
      }

      /* Parse ship stats. */
      if (xml_isNode(node,"stats")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (ship_statsParseSingle( &temp->stats, cur ))
               WARN("Ship '%s' has unknown stat '%s'.", temp->name, cur->name);
         } while (xml_nextNode(cur));
         temp->desc_stats = malloc( STATS_DESC_MAX );
         i = ship_statsDesc( &temp->stats, temp->desc_stats, STATS_DESC_MAX, 0, 0 );
         if (i <= 0) {
            free( temp->desc_stats );
            temp->desc_stats = NULL;
         }
         continue;
      }

      DEBUG("Ship '%s' has unknown node '%s'.", temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post processing. */
   temp->dmg_absorb   /= 100.;
   temp->armour_regen /= 60.;
   temp->shield_regen /= 60.;
   temp->energy_regen /= 60.;
   temp->turn         *= M_PI / 180.; /* Convert to rad. */
   temp->thrust *= temp->mass;

   /* Second pass default values for slot size. */
   if ((temp->class == SHIP_CLASS_BULK_CARRIER) ||
         (temp->class == SHIP_CLASS_CRUISER) ||
         (temp->class == SHIP_CLASS_CARRIER) ||
         (temp->class == SHIP_CLASS_MOTHERSHIP))
      base_size = OUTFIT_SLOT_SIZE_HEAVY;
   else if ((temp->class == SHIP_CLASS_CRUISE_SHIP) ||
         (temp->class == SHIP_CLASS_FREIGHTER) ||
         (temp->class == SHIP_CLASS_DESTROYER) ||
         (temp->class == SHIP_CLASS_CORVETTE) ||
         (temp->class == SHIP_CLASS_HEAVY_DRONE) ||
         (temp->class == SHIP_CLASS_ARMOURED_TRANSPORT))
      base_size = OUTFIT_SLOT_SIZE_MEDIUM;
   else
      base_size = OUTFIT_SLOT_SIZE_LIGHT;
   for (i=0; i<temp->outfit_nweapon; i++)
      if (temp->outfit_weapon[i].slot.size == OUTFIT_SLOT_SIZE_NA)
         temp->outfit_weapon[i].slot.size = base_size;
   for (i=0; i<temp->outfit_nutility; i++)
      if (temp->outfit_utility[i].slot.size == OUTFIT_SLOT_SIZE_NA)
         temp->outfit_utility[i].slot.size = base_size;
   for (i=0; i<temp->outfit_nstructure; i++)
      if (temp->outfit_structure[i].slot.size == OUTFIT_SLOT_SIZE_NA)
         temp->outfit_structure[i].slot.size = base_size;

   /* ship validator */
#define MELEMENT(o,s)      if (o) WARN("Ship '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->base_type==NULL,"base_type");
   MELEMENT(temp->gfx_space==NULL,"GFX");
   MELEMENT(temp->gui==NULL,"GUI");
   MELEMENT(temp->class==SHIP_CLASS_NULL,"class");
   MELEMENT(temp->price==0,"price");
   MELEMENT(temp->fabricator==NULL,"fabricator");
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->thrust==-1,"thrust");
   MELEMENT(temp->turn==0,"turn");
   MELEMENT(temp->speed==-1,"speed");
   MELEMENT(temp->armour==0,"armour");
   MELEMENT(temp->shield==0,"shield");
   MELEMENT(temp->shield_regen==0,"shield_regen");
   MELEMENT(temp->energy==0,"energy");
   MELEMENT(temp->energy_regen==0,"energy_regen");
   MELEMENT(temp->fuel==0,"fuel");
   MELEMENT(temp->crew==0,"crew");
   MELEMENT(temp->mass==0.,"mass");
   MELEMENT(temp->cpu==0.,"cpu");
   MELEMENT(temp->cap_cargo==0,"cap_cargo");
#undef MELEMENT

   return 0;
}


/**
 * @brief Loads all the ships in the data files.
 *
 *    @return 0 on success.
 */
int ships_load (void)
{
   uint32_t bufsize;
   char *buf = ndata_read( SHIP_DATA, &bufsize);

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode; /* Ships node */
   if (strcmp((char*)node->name,XML_ID)) {
      ERR("Malformed "SHIP_DATA" file: missing root element '"XML_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first ship node */
   if (node == NULL) {
      ERR("Malformed "SHIP_DATA" file: does not contain elements");
      return -1;
   }

   ship_stack = array_create(Ship);
   do {
      if (xml_isNode(node, XML_SHIP))
         /* Load the ship. */
         ship_parse(&array_grow(&ship_stack), node);
   } while (xml_nextNode(node));
   array_shrink(&ship_stack);

   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Ship%s", array_size(ship_stack), (array_size(ship_stack)==1) ? "" : "s" );

   return 0;
}


/**
 * @brief Frees all the ships.
 */
void ships_free (void)
{
   Ship *s;
   int i;
   for (i = 0; i < array_size(ship_stack); i++) {
      s = &ship_stack[i];

      /* Free stored strings. */
      if (s->name != NULL)
         free(s->name);
      if (s->description != NULL)
         free(s->description);
      if (s->gui != NULL)
         free(s->gui);
      if (s->base_type != NULL)
         free(s->base_type);
      if (s->fabricator != NULL)
         free(s->fabricator);
      if (s->license != NULL)
         free(s->license);
      if (s->desc_stats != NULL)
         free(s->desc_stats);

      /* Free outfits. */
      if (s->outfit_structure != NULL)
         free(s->outfit_structure);
      if (s->outfit_utility != NULL)
         free(s->outfit_utility);
      if (s->outfit_weapon != NULL)
         free(s->outfit_weapon);

      /* Free graphics. */
      gl_freeTexture(s->gfx_space);
      if (s->gfx_engine != NULL)
         gl_freeTexture(s->gfx_engine);
      if (s->gfx_target != NULL)
         gl_freeTexture(s->gfx_target);
      if (s->gfx_store != NULL)
         gl_freeTexture(s->gfx_store);
      free(s->gfx_comm);
   }

   array_free(ship_stack);
   ship_stack = NULL;
}


/**
 * @brief Used to visualize the ships stats.
 *
 * @todo Take into account outfits or something like that.
 *
 *    @param unused Unused.
 *    @param shipname Ship ot view the stats of.
 */
void ship_view( unsigned int unused, char* shipname )
{
   (void) unused;
   Ship *s;
   char buf[1024];
   unsigned int wid;
   int h;
   s = ship_get( shipname );
   snprintf(buf, 1024,
         "Name:\n"
         "Class:\n"
         "Fabricator:\n"
         "\n"
         "Crew:\n"
         "CPU:\n"
         "Mass:\n"
         "\n"
         "Weapon slots:\n"
         "Utility slots:\n"
         "Structure slots:\n"
         "\n"
         "Thrust:\n"
         "Max Speed:\n"
         "Turn:\n"
         "\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "\n"
         "Cargo Space:\n"
         "Fuel:\n"
         );
   h = gl_printHeightRaw( &gl_smallFont, VIEW_WIDTH, buf );

   wid = window_create( shipname, -1, -1, VIEW_WIDTH, h+60+BUTTON_HEIGHT );
   window_addText( wid, 20, -40, VIEW_WIDTH, h,
         0, "txtLabel", &gl_smallFont, &cDConsole, buf );
   snprintf( buf, 1024,
         "%s\n" /* Name */
         "%s\n" /* Class */
         "%s\n" /* Fabricator */
         "\n"
         "%d\n" /* Crew */
         "%.0f TFlops\n" /* CPU */
         "%.0f Tons\n" /* Mass */
         "\n"
         "%d\n" /* Weapon slots */
         "%d\n" /* Utility slots */
         "%d\n" /* Structure slots */
         "\n"
         "%.0f MN/ton\n" /* Thrust */
         "%.0f M/s\n" /* Speed */
         "%.0f Grad/s\n" /* Turn */
         "\n"
         "%.0f MJ (%.1f MJ/s)\n" /* Shield */
         "%.0f MJ (%.1f MJ/s)\n" /* Armour */
         "%.0f MJ (%.1f MJ/s)\n" /* Energy */
         "\n"
         "%.0f Tons\n" /* Cargo */
         "%d Units\n" /* Fuel */
         , s->name, ship_class(s), s->fabricator,
         s->crew, s->cpu, s->mass,
         s->outfit_nweapon, s->outfit_nutility, s->outfit_nstructure,
         s->thrust/s->mass, s->speed, s->turn,
         s->shield, s->shield_regen, s->armour, s->armour_regen,
         s->energy, s->energy_regen,
         s->cap_cargo, s->fuel );
   window_addText( wid, 120, -40, VIEW_WIDTH-140, h,
         0, "txtProperties", &gl_smallFont, &cBlack, buf );

   /* close button */
   snprintf( buf, 37, "close%s", shipname );
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         buf, "Close", window_close );
}

