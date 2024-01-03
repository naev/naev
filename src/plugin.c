/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file plugin.c
 *
 * @brief Handles plugins to augment data.
 */
/** @cond */
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "plugin.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "log.h"
#include "array.h"
#include "nfile.h"
#include "nxml.h"
#include "physfs_archiver_blacklist.h"

static plugin_t *plugins;

/**
 * @brief Parses a plugin description file.
 */
static int plugin_parse( plugin_t *plg, const char *file, const char *path )
{
   xmlNodePtr node, parent;
   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first faction node */
   if (parent == NULL) {
      char buf[ PATH_MAX ];
      nfile_concatPaths( buf, sizeof(buf), plg->mountpoint, file );
      ERR( _("Malformed '%s' file: does not contain elements"), buf);
      return -1;
   }

   xmlr_attr_strd( parent, "name", plg->name );
   if (plg->name == NULL)
      WARN(_("Plugin '%s' has no name!"), path);

   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      xmlr_strd( node, "author", plg->author );
      xmlr_strd( node, "version", plg->version );
      xmlr_strd( node, "description", plg->description );
      xmlr_strd( node, "compatibility", plg->compatibility );
      xmlr_int( node, "priority", plg->priority );
      xmlr_strd( node, "source", plg->source );
      if (xml_isNode( node, "blacklist" )) {
         blacklist_append( xml_get(node) );
         continue;
      }
      if (xml_isNode( node, "total_conversion" )) {
         const char *blk[] = {
            "^ssys/.*\\.xml",
            "^spob/.*\\.xml",
            "^spob_virtual/.*\\.xml",
            "^factions/.*\\.xml",
            "^commodities/.*\\.xml",
            "^ships/.*\\.xml",
            "^outfits/.*\\.xml",
            "^missions/.*\\.lua",
            "^events/.*\\.lua",
            "^tech/.*\\.xml",
            "^asteroids/.*\\.xml",
            "^unidiff/.*\\.xml",
            "^map_decorator/.*\\.xml",
            "^intro",
            NULL
         };
         for (int i=0; blk[i]!=NULL; i++)
            blacklist_append( blk[i] );
         plg->total_conversion = 1;
         continue;
      }
      WARN(_("Plugin '%s' has unknown metadata node '%s'!"),path,xml_get(node));
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief For qsort on plugins.
 */
static int plugin_cmp( const void *a, const void *b )
{
   const plugin_t *pa = (const plugin_t *) a;
   const plugin_t *pb = (const plugin_t *) b;

   if (pa->priority < pb->priority)
      return -1;
   else if (pa->priority > pb->priority)
      return 1;

   return strcmp( pa->mountpoint, pb->mountpoint );
}

/**
 * @brief Initialize and loads all the available plugins.
 *
 *    @return 0 on success.
 */
int plugin_init (void)
{
   char buf[ PATH_MAX ];
   PHYSFS_Stat stat;

   plugins = array_create( plugin_t );

   PHYSFS_stat( "plugins", &stat );
   if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
      int n;
      char **files = PHYSFS_enumerateFiles( "plugins" );
      for (char **f = files; *f != NULL; f++) {
         const char *realdir;
         plugin_t *plg;
         nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins", *f );
         if (!nfile_fileExists( buf ) && !nfile_dirExists( buf ))
            continue;

         if (PHYSFS_mount( buf, NULL, 0 )==0) {
            WARN(_("Failed to mount plugin '%s': %s"), buf,
                  _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode()) ));
         }

         /* Initialize new plugin. */
         plg = &array_grow( &plugins );
         memset( plg, 0, sizeof(plugin_t) );
         plg->mountpoint = strdup( buf );
         plg->priority = 5;

         PHYSFS_stat( "plugin.xml", &stat );
         realdir = PHYSFS_getRealDir( "plugin.xml" );
         if ((stat.filetype == PHYSFS_FILETYPE_REGULAR) &&
               realdir && strcmp(realdir,buf)==0)
            plugin_parse( plg, "plugin.xml", *f );
         else
            WARN(_("Plugin '%s' does not have a valid '%s'!"), buf, "plugin.xml");

         /* Set some defaults. */
         if (plg->author == NULL)
            plg->author = strdup(_("Unknown"));
         if (plg->version == NULL)
            plg->version = strdup(_("Unknown"));
         if (plg->description == NULL)
            plg->description = strdup(_("Unknown"));
      }
      PHYSFS_freeList(files);
      n = array_size(plugins);

      /* Unmount all. */
      for (int i=0; i<n; i++)
         PHYSFS_unmount( plugins[i].mountpoint );

      /* Sort by priority. */
      qsort( plugins, n, sizeof(plugin_t), plugin_cmp );

      /* Initialize blacklist. */
      blacklist_init();

      /* Remount. */
      for (int i=n-1; i>=0; i--) /* Reverse order as we prepend. */
         PHYSFS_mount( plugins[i].mountpoint, NULL, 0 );

      if (n > 0) {
         DEBUG("Loaded plugins:");
         for (int i=0; i<n; i++) { /* Reverse order. */
            DEBUG("   %s", plugin_name( &plugins[i] ) );
         }
      }
   }
   else {
      nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins" );
      nfile_dirMakeExist( buf );
   }

   return 0;
}

