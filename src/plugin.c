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

#include "array.h"
#include "log.h"
#include "nfile.h"
#include "nxml.h"
#include "physfs_archiver_blacklist.h"

static plugin_t *plugins; /**< The list of active plugins. */

/*
 * Prototypes.
 */
static int plugin_parse( plugin_t *plg, const char *file, const char *path,
                         int apply );

/**
 * @brief Tests to see if a file is a plugin and loads information.
 */
plugin_t *plugin_test( const char *filename )
{
   int         ret;
   PHYSFS_Stat stat;
   const char *realdir;

   /* File must exist. */
   if ( !nfile_fileExists( filename ) )
      return NULL;

   /* Try to mount. */
   if ( PHYSFS_mount( filename, NULL, 0 ) == 0 ) {
      WARN( _( "Failed to mount plugin '%s': %s" ), filename,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      return NULL;
   }

   /* Check to see if plugin file exists. */
   PHYSFS_stat( "plugin.xml", &stat );
   realdir = PHYSFS_getRealDir( "plugin.xml" );
   if ( ( stat.filetype != PHYSFS_FILETYPE_REGULAR ) || ( realdir == NULL ) ||
        strcmp( realdir, filename ) != 0 )
      return NULL;

   /* Load data and send over. */
   plugin_t *plg = calloc( 1, sizeof( plugin_t ) );
   ret           = plugin_parse( plg, "plugin.xml", filename, 0 );

   /* Set some defaults. */
   if ( plg->author == NULL )
      plg->author = strdup( _( "Unknown" ) );
   if ( plg->version == NULL )
      plg->version = strdup( _( "Unknown" ) );
   if ( plg->description == NULL )
      plg->description = strdup( _( "Unknown" ) );

   /* Clean up. */
   PHYSFS_unmount( filename );

   if ( ret )
      return NULL;
   return plg;
}

/**
 * @brief Parses a plugin description file.
 */
static int plugin_parse( plugin_t *plg, const char *file, const char *path,
                         int apply )
{
   xmlNodePtr node, parent;
   xmlDocPtr  doc = xml_parsePhysFS( file );
   if ( doc == NULL )
      return -1;

   parent = doc->xmlChildrenNode; /* first faction node */
   if ( parent == NULL ) {
      char buf[PATH_MAX];
      nfile_concatPaths( buf, sizeof( buf ), plg->mountpoint, file );
      WARN( _( "Malformed '%s' file: does not contain elements" ), buf );
      return -1;
   }

   xmlr_attr_strd( parent, "name", plg->name );
   if ( plg->name == NULL )
      WARN( _( "Plugin '%s' has no name!" ), path );

   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes( node );

      xmlr_strd( node, "author", plg->author );
      xmlr_strd( node, "version", plg->version );
      xmlr_strd( node, "description", plg->description );
      xmlr_strd( node, "compatibility", plg->compatibility );
      xmlr_strd( node, "naev_version", plg->naev_version );
      xmlr_int( node, "priority", plg->priority );
      xmlr_strd( node, "source", plg->source );
      if ( xml_isNode( node, "blacklist" ) ) {
         if ( apply )
            blacklist_append( xml_get( node ) );
         continue;
      }
      if ( xml_isNode( node, "whitelist" ) ) {
         if ( apply )
            whitelist_append( xml_get( node ) );
         continue;
      }
      if ( xml_isNode( node, "total_conversion" ) ) {
         if ( apply ) {
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
               "^naevpedia/.*\\.xml",
               "^intro",
               NULL,
            };
            const char *wht[] = {
               "^events/settings.lua",
               NULL,
            };
            for ( int i = 0; blk[i] != NULL; i++ )
               blacklist_append( blk[i] );
            for ( int i = 0; wht[i] != NULL; i++ )
               whitelist_append( wht[i] );
         }
         plg->total_conversion = 1;
         continue;
      }

      WARN( _( "Plugin '%s' has unknown metadata node '%s'!" ),
            plugin_name( plg ), xml_get( node ) );
   } while ( xml_nextNode( node ) );

   xmlFreeDoc( doc );

   if ( plg->compatibility != NULL ) {
      WARN(
         "Plugin '%s' uses deprecated <compatibility> that will be removed "
         "in 0.14.0. Use <naev_version> with semver requirements "
         "instead:\n<naev_version>&gt;=0.13.0-0, &lt;0.14.0-0</naev_version>.",
         plg->name );
   }

#define MELEMENT( o, s )                                                       \
   if ( o )                                                                    \
   WARN( _( "Plugin '%s' missing/invalid '%s' element" ), plg->name,           \
         s ) /**< Define to help check for data errors. */
   MELEMENT( plg->author == NULL, "author" );
   MELEMENT( plg->version == NULL, "version" );
   MELEMENT( plg->description == NULL, "description" );
   MELEMENT( plg->naev_version == NULL, "naev_version" );
   MELEMENT( plg->source == NULL, "source" );
#undef MELEMENT

   return 0;
}

/**
 * @brief For qsort on plugins.
 */
static int plugin_cmp( const void *a, const void *b )
{
   const plugin_t *pa = (const plugin_t *)a;
   const plugin_t *pb = (const plugin_t *)b;

   if ( pa->priority < pb->priority )
      return -1;
   else if ( pa->priority > pb->priority )
      return 1;

   return strcmp( pa->mountpoint, pb->mountpoint );
}

/**
 * @brief Gets the plugin directory.
 */
const char *plugin_dir( void )
{
   static char dir[PATH_MAX];
   nfile_concatPaths( dir, sizeof( dir ), PHYSFS_getWriteDir(), "plugins" );
   return dir;
}

/**
 * @brief Initialize and loads all the available plugins.
 *
 *    @return 0 on success.
 */
