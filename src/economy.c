/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file economy.c
 *
 * @brief Handles economy stuff.
 *
 */


#include "economy.h"

#include "naev.h"

#include <stdio.h>
#include "nstring.h"
#include <stdint.h>
#include <limits.h>

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


#define SYS_STARTING_CREDITS   100000000. /**< ### How many credits are initially given to each system*/
#define SYS_STARTING_GOODS     10000. 
#define PL_STARTING_CREDITS    1000000.
#define PL_STARTING_GOODS      100.

#define INITIAL_TRADE_MODIFIER     0.99  /**< How much trade that wants to happen actually happens. 
                                             High means low price difference, low means large differences */
#define INITIAL_PRODUCTION_MODIFIER 1.0 /**< galaxial production modifier */

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

double **xml_prodmods; /**< the asset production modifiers definied in the xml, size planet_nstack */

/*
 * Global Economy variables. Most of it stored with systems 
 */
static int econ_initialized   = 0; /**< Is economy system initialized? */
static int *econ_comm         = NULL; /**< Commodities to calculate. */
int econ_nprices              = 0; /**< Number of prices to calculate. */
double trade_modifier         = INITIAL_TRADE_MODIFIER; /** How much trade actually happens */
double production_modifier    = INITIAL_PRODUCTION_MODIFIER; /**< multiplier of how much production happens */

/*
 * Prototypes.
 */
/* Commodity. */
static void commodity_freeOne( Commodity* com );
static int commodity_parse( Commodity *temp, xmlNodePtr parent );

/* Economy. */
credits_t economy_getPrice( const Commodity *com, 
   const StarSystem *sys, const Planet *p ); /* externed in space.c */
void produce_consume(void);
void trade_update(void);
void refresh_sys_prices(StarSystem *sys);   /* refresh the prices of a system */
void refresh_pl_prices(Planet *pl); /* refresh the prices of a planet */



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
   if (name==NULL){
      WARN("Commodity name is NULL!\n");
      return NULL;
   }
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
   int ind;
   double price;

   /* Find the index */
   ind = com->index;

   /* Check if found. */
   if (ind >= econ_nprices) {
      WARN("Commodity '%s' is not buyable or sellable\n", com->name);
      return 0;
   }

      /* Calculate price. */
   if (planet_isFlag(p, PL_ECONOMICALLY_ACTIVE))
      price = PRICE(p->credits, p->stockpiles[ind]);
   else
      price = PRICE(sys->credits, sys->stockpiles[ind]);

   return (credits_t) price;
}

/**
 * @brief Gets the price for purchasing n tons of goods from an asset with finite funds
 *
 */
credits_t price_of_buying(int n_tons, double p_creds, double p_goods)
{

   int i;
   int increment = 1;   /* the granularity of the approximation */

   increment *= (n_tons>0) ? 1 : -1;

   credits_t t_price;

      /* if trying to buy more than is in store, return almost max value */
   if (p_goods-(double)n_tons<=1.0){
      t_price  = CREDITS_MAX*increment;
      return t_price;
   }

   double price;
   double price_after;  /* next price if price was used */
   double avg_price; /* avg of price and price_after */

   double f_price = 0;

   for (i=0; i!=n_tons; i+=increment){

      price = PRICE(p_creds, p_goods);
      price_after = PRICE(p_creds+price*increment, p_goods-increment);
      avg_price = (price+price_after) / 2;

      p_creds+=avg_price*increment;
      p_goods-=increment;

      f_price+=avg_price;
   }

   t_price = (credits_t) f_price;

   return t_price;
}

/**
 * @brief Initializes the economy.
 *
 *    @return 0 on success.
 */
