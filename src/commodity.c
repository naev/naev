/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file commodity.c
 *
 * @brief Handles commidities.
 */
/** @cond */
#include <stdio.h>
#include <stdint.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "commodity.h"

#include "conf.h"
#include "array.h"
#include "economy.h"
#include "gatherable.h"
#include "hook.h"
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
#include "threadpool.h"

#define XML_COMMODITY_ID      "commodity" /**< XML document identifier */
#define CRED_TEXT_MAX         (ECON_CRED_STRLEN-4) /* Maximum length of just credits2str text, no markup */

/* commodity stack */
Commodity* commodity_stack = NULL; /**< Contains all the commodities. */
static Commodity** commodity_temp = NULL; /**< Contains all the temporary commodities. */

/* @TODO remove externs. */
extern int *econ_comm;

/**
 * @brief For threaded loading of commodities.
 */
typedef struct CommodityThreadData_ {
   char *filename;
   Commodity com;
   int ret;
} CommodityThreadData;

/*
 * Prototypes.
 */
/* Commodity. */
static int commodity_parseThread( void *ptr );
static void commodity_freeOne( Commodity* com );
static int commodity_parse( Commodity *temp, const char *filename );

/**
 * @brief Converts credits to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a size of ECON_CRED_STRLEN.
 *    @param credits Credits to display, negative value to display full string.
 *    @param decimals Decimals to use.
 */
void credits2str( char *str, credits_t credits, int decimals )
{
   if (decimals < 0) {
      /* TODO support , separator like fmt.credits(). */
      snprintf( str, CRED_TEXT_MAX, _("%.*f ¤"), 0, (double)credits );
   }
   else if (credits >= 1000000000000000000LL)
      snprintf( str, CRED_TEXT_MAX, _("%.*f E¤"), decimals, (double)credits / 1e18 );
   else if (credits >= 1000000000000000LL)
      snprintf( str, CRED_TEXT_MAX, _("%.*f P¤"), decimals, (double)credits / 1e15 );
   else if (credits >= 1000000000000LL)
      snprintf( str, CRED_TEXT_MAX, _("%.*f T¤"), decimals, (double)credits / 1e12 );
   else if (credits >= 1000000000L)
      snprintf( str, CRED_TEXT_MAX, _("%.*f G¤"), decimals, (double)credits / 1e9 );
   else if (credits >= 1000000)
      snprintf( str, CRED_TEXT_MAX, _("%.*f M¤"), decimals, (double)credits / 1e6 );
   else if (credits >= 1000)
      snprintf( str, CRED_TEXT_MAX, _("%.*f k¤"), decimals, (double)credits / 1e3 );
   else
      snprintf (str, CRED_TEXT_MAX, _("%.*f ¤"), decimals, (double)credits );
}

/**
 * @brief Given a price and on-hand credits, outputs a colourized string.
 *
 *    @param[out] str Output is stored here, must have at least a size of ECON_CRED_STRLEN.
 *    @param price Price to display.
 *    @param credits Credits available.
 *    @param decimals Decimals to use.
 */
void price2str(char *str, credits_t price, credits_t credits, int decimals )
{
   char buf[ CRED_TEXT_MAX ];

   if (price <= credits) {
      credits2str( str, price, decimals );
      return;
   }

   credits2str( buf, price, decimals );
   snprintf( str, ECON_CRED_STRLEN, "#r%s#0", (char*)buf );
}

/**
 * @brief Converts tonnes to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a size of ECON_MASS_STRLEN.
 *    @param tonnes Number of tonnes to display.
 */
void tonnes2str( char *str, int tonnes )
{
   snprintf( str, ECON_MASS_STRLEN, n_( "%d tonne", "%d tonnes", tonnes ), tonnes );
}

/**
 * @brief Gets all the commodities.
 */
Commodity* commodity_getAll (void)
{
   return commodity_stack;
}

/**
 * @brief Gets a commodity by name.
 *
 *    @param name Name to match.
 *    @return Commodity matching name.
 */
Commodity* commodity_get( const char* name )
{
   Commodity *c = commodity_getW( name );
   if (c!=NULL)
      return c;
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
   for (int i=0; i<array_size(commodity_stack); i++)
      if (strcmp(commodity_stack[i].name, name) == 0)
         return &commodity_stack[i];
   for (int i=0; i<array_size(commodity_temp); i++)
      if (strcmp(commodity_temp[i]->name, name) == 0)
         return commodity_temp[i];
   return NULL;
}

/**
 * @brief Return the number of commodities globally.
 *
 *    @return Number of commodities globally.
 */
int commodity_getN (void)
{
   return array_size(econ_comm);
}

/**
 * @brief Gets a commodity by index.
 *
 *    @param indx Index of the commodity.
 *    @return Commodity at that index or NULL.
 */
