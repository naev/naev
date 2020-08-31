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
#include "player.h"
#include "rng.h"
#include "space.h"
#include "ntime.h"


/*
 * Economy Nodal Analysis parameters.
 */
#define ECON_BASE_RES      30. /**< Base resistance value for any system. */
#define ECON_SELF_RES      3. /**< Additional resistance for the self node. */
#define ECON_FACTION_MOD   0.1 /**< Modifier on Base for faction standings. */
#define ECON_PROD_MODIFIER 500000. /**< Production modifier, divide production by this amount. */
#define ECON_PROD_VAR      0.01 /**< Defines the variability of production. */


/* systems stack. */
extern StarSystem *systems_stack; /**< Star system stack. */
extern int systems_nstack; /**< Number of star systems. */


/* @TODO get rid of these externs. */
extern Commodity* commodity_stack;
extern int commodity_nstack;


/*
 * Nodal analysis simulation for dynamic economies.
 */
static int econ_initialized   = 0; /**< Is economy system initialized? */
static int econ_queued        = 0; /**< Whether there are any queued updates. */
static cs *econ_G             = NULL; /**< Admittance matrix. */
int *econ_comm         = NULL; /**< Commodities to calculate. */
int econ_nprices       = 0; /**< Number of prices to calculate. */


/*
 * Prototypes.
 */
/* Economy. */
//static double econ_calcJumpR( StarSystem *A, StarSystem *B );
//static double econ_calcSysI( unsigned int dt, StarSystem *sys, int price );
//static int econ_createGMatrix (void);

/*
 * Externed prototypes.
 */
int economy_sysSave( xmlTextWriterPtr writer );
int economy_sysLoad( xmlNodePtr parent );



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
   /* Get current time in periods.
    */
   return economy_getPriceAtTime( com, sys, p, ntime_get());
}

/**
 * @brief Gets the price of a good on a planet in a system.
 *
 *    @param com Commodity to get price of.
 *    @param sys System to get price of commodity.
 *    @param p Planet to get price of commodity.
 *    @param t Time to get price at, eg as retunred by ntime_get()
 *    @return The price of the commodity.
 */
credits_t economy_getPriceAtTime( const Commodity *com,
                                  const StarSystem *sys, const Planet *p, ntime_t tme )
{
   int i, k;
   double price;
   double t;
   CommodityPrice *commPrice;
   (void) sys;
   /* Get current time in periods.
    * Note, taking off and landing takes about 1e7 ntime, which is 1 period.  
    * Time does not advance when on a planet. 
    * Journey with a single jump takes approx 3e7, so about 3 periods.
    */
   t = ntime_convertSeconds( tme ) / NT_PERIOD_SECONDS;

   /* Get position in stack. */
   k = com - commodity_stack;

   /* Find what commodity that is. */
   for (i=0; i<econ_nprices; i++)
      if (econ_comm[i] == k)
         break;

   /* Check if found. */
   if (i >= econ_nprices) {
      WARN(_("Price for commodity '%s' not known."), com->name);
      return 0;
   }

   /* and get the index on this planet */
   for ( i=0; i<p->ncommodities; i++) {
     if ( ( strcmp(p->commodities[i]->name, com->name) == 0 ) )
       break;
   }
   if (i >= p->ncommodities) {
     WARN(_("Price for commodity '%s' not known on this planet."), com->name);
     return 0;
   }
   commPrice = &p->commodityPrice[i];
   /* Calculate price. */
   /* price  = (double) com->price; */
   /* price *= sys->prices[i]; */
   price = (commPrice->price + commPrice->sysVariation
            * sin(2 * M_PI * t / commPrice->sysPeriod)
         + commPrice->planetVariation
            * sin(2 * M_PI * t / commPrice->planetPeriod));
   return (credits_t) (price+0.5);/* +0.5 to round */
}

/**
 * @brief Gets the average price of a good on a planet in a system, using a rolling average over the times the player has landed here..
 *
 *    @param com Commodity to get price of.
 *    @param p Planet to get price of commodity.
 *    @return The average price of the commodity.
 */
