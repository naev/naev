/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file economy.c
 *
 * @brief Handles economy stuff.
 *
 * Economy is handled with Nodal Analysis.  Systems are modelled as nodes,
 *  jump routes are resistances and production is modelled as node intensity.
 *  This is then solved with linear algebra after each time increment.
 */


#include "economy.h"
#include "commodity.h"
#include "naev.h"

#include <stdio.h>
#include "nstring.h"
#include <stdint.h>

#include "nxml.h"
#include "ndata.h"
#include "log.h"
#include "spfx.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "space.h"
#include "ntime.h"
#include "hook.h"


#define XML_COMMODITY_ID      "Commodities" /**< XML document identifier */
#define XML_COMMODITY_TAG     "commodity" /**< XML commodity identifier. */


/* Gatherables */
#define GATHER_DIST 30. /**< Maximum distance a gatherable can be gathered. */


/* commodity stack */
Commodity* commodity_stack = NULL; /**< Contains all the commodities. */
int commodity_nstack       = 0; /**< Number of commodities in the stack. */


/* standard commodities (ie. sellable and buyable anywhere) */
static int* commodity_standard = NULL; /**< Contains all the standard commodity's indices. */
static int commodity_nstandard = 0; /**< Number of standard commodities. */


/* gatherables stack */
static Gatherable* gatherable_stack = NULL; /**< Contains the gatherable stuff floating around. */
static int gatherable_nstack        = 0; /**< Number of gatherables in the stack. */
float noscoop_timer                 = 1.; /**< Timer for the "full cargo" message . */

/* @TODO remove externs. */
extern int *econ_comm;
extern int econ_nprices;


/*
 * Prototypes.
 */
/* Commodity. */
static void commodity_freeOne( Commodity* com );
static int commodity_parse( Commodity *temp, xmlNodePtr parent );


/**
 * @brief Converts credits to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a length of 32
 *                     char.
 *    @param credits Credits to display.
 *    @param decimals Decimals to use.
 */
void credits2str( char *str, credits_t credits, int decimals )
{
   if (decimals < 0)
      nsnprintf( str, ECON_CRED_STRLEN, "%"CREDITS_PRI" ¢", credits );
   else if (credits >= 1000000000000000000LL)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f E¢", decimals, (double)credits / 1000000000000000000. );
   else if (credits >= 1000000000000000LL)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f P¢", decimals, (double)credits / 1000000000000000. );
   else if (credits >= 1000000000000LL)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f T¢", decimals, (double)credits / 1000000000000. );
   else if (credits >= 1000000000L)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f G¢", decimals, (double)credits / 1000000000. );
   else if (credits >= 1000000)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f M¢", decimals, (double)credits / 1000000. );
   else if (credits >= 1000)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*f k¢", decimals, (double)credits / 1000. );
   else
      nsnprintf (str, ECON_CRED_STRLEN, "%"CREDITS_PRI" ¢", credits );
}

/**
 * @brief Given a price and on-hand credits, outputs a colourized string.
 *
 *    @param[out] str Output is stored here, must have at least a length of 32
 *                     char.
 *    @param price Price to display.
 *    @param credits Credits available.
 *    @param decimals Decimals to use.
 */
void price2str(char *str, credits_t price, credits_t credits, int decimals )
{
   char *buf;

   credits2str(str, price, decimals);
   if (price <= credits)
      return;

   buf = strdup(str);
   nsnprintf(str, ECON_CRED_STRLEN, "\ar%s\a0", buf);
   free(buf);
}

/**
 * @brief Gets a commodity by name.
 *
 *    @param name Name to match.
 *    @return Commodity matching name.
 */
Commodity* commodity_get( const char* name )
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      if (strcmp(commodity_stack[i].name,name)==0)
         return &commodity_stack[i];

   WARN(_("Commodity '%s' not found in stack"), name);
   return NULL;
}


