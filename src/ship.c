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


#define XML_ID    "Ships"  /**< XML document identifier */
#define XML_SHIP  "ship" /**< XML individual ship identifier. */

#define SHIP_DATA    "dat/ship.xml" /**< XML file containing ships. */
#define SHIP_GFX     "gfx/ship/" /**< Location of ship graphics. */
#define SHIP_EXT     ".png" /**< Ship graphics extension format. */
#define SHIP_TARGET  "_target" /**< Target graphic extension. */
#define SHIP_COMM    "_comm" /**< Communicatio graphic extension. */

#define VIEW_WIDTH   300 /**< Ship view window width. */
#define VIEW_HEIGHT  300 /**< Ship view window height. */

#define BUTTON_WIDTH  80 /**< Button width in ship view window. */
#define BUTTON_HEIGHT 30 /**< Button height in ship view window. */


#define CHUNK_SIZE    32 /**< Rate at which to allocate memory. */


static Ship* ship_stack = NULL; /**< Stack of ships available in the game. */
static int ship_nstack = 0; /**< Number of ships in the stack. */


/*
 * Prototypes
 */
static int ship_parse( Ship *temp, xmlNodePtr parent );


/**
 * @brief Gets a ship based on its name.
 *
 *    @param name Name to match.
 *    @return Ship matching name or NULL if not found.
 */
Ship* ship_get( const char* name )
{
   Ship* temp = ship_stack;
   int i;

   for (i=0; i < ship_nstack; i++)
      if (strcmp((temp+i)->name, name)==0) break;

   if (i == ship_nstack) /* ship does not exist, game will probably crash now */
      WARN("Ship %s does not exist", name);

   return temp+i;
}


/**
 * @brief Get the position of a ship mount point.
 *
 *    @param s Ship to get the mount point of.
 *    @param dir Direction ship is facing.
 *    @param id ID of the mount point to get.
 *    @param[out] p The position of the mount point.
 *    @return 0 on success.
 */
int ship_getMount( Ship* s, double dir, const int id, Vector2d *p )
{
   ShipMount *m;
   double cm, sm;

   /* Special case no mounts. */
   if ((id == 0) && (s->nmounts == 0)) {
      p->x = 0.;
      p->y = 0.;
      return 0;
   }

   /* Check to see if mount is valid. */
   if ((id >= s->nmounts) || (id < 0)) {
      WARN("Invalid mount point id '%d' on ship class '%s'.", id,  s->name);
      p->x = 0.;
      p->y = 0.;
      return 1;
   }

   m = &s->mounts[id];
   /* 2d rotation matrix
    * [ x' ]   [  cos  sin  ]   [ x ]
    * [ y' ] = [ -sin  cos  ] * [ y ]
    *
    * dir is inverted so that rotation is counter-clockwise.
    */
   cm = cos(-dir);
   sm = sin(-dir);
   p->x = m->x * cm + m->y * sm;
   p->y = m->x *-sm + m->y * cm;

   /* Correction for ortho perspective. */
   p->y *= M_SQRT1_2;

   /* Don't forget to add height. */
   p->y += m->h;

   return 0;
}


/**
 * @brief Gets all the ships in text form matching tech.
 *
 * You have to free all the strings created in the string array too.
 *
 *    @param[out] n Number of ships found.
 *    @param tech List of technologies to use.
 *    @param techmax Number of technologies in tech.
 *    @return An array of allocated ship names.
 */
