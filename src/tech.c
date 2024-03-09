/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file tech.c
 *
 * @brief Handles tech groups and metagroups for populating the spob outfitter,
 *        shipyard and commodity exchange.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "tech.h"

#include "conf.h"
#include "array.h"
#include "commodity.h"
#include "log.h"
#include "ndata.h"
#include "nxml.h"
#include "outfit.h"
#include "ship.h"

#define XML_TECH_ID   "Techs" /**< Tech xml document tag. */
#define XML_TECH_TAG  "tech"  /**< Individual tech xml tag. */

/**
 * @brief Different tech types.
 */
typedef enum tech_item_type_e {
   TECH_TYPE_OUTFIT,       /**< Tech contains an outfit. */
   TECH_TYPE_SHIP,         /**< Tech contains a ship. */
   TECH_TYPE_COMMODITY,    /**< Tech contains a commodity. */
   TECH_TYPE_GROUP,        /**< Tech contains another tech group. */
   TECH_TYPE_GROUP_POINTER /**< Tech contains a tech group pointer. */
} tech_item_type_t;

/**
 * @brief Item contained in a tech group.
 */
typedef struct tech_item_s {
   tech_item_type_t type;  /**< Type of data. */
   union {
      void *ptr;           /**< Pointer when needing to do indifferent voodoo. */
      const Outfit *outfit;/**< Outfit pointer. */
      const Ship *ship;    /**< Ship pointer. */
      const Commodity *comm;/**< Commodity pointer. */
      int grp;             /**< Identifier of another tech group. */
      const tech_group_t *grpptr; /**< Pointer to another tech group. */
   } u;                    /**< Data union. */
} tech_item_t;

/**
 * @brief Group of tech items, basic unit of the tech trees.
 */
struct tech_group_s {
   char *name;          /**< Name of the tech group. */
   char *filename;      /**< Name of the file. */
   tech_item_t *items;  /**< Items in the tech group. */
};

/*
 * Group list.
 */
static tech_group_t *tech_groups = NULL;

/*
 * Prototypes.
 */
static void tech_createMetaGroup( tech_group_t *grp, tech_group_t **tech, int num );
static void tech_freeGroup( tech_group_t *grp );
static char* tech_getItemName( tech_item_t *item );
/* Loading. */
static tech_item_t *tech_itemGrow( tech_group_t *grp );
static int tech_parseFile( tech_group_t *tech, const char *file );
static int tech_parseFileData( tech_group_t *tech );
static int tech_parseXMLData( tech_group_t *tech, xmlNodePtr parent );
static int tech_addItemOutfit( tech_group_t *grp, const char* name );
static int tech_addItemShip( tech_group_t *grp, const char* name );
static int tech_addItemCommodity( tech_group_t *grp, const char* name );
static int tech_getID( const char *name );
static int tech_addItemGroupPointer( tech_group_t *grp, const tech_group_t *ptr );
static int tech_addItemGroup( tech_group_t *grp, const char* name );
/* Getting by tech. */
static void** tech_addGroupItem( void **items, tech_item_type_t type, const tech_group_t *tech );


static int tech_cmp( const void *p1, const void *p2 )
{
   const tech_group_t *t1 = p1;
   const tech_group_t *t2 = p2;
   return strcmp(t1->name,t2->name);
}

/**
 * @brief Loads the tech information.
 */
int tech_load (void)
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   int s;
   char **tech_files = ndata_listRecursive( TECH_DATA_PATH );

   /* Create the array. */
   tech_groups = array_create( tech_group_t );

   /* First pass create the groups - needed to reference them later. */
   for (int i=0; i<array_size(tech_files); i++) {
      tech_group_t tech;
      int ret;

      if (!ndata_matchExt( tech_files[i], "xml" ))
         continue;

      ret = tech_parseFile( &tech, tech_files[i] );
      if (ret==0) {
         tech.filename = strdup( tech_files[i] );
         array_push_back( &tech_groups, tech );
      }

      free( tech_files[i] );
   }
   array_free( tech_files );
   array_shrink( &tech_groups );

   /* Sort. */
   qsort( tech_groups, array_size(tech_groups), sizeof(tech_group_t), tech_cmp );

   /* Now we load the data. */
   s = array_size( tech_groups );
   for (int i=0; i<s; i++)
      tech_parseFileData( &tech_groups[i] );

   /* Info. */