/**
 * @brief Gets a commodity by name without warning.
 *
 *    @param name Name to match.
 *    @return Commodity matching name.
 */
Commodity* commodity_getW( const char* name )
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      if (strcmp(commodity_stack[i].name, name) == 0)
         return &commodity_stack[i];
   return NULL;
}

/**
 * @brief Return the number of commodities globally.
 *
 *    @return Number of commodities globally.
 */
int commodity_getN(void )
{
   return econ_nprices;
}

/**
 * @brief Gets a commodity by index.
 *
 *    @param indx Index of the commodity.
 *    @return Commodity at that index or NULL.
 */
Commodity* commodity_getByIndex( const int indx )
{
   if ( indx < 0 || indx >= econ_nprices ) {
      WARN(_("Commodity with index %d not found"),indx);
      return NULL;
   }
   return &commodity_stack[econ_comm[indx]];
}


/**
 * @brief Frees a commodity.
 *
 *    @param com Commodity to free.
 */
static void commodity_freeOne( Commodity* com )
{
   CommodityModifier *this,*next;
   if (com->name)
      free(com->name);
   if (com->description)
      free(com->description);
   if (com->gfx_store)
      gl_freeTexture(com->gfx_store);
   if (com->gfx_space)
      gl_freeTexture(com->gfx_space);
   next = com->planet_modifier;
   com->planet_modifier = NULL;
   while (next != NULL ) {
      this = next;
      next = this->next;
      free(this->name);
      free(this);
   }
   next = com->faction_modifier;
   com->faction_modifier = NULL;
   while (next != NULL ) {
      this=next;
      next=this->next;
      free(this->name);
      free(this);
   }
   /* Clear the memory. */
   memset(com, 0, sizeof(Commodity));
}


/**
 * @brief Function meant for use with C89, C99 algorithm qsort().
 *
 *    @param commodity1 First argument to compare.
 *    @param commodity2 Second argument to compare.
 *    @return -1 if first argument is inferior, +1 if it's superior, 0 if ties.
 */
int commodity_compareTech( const void *commodity1, const void *commodity2 )
{
   const Commodity *c1, *c2;

   /* Get commodities. */
   c1 = * (const Commodity**) commodity1;
   c2 = * (const Commodity**) commodity2;

   /* Compare price. */
   if (c1->price < c2->price)
      return +1;
   else if (c1->price > c2->price)
      return -1;

   /* It turns out they're the same. */
   return strcmp( c1->name, c2->name );
}


/**
 * @brief Return the list of standard commodities.
 *
 *    @param[out] Commodity* List of commodities.
 *    @return size of the list.
 */
Commodity ** standard_commodities( unsigned int *nb )
{
   int i;
   Commodity **com;

   *nb = commodity_nstandard;

   if (commodity_nstandard == 0)
      return NULL;

   com = malloc( commodity_nstandard * sizeof(Commodity*) );
   for (i=0; i<commodity_nstandard; i++) {
      com[i] = &commodity_stack[ commodity_standard[i] ];
   }
   return com;
}

/**
 * @brief Loads a commodity.
 *
 *    @param temp Commodity to load data into.
 *    @param parent XML node to load from.
 *    @return Commodity loaded from parent.
 */