int economy_init (void)
{     
   printf("\nInit ing economy");

   int i, goodnum, p;
   StarSystem *sys;
   Planet *pl;

   /* Must not be initialized. */
   if (econ_initialized){
      printf("economy already initialized\n");
      return 0;
   }
   if (xml_prodmods==NULL){
      printf("must load the galaxy first before starting the economy!\n");
      return 0;
   }

   trade_modifier         = INITIAL_TRADE_MODIFIER; /** How much trade that wants to happen actually happens */
   production_modifier    = INITIAL_PRODUCTION_MODIFIER; /**< multiplier of how much production happens */


   /* Allocate price space, commodity space, credits, and stockpiles */
   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];
      if (sys->prices != NULL)
         free(sys->prices);

      sys->prices = calloc(econ_nprices, sizeof(double));
      sys->stockpiles = calloc(econ_nprices, sizeof(double));
      sys->credits = SYS_STARTING_CREDITS;

      /* set the default starting stockpiles */
      for (goodnum=0; goodnum<econ_nprices; goodnum++){
         sys->stockpiles[goodnum] = SYS_STARTING_GOODS;
      }

      /* set the planet stuff */
      for (p=0; p<planet_nstack; p++){
         pl = &planet_stack[p];
         if (planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)){
            pl->credits = PL_STARTING_CREDITS;
            pl->prices = calloc(econ_nprices, sizeof(double));
            pl->stockpiles = calloc(econ_nprices, sizeof(double));
            for (goodnum=0; goodnum<econ_nprices; goodnum++){
               pl->stockpiles[goodnum] = PL_STARTING_GOODS;
            }
         }
      }

      sys->bought = calloc(econ_nprices, sizeof(double)); //bought should be removed later
   }

   /* set the production modifiers to their default values */
   for (i=0; i<planet_nstack; i++) {
      pl = planet_stack+i;

      if (pl->prod_mods!=NULL)
         WARN("planet %d, %s already has prod mods [?]", pl->id, pl->name); //rm me
      else if (planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
         pl->prod_mods = (double *) malloc(econ_nprices*sizeof(double));

         /* if there are no values to put in */
      if (xml_prodmods[i]==NULL)
         continue;


      memcpy(pl->prod_mods, xml_prodmods[i], econ_nprices*sizeof(double));
   }

   /* Mark economy as initialized. */
   econ_initialized = 1;

   /* Refresh economy. */
   refresh_prices();

   return 0;
}

   /* How to produce/consume in a single update */
double production(double mod, double goods)
{
      /* returns to much to produce/consume. Will work unless mod<-18000 */
   if (mod >= 0)
      return production_modifier * mod * (180000 / (goods));
   else
      return production_modifier * mod * (goods/(18000));
} 


/* Every asset produces and consumes their appropriate amount */
void produce_consume(void)
{

   int p, goodnum;
   double mod, goods;
   Planet *pl;

      /* for every planet, produce and consume */
   for (p=0; p<planet_nstack; p++){

      pl = planet_stack+p;

      if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
         continue;

      for (goodnum=0; goodnum<econ_nprices; goodnum++) {

         if (pl->prod_mods==NULL || pl->stockpiles==NULL){  //rm me when this is done
            WARN("planet %d ethier does not have any prod_mods or stockpiles!\n",pl->id);
            continue;
         }

         mod   = pl->prod_mods[goodnum];
         goods = pl->stockpiles[goodnum];

         if (mod==0.0)
            continue;

         pl->stockpiles[goodnum]+=production(mod,goods);
      }
   }
}