int economy_getAveragePlanetPrice( const Commodity *com, const Planet *p, credits_t *mean, double *std)
{
   int i,k;
   CommodityPrice *commPrice;
   /* Get position in stack */
   k = com - commodity_stack;

   /* Find what commodity this is */
   for ( i=0; i<econ_nprices; i++ )
      if (econ_comm[i] == k)
         break;

   /* Check if found */
   if ( i>= econ_nprices) {
      WARN(_("Average price for commodity '%s' not known."), com->name);
      *mean=0;
      *std=0;
      return -1;
   }

   /* and get the index on this planet */
   for ( i=0; i<p->ncommodities; i++) {
     if ( ( strcmp(p->commodities[i]->name, com->name) == 0 ) )
       break;
   }
   if (i >= p->ncommodities) {
      WARN(_("Price for commodity '%s' not known on this planet."), com->name);
      *mean = 0;
      *std = 0;
      return -1;
   }
   commPrice = &p->commodityPrice[i];
   if ( commPrice->cnt>0 ) {
      *mean = (credits_t)(commPrice->sum/commPrice->cnt + 0.5); /* +0.5 to round*/
      *std = (sqrt(commPrice->sum2 / commPrice->cnt
               - (commPrice->sum * commPrice->sum)
                  / (commPrice->cnt * commPrice->cnt)));
   } else {
      *mean = 0;
      *std = 0;
   }
   return 0;
}


/**
 * @brief Gets the average price of a good as seen by the player (anywhere).
 *
 *    @param com Commodity to get price of.
 *    @return The average price of the commodity.
 */

int economy_getAveragePrice( const Commodity *com, credits_t *mean, double *std ) {
   int i,j,k;
   CommodityPrice *commPrice;
   StarSystem *sys;
   Planet *p;
   double av = 0;
   double av2 = 0;
   int cnt = 0;
   /* Get position in stack */
   k = com - commodity_stack;

   /* Find what commodity this is */
   for ( i=0; i<econ_nprices; i++ )
      if (econ_comm[i] == k)
         break;

   /* Check if found */
   if ( i>= econ_nprices) {
      WARN(_("Average price for commodity '%s' not known."), com->name);
      *mean = 0;
      *std = 0;
      return 1;
   }
   for ( i=0; i<systems_nstack ; i++) {
      sys = &systems_stack[i];
      for ( j=0; j<sys->nplanets; j++) {
         p = sys->planets[j];
         /* and get the index on this planet */
         for ( k=0; k<p->ncommodities; k++) {
            if ( ( strcmp(p->commodities[k]->name, com->name) == 0 ) )
               break;
         }
         if (k < p->ncommodities) {
            commPrice=&p->commodityPrice[k];
            if ( commPrice->cnt>0) {
               av+=commPrice->sum/commPrice->cnt;
               av2+=commPrice->sum*commPrice->sum/(commPrice->cnt*commPrice->cnt);
               cnt++;
            }
         }
      }
   }
   if (cnt > 0) {
      av /= cnt;
      av2 = sqrt(av2/cnt - av*av);
   }
   *mean = (credits_t)(av + 0.5);
   *std = av2;
   return 0;
}


#if 0
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

   /* @todo Modify based on fleets. */

   return R;
}


/**
 * @brief Calculates the intensity in a system node.
 *
 * @todo Make it time/item dependent.
 */
static double econ_calcSysI( unsigned int dt, StarSystem *sys, int price )
{
   int i;
   double I;
   double prodfactor, p;
   double ddt;
   Planet *planet;

   ddt = (double)(dt / NTIME_UNIT_LENGTH);

   /* Calculate production level. */
   p = 0.;
   for (i=0; i<sys->nplanets; i++) {
      planet = sys->planets[i];
      if (planet_hasService(planet, PLANET_SERVICE_INHABITED)) {
         /*
          * Calculate production.
          */
         /* We base off the current production. */
         prodfactor  = planet->cur_prodfactor;
         /* Add a variability factor based on the Gaussian distribution. */
         prodfactor += ECON_PROD_VAR * RNG_2SIGMA() * ddt;
         /* Add a tendency to return to the planet's base production. */
         prodfactor -= ECON_PROD_VAR *
               (planet->cur_prodfactor - prodfactor)*ddt;
         /* Save for next iteration. */
         planet->cur_prodfactor = prodfactor;
         /* We base off the sqrt of the population otherwise it changes too fast. */
         p += prodfactor * sqrt(planet->population);
      }
   }

   /* The intensity is basically the modified production. */
   I = p / ECON_PROD_MODIFIER;

   return I;
}