static int commodity_parse( Commodity *temp, xmlNodePtr parent )
{
   xmlNodePtr node;
   CommodityModifier *newdict;
   /* Clear memory. */
   memset( temp, 0, sizeof(Commodity) );
   temp->period = 200;
   temp->population_modifier = 0.;
   temp->standard = 0;

   /* Parse body. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      xmlr_strd(node, "name", temp->name);
      xmlr_strd(node, "description", temp->description);
      xmlr_int(node, "price", temp->price);
      if (xml_isNode(node,"gfx_space"))
         temp->gfx_space = xml_parseTexture( node,
               COMMODITY_GFX_PATH"space/%s.png", 1, 1, OPENGL_TEX_MIPMAPS );
      if (xml_isNode(node,"gfx_store")) {
         temp->gfx_store = xml_parseTexture( node,
               COMMODITY_GFX_PATH"%s.png", 1, 1, OPENGL_TEX_MIPMAPS );
         if (temp->gfx_store != NULL) {
         } else {
            temp->gfx_store = gl_newImage( COMMODITY_GFX_PATH"_default.png", 0 );
         }
         continue;
      }
      if (xml_isNode(node, "standard")) {
         temp->standard = 1;
         /* There is a shortcut list containing the standard commodities. */
         commodity_standard = realloc(commodity_standard, sizeof(int)*(++commodity_nstandard));
         commodity_standard[ commodity_nstandard-1 ] = commodity_nstack-1;
         continue;
      }
      xmlr_float(node, "population_modifier", temp->population_modifier);
      xmlr_float(node, "period", temp->period);
      if (xml_isNode(node, "planet_modifier")) {
         newdict = malloc(sizeof(CommodityModifier));
         newdict->next = temp->planet_modifier;
         newdict->name = xml_nodeProp(node,(xmlChar*)"type");
         newdict->value = xml_getFloat(node);
         temp->planet_modifier = newdict;
         continue;
      }
      if (xml_isNode(node, "faction_modifier")) {
         newdict = malloc(sizeof(CommodityModifier));
         newdict->next = temp->faction_modifier;
         newdict->name = xml_nodeProp(node, (xmlChar*)"type");
         newdict->value = xml_getFloat(node);
         temp->faction_modifier = newdict;
      }
   
   } while (xml_nextNode(node));
   if (temp->name == NULL)
      WARN( _("Commodity from %s has invalid or no name"), COMMODITY_DATA_PATH);
   if ((temp->price > 0)) {
      if (temp->gfx_store == NULL) {
         WARN(_("No <gfx_store> node found, using default texture for commodity \"%s\""), temp->name);
         temp->gfx_store = gl_newImage( COMMODITY_GFX_PATH"_default.png", 0 );
      }
      if (temp->gfx_space == NULL)
         temp->gfx_space = gl_newImage( COMMODITY_GFX_PATH"space/_default.png", 0 );
   }

   

#if 0 /* shouldn't be needed atm */
#define MELEMENT(o,s)   if (o) WARN("Commodity '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->high==0,"high");
   MELEMENT(temp->medium==0,"medium");
   MELEMENT(temp->low==0,"low");
#undef MELEMENT
#endif

   return 0;
}


/**
 * @brief Throws cargo out in space graphically.
 *
 *    @param pilot ID of the pilot throwing the stuff out
 *    @param com Commodity to throw out.
 *    @param quantity Quantity thrown out.
 */
void commodity_Jettison( int pilot, Commodity* com, int quantity )
{
   (void)com;
   int i;
   Pilot* p;
   int n, effect;
   double px,py, bvx, bvy, r,a, vx,vy;

   p   = pilot_get( pilot );

   n   = MAX( 1, RNG(quantity/10, quantity/5) );
   px  = p->solid->pos.x;
   py  = p->solid->pos.y;
   bvx = p->solid->vel.x;
   bvy = p->solid->vel.y;
   for (i=0; i<n; i++) {
      effect = spfx_get("cargo");

      /* Radial distribution gives much nicer results */
      r  = RNGF()*25 - 12.5;
      a  = 2. * M_PI * RNGF();
      vx = bvx + r*cos(a);
      vy = bvy + r*sin(a);

      /* Add the cargo effect */
      spfx_add( effect, px, py, vx, vy, SPFX_LAYER_BACK );
   }
}


/**
 * @brief Initializes a gatherable object
 *
 *    @param com Type of commodity.
 *    @param pos Position.
 *    @param vel Velocity.
 */
