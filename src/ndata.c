/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ndata.c
 *
 * @brief Wrapper to handle reading/writing the ndata file.
 *
 * Optimizes to minimize the opens and frees.
 *
 * Detection in a nutshell:
 *
 * -- DONE AT INIT --
 *  1) CLI option
 *  2) conf.lua option
 * -- DONE AS NEEDED --
 *  3) Current dir laid out (debug only)
 *  4) Compile time defined path
 *  5) dirname(argv[0])/ndata* (binary path)
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
#include "SDL_mutex.h"

#include "naev.h"
/** @endcond */

#include "ndata.h"

#include "array.h"
#include "attributes.h"
#include "conf.h"
#include "env.h"
#if HAS_MACOS
#include "glue_macos.h"
#endif /* HAS_MACOS */
#include "log.h"
#include "nfile.h"
#include "npng.h"
#include "nstring.h"
#include "nxml.h"
#include "start.h"


#define NDATA_PATHNAME  "dat" /**< Generic ndata file name. */

#define NDATA_SRC_CWD     0 /**< Current working directory. (debug builds only) */
#define NDATA_SRC_USER    1 /**< User defined directory. */
#define NDATA_SRC_DEFAULT 2 /**< Default derectory. (Set at compile time) */
#define NDATA_SRC_BINARY  3 /**< Next to the Naev binary */

#if DEBUGGING
#define NDATA_SRC_SEARCH_START NDATA_SRC_CWD
#else /* DEBUGGING */
#define NDATA_SRC_SEARCH_START NDATA_SRC_USER
#endif /* DEBUGGING */


/*
 * ndata directory.
 */
static char      *ndata_dir        = NULL; /**< ndata directory name. */
static SDL_mutex *ndata_lock       = NULL; /**< Lock for ndata creation. */
static int        ndata_source     = NDATA_SRC_SEARCH_START;


/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static int ndata_isndata( const char *path );
static int ndata_enumerateCallback( void* data, const char* origdir, const char* fname );


/**
 * @brief Sets the current ndata path to use.
 *
 * Should be called before any function that accesses or reads from ndata.
 *
 *    @param path Path to set.
 *    @return 0 on success.
 */
int ndata_setPath( const char *path )
{
   int len;
   char  buf[ PATH_MAX ];

   free( ndata_dir );
   ndata_dir = NULL;

   if ( path != NULL && ndata_isndata( path ) ) {
      len            = strlen( path );
      ndata_dir      = strdup( path );
      if ( nfile_isSeparator( ndata_dir[ len - 1 ] ) )
         ndata_dir[ len - 1 ] = '\0';
      ndata_source = NDATA_SRC_USER;
   }
   else {
      ndata_source = NDATA_SRC_SEARCH_START;

      switch ( ndata_source ) {
      case NDATA_SRC_CWD:
         if ( ndata_isndata( NDATA_PATHNAME ) ) {
            ndata_dir    = strdup( NDATA_PATHNAME );
            ndata_source = NDATA_SRC_CWD;
            break;
         }
         FALLTHROUGH;
      case NDATA_SRC_USER:
         // This already didn't work out when we checked the provided path.
      case NDATA_SRC_DEFAULT:
#if HAS_MACOS
         if ( macos_isBundle() && macos_resourcesPath( buf, PATH_MAX ) >= 0 ) {
            len = strlen( buf ) + 1 + strlen(NDATA_PATHNAME);
            ndata_dir    = malloc(len+1);
            nsnprintf( ndata_dir, len+1, "%s/%s", buf, NDATA_PATHNAME );
            ndata_source = NDATA_SRC_DEFAULT;
            break;
         }
#endif /* HAS_MACOS */
         if ( env.isAppImage && nfile_concatPaths( buf, PATH_MAX, env.appdir, PKGDATADIR, NDATA_PATHNAME ) >= 0 && ndata_isndata( buf ) ) {
            ndata_dir    = strdup( buf );
            ndata_source = NDATA_SRC_DEFAULT;
            break;
         }
         nfile_concatPaths( buf, PATH_MAX, PKGDATADIR, NDATA_PATHNAME );
         if ( ndata_isndata( buf ) ) {
            ndata_dir    = strdup( buf );
            ndata_source = NDATA_SRC_DEFAULT;
            break;
         }
         FALLTHROUGH;
      case NDATA_SRC_BINARY:
         nfile_concatPaths( buf, PATH_MAX, nfile_dirname(naev_binary()), NDATA_PATHNAME );
         if ( ndata_isndata( buf ) ) {
            ndata_dir    = strdup( buf );
            ndata_source = NDATA_SRC_BINARY;
            break;
         }
         FALLTHROUGH;
      default:
         // Couldn't find ndata
         return -1;
      }
   }

   if( PHYSFS_mount( ndata_dir, NULL, 0 ) == 0 ) {
      WARN( "PhysicsFS mount failed: %s",
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return -1;
   }
   LOG( _( "Found ndata: %s" ), ndata_dir );
   ndata_testVersion();

   return 0;
}


/**
 * @brief Checks to see if a directory is an ndata.
 */
static int ndata_isndata( const char *dir )
{
   if ( dir == NULL )
      return 0;

   /* File must exist. */
   if ( !nfile_dirExists( dir ) )
      return 0;

   /* Verify that the directory contains dat/start.xml
    * This is arbitrary, but it's one of the many hard-coded files that must
    * be present for Naev to run.
    */
   if ( !nfile_fileExists( dir, START_DATA_PATH ) )
      return 0;

   return 1;
}


/**
 * @brief Test version to see if it matches.
 */
static void ndata_testVersion (void)
{
   size_t i, size;
   char *buf, cbuf[PATH_MAX];
   int diff;

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
   /* Create the lock. */
   ndata_lock = SDL_CreateMutex();

   /* Set path to configuration. */
   ndata_setPath(conf.ndata);
   /*
   if (ndata_setPath(conf.ndata))
      ERR(_("Couldn't find ndata"));
   */

   return 0;
}


/**
 * @brief Closes and cleans up the ndata directory.
 */
void ndata_close (void)
{
   PHYSFS_deinit();
   free( ndata_dir );
   ndata_dir = NULL;

   /* Destroy the lock. */
   if (ndata_lock != NULL) {
      SDL_DestroyMutex(ndata_lock);
      ndata_lock = NULL;
   }
}


/**
 * @brief Gets the ndata's name.
 *
 *    @return The ndata's name.
 */
const char* ndata_name (void)
{
   return start_name();
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
