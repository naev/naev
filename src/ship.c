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


#define XML_ID    "Ships"  /* XML section identifier */
#define XML_SHIP  "ship"

#define SHIP_DATA    "dat/ship.xml"
#define SHIP_GFX     "gfx/ship/"
#define SHIP_EXT     ".png"
#define SHIP_TARGET  "_target"

#define VIEW_WIDTH   300
#define VIEW_HEIGHT  300

#define BUTTON_WIDTH  80
#define BUTTON_HEIGHT 30


static Ship* ship_stack = NULL;
static int ship_nstack = 0;


/*
 * Prototypes
 */
static Ship* ship_parse( xmlNodePtr parent );
static void ship_view_close( char* btn );


/*
 * Gets a ship based on its name
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


/*
 * returns all the ships in  text form
 */
char** ship_getTech( int *n, const int *tech, const int techmax )
{
   int i,j,k, num, price;
   char **shipnames;
   Ship **ships;
   
   ships = malloc(sizeof(Ship*) * ship_nstack);
   num = 0;
   for (i=0; i < ship_nstack; i++) {
      if (ship_stack[i].tech <= tech[0]) {
         ships[num] = &ship_stack[i];
         num++;
      }
      else {
         for (j=0; j<techmax; j++)
            if (tech[j] == ship_stack[i].tech) {
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


/*
 * Gets the ship's classname
 */
static char* ship_classes[] = { "NULL",
      "Civilian Light", "Civilian Medium", "Civilian Heavy",
      "Military Light", "Military Medium", "Military Heavy",
      "Robotic Light", "Robotic Medium", "Robotic Heavy",
      "Hybrid Light", "Hybrid Medium", "Hybrid Heavy"
};
char* ship_class( Ship* s )
{
   return ship_classes[s->class];
}


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
         snprintf( str, strlen(xml_get(node))+
               sizeof(SHIP_GFX)+sizeof(SHIP_EXT),
               SHIP_GFX"%s"SHIP_EXT, xml_get(node));
         temp->gfx_space = gl_newSprite(str, 6, 6);
         snprintf( str, strlen(xml_get(node))+
               sizeof(SHIP_GFX)+sizeof(SHIP_TARGET)+sizeof(SHIP_EXT),
               SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, xml_get(node));
         temp->gfx_target = gl_newImage(str);
      }

      xmlr_strd(node,"GUI",temp->gui);
      if (xml_isNode(node,"sound"))
         temp->sound = sound_get( xml_get(node) );
      xmlr_int(node,"class",temp->class);
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
      }
      else if (xml_isNode(node,"health")) {
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
      }
      else if (xml_isNode(node,"caracteristics")) {
         cur = node->children;
         do {
            xmlr_int(cur,"crew",temp->crew);
            xmlr_float(cur,"mass",temp->mass);
            xmlr_int(cur,"fuel",temp->fuel);
            xmlr_int(cur,"cap_weapon",temp->cap_weapon);
            xmlr_int(cur,"cap_cargo",temp->cap_cargo);
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"outfits")) {
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
      }
   } while (xml_nextNode(node));
   temp->thrust *= temp->mass; /* helps keep numbers sane */

   /* ship validator */
#define MELEMENT(o,s)      if (o) WARN("Ship '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->name==NULL,"name");
   MELEMENT(temp->gfx_space==NULL,"GFX");
   MELEMENT(temp->gui==NULL,"GUI");
   MELEMENT(temp->class==0,"class");
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


/*
 * used to visualize the ships stats
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
         "%s\n"
         "%s\n"
         "%d\n"
         "%d Tons\n"
         "\n"
         "%.2f MN\n"
         "%.2f M/s\n"
         "%.2f Grad/s\n"
         "\n"
         "%.2f MJ (%.2f MJ/s)\n"
         "%.2f MJ (%.2f MJ/s)\n"
         "%.2f MJ (%.2f MJ/s)\n"
         "\n"
         "%d Tons\n"
         "%d Tons\n"
         "%d Units\n"
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
static void ship_view_close( char* btn )
{
   window_destroy( window_get( btn+5 /* "closeFoo -> Foo" */ ) );
}

