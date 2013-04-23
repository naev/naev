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

float **xml_econvals; /* an array of array of all systems's original weights and good values */

static int econ_initialized   = 0; /**< Is economy system initialized? */
static int *econ_comm         = NULL; /**< Commodities to calculate. */
int econ_nprices              = 0; /**< Number of prices to calculate. */


float *solutions; /* the solution matrix: formulas for system prices in terms of given prices */

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
   if (name==NULL) return NULL;
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
 * @brief update solution matrix, which is a table of variables that tell the price of every 
 *    system in terms of the other systems. Expensive
 *    @return 0 on success.
 */
int econ_refreshsolutions(void)
{
   int eq, i, j, v, jmp;
   float denom, val, factor;
   StarSystem *sys;
   int sysw = systems_nstack*2;   /* the width of the system of equations */
   float *tmp = malloc(sizeof(float)*sysw);

   /* make a system of equations where every system's prices (real good value) are equal to
      weighted value of own given value (price) and neighbor's real good values/prices,
      then solve it (using gaussian elimination), and put the results into the solution matrix */

      /* initialize the system of equations */
   float *eqsystem = calloc(sizeof(float), systems_nstack*sysw);

      /* setup the system of equations */
   for (eq=0; eq<systems_nstack; eq++){
      sys = systems_stack+eq;
      denom=sys->weight;
      for (jmp=0; jmp<sys->njumps; jmp++){   /* get number of trading neighbors */
         if (jp_isFlag( sys->jumps+jmp, JP_EXITONLY) || jp_isFlag(sys->jumps+jmp, JP_HIDDEN))
            continue;
         for (j=0; j<sys->jumps[jmp].target->njumps; j++)
            if (sys == sys->jumps[jmp].target->jumps[j].target){
               if (!jp_isFlag( sys->jumps[jmp].target->jumps+j, JP_EXITONLY ) && !jp_isFlag(sys->jumps+jmp, JP_HIDDEN))
                  denom+=sys->jumps[jmp].trade_weight;
               break;
            }
      }
      val = -1.0/((denom!=0.0)?denom:1.0);
      for (jmp=0; jmp<sys->njumps; jmp++){   /* get number of trading neighbors */
         if (jp_isFlag( sys->jumps+jmp, JP_EXITONLY) || jp_isFlag(sys->jumps+jmp, JP_HIDDEN))
            continue;
         for (j=0; j<sys->jumps[jmp].target->njumps; j++)
            if (sys == sys->jumps[jmp].target->jumps[j].target){
               if (!jp_isFlag( sys->jumps[jmp].target->jumps+j, JP_EXITONLY ) && !jp_isFlag(sys->jumps+jmp, JP_HIDDEN))
                  eqsystem[eq*sysw+sys->jumps[jmp].target->id] = val * sys->jumps[jmp].trade_weight;
               break;
            }
      }
      eqsystem[eq*sysw + eq] = 1.0;
      if (sys->given_prices!=NULL)  /* if the system has a preferred value */
         eqsystem[eq*sysw + systems_nstack + eq] = sys->weight/denom;
   }

      /* convert the system of equations into triangle form */
   for (i=0; i<systems_nstack; i++){

      /* factor out var i from all equations below i */
      for (j=i+1; j<systems_nstack; j++){
         if (eqsystem[j*sysw + i]==0) /* if already at 0 */
            continue;
         factor = -eqsystem[i*sysw+i]/eqsystem[j*sysw+i];
         for (v=i; v<sysw; v++){
            eqsystem[j*sysw+v]*=factor;
            eqsystem[j*sysw+v]+=eqsystem[i*sysw+v];
         }
            /* manipulate var j of equation j to 1 */
         if (eqsystem[j*sysw + j]!=0){
            factor = eqsystem[j*sysw + j];
            for (v=i+1; v<sysw; v++)
               eqsystem[j*sysw + v]/=factor;
         }
         else eqsystem[j*sysw + j]=1.0;
      }

   }

      /* get the solutions (the real values in terms of the given values) */
   for (i=systems_nstack-1; i>=0; i-- ){

      /* factor val i in eq i to 1 */
      factor = eqsystem[i*sysw + i];
      for (v=i; v<sysw; v++)
         eqsystem[i*sysw + v]/=factor;

      /* substitute in known values */
      for (j=i+1; j<systems_nstack; j++){
         for (v=0; v<systems_nstack; v++){
            eqsystem[i*sysw+systems_nstack+v]-=eqsystem[i*sysw+j]*solutions[j*systems_nstack+v];
         }
      }

      memcpy(solutions+i*systems_nstack, eqsystem+i*sysw+systems_nstack, systems_nstack*sizeof(float));
   }


   free(tmp);
   free(eqsystem);

   return 0;
}