/**
 * @brief Creates the admittance matrix.
 *
 *    @return 0 on success.
 */
static int econ_createGMatrix (void)
{
   int ret;
   int i, j;
   double R, Rsum;
   cs *M;
   StarSystem *sys;

   /* Create the matrix. */
   M = cs_spalloc( systems_nstack, systems_nstack, 1, 1, 1 );
   if (M == NULL)
      ERR(_("Unable to create CSparse Matrix."));

   /* Fill the matrix. */
   for (i=0; i < systems_nstack; i++) {
      sys   = &systems_stack[i];
      Rsum = 0.;

      /* Set some values. */
      for (j=0; j < sys->njumps; j++) {

         /* Get the resistances. */
         R     = econ_calcJumpR( sys, sys->jumps[j].target );
         R     = 1./R; /* Must be inverted. */
         Rsum += R;

         /* Matrix is symmetrical and non-diagonal is negative. */
         ret = cs_entry( M, i, sys->jumps[j].target->id, -R );
         if (ret != 1)
            WARN(_("Unable to enter CSparse Matrix Cell."));
         ret = cs_entry( M, sys->jumps[j].target->id, i, -R );
         if (ret != 1)
            WARN(_("Unable to enter CSparse Matrix Cell."));
      }

      /* Set the diagonal. */
      Rsum += 1./ECON_SELF_RES; /* We add a resistance for dampening. */
      cs_entry( M, i, i, Rsum );
   }

   /* Compress M matrix and put into G. */
   if (econ_G != NULL)
      cs_spfree( econ_G );
   econ_G = cs_compress( M );
   if (econ_G == NULL)
      ERR(_("Unable to create economy G Matrix."));

   /* Clean up. */
   cs_spfree(M);

   return 0;
}
#endif


/**
 * @brief Initializes the economy.
 *
 *    @return 0 on success.
 */
int economy_init (void)
{
   int i;

   /* Must not be initialized. */
   if (econ_initialized)
      return 0;

   /* Allocate price space. */
   for (i=0; i<systems_nstack; i++) {
      if (systems_stack[i].prices != NULL)
         free(systems_stack[i].prices);
      systems_stack[i].prices = calloc(econ_nprices, sizeof(double));
   }

   /* Mark economy as initialized. */
   econ_initialized = 1;

   /* Refresh economy. */
   economy_refresh();

   return 0;
}


/**
 * @brief Increments the queued update counter.
 *
 * @sa economy_execQueued
 */
void economy_addQueuedUpdate (void)
{
   econ_queued++;
}


/**
 * @brief Calls economy_refresh if an economy update is queued.
 */
int economy_execQueued (void)
{
   if (econ_queued)
      return economy_refresh();

   return 0;
}


/**
 * @brief Regenerates the economy matrix.  Should be used if the universe
 *  changes in any permanent way.
 */
int economy_refresh (void)
{
   /* Economy must be initialized. */
   if (econ_initialized == 0)
      return 0;

   /* Create the resistance matrix. */
   //if (econ_createGMatrix())
   //   return -1;

   /* Initialize the prices. */
   economy_update( 0 );

   return 0;
}


/**
 * @brief Updates the economy.
 *
 *    @param dt Deltatick in NTIME.
 */
