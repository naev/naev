/*
 * See Licensing and Copyright notice in naev.h
 */



#include "ship.h"

#include <string.h>
#include <limits.h>

#include "xml.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "toolkit.h"


#define XML_ID    "Ships"  /**< XML document identifier */
#define XML_SHIP  "ship" /**< XML individual ship identifier. */

#define SHIP_DATA    "dat/ship.xml" /**< XML file containing ships. */
#define SHIP_GFX     "gfx/ship/" /**< Location of ship graphics. */
#define SHIP_EXT     ".png" /**< Ship graphics extension format. */
#define SHIP_TARGET  "_target" /**< Target graphic extension. */

#define VIEW_WIDTH   300 /**< Ship view window width. */
#define VIEW_HEIGHT  300 /**< Ship view window height. */

#define BUTTON_WIDTH  80 /**< Button width in ship view window. */
#define BUTTON_HEIGHT 30 /**< Button height in ship view window. */


static Ship* ship_stack = NULL; /**< Stack of ships available in the game. */
static int ship_nstack = 0; /**< Number of ships in the stack. */


/*
 * Prototypes
 */
static Ship* ship_parse( xmlNodePtr parent );
static void ship_view_close( char* btn );
static ShipClass ship_classFromString( char* str );


/**
 * @fn Ship* ship_get( const char* name )
 *
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
 * @fn char** ship_getTech( int *n, const int *tech, const int techmax )
 *
 * @brief Gets all the ships in text form matching tech.
 *
 * You have to free all the strings created in the string array too.
 *
 *    @param[out] n Number of ships found.
 *    @param tech List of technologies to use.
 *    @param techmax Number of technologies in tech.
 *    @return An array of allocated ship names.
 */
char** ship_getTech( int *n, const int *tech, const int techmax )
{
   int i,j,k, num, price;
   char **shipnames;
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
   shipnames = malloc(sizeof(char*) * num);
   /* until we fill the new stack */
   for (i=0; i<num; i++) {

      /* check for cheapest */
      for (j=0; j<num; j++) {

         /* is cheapest? */
         if ((price == -1) || (ships[price]->price > ships[j]->price)) {

            /* check if already in stack */
            for (k=0; k<(*n); k++)
               if (strcmp(shipnames[k],ships[j]->name)==0)
                  break;

            /* not in stack and therefore is cheapest */
            if (k == (*n))
               price = j;
         }
      }

      /* add current cheapest to stack */
      shipnames[i] = strdup(ships[price]->name);
      (*n)++;
      price = -1;
   }

   /* cleanup */
   free(ships);

   return shipnames;
}


/**
 * @fn char* ship_class( Ship* s )
 *
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
      case SHIP_CLASS_LUXERY_YACHT:
         return "Luxery Yacht";
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
 * @fn static ShipClass ship_classFromString( char* str )
 *
 * @brief Gets the machine ship class identifier from a human readable string.
 *
 *    @param str String to extract ship class identifier from.
 */
static ShipClass ship_classFromString( char* str )
{
   /* Civilian */
   if (strcmp(str,"Yacht")==0)
      return SHIP_CLASS_YACHT;
   else if (strcmp(str,"Luxery Yacht")==0)
      return SHIP_CLASS_LUXERY_YACHT;
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
 * @fn int ship_basePrice( Ship* s )
 *
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

   return price;
}


/**
 * @fn static Ship* ship_parse( xmlNodePtr parent )
 *
 * @brief Extracts the ingame ship from an XML node.
 *
 *    @param parent Node to get ship from.
 *    @return The newly created ship.
 */
static Ship* ship_parse( xmlNodePtr parent )
{
   xmlNodePtr cur, node;
   Ship* temp = CALLOC_ONE(Ship);
   ShipOutfit *otemp, *ocur;

   char str[PATH_MAX] = "\0";
   char* stmp;

   xmlr_attr(parent,"name",temp->name);
   if (temp->name == NULL) WARN("Ship in "SHIP_DATA" has invalid or no name");

   node = parent->xmlChildrenNode;

   do { /* load all the data */
      if (xml_isNode(node,"GFX")) {

         /* Load the base graphic */
         snprintf( str, strlen(xml_get(node))+
               sizeof(SHIP_GFX)+sizeof(SHIP_EXT),
               SHIP_GFX"%s"SHIP_EXT, xml_get(node));
         temp->gfx_space = gl_newSprite(str, 6, 6);


         xmlr_attr(node,"target",stmp);
         if (stmp != NULL) {
            snprintf( str, strlen(stmp)+
                  sizeof(SHIP_GFX)+sizeof(SHIP_TARGET)+sizeof(SHIP_EXT),
                  SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, stmp);
            temp->gfx_target = gl_newImage(str);
            free(stmp);
         }
         else { /* Load standard target graphic */
            snprintf( str, strlen(xml_get(node))+
                  sizeof(SHIP_GFX)+sizeof(SHIP_TARGET)+sizeof(SHIP_EXT),
                  SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, xml_get(node));
            temp->gfx_target = gl_newImage(str);
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
               otemp = MALLOC_ONE(ShipOutfit);
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
#undef MELEMENT

   return temp;
}


/**
 * @fn int ships_load(void)
 *
 * @brief Loads all the ships in the data files.
 *
 *    @return 0 on success.
 */
int ships_load(void)
{
   uint32_t bufsize;
   char *buf = pack_readfile(DATA, SHIP_DATA, &bufsize);

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   Ship* temp = NULL;

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

   do {
      if (node->type ==XML_NODE_START &&
            strcmp((char*)node->name,XML_SHIP)==0) {
         temp = ship_parse(node);
         ship_stack = realloc(ship_stack, sizeof(Ship)*(++ship_nstack));
         memcpy(ship_stack+ship_nstack-1, temp, sizeof(Ship));
         free(temp);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Ship%s", ship_nstack, (ship_nstack==1) ? "" : "s" );

   return 0;
}


/**
 * @fn void ships_free()
 *
 * @brief Frees all the ships.
 */
void ships_free()
{
   ShipOutfit *so, *sot;
   int i;
   for (i = 0; i < ship_nstack; i++) {
      /* free stored strings */
      if ((ship_stack+i)->name) free(ship_stack[i].name);
      if ((ship_stack+i)->description) free(ship_stack[i].description);
      if ((ship_stack+i)->gui) free(ship_stack[i].gui);
      if ((ship_stack+i)->fabricator) free(ship_stack[i].fabricator);

      so=(ship_stack+i)->outfit;
      while (so) { /* free the outfits */
         sot = so;
         so = so->next;
         free(sot);
      }

      gl_freeTexture((ship_stack+i)->gfx_space);
      gl_freeTexture((ship_stack+i)->gfx_target);
   }
   free(ship_stack);
   ship_stack = NULL;
}


/**
 * @fn void ship_view( char* shipname )
 *
 * @brief Used to visualize the ships stats.
 *
 * @todo Take into account outfits or something like that.
 *
 *    @param shipname Ship ot view the stats of.
 */
void ship_view( char* shipname )
{
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
         buf, "Close", ship_view_close );
}
/**
 * @fn static void ship_view_close( char* btn )
 * @brief Closes the ship view window.
 *    @param btn Used internally by buttons.
 */
static void ship_view_close( char* btn )
{
   window_destroy( window_get( btn+5 /* "closeFoo -> Foo" */ ) );
}