/**
 * @brief update prices using the solution matrix. not expensive
 */
void econ_updateprices(void)
{
   int i, s, g;
   StarSystem *sys;

      /* get the real values from the given values and the solution matrix*/
   for (i=0; i<systems_nstack; i++){
      sys = systems_stack+i;
      for (g=0; g<econ_nprices; g++){
         sys->real_prices[g]=0.0;
         for (s=0; s<systems_nstack; s++){
            if (systems_stack[s].given_prices==NULL)
               continue;
            sys->real_prices[g] 
               += systems_stack[s].given_prices[g] * solutions[i*systems_nstack+s];
         }
      }
   }

}

/**
 * @brief revert all economic values to their original, XML designated values
 */
void econ_revert(void)
{
   int i, g;
   StarSystem *sys;\

      /* revert to the old values */
   for (i=0;i<systems_nstack; i++){
      sys = systems_stack+i;
      if (xml_econvals[i]==NULL){
         sys->weight = 0;
         free(sys->given_prices);
         sys->given_prices = NULL;
         continue;
      }
      sys->weight = xml_econvals[i][0];
      for (g=0; g<econ_nprices; g++)
         sys->given_prices[g] = xml_econvals[i][1 + g];
   }
}

/**
 * @brief initializes all economic variables. Called immediately after starting galaxy
 */
void econ_init(void)
{
   int s, g, jmp;
   StarSystem *sys;

   if (econ_initialized){ WARN("economy already initialized!\n"); return;}

   for (s=0; s<systems_nstack; s++){
      sys=systems_stack+s;
      sys->real_prices = (float *) calloc(sizeof(float), econ_nprices);
      for (jmp=0; jmp<sys->njumps; jmp++)
         sys->jumps[jmp].trade_weight=1.0;
   }
   solutions = malloc(sizeof(float)*systems_nstack*systems_nstack);

      /* save original values */
   int w = 1 + econ_nprices;  /* width of xml_econvals entry; system weight and preferred prices */
   xml_econvals = (float **) calloc(sizeof(float *), systems_nstack);
   for (s=0; s<systems_nstack; s++){
      sys = systems_stack+s;
      if (sys->given_prices==NULL)
         continue;
      xml_econvals[s] = (float *) malloc(sizeof(float)*w);
      xml_econvals[s][0] = sys->weight;
      for (g=0; g<econ_nprices; g++)
         xml_econvals[s][1+g] = sys->given_prices[g];
   }

   econ_initialized = 1;
}

/**
 * @brief frees all economy related variables. Only use when exiting naev
 */
void econ_destroy(void)
{
   int s;
   StarSystem *sys;
   printf("destroying econ!\n");
   if (econ_initialized!=1){ WARN("economy not inited!\n"); return; }
   for (s=0; s<systems_nstack; s++){
      sys=systems_stack+s;
      free(sys->given_prices); 
      free(sys->real_prices);
      free(xml_econvals[s]);
      sys->given_prices=NULL;
      sys->real_prices=NULL;
   }
   free(solutions);
   free(xml_econvals);
   solutions=NULL;
   econ_initialized=0;
}





