/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ship.c
 *
 * @brief Handles the ship details.
 */


/** @cond */
#include <limits.h>
#include "physfsrwops.h"
#include "SDL_image.h"

#include "naev.h"
/** @endcond */

#include "ship.h"

#include "array.h"
#include "colour.h"
#include "conf.h"
#include "log.h"
#include "ndata.h"
#include "nfile.h"
#include "nstring.h"
#include "nxml.h"
#include "shipstats.h"
#include "slots.h"
#include "toolkit.h"
#include "unistd.h"


#define XML_SHIP  "ship" /**< XML individual ship identifier. */

#define SHIP_ENGINE  "_engine" /**< Engine graphic extension. */
#define SHIP_TARGET  "_target" /**< Target graphic extension. */
#define SHIP_COMM    "_comm" /**< Communication graphic extension. */

#define VIEW_WIDTH   300 /**< Ship view window width. */
#define VIEW_HEIGHT  300 /**< Ship view window height. */

#define BUTTON_WIDTH  80 /**< Button width in ship view window. */
#define BUTTON_HEIGHT 30 /**< Button height in ship view window. */

#define STATS_DESC_MAX 256 /**< Maximum length for statistics description. */


static Ship* ship_stack = NULL; /**< Stack of ships available in the game. */


/*
 * Prototypes
 */
static int ship_loadGFX( Ship *temp, const char *buf, int sx, int sy, int engine );
static int ship_loadPLG( Ship *temp, const char *buf, int size_hint );
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

   WARN(_("Ship %s does not exist"), name);
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
 * @brief Gets the array (array.h) of all ships.
 */
const Ship* ship_getAll (void)
{
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
      return +1;
   else if (s1->class > s2->class)
      return -1;

   /* Compare price. */
   if (s1->price < s2->price)
      return +1;
   else if (s1->price > s2->price)
      return -1;

   /* Same. */
   return strcmp( s1->name, s2->name );
}


/**
 * @brief Gets the ship's class name in human readable form.
 *
 *    @param s Ship to get the class name from.
 *    @return The human readable class name.
 */
const char* ship_class( const Ship* s )
{
   return ship_classToString( s->class );
}


/**
 * @brief Gets the ship's display class in human readable form.
 *
 *    @param s Ship to get the display class name from.
 *    @return The human readable display class name.
 */
const char* ship_classDisplay( const Ship* s )
{
   if (s->class_display)
      return s->class_display;
   return ship_class( s );
}


/**
 * @brief Gets the ship class name in human readable form.
 *
 *    @param class Class to get name of.
 *    @return The human readable class name.
 */
const char *ship_classToString( ShipClass class )
{
   switch (class) {
      case SHIP_CLASS_NULL:
         return "NULL";

      /* Civilian. */
      case SHIP_CLASS_YACHT:
         return gettext_noop("Yacht");
      case SHIP_CLASS_COURIER:
         return gettext_noop("Courier");
      case SHIP_CLASS_FREIGHTER:
         return gettext_noop("Freighter");
      case SHIP_CLASS_BULK_CARRIER:
         return gettext_noop("Bulk Carrier");
      case SHIP_CLASS_ARMOURED_TRANSPORT:
         return gettext_noop("Armoured Transport");

      /* Military. */
      case SHIP_CLASS_SCOUT:
         return gettext_noop("Scout");
      case SHIP_CLASS_INTERCEPTOR:
         return gettext_noop("Interceptor");
      case SHIP_CLASS_FIGHTER:
         return gettext_noop("Fighter");
      case SHIP_CLASS_BOMBER:
         return gettext_noop("Bomber");
      case SHIP_CLASS_CORVETTE:
         return gettext_noop("Corvette");
      case SHIP_CLASS_DESTROYER:
         return gettext_noop("Destroyer");
      case SHIP_CLASS_CRUISER:
         return gettext_noop("Cruiser");
      case SHIP_CLASS_BATTLESHIP:
         return gettext_noop("Battleship");
      case SHIP_CLASS_CARRIER:
         return gettext_noop("Carrier");

      /* Unknown. */
      default:
         return gettext_noop("Unknown");
   }
}


#define STRTOSHIP( x, y ) if (strcmp(str,x)==0) return y
/**
 * @brief Gets the machine ship class identifier from a human readable string.
 *
 *    @param str String to extract ship class identifier from.
 */