int economy_update( unsigned int dt )
{
   (void)dt;
   int i, j;
   double *X;
   double scale, offset;
   /*double min, max;*/

   /* Economy must be initialized. */
   if (econ_initialized == 0)
      return 0;

   /* Create the vector to solve the system. */
   X = malloc(sizeof(double)*systems_nstack);
   if (X == NULL) {
      WARN(_("Out of Memory"));
      return -1;
   }

   /* Calculate the results for each price set. */
   for (j=0; j<econ_nprices; j++) {

#if 0
      /* First we must load the vector with intensities. */
      for (i=0; i<systems_nstack; i++)
         X[i] = econ_calcSysI( dt, &systems_stack[i], j );

      /* Solve the system. */
      /** @TODO This should be improved to try to use better factorizations (LU/Cholesky)
       * if possible or just outright try to use some other library that does fancy stuff
       * like UMFPACK. Would be also interesting to see if it could be optimized so we
       * store the factorization or update that instead of handling it individually. Another
       * point of interest would be to split loops out to make the solving faster, however,
       * this may be trickier to do (although it would surely let us use cholesky always if we
       * enforce that condition). */
      ret = cs_qrsol( 3, econ_G, X );
      if (ret != 1)
         WARN(_("Failed to solve the Economy System."));
#endif

      /*
       * Get the minimum and maximum to scale.
       */
      /*
      min = +HUGE_VALF;
      max = -HUGE_VALF;
      for (i=0; i<systems_nstack; i++) {
         if (X[i] < min)
            min = X[i];
         if (X[i] > max)
            max = X[i];
      }
      scale = 1. / (max - min);
      offset = 0.5 - min * scale;
      */

      /*
       * I'm not sure I like the filtering of the results, but it would take
       * much more work to get a good system working without the need of post
       * filtering.
       */
      scale    = 1.;
      offset   = 1.;
      for (i=0; i<systems_nstack; i++)
         systems_stack[i].prices[j] = X[i] * scale + offset;
   }

   /* Clean up. */
   free(X);

   econ_queued = 0;
   return 0;
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
         systems_stack[i].prices = NULL;
      }
   }

   /* Destroy the economy matrix. */
   if (econ_G != NULL) {
      cs_spfree( econ_G );
      econ_G = NULL;
   }

   /* Economy is now deinitialized. */
   econ_initialized = 0;
}

/**
 * @brief Used during startup to set price and variation of the economy, depending on planet information.
 * 
 *    @param planet The planet to set price on.
 *    @param commodity The commodity to set the price of.
 *    @param commodityPrice Where to write the commodity price to.
 *    @return 0 on success.
 */
static int economy_calcPrice( Planet *planet, Commodity *commodity, CommodityPrice *commodityPrice ) {

   CommodityModifier *cm;
   double base, scale, factor;
   char *factionname;

   /* Check the faction is not NULL.*/
   if ( planet->faction == -1 ) {
      WARN(_("Planet '%s' appears to have commodity '%s' defined, but no faction."), planet->name, commodity->name);
      return 1;
   }

   /* Reset price to the base commodity price. */
   commodityPrice->price = commodity->price;
   
   /* Get the cost modifier suitable for planet type/class. */
   cm = commodity->planet_modifier;
   scale = 1.;
   while ( cm != NULL ) {
      if ( ( strcmp( planet->class, cm->name ) == 0 ) ) {
         scale = cm->value;
         break;
      }
      cm = cm->next;
   }
   commodityPrice->price *= scale;
   commodityPrice->planetVariation = 0.5;
   commodityPrice->sysVariation = 0.;
   /*commodityPrice->sum = 0.;
   commodityPrice->sum2 = 0.;
   commodityPrice->cnt = 0;
   commodityPrice->updateTime = 0;*/
   /* Use filename to specify a variation period. */
   base = 100;
   commodity->period = 32 * (planet->gfx_spaceName[strlen(PLANET_GFX_SPACE_PATH)] % 32) + planet->gfx_spaceName[strlen(PLANET_GFX_SPACE_PATH) + 1] % 32;
   commodityPrice->planetPeriod = commodity->period + base;

   /* Use filename of exterior graphic to modify the variation period.  
      No rhyme or reason, just gives some variability. */
   scale = 1 + (strlen(planet->gfx_exterior) - strlen(PLANET_GFX_EXTERIOR_PATH) - 19) / 100.;
   commodityPrice->planetPeriod *= scale;

   /* Use population to modify price and variability.  The tanh function scales from -1 (small population)
      to +1 (large population), done on a log scale.  Price is then modified by this factor, scaled by a 
      value defined in the xml, as is variation.  So for some commodities, prices increase with population,
      while for others, prices decrease. */
   factor = -1;
   if ( planet->population > 0 )
      factor = tanh( ( log((double)planet->population) - log(1e8) ) / 2 );
   base = commodity->population_modifier;
   commodityPrice->price *= 1 + factor * base;
   commodityPrice->planetVariation *= 0.5 - factor * 0.25;
   commodityPrice->planetPeriod *= 1 + factor * 0.5;

   /* Modify price based on faction (as defined in the xml). 
      Some factions place a higher value on certain goods.
      Some factions are more stable than others.*/
   scale = 1.;
   cm = commodity->planet_modifier;

   factionname = faction_name(planet->faction);
   while ( cm != NULL ) {
      if ( strcmp( factionname, cm->name ) == 0 ) {
         scale = cm->value;
         break;
      }
      cm = cm->next;
   }
   commodityPrice->price *= scale;

   /* Range seems to go from 0-5, with median being 2.  Increased range
    * will increase safety and so lower prices and improve stability */
   commodityPrice->price *= (1 - planet->presenceRange/30.);
   commodityPrice->planetPeriod *= 1 / (1 - planet->presenceRange/30.);

   /* Make sure the price is always positive and non-zero */
   commodityPrice->price = MAX( commodityPrice->price, 1 );

   return 0;
}