Ship** ship_getTech( int *n, const int *tech, const int techmax )
{
   int i,j,k, num, price;
   Ship **result;
   Ship **ships;
  
   /* get available ships for tech */
   ships = malloc(sizeof(Ship*) * ship_nstack);
   num = 0;
   for (i=0; i < ship_nstack; i++) {
      if (ship_stack[i].tech <= tech[0]) { /* check vs base tech */
         ships[num] = &ship_stack[i];
         num++;
      }
      else {
         for (j=0; j<techmax; j++) 
            if (tech[j] == ship_stack[i].tech) { /* check vs special tech */
               ships[num] = &ship_stack[i];
               num++;
            }
      }
   }

   /* now sort by price */
   *n = 0;
   price = -1;
   result = malloc(sizeof(Ship*) * num);
   /* until we fill the new stack */
   for (i=0; i<num; i++) {

      /* check for cheapest */
      for (j=0; j<num; j++) {

         /* is cheapest? */
         if ((price == -1) || (ships[price]->price > ships[j]->price)) {

            /* check if already in stack */
            for (k=0; k<(*n); k++)
               if (strcmp(result[k]->name,ships[j]->name)==0)
                  break;

            /* not in stack and therefore is cheapest */
            if (k == (*n))
               price = j;
         }
      }

      /* add current cheapest to stack */
      result[i] = ships[price];
      (*n)++;
      price = -1;
   }

   /* cleanup */
   free(ships);

   return result;
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
int ship_basePrice( Ship* s )
{
   int price;
   ShipOutfit *o;

   /* Base price is ship's price minus it's outfits. */
   price = s->price;
   for (o=s->outfit; o!=NULL; o=o->next)
      price -= o->quantity * o->data->price;

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
   char buf[PATH_MAX];
   snprintf( buf, PATH_MAX, SHIP_GFX"%s"SHIP_COMM SHIP_EXT, s->gfx_comm );
   return gl_newImage( buf, 0 );
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
   xmlNodePtr cur, node;
   ShipOutfit *otemp, *ocur;
   char str[PATH_MAX];
   char* stmp;
   int id;

   /* Clear memory. */
   memset( temp, 0, sizeof(Ship) );

   /* Defaults. */
   str[0] = '\0';

   /* Get name. */
   xmlr_attr(parent,"name",temp->name);
   if (temp->name == NULL)
      WARN("Ship in "SHIP_DATA" has invalid or no name");

   /* Load data. */
   node = parent->xmlChildrenNode;
   do { /* load all the data */
      if (xml_isNode(node,"GFX")) {

         /* Load the base graphic */
         temp->gfx_space = xml_parseTexture( node,
               SHIP_GFX"%s"SHIP_EXT, 6, 6,
               OPENGL_TEX_MAPTRANS );

         /* Calculate mount angle. */
         temp->mangle  = 2.*M_PI;
         temp->mangle /= temp->gfx_space->sx * temp->gfx_space->sy;

         /* Get the comm graphic for future loading. */
         temp->gfx_comm = xml_getStrd(node);

         /* Load the target graphic. */
         xmlr_attr(node,"target",stmp);
         if (stmp != NULL) {
            snprintf( str, PATH_MAX,
                  SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, stmp);
            temp->gfx_target = gl_newImage(str, 0);
            free(stmp);
         }
         else { /* Load standard target graphic */
            snprintf( str, PATH_MAX,
                  SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, xml_get(node));
            temp->gfx_target = gl_newImage(str, 0);
         }
      }

      xmlr_strd(node,"GUI",temp->gui);
      if (xml_isNode(node,"sound")) {
         temp->sound = sound_get( xml_get(node) );
         continue;
      }
      if (xml_isNode(node,"class")) {
         temp->class = ship_classFromString( xml_get(node) );
         continue;
      }
      xmlr_int(node,"price",temp->price);
      xmlr_int(node,"tech",temp->tech);
      xmlr_strd(node,"license",temp->license);
      xmlr_strd(node,"fabricator",temp->fabricator);
      xmlr_strd(node,"description",temp->description);
      if (xml_isNode(node,"movement")) {
         cur = node->children;
         do {
            xmlr_int(cur,"thrust",temp->thrust);
            xmlr_int(cur,"turn",temp->turn);
            xmlr_int(cur,"speed",temp->speed);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"health")) {
         cur = node->children;
         do {
            xmlr_float(cur,"armour",temp->armour);
            xmlr_float(cur,"shield",temp->shield);
            xmlr_float(cur,"energy",temp->energy);
            if (xml_isNode(cur,"armour_regen"))
               temp->armour_regen = (double)(xml_getInt(cur))/60.0;
            else if (xml_isNode(cur,"shield_regen"))
               temp->shield_regen = (double)(xml_getInt(cur))/60.0;
            else if (xml_isNode(cur,"energy_regen"))
               temp->energy_regen = (double)(xml_getInt(cur))/60.0;
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"caracteristics")) {
         cur = node->children;
         do {
            xmlr_int(cur,"crew",temp->crew);
            xmlr_float(cur,"mass",temp->mass);
            xmlr_int(cur,"fuel",temp->fuel);
            xmlr_int(cur,"cap_weapon",temp->cap_weapon);
            xmlr_int(cur,"cap_cargo",temp->cap_cargo);
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"outfits")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"outfit")) {
               otemp = malloc(sizeof(ShipOutfit));
               otemp->data = outfit_get(xml_get(cur));
               stmp = xml_nodeProp(cur,"quantity");
               if (!stmp)
                  WARN("Ship '%s' is missing tag 'quantity' for outfit '%s'",
                        temp->name, otemp->data->name);
               otemp->quantity = atoi(stmp);
               free(stmp);
               otemp->next = NULL;
               
               if ((ocur=temp->outfit) == NULL) temp->outfit = otemp;
               else {
                  while (ocur->next) ocur = ocur->next;
                  ocur->next = otemp;
               }
            }
         } while (xml_nextNode(cur));
         continue;
      }
      if (xml_isNode(node,"mounts")) {
         /* First pass, get number of mounts. */
         cur = node->children;
         do {
            if (xml_isNode(cur,"mount"))
               temp->nmounts++;
         } while (xml_nextNode(cur));
         /* Allocate the space. */
         temp->mounts = calloc( temp->nmounts, sizeof(ShipMount) );
         /* Second pass, initialize the mounts. */
         cur = node->children;
         do {
            if (xml_isNode(cur,"mount")) {
               id = xml_getInt(cur);
               xmlr_attr(cur,"x",stmp);
               temp->mounts[id].x = atof(stmp);
               free(stmp);
               xmlr_attr(cur,"y",stmp);
               temp->mounts[id].y  = atof(stmp);
               /* Since we measure in pixels, we have to modify it so it
                *  doesn't get corrected by the ortho correction. */
               temp->mounts[id].y *= M_SQRT2;
               free(stmp);
               xmlr_attr(cur,"h",stmp);
               temp->mounts[id].h = atof(stmp);
               free(stmp);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   temp->thrust *= temp->mass; /* helps keep numbers sane */

   /* ship validator */
#define MELEMENT(o,s)      if (o) WARN("Ship '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->gfx_space==NULL,"GFX");
   MELEMENT(temp->gui==NULL,"GUI");
   MELEMENT(temp->class==SHIP_CLASS_NULL,"class");
   MELEMENT(temp->price==0,"price");
   MELEMENT(temp->tech==0,"tech");
   MELEMENT(temp->fabricator==NULL,"fabricator");
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->thrust==0,"thrust");
   MELEMENT(temp->turn==0,"turn");
   MELEMENT(temp->speed==0,"speed");
   MELEMENT(temp->armour==0,"armour");
   MELEMENT(temp->armour_regen==0,"armour_regen");
   MELEMENT(temp->shield==0,"shield");
   MELEMENT(temp->shield_regen==0,"shield_regen");
   MELEMENT(temp->energy==0,"energy");
   MELEMENT(temp->energy_regen==0,"energy_regen");
   MELEMENT(temp->fuel==0,"fuel");
   MELEMENT(temp->crew==0,"crew");
   MELEMENT(temp->mass==0,"mass");
   MELEMENT(temp->cap_cargo==0,"cap_cargo");
   MELEMENT(temp->cap_weapon==0,"cap_weapon");
   MELEMENT(temp->mounts==NULL,"mounts");
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
   int mem;
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

   mem = 0;
   do {
      if (xml_isNode(node, XML_SHIP)) {
         ship_nstack++;

         /* Check to see if need to grow memory. */
         if (ship_nstack > mem) {
            mem += CHUNK_SIZE;
            ship_stack = realloc(ship_stack, sizeof(Ship)*mem);
         }

         /* Load the ship. */
         ship_parse(&ship_stack[ship_nstack-1], node);
      }
   } while (xml_nextNode(node));

   /* Shrink to minimum size - won't change later. */
   ship_stack = realloc(ship_stack, sizeof(Ship) * ship_nstack);

   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Ship%s", ship_nstack, (ship_nstack==1) ? "" : "s" );

   return 0;
}


/**
 * @brief Frees all the ships.
 */
void ships_free (void)
{
   Ship *s;
   ShipOutfit *so, *sot;
   int i;
   for (i = 0; i < ship_nstack; i++) {
      s = &ship_stack[i];

      /* Free stored strings. */
      if (s->name != NULL)
         free(s->name);
      if (s->description != NULL)
         free(s->description);
      if (s->gui != NULL)
         free(s->gui);
      if (s->fabricator != NULL)
         free(s->fabricator);
      if (s->license != NULL)
         free(s->license);

      /* Free outfits. */
      so = s->outfit;
      while (so) {
         sot = so;
         so = so->next;
         free(sot);
      }

      /* Free mounts. */
      if (s->nmounts > 0)
         free(s->mounts);

      /* Free graphics. */
      gl_freeTexture(s->gfx_space);
      gl_freeTexture(ship_stack[i].gfx_target);
      free(s->gfx_comm);
   }
   free(ship_stack);
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
         "Crew:\n"
         "Mass:\n"
         "\n"
         "Thrust:\n"
         "Max Speed:\n"
         "Turn:\n"
         "\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "\n"
         "Weapon Space:\n"
         "Cargo Space:\n"
         "Fuel:\n"
         );
   h = gl_printHeight( &gl_smallFont, VIEW_WIDTH, buf );

   wid = window_create( shipname, -1, -1, VIEW_WIDTH, h+60+BUTTON_HEIGHT );
   window_addText( wid, 20, -40, VIEW_WIDTH, h,
         0, "txtLabel", &gl_smallFont, &cDConsole, buf );
   snprintf( buf, 1024,
         "%s\n" /* Name */
         "%s\n" /* Class */
         "%d\n" /* Crew */
         "%d Tons\n" /* Mass */
         "\n"
         "%.2f MN/ton\n" /* Thrust */
         "%.2f M/s\n" /* Speed */
         "%.2f Grad/s\n" /* Turn */
         "\n"
         "%.2f MJ (%.2f MJ/s)\n" /* Shield */
         "%.2f MJ (%.2f MJ/s)\n" /* Armour */
         "%.2f MJ (%.2f MJ/s)\n" /* Energy */
         "\n"
         "%d Tons\n" /* Weapon */
         "%d Tons\n" /* Cargo */
         "%d Units\n" /* Fuel */
         , s->name, ship_class(s), s->crew, s->mass,
         s->thrust/s->mass, s->speed, s->turn,
         s->shield, s->shield_regen, s->armour, s->armour_regen,
         s->energy, s->energy_regen,
         s->cap_weapon, s->cap_cargo, s->fuel );
   window_addText( wid, 120, -40, VIEW_WIDTH-140, h,
         0, "txtProperties", &gl_smallFont, &cBlack, buf );

   /* close button */
   snprintf( buf, 37, "close%s", shipname );
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         buf, "Close", window_close );
}

