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

#include "naev.h"

#include <stdio.h>
#include "nstring.h"
#include <stdint.h>

#ifdef HAVE_SUITESPARSE_CS_H
#include <suitesparse/cs.h>
#else
#include <cs.h>
#endif

#include "nxml.h"
#include "ndata.h"
#include "log.h"
#include "spfx.h"
#include "pilot.h"
#include "rng.h"
#include "space.h"
#include "ntime.h"


#define XML_COMMODITY_ID      "Commodities" /**< XML document identifier */
#define XML_COMMODITY_TAG     "commodity" /**< XML commodity identifier. */


/*
 * Economy Nodal Analysis parameters.
 */
 // ### will have to modifiy
#define ECON_BASE_RES      10. /**< Base resistance value for any system. */
#define ECON_SELF_RES      3. /**< Additional resistance for the self node. */
#define ECON_FACTION_MOD   0.1 /**< Modifier on Base for faction standings. */

         //Not something I inteded to use, but can be useful nonetheless
#define ECON_PROD_VAR      0.01 /**< Defines the variability of production. */

#define STARTING_CREDITS   100000000. /**< ### How many credits are initially given to each system*/
#define STARTING_GOODS     10000.

#define PRICE(Credits,Goods)  (Credits / (Goods * 20)) /**< Price of a good*/

#define AVG_POPULATION     10000000 /**< Used for prod_mods as divisor populations */
#define TRADE_MAX          systems_nstack*300

/* commodity stack */
Commodity* commodity_stack = NULL; /**< Contains all the commodities. */
static int commodity_nstack       = 0; /**< Number of commodities in the stack. */


/* systems stack. */
extern StarSystem *systems_stack; /**< Star system stack. */
extern int systems_nstack; /**< Number of star systems. */

/* planet stack  */
extern Planet *planet_stack; /**< Planet stack. */
extern int planet_nstack; /**< Num of planets */


/*
 * Global Economy variables. Most of it stored with systems 
 */
static int econ_initialized   = 0; /**< Is economy system initialized? */
static int *econ_comm         = NULL; /**< Commodities to calculate. ### Is this needed for anything? */
int econ_nprices       = 0; /**< Number of prices to calculate. */
static double trade_max       = 0.; /**< @@@ Maximum trade in the galaxy at any dt */

/*
 * Prototypes.
 */
/* Commodity. */
static void commodity_freeOne( Commodity* com );
static int commodity_parse( Commodity *temp, xmlNodePtr parent );

/* Economy. */
static double econ_calcJumpR( StarSystem *A, StarSystem *B );
credits_t economy_getPrice( const Commodity *com,
      const StarSystem *sys, const Planet *p ); /* externed in land.c */
void produce_consume(void);
void trade_update(void);
void refresh_prices(void);
// void economy_update( unsigned int dt );