/**
 * @brief Modifies commodity price based on system characteristics.
 *
 *    @param sys System.
 */
static void economy_modifySystemCommodityPrice(StarSystem *sys)
{
   int i,j,k;
   Planet *planet;
   CommodityPrice *avprice=NULL;
   int nav=0;
   
   for ( i=0; i<sys->nplanets; i++ ) {
      planet=sys->planets[i];
      for ( j=0; j<planet->ncommodities; j++ ) {
        /* Largest is approx 35000.  Increased radius will increase price since further to travel, 
           and also increase stability, since longer for prices to fluctuate, but by a larger amount when they do.*/
         planet->commodityPrice[j].price *= 1 + sys->radius/200000;
         planet->commodityPrice[j].planetPeriod *= 1 / (1 - sys->radius/200000.);
         planet->commodityPrice[j].planetVariation *= 1 / (1 - sys->radius/300000.);

         /* Increase price with volatility, which goes up to about 600.
            And with interference, since systems are harder to find, which goes up to about 1000.*/
         planet->commodityPrice[j].price *= 1 + sys->nebu_volatility/6000.;
         planet->commodityPrice[j].price *= 1 + sys->interference/10000.;
         
         /* Use number of jumps to determine sytsem time period.  More jumps means more options for trade
            so shorter period.  Between 1 to 6 jumps.  Make the base time 1000.*/
         planet->commodityPrice[j].sysPeriod = 2000. / (sys->njumps + 1);
         
         for ( k=0; k<nav; k++) {
            if ( ( strcmp( planet->commodities[j]->name, avprice[k].name ) == 0 ) ) {
               avprice[k].updateTime++;
               avprice[k].price+=planet->commodityPrice[j].price;
               avprice[k].planetPeriod+=planet->commodityPrice[j].planetPeriod;
               avprice[k].sysPeriod+=planet->commodityPrice[j].sysPeriod;
               avprice[k].planetVariation+=planet->commodityPrice[j].planetVariation;
               avprice[k].sysVariation+=planet->commodityPrice[j].sysVariation;
               break;
            }
         }
         if ( k == nav ) {/* first visit of this commodity for this system */
            nav++;
            avprice=realloc( avprice, nav * sizeof(CommodityPrice) );
            avprice[k].name=planet->commodities[j]->name;
            avprice[k].updateTime=1;
            avprice[k].price=planet->commodityPrice[j].price;
            avprice[k].planetPeriod=planet->commodityPrice[j].planetPeriod;
            avprice[k].sysPeriod=planet->commodityPrice[j].sysPeriod;
            avprice[k].planetVariation=planet->commodityPrice[j].planetVariation;
            avprice[k].sysVariation=planet->commodityPrice[j].sysVariation;
         }
      }
   }
   /* Do some inter-planet averaging */
   for ( k=0; k<nav; k++ ) {
      avprice[k].price/=avprice[k].updateTime;
      avprice[k].planetPeriod/=avprice[k].updateTime;
      avprice[k].sysPeriod/=avprice[k].updateTime;
      avprice[k].planetVariation/=avprice[k].updateTime;
      avprice[k].sysVariation/=avprice[k].updateTime;
   }
   /* And now apply the averaging */
   for ( i=0; i<sys->nplanets; i++ ) {
      planet=sys->planets[i];
      for ( j=0; j<planet->ncommodities; j++ ) {
         for ( k=0; k<nav; k++ ) {
            if ( ( strcmp( planet->commodities[j]->name, avprice[k].name ) == 0 ) ) {
               planet->commodityPrice[j].price*=0.25;
               planet->commodityPrice[j].price+=0.75*avprice[k].price;
               planet->commodityPrice[j].sysVariation=0.2*avprice[k].planetVariation;
            }
         }
      }
   }
   sys->averagePrice=avprice;
   sys->ncommodities=nav;
}