ShipClass ship_classFromString( const char* str )
{
   if ( str != NULL ) {
      /* Civilian */
      STRTOSHIP( "Yacht",              SHIP_CLASS_YACHT );
      STRTOSHIP( "Courier",            SHIP_CLASS_COURIER );
      STRTOSHIP( "Freighter",          SHIP_CLASS_FREIGHTER );
      STRTOSHIP( "Bulk Carrier",       SHIP_CLASS_BULK_CARRIER );
      STRTOSHIP( "Armoured Transport", SHIP_CLASS_ARMOURED_TRANSPORT );

      /* Military */
      STRTOSHIP( "Scout",              SHIP_CLASS_SCOUT );
      STRTOSHIP( "Interceptor",        SHIP_CLASS_INTERCEPTOR );
      STRTOSHIP( "Fighter",            SHIP_CLASS_FIGHTER );
      STRTOSHIP( "Bomber",             SHIP_CLASS_BOMBER );
      STRTOSHIP( "Corvette",           SHIP_CLASS_CORVETTE );
      STRTOSHIP( "Destroyer",          SHIP_CLASS_DESTROYER );
      STRTOSHIP( "Cruiser",            SHIP_CLASS_CRUISER );
      STRTOSHIP( "Battleship",         SHIP_CLASS_BATTLESHIP);
      STRTOSHIP( "Carrier",            SHIP_CLASS_CARRIER );
   }

  /* Unknown */
  return SHIP_CLASS_NULL;
}
#undef STRTOSHIP


/**
 * @brief Gets the ship's base price (no outfits).
 */
credits_t ship_basePrice( const Ship* s )
{
   credits_t price;

   /* Get ship base price. */
   price = s->price;

   if (price < 0) {
      WARN(_("Negative ship base price!"));
      price = 0;
   }

   return price;
}


/**
 * @brief The ship buy price, includes default outfits.
 */