int plugin_init( void )
{
   char        buf[PATH_MAX];
   PHYSFS_Stat stat;

   plugins = array_create( plugin_t );

   PHYSFS_stat( "plugins", &stat );
   if ( stat.filetype == PHYSFS_FILETYPE_DIRECTORY ) {
      int    n;
      char **files = PHYSFS_enumerateFiles( "plugins" );
      for ( char **f = files; *f != NULL; f++ ) {
         const char *realdir;
         plugin_t   *plg;
         nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins",
                            *f );
         if ( !nfile_fileExists( buf ) && !nfile_dirExists( buf ) )
            continue;

         if ( PHYSFS_mount( buf, NULL, 0 ) == 0 ) {
            WARN( _( "Failed to mount plugin '%s': %s" ), buf,
                  _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
            continue;
         }

         /* Initialize new plugin. */
         plg = &array_grow( &plugins );
         memset( plg, 0, sizeof( plugin_t ) );
         plg->mountpoint = strdup( buf );
         plg->priority   = 5;

         PHYSFS_stat( "plugin.xml", &stat );
         realdir = PHYSFS_getRealDir( "plugin.xml" );
         if ( ( stat.filetype == PHYSFS_FILETYPE_REGULAR ) && realdir &&
              strcmp( realdir, buf ) == 0 )
            plugin_parse( plg, "plugin.xml", *f, 1 );
         else
            WARN( _( "Plugin '%s' does not have a valid '%s'!" ), buf,
                  "plugin.xml" );

         /* Set some defaults. */
         if ( plg->author == NULL )
            plg->author = strdup( _( "Unknown" ) );
         if ( plg->version == NULL )
            plg->version = strdup( _( "Unknown" ) );
         if ( plg->description == NULL )
            plg->description = strdup( _( "Unknown" ) );
      }
      PHYSFS_freeList( files );
      n = array_size( plugins );

      /* Unmount all. */
      for ( int i = 0; i < n; i++ )
         PHYSFS_unmount( plugins[i].mountpoint );

      /* Sort by priority. */
      qsort( plugins, n, sizeof( plugin_t ), plugin_cmp );

      /* Initialize blacklist. */
      blacklist_init();

      /* Remount. */
      for ( int i = n - 1; i >= 0; i-- ) /* Reverse order as we prepend. */
         PHYSFS_mount( plugins[i].mountpoint, NULL, 0 );

      if ( n > 0 ) {
         DEBUG( "Loaded plugins:" );
         for ( int i = 0; i < n; i++ ) { /* Reverse order. */
            DEBUG( "   %s", plugin_name( &plugins[i] ) );
         }
      }
   } else {
      nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins" );
      nfile_dirMakeExist( buf );
   }

   return 0;
}

/**
 * @brief Inserts a plugin to the list, but does not properly enable it
 * (requires restart).
 */
void plugin_insert( plugin_t *plg )
{
   array_push_back( &plugins, *plg );
}

/**
 * @brief Exits the plugin stuff.
 */
void plugin_exit( void )
{
   for ( int i = 0; i < array_size( plugins ); i++ )
      plugin_free( &plugins[i] );
   array_free( plugins );
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
   if ( plg->name != NULL )
      return plg->name;
   return plg->mountpoint;
}

/**
 * @brief Frees a previously allocated plugin.
 */
void plugin_free( plugin_t *plg )
{
   free( plg->name );
   free( plg->author );
   free( plg->version );
   free( plg->description );
   free( plg->compatibility );
   free( plg->source );
   free( plg->mountpoint );
}

/**
 * @brief Checks to see if the plugins are self-declared compatible with Naev.
 *
 *    @return Number of incompatible plugins.
 */
int plugin_check( void )
{
   int         failed  = 0;
   const char *version = naev_version( 0 );
   for ( int i = 0; i < array_size( plugins ); i++ ) {
      plugin_t *plg = &plugins[i];

      /* Check deprecated compatibility first. */
      if ( plg->compatibility != NULL ) {
         int               errornumber;
         PCRE2_SIZE        erroroffset;
         pcre2_match_data *match_data;
         pcre2_code       *re = pcre2_compile( (PCRE2_SPTR)plg->compatibility,
                                               PCRE2_ZERO_TERMINATED, 0, &errornumber,
                                               &erroroffset, NULL );
         if ( re == NULL ) {
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message( errornumber, buffer, sizeof( buffer ) );
            WARN( _( "Plugin '%s' PCRE2 compilation failed at offset %d: %s" ),
                  plugin_name( plg ), (int)erroroffset, buffer );
            failed++;
            continue;
         }

         match_data = pcre2_match_data_create_from_pattern( re, NULL );
         int rc = pcre2_match( re, (PCRE2_SPTR)version, strlen( version ), 0, 0,
                               match_data, NULL );
         pcre2_match_data_free( match_data );
         if ( rc < 0 ) {
            switch ( rc ) {
            case PCRE2_ERROR_NOMATCH:
               WARN( "Plugin '%s' does not support Naev version '%s'.",
                     plugin_name( plg ), version );
               failed++;
               break;
            default:
               WARN( _( "Matching error %d" ), rc );
               failed++;
               break;
            }
         } else
            plg->compatible = 1;

         pcre2_code_free( re );
      }

      /* New more proper way to test version. */
      if ( plg->naev_version ) {
         int compat = naev_versionMatchReq( version, plg->naev_version );
         if ( !compat ) {
            WARN( "Plugin '%s' does not support Naev version '%s'.",
                  plugin_name( plg ), version );
            failed++;
         }
         plg->compatible = compat;
      }
   }

   return failed;
}

/**
 * @brief Returns the list of all the plugins.
 *
 *    @return List of the loaded plugins.
 */
const plugin_t *plugin_list( void )
{
   return plugins;
}