/**
 * @brief Calculates smoothing of commodity price based on neighbouring systems
 *
 *    @param sys System.
 */
static void economy_smoothCommodityPrice(StarSystem *sys)
{
   StarSystem *neighbour;
   int nav=sys->ncommodities;
   CommodityPrice *avprice=sys->averagePrice;
   double price;
   int n,i,j,k;
   /*Now modify based on neighbouring systems */
   /*First, calculate mean price of neighbouring systems */
   
   for ( j =0; j<nav; j++ ) {/* for each commodity in this system */
      price=0.;
      n=0;
      for ( i=0; i<sys->njumps; i++ ) {/* for each neighbouring system */
         neighbour=sys->jumps[i].target;
         for ( k=0; k<neighbour->ncommodities; k++ ) {
            if ( ( strcmp( neighbour->averagePrice[k].name, avprice[j].name ) == 0 ) ) {
               price+=neighbour->averagePrice[k].price;
               n++;
               break;
            }
         }
      }
      if (n!=0)
         avprice[j].sum=price/n;
      else
         avprice[j].sum=avprice[j].price;
   }
}

/**
 * @brief Modifies commodity price based on neighbouring systems
 *
 *    @param sys System.
 */
static void economy_calcUpdatedCommodityPrice(StarSystem *sys)
{
   int nav=sys->ncommodities;
   CommodityPrice *avprice=sys->averagePrice;
   Planet *planet;
   int i,j,k;
   for ( j=0; j<nav; j++ ) {
      /*Use mean price to adjust current price */
      avprice[j].price=0.5*(avprice[j].price + avprice[j].sum);
   }
   /*and finally modify assets based on the means */
   for ( i=0; i<sys->nplanets; i++ ) {
      planet=sys->planets[i];
      for ( j=0; j<planet->ncommodities; j++ ) {
         for ( k=0; k<nav; k++ ) {
            if ( ( strcmp(avprice[k].name, planet->commodities[j]->name) == 0 ) ) {
               planet->commodityPrice[j].price = (
                     0.25*planet->commodityPrice[j].price
                        + 0.75*avprice[k].price );
               planet->commodityPrice[j].planetVariation = (
                     0.1 * (0.5*avprice[k].planetVariation
                           + 0.5*planet->commodityPrice[j].planetVariation) );
               planet->commodityPrice[j].planetVariation *= planet->commodityPrice[j].price;
               planet->commodityPrice[j].sysVariation *= planet->commodityPrice[j].price;
               break;
            }
         }
      }
   }
   free( sys->averagePrice );
   sys->averagePrice=NULL;
   sys->ncommodities=0;
}

/**
 * @brief Initialises commodity prices for the sinusoidal economy model.
 *
 */
void economy_initialiseCommodityPrices(void)
{
   int i, j, k;
   Planet *planet;
   StarSystem *sys;
   Commodity *com;
   CommodityModifier *this, *next;
   /* First use planet attributes to set prices and variability */
   for (k=0; k<systems_nstack; k++) {
      sys = &systems_stack[k];
      for ( j=0; j<sys->nplanets; j++ ) {
         planet = sys->planets[j];
         /* Set up the commodity prices on the system, based on its attributes. */
         for ( i=0; i<planet->ncommodities; i++ ) {
            if (economy_calcPrice(planet, planet->commodities[i], &planet->commodityPrice[i]))
               return;
         }
      }
   }
   
   /* Modify prices and availability based on system attributes, and do some inter-planet averaging to smooth prices */
   for ( i=0; i<systems_nstack; i++ ) {
      sys = &systems_stack[i];
      economy_modifySystemCommodityPrice(sys);
   }

   /* Compute average prices for all systems */
   for ( i=0; i<systems_nstack; i++ ) {
      sys = &systems_stack[i];
      economy_smoothCommodityPrice(sys);
   }

   /* Smooth prices based on neighbouring systems */
   for ( i=0; i<systems_nstack; i++ ) {
      sys = &systems_stack[i];
      economy_calcUpdatedCommodityPrice(sys);
   }
   /* And now free temporary commodity information */
   for ( i=0 ; i<commodity_nstack; i++ ) {
      com = &commodity_stack[i];
      next = com->planet_modifier;
      com->planet_modifier = NULL;
      while (next != NULL) {
         this = next;
         next = this->next;
         free(this->name);
         free(this);
      }
      next = com->faction_modifier;
      com->faction_modifier = NULL;
      while (next != NULL) {
         this = next;
         next = this->next;
         free(this->name);
         free(this);
      }
   }
}

