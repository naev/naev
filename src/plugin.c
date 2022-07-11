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

static plugin_t *plugins;

static const char *plugin_name( plugin_t *plg );

static int plugin_parse( plugin_t *plg, const char *file )
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

   node   = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      xmlr_strd( node, "name", plg->name );
      xmlr_strd( node, "author", plg->author );
      xmlr_strd( node, "version", plg->version );
      xmlr_strd( node, "description", plg->description );
      xmlr_strd( node, "compatibility", plg->compatibility );
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);

   return 0;
}

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

         PHYSFS_stat( "plugin.xml", &stat );
         realdir = PHYSFS_getRealDir( "plugin.xml" );
         if ((stat.filetype == PHYSFS_FILETYPE_REGULAR) &&
               realdir && strcmp(realdir,buf)==0)
            plugin_parse( plg, "plugin.xml" );
      }
      PHYSFS_freeList(files);

      n = array_size(plugins);
      if (n > 0) {
         DEBUG("Loaded plugins:");
         for (int i=0; i<n; i++) {
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

void plugin_exit (void)
{
   array_free(plugins);
   plugins = NULL;
}

static const char *plugin_name( plugin_t *plg )
{
   if (plg->name != NULL)
      return plg->name;
   return plg->mountpoint;
}

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

const plugin_t *plugin_list (void)
{
   for (int i=0; i<array_size(plugins); i++) {
      plugin_t *plg = &plugins[i];
      free( plg->name );
      free( plg->author );
      free( plg->version );
      free( plg->description );
      free( plg->compatibility );
      free( plg->mountpoint );
   }
   return plugins;
}
