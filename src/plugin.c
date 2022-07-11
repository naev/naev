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

#include "log.h"
#include "array.h"
#include "nfile.h"

static plugin_t *plugins;

int plugin_init (void)
{
   char buf[ PATH_MAX ];
   PHYSFS_Stat stat;
   PHYSFS_stat( "plugins", &stat );
   if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
      char **files = PHYSFS_enumerateFiles( "plugins" );
      for (char **f = files; *f != NULL; f++) {
         const char *realdir;
         nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins", *f );
         if (!nfile_fileExists( buf ) && !nfile_dirExists( buf ))
            continue;

         if (PHYSFS_mount( buf, NULL, 0 )==0) {
            WARN(_("Failed to mount plugin '%s': %s"), buf,
                  _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode()) ));
         }

         PHYSFS_stat( "mod.xml", &stat );
         realdir = PHYSFS_getRealDir( "mod.xml" );
         if ((stat.filetype == PHYSFS_FILETYPE_REGULAR) &&
               realdir && strcmp(realdir,buf)==0)
            continue;
      }
      PHYSFS_freeList(files);
   }
   else {
      nfile_concatPaths( buf, PATH_MAX, PHYSFS_getWriteDir(), "plugins" );
      nfile_dirMakeExist( buf );
   }
   return 0;
}

const plugin_t *plugin_list (void)
{
   return plugins;
}
