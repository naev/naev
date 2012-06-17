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
#include <math.h> //REMOVE ME this is for sqrt()

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
#include "land.h"


#define XML_COMMODITY_ID      "Commodities" /**< XML document identifier */
#define XML_COMMODITY_TAG     "commodity" /**< XML commodity identifier. */


/*
 * Economy Nodal Analysis parameters.
 */
 // ### will have to modifiy
#define ECON_BASE_RES      5 /**< Base resistance value for any system. */
#define ECON_SELF_RES      3. /**< Additional resistance for the self node. */
#define ECON_FACTION_MOD   0.1 /**< Modifier on Base for faction standings. */

         //Not something I inteded to use, but can be useful nonetheless
#define ECON_PROD_VAR      0.01 /**< Defines the variability of production. */

#define STARTING_CREDITS   100000000. /**< ### How many credits are initially given to each system*/
#define STARTING_GOODS     100000.  //originally 200000


#define AVG_POPULATION     50000000 /**< Used for prod_mods as a divisor to populations in production modifiers */
#define TRADE_MODIFIER     .99  /**< How much trade that wants to happen actually happens. Adjusts price changes */

/* commodity stack */
Commodity* commodity_stack = NULL; /**< Contains all the commodities. */
static int commodity_nstack       = 0; /**< Number of commodities in the stack. */


/* systems stack. */
extern StarSystem *systems_stack; /**< Star system stack. */
extern int systems_nstack; /**< Number of star systems. */

/* planet stack  */
extern Planet *planet_stack; /**< Planet stack. */
extern int planet_nstack; /**< Num of planets */
extern int commodity_mod;  /**< Smallest unit the player can buy, for player prices */


/*
 * Global Economy variables. Most of it stored with systems 
 */
static int econ_initialized   = 0; /**< Is economy system initialized? */
static int *econ_comm         = NULL; /**< Commodities to calculate. */
int econ_nprices       = 0; /**< Number of prices to calculate. */

static int iters=0;     //REMOVE ME

/*
 * Prototypes.
 */
/* Commodity. */
static void commodity_freeOne( Commodity* com );
static int commodity_parse( Commodity *temp, xmlNodePtr parent );

/* Economy. */
// static double econ_calcJumpR( StarSystem *A, StarSystem *B );
credits_t economy_getPrice( const Commodity *com, 
   const StarSystem *sys, const Planet *p ); /* externed in space.c */
credits_t economy_getCost( const Commodity *com, const StarSystem *sys, int buying);
void produce_consume(void);
void trade_update(void);
void refresh_prices(void);


// void economy_update( unsigned int dt );


//### These are planet classification production modifiers. These are multiplied
//     by population to get final planetary modifiers. Not sure how this should be dealt with
//planet classifications are in space.h
//Commodities are Food, Ore, Industrial Goods, Medicine, Luxury Items
double planet_class_mods[][5] = {
   { 0., 0., 0., 0., 0.},
   {130., 100.,-45.,100., 120.},
   { 0., 1200., 0.,-3., 600.},
   {-6., 1800., 3.,-3., 300.},
   {-6., 5000.,-6.,-3., 0.},
   {-3., 2600., 0.,-3.,-2.},
   {-6.,3200., 6.,-3., 0.},
   { 3., 2400.,-3., 6., 900.},
   {-15.,3200.,-6.,9.,1800.},
   {-9., 1800.,-6., 9.,-2.},
   {-3., 1300.,-3.,-3., 280.},
   { 8.,-1,-2., 1.,-2.},
   {10.,-2,-2.,-1.,-2.},
   {14.,-3.,-5.,8.,-5.},
   {-5., 1200.,-2., 6., 650.},
   {20., 130.,-130.,-2.,-.5},
   { 6., 100.,-5.,-1., 150.},
   { 0., 600.,-2.,-1., 0.},
   {-3., 600.,-1., 0., 100.},
   {-1., 300.,-1., 1., 100.},
   {-1., 300.,-1., 1., 100.},
   { 0., 0., 0., 0., 0.},
   { 0., 0., 0., 0., 0.},
   { 0., 0., 0., 0., 0.},
   {-300000.,-400000.,1600000., 300000., 100000.}, //stations are buffed to account for population
   {-400000.,-600000.,2000000.,-100000., 0.},
   {-300000.,-400000.,1400000., 00., 100000.},  //
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
{           //### is this necessary?
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
            commodity_stack[econ_nprices].index=econ_nprices;
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
   int i;
   double price;

   /* Find what commodity that is. */
   for (i=0; i<econ_nprices; i++)
      if (econ_comm[i] == com->index)
         break;


   /* Check if found. */
   if (i >= econ_nprices) {
      WARN("Price for commodity '%s' not known.", com->name);
      return 0;
   }

   /* Calculate price. */
   price = sys->prices[i];
   return (credits_t) price;
}

/**
 * @brief Calculates the resistance between two star systems.
 *
 *    @param A Star system to calculate the resistance between.
 *    @param B Star system to calculate the resistance between.
 *    @return Resistance between A and B.
 */
// static double econ_calcJumpR( StarSystem *A, StarSystem *B )   //### unused for now
// {
//    double R;   

//    /* Set to base to ensure price change. */
//    R = ECON_BASE_RES;

//    /* Modify based on system conditions. */
//    R -= (A->nebu_density + B->nebu_density) / 1000.; /* Density shouldn't affect much. */
//    R -= (A->nebu_volatility + B->nebu_volatility) / 100.; /* Volatility should. */

//    /* Modify based on global faction. */
//    if ((A->faction != -1) && (B->faction != -1)) {
//       if (areEnemies(A->faction, B->faction))
//          R -= ECON_FACTION_MOD * ECON_BASE_RES;
//       else if (areAllies(A->faction, B->faction))
//          R += ECON_FACTION_MOD * ECON_BASE_RES;
//    }

//    /* @todo Modify based on trader/faction presence. */

//    return R;
// }


/**
 * @brief Initializes the economy.
 *
 *    @return 0 on success.
 */
int economy_init (void) //Not for loading loading economies
{     
   printf("\nInit ing economy");

   int i;
   int i0;

   int goodnum;

   StarSystem *sys1;
   // StarSystem *sys2;

   Planet *planet;

   /* Must not be initialized. */
   if (econ_initialized)
      return 0;


   /* Allocate price space, commodity space, and credits stockpile */
   for (i=0; i<systems_nstack; i++) {
      if (systems_stack[i].prices != NULL)
         free(systems_stack[i].prices);

      systems_stack[i].prices = calloc(econ_nprices, sizeof(double));
      systems_stack[i].stockpiles = calloc(econ_nprices, sizeof(double));
      systems_stack[i].credits = STARTING_CREDITS;
      systems_stack[i].prod_mods = calloc(econ_nprices, sizeof(double));

      systems_stack[i].bought = calloc(econ_nprices, sizeof(double)); //REMOVE ME, along with StarSystem.bought
   }

   /* Initialize starting values */ 
   for (i=0; i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      (*sys1).credits = STARTING_CREDITS;

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         (*sys1).stockpiles[goodnum] = STARTING_GOODS;
      }
   }

   /* Set up production modifiers, based on asset classifications */
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

   return 0;
}

   /* How to produce/consume in a single update */