Commodity* commodity_getByIndex( const int indx )
{
   if (indx < 0 || indx >= array_size(econ_comm)) {
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
   free(com->name);
   free(com->description);
   free(com->price_ref);
   gl_freeTexture(com->gfx_store);
   gl_freeTexture(com->gfx_space);
   next = com->spob_modifier;
   com->spob_modifier = NULL;
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
   array_free(com->illegalto);
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
 * @brief Return an array (array.h) of standard commodities. Free with array_free. (Don't free contents.)
 */
Commodity **standard_commodities (void)
{
   int n = array_size(commodity_stack);
   Commodity **com = array_create_size( Commodity*, n );
   for (int i=0; i<n; i++) {
      Commodity *c = &commodity_stack[i];
      if (commodity_isFlag(c,COMMODITY_FLAG_STANDARD))
         array_push_back( &com, c );
   }
   return com;
}

/**
 * @brief Loads a commodity.
 *
 *    @param temp Commodity to load data into.
 *    @param filename File to parse.
 *    @return Commodity loaded from parent.
 */
static int commodity_parse( Commodity *temp, const char *filename )
{
   xmlNodePtr node, parent;
   xmlDocPtr doc;

   doc = xml_parsePhysFS( filename );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* Commodities node */
   if (strcmp((char*)parent->name,XML_COMMODITY_ID)) {
      ERR(_("Malformed %s file: missing root element '%s'"), filename, XML_COMMODITY_ID);
      return -1;
   }

   /* Clear memory and set defaults. */
   memset( temp, 0, sizeof(Commodity) );
   temp->period   = 200;
   temp->price_mod = 1.;

   /* Parse body. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      xmlr_strd(node, "name", temp->name);
      xmlr_strd(node, "description", temp->description);
      xmlr_int(node, "price", temp->price);
      xmlr_float(node, "price_mod", temp->price_mod);
      xmlr_strd(node, "price_ref", temp->price_ref);

      if (xml_isNode(node,"gfx_space")) {
         temp->gfx_space = xml_parseTexture( node,
               COMMODITY_GFX_PATH"space/%s", 1, 1, OPENGL_TEX_MIPMAPS );
         continue;
      }
      if (xml_isNode(node,"gfx_store")) {
         temp->gfx_store = xml_parseTexture( node,
               COMMODITY_GFX_PATH"%s", 1, 1, OPENGL_TEX_MIPMAPS );
         continue;
      }
      if (xml_isNode(node, "standard")) {
         commodity_setFlag( temp, COMMODITY_FLAG_STANDARD );
         continue;
      }
      if (xml_isNode(node, "always_can_sell")) {
         commodity_setFlag( temp, COMMODITY_FLAG_ALWAYS_CAN_SELL );
         continue;
      }
      if (xml_isNode(node, "price_constant")) {
         commodity_setFlag( temp, COMMODITY_FLAG_PRICE_CONSTANT );
         continue;
      }
      if (xml_isNode(node, "illegalto")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         temp->illegalto = array_create( int );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "faction")) {
               int f = faction_get( xml_get(cur) );
               array_push_back( &temp->illegalto, f );
            }
         } while (xml_nextNode(node));
         continue;
      }
      xmlr_float(node, "population_modifier", temp->population_modifier);
      xmlr_float(node, "period", temp->period);
      if (xml_isNode(node, "spob_modifier")) {
         CommodityModifier *newdict = malloc(sizeof(CommodityModifier));
         newdict->next = temp->spob_modifier;
         xmlr_attr_strd(node, "type", newdict->name);
         newdict->value = xml_getFloat(node);
         temp->spob_modifier = newdict;
         continue;
      }
      if (xml_isNode(node, "faction_modifier")) {
         CommodityModifier *newdict = malloc(sizeof(CommodityModifier));
         newdict->next = temp->faction_modifier;
         xmlr_attr_strd(node, "type", newdict->name);
         newdict->value = xml_getFloat(node);
         temp->faction_modifier = newdict;
         continue;
      }

      WARN(_("Commodity '%s' has unknown node '%s'"), temp->name, node->name);
   } while (xml_nextNode(node));

   if (temp->name == NULL)
      WARN( _("Commodity from %s has invalid or no name"), COMMODITY_DATA_PATH);

   if ((temp->price > 0) || (temp->price_ref != NULL)) {
      if (temp->gfx_store == NULL) {
         WARN(_("No <gfx_store> node found, using default texture for commodity \"%s\""), temp->name);
         temp->gfx_store = gl_newImage( COMMODITY_GFX_PATH"_default.webp", 0 );
      }
   }
   if (temp->gfx_space == NULL)
      temp->gfx_space = gl_newImage( COMMODITY_GFX_PATH"space/_default.webp", 0 );

   if (temp->price_ref != NULL) {
      if (temp->price > 0.)
         WARN(_("Commodity '%s' is setting both 'price' and 'price_ref'."),temp->name);
   }

#if 0 /* shouldn't be needed atm */
#define MELEMENT(o,s)   if (o) WARN( _("Commodity '%s' missing '"s"' element"), temp->name)
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->high==0,"high");
   MELEMENT(temp->medium==0,"medium");
   MELEMENT(temp->low==0,"low");
#undef MELEMENT
#endif

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Checks to see if a commodity is illegal to a faction.
 *
 *    @param com Commodity to check.
 *    @param faction Faction to check to see if it is illegal to.
 *    @return 1 if it is illegal, 0 otherwise.
 */
int commodity_checkIllegal( const Commodity *com, int faction )
{
   for (int i=0; i<array_size(com->illegalto); i++) {
      if (com->illegalto[i] == faction)
         return 1;
   }
   return 0;
}

/**
 * @brief Checks to see if a commodity is temporary.
 *
 *    @brief Name of the commodity to check.
 *    @return 1 if temorary, 0 otherwise.
 */
int commodity_isTemp( const char* name )
{
   for (int i=0; i<array_size(commodity_temp); i++)
      if (strcmp(commodity_temp[i]->name, name) == 0)
         return 1;
   for (int i=0; i<array_size(commodity_stack); i++)
      if (strcmp(commodity_stack[i].name,name)==0)
         return 0;

   WARN(_("Commodity '%s' not found in stack"), name);
   return 0;
}

/**
 * @brief Creates a new temporary commodity.
 *
 *    @param name Name of the commodity to create.
 *    @param desc Description of the commodity to create.
 *    @return newly created commodity.
 */
Commodity* commodity_newTemp( const char* name, const char* desc )
{
   Commodity **c;
   if (commodity_temp == NULL)
      commodity_temp = array_create( Commodity* );

   c              = &array_grow(&commodity_temp);
   *c             = calloc( 1, sizeof(Commodity) );
   (*c)->istemp   = 1;
   (*c)->name     = strdup(name);
   (*c)->description = strdup(desc);
   return *c;
}

/**
 * @brief Makes a temporary commodity illegal to something.
 */
int commodity_tempIllegalto( Commodity *com, int faction )
{
   if (!com->istemp) {
      WARN(_("Trying to modify temporary commodity '%s'!"), com->name);
      return -1;
   }

   if (com->illegalto==NULL)
      com->illegalto = array_create( int );

   /* Don't add twice. */
   for (int i=0; i<array_size(com->illegalto); i++) {
      if (com->illegalto[i] == faction)
         return 0;
   }

   array_push_back( &com->illegalto, faction );

   return 0;
}

static int commodity_parseThread( void *ptr )
{
   CommodityThreadData *data = ptr;
   data->ret = commodity_parse( &data->com, data->filename );
   /* Render if necessary. */
   if (naev_shouldRenderLoadscreen()) {
      gl_contextSet();
      naev_renderLoadscreen();
      gl_contextUnset();
   }
   return data->ret;
}

/**
 * @brief Loads all the commodity data.
 *
 *    @return 0 on success.
 */
int commodity_load (void)
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   ThreadQueue *tq = vpool_create();
   CommodityThreadData *cdata = array_create( CommodityThreadData );
   char **commodities = ndata_listRecursive( COMMODITY_DATA_PATH );

   commodity_stack = array_create( Commodity );
   econ_comm = array_create( int );

   gatherable_load();

   /* Prepare files to run. */
   for (int i=0; i<array_size(commodities); i++) {
      if (ndata_matchExt( commodities[i], "xml" )) {
         CommodityThreadData *cd = &array_grow( &cdata );
         cd->filename = commodities[i];
      }
      else
         free( commodities[i] );
   }
   array_free( commodities );

   /* Enqueue the jobs after the data array is done. */
   for (int i=0; i<array_size(cdata); i++)
      vpool_enqueue( tq, commodity_parseThread, &cdata[i] );

   /* Wait until done processing. */
   SDL_GL_MakeCurrent( gl_screen.window, NULL );
   vpool_wait( tq );
   SDL_GL_MakeCurrent( gl_screen.window, gl_screen.context );

   /* Finally load. */
   for (int i=0; i<array_size(cdata); i++) {
      CommodityThreadData *cd = &cdata[i];
      if (!cd->ret) {
         array_push_back( &commodity_stack, cd->com );
      }
      free( cd->filename );
   }
   array_free(cdata);

   /* Load into commodity stack. */
   for (int i=0; i<array_size(commodity_stack); i++) {
      const Commodity *com = &commodity_stack[i];
      /* See if should get added to commodity list. */
      if (com->price > 0.)
         array_push_back( &econ_comm, i );
   }

#if DEBUGGING
   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d Commodity in %.3f s", "Loaded %d Commodities in %.3f s", array_size(commodity_stack) ), array_size(commodity_stack), time/1000. );
   }
   else
      DEBUG( n_( "Loaded %d Commodity", "Loaded %d Commodities", array_size(commodity_stack) ), array_size(commodity_stack) );
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Frees all the loaded commodities.
 */
void commodity_free (void)
{
   for (int i=0; i<array_size(commodity_stack); i++)
      commodity_freeOne( &commodity_stack[i] );
   array_free( commodity_stack );
   commodity_stack = NULL;

   for (int i=0; i<array_size(commodity_temp); i++) {
      commodity_freeOne( commodity_temp[i] );
      free( commodity_temp[i] );
   }
   array_free( commodity_temp );
   commodity_temp = NULL;

   /* More clean up. */
   array_free( econ_comm );
   econ_comm = NULL;

   gatherable_cleanup();
}
