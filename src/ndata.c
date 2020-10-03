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

#include "ndata.h"

#include "naev.h"
#include <limits.h>
#include <stdlib.h>

#if HAS_POSIX
#include <libgen.h>
#endif /* HAS_POSIX */
#if HAS_WIN32
#include <windows.h>
#endif /* HAS_WIN32 */
#if HAS_MACOS
#include "glue_macos.h"
#endif /* HAS_MACOS */
#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "SDL_mutex.h"

#include "conf.h"
#include "env.h"
#include "log.h"
#include "nfile.h"
#include "npng.h"
#include "nstring.h"
#include "nxml.h"
#include "start.h"


#define NDATA_PATHNAME  "dat" /**< Generic ndata file name. */
#ifndef NDATA_DEF
#define NDATA_DEF       NDATA_PATHNAME /**< Default ndata to use. */
#endif /* NDATA_DEF */


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
   char *pathBuf;
   char *dirnameBuf;

   if ( ndata_dir != NULL ) {
      free( ndata_dir );
      ndata_dir = NULL;
   }

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
         if (ndata_isndata( "." )) {
            ndata_dir    = strdup( "." );
            ndata_source = NDATA_SRC_CWD;
            break;
         }
         __attribute__( ( fallthrough ) );
      case NDATA_SRC_USER:
         // This already didn't work out when we checked the provided path.
      case NDATA_SRC_DEFAULT:
         if ( env.isAppImage && nfile_concatPaths( buf, PATH_MAX, env.appdir, NDATA_DEF ) >= 0 && ndata_isndata( buf ) ) {
            ndata_dir    = strdup( buf );
            ndata_source = NDATA_SRC_DEFAULT;
            break;
         }
         if ( ndata_isndata( NDATA_DEF ) ) {
            ndata_dir    = strdup( NDATA_DEF );
            ndata_source = NDATA_SRC_DEFAULT;
            break;
         }
         __attribute__( ( fallthrough ) );
      case NDATA_SRC_BINARY:
         pathBuf    = strdup( naev_binary() );
         dirnameBuf = nfile_dirname( pathBuf );
         nfile_concatPaths( buf, PATH_MAX, dirnameBuf, NDATA_PATHNAME );
         free( pathBuf );
         dirnameBuf = NULL;
         if ( ndata_isndata( buf ) ) {
            ndata_dir    = strdup( buf );
            ndata_source = NDATA_SRC_BINARY;
            break;
         }
         __attribute__( ( fallthrough ) );
      default:
         // Couldn't find ndata
         return -1;
      }
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

   /* Verify that the folder contains dat/start.xml
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
   int ret;
   size_t size;
   int version[3];
   char *buf;
   int diff;

   /* Parse version. */
   buf = ndata_read( "VERSION", &size );
   ret = naev_versionParse( version, buf, (int)size );
   free(buf);
   if (ret != 0) {
      WARN(_("Problem reading VERSION file from ndata!"));
      return;
   }

   diff = naev_versionCompare( version );
   if (diff != 0) {
      WARN( _("ndata version inconsistancy with this version of Naev!") );
      WARN( _("Expected ndata version %d.%d.%d got %d.%d.%d."),
            VMAJOR, VMINOR, VREV, version[0], version[1], version[2] );

      if (ABS(diff) > 2)
         ERR( _("Please get a compatible ndata version!") );

      if (ABS(diff) > 1)
         WARN( _("Naev will probably crash now as the versions are probably not compatible.") );
   }
}


/**
 * @brief Opens the ndata folder.
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
 * @brief Closes and cleans up the ndata folder.
 */
void ndata_close (void)
{
   if ( ndata_dir != NULL ) {
      free( ndata_dir );
      ndata_dir = NULL;
   }

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
 * @brief Checks to see if a file is in the NDATA.
 *    @param filename Name of the file to check.
 *    @return 1 if the file exists, 0 otherwise.
 */
int ndata_exists( const char *filename )
{
   return nfile_fileExists( ndata_dir, filename );
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
 * @brief Gets a list of files in the ndata that are direct children of a path.
 *
 *    @sa nfile_readDir
 */
char **ndata_list( const char *path, size_t *nfiles )
{
   return nfile_readDir( nfiles, ndata_dir, path );
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


/**
 * @brief Small qsort wrapper.
 */
static int ndata_sortFunc( const void *name1, const void *name2 )
{
   const char **f1, **f2;
   f1 = (const char**) name1;
   f2 = (const char**) name2;
   return strcmp( f1[0], f2[0] );
}


/**
 * @brief Sorts the files by name.
 *
 * Meant to be used directly by ndata_list.
 *
 *    @param files Filenames to sort.
 *    @param nfiles Number of files to sort.
 */
void ndata_sortName( char **files, size_t nfiles )
{
   qsort( files, nfiles, sizeof(char*), ndata_sortFunc );
}



