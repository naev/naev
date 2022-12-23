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
/** @cond */
#include <stdint.h>
#include <stdio.h>

#if HAVE_SUITESPARSE_CS_H
#include <suitesparse/cs.h>
#else /* HAVE_SUITESPARSE_CS_H */
#include <cs.h>
#endif /* HAVE_SUITESPARSE_CS_H */

#include "naev.h"
/** @endcond */

#include "economy.h"

#include "array.h"
#include "log.h"
#include "ndata.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "space.h"
#include "spfx.h"

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

/* @TODO get rid of these externs. */
extern Commodity* commodity_stack;

/*
 * Nodal analysis simulation for dynamic economies.
 */
static int econ_initialized   = 0; /**< Is economy system initialized? */
static int econ_queued        = 0; /**< Whether there are any queued updates. */
static cs *econ_G             = NULL; /**< Admittance matrix. */
int *econ_comm         = NULL; /**< Commodities to calculate. */

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
 * @brief Gets the price of a good on a spob in a system.
 *
 *    @param com Commodity to get price of.
 *    @param sys System to get price of commodity.
 *    @param p Spob to get price of commodity.
 *    @return The price of the commodity.
 */
credits_t economy_getPrice( const Commodity *com,
      const StarSystem *sys, const Spob *p )
{
   /* Get current time in periods. */
   return economy_getPriceAtTime( com, sys, p, ntime_get());
}

/**
 * @brief Gets the price of a good on a spob in a system.
 *
 *    @param com Commodity to get price of.
 *    @param sys System to get price of commodity.
 *    @param p Spob to get price of commodity.
 *    @param tme Time to get price at, eg as retunred by ntime_get()
 *    @return The price of the commodity.
 */
credits_t economy_getPriceAtTime( const Commodity *com,
      const StarSystem *sys, const Spob *p, ntime_t tme )
{
   (void) sys;
   int i, k;
   double price;
   double t;
   CommodityPrice *commPrice;

   /* If commodity is using a reference, just return that. */
   if (com->price_ref != NULL) {
      const Commodity *ref = commodity_get( com->price_ref );
      if (ref==NULL)
         return 1e6; /* Just arbitrary large number so players notice. */
      price = economy_getPriceAtTime( ref, sys, p, tme ) * com->price_mod;
      return (credits_t) (price+0.5);
   }

   /* Constant price. */
   if (commodity_isFlag(com, COMMODITY_FLAG_PRICE_CONSTANT))
      return com->price;

   /* Get current time in periods.
    * Note, taking off and landing takes about 1e7 ntime, which is 1 period.
    * Time does not advance when on a spob.
    * Journey with a single jump takes approx 3e7, so about 3 periods. */
   t = ntime_convertSeconds( tme ) / NT_PERIOD_SECONDS;

   /* Get position in stack. */
   k = com - commodity_stack;

   /* Find what commodity that is. */
   for (i=0; i<array_size(econ_comm); i++)
      if (econ_comm[i] == k)
         break;

   /* Check if found. */
   if (i >= array_size(econ_comm)) {
      WARN(_("Price for commodity '%s' not known."), com->name);
      return 0;
   }

   /* and get the index on this spob */
   for (i=0; i<array_size(p->commodities); i++) {
      if ((strcmp(p->commodities[i]->name, com->name)==0))
         break;
   }
   if (i >= array_size(p->commodities)) {
      WARN(_("Price for commodity '%s' not known on this spob."), com->name);
      return 0;
   }
   commPrice = &p->commodityPrice[i];
   /* Calculate price. */
   /* price  = (double) com->price; */
   /* price *= sys->prices[i]; */
   price = (commPrice->price + commPrice->sysVariation
         * sin(2. * M_PI * t / commPrice->sysPeriod)
         + commPrice->spobVariation
         * sin(2. * M_PI * t / commPrice->spobPeriod));
   return (credits_t) (price+0.5);/* +0.5 to round */
}

