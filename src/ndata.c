/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ndata.c
 *
 * @brief Wrapper to handle reading/writing the ndata file.
 *
 * Optimizes to minimize the opens and frees, plus tries to read from the
 *  filesystem instead always looking for a ndata archive.
 *
 * Detection in a nutshell:
 *
 * -- DONE AT INIT --
 *  1) CLI option
 *  2) conf.lua option
 * -- DONE AS NEEDED --
 *  3) Current dir laid out (does not work well when iterating through directories)
 *  4) ndata-$VERSION
 *  5) Makefile version
 *  6) ./ndata*
 *  7) dirname(argv[0])/ndata* (binary path)
 */

#include "ndata.h"

#include "naev.h"

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

#include "SDL.h"
#include "SDL_mutex.h"

#include "log.h"
#include "nxml.h"
#include "nfile.h"
#include "conf.h"
#include "npng.h"
#include "nstring.h"
#include "start.h"


#define NDATA_FILENAME  "ndata" /**< Generic ndata file name. */
#ifndef NDATA_DEF
#define NDATA_DEF       NDATA_FILENAME /**< Default ndata to use. */
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
 * ndata archive.
 */
static char* ndata_filename         = NULL; /**< ndata archive name. */
static SDL_mutex *ndata_lock        = NULL; /**< Lock for ndata creation. */
static int ndata_loadedfile         = 0;    /**< Already loaded a file? */
static int ndata_source             = NDATA_SRC_SEARCH_START;


/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static char *ndata_findInDir( const char *path );
static int ndata_isndata( const char *path );
static int ndata_prompt( void *data );
static int ndata_notfound (void);


/**
 * @brief Sets the current ndata path to use.
 *
 * Should be called before ndata_open.
 *
 *    @param path Path to set.
 *    @return 0 on success.
 */
int ndata_setPath( const char *path )
{
   int len;
   char *buf;

   if ( ndata_filename != NULL ) {
      free( ndata_filename );
      ndata_filename = NULL;
   }

   if ( path != NULL && ndata_isndata( path ) ) {
      len            = strlen( path );
      ndata_filename = strdup( path );
      if ( nfile_isSeparator( ndata_filename[ len - 1 ] ) )
         ndata_filename[ len - 1 ] = '\0';
      ndata_source = NDATA_SRC_USER;
   }
   else {
      ndata_source = NDATA_SRC_SEARCH_START;

      switch ( ndata_source ) {
      case NDATA_SRC_CWD:
         ndata_filename = ndata_findInDir( "." );
         if ( ndata_filename != NULL ) {
            ndata_source = NDATA_SRC_CWD;
            break;
         }
         __attribute__( ( fallthrough ) );
      case NDATA_SRC_USER:
         // This already didn't work out when we checked the provided path.
      case NDATA_SRC_DEFAULT:
         if ( ndata_isndata( NDATA_DEF ) ) {
            ndata_filename = strdup( NDATA_DEF );
            ndata_source   = NDATA_SRC_DEFAULT;
            break;
         }
         __attribute__( ( fallthrough ) );
      case NDATA_SRC_BINARY:
         buf            = strdup( naev_binary() );
         ndata_filename = ndata_findInDir( nfile_dirname( buf ) );
         free( buf );
         if ( ndata_filename != NULL ) {
            ndata_source = NDATA_SRC_BINARY;
            break;
         }
         __attribute__( ( fallthrough ) );
      default:
         // Couldn't find ndata
         return -1;
      }
   }

   ndata_testVersion();

   return 0;
}


/**
 * @brief Get the current ndata path.
 */
const char* ndata_getPath (void)
{
   return ndata_filename;
}


static int ndata_prompt( void *data )
{
   int ret;

   ret = SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, _("Missing Data"),
         _("Ndata could not be found. If you have the ndata file, drag\n"
         "and drop it onto the 'NAEV - INSERT NDATA' window.\n\n"
         "If you don't have the ndata, download it from naev.org"), (SDL_Window*)data );

   return ret;
}


#define NONDATA
#include "nondata.c"
/**
 * @brief Displays an ndata not found message and dies.
 */
static int ndata_notfound (void)
{
   SDL_Surface *screen;
   SDL_Event event;
   SDL_Surface *sur;
   SDL_RWops *rw;
   npng_t *npng;
   const char *title = _("NAEV - INSERT NDATA");
   int found;

   /* Make sure it's initialized. */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
      WARN(_("Unable to init SDL Video subsystem"));
      return 0;
   }

   /* Create the window. */
   SDL_Window *window;
   SDL_Renderer *renderer;
   window = SDL_CreateWindow( title,
         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
         320, 240, SDL_WINDOW_SHOWN);
   renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_SOFTWARE );
   screen = SDL_GetWindowSurface( window );

   /* Create the surface. */
   rw    = SDL_RWFromConstMem( nondata_png, sizeof(nondata_png) );
   npng  = npng_open( rw );
   sur   = npng_readSurface( npng, 0, 0 );
   npng_close( npng );
   SDL_RWclose( rw );

   /* Render. */
   SDL_BlitSurface( sur, NULL, screen, NULL );
   SDL_EventState( SDL_DROPFILE, SDL_ENABLE );