int gatherable_init( Commodity* com, Vector2d pos, Vector2d vel, double lifeleng, int qtt )
{
   gatherable_stack = realloc(gatherable_stack,
                              sizeof(Gatherable)*(++gatherable_nstack));

   gatherable_stack[gatherable_nstack-1].type = com;
   gatherable_stack[gatherable_nstack-1].pos = pos;
   gatherable_stack[gatherable_nstack-1].vel = vel;
   gatherable_stack[gatherable_nstack-1].timer = 0.;
   gatherable_stack[gatherable_nstack-1].quantity = qtt;

   if (lifeleng < 0.)
      gatherable_stack[gatherable_nstack-1].lifeleng = RNGF()*100. + 50.;
   else
      gatherable_stack[gatherable_nstack-1].lifeleng = lifeleng;

   return gatherable_nstack-1;
}


/**
 * @brief Updates all gatherable objects
 *
 *    @param dt Elapsed time.
 */
void gatherable_update( double dt )
{
   int i;

   /* Update the timer for "full cargo" message. */
   noscoop_timer += dt;

   for (i=0; i < gatherable_nstack; i++) {
      gatherable_stack[i].timer += dt;
      gatherable_stack[i].pos.x += dt*gatherable_stack[i].vel.x;
      gatherable_stack[i].pos.y += dt*gatherable_stack[i].vel.y;

      /* Remove the gatherable */
      if (gatherable_stack[i].timer > gatherable_stack[i].lifeleng) {
         gatherable_nstack--;
         memmove( &gatherable_stack[i], &gatherable_stack[i+1],
                 sizeof(Gatherable)*(gatherable_nstack-i) );
         gatherable_stack = realloc(gatherable_stack,
                                    sizeof(Gatherable) * gatherable_nstack);
         i--;
      }
   }
}


/**
 * @brief Frees all the gatherables
 */
void gatherable_free( void )
{
   free(gatherable_stack);
   gatherable_stack = NULL;
   gatherable_nstack = 0;
}


/**
 * @brief Renders all the gatherables
 */
void gatherable_render( void )
{
   int i;
   Gatherable *gat;

   for (i=0; i < gatherable_nstack; i++) {
      gat = &gatherable_stack[i];
      gl_blitSprite( gat->type->gfx_space, gat->pos.x, gat->pos.y, 0, 0, NULL );
   }
}


/**
 * @brief Gets the closest gatherable from a given position, within a given radius
 *
 *    @param pos position.
 *    @param rad radius.
 *    @return The id of the closest gatherable, or -1 if none is found.
 */
int gatherable_getClosest( Vector2d pos, double rad )
{
   int i, curg;
   Gatherable *gat;
   double mindist, curdist;

   curg = -1;
   mindist = INFINITY;

   for (i=0; i < gatherable_nstack; i++) {
      gat = &gatherable_stack[i];
      curdist = vect_dist(&pos, &gat->pos);
      if ( (curdist<mindist) && (curdist<rad) ) {
         curg = i;
         mindist = curdist;
      }
   }
   return curg;
}


/**
 * @brief Returns the position and velocity of a gatherable
 *
 *    @param pos pointer to the position.
 *    @param vel pointer to the velocity.
 *    @param id Id of the gatherable in the stack.
 *    @return flag 1->there exists a gatherable 0->elsewere.
 */
int gatherable_getPos( Vector2d* pos, Vector2d* vel, int id )
{
   Gatherable *gat;

   if ((id < 0) || (id > gatherable_nstack-1) ) {
      vectnull( pos );
      vectnull( vel );
      return 0;
   }

   gat = &gatherable_stack[id];
   *pos = gat->pos;
   *vel = gat->vel;

   return 1;
}


/**
 * @brief See if the pilot can gather anything
 *
 *    @param pilot ID of the pilot
 */