/**
 * @brief Gets the average price of a good on a spob in a system, using a rolling average over the times the player has landed here.
 *
 *    @param com Commodity to get price of.
 *    @param p Spob to get price of commodity.
 *    @param[out] mean Sample mean, rounded to nearest credit.
 *    @param[out] std Sample standard deviation (via uncorrected population formula).
 *    @return The average price of the commodity.
 */
int economy_getAverageSpobPrice( const Commodity *com, const Spob *p, credits_t *mean, double *std )
{
   int i,k;
   CommodityPrice *commPrice;

   if (com->price_ref != NULL) {
      const Commodity *ref = commodity_get( com->price_ref );
      if (ref==NULL)
         return -1;
      int ret = economy_getAverageSpobPrice( ref, p, mean, std );
      *mean = (credits_t) ((double)*mean*com->price_mod+0.5);
      *std = (*std*com->price_mod);
      return ((double)ret*com->price_mod+0.5);
   }

   /* Constant price. */
   if (commodity_isFlag(com, COMMODITY_FLAG_PRICE_CONSTANT)) {
      *mean = com->price;
      *std  = 0.;
      return com->price;
   }

   /* Get position in stack */
   k = com - commodity_stack;

   /* Find what commodity this is */
   for (i=0; i<array_size(econ_comm); i++)
      if (econ_comm[i] == k)
         break;

   /* Check if found */
   if (i >= array_size(econ_comm)) {
      WARN(_("Average price for commodity '%s' not known."), com->name);
      *mean = 0;
      *std  = 0;
      return -1;
   }

   /* and get the index on this spob */
   for (i=0; i<array_size(p->commodities); i++) {
      if ((strcmp(p->commodities[i]->name, com->name) == 0))
         break;
   }
   if (i >= array_size(p->commodities)) {
      WARN(_("Price for commodity '%s' not known on this spob."), com->name);
      *mean = 0;
      *std  = 0;
      return -1;
   }
   commPrice = &p->commodityPrice[i];
   if (commPrice->cnt > 0) {
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
 *    @param[out] mean Sample mean, rounded to nearest credit.
 *    @param[out] std Sample standard deviation (via uncorrected population formula).
 *    @return The average price of the commodity.
 */
int economy_getAveragePrice( const Commodity *com, credits_t *mean, double *std )
{
   int i, k;
   CommodityPrice *commPrice;
   double av = 0;
   double av2 = 0;
   int cnt = 0;

   if (com->price_ref != NULL) {
      const Commodity *ref = commodity_get( com->price_ref );
      if (ref==NULL)
         return -1;
      int ret = economy_getAveragePrice( ref, mean, std );
      *mean = (credits_t) ((double)*mean*com->price_mod+0.5);
      *std = (*std*com->price_mod);
      return ((double)ret*com->price_mod+0.5);
   }

   /* Constant price. */
   if (commodity_isFlag(com, COMMODITY_FLAG_PRICE_CONSTANT)) {
      *mean = com->price;
      *std  = 0.;
      return com->price;
   }

   /* Get position in stack */
   k = com - commodity_stack;

   /* Find what commodity this is */
   for (i=0; i<array_size(econ_comm); i++)
      if (econ_comm[i] == k)
         break;

   /* Check if found */
   if (i >= array_size(econ_comm)) {
      WARN(_("Average price for commodity '%s' not known."), com->name);
      *mean = 0;
      *std = 0;
      return 1;
   }
   for (i=0; i<array_size(systems_stack) ; i++) {
      StarSystem *sys = &systems_stack[i];
      for (int j=0; j<array_size(sys->spobs); j++) {
         Spob *p = sys->spobs[j];

         /* and get the index on this spob */
         for (k=0; k<array_size(p->commodities); k++) {
            if ((strcmp(p->commodities[k]->name, com->name) == 0 ))
               break;
         }
         if (k < array_size(p->commodityPrice)) {
            commPrice=&p->commodityPrice[k];
            if ( commPrice->cnt>0) {
               av  += commPrice->sum/commPrice->cnt;
               av2 += commPrice->sum*commPrice->sum/(commPrice->cnt*commPrice->cnt);
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
   Spob *spob;

   ddt = (double)(dt / NTIME_UNIT_LENGTH);

   /* Calculate production level. */
   p = 0.;
   for (i=0; i<sys->nspobs; i++) {
      spob = sys->spobs[i];
      if (spob_hasService(spob, SPOB_SERVICE_INHABITED)) {
         /*
          * Calculate production.
          */
         /* We base off the current production. */
         prodfactor  = spob->cur_prodfactor;
         /* Add a variability factor based on the Gaussian distribution. */
         prodfactor += ECON_PROD_VAR * RNG_2SIGMA() * ddt;
         /* Add a tendency to return to the spob's base production. */
         prodfactor -= ECON_PROD_VAR *
               (spob->cur_prodfactor - prodfactor)*ddt;
         /* Save for next iteration. */
         spob->cur_prodfactor = prodfactor;
         /* We base off the sqrt of the population otherwise it changes too fast. */
         p += prodfactor * sqrt(spob->population);
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
   M = cs_spalloc( array_size(systems_stack), array_size(systems_stack), 1, 1, 1 );
   if (M == NULL)
      ERR(_("Unable to create CSparse Matrix."));

   /* Fill the matrix. */
   for (i=0; i < array_size(systems_stack); i++) {
      sys   = &systems_stack[i];
      Rsum = 0.;

      /* Set some values. */
      for (j=0; j < array_size(sys->jumps); j++) {

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
   /* Must not be initialized. */
   if (econ_initialized)
      return 0;

   /* Allocate price space. */
   for (int i=0; i<array_size(systems_stack); i++) {
      free(systems_stack[i].prices);
      systems_stack[i].prices = calloc(array_size(econ_comm), sizeof(double));
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
#if 0
   int i, j;
   double *X;
   double scale, offset;
   /*double min, max;*/

   /* Economy must be initialized. */
   if (econ_initialized == 0)
      return 0;

   /* Create the vector to solve the system. */
   X = malloc(sizeof(double)*array_size(systems_stack));
   if (X == NULL) {
      WARN(_("Out of Memory"));
      return -1;
   }

   /* Calculate the results for each price set. */
   for (j=0; j<array_size(econ_comm); j++) {

      /* First we must load the vector with intensities. */
      for (i=0; i<array_size(systems_stack); i++)
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

      /*
       * Get the minimum and maximum to scale.
       */
      /*
      min = +HUGE_VALF;
      max = -HUGE_VALF;
      for (i=0; i<array_size(systems_stack); i++) {
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
      for (i=0; i<array_size(systems_stack); i++)
         systems_stack[i].prices[j] = X[i] * scale + offset;
   }

   /* Clean up. */
   free(X);

#endif
   econ_queued = 0;
   return 0;
}

/**
 * @brief Destroys the economy.
 */
void economy_destroy (void)
{
   /* Must be initialized. */
   if (!econ_initialized)
      return;

   /* Clean up the prices in the systems stack. */
   for (int i=0; i<array_size(systems_stack); i++) {
      free(systems_stack[i].prices);
      systems_stack[i].prices = NULL;
   }

   /* Destroy the economy matrix. */
   cs_spfree( econ_G );
   econ_G = NULL;

   /* Economy is now deinitialized. */
   econ_initialized = 0;
}

/**
 * @brief Used during startup to set price and variation of the economy, depending on spob information.
 *
 *    @param spob The spob to set price on.
 *    @param commodity The commodity to set the price of.
 *    @param commodityPrice Where to write the commodity price to.
 *    @return 0 on success.
 */
static int economy_calcPrice( Spob *spob, Commodity *commodity, CommodityPrice *commodityPrice )
{
   CommodityModifier *cm;
   double base, scale, factor;
   const char *factionname;

   /* Ignore spobs with no commodity stuff. */
   if (!spob_hasService( spob, SPOB_SERVICE_COMMODITY ))
      return 0;

   /* Check the faction is not NULL.*/
   if (spob->presence.faction == -1) {
      WARN(_("Spob '%s' appears to have commodity '%s' defined, but no faction."), spob->name, commodity->name);
      return 1;
   }

   /* Reset price to the base commodity price. */
   commodityPrice->price = commodity->price;

   /* Get the cost modifier suitable for spob type/class. */
   cm = commodity->spob_modifier;
   scale = 1.;
   while (cm != NULL) {
      if ((strcmp( spob->class, cm->name ) == 0)) {
         scale = cm->value;
         break;
      }
      cm = cm->next;
   }
   commodityPrice->price *= scale;
   commodityPrice->spobVariation = 0.5;
   commodityPrice->sysVariation = 0.;
   /*commodityPrice->sum = 0.;
   commodityPrice->sum2 = 0.;
   commodityPrice->cnt = 0;
   commodityPrice->updateTime = 0;*/
   /* Use filename to specify a variation period. */
   base = 100;
   commodity->period = 32 * (spob->gfx_spaceName[strlen(SPOB_GFX_SPACE_PATH)] % 32) + spob->gfx_spaceName[strlen(SPOB_GFX_SPACE_PATH) + 1] % 32;
   commodityPrice->spobPeriod = commodity->period + base;

   /* Use filename of exterior graphic to modify the variation period.
      No rhyme or reason, just gives some variability. */
   scale = 1 + (strlen(spob->gfx_exterior) - strlen(SPOB_GFX_EXTERIOR_PATH) - 19) / 100.;
   commodityPrice->spobPeriod *= scale;

   /* Use population to modify price and variability.  The tanh function scales from -1 (small population)
      to +1 (large population), done on a log scale.  Price is then modified by this factor, scaled by a
      value defined in the xml, as is variation.  So for some commodities, prices increase with population,
      while for others, prices decrease. */
   factor = -1;
   if ( spob->population > 0 )
      factor = tanh( ( log((double)spob->population) - log(1e8) ) / 2 );
   base = commodity->population_modifier;
   commodityPrice->price *= 1 + factor * base;
   commodityPrice->spobVariation *= 0.5 - factor * 0.25;
   commodityPrice->spobPeriod *= 1 + factor * 0.5;

   /* Modify price based on faction (as defined in the xml).
      Some factions place a higher value on certain goods.
      Some factions are more stable than others.*/
   scale = 1.;
   cm = commodity->spob_modifier;

   factionname = faction_name(spob->presence.faction);
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
   commodityPrice->price *= (1 - spob->presence.range/30.);
   commodityPrice->spobPeriod *= 1 / (1 - spob->presence.range/30.);

   /* Make sure the price is always positive and non-zero */
   commodityPrice->price = MAX( commodityPrice->price, 1 );

   return 0;
}

/**
 * @brief Modifies commodity price based on system characteristics.
 *
 *    @param sys System.
 */
static void economy_modifySystemCommodityPrice( StarSystem *sys )
{
   int k;
   CommodityPrice *avprice;

   avprice = array_create( CommodityPrice );
   for (int i=0; i<array_size(sys->spobs); i++) {
      Spob *spob = sys->spobs[i];
      for (int j=0; j<array_size(spob->commodityPrice); j++) {
        /* Largest is approx 35000.  Increased radius will increase price since further to travel,
           and also increase stability, since longer for prices to fluctuate, but by a larger amount when they do.*/
         spob->commodityPrice[j].price *= 1 + sys->radius/200e3;
         spob->commodityPrice[j].spobPeriod *= 1 / (1 - sys->radius/200e3);
         spob->commodityPrice[j].spobVariation *= 1 / (1 - sys->radius/300e3);

         /* Increase price with volatility, which goes up to about 600.
            And with interference, since systems are harder to find, which goes up to about 1000.*/
         spob->commodityPrice[j].price *= 1 + sys->nebu_volatility/600.;
         spob->commodityPrice[j].price *= 1 + sys->interference/10e3;

         /* Use number of jumps to determine sytsem time period.  More jumps means more options for trade
            so shorter period.  Between 1 to 6 jumps.  Make the base time 1000.*/
         spob->commodityPrice[j].sysPeriod = 2000. / (array_size(sys->jumps) + 1);

         for (k=0; k<array_size(avprice); k++) {
            if ( ( strcmp( spob->commodities[j]->name, avprice[k].name ) == 0 ) ) {
               avprice[k].updateTime++;
               avprice[k].price+=spob->commodityPrice[j].price;
               avprice[k].spobPeriod+=spob->commodityPrice[j].spobPeriod;
               avprice[k].sysPeriod+=spob->commodityPrice[j].sysPeriod;
               avprice[k].spobVariation+=spob->commodityPrice[j].spobVariation;
               avprice[k].sysVariation+=spob->commodityPrice[j].sysVariation;
               break;
            }
         }
         if ( k == array_size(avprice) ) {/* first visit of this commodity for this system */
            (void)array_grow( &avprice );
            avprice[k].name=spob->commodities[j]->name;
            avprice[k].updateTime=1;
            avprice[k].price=spob->commodityPrice[j].price;
            avprice[k].spobPeriod=spob->commodityPrice[j].spobPeriod;
            avprice[k].sysPeriod=spob->commodityPrice[j].sysPeriod;
            avprice[k].spobVariation=spob->commodityPrice[j].spobVariation;
            avprice[k].sysVariation=spob->commodityPrice[j].sysVariation;
         }
      }
   }
   /* Do some inter-spob averaging */
   for ( k=0; k<array_size(avprice); k++ ) {
      avprice[k].price/=avprice[k].updateTime;
      avprice[k].spobPeriod/=avprice[k].updateTime;
      avprice[k].sysPeriod/=avprice[k].updateTime;
      avprice[k].spobVariation/=avprice[k].updateTime;
      avprice[k].sysVariation/=avprice[k].updateTime;
   }
   /* And now apply the averaging */
   for (int i=0; i<array_size(sys->spobs); i++) {
      Spob *spob = sys->spobs[i];
      for (int j=0; j<array_size(spob->commodities); j++) {
         for ( k=0; k<array_size(avprice); k++ ) {
            if ( ( strcmp( spob->commodities[j]->name, avprice[k].name ) == 0 ) ) {
               spob->commodityPrice[j].price*=0.25;
               spob->commodityPrice[j].price+=0.75*avprice[k].price;
               spob->commodityPrice[j].sysVariation=0.2*avprice[k].spobVariation;
            }
         }
      }
   }
   array_shrink( &avprice );
   array_free( sys->averagePrice );
   sys->averagePrice = avprice;
}

/**
 * @brief Calculates smoothing of commodity price based on neighbouring systems
 *
 *    @param sys System.
 */
static void economy_smoothCommodityPrice(StarSystem *sys)
{
   StarSystem *neighbour;
   CommodityPrice *avprice=sys->averagePrice;
   double price;
   int n,i,j,k;
   /*Now modify based on neighbouring systems */
   /*First, calculate mean price of neighbouring systems */

   for ( j =0; j<array_size(avprice); j++ ) {/* for each commodity in this system */
      price=0.;
      n=0;
      for ( i=0; i<array_size(sys->jumps); i++ ) {/* for each neighbouring system */
         neighbour=sys->jumps[i].target;
         for ( k=0; k<array_size(neighbour->averagePrice); k++ ) {
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
   CommodityPrice *avprice=sys->averagePrice;
   Spob *spob;
   int i,j,k;
   for ( j=0; j<array_size(avprice); j++ ) {
      /*Use mean price to adjust current price */
      avprice[j].price=0.5*(avprice[j].price + avprice[j].sum);
   }
   /*and finally modify spobs based on the means */
   for ( i=0; i<array_size(sys->spobs); i++ ) {
      spob=sys->spobs[i];
      for ( j=0; j<array_size(spob->commodities); j++ ) {
         for ( k=0; k<array_size(avprice); k++ ) {
            if ( ( strcmp(avprice[k].name, spob->commodities[j]->name) == 0 ) ) {
               spob->commodityPrice[j].price = (
                     0.25*spob->commodityPrice[j].price
                        + 0.75*avprice[k].price );
               spob->commodityPrice[j].spobVariation = (
                     0.1 * (0.5*avprice[k].spobVariation
                           + 0.5*spob->commodityPrice[j].spobVariation) );
               spob->commodityPrice[j].spobVariation *= spob->commodityPrice[j].price;
               spob->commodityPrice[j].sysVariation *= spob->commodityPrice[j].price;
               break;
            }
         }
      }
   }
   array_free( sys->averagePrice );
   sys->averagePrice=NULL;
}

/**
 * @brief Initialises commodity prices for the sinusoidal economy model.
 *
 */
void economy_initialiseCommodityPrices(void)
{
   /* First use spob attributes to set prices and variability */
   for (int k=0; k<array_size(systems_stack); k++) {
      StarSystem *sys = &systems_stack[k];
      for (int j=0; j<array_size(sys->spobs); j++) {
         Spob *spob = sys->spobs[j];
         /* Set up the commodity prices on the system, based on its attributes. */
         for (int i=0; i<array_size(spob->commodities); i++) {
            if (economy_calcPrice(spob, spob->commodities[i], &spob->commodityPrice[i]))
               return;
         }
      }
   }

   /* Modify prices and availability based on system attributes, and do some inter-spob averaging to smooth prices */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      economy_modifySystemCommodityPrice(sys);
   }

   /* Compute average prices for all systems */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      economy_smoothCommodityPrice(sys);
   }

   /* Smooth prices based on neighbouring systems */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      economy_calcUpdatedCommodityPrice(sys);
   }
   /* And now free temporary commodity information */
   for (int i=0 ; i<array_size(commodity_stack); i++) {
      CommodityModifier *this, *next;
      Commodity *com = &commodity_stack[i];
      next = com->spob_modifier;
      com->spob_modifier = NULL;
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
 * Calculates commodity prices for a single spob (e.g. as added by the unidiff), and does some smoothing over the system, but not neighbours.
 */
void economy_initialiseSingleSystem( StarSystem *sys, Spob *spob )
{
   for (int i=0; i<array_size(spob->commodities); i++)
      economy_calcPrice( spob, spob->commodities[i], &spob->commodityPrice[i] );
   economy_modifySystemCommodityPrice(sys);
}

void economy_averageSeenPrices( const Spob *p )
{
   ntime_t t = ntime_get();
   for (int i=0; i < array_size(p->commodities); i++) {
      Commodity *c = p->commodities[i];
      CommodityPrice *cp = &p->commodityPrice[i];
      if (cp->updateTime < t) { /* has not yet been updated at present time. */
         credits_t price;
         cp->updateTime = t;
         /* Calculate values for mean and std */
         cp->cnt++;
         price = economy_getPrice(c, NULL, p);
         cp->sum += price;
         cp->sum2 += price*price;
      }
   }
}

void economy_averageSeenPricesAtTime( const Spob *p, const ntime_t tupdate )
{
   ntime_t t = ntime_get();
   for (int i=0; i < array_size(p->commodities); i++) {
      Commodity *c = p->commodities[i];
      CommodityPrice *cp = &p->commodityPrice[i];
      if (cp->updateTime < t) { /* has not yet been updated at present time. */
         credits_t price;
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
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      for (int j=0; j<array_size(sys->spobs); j++) {
         Spob *p = sys->spobs[j];
         for (int k=0; k<array_size(p->commodityPrice); k++) {
            CommodityPrice *cp = &p->commodityPrice[k];
            cp->cnt = 0;
            cp->sum = 0;
            cp->sum2 = 0;
            cp->updateTime = 0;
         }
      }
   }
   for (int i=0; i<array_size(commodity_stack); i++ )
      commodity_stack[i].lastPurchasePrice=0;
}

/**
 * @brief Clears all economy knowledge of a given spob.  Used by the unidiff system.
 */
void economy_clearSingleSpob(Spob *p)
{
   for (int k=0; k<array_size(p->commodityPrice); k++) {
      CommodityPrice *cp = &p->commodityPrice[k];
      cp->cnt = 0;
      cp->sum = 0;
      cp->sum2 = 0;
      cp->updateTime = 0;
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
   xmlNodePtr node = parent->xmlChildrenNode;

   economy_clearKnown();

   do {
      if (xml_isNode(node,"economy")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            char *str;
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "system")) {
               /* Ignore "name" attribute. */
               xmlNodePtr nodeSpob = cur->xmlChildrenNode;
               do {
                  xml_onlyNodes(nodeSpob);
                  if (xml_isNode(nodeSpob, "spob")) {
                     xmlr_attr_strd(nodeSpob,"name",str);
                     Spob *spob = spob_get(str);
                     if (spob==NULL)
                        WARN(_("Spob '%s' has saved economy data but doesn't exist!"), str);
                     free(str);
                     if (spob==NULL)
                        continue;
                     xmlNodePtr nodeCommodity = nodeSpob->xmlChildrenNode;
                     do {
                        xml_onlyNodes(nodeCommodity);
                        if (xml_isNode(nodeCommodity, "commodity")) {
                           xmlr_attr_strd(nodeCommodity,"name",str);
                           CommodityPrice *cp = NULL;
                           for (int i=0; i<array_size(spob->commodities); i++) {
                              if ( (strcmp(str,spob->commodities[i]->name) == 0) ) {
                                 cp = &spob->commodityPrice[i];
                                 break;
                              }
                           }
                           free(str);
                           if (cp != NULL) {
                              xmlr_attr_float(nodeCommodity,"sum",cp->sum);
                              xmlr_attr_float(nodeCommodity,"sum2",cp->sum2);
                              xmlr_attr_int(nodeCommodity,"cnt",cp->cnt);
                              xmlr_attr_long(nodeCommodity,"time",cp->updateTime);
                           }
                        }
                     } while (xml_nextNode(nodeCommodity));
                  }
               } while (xml_nextNode(nodeSpob));
            } else if (xml_isNode(cur, "lastPurchase")) {
               xmlr_attr_strd(cur, "name", str);
               if (str) {
                  Commodity *c = commodity_get(str);
                  c->lastPurchasePrice = xml_getLong(cur);
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
   /* Save what the player has seen of the economy at each spob */
   xmlw_startElem(writer,"economy");
   for (int i=0; i<array_size(commodity_stack); i++) {
      if ( commodity_stack[i].lastPurchasePrice > 0 ) {
         xmlw_startElem(writer, "lastPurchase");
         xmlw_attr(writer,"name","%s",commodity_stack[i].name);
         xmlw_str(writer,"%"PRIu64,commodity_stack[i].lastPurchasePrice);
         xmlw_endElem(writer);
      }
   }
   for (int i=0; i<array_size(systems_stack); i++) {
      int doneSys=0;
      StarSystem *sys = &systems_stack[i];
      for (int j=0; j<array_size(sys->spobs); j++) {
         Spob *p = sys->spobs[j];
         int doneSpob=0;
         for (int k=0; k<array_size(p->commodities); k++) {
            CommodityPrice *cp = &p->commodityPrice[k];
            if (cp->cnt > 0) {/* Player has seen this commodity at this location */
               if (doneSys==0) {
                  doneSys = 1;
                  xmlw_startElem(writer, "system");
                  xmlw_attr(writer,"name","%s",sys->name);
               }
               if (doneSpob==0) {
                  doneSpob = 1;
                  xmlw_startElem(writer, "spob");
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
         if (doneSpob==1)
            xmlw_endElem(writer); /* spob */
      }
      if (doneSys==1)
         xmlw_endElem(writer); /* system */
   }
   xmlw_endElem(writer); /* economy */
   return 0;
}