#if DEBUGGING
   if (conf.devmode) {
      DEBUG( n_( "Loaded %d tech group in %.3f s", "Loaded %d tech groups in %.3f s", s ), s, (SDL_GetTicks()-time)/1000. );
   }
   else
      DEBUG( n_( "Loaded %d tech group", "Loaded %d tech groups", s ), s );
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Cleans up after the tech stuff.
 */
void tech_free (void)
{
   /* Free all individual techs. */
   int s = array_size( tech_groups );
   for (int i=0; i<s; i++)
      tech_freeGroup( &tech_groups[i] );

   /* Free the tech array. */
   array_free( tech_groups );
}

/**
 * @brief Cleans up a tech group.
 */
static void tech_freeGroup( tech_group_t *grp )
{
   free(grp->name);
   free(grp->filename);
   array_free( grp->items );
}

/**
 * @brief Creates a tech group from an XML node.
 */
tech_group_t *tech_groupCreateXML( xmlNodePtr node )
{
   /* Load data. */
   tech_group_t *tech = tech_groupCreate();
   tech_parseXMLData( tech, node );
   return tech;
}

/**
 * @brief Creates a tech group.
 */
tech_group_t *tech_groupCreate( void )
{
   tech_group_t *tech = calloc( sizeof(tech_group_t), 1 );
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
      case TECH_TYPE_GROUP_POINTER:
         return item->u.grpptr->name;
   }

   return NULL;
}

/**
 * @brief Writes a group in an xml node.
 */
int tech_groupWrite( xmlTextWriterPtr writer, tech_group_t *grp )
{
   int s;

   /* Handle empty groups. */
   if (grp == NULL)
      return 0;

   /* Node header. */
   xmlw_startElem( writer, "tech" );

   /* Save items. */
   s = array_size( grp->items );
   for (int i=0; i<s; i++)
      xmlw_elem( writer, "item", "%s", tech_getItemName( &grp->items[i] ) );

   xmlw_endElem( writer ); /* "tech" */

   return 0;
}

/**
 * @brief Parses an XML tech node.
 */
static int tech_parseFile( tech_group_t *tech, const char *file )
{
   xmlNodePtr parent;
   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first faction node */
   if (parent == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), file);
      return -1;
   }

   /* Just in case. */
   memset( tech, 0, sizeof(tech_group_t) );

   /* Get name. */
   xmlr_attr_strd( parent, "name", tech->name );
   if (tech->name == NULL) {
      WARN(_("tech node does not have 'name' attribute"));
      return 1;
   }

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Parses an XML tech node.
 */
static int tech_parseXMLData( tech_group_t *tech, xmlNodePtr parent )
{
   /* Parse the data. */
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,"item")) {
         char *buf, *name;

         /* Must have name. */
         name = xml_get( node );
         if (name == NULL) {
            WARN(_("Tech group '%s' has an item without a value."), tech->name);
            continue;
         }

         /* Try to find hard-coded type. */
         xmlr_attr_strd( node, "type", buf );
         if (buf == NULL) {
            int ret = 1;
            //if (ret)
               ret = tech_addItemGroup( tech, name );
            if (ret)
               ret = tech_addItemOutfit( tech, name );
            if (ret)
               ret = tech_addItemShip( tech, name );
            if (ret)
               ret = tech_addItemCommodity( tech, name );
            if (ret)
               WARN(_("Generic item '%s' not found in tech group '%s'"),
                     name, tech->name );
         }
         else if (strcmp(buf,"group")==0) {
            if (!tech_addItemGroup( tech, name ))
               WARN(_("Group item '%s' not found in tech group '%s'."),
                     name, tech->name );
         }
         else if (strcmp(buf,"outfit")==0) {
            if (!tech_addItemGroup( tech, name ))
               WARN(_("Outfit item '%s' not found in tech group '%s'."),
                     name, tech->name );
         }
         else if (strcmp(buf,"ship")==0) {
            if (!tech_addItemGroup( tech, name ))
               WARN(_("Ship item '%s' not found in tech group '%s'."),
                     name, tech->name );
         }
         else if (strcmp(buf,"commodity")==0) {
            if (!tech_addItemGroup( tech, name ))
               WARN(_("Commodity item '%s' not found in tech group '%s'."),
                     name, tech->name );
         }
         free( buf );
         continue;
      }
      WARN(_("Tech group '%s' has unknown node '%s'."), tech->name, node->name);
   } while (xml_nextNode( node ));

   return 0;
}