double production(double mod, double goods)
{
      //### @@@ Should this be defined as a macro?
   if (mod >= 0)
      return mod * (180000 / (goods));
   else
      return mod * (goods/(18000));   //will work for all cases except when mod>1800000
} 


/* Every system produces and consumes their appropriate amount */
void produce_consume(void)
{

   int i;
   int goodnum;

   double mod;
   double goods;

   StarSystem *sys1;

      /* for every system produce and consume */
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
   double price;
   Commodity *comm;

   StarSystem *sys1;

      /* for every system, update prices on every good */
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         credits  = (*sys1).credits;
         goods    = (*sys1).stockpiles[goodnum];
         comm     = &commodity_stack[goodnum];

         price  = (double) (*comm).price; //price defined in XML

         (*sys1).prices[goodnum] = price * PRICE(credits,goods);

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

   StarSystem *sys1;
   StarSystem *sys2;

   // printf("\nTrading!");

      //REMOVE ME set sys.bought to 0
   for (i=0;i<systems_nstack; i++) {
      for (goodnum=0; goodnum<econ_nprices; goodnum++) {
         systems_stack[i].bought[goodnum]=0.;
      }
   }

      /* Trade! */
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

      for (jumpnum=0; jumpnum<(*sys1).njumps; jumpnum++) {

         sys2=&systems_stack[ (*sys1).jumps[jumpnum].targetid ];

            //if we haven't already visited this jump
         if ( i < (*sys2).id ) {

            for (goodnum=0; goodnum<econ_nprices; goodnum++) {

                  //@@@ trade goods for credits!

                  //average of the two prices
               price =  ( fabs((*sys1).prices[goodnum] + (*sys2).prices[goodnum] ) / 2 );

                     //amount to be traded: if trade_modifier is set to 1, after a trade prices will be equal
                        //no matter what the price
               trade = TRADE_MODIFIER * ((*sys1).credits * (*sys2).stockpiles[goodnum] - (*sys2).credits*(*sys1).stockpiles[goodnum])
                  /(price * ((*sys1).stockpiles[goodnum]+(*sys2).stockpiles[goodnum]) + (*sys1).credits+(*sys2).credits);

               (*sys1).credits               -= price * trade;
               (*sys2).credits               += price * trade;

               (*sys1).stockpiles[goodnum]   += trade;
               (*sys2).stockpiles[goodnum]   -= trade;

               (*sys1).bought[goodnum]       += trade;   //REMOVE ME
               (*sys2).bought[goodnum]       -= trade;
            }
         }
      }
   }
}

/**
 * @brief Updates the economy.
 *
 *    @param dt Deltatick in NTIME.
 */
void economy_update( unsigned int dt )
{

   uint i=0;

   refresh_prices();

   printf("Updating economy");

      /* Trade and produce/consume */
   // for (i=0; i<dt; i+=10000000) {
   for (i=0; i<dt; i+=10000000) {   //@@@ changed this to run 1x every STP

      iters++;

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
   printf("\nDestroying economy");

   /* Must be initialized. */
   if (!econ_initialized)
      return;

   /* Clean up the prices in the systems stack. */
   for (i=0; i<systems_nstack; i++) {
      if (systems_stack[i].prices != NULL) {
         free(systems_stack[i].prices);
         free(systems_stack[i].stockpiles);
         free(systems_stack[i].prod_mods);
         free(systems_stack[i].bought);
         systems_stack[i].prices    = NULL;
         systems_stack[i].stockpiles= NULL;  //@@@ Correct, yes?
         systems_stack[i].prod_mods = NULL;
         systems_stack[i].bought    = NULL;
      }
   }

   /* Economy is now deinitialized. */
   econ_initialized = 0;
}