void gatherable_gather( int pilot )
{
   int i, q;
   Gatherable *gat;
   Pilot* p;
   HookParam hparam[3];

   p = pilot_get( pilot );

   for (i=0; i < gatherable_nstack; i++) {
      gat = &gatherable_stack[i];

      if (vect_dist( &p->solid->pos, &gat->pos ) < GATHER_DIST ) {
         /* Add cargo to pilot. */
         q = pilot_cargoAdd( p, gat->type, gat->quantity, 0 );

         if (q>0) {
            if (pilot_isPlayer(p)) {
               player_message( ngettext("%d ton of %s gathered", "%d tons of %s gathered", q), q, gat->type->name );

               /* Run hooks. */
               hparam[0].type    = HOOK_PARAM_STRING;
               hparam[0].u.str   = gat->type->name;
               hparam[1].type    = HOOK_PARAM_NUMBER;
               hparam[1].u.num   = q;
               hparam[2].type    = HOOK_PARAM_SENTINEL;
               hooks_runParam( "gather", hparam );
            }

            /* Remove the object from space. */
            gatherable_nstack--;
            memmove( &gatherable_stack[i], &gatherable_stack[i+1],
                    sizeof(Gatherable)*(gatherable_nstack-i) );
            gatherable_stack = realloc(gatherable_stack,
                                       sizeof(Gatherable) * gatherable_nstack);

            /* Test if there is still cargo space */
            if ((pilot_cargoFree(p) < 1) && (pilot_isPlayer(p)))
               player_message( _("No more cargo space available") );
         }
         else if ((pilot_isPlayer(p)) && (noscoop_timer > 2.)) {
            noscoop_timer = 0.;
            player_message( _("Cannot gather material: no more cargo space available") );
         }
      }
   }
}


/**
 * @brief Loads all the commodity data.
 *
 *    @return 0 on success.
 */
int commodity_load (void)
{
   size_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load the file. */
   buf = ndata_read( COMMODITY_DATA_PATH, &bufsize);
   if (buf == NULL)
      return -1;

   /* Handle the XML. */
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      WARN(_("'%s' is not valid XML."), COMMODITY_DATA_PATH);
      return -1;
   }

   node = doc->xmlChildrenNode; /* Commodities node */
   if (strcmp((char*)node->name,XML_COMMODITY_ID)) {
      ERR(_("Malformed %s file: missing root element '%s'"), COMMODITY_DATA_PATH, XML_COMMODITY_ID);
      return -1;
   }

   node = node->xmlChildrenNode; /* first commodity type */
   if (node == NULL) {
      ERR(_("Malformed %s file: does not contain elements"), COMMODITY_DATA_PATH);
      return -1;
   }

   do {
      xml_onlyNodes(node);
      if (xml_isNode(node, XML_COMMODITY_TAG)) {

         /* Make room for commodity. */
         commodity_stack = realloc(commodity_stack,
               sizeof(Commodity)*(++commodity_nstack));

         /* Load commodity. */
         commodity_parse(&commodity_stack[commodity_nstack-1], node);

         /* See if should get added to commodity list. */
         if (commodity_stack[commodity_nstack-1].price > 0.) {
            econ_nprices++;
            econ_comm = realloc(econ_comm, econ_nprices * sizeof(int));
            econ_comm[econ_nprices-1] = commodity_nstack-1;
         }
      }
      else
         WARN(_("'%s' has unknown node '%s'."), COMMODITY_DATA_PATH, node->name);
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);

   DEBUG( ngettext( "Loaded %d Commodity", "Loaded %d Commodities", commodity_nstack ), commodity_nstack );

   return 0;


}


/**
 * @brief Frees all the loaded commodities.
 */
void commodity_free (void)
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      commodity_freeOne( &commodity_stack[i] );
   free( commodity_stack );
   commodity_stack = NULL;
   commodity_nstack = 0;
   free( commodity_standard );
   commodity_standard = NULL;
   commodity_nstandard = 0;

   /* More clean up. */
   free( econ_comm );
}