/*
 * Calculates commodity prices for a single planet (e.g. as added by the unidiff), and does some smoothing over the system, but not neighbours.
 */

void economy_initialiseSingleSystem( StarSystem *sys, Planet *planet )
{
   int i;
   for ( i=0; i<planet->ncommodities; i++ ) {
      economy_calcPrice(planet, planet->commodities[i], &planet->commodityPrice[i]);
   }
   economy_modifySystemCommodityPrice(sys);
}


void economy_averageSeenPrices( const Planet *p )
{
   int i;
   ntime_t t;
   Commodity *c;
   CommodityPrice *cp;
   credits_t price;
   t = ntime_get();
   for ( i = 0 ; i < p->ncommodities ; i++ ) {
      c=p->commodities[i];
      cp=&p->commodityPrice[i];
      if ( cp->updateTime < t ) { /* has not yet been updated at present time. */
         cp->updateTime = t;
         /* Calculate values for mean and std */
         cp->cnt++;
         price = economy_getPrice(c, NULL, p);
         cp->sum += price;
         cp->sum2 += price*price;
      }
   }
}


void economy_averageSeenPricesAtTime( const Planet *p, const ntime_t tupdate )
{
   int i;
   ntime_t t;
   Commodity *c;
   CommodityPrice *cp;
   credits_t price;
   t = ntime_get();
   for ( i = 0 ; i < p->ncommodities ; i++ ) {
      c=p->commodities[i];
      cp=&p->commodityPrice[i];
      if ( cp->updateTime < t ) { /* has not yet been updated at present time. */
         cp->updateTime = t;
         cp->cnt++;
         price = economy_getPriceAtTime(c, NULL, p, tupdate);
         cp->sum += price;
         cp->sum2 += price*price;
      }
   }

}

/**
 * @brief Clears all system knowledge.
 */
void economy_clearKnown (void)
{
   int i, j, k;
   StarSystem *sys;
   Planet *p;
   CommodityPrice *cp;
   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];
      for (j=0; j<sys->nplanets; j++) {
         p=sys->planets[j];
         for (k=0; k<p->ncommodities; k++) {
            cp=&p->commodityPrice[k];
            cp->cnt=0;
            cp->sum=0;
            cp->sum2=0;
            cp->updateTime=0;
         }
      }
   }
   for (i=0; i<commodity_nstack; i++ ) {
      commodity_stack[i].lastPurchasePrice=0;
   }
}

/**
 * @brief Clears all economy knowledge of a given planet.  Used by the unidiff system.
 */
void economy_clearSinglePlanet(Planet *p)
{
   CommodityPrice *cp;
   int k;
   for ( k=0; k<p->ncommodities; k++ ) {
      cp=&p->commodityPrice[k];
      cp->cnt=0;
      cp->sum=0;
      cp->sum2=0;
      cp->updateTime=0;
   }
}


/**
 * @brief Loads player's economy properties from an XML node.
 *
 *    @param parent Parent node for economy.
 *    @return 0 on success.
 */