/**
 * @brief Parses an XML tech node.
 */
static int tech_parseFileData( tech_group_t *tech )
{
   xmlNodePtr parent;
   const char *file = tech->filename;
   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first faction node */
   if (parent == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), file);
      return -1;
   }

   /* Parse the data. */
   tech_parseXMLData( tech, parent );

   xmlFreeDoc(doc);

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
   const Outfit *o;

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
   const Ship *s;

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
      WARN(_("Trying to add item '%s' to non-existent tech '%s'."), value, name );
      return -1;
   }

   /* Comfort. */
   tech  = &tech_groups[id];

   /* Try to add the tech. */
   ret = tech_addItemGroup( tech, value );
   if (ret)
      ret = tech_addItemOutfit( tech, value );
   if (ret)
      ret = tech_addItemShip( tech, value );
   if (ret)
      ret = tech_addItemCommodity( tech, value );
   if (ret) {
      WARN(_("Generic item '%s' not found in tech group '%s'"), value, name );
      return -1;
   }

   return 0;
}

/**
 * @brief Adds an item to a tech.
 */
int tech_addItemTech( tech_group_t *tech, const char *value )
{
   /* Try to add the tech. */
   int ret = tech_addItemGroup( tech, value );
   if (ret)
      ret = tech_addItemOutfit( tech, value );
   if (ret)
      ret = tech_addItemShip( tech, value );
   if (ret)
      ret = tech_addItemCommodity( tech, value );
   if (ret) {
      WARN(_("Generic item '%s' not found in tech group"), value );
      return -1;
   }
   return 0;
}

/**
 * @brief Removes an item from a tech.
 */
int tech_rmItemTech( tech_group_t *tech, const char *value )
{
   /* Iterate over to find it. */
   int s = array_size( tech->items );
   for (int i=0; i<s; i++) {
      const char *buf = tech_getItemName( &tech->items[i] );
      if (strcmp(buf, value)==0) {
         array_erase( &tech->items, &tech->items[i], &tech->items[i+1] );
         return 0;
      }
   }

   WARN(_("Item '%s' not found in tech group"), value );
   return -1;
}

/**
 * @brief Removes a tech item.
 */
int tech_rmItem( const char *name, const char *value )
{
   int id, s;
   tech_group_t *tech;

   /* Get ID. */
   id = tech_getID( name);
   if (id < 0) {
      WARN(_("Trying to remove item '%s' to non-existent tech '%s'."), value, name );
      return -1;
   }

   /* Comfort. */
   tech  = &tech_groups[id];

   /* Iterate over to find it. */
   s = array_size( tech->items );
   for (int i=0; i<s; i++) {
      const char *buf = tech_getItemName( &tech->items[i] );
      if (strcmp(buf, value)==0) {
         array_erase( &tech->items, &tech->items[i], &tech->items[i+1] );
         return 0;
      }
   }

   WARN(_("Item '%s' not found in tech group '%s'"), value, name );
   return -1;
}

/**
 * @brief Gets the ID of a tech.
 */
static int tech_getID( const char *name )
{
   const tech_group_t q = { .name = (char*)name };
   const tech_group_t *t = bsearch( &q, tech_groups, array_size(tech_groups), sizeof(tech_group_t), tech_cmp );
   if (t==NULL)
      return -1L;
   return t-tech_groups;
}