//### These are planet classification production modifiers. These are multiplied
//     by population to get final planetary modifiers. Not sure how this should be dealt with
//planet classifications are in space.h
//Commodities are Food, Ore, Industrial Goods, Medicine, Luxury Items
double planet_class_mods[][5] = {
   { 0., 0., 0., 0., 0.},  //for planets w/out class
   { 2.,-1.,-1., 2., 3.},
   { 0., 1., 0.,-1., 2.},
   {-2., 2., 1.,-1., 1.},
   {-2., 7.,-2.,-1., 0.},
   {-1., 3., 0.,-1.,-1.},
   {-2., 4., 2.,-1., 0.},
   { 1., 3.,-1., 2., 3.},
   {-5., 4.,-2., 3., 6.},
   {-3., 2.,-2., 3.,-1.},
   {-1., 3.,-1.,-1., 0.},
   { 8.,-2.,-2., 1.,-3.},
   {10.,-3.,-2.,-1.,-3.},
   {14.,-12.,-5.,8.,-7.},
   {-5., 4.,-2., 6., 0.},
   {20., 0.,-13.,-2.,-1.},
   { 6., 0.,-5.,-1., 1.},
   { 0., 4.,-2.,-1., 0.},
   {-3., 2.,-1., 0., 1.},
   {-1., 1.,-1., 1., 1.},
   {-1., 1.,-1., 1., 1.},
   { 0., 0., 0., 0., 0.},
   { 0., 0., 0., 0., 0.},
   { 0., 0., 0., 0., 0.},
   {-3.,-8.,16., 3., 1.}, //stations are no longer buffed to account for population
   {-4.,-9.,20.,-1., 0.},
   {-3.,-6.,14., 0., 1.},
   { 0., 0., 0., 0., 0.}
};





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
      nsnprintf( str, ECON_CRED_STRLEN, "%"CREDITS_PRI, credits );
   else if (credits >= 1000000000000000LL)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*fQ", decimals, (double)credits / 1000000000000000. );
   else if (credits >= 1000000000000LL)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*fT", decimals, (double)credits / 1000000000000. );
   else if (credits >= 1000000000L)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*fB", decimals, (double)credits / 1000000000. );
   else if (credits >= 1000000)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*fM", decimals, (double)credits / 1000000. );
   else if (credits >= 1000)
      nsnprintf( str, ECON_CRED_STRLEN, "%.*fK", decimals, (double)credits / 1000. );
   else
      nsnprintf (str, ECON_CRED_STRLEN, "%"CREDITS_PRI, credits );
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

   WARN("Commodity '%s' not found in stack", name);
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
      if (strcmp(commodity_stack[i].name,name)==0)
         return &commodity_stack[i];
   return NULL;
}


/**
 * @brief Frees a commodity.
 *
 *    @param com Commodity to free.
 */