/**
 * @brief Exits the plugin stuff.
 */
void plugin_exit (void)
{
   for (int i=0; i<array_size(plugins); i++) {
      plugin_t *p = &plugins[i];
      free( p->name );
      free( p->author );
      free( p->version );
      free( p->description );
      free( p->compatibility );
      free( p->source );
      free( p->mountpoint );
   }
   array_free(plugins);
   plugins = NULL;

   blacklist_exit();
}

/**
 * @brief Tries to tget the name of a plugin.
 *
 *    @param plg Plugin to try to get name of.
 *    @return Name of the plugin.
 */
const char *plugin_name( const plugin_t *plg )
{
   if (plg->name != NULL)
      return plg->name;
   return plg->mountpoint;
}

/**
 * @brief Checks to see if the plugins are self-declared compatible with Naev.
 *
 *    @return Number of incompatible plugins.
 */
int plugin_check (void)
{
   int failed = 0;
   const char *version = naev_version( 0 );
   for (int i=0; i<array_size(plugins); i++) {
      int errornumber;
      PCRE2_SIZE erroroffset;
      pcre2_code *re;
      pcre2_match_data *match_data;

      if (plugins[i].compatibility == NULL)
         continue;

      re = pcre2_compile( (PCRE2_SPTR)plugins[i].compatibility, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL );
      if (re == NULL) {
         PCRE2_UCHAR buffer[256];
         pcre2_get_error_message( errornumber, buffer, sizeof(buffer) );
         WARN(_("Plugin '%s' PCRE2 compilation failed at offset %d: %s"), plugin_name(&plugins[i]), (int)erroroffset, buffer );
         failed++;
         continue;
      }

      match_data = pcre2_match_data_create_from_pattern( re, NULL );
      int rc = pcre2_match( re, (PCRE2_SPTR)version, strlen(version), 0, 0, match_data, NULL );
      pcre2_match_data_free( match_data );
      if (rc < 0) {
         switch (rc) {
            case PCRE2_ERROR_NOMATCH:
               WARN("Plugin '%s' does not support Naev version '%s'.", plugin_name(&plugins[i]), version);
               failed++;
               break;
            default:
               WARN(_("Matching error %d"), rc );
               failed++;
               break;
         }
      }
      else
         plugins[i].compatible = 1;

      pcre2_code_free( re );
   }

   return failed;
}

/**
 * @brief Returns the list of all the plugins.
 *
 *    @return List of the loaded plugins.
 */
const plugin_t *plugin_list (void)
{
   return plugins;
}
