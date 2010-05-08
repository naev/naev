/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file tech.c
 *
 * @brief Handles tech groups and metagroups for populating the planet outfitter,
 *        shipyard and commodity exchange.
 */


#include "tech.h"

#include "naev.h"

#include "nxml.h"
#include "log.h"
#include "ndata.h"
#include "outfit.h"
#include "ship.h"
#include "economy.h"
#include "array.h"


#define TECH_DATA           "dat/tech.xml"   /**< XML file containing techs. */
#define XML_TECH_ID         "Techs"          /**< Tech xml document tag. */
#define XML_TECH_TAG        "tech"           /**< Individual tech xml tag. */


/**
 * @brief Different tech types.
 */
typedef enum tech_item_type_e {
   TECH_TYPE_OUTFIT,       /**< Tech contains an outfit. */
   TECH_TYPE_SHIP,         /**< Tech contains a ship. */
   TECH_TYPE_COMMODITY,    /**< Tech contains a commodity. */
   /*TECH_TYPE_CONTRABAND,*/   /**< Tech contains contraband. */
   TECH_TYPE_GROUP,        /**< Tech contains another tech group. */
} tech_item_type_t;


/**
 * @brief Item contained in a tech group.
 */
typedef struct tech_item_s {
   tech_item_type_t type;  /**< Type of data. */
   union {
      Outfit *outfit;      /**< Outfit pointer. */
      Ship *ship;          /**< Ship pointer. */
      Commodity *comm;     /**< Commodity pointer. */
      int grp;             /**< Identifier of another tech group. */
   } u;                    /**< Data union. */
} tech_item_t;


/**
 * @brief Group of tech items, basic unit of the tech trees.
 */
struct tech_group_s {
   char *name;          /**< Name of the tech group. */
   tech_item_t *items;  /**< Items in the tech group. */
};


/*
 * Group list.
 */
static tech_group_t *tech_groups = NULL;


/*
 * Prototypes.
 */
static void tech_freeGroup( tech_group_t *grp );
static char* tech_getItemName( tech_item_t *item );
/* Loading. */
static tech_item_t *tech_itemGrow( tech_group_t *grp );
static int tech_parseNode( tech_group_t *tech, xmlNodePtr parent );
static int tech_parseNodeData( tech_group_t *tech, xmlNodePtr parent );
static int tech_addItemOutfit( tech_group_t *grp, const char* name );
static int tech_addItemShip( tech_group_t *grp, const char* name );
static int tech_addItemCommodity( tech_group_t *grp, const char* name );
static int tech_getID( const char *name );
static int tech_addItemGroup( tech_group_t *grp, const char* name );
/* Getting by tech. */
static Outfit** tech_addGroupOutfit( Outfit **o, tech_group_t *tech, int *n, int *m );
static Ship** tech_addGroupShip( Ship **s, tech_group_t *tech, int *n, int *m );


/**
 * @brief Loads the tech information.
 */
int tech_load (void)
{
   int i, ret, s;
   uint32_t bufsize;
   char *buf, *data;
   xmlNodePtr node, parent;
   xmlDocPtr doc;
   tech_group_t *tech;

   /* Load the data. */
   data = ndata_read( TECH_DATA, &bufsize );
   if (data == NULL)
      return -1;

   /* Load the document. */
   doc = xmlParseMemory( data, bufsize );
   if (doc == NULL) {
      WARN("'%s' is not a valid XML file.", TECH_DATA);
      return -1;
   }

   /* Load root element. */
   parent = doc->xmlChildrenNode;
   if (!xml_isNode(parent,XML_TECH_ID)) {
      WARN("Malformed "TECH_DATA" file: missing root element '"XML_TECH_ID"'");
      return -1;
   }

   /* Get first node. */
   node = parent->xmlChildrenNode;
   if (node == NULL) {
      WARN("Malformed "TECH_DATA" file: does not contain elements");
      return -1;
   }

   /* Create the array. */
   tech_groups = array_create( tech_group_t );

   /* First pass create the groups - needed to reference them later. */
   ret = 0;
   do {
      /* Must match tag. */
      if (!xml_isNode(node, XML_TECH_TAG))
         continue;
      if ((ret == 0) && xml_isNode(node, XML_TECH_TAG)) /* Write over failures. */
         tech = &array_grow( &tech_groups );
      ret = tech_parseNode( tech, node );
   } while (xml_nextNode(node));
   array_shrink( &tech_groups );

   /* Now we load the data. */
   node  = parent->xmlChildrenNode;
   s     = array_size( tech_groups );
   do {
      /* Must match tag. */
      if (!xml_isNode(node, XML_TECH_TAG))
         continue;

      /* Must aviod warning by checking explicit NULL. */
      xmlr_attr( node, "name", buf );
      if (buf == NULL)
         continue;

      /* Load next tech. */
      for (i=0; i<s; i++) {
         tech  = &tech_groups[i];
         if (strcmp(tech->name, buf)==0)
            tech_parseNodeData( tech, node );
      }

      /* Free memory. */
      free(buf);
   } while (xml_nextNode(node));

   /* Info. */
   DEBUG("Loaded %d tech group%s", s, (s == 1) ? "" : "s" );

   /* Free memory. */
   free(data);

   return 0;
}