int economy_sysLoad( xmlNodePtr parent )
{
   xmlNodePtr node, cur, nodeAsset, nodeCommodity;
   //StarSystem *sys;
   char *str;
   Planet *planet;
   int i;
   CommodityPrice *cp;
   Commodity *c;
   economy_clearKnown();

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"economy")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur, "system")) {
               xmlr_attr(cur,"name",str);
               //sys = system_get(str);
               free(str);
               nodeAsset = cur->xmlChildrenNode;
               do{
                  if (xml_isNode(nodeAsset, "planet")) {
                     xmlr_attr(nodeAsset,"name",str);
                     planet = planet_get(str);
                     free(str);
                     nodeCommodity = nodeAsset->xmlChildrenNode;
                     do{
                        if (xml_isNode(nodeCommodity, "commodity")) {
                           xmlr_attr(nodeCommodity,"name",str);
                           cp = NULL;
                           for (i=0; i<planet->ncommodities; i++) {
                              if ( (strcmp(str,planet->commodities[i]->name) == 0) ) {
                                 cp = &planet->commodityPrice[i];
                                 break;
                              }
                           }
                           free(str);
                           if ( cp != NULL ) {
                              xmlr_attr(nodeCommodity,"sum",str);
                              if (str) {
                                 cp->sum = atof(str);
                                 free(str);
                              }
                              xmlr_attr(nodeCommodity,"sum2",str);
                              if (str) {
                                 cp->sum2 = atof(str);
                                 free(str);
                              }
                              xmlr_attr(nodeCommodity,"cnt",str);
                              if (str) {
                                 cp->cnt = atoi(str);
                                 free(str);
                              }
                              xmlr_attr(nodeCommodity,"time",str);
                              if (str) {
                                 cp->updateTime = atol(str);
                                 free(str);
                              }
                           }
                        }
                     } while (xml_nextNode(nodeCommodity));
                  }
               } while (xml_nextNode(nodeAsset));
            } else if (xml_isNode(cur, "lastPurchase")) {
               xmlr_attr(cur, "name", str);
               if (str) {
                  c=commodity_get(str);
                  c->lastPurchasePrice=atol(xml_raw(cur));
                  free(str);
               }
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   return 0;
}



/**
 * @brief Saves what is needed to be saved for economy.
 *
 *    @param writer XML writer to use.
 *    @return 0 on success.
 */
int economy_sysSave( xmlTextWriterPtr writer )
{
  int i,j,k,doneSys,donePlanet;
   StarSystem *sys;
   Planet *p;
   CommodityPrice *cp;
   /* Save what the player has seen of the economy at each planet */
   xmlw_startElem(writer,"economy");
   for (i=0; i<commodity_nstack; i++) {
      if ( commodity_stack[i].lastPurchasePrice > 0 ) {
         xmlw_startElem(writer, "lastPurchase");
         xmlw_attr(writer,"name","%s",commodity_stack[i].name);
         xmlw_str(writer,"%"PRIu64,commodity_stack[i].lastPurchasePrice);
         xmlw_endElem(writer);
      }
   }
   for (i=0; i<systems_nstack; i++) {
      doneSys=0;
      sys=&systems_stack[i];
      for (j=0; j<sys->nplanets; j++) {
         p=sys->planets[j];
         donePlanet=0;
         for ( k=0; k<p->ncommodities; k++) {
            cp=&p->commodityPrice[k];
            if ( cp->cnt > 0 ) {/* Player has seen this commodity at this location */
               if ( doneSys==0 ) {
                  doneSys=1;
                  xmlw_startElem(writer, "system");
                  xmlw_attr(writer,"name","%s",sys->name);
               }
               if ( donePlanet==0 ) {
                  donePlanet=1;
                  xmlw_startElem(writer, "planet");
                  xmlw_attr(writer,"name","%s",p->name);
               }
               xmlw_startElem(writer, "commodity");
               xmlw_attr(writer,"name","%s",p->commodities[k]->name);
               xmlw_attr(writer,"sum","%f",cp->sum);
               xmlw_attr(writer,"sum2","%f",cp->sum2);
               xmlw_attr(writer,"cnt","%d",cp->cnt);
               xmlw_attr(writer,"time","%"PRIu64,cp->updateTime);
               xmlw_endElem(writer); /* commodity */
            }
         }
         if ( donePlanet==1 )
            xmlw_endElem(writer); /* planet */
      }
      if ( doneSys==1 )
         xmlw_endElem(writer); /* system */
   }
   xmlw_endElem(writer); /* economy */
   return 0;
}