#if SDL_VERSION_ATLEAST(2,0,2)
   SDL_Thread *thread = SDL_CreateThread( &ndata_prompt, "Prompt", window );
   SDL_DetachThread(thread);
#else /* SDL_VERSION_ATLEAST(2,0,2) */
   /* Ignore return value because SDL_DetachThread is only present in
    * SDL >= 2.0.2 */
   SDL_CreateThread( &ndata_prompt, "Prompt", window );
#endif /* SDL_VERSION_ATLEAST(2,0,2) */

   /* TODO substitute. */
   SDL_RenderPresent( renderer );

   found = 0;

   /* Infinite loop. */
   while (1) {
      SDL_WaitEvent(&event);

      /* Listen on a certain amount of events. */
      if (event.type == SDL_QUIT)
         exit(1);
      else if (event.type == SDL_KEYDOWN) {
         switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_q:
               exit(1);

            default:
               break;
         }
      }
      else if (event.type == SDL_DROPFILE) {
         found = ndata_isndata( event.drop.file );
         if (found) {
            ndata_setPath( event.drop.file );

            /* Minor hack so ndata filename is saved in conf.lua */
            conf.ndata = strdup( event.drop.file );
            free( event.drop.file );
            break;
         }
         else
            free( event.drop.file );
      }

      /* Render. */
      SDL_BlitSurface( sur, NULL, screen, NULL );
      /* TODO substitute. */
      SDL_RenderPresent( renderer );
   }

   SDL_EventState( SDL_DROPFILE, SDL_DISABLE );
   SDL_DestroyWindow(window);

   return found;
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

   /* Verify that the zip contains dat/start.xml
    * This is arbitrary, but it's one of the many hard-coded files that must
    * be present for Naev to run.
    */
   if ( !nfile_fileExists( dir, START_DATA_PATH ) )
      return 0;

   return 1;
}


/**
 * @brief Tries to find a valid ndata archive in the directory listed by path.
 *
 *    @return Newly allocated ndata name or NULL if not found.
 */
static char *ndata_findInDir( const char *path )
{
   size_t i;
   int l;
   char **files;
   size_t nfiles;
   size_t len;
   char *ndata_file;

   /* Defaults. */
   ndata_file = NULL;

   /* Iterate over files. */
   files = nfile_readDir( &nfiles, path );
   if (files != NULL) {
      len   = strlen(NDATA_FILENAME);
      for (i=0; i<nfiles; i++) {

         /* Didn't match. */
         if (strncmp(files[i], NDATA_FILENAME, len)!=0)
            continue;

         /* Formatting. */
         l           = strlen(files[i]) + strlen(path) + 2;
         ndata_file  = malloc( l );
         nsnprintf( ndata_file, l, "%s/%s", path, files[i] );

         /* Must be zip file. */
         if ( !ndata_isndata( ndata_file ) ) {
            free(ndata_file);
            ndata_file = NULL;
            continue;
         }

         /* Found it. */
         break;
      }

      /* Clean up. */
      for (i=0; i<nfiles; i++)
         free(files[i]);
      free(files);
   }

   return ndata_file;
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
 * @brief Opens the ndata file.
 *
 *    @return 0 on success.
 */
int ndata_open (void)
{
   /* Create the lock. */
   ndata_lock = SDL_CreateMutex();

   /* Set path to configuration. */
   ndata_setPath(conf.ndata);

   return 0;
}


/**
 * @brief Closes and cleans up the ndata file.
 */
void ndata_close (void)
{
   if ( ndata_filename != NULL ) {
      free( ndata_filename );
      ndata_filename = NULL;
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
int ndata_exists( const char* filename )
{
   return nfile_fileExists( ndata_filename, filename );
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

   buf = nfile_readFile( filesize, ndata_filename, filename );
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

   if ( nfile_concatPaths( path, PATH_MAX, ndata_filename, filename ) ) {
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
char** ndata_list( const char* path, size_t* nfiles )
{
   return nfile_readDir( nfiles, ndata_filename, path );
}


/**
 * @brief Gets a list of files in the ndata below a certain path.
 *
 *    @sa nfile_readDirRecursive
 */
char **ndata_listRecursive( const char *path )
{
   return nfile_readDirRecursive( ndata_filename, path );
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