/**
 * @brief Cleans up after the tech stuff.
 */
void tech_free (void)
{
   int i, s;

   /* Free all individual techs. */
   s = array_size( tech_groups );
   for (i=0; i<s; i++)
      tech_freeGroup( &tech_groups[i] );

   /* Free the tech array. */
   array_free( tech_groups );
}


/**
 * @brief Cleans up a tech group.
 */
static void tech_freeGroup( tech_group_t *grp )
{
   if (grp->name != NULL)
      free(grp->name);
   if (grp->items != NULL)
      array_free( grp->items );
}


/**
 * @brief Creates a tech group.
 */
tech_group_t *tech_groupCreate( xmlNodePtr node )
{
   tech_group_t *tech;
   /* Load data. */
   tech  = calloc( sizeof(tech_group_t), 1 );
   tech_parseNodeData( tech, node );
   return tech;
}


/**
 * @brief Frees a tech group.
 */
void tech_groupDestroy( tech_group_t *grp )
{
   if (grp == NULL)
      return;

   tech_freeGroup( grp );
   free(grp);
}


/**
 * @brief Gets an item's name. 
 */
static char* tech_getItemName( tech_item_t *item )
{
   /* Handle type. */
   switch (item->type) {
      case TECH_TYPE_OUTFIT:
         return item->u.outfit->name;
      case TECH_TYPE_SHIP:
         return item->u.ship->name;
      case TECH_TYPE_COMMODITY:
         return item->u.comm->name;
      case TECH_TYPE_GROUP:
         return tech_groups[ item->u.grp ].name;
   }

   return NULL;
}


/**
 * @brief Writes a group in an exml node.
 */
int tech_groupWrite( xmlTextWriterPtr writer, tech_group_t *grp )
{
   int i, s;

   /* Handle empty groups. */
   if (grp == NULL)
      return 0;

   /* Node header. */
   xmlw_startElem( writer, "tech" );

   /* Save items. */
   s  = array_size( grp->items );
   for (i=0; i<s; i++)
      xmlw_elem( writer, "item", "%s", tech_getItemName( &grp->items[i] ) );

   xmlw_endElem( writer ); /* "tech" */

   return 0;
}


/**
 * @brief Parses an XML tech node.
 */
static int tech_parseNode( tech_group_t *tech, xmlNodePtr parent )
{
   /* Just in case. */
   memset( tech, 0, sizeof(tech_group_t) );

   /* Get name. */
   xmlr_attr( parent, "name", tech->name);
   if (tech->name == NULL) {
      WARN("tech node does not have 'name' attribute");
      return 1;
   }

   return 0;
}


/**
 * @brief Parses an XML tech node.
 */
static int tech_parseNodeData( tech_group_t *tech, xmlNodePtr parent )
{
   xmlNodePtr node;
   char *buf, *name;
   int ret;

   /* Parse the data. */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"item")) {

         /* Must have name. */
         name = xml_get( node );
         if (name == NULL) {
            WARN("Tech group '%s' has an item without a value.", tech->name);
            continue;
         }

         /* Try to find hardcoded type. */
         buf = xml_nodeProp( node, "type" );
         if (buf == NULL) {
            ret = 1;
            if (ret)
               ret = tech_addItemGroup( tech, name );
            if (ret)
               ret = tech_addItemOutfit( tech, name );
            if (ret)
               ret = tech_addItemShip( tech, name );
            if (ret)
               ret = tech_addItemCommodity( tech, name );
            if (ret) {
               WARN("Generic item '%s' not found in tech group '%s'",
                     name, tech->name );
               continue;
            }
         }
         else if (strcmp(buf,"group")==0) {
            if (!tech_addItemGroup( tech, name )) {
               WARN("Group item '%s' not found in tech group '%s'.",
                     name, tech->name );
               continue;
            }
         }
         else if (strcmp(buf,"outfit")==0) {
            if (!tech_addItemGroup( tech, name )) {
               WARN("Outfit item '%s' not found in tech group '%s'.",
                     name, tech->name );
               continue;
            }
         }
         else if (strcmp(buf,"ship")==0) {
            if (!tech_addItemGroup( tech, name )) {
               WARN("Ship item '%s' not found in tech group '%s'.",
                     name, tech->name );
               continue;
            }
         }
         else if (strcmp(buf,"commodity")==0) {
            if (!tech_addItemGroup( tech, name )) {
               WARN("Commodity item '%s' not found in tech group '%s'.",
                     name, tech->name );
               continue;
            }
         }
      }
   } while (xml_nextNode( node ));

   return 0;
}