/* refresh all prices in the system */
void refresh_prices(void)
{
   int s, p;
   StarSystem *sys;
   Planet *pl;

   for (s=0; s<systems_nstack; s++){
      sys = &systems_stack[s];
      refresh_sys_prices(sys);
      for (p=0; p<sys->nplanets; p++){
         pl = sys->planets[p];
         if (planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
            continue;
         refresh_pl_prices(pl);
      }
   }

}

/* Refresh price of planet (if the credits or stockpiles have been changed) */
void refresh_pl_prices(Planet *pl)
{
   if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
      return;

   int goodnum;
   double credits, goods, price;
   Commodity *comm;

      /* update prices on every good */
   for (goodnum=0; goodnum<econ_nprices; goodnum++) {
      credits  = pl->credits;
      goods    = pl->stockpiles[goodnum];
      comm     = &commodity_stack[goodnum];

         /* price defined in XML */
      price  = (double) comm->price; 
      pl->prices[goodnum] = price * PRICE(credits,goods);
   }
}

/* refresh price of a system (if the credits or stockpiles have been changed) */
void refresh_sys_prices(StarSystem *sys)
{
   int goodnum;
   double credits, goods, price;
   Commodity *comm;

      /* update prices on every good */
   for (goodnum=0; goodnum<econ_nprices; goodnum++) {
      credits  = sys->credits;
      goods    = sys->stockpiles[goodnum];
      comm     = &commodity_stack[goodnum];

         /* price defined in XML */
      price  = (double) comm->price; 
      sys->prices[goodnum] = price * PRICE(credits,goods);
   }
}

/* trade in the galaxy, does not update prices */
void trade_update(void)
{

   int i, p, goodnum, jumpnum;

   double price;
   double trade;

   Planet *pl;

   StarSystem *sys1;
   StarSystem *sys2;

   //sys->bought is unnecessary, and is only for viewing the modified map
      /* set sys.bought to 0 */
   for (i=0;i<systems_nstack; i++) {
      for (goodnum=0; goodnum<econ_nprices; goodnum++) {
         systems_stack[i].bought[goodnum]=0.;
      }
   }

      /* Trade between and it's respective economically active planets, and it's neighbors */
   for (i=0;i<systems_nstack; i++) {

      sys1=&systems_stack[i];

         /* trade with planets */
      for (p=0; p<sys1->nplanets; p++){
         pl = sys1->planets[p];
         if (!(planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)))
            continue;

         /* trade */
         for (goodnum=0; goodnum<econ_nprices; goodnum++){
            price = (sys1->credits+pl->credits) / (sys1->stockpiles[goodnum]+pl->stockpiles[goodnum]);

               //If everything works fine, try removing that .99
            trade = .99 * (sys1->credits * pl->stockpiles[goodnum] - pl->credits*sys1->stockpiles[goodnum])
                  / (price * (sys1->stockpiles[goodnum]+pl->stockpiles[goodnum]) + sys1->credits+pl->credits);

            sys1->credits               -= price * trade;
            pl->credits                 += price * trade;

            sys1->stockpiles[goodnum]   += trade;
            pl->stockpiles[goodnum]     -= trade;

         }
      }

         /* trade w/neighbors */
      for (jumpnum=0; jumpnum<sys1->njumps; jumpnum++) {

         sys2=&systems_stack[ sys1->jumps[jumpnum].targetid ];

            /* if we haven't already visited this jump */
         if ( i < sys2->id )
            continue;

         for (goodnum=0; goodnum<econ_nprices; goodnum++) {

               /* trade at the price of both system's total credits and goods */
            price =  ( (sys1->credits+sys2->credits) / (sys1->stockpiles[goodnum]+sys2->stockpiles[goodnum]) );

               /* Trade at a single point till equiblibrium */
            trade = trade_modifier * (sys1->credits * sys2->stockpiles[goodnum] - sys2->credits*sys1->stockpiles[goodnum])
               / (price * (sys1->stockpiles[goodnum]+sys2->stockpiles[goodnum]) + sys1->credits+sys2->credits);

            sys1->credits               -= price * trade;
            sys2->credits               += price * trade;

            sys1->stockpiles[goodnum]   += trade;
            sys2->stockpiles[goodnum]   -= trade;

            sys1->bought[goodnum]       += trade;
            sys2->bought[goodnum]       -= trade;
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

   unsigned int i=0;

   refresh_prices();

   printf("Updating economy %d cycles\n", dt/10000000);

      /* Trade and produce/consume, is passed 10000000 every standard jump and landing */
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
   int i, p;
   StarSystem *sys;
   Planet *pl;

   /* Must be initialized. */
   if (!econ_initialized)
      return;

   printf("\nDestroying economy");

   /* Clean up the prices in the systems stack. */
   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];
      if (sys->prices != NULL) {
         free(sys->prices);
         free(sys->stockpiles);
         free(sys->bought);
         sys->prices    = NULL;
         sys->stockpiles= NULL;
         sys->bought    = NULL;
      }
   }

   for (p=0; p<planet_nstack; p++){
      pl = planet_stack+p;
      if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
         continue;
      free(pl->stockpiles);
      pl->stockpiles = NULL;
      free(pl->prod_mods); 
      pl->prod_mods = NULL;

   }

   /* Economy is now deinitialized. */
   econ_initialized = 0;
}





