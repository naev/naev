/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ndata.c
 *
 * @brief Wrappers to set up and access game assets (mounted via PhysicsFS).
 *        The main search for data takes place in ndata_open().
 *        However, conf.c code may have seeded the search path based on command-line arguments.
 */

/** @cond */
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#if HAS_WIN32
#include <windows.h>
#endif /* HAS_WIN32 */

#include "physfs.h"
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "ndata.h"

#include "array.h"
#include "conf.h"
#include "env.h"
#if HAS_MACOS
#include "glue_macos.h"
#endif /* HAS_MACOS */
#include "log.h"
#include "nfile.h"
#include "nstring.h"


/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static int ndata_found (void);
static int ndata_enumerateCallback( void* data, const char* origdir, const char* fname );


/**
 * @brief Checks to see if the physfs search path is enough to find game data.
 */
static int ndata_found( void )
{
   /* Verify that we can find VERSION and start.xml.
    * This is arbitrary, but these are among the hard dependencies to self-identify and start.
    */
   return PHYSFS_exists( "VERSION" ) && PHYSFS_exists( START_DATA_PATH );
}


/**
 * @brief Test version to see if it matches.
 */
static void ndata_testVersion (void)
{
   size_t i, size;
   char *buf, cbuf[PATH_MAX];
   int diff;

   if (!ndata_found())
      ERR( _("Unable to find game data. You may need to install, specify a datapath, or run using naev.sh (if developing).") );

   /* Parse version. */
   buf = ndata_read( "VERSION", &size );
   for (i=0; i<MIN(size,PATH_MAX-1); i++)
      cbuf[i] = buf[i];
   cbuf[MIN(size-1,PATH_MAX-1)] = '\0';
   diff = naev_versionCompare( cbuf );
   if (diff != 0) {
      WARN( _("ndata version inconsistency with this version of Naev!") );
      WARN( _("Expected ndata version %s got %s."), VERSION, cbuf );
      if (ABS(diff) > 2)
         ERR( _("Please get a compatible ndata version!") );
      if (ABS(diff) > 1)
         WARN( _("Naev will probably crash now as the versions are probably not compatible.") );
   }
   free( buf );
}


/**
 * @brief Opens the ndata directory.
 *
 *    @return 0 on success.
 */
int ndata_open (void)
{
   char buf[ PATH_MAX ];

   if ( conf.ndata != NULL && PHYSFS_mount( conf.ndata, NULL, 1 ) )
      LOG(_("Added datapath from conf.lua file: %s"), conf.ndata);

#if HAS_MACOS
   if ( !ndata_found() && macos_isBundle() && macos_resourcesPath( buf, PATH_MAX-4 ) >= 0 && strncat( buf, "/dat", 4 ) ) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }
#endif /* HAS_MACOS */

   if ( !ndata_found() && env.isAppImage && nfile_concatPaths( buf, PATH_MAX, env.appdir, PKGDATADIR, "dat" ) >= 0 ) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   if (!ndata_found() && nfile_concatPaths( buf, PATH_MAX, PKGDATADIR, "dat" ) >= 0) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   if (!ndata_found() && nfile_concatPaths( buf, PATH_MAX, PHYSFS_getBaseDir(), "dat" ) >= 0) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   ndata_testVersion();
   return 0;
}


/**
 * @brief Closes and cleans up the ndata directory.
 */
void ndata_close (void)
{
   PHYSFS_deinit();
}


/**
 * @brief Reads a file from the ndata.
 *
 *    @param path Path of the file to read.
 *    @param[out] filesize Stores the size of the file.
 *    @return The file data or NULL on error.
 */
void* ndata_read( const char* path, size_t *filesize )
{
   char *buf;
   PHYSFS_file *file;
   PHYSFS_sint64 len, n;
   size_t pos;
   PHYSFS_Stat path_stat;

   if (!PHYSFS_stat( path, &path_stat )) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      *filesize = 0;
      return NULL;
   }
   if (path_stat.filetype != PHYSFS_FILETYPE_REGULAR) {
      WARN( _( "Error occurred while opening '%s': It is not a regular file" ), path );
      *filesize = 0;
      return NULL;
   }

   /* Open file. */
   file = PHYSFS_openRead( path );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. TODO: Don't assume this is always possible? */
   len = PHYSFS_fileLength( file );
   if ( len == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len+1 );
   if (buf == NULL) {
      WARN(_("Out of Memory"));
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while ( n < len ) {
      pos = PHYSFS_readBytes( file, &buf[ n ], len - n );
      if ( pos <= 0 ) {
         WARN( _( "Error occurred while reading '%s': %s" ), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
         PHYSFS_close( file );
         *filesize = 0;
         free(buf);
         return NULL;
      }
      n += pos;
   }

   /* Close the file. */
   PHYSFS_close(file);

   *filesize = len;
   return buf;
}


/**
 * @brief Lists all the visible files in a directory, at any depth.
 *
 * Will sort by path, and (unlike underlying PhysicsFS) make sure to list each file path only once.
 *
 *    @return Array of (allocated) file paths relative to base_dir.
 */
char **ndata_listRecursive( const char *path )
{
   char **files;
   int i;

   files = array_create( char * );
   PHYSFS_enumerate( path, ndata_enumerateCallback, &files );
   /* Ensure unique. PhysicsFS can enumerate a path twice if it's in multiple components of a union. */
   qsort( files, array_size(files), sizeof(char*), strsort );
   for (i=0; i+1<array_size(files); i++)
      if (strcmp(files[i], files[i+1]) == 0) {
         free( files[i] );
         array_erase( &files, &files[i], &files[i+1] );
	 i--; /* We're not done checking for dups of files[i]. */
      }
   return files;
}

/**
 * @brief The PHYSFS_EnumerateCallback for ndata_listRecursive
 */
static int ndata_enumerateCallback( void* data, const char* origdir, const char* fname )
{
   char *path;
   const char *fmt;
   size_t dir_len, path_size;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   path_size = dir_len + strlen( fname ) + 2;
   path = malloc( path_size );
   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   nsnprintf( path, path_size, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat )) {
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      free( path );
   }
   else if (stat.filetype == PHYSFS_FILETYPE_REGULAR)
      array_push_back( (char***)data, path );
   else if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY ) {
      PHYSFS_enumerate( path, ndata_enumerateCallback, data );
      free( path );
   }
   else
      free( path );
   return PHYSFS_ENUM_OK;
}
