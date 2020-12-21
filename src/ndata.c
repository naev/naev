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
static int        ndata_loadedfile = 0;    /**< Already loaded a file? */
static int        ndata_source     = NDATA_SRC_SEARCH_START;


/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static int ndata_isndata( const char *path );


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
 * @brief Get the current ndata path.
 */
const char *ndata_getPath( void )
{
   return ndata_dir;
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
   if (ndata_setPath(conf.ndata))
      ERR(_("Couldn't find ndata"));

   return 0;
}


/**
 * @brief Closes and cleans up the ndata directory.
 */
void ndata_close (void)
{
   if( PHYSFS_unmount( ndata_dir ) == 0 )
      WARN( "PhysicsFS unmount failed: %s",
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
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
 *    @param filename Name of the file to read.
 *    @param[out] filesize Stores the size of the file.
 *    @return The file data or NULL on error.
 */
void* ndata_read( const char* filename, size_t *filesize )
{
   char *buf;

   buf = nfile_readFile( filesize, ndata_dir, filename );
   if ( buf != NULL ) {
      ndata_loadedfile = 1;
      return buf;
   }

   /* Wasn't able to open the file. */
   WARN( _( "Unable to open file '%s': not found." ), filename );
   *filesize = 0;
   return NULL;
}


/**
 * @brief Creates an rwops from a file in the ndata.
 *
 *    @param filename Name of the file to create rwops of.
 *    @return rwops that accesses the file in the ndata.
 */
SDL_RWops *ndata_rwops( const char* filename )
{
   char       path[ PATH_MAX ];
   SDL_RWops *rw;

   if ( nfile_concatPaths( path, PATH_MAX, ndata_dir, filename ) < 0 ) {
      WARN( _( "Unable to open file '%s': file path too long." ), filename );
      return NULL;
   }

   rw = SDL_RWFromFile( path, "rb" );
   if ( rw != NULL ) {
      ndata_loadedfile = 1;
      return rw;
   }

   /* Wasn't able to open the file. */
   WARN( _( "Unable to open file '%s': not found." ), filename );
   return NULL;
}


/**
 * @brief Gets a list of files in the ndata below a certain path.
 *
 *    @sa nfile_readDirRecursive
 */
char **ndata_listRecursive( const char *path )
{
   return nfile_readDirRecursive( ndata_dir, path );
}