/**
 * @brief Adds an item to a tech.
 */
static tech_item_t *tech_itemGrow( tech_group_t *grp )
{
   if (grp->items == NULL)
      grp->items = array_create( tech_item_t );
   return &array_grow( &grp->items );
}


static int tech_addItemOutfit( tech_group_t *grp, const char *name )
{
   tech_item_t *item;
   Outfit *o;

   /* Get the outfit. */
   o = outfit_getW( name );
   if (o==NULL)
      return 1;
 
   /* Load the new item. */
   item           = tech_itemGrow( grp );
   item->type     = TECH_TYPE_OUTFIT;
   item->u.outfit = o;
   return 0;
}


/**
 * @brief Loads a group item pertaining to a outfit.
 *
 *    @return 0 on success.
 */
static int tech_addItemShip( tech_group_t *grp, const char* name )
{
   tech_item_t *item;
   Ship *s;

   /* Get the outfit. */
   s = ship_getW( name );
   if (s==NULL)
      return 1;
 
   /* Load the new item. */
   item           = tech_itemGrow( grp );
   item->type     = TECH_TYPE_SHIP;
   item->u.ship   = s;
   return 0;
}


/**
 * @brief Loads a group item pertaining to a outfit.
 *
 *    @return 0 on success.
 */
static int tech_addItemCommodity( tech_group_t *grp, const char* name )
{
   tech_item_t *item;
   Commodity *c;

   /* Get the outfit. */
   c = commodity_getW( name );
   if (c==NULL)
      return 1;
 
   /* Load the new item. */
   item           = tech_itemGrow( grp );
   item->type     = TECH_TYPE_COMMODITY;
   item->u.comm   = c;
   return 0;
}


/**
 * @brief Adds an item to a tech.
 */
int tech_addItem( const char *name, const char *value )
{
   int id, ret;
   tech_group_t *tech;

   /* Get ID. */
   id = tech_getID( name );
   if (id < 0) {
      WARN("Trying to add item '%s' to non-existant tech '%s'.", value, name );
      return -1;
   }

   /* Comfort. */
   tech  = &tech_groups[id];

   /* Try to add the techu. */ 
   ret = tech_addItemGroup( tech, value );
   if (ret)
      ret = tech_addItemOutfit( tech, value );
   if (ret)
      ret = tech_addItemShip( tech, value );
   if (ret)
      ret = tech_addItemCommodity( tech, value );
   if (ret) {
      WARN("Generic item '%s' not found in tech group '%s'", value, name );
      return -1;
   }

   return 0;
}


/**
 * @Brief Removes a tech item.
 */
int tech_rmItem( const char *name, const char *value )
{
   int id, i, s;
   tech_group_t *tech;
   char *buf;

   /* Get ID. */
   id = tech_getID( name);
   if (id < 0) {
      WARN("Trying to remove item '%s' to non-existant tech '%s'.", value, name );
      return -1;
   }

   /* Comfort. */
   tech  = &tech_groups[id];

   /* Iterate over to find it. */
   s = array_size( tech->items );
   for (i=0; i<s; i++) {
      buf = tech_getItemName( &tech->items[i] );
      if (strcmp(buf, value)==0) {
         array_erase( &tech->items, &tech->items[i], &tech->items[i+1] );
         return 0;
      }
   }

   WARN("Item '%s' not found in tech group '%s'", value, name );
   return -1;
}


/**
 * @brief Gets the ID of a tech.
 */
static int tech_getID( const char *name )
{
   int i, s;
   tech_group_t *tech;

   /* NULL cas. */
   if (tech_groups == NULL)
      return -1;

   s  = array_size( tech_groups );
   for (i=0; i<s; i++) {
      tech  = &tech_groups[i];
      if (tech->name == NULL)
         continue;
      if (strcmp(tech->name, name)==0)
         return i;
   }

   return -1L;
}


