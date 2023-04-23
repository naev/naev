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
#include "nlua.h"
#include "nlua_gfx.h"
#include "nlua_camera.h"
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
static int ship_parse( Ship *temp, const char *filename );
static void ship_freeSlot( ShipOutfitSlot* s );

/**
 * @brief Compares two ship pointers for qsort.
 */
static int ship_cmp( const void *p1, const void *p2 )
{
   const Ship *s1, *s2;
   s1 = (const Ship*) p1;
   s2 = (const Ship*) p2;
   return strcmp( s1->name, s2->name );
}

/**
 * @brief Gets a ship based on its name.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
const Ship* ship_get( const char* name )
{
   const Ship *s = ship_getW( name );
   if (s==NULL)
      WARN(_("Ship %s does not exist"), name);
   return s;
}

/**
 * @brief Gets a ship based on its name without warning.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
const Ship* ship_getW( const char* name )
{
   const Ship s = {.name = (char*)name };
   return bsearch( &s, ship_stack, array_size(ship_stack), sizeof(Ship), ship_cmp );
}

/**
 * @brief Checks to see if an ship exists matching name (case insensitive).
 */
const char *ship_existsCase( const char* name )
{
   for (int i=0; i<array_size(ship_stack); i++)
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

   /* Compare requirements. */
   if ((s1->condstr!=NULL) && (s2->condstr==NULL))
      return -1;
   else if ((s2->condstr!=NULL) && (s1->condstr==NULL))
      return +1;

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
         return N_("Yacht");
      case SHIP_CLASS_COURIER:
         return N_("Courier");
      case SHIP_CLASS_FREIGHTER:
         return N_("Freighter");
      case SHIP_CLASS_ARMOURED_TRANSPORT:
         return N_("Armoured Transport");
      case SHIP_CLASS_BULK_FREIGHTER:
         return N_("Bulk Freighter");
      /* Military. */
      case SHIP_CLASS_SCOUT:
         return N_("Scout");
      case SHIP_CLASS_INTERCEPTOR:
         return N_("Interceptor");
      case SHIP_CLASS_FIGHTER:
         return N_("Fighter");
      case SHIP_CLASS_BOMBER:
         return N_("Bomber");
      case SHIP_CLASS_CORVETTE:
         return N_("Corvette");
      case SHIP_CLASS_DESTROYER:
         return N_("Destroyer");
      case SHIP_CLASS_CRUISER:
         return N_("Cruiser");
      case SHIP_CLASS_BATTLESHIP:
         return N_("Battleship");
      case SHIP_CLASS_CARRIER:
         return N_("Carrier");
      /* Unknown. */
      default:
         return N_("Unknown");
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
   if (str==NULL)
      return SHIP_CLASS_NULL;
   /* Civilian */
   STRTOSHIP( "Yacht",              SHIP_CLASS_YACHT );
   STRTOSHIP( "Courier",            SHIP_CLASS_COURIER );
   STRTOSHIP( "Freighter",          SHIP_CLASS_FREIGHTER );
   STRTOSHIP( "Armoured Transport", SHIP_CLASS_ARMOURED_TRANSPORT );
   STRTOSHIP( "Bulk Freighter",     SHIP_CLASS_BULK_FREIGHTER );

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

   /* Unknown */
   return SHIP_CLASS_NULL;
}
#undef STRTOSHIP

/**
 * @brief Gets the ship's base price (no outfits).
 */
credits_t ship_basePrice( const Ship* s )
{
   return s->price;
}

/**
 * @brief The ship buy price, includes default outfits.
 */