credits_t ship_buyPrice( const Ship* s )
{
   int i;
   credits_t price;
   Outfit *o;

   /* Get base price. */
   price = ship_basePrice(s);

   for (i=0; i<array_size(s->outfit_structure); i++) {
      o = s->outfit_structure[i].data;
      if (o != NULL)
         price += o->price;
   }
   for (i=0; i<array_size(s->outfit_utility); i++) {
      o = s->outfit_utility[i].data;
      if (o != NULL)
         price += o->price;
   }
   for (i=0; i<array_size(s->outfit_weapon); i++) {
      o = s->outfit_weapon[i].data;
      if (o != NULL)
         price += o->price;
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
   if (s->gfx_comm != NULL)
      return gl_newImage( s->gfx_comm, 0 );
   return NULL;
}


/**
 * @brief Gets the size of the ship.
 *
 *    @brief s Ship to get the size of.
 * @return Size of the ship.
 */
int ship_size( const Ship *s )
{
   switch (s->class) {
      case SHIP_CLASS_YACHT:
      case SHIP_CLASS_COURIER:
      case SHIP_CLASS_SCOUT:
      case SHIP_CLASS_INTERCEPTOR:
      case SHIP_CLASS_FIGHTER:
      case SHIP_CLASS_BOMBER:
         return 1;

      case SHIP_CLASS_FREIGHTER:
      case SHIP_CLASS_ARMOURED_TRANSPORT:
      case SHIP_CLASS_CORVETTE:
      case SHIP_CLASS_DESTROYER:
         return 2;

      case SHIP_CLASS_BULK_CARRIER:
      case SHIP_CLASS_CRUISER:
      case SHIP_CLASS_BATTLESHIP:
      case SHIP_CLASS_CARRIER:
         return 3;

      default:
         return -1;
   }
}


/**
 * @brief Generates a target graphic for a ship.
 */
static int ship_genTargetGFX( Ship *temp, SDL_Surface *surface, int sx, int sy )
{
   SDL_Surface *gfx, *gfx_store;
   int x, y, sw, sh;
   SDL_Rect rtemp, dstrect;
#if 0 /* Required for scanlines. */
   int i, j;
   uint32_t *pix;
   double r, g, b, a;
   double h, s, v;
#endif
   char buf[PATH_MAX];

   /* Get sprite size. */
   sw = temp->gfx_space->w / sx;
   sh = temp->gfx_space->h / sy;

   /* Create the surface. */
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* create the temp POT surface */
   gfx = SDL_CreateRGBSurface( 0, sw, sh,
         surface->format->BytesPerPixel*8, RGBAMASK );
   gfx_store = SDL_CreateRGBSurface( 0, SHIP_TARGET_W, SHIP_TARGET_H,
         surface->format->BytesPerPixel*8, RGBAMASK );

   if (gfx == NULL) {
      WARN( _("Unable to create ship '%s' targeting surface."), temp->name );
      return -1;
   }

   /* Copy over for target. */
   gl_getSpriteFromDir( &x, &y, temp->gfx_space, M_PI* 5./4. );
   rtemp.x = sw * x;
   rtemp.y = sh * y;
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

   /* Load the store surface. */
   snprintf( buf, sizeof(buf), "%s_gfx_store", temp->name );
   temp->gfx_store = gl_loadImagePad( buf, gfx_store, OPENGL_TEX_VFLIP, SHIP_TARGET_W, SHIP_TARGET_H, 1, 1, 1 );

   /* Load the surface. */
   snprintf( buf, sizeof(buf), "%s_gfx_target", temp->name );
   temp->gfx_target = gl_loadImagePad( buf, gfx, OPENGL_TEX_VFLIP, sw, sh, 1, 1, 1 );

   return 0;
}


/**
 * @brief Loads the space graphics for a ship from an image.
 *
 *    @param temp Ship to load into.
 *    @param str Path of the image to use.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 */
static int ship_loadSpaceImage( Ship *temp, char *str, int sx, int sy )
{
   SDL_RWops *rw;
   SDL_Surface *surface;
   int ret;

   /* Load the space sprite. */
   rw    = PHYSFSRWOPS_openRead( str );
   if (rw==NULL) {
      WARN(_("Unable to open '%s' for reading!"), str);
      return -1;
   }
   surface = IMG_Load_RW( rw, 0 );

   /* Load the texture. */
   temp->gfx_space = gl_loadImagePadTrans( str, surface, rw,
         OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS | OPENGL_TEX_VFLIP,
         surface->w, surface->h, sx, sy, 0 );

   /* Create the target graphic. */
   ret = ship_genTargetGFX( temp, surface, sx, sy );
   if (ret != 0)
      return ret;

   /* Free stuff. */
   SDL_RWclose( rw );
   SDL_FreeSurface( surface );

   /* Calculate mount angle. */
   temp->mangle  = 2.*M_PI;
   temp->mangle /= temp->gfx_space->sx * temp->gfx_space->sy;
   return 0;
}


/**
 * @brief Loads the space graphics for a ship from an image.
 *
 *    @param temp Ship to load into.
 *    @param str Path of the image to use.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 */
static int ship_loadEngineImage( Ship *temp, char *str, int sx, int sy )
{
   temp->gfx_engine = gl_newSprite( str, sx, sy, OPENGL_TEX_MIPMAPS );
   return (temp->gfx_engine != NULL);
}


/**
 * @brief Loads the graphics for a ship.
 *
 *    @param temp Ship to load into.
 *    @param buf Name of the texture to work with.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 *    @param engine Whether there is also an engine image to load.
 */
static int ship_loadGFX( Ship *temp, const char *buf, int sx, int sy, int engine )
{
   char str[PATH_MAX], *ext, *base, *delim;

   /* Get base path. */
   delim = strchr( buf, '_' );
   base = delim==NULL ? strdup( buf ) : strndup( buf, delim-buf );

   ext = ".webp";
   snprintf( str, sizeof(str), SHIP_GFX_PATH"%s/%s%s", base, buf, ext );
   if (!PHYSFS_exists(str)) {
      ext = ".png";
      snprintf( str, sizeof(str), SHIP_GFX_PATH"%s/%s%s", base, buf, ext );
   }
   ship_loadSpaceImage( temp, str, sx, sy );

   /* Load the engine sprite .*/
   if (engine) {
      snprintf( str, sizeof(str), SHIP_GFX_PATH"%s/%s"SHIP_ENGINE"%s", base, buf, ext );
      ship_loadEngineImage( temp, str, sx, sy );
      if (temp->gfx_engine == NULL)
         WARN(_("Ship '%s' does not have an engine sprite (%s)."), temp->name, str );
   }

   /* Get the comm graphic for future loading. */
   asprintf( &temp->gfx_comm, SHIP_GFX_PATH"%s/%s"SHIP_COMM"%s", base, buf, ext );
   free( base );

   return 0;
}


/**
 * @brief Loads the collision polygon for a ship.
 *
 *    @param temp Ship to load into.
 *    @param buf Name of the file.
 *    @param size_hint Expected array length required.
 */
static int ship_loadPLG( Ship *temp, const char *buf, int size_hint )
{
   char *file;
   CollPoly *polygon;
   xmlDocPtr doc;
   xmlNodePtr node, cur;

   asprintf( &file, "%s%s.xml", SHIP_POLYGON_PATH, buf );

   /* See if the file does exist. */
   if (!PHYSFS_exists(file)) {
      WARN(_("%s xml collision polygon does not exist!\n \
               Please use the script 'polygon_from_sprite.py' if sprites are used,\n \
               And 'polygonSTL.py' if 3D model is used in game.\n \
               These files can be found in Naev's artwork repo."), file);
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
         temp->polygon = array_create_size( CollPoly, size_hint );
         do {
            if (xml_isNode(cur,"polygon")) {
               polygon = &array_grow( &temp->polygon );
               LoadPolygon( polygon, cur );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   return 0;
}


/**
 * @brief Parses a slot for a ship.
 *
 *    @param temp Ship to be parsed.
 *    @param slot Slot being parsed.
 *    @param type Type of the slot.
 *    @param node Node containing the data.
 *    @return 0 on success.
 */
static int ship_parseSlot( Ship *temp, ShipOutfitSlot *slot, OutfitSlotType type, xmlNodePtr node )
{
   OutfitSlotSize base_size;
   char *buf;
   Outfit *o;

   /* Initialize. */
   memset( slot, 0, sizeof(ShipOutfitSlot) );
   /* Parse size. */
   xmlr_attr_strd( node, "size", buf );
   if (buf != NULL)
      base_size = outfit_toSlotSize( buf );
   else {
      WARN(_("Ship '%s' has undefined slot size, setting to '%s'"),temp->name, "Small");
      base_size = OUTFIT_SLOT_SIZE_LIGHT;
   }
   free(buf);

   /* Get mount point for weapons. */
   if (type == OUTFIT_SLOT_WEAPON) {
      xmlr_attr_float( node, "x", slot->mount.x );
      xmlr_attr_float( node, "y", slot->mount.y );
      /* Since we measure in pixels, we have to modify it so it
       *  doesn't get corrected by the ortho correction. */
      slot->mount.y *= M_SQRT2;
      xmlr_attr_float( node, "h", slot->mount.h );
   }

   /* Parse property. */
   xmlr_attr_strd( node, "prop", buf );
   if (buf != NULL) {
      slot->slot.spid = sp_get( buf );
      slot->exclusive = sp_exclusive( slot->slot.spid );
      slot->required  = sp_required( slot->slot.spid );
      free( buf );
   }
   //TODO: consider inserting those two parse blocks below inside the parse block above

   /* Parse exclusive flag, default false. */
   xmlr_attr_int( node, "exclusive", slot->exclusive );
   /* TODO: decide if exclusive should even belong in ShipOutfitSlot,
    * remove this hack, and fix slot->exclusive to slot->slot.exclusive
    * in it's two previous occurrences, meaning three lines above and 12
    * lines above */
   /* hack */
   slot->slot.exclusive = slot->exclusive;

   /* Parse required flag, default false. */
   xmlr_attr_int( node, "required", slot->required );

   /* Parse default outfit. */
   buf = xml_get(node);
   if (buf != NULL) {
      o = outfit_get( buf );
      if (o == NULL)
         WARN( _("Ship '%s' has default outfit '%s' which does not exist."), temp->name, buf );
      slot->data = o;
   }

   /* Set stuff. */
   slot->slot.size = base_size;
   slot->slot.type = type;

   /* Required slots need a default outfit. */
   if (slot->required && (slot->data == NULL))
      WARN(_("Ship '%s' has required slot without a default outfit."), temp->name);

   return 0;
}

/**
 * @brief Extracts the in-game ship from an XML node.
 *
 *    @param temp Ship to load data into.
 *    @param parent Node to get ship from.
 *    @return 0 on success.
 */
static int ship_parse( Ship *temp, xmlNodePtr parent )
{
   int i;
   xmlNodePtr cur, node;
   int sx, sy;
   char *buf;
   char str[PATH_MAX];
   int noengine;
   ShipStatList *ll;
   ShipTrailEmitter trail;

   /* Clear memory. */
   memset( temp, 0, sizeof(Ship) );

   /* Defaults. */
   ss_statsInit( &temp->stats_array );

   /* Get name. */
   xmlr_attr_strd( parent, "name", temp->name );
   if (temp->name == NULL)
      WARN( _("Ship in %s has invalid or no name"), SHIP_DATA_PATH );

   /* Datat that must be loaded first. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      if (xml_isNode(node,"class")) {
         xmlr_attr_strd( node, "display", temp->class_display );
         temp->class = ship_classFromString( xml_get(node) );
         continue;
      }
   } while (xml_nextNode(node));

   /* Default offsets for the engine. */
   temp->trail_emitters = NULL;

   /* Load the rest of the data. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"GFX")) {

         /* Get base graphic name. */
         buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': GFX element is NULL"), temp->name);
            continue;
         }

         /* Get sprite size. */
         xmlr_attr_int_def( node, "sx", sx, 8 );
         xmlr_attr_int_def( node, "sy", sy, 8 );

         xmlr_attr_int(node, "noengine", noengine );

         /* Load the graphics. */
         ship_loadGFX( temp, buf, sx, sy, !noengine );

         /* Load the polygon. */
         ship_loadPLG( temp, buf, sx*sy );

         /* Validity check: there must be 1 polygon per sprite. */
         if (array_size(temp->polygon) != sx*sy) {
            WARN(_("Ship '%s': the number of collision polygons is wrong.\n \
                    npolygon = %i and sx*sy = %i"),
                    temp->name, array_size(temp->polygon), sx*sy);
         }

         continue;
      }

      if (xml_isNode(node,"gfx_space")) {

         /* Get path */
         buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': gfx_space element is NULL"), temp->name);
            continue;
         }
         snprintf( str, sizeof(str), GFX_PATH"%s", buf );

         /* Get sprite size. */
         xmlr_attr_int_def( node, "sx", sx, 8 );
         xmlr_attr_int_def( node, "sy", sy, 8 );

         /* Load the graphics. */
         ship_loadSpaceImage( temp, str, sx, sy );

         continue;
      }

      if (xml_isNode(node,"gfx_engine")) {

         /* Get path */
         buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': gfx_engine element is NULL"), temp->name);
            continue;
         }
         snprintf( str, sizeof(str), GFX_PATH"%s", buf );

         /* Get sprite size. */
         xmlr_attr_int_def( node, "sx", sx, 8 );
         xmlr_attr_int_def( node, "sy", sy, 8 );

         /* Load the graphics. */
         ship_loadEngineImage( temp, str, sx, sy );

         continue;
      }

      if (xml_isNode(node,"gfx_comm")) {
         /* Get path */
         buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': gfx_comm element is NULL"), temp->name);
            continue;
         }
         snprintf( str, sizeof(str), GFX_PATH"%s", buf );
         temp->gfx_comm = strdup(str);
         continue;
      }
      if (xml_isNode(node,"gfx_overlays")) {
         cur = node->children;
         temp->gfx_overlays = array_create_size( glTexture*, 2 );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"gfx_overlay"))
               array_push_back( &temp->gfx_overlays,
                     xml_parseTexture( cur, OVERLAY_GFX_PATH"%s", 1, 1, OPENGL_TEX_MIPMAPS ) );
         } while (xml_nextNode(cur));
         continue;
      }

      xmlr_strd(node,"GUI",temp->gui);
      if (xml_isNode(node,"sound")) {
         temp->sound = sound_get( xml_get(node) );
         continue;
      }
      xmlr_strd(node,"base_type",temp->base_type);
      if (xml_isNode(node,"class")) {
         /* Already preemptively loaded, avoids warning. */
         continue;
      }
      xmlr_float(node,"time_mod",temp->dt_default);
      xmlr_long(node,"price",temp->price);
      xmlr_strd(node,"license",temp->license);
      xmlr_strd(node,"fabricator",temp->fabricator);
      xmlr_strd(node,"description",temp->description);
      xmlr_int(node,"rarity",temp->rarity);

      if (xml_isNode(node,"trail_generator")) {
         xmlr_attr_float( node, "x", trail.x_engine );
         xmlr_attr_float( node, "y", trail.y_engine );
         xmlr_attr_float( node, "h", trail.h_engine );
         xmlr_attr_int_def( node, "always_under", trail.always_under, 0 );
         if (temp->trail_emitters == NULL) {
            temp->trail_emitters = array_create( ShipTrailEmitter );
         }
         buf = xml_get(node);
         if (buf == NULL)
            buf = "default";
         trail.trail_spec = trailSpec_get( buf );
         if (trail.trail_spec != NULL)
            array_push_back( &temp->trail_emitters, trail );
         continue;
      }

      if (xml_isNode(node,"movement")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_float(cur,"thrust",temp->thrust);
            xmlr_float(cur,"turn",temp->turn);
            xmlr_float(cur,"speed",temp->speed);
            /* All the xmlr_ stuff have continue cases. */
            WARN(_("Ship '%s' has unknown movement node '%s'."), temp->name, cur->name);
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
            WARN(_("Ship '%s' has unknown health node '%s'."), temp->name, cur->name);
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
            xmlr_int(cur,"fuel_consumption",temp->fuel_consumption);
            xmlr_float(cur,"cargo",temp->cap_cargo);
            /* All the xmlr_ stuff have continue cases. */
            WARN(_("Ship '%s' has unknown characteristic node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"slots")) {
         /* Allocate the space. */
         temp->outfit_structure  = array_create( ShipOutfitSlot );
         temp->outfit_utility    = array_create( ShipOutfitSlot );
         temp->outfit_weapon     = array_create( ShipOutfitSlot );

         /* Initialize the mounts. */
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"structure"))
               ship_parseSlot( temp, &array_grow(&temp->outfit_structure), OUTFIT_SLOT_STRUCTURE, cur );
            else if (xml_isNode(cur,"utility"))
               ship_parseSlot( temp, &array_grow(&temp->outfit_utility), OUTFIT_SLOT_UTILITY, cur );
            else if (xml_isNode(cur,"weapon"))
               ship_parseSlot( temp, &array_grow(&temp->outfit_weapon), OUTFIT_SLOT_WEAPON, cur );
            else
               WARN(_("Ship '%s' has unknown slot node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         array_shrink( &temp->outfit_structure );
         array_shrink( &temp->outfit_utility );
         array_shrink( &temp->outfit_weapon );
         continue;
      }

      /* Parse ship stats. */
      if (xml_isNode(node,"stats")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            ll = ss_listFromXML( cur );
            if (ll != NULL) {
               ll->next    = temp->stats;
               temp->stats = ll;
               continue;
            }
            WARN(_("Ship '%s' has unknown stat '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));

         /* Load array. */
         ss_statsInit( &temp->stats_array );
         ss_statsModFromList( &temp->stats_array, temp->stats );

         /* Create description. */
         if (temp->stats != NULL) {
            temp->desc_stats = malloc( STATS_DESC_MAX );
            i = ss_statsListDesc( temp->stats, temp->desc_stats, STATS_DESC_MAX, 0 );
            if (i <= 0) {
               free( temp->desc_stats );
               temp->desc_stats = NULL;
            }
         }

         continue;
      }

      /* Used by on-valid and NSH utils, no in-game meaning. */
      if (xml_isNode(node,"mission"))
         continue;

      DEBUG(_("Ship '%s' has unknown node '%s'."), temp->name, node->name);
   } while (xml_nextNode(node));

   /* Post processing. */
   temp->dmg_absorb   /= 100.;
   temp->turn         *= M_PI / 180.; /* Convert to rad. */

   /* ship validator */
#define MELEMENT(o,s)      if (o) WARN( _("Ship '%s' missing '%s' element"), temp->name, s)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->base_type==NULL,"base_type");
   MELEMENT((temp->gfx_space==NULL) || (temp->gfx_comm==NULL),"GFX");
   MELEMENT(temp->gui==NULL,"GUI");
   MELEMENT(temp->class==SHIP_CLASS_NULL,"class");
   MELEMENT(temp->price==0,"price");
   MELEMENT(temp->dt_default==0.,"time_mod");
   MELEMENT(temp->fabricator==NULL,"fabricator");
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->armour==0.,"armour");
   /*MELEMENT(temp->thrust==0.,"thrust");
   MELEMENT(temp->turn==0.,"turn");
   MELEMENT(temp->speed==0.,"speed");
   MELEMENT(temp->shield==0.,"shield");
   MELEMENT(temp->shield_regen==0.,"shield_regen");
   MELEMENT(temp->energy==0.,"energy");
   MELEMENT(temp->energy_regen==0.,"energy_regen");
   MELEMENT(temp->fuel==0.,"fuel");*/
   MELEMENT(temp->crew==0,"crew");
   MELEMENT(temp->mass==0.,"mass");
   MELEMENT(temp->fuel_consumption==0,"fuel_consumption");
   /*MELEMENT(temp->cap_cargo==0,"cargo");
   MELEMENT(temp->cpu==0.,"cpu");*/
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
   size_t nfiles;
   char **ship_files, *file;
   int i;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Validity. */
   ss_check();

   ship_files = PHYSFS_enumerateFiles( SHIP_DATA_PATH );
   for (nfiles=0; ship_files[nfiles]!=NULL; nfiles++) {}

   /* Initialize stack if needed. */
   if (ship_stack == NULL)
      ship_stack = array_create_size(Ship, nfiles);

   for (i=0; ship_files[i]!=NULL; i++) {
      if (!ndata_matchExt( ship_files[i], "xml" ))
         continue;

      /* Get the file name .*/
      asprintf( &file, "%s%s", SHIP_DATA_PATH, ship_files[i] );

      /* Load the XML. */
      doc  = xml_parsePhysFS( file );

      if (doc == NULL) {
         free(file);
         continue;
      }

      node = doc->xmlChildrenNode; /* First ship node */
      if (node == NULL) {
         xmlFreeDoc(doc);
         WARN(_("Malformed %s file: does not contain elements"), file);
         free(file);
         continue;
      }

      free(file);

      if (xml_isNode(node, XML_SHIP))
         /* Load the ship. */
         ship_parse( &array_grow(&ship_stack), node );

      /* Clean up. */
      xmlFreeDoc(doc);
   }

   /* Shrink stack. */
   array_shrink(&ship_stack);
   DEBUG( n_( "Loaded %d Ship", "Loaded %d Ships", array_size(ship_stack) ), array_size(ship_stack) );

   /* Clean up. */
   PHYSFS_freeList( ship_files );

   return 0;
}


/**
 * @brief Frees all the ships.
 */
void ships_free (void)
{
   Ship *s;
   int i, j;
   for (i = 0; i < array_size(ship_stack); i++) {
      s = &ship_stack[i];

      /* Free stored strings. */
      free(s->name);
      free(s->class_display);
      free(s->description);
      free(s->gui);
      free(s->base_type);
      free(s->fabricator);
      free(s->license);
      free(s->desc_stats);

      /* Free outfits. */
      for (j=0; j<array_size(s->outfit_structure); j++)
         outfit_freeSlot( &s->outfit_structure[j].slot );
      for (j=0; j<array_size(s->outfit_utility); j++)
         outfit_freeSlot( &s->outfit_utility[j].slot );
      for (j=0; j<array_size(s->outfit_weapon); j++)
         outfit_freeSlot( &s->outfit_weapon[j].slot );
      array_free(s->outfit_structure);
      array_free(s->outfit_utility);
      array_free(s->outfit_weapon);

      ss_free( s->stats );

      /* Free graphics. */
      gl_freeTexture(s->gfx_space);
      gl_freeTexture(s->gfx_engine);
      gl_freeTexture(s->gfx_target);
      gl_freeTexture(s->gfx_store);
      free(s->gfx_comm);
      for (j=0; j<array_size(s->gfx_overlays); j++)
         gl_freeTexture(s->gfx_overlays[j]);
      array_free(s->gfx_overlays);

      /* Free collision polygons. */
      for (j=0; j<array_size(s->polygon); j++) {
         free(s->polygon[j].x);
         free(s->polygon[j].y);
      }

      array_free(s->trail_emitters);
      array_free(s->polygon);
   }

   array_free(ship_stack);
   ship_stack = NULL;
}