static void commodity_freeOne( Commodity* com )
{
   if (com->name)
      free(com->name);
   if (com->description)
      free(com->description);

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
 * @brief Loads a commodity.
 *
 *    @param temp Commodity to load data into.
 *    @param parent XML node to load from.
 *    @return Commodity loaded from parent.
 */
static int commodity_parse( Commodity *temp, xmlNodePtr parent )
{
   xmlNodePtr node;

   /* Clear memory. */
   memset( temp, 0, sizeof(Commodity) );

   /* Get name. */
   xmlr_attr( parent, "name", temp->name );
   if (temp->name == NULL)
      WARN("Commodity from "COMMODITY_DATA_PATH" has invalid or no name");

   /* Parse body. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      xmlr_strd(node, "description", temp->description);
      xmlr_int(node, "price", temp->price);
      WARN("Commodity '%s' has unknown node '%s'.", temp->name, node->name);
   } while (xml_nextNode(node));

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
 * @brief Loads all the commodity data.
 *
 *    @return 0 on success.
 */
int commodity_load (void)
{
   uint32_t bufsize;
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
      WARN("'%s' is not valid XML.", COMMODITY_DATA_PATH);
      return -1;
   }

   node = doc->xmlChildrenNode; /* Commodities node */
   if (strcmp((char*)node->name,XML_COMMODITY_ID)) {
      ERR("Malformed "COMMODITY_DATA_PATH" file: missing root element '"XML_COMMODITY_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first faction node */
   if (node == NULL) {
      ERR("Malformed "COMMODITY_DATA_PATH" file: does not contain elements");
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
         WARN("'"COMMODITY_DATA_PATH"' has unknown node '%s'.", node->name);
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Commodit%s", commodity_nstack, (commodity_nstack==1) ? "y" : "ies" );

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

   /* More clean up. */
   free( econ_comm );
}


/**
 * @brief Gets the price of a good on a planet in a system.
 *
 *    @param com Commodity to get price of.
 *    @param sys System to get price of commodity.
 *    @param p Planet to get price of commodity.
 *    @return The price of the commodity.
 */
credits_t economy_getPrice( const Commodity *com,
      const StarSystem *sys, const Planet *p )
{
   (void) p;
   int i, k;
   double price;

   /* Get position in stack. */
   k = com - commodity_stack;

   /* Find what commodity that is. */
   for (i=0; i<econ_nprices; i++)
      if (econ_comm[i] == k)
         break;

   /* Check if found. */
   if (i >= econ_nprices) {
      WARN("Price for commodity '%s' not known.", com->name);
      return 0;
   }

   /* Calculate price. */
   //price  = (double) com->price;  //@@@ this voids the price multiplier in commodities.xml
   price = sys->prices[i];
   return (credits_t) price;
}

/**
 * @brief gets the number of credits in a system
 *
 *    @param sys System to get credit stockpile of
 *
 *
 *
*/

/**
 * @brief Calculates the resistance between two star systems.
 *
 *    @param A Star system to calculate the resistance between.
 *    @param B Star system to calculate the resistance between.
 *    @return Resistance between A and B.
 */
static double econ_calcJumpR( StarSystem *A, StarSystem *B )
{
   double R;

   /* Set to base to ensure price change. */
   R = ECON_BASE_RES;

   /* Modify based on system conditions. */
   R += (A->nebu_density + B->nebu_density) / 1000.; /* Density shouldn't affect much. */
   R += (A->nebu_volatility + B->nebu_volatility) / 100.; /* Volatility should. */

   /* Modify based on global faction. */
   if ((A->faction != -1) && (B->faction != -1)) {
      if (areEnemies(A->faction, B->faction))
         R += ECON_FACTION_MOD * ECON_BASE_RES;
      else if (areAllies(A->faction, B->faction))
         R -= ECON_FACTION_MOD * ECON_BASE_RES;
   }

   /* @todo Modify based on trader/faction presence. */

   return R;
}


/**
 * @brief Initializes the economy.
 *
 *    @return 0 on success.
 */
int economy_init (void)    //IMPORTANT @@@ 
{     //Not for loading loading economies
   printf("\nInit ing economy");

   int i;
   int i0;

   int goodnum;

   StarSystem *sys1;
   StarSystem *sys2;

   Planet *planet;

   /* Must not be initialized. */
   if (econ_initialized)
      return 0;

      //### Put in resistances
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (i0=0;i0<(*sys1).njumps; i0++) {

         sys2=&systems_stack[ (*sys1).jumps[i0].targetid ];

         (*sys1).jumps[i0].jump_resistance=econ_calcJumpR(sys1,sys2);
      }
   }


   /* Allocate price space, commodity space, and credits stockpile */
   for (i=0; i<systems_nstack; i++) {
      if (systems_stack[i].prices != NULL)
         free(systems_stack[i].prices);

      systems_stack[i].prices = calloc(econ_nprices, sizeof(double));
      systems_stack[i].stockpiles = calloc(econ_nprices, sizeof(double));
      systems_stack[i].credits = STARTING_CREDITS;
      systems_stack[i].prod_mods = calloc(econ_nprices, sizeof(double));
   }     



   /* Initialize starting values */ 
         //For now this means set stockpiles and credits to starting values
   for (i=0; i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      (*sys1).credits = STARTING_CREDITS;

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         (*sys1).stockpiles[goodnum] = STARTING_GOODS;
      }

   }     


   /* Set up production modifiers, based on asset classifications */
      //For every system, for every planet, for every good, add 
      //    (planet.population * planet_prod_mod) to system.prod_mods
   for (i=0; i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (i0=0; i0<(*sys1).nplanets; i0++) {

         planet = &planet_stack[ (*sys1).planetsid[i0] ];

         for (goodnum=0; goodnum<econ_nprices ; goodnum++) {

            (*sys1).prod_mods[goodnum] +=  planet_class_mods[(*planet).class][goodnum] 
               * (*planet).population / AVG_POPULATION;
         } 
      }
   }


   /* Mark economy as initialized. */
   econ_initialized = 1;

   /* Refresh economy. */
   refresh_prices();//###


   /* set trade_max, max trade in galaxy */
   trade_max = 300 * systems_nstack;

   return 0;
}

   /* How to produce/consume in a single update */
double production(double mod, double goods)
{
      //### @@@ Should this be defined as a macro?
   if (mod >= 0)
      return mod * (4000 / (goods + 700));
   else
      return mod * ((goods + 500) / 6000);

}


/* Every system produces and consumes their appropriate amount */
void produce_consume(void)
{

   int i;
   int goodnum;

   double mod;
   double goods;

   StarSystem *sys1;


   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         mod   = (*sys1).prod_mods[goodnum];
         goods = (*sys1).stockpiles[goodnum];

         (*sys1).stockpiles[goodnum]+=production(mod,goods);

      }

   }
}