credits_t ship_buyPrice( const Ship* s )
{
   /* Get base price. */
   credits_t price = ship_basePrice(s);

   for (int i=0; i<array_size(s->outfit_structure); i++) {
      const Outfit *o = s->outfit_structure[i].data;
      if (o != NULL)
         price += o->price;
   }
   for (int i=0; i<array_size(s->outfit_utility); i++) {
      const Outfit *o = s->outfit_utility[i].data;
      if (o != NULL)
         price += o->price;
   }
   for (int i=0; i<array_size(s->outfit_weapon); i++) {
      const Outfit *o = s->outfit_weapon[i].data;
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
glTexture* ship_loadCommGFX( const Ship* s )
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
      case SHIP_CLASS_SCOUT:
      case SHIP_CLASS_INTERCEPTOR:
         return 1;

      case SHIP_CLASS_COURIER:
      case SHIP_CLASS_FIGHTER:
      case SHIP_CLASS_BOMBER:
         return 2;

      case SHIP_CLASS_FREIGHTER:
      case SHIP_CLASS_CORVETTE:
         return 3;

      case SHIP_CLASS_DESTROYER:
      case SHIP_CLASS_ARMOURED_TRANSPORT:
         return 4;

      case SHIP_CLASS_BULK_FREIGHTER:
      case SHIP_CLASS_CRUISER:
         return 5;

      case SHIP_CLASS_BATTLESHIP:
      case SHIP_CLASS_CARRIER:
         return 6;

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
   /* Don't try to be smart here and avoid loading the transparency map or
    * we'll hit issues with collisions. */
   /*
   if (temp->polygon != NULL)
      temp->gfx_space = gl_loadImagePad( str, surface,
            OPENGL_TEX_MIPMAPS | OPENGL_TEX_VFLIP,
            surface->w, surface->h, sx, sy, 0 );
   else
   */
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

   /* Load the 3d model */
   snprintf(str, sizeof(str), SHIP_3DGFX_PATH"%s/%s/%s.obj", base, buf, buf);
   if (PHYSFS_exists(str)) {
      temp->gfx_3d = object_loadFromFile(str);
   }

   /* Load the space sprite. */
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
   if (temp->gfx_comm != NULL) {
      WARN(_("Ship '%s' has doubly defined 'gfx_comm'!"),temp->name);
      free(temp->gfx_comm);
   }
   SDL_asprintf( &temp->gfx_comm, SHIP_GFX_PATH"%s/%s"SHIP_COMM"%s", base, buf, ext );
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
   char file[PATH_MAX];
   CollPoly *polygon;
   xmlDocPtr doc;
   xmlNodePtr node;

   snprintf( file, sizeof(file), "%s%s.xml", SHIP_POLYGON_PATH, buf );

   /* See if the file does exist. */
   if (!PHYSFS_exists(file)) {
      WARN(_("%s xml collision polygon does not exist!\n \
               Please use the script 'polygon_from_sprite.py' if sprites are used,\n \
               And 'polygonSTL.py' if 3D model is used in game.\n \
               These files can be found in Naev's artwork repo."), file);
      return 0;
   }

   /* Load the XML. */
   doc  = xml_parsePhysFS( file );
   if (doc == NULL)
      return 0;

   node = doc->xmlChildrenNode; /* First polygon node */
   if (node == NULL) {
      xmlFreeDoc(doc);
      WARN(_("Malformed %s file: does not contain elements"), file);
      return 0;
   }

   do { /* load the polygon data */
      if (xml_isNode(node,"polygons")) {
         xmlNodePtr cur = node->children;
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
      slot->locked    = sp_locked( slot->slot.spid );
      free( buf );
   }
   //TODO: consider inserting those two parse blocks below inside the parse block above

   /* Parse exclusive flag, default false. */
   xmlr_attr_int_def( node, "exclusive", slot->exclusive, slot->exclusive );
   /* TODO: decide if exclusive should even belong in ShipOutfitSlot,
    * remove this hack, and fix slot->exclusive to slot->slot.exclusive
    * in it's two previous occurrences, meaning three lines above and 12
    * lines above */
   /* hack */
   slot->slot.exclusive = slot->exclusive;

   /* Parse required flag, default false. */
   xmlr_attr_int_def( node, "required", slot->required, slot->required );

   /* Parse locked flag, default false. */
   xmlr_attr_int_def( node, "locked", slot->locked, slot->locked );

   /* Name if applicable. */
   xmlr_attr_strd( node, "name", slot->name );

   /* Parse default outfit. */
   buf = xml_get(node);
   if (buf != NULL) {
      const Outfit *o = outfit_get( buf );
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
 *    @param filename File to load ship from.
 *    @return 0 on success.
 */
static int ship_parse( Ship *temp, const char *filename )
{
   xmlNodePtr parent, node;
   xmlDocPtr doc;
   int sx, sy;
   char str[PATH_MAX];
   int noengine;
   ShipStatList *ll;
   ShipTrailEmitter trail;

   /* Load the XML. */
   doc  = xml_parsePhysFS( filename );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* First ship node */
   if (parent == NULL) {
      xmlFreeDoc(doc);
      WARN(_("Malformed %s file: does not contain elements"), filename);
      return -1;
   }

   /* Clear memory. */
   memset( temp, 0, sizeof(Ship) );

   /* Defaults. */
   ss_statsInit( &temp->stats_array );
   temp->dt_default = 1.;

   /* Lua defaults. */
   temp->lua_env     = LUA_NOREF;
   temp->lua_init    = LUA_NOREF;
   temp->lua_cleanup = LUA_NOREF;
   temp->lua_update  = LUA_NOREF;
   temp->lua_explode_init = LUA_NOREF;
   temp->lua_explode_update = LUA_NOREF;

   /* Get name. */
   xmlr_attr_strd( parent, "name", temp->name );
   if (temp->name == NULL)
      WARN( _("Ship in %s has invalid or no name"), SHIP_DATA_PATH );

   /* Default offsets for the engine. */
   temp->trail_emitters = NULL;

   /* Load the rest of the data. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"class")) {
         xmlr_attr_strd( node, "display", temp->class_display );
         temp->class = ship_classFromString( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"GFX")) {
         /* Get base graphic name. */
         char *buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': GFX element is NULL"), temp->name);
            continue;
         }

         /* Get size. */
         xmlr_attr_float_def(node, "size", temp->gfx_3d_scale, 1);
         xmlr_attr_int_def( node, "sx", sx, 8 );
         xmlr_attr_int_def( node, "sy", sy, 8 );

         xmlr_attr_int(node, "noengine", noengine );

         /* Load the polygon. */
         ship_loadPLG( temp, buf, sx*sy );

         /* Load the graphics. */
         ship_loadGFX( temp, buf, sx, sy, !noengine );

         /* Validity check: there must be 1 polygon per sprite. */
         if ((temp->polygon != NULL) && array_size(temp->polygon) != sx*sy) {
            WARN(_("Ship '%s': the number of collision polygons is wrong.\n \
                    npolygon = %i and sx*sy = %i"),
                    temp->name, array_size(temp->polygon), sx*sy);
         }

         continue;
      }

      if (xml_isNode(node,"gfx_space")) {
         /* Get path */
         char *buf = xml_get(node);
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
         char *buf = xml_get(node);
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
         char *buf = xml_get(node);
         if (buf==NULL) {
            WARN(_("Ship '%s': gfx_comm element is NULL"), temp->name);
            continue;
         }
         snprintf( str, sizeof(str), GFX_PATH"%s", buf );
         if (temp->gfx_comm != NULL) {
            WARN(_("Ship '%s' has doubly defined 'gfx_comm'!"),temp->name);
            free(temp->gfx_comm);
         }
         temp->gfx_comm = strdup(str);
         continue;
      }
      if (xml_isNode(node,"gfx_overlays")) {
         xmlNodePtr cur = node->children;
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
         xmlr_attr_float_def( node, "pitch", temp->engine_pitch, 1. );
         temp->sound = sound_get( xml_get(node) );
         continue;
      }
      xmlr_strd(node,"base_type",temp->base_type);
      xmlr_float(node,"time_mod",temp->dt_default);
      xmlr_long(node,"price",temp->price);
      xmlr_strd(node,"license",temp->license);
      xmlr_strd(node,"cond",temp->cond);
      xmlr_strd(node,"condstr",temp->condstr);
      xmlr_strd(node,"fabricator",temp->fabricator);
      xmlr_strd(node,"description",temp->description);
      xmlr_int(node,"points",temp->points);
      xmlr_int(node,"rarity",temp->rarity);
      xmlr_strd(node,"lua",temp->lua_file);

      if (xml_isNode(node,"flags")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"noplayer")) {
               ship_setFlag( temp, SHIP_NOPLAYER );
               continue;
            }
            if (xml_isNode(cur,"noescort")) {
               ship_setFlag( temp, SHIP_NOESCORT );
               continue;
            }
            if (xml_isNode(cur,"unique")) {
               ship_setFlag( temp, SHIP_UNIQUE );
               continue;
            }
            WARN(_("Ship '%s' has unknown flags node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"trail_generator")) {
         char *buf;
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
         xmlNodePtr cur = node->children;
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
         xmlNodePtr cur = node->children;
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
         xmlNodePtr cur = node->children;
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
         xmlNodePtr cur = node->children;
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
         xmlNodePtr cur = node->children;
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
            int i;
            temp->desc_stats = malloc( STATS_DESC_MAX );
            i = ss_statsListDesc( temp->stats, temp->desc_stats, STATS_DESC_MAX, 0 );
            if (i <= 0) {
               free( temp->desc_stats );
               temp->desc_stats = NULL;
            }
         }

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
            WARN(_("Ship '%s' has unknown node in tags '%s'."), temp->name, cur->name );
         } while (xml_nextNode(cur));
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

   /* Check license. */
   if (temp->license && !outfit_licenseExists(temp->license))
      WARN(_("Ship '%s' has inexistent license requirement '%s'!"), temp->name, temp->license);

   /* ship validator */
#define MELEMENT(o,s)      if (o) WARN( _("Ship '%s' missing '%s' element"), temp->name, s)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->base_type==NULL,"base_type");
   MELEMENT((temp->gfx_space==NULL) || (temp->gfx_comm==NULL),"GFX");
   MELEMENT(temp->gui==NULL,"GUI");
   MELEMENT(temp->class==SHIP_CLASS_NULL,"class");
   MELEMENT(temp->points==0,"points");
   MELEMENT(temp->price==0,"price");
   MELEMENT(temp->dt_default<=0.,"time_mod");
   MELEMENT(temp->fabricator==NULL,"fabricator");
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->armour==0.,"armour");
   MELEMENT((temp->cond!=NULL) && (temp->condstr==NULL), "condstr");
   MELEMENT((temp->cond==NULL) && (temp->condstr!=NULL), "cond");
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

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Loads all the ships in the data files.
 *
 *    @return 0 on success.
 */
int ships_load (void)
{
   char **ship_files;
   int nfiles;
   Uint32 time = SDL_GetTicks();

   /* Validity. */
   ss_check();

   ship_files = ndata_listRecursive( SHIP_DATA_PATH );
   nfiles = array_size( ship_files );

   /* Initialize stack if needed. */
   if (ship_stack == NULL)
      ship_stack = array_create_size(Ship, nfiles);

   /* First pass to load data. */
   for (int i=0; i<nfiles; i++) {
      if (ndata_matchExt( ship_files[i], "xml" )) {
         /* Load the ship. */
         Ship s;
         int ret = ship_parse( &s, ship_files[i] );
         if (ret == 0)
            array_push_back( &ship_stack, s );

         /* Render if necessary. */
         naev_renderLoadscreen();
      }

      /* Clean up. */
      free( ship_files[i] );
   }
   qsort( ship_stack, array_size(ship_stack), sizeof(Ship), ship_cmp );

#if DEBUGGING
   /* Check to see if there are name collisions. */
   for (int i=1; i<array_size(ship_stack); i++)
      if (strcmp( ship_stack[i-1].name, ship_stack[i].name )==0)
         WARN(_("Duplicated ship name '%s' detected!"), ship_stack[i].name);
#endif /* DEBUGGING */

   /* Shrink stack. */
   array_shrink(&ship_stack);

   /* Second pass to load Lua. */
   for (int i=0; i<array_size(ship_stack); i++) {
      Ship *s = &ship_stack[i];
      if (s->lua_file==NULL)
         continue;

      nlua_env env;
      size_t sz;
      char *dat = ndata_read( s->lua_file, &sz );
      if (dat==NULL) {
         WARN(_("Ship '%s' failed to read Lua '%s'!"), s->name, s->lua_file );
         continue;
      }

      env = nlua_newEnv();
      s->lua_env = env;
      /* TODO limit libraries here. */
      nlua_loadStandard( env );
      nlua_loadGFX( env );
      nlua_loadCamera( env );

      /* Run code. */
      if (nlua_dobufenv( env, dat, sz, s->lua_file ) != 0) {
         WARN(_("Ship '%s' Lua error:\n%s"), s->name, lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         nlua_freeEnv( s->lua_env );
         free( dat );
         s->lua_env = LUA_NOREF;
         continue;
      }
      free( dat );

      /* Check functions as necessary. */
      s->lua_init       = nlua_refenvtype( env, "init",     LUA_TFUNCTION );
      s->lua_cleanup    = nlua_refenvtype( env, "cleanup",  LUA_TFUNCTION );
      s->lua_update     = nlua_refenvtype( env, "update",   LUA_TFUNCTION );
      s->lua_explode_init = nlua_refenvtype( env, "explode_init", LUA_TFUNCTION );
      s->lua_explode_update = nlua_refenvtype( env, "explode_update", LUA_TFUNCTION );
   }

   /* Debugging timings. */
   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d Ship in %.3f s", "Loaded %d Ships in %.3f s", array_size(ship_stack) ), array_size(ship_stack), time/1000. );
   }
   else
      DEBUG( n_( "Loaded %d Ship", "Loaded %d Ships", array_size(ship_stack) ), array_size(ship_stack) );

   /* Clean up. */
   array_free( ship_files );

   return 0;
}

/**
 * @brief Frees all the ships.
 */
void ships_free (void)
{
   for (int i=0; i < array_size(ship_stack); i++) {
      Ship *s = &ship_stack[i];

      /* Free stored strings. */
      free(s->name);
      free(s->class_display);
      free(s->description);
      free(s->gui);
      free(s->base_type);
      free(s->fabricator);
      free(s->license);
      free(s->cond);
      free(s->condstr);
      free(s->desc_stats);

      /* Free outfits. */
      for (int j=0; j<array_size(s->outfit_structure); j++)
         ship_freeSlot( &s->outfit_structure[j] );
      for (int j=0; j<array_size(s->outfit_utility); j++)
         ship_freeSlot( &s->outfit_utility[j] );
      for (int j=0; j<array_size(s->outfit_weapon); j++)
         ship_freeSlot( &s->outfit_weapon[j] );
      array_free(s->outfit_structure);
      array_free(s->outfit_utility);
      array_free(s->outfit_weapon);

      ss_free( s->stats );

      /* Free graphics. */
      object_free(s->gfx_3d);
      gl_freeTexture(s->gfx_space);
      gl_freeTexture(s->gfx_engine);
      gl_freeTexture(s->gfx_target);
      gl_freeTexture(s->gfx_store);
      free(s->gfx_comm);
      for (int j=0; j<array_size(s->gfx_overlays); j++)
         gl_freeTexture(s->gfx_overlays[j]);
      array_free(s->gfx_overlays);

      /* Free collision polygons. */
      for (int j=0; j<array_size(s->polygon); j++) {
         free(s->polygon[j].x);
         free(s->polygon[j].y);
      }

      array_free(s->trail_emitters);
      array_free(s->polygon);

      /* Free tags. */
      for (int j=0; j<array_size(s->tags); j++)
         free(s->tags[j]);
      array_free(s->tags);

      /* Free Lua. */
      nlua_freeEnv( s->lua_env );
      s->lua_env = LUA_NOREF;
      free(s->lua_file);
   }

   array_free(ship_stack);
   ship_stack = NULL;
}

static void ship_freeSlot( ShipOutfitSlot* s )
{
   outfit_freeSlot( &s->slot );
   free( s->name );
}