/**
 * @brief Loads a group item pertaining to a group.
 *
 *    @return 0 on success.
 */
static int tech_addItemGroup( tech_group_t *grp, const char *name )
{
   tech_item_t *item;
   int tech;

   /* Try to find the tech. */
   tech = tech_getID( name );
   if (tech < 0)
      return 1;

   /* Load the new item. */
   item           = tech_itemGrow( grp );
   item->type     = TECH_TYPE_GROUP;
   item->u.grp    = tech;
   return 0;
}


/**
 * @brief Recursive function for creating an array of outfits from a tech group.
 */
static Outfit** tech_addGroupOutfit( Outfit **o, tech_group_t *tech, int *n, int *m )
{
   int i, j, s, f;
   tech_item_t *item;

   /* Must have items. */
   if (tech->items == NULL)
      return o;

   /* Comfort. */
   s     = array_size( tech->items );

   /* Load outfits first, then we handle groups. */
   for (i=0; i<s; i++) {
      item = &tech->items[i];

      /* Only handle outfits for now. */
      if (item->type != TECH_TYPE_OUTFIT)
         continue;

      /* Skip if already in list. */
      f = 0;
      for (j=0; j<*n; j++) {
         if (o[j] == item->u.outfit) {
            f = 1;
            break;
         }
      }
      if (f == 1)
         continue;
   
      /* Allocate memory if needed. */
      (*n)++;
      if ((*n) > (*m)) {
         if ((*m) == 0)
            (*m)  = 1;
         (*m) *= 2;
         o = realloc( o, sizeof(Outfit*) * (*m) );
      }

      /* Add. */
      o[ (*n)-1 ]  = item->u.outfit;
   }
   
   /* Now handle other groups. */
   for (i=0; i<s; i++) {
      item = &tech->items[i];

      /* Only handle outfits for now. */
      if (item->type != TECH_TYPE_GROUP)
         continue;

      /* Recursive */
      o  = tech_addGroupOutfit( o, &tech_groups[ item->u.grp ], n, m );
   }

   return o;
}


/**
 * @brief Gets all of the outfits assosciated to a tech group.
 */
Outfit** tech_getOutfit( tech_group_t *tech, int *n )
{
   int m;
   Outfit **o;

   /* Get the outfits. */
   *n = 0;
   m  = 0;
   o  = tech_addGroupOutfit( NULL, tech, n, &m );

   /* None found case. */
   if (o == NULL) {
      *n = 0;
      return NULL;
   }

   /* Sort. */
   qsort( o, *n, sizeof(Outfit*), outfit_compareTech );
   return o;
}


/**
 * @brief Recursive function for creating an array of outfits from a tech group.
 */
static Ship** tech_addGroupShip( Ship **s, tech_group_t *tech, int *n, int *m )
{
   int i, j, size, f;
   tech_item_t *item;

   /* Must have items. */
   if (tech->items == NULL)
      return s;

   /* Comfort. */
   size  = array_size( tech->items );

   /* Load outfits first, then we handle groups. */
   for (i=0; i<size; i++) {
      item = &tech->items[i];

      /* Only handle outfits for now. */
      if (item->type != TECH_TYPE_SHIP)
         continue;

      /* Skip if already in list. */
      f = 0;
      for (j=0; j<*n; j++) {
         if (s[j] == item->u.ship) {
            f = 1;
            break;
         }
      }
      if (f == 1)
         continue;
   
      /* Allocate memory if needed. */
      (*n)++;
      if ((*n) > (*m)) {
         if ((*m) == 0)
            (*m)  = 1;
         (*m) *= 2;
         s = realloc( s, sizeof(Ship*) * (*m) );
      }

      /* Add. */
      s[ (*n)-1 ]  = item->u.ship;
   }
   
   /* Now handle other groups. */
   for (i=0; i<size; i++) {
      item = &tech->items[i];

      /* Only handle outfits for now. */
      if (item->type != TECH_TYPE_GROUP)
         continue;

      /* Recursive */
      s  = tech_addGroupShip( s, &tech_groups[ item->u.grp ], n, m );
   }

   return s;
}


/**
 * @brief Gets all of the ships assosciated to a tech group.
 */
Ship** tech_getShip( tech_group_t *tech, int *n )
{
   int m;
   Ship **s;

   /* Get the outfits. */
   *n = 0;
   m  = 0;
   s  = tech_addGroupShip( NULL, tech, n, &m );

   /* None found case. */
   if (s == NULL) {
      *n = 0;
      return NULL;
   }

   /* Sort. */
   qsort( s, *n, sizeof(Ship*), outfit_compareTech );
   return s;
}