/* Refresh prices to be accurate */
void refresh_prices(void)
{
   int i;
   int goodnum;

   double credits;
   double goods;

   StarSystem *sys1;


   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         credits  = (*sys1).credits;
         goods    = (*sys1).stockpiles[goodnum];

         // printf("\nprice is goods at %f, credits at %f, %f",goods,credits,PRICE(credits,goods));

         (*sys1).prices[goodnum] = PRICE(credits,goods);

      }

   }

}

/* trade in the galaxy */
void trade_update(void)
{

   int i;
   int goodnum;
   int jumpnum;

   double price;
   double trade;

   double total_wanted_trade = 0; //How much trade or "current" wants to flow
               // Here, "current" is price difference / resistance
   double current_modifier = 0;  //the fraction of "trade current" that shall
               //actually be traded.

   StarSystem *sys1;
   StarSystem *sys2;

   printf("\nTrading!");

      //get total wanted traded
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (jumpnum=0; jumpnum<(*sys1).njumps; jumpnum++) {

         sys2=&systems_stack[ (*sys1).jumps[jumpnum].targetid ];

            //if we haven't already visited this jump
         if ( i < (*sys2).id ) {

            for (goodnum=0; goodnum<econ_nprices; goodnum++) {

               // printf("\nTrade current is %f, prices at %f and %f",( fabs((*sys1).prices[goodnum] - (*sys2).prices[goodnum] )
               //    / (*sys1).jumps[jumpnum].jump_resistance ),(*sys1).prices[goodnum],(*sys2).prices[goodnum]);

               total_wanted_trade +=  ( fabs((*sys1).prices[goodnum] - (*sys2).prices[goodnum] )
                  / (*sys1).jumps[jumpnum].jump_resistance );
            }
         }
      }
   }

   printf("\nWanted trade is %f, trade_max is %f",total_wanted_trade,trade_max);
   current_modifier = fminf( 1.0 ,trade_max / total_wanted_trade );

   printf("\nCurrent modifier set at %f",current_modifier);


      //Trade!
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (jumpnum=0; jumpnum<(*sys1).njumps; jumpnum++) {

         sys2=&systems_stack[ (*sys1).jumps[jumpnum].targetid ];

            //if we haven't already visited this jump
         if ( i < (*sys2).id ) {

            for (goodnum=0; goodnum<econ_nprices; goodnum++) {

                  //@@@ trade goods for credits!

                  //average of the two prices
               price =  ( fabs((*sys1).prices[goodnum] - (*sys2).prices[goodnum] ) / 2 );

                     //amount to be traded: current (with direction) time current_modifier
               trade = current_modifier * ( ( (*sys1).prices[goodnum] - (*sys2).prices[goodnum] )
                  / (*sys1).jumps[jumpnum].jump_resistance );

               (*sys1).credits               += price * trade;
               (*sys2).credits               -= price * trade;

               (*sys1).stockpiles[goodnum]   -= trade;
               (*sys2).stockpiles[goodnum]   += trade;

            }
         }
      }
   }


   printf("\nTrading done");

}




/**
 * @brief Updates the economy.
 *
 *    @param dt Deltatick in NTIME.
 */
void economy_update( unsigned int dt )
{

   uint i=0;

   for (i=0; i<dt; i+=10000000) {

      trade_update();
      produce_consume();
      refresh_prices();

   }

}






/**
 * @brief Destroys the economy.
 */
void economy_destroy (void)
{
   int i;

   /* Must be initialized. */
   if (!econ_initialized)
      return;

   /* Clean up the prices in the systems stack. */
   for (i=0; i<systems_nstack; i++) {
      if (systems_stack[i].prices != NULL) {
         free(systems_stack[i].prices);
         free(systems_stack[i].stockpiles);
         free(systems_stack[i].prod_mods);
         systems_stack[i].prices    = NULL;
         systems_stack[i].stockpiles= NULL;  //@@@ Correct, yes?
         systems_stack[i].prod_mods = NULL;
      }
   }

   /* Economy is now deinitialized. */
   econ_initialized = 0;
}