/**
 * @brief Adds a group pointer to a group.
 */
static int tech_addItemGroupPointer( tech_group_t *grp, const tech_group_t *ptr )
{
   /* Load the new item. */
   tech_item_t *item = tech_itemGrow( grp );
   item->type        = TECH_TYPE_GROUP_POINTER;
   item->u.grpptr    = ptr;
   return 0;
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
 * @brief Creates a meta-tech group pointing only to other groups.
 *
 *    @param grp Group to initialize.
 *    @param tech List of tech groups to attach.
 *    @param num Number of tech groups.
 */
static void tech_createMetaGroup( tech_group_t *grp, tech_group_t **tech, int num )
{
   /* Create meta group. */
   memset( grp, 0, sizeof(tech_group_t) );

   /* Create a meta-group. */
   for (int i=0; i<num; i++)
      tech_addItemGroupPointer( grp, tech[i] );
}

/**
 * @brief Recursive function for creating an array of commodities from a tech group.
 */
static void** tech_addGroupItem( void **items, tech_item_type_t type, const tech_group_t *tech )
{
   /* Set up. */
   int size  = array_size( tech->items );

   /* Load commodities first, then we handle groups. */
   for (int i=0; i<size; i++) {
      int f;
      tech_item_t *item = &tech->items[i];

      /* Only handle commodities for now. */
      if (item->type != type)
         continue;

      /* Skip if already in list. */
      f = 0;
      for (int j=0; j<array_size(items); j++) {
         if (items[j] == item->u.ptr) {
            f = 1;
            break;
         }
      }
      if (f == 1)
         continue;

      /* Add. */
      if (items == NULL)
         items = array_create( void* );
      array_push_back( &items, item->u.ptr );
   }

   /* Now handle other groups. */
   for (int i=0; i<size; i++) {
      tech_item_t *item = &tech->items[i];

      /* Only handle commodities for now. */
      if (item->type == TECH_TYPE_GROUP)
         items  = tech_addGroupItem( items, type, &tech_groups[ item->u.grp ] );
      else if (item->type == TECH_TYPE_GROUP_POINTER)
         items  = tech_addGroupItem( items, type, item->u.grpptr );
   }

   return items;
}

/**
 * @brief Checks whether a given tech group has the specified item.
 *
 *    @param tech Tech to search within.
 *    @param item The item name to search for.
 *    @return Whether or not the item was found.
 */
int tech_hasItem( const tech_group_t *tech, const char *item )
{
   int s = array_size( tech->items );
   for (int i=0; i<s; i++) {
      const char *buf = tech_getItemName( &tech->items[i] );
      if (strcmp(buf,item)==0)
         return 1;
   }
   return 0;
}

/**
 * @brief Gets the number of techs within a given group.
 *
 *   @return Number of techs.
 */
int tech_getItemCount( const tech_group_t *tech )
{
   return array_size( tech->items );
}

/**
 * @brief Gets the names of all techs within a given group.
 *
 *    @param tech Tech group to operate on.
 *    @param[out] n Number of techs in the group.
 *    @return The names of the techs contained within the group.
 */
char** tech_getItemNames( const tech_group_t *tech, int *n )
{
   int s;
   char **names;

   *n = s = array_size( tech->items );
   names = malloc( sizeof(char*) * s );

   for (int i=0; i<s; i++)
      names[i] = strdup( tech_getItemName( &tech->items[i] ) );

   return names;
}

/**
 * @brief Gets the names of all techs.
 *
 *    @param[out] n Number of techs.
 *    @return The names of all techs.
 */
char** tech_getAllItemNames( int *n )
{
   int s;
   char **names;

   *n = s = array_size( tech_groups );
   names = malloc( sizeof(char*) * s );

   for (int i=0; i<s; i++)
      names[i] = strdup( tech_groups[i].name );

   return names;
}

/**
 * @brief Gets all of the outfits associated to a tech group.
 *
 * @note The returned list must be freed (but not the pointers).
 *
 *    @param tech Tech to get outfits from.
 *    @return Array (array.h): Outfits found.
 */
Outfit** tech_getOutfit( const tech_group_t *tech )
{
   Outfit **o;

   if (tech==NULL)
      return NULL;

   o  = (Outfit**)tech_addGroupItem( NULL, TECH_TYPE_OUTFIT, tech );

   /* Sort. */
   if (o != NULL)
      qsort( o, array_size(o), sizeof(Outfit*), outfit_compareTech );

   return o;
}

/**
 * @brief Gets the outfits from an array of techs.
 *
 * @note The returned list must be freed (but not the pointers).
 *
 *    @param tech Array of techs to get from.
 *    @param num Number of elements in the array.
 *    @return Array (array.h): Outfits found.
 */
Outfit** tech_getOutfitArray( tech_group_t **tech, int num )
{
   tech_group_t grp;
   Outfit **o;

   if (tech==NULL)
      return NULL;

   tech_createMetaGroup( &grp, tech, num );
   o = tech_getOutfit( &grp );
   tech_freeGroup( &grp );

   return o;
}

/**
 * @brief Gets all of the ships associated to a tech group.
 *
 * @note The returned array must be freed (but not the pointers).
 *
 *    @param tech Tech group to get list of ships from.
 *    @return Array (array.h): The ships found.
 */
Ship** tech_getShip( const tech_group_t *tech )
{
   Ship **s;

   if (tech==NULL)
      return NULL;

   /* Get the outfits. */
   s  = (Ship**) tech_addGroupItem( NULL, TECH_TYPE_SHIP, tech );

   /* Sort. */
   if (s != NULL)
      qsort( s, array_size(s), sizeof(Ship*), ship_compareTech );

   return s;
}

/**
 * @brief Gets the ships from an array of techs.
 *
 * @note The returned list must be freed (but not the pointers).
 *
 *    @param tech Array of techs to get from.
 *    @param num Number of elements in the array.
 *    @return Array (array.h): Ships found.
 */
Ship** tech_getShipArray( tech_group_t **tech, int num )
{
   tech_group_t grp;
   Ship **s;

   if (tech==NULL)
      return NULL;

   tech_createMetaGroup( &grp, tech, num );
   s = tech_getShip( &grp );
   tech_freeGroup( &grp );

   return s;
}

/**
 * @brief Gets the ships from an array of techs.
 *
 * @note The returned list must be freed (but not the pointers).
 *
 *    @param tech Array of techs to get from.
 *    @param num Number of elements in the array.
 *    @return Array (array.h): Commodities found.
 */
Commodity** tech_getCommodityArray( tech_group_t **tech, int num )
{
   tech_group_t grp;
   Commodity **c;

   if (tech==NULL)
      return NULL;

   tech_createMetaGroup( &grp, tech, num );
   c = tech_getCommodity( &grp );
   tech_freeGroup( &grp );

   return c;
}

/**
 * @brief Gets all of the ships associated to a tech group.
 *
 * @note The returned array must be freed (but not the pointers).
 *
 *    @param tech Tech group to get list of ships from.
 *    @return Array (array.h): The commodities found.
 */
Commodity** tech_getCommodity( const tech_group_t *tech )
{
   Commodity **c;

   if (tech==NULL)
      return NULL;

   /* Get the commodities. */
   c  = (Commodity**) tech_addGroupItem( NULL, TECH_TYPE_COMMODITY, tech );

   /* Sort. */
   if (c != NULL)
      qsort( c, array_size(c), sizeof(Commodity*), commodity_compareTech );

   return c;
}

/**
 * @brief Checks to see if there is an outfit in the tech group.
 */
int tech_checkOutfit( const tech_group_t *tech, const Outfit *o )
{
   Outfit **to = tech_getOutfit( tech );
   for (int i=0; i<array_size(to); i++) {
      if (to[i]==o) {
         array_free(to);
         return 1;
      }
   }
   array_free(to);
   return 0;
}
