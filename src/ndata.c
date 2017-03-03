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
#include <stdarg.h>

#include "SDL.h"
#include "SDL_mutex.h"

#include "log.h"
#include "nxml.h"
#include "nzip.h"
#include "nfile.h"
#include "conf.h"
#include "npng.h"
#include "nstring.h"
#include "start.h"


#define NDATA_FILENAME  "ndata" /**< Generic ndata file name. */
#ifndef NDATA_DEF
#define NDATA_DEF       NDATA_FILENAME /**< Default ndata to use. */
#endif /* NDATA_DEF */


#define NDATA_SRC_LAIDOUT        0
#define NDATA_SRC_DIRNAME        1
#define NDATA_SRC_NDATADEF       2
#define NDATA_SRC_BINARY         3


/*
 * ndata archive.
 */
static char* ndata_filename         = NULL; /**< ndata archive name. */
static char* ndata_dirname          = NULL; /**< Directory name. */
static struct zip* ndata_archive    = NULL; /**< ndata file on disk */
static char* ndata_arcName          = NULL; /**< Name of the ndata module. */
static SDL_mutex *ndata_lock        = NULL; /**< Lock for ndata creation. */
static int ndata_loadedfile         = 0; /**< Already loaded a file? */
static int ndata_source             = 0;

/*
 * File list.
 */
static char **ndata_fileList  = NULL; /**< List of files in the archive. */
static uint32_t ndata_fileNList     = 0; /**< Number of files in ndata_fileList. */


/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static char *ndata_findInDir( const char *path );
static int ndata_openFile (void);
static int ndata_isndata( const char *path, ... );
#if SDL_VERSION_ATLEAST(2,0,0)
static int ndata_prompt( void *data );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
static int ndata_notfound (void);
static char** ndata_listBackend( const char* path, uint32_t* nfiles, int dirs );
static char **stripPath( const char **list, int nlist, const char *path );
static char** filterList( const char** list, int nlist,
      const char* path, uint32_t* nfiles, int recursive );


/**
 * @brief Checks to see if path is a ndata file.
 *
 * Should be called before ndata_open.
 *
 *    @param path Path to check to see if it's an ndata file.
 *    @return 1 if it is an ndata file, 0 else.
 */
int ndata_check( const char* path )
{
   return nzip_isZip( path );
}


/**
 * @brief Sets the current ndata path to use.
 *
 * Should be called before ndata_open.
 *
 *    @param path Path to set.
 *    @return 0 on success.
 */
int ndata_setPath( const char* path )
{
   int len;

   free(ndata_filename);
   free(ndata_dirname);
   ndata_filename = NULL;
   ndata_dirname  = NULL;

   if (path == NULL)
      return 0;
   else if (nfile_dirExists(path)) {
      len = strlen(path);
      ndata_dirname = strdup(path);
      if (ndata_dirname[len - 1] == '/')
         ndata_dirname[len - 1] = '\0';
   }
   else if (nfile_fileExists(path)) {
      char *tmp = strdup(path);
      ndata_filename = strdup(path);
      ndata_dirname  = nfile_dirname(tmp);
   }
   return 0;
}


/**
 * @brief Get the current ndata path.
 */
const char* ndata_getPath (void)
{
   return ndata_filename;
}


#if SDL_VERSION_ATLEAST(2,0,0)
static int ndata_prompt( void *data )
{
   int ret;

   ret = SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Missing Data",
         "Ndata could not be found. If you have the ndata file, drag\n"
         "and drop it onto the 'NAEV - INSERT NDATA' window.\n\n"
         "If you don't have the ndata, download it from naev.org", (SDL_Window*)data );

   return ret;
}
#endif /* SDL_VERSION_ATLEAST(2,0,0) */


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
   const char *title = "NAEV - INSERT NDATA";
   int found;

   /* Make sure it's initialized. */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
      WARN("Unable to init SDL Video subsystem");
      return 0;
   }

   /* Create the window. */
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_Window *window;
   SDL_Renderer *renderer;
   window = SDL_CreateWindow( title,
         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
         320, 240, SDL_WINDOW_SHOWN);
   renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_SOFTWARE );
   screen = SDL_GetWindowSurface( window );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   screen = SDL_SetVideoMode( 320, 240, 0, SDL_SWSURFACE);
   if (screen == NULL) {
      WARN("Unable to set video mode");
      return 0;
   }

   /* Set caption. */
   SDL_WM_SetCaption( title, "NAEV" );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Create the surface. */
   rw    = SDL_RWFromConstMem( nondata_png, sizeof(nondata_png) );
   npng  = npng_open( rw );
   sur   = npng_readSurface( npng, 0, 0 );
   npng_close( npng );
   SDL_RWclose( rw );

   /* Render. */
   SDL_BlitSurface( sur, NULL, screen, NULL );
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_EventState( SDL_DROPFILE, SDL_ENABLE );
#if SDL_VERSION_ATLEAST(2,0,2)
   SDL_Thread *thread = SDL_CreateThread( &ndata_prompt, "Prompt", window );
   SDL_DetachThread(thread);
#else
   /* Ignore return value because SDL_DetachThread is only present in
    * SDL >= 2.0.2 */
   SDL_CreateThread( &ndata_prompt, "Prompt", window );
#endif /* SDL_VERSION_ATLEAST(2,0,2) */

   /* TODO substitute. */
   SDL_RenderPresent( renderer );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_Flip(screen);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

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
#if SDL_VERSION_ATLEAST(2,0,0)
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
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

      /* Render. */
      SDL_BlitSurface( sur, NULL, screen, NULL );
#if SDL_VERSION_ATLEAST(2,0,0)
      /* TODO substitute. */
      SDL_RenderPresent( renderer );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
      SDL_Flip(screen);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   }

#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_EventState( SDL_DROPFILE, SDL_DISABLE );
   SDL_DestroyWindow(window);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   return found;
}


/**
 * @brief Checks to see if a file is an ndata.
 */
static int ndata_isndata( const char *path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   struct zip *arc;

   if (path == NULL)
      return 0;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* File must exist. */
   if (!nfile_fileExists(file))
      return 0;

   /* Must be ndata. */
   if (!nzip_isZip(file))
      return 0;

   /* Verify that the zip contains dat/start.xml
    * This is arbitrary, but it's one of the many hard-coded files that must
    * be present for Naev to run.
    */
   arc = nzip_open(file);

   if (!nzip_hasFile(arc, START_DATA_PATH)) {
      nzip_close(arc);
      return 0;
   }

   nzip_close(arc);
   return 1;
}


/**
 * @brief Tries to find a valid ndata archive in the directory listed by path.
 *
 *    @return Newly allocated ndata name or NULL if not found.
 */
static char *ndata_findInDir( const char *path )
{
   int i, l;
   char **files;
   int nfiles;
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
         if (!nzip_isZip(ndata_file)) {
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
 * @brief Opens an ndata archive if needed.
 *
 *    @return 0 on success.
 */
static int ndata_openFile (void)
{
   char path[PATH_MAX], *buf;
   char pathname[PATH_MAX];

   /* Must be thread safe. */
   SDL_mutexP(ndata_lock);

   /* Was opened while locked. */
   if (ndata_archive != NULL) {
      SDL_mutexV(ndata_lock);
      return 0;
   }

   /* Check dirname first. */
   if ((ndata_filename == NULL) && (ndata_dirname != NULL))
      ndata_filename = ndata_findInDir( ndata_dirname );

   /*
    * Try to find the ndata file.
    */
   if (ndata_filename == NULL) {

      /* Check ndata with version appended. */
#if VREV < 0
      nsnprintf ( pathname, PATH_MAX, "%s-%d.%d.0-beta%d", NDATA_FILENAME, VMAJOR, VMINOR, ABS ( VREV ) );
#else /* VREV < 0 */
      nsnprintf ( pathname, PATH_MAX, "%s-%d.%d.%d", NDATA_FILENAME, VMAJOR, VMINOR, VREV );
#endif /* VREV < 0 */

      if (ndata_isndata(pathname)) {
         ndata_filename = malloc(PATH_MAX);
         strncpy(ndata_filename, pathname, PATH_MAX);
      }
      else if (ndata_isndata(strncat(pathname, ".zip", PATH_MAX-1))) {
         ndata_filename = malloc(PATH_MAX);
         strncpy(ndata_filename, pathname, PATH_MAX);
      }
      /* Check default ndata. */
      else if (ndata_isndata(NDATA_DEF))
         ndata_filename = strdup(NDATA_DEF);

      /* Try to open any ndata in path. */
      else {
         /* Check in NDATA_DEF path. */
         buf = strdup(NDATA_DEF);
         nsnprintf( path, PATH_MAX, "%s", nfile_dirname( buf ) );
         ndata_filename = ndata_findInDir( path );
         free(buf);

         /* Check in current directory. */
         if (ndata_filename == NULL)
            ndata_filename = ndata_findInDir( "." );

         /* Keep looking. */
         if (ndata_filename == NULL) {
            buf = strdup( naev_binary() );
            nsnprintf( path, PATH_MAX, "%s", nfile_dirname( buf ) );
            ndata_filename = ndata_findInDir( path );
            free(buf);
         }
      }
   }

   /* Open the archive. */
   if (ndata_isndata( ndata_filename ) != 1) {
      if (!ndata_loadedfile) {
         WARN("Cannot find ndata file!");
         WARN("Please run with ndata path suffix or specify in conf.lua.");
         WARN("E.g. naev ~/ndata or data = \"~/ndata\"");

         /* Display the not found message. */
         if (!ndata_notfound())
            exit(1);
      }
      else
         return -1;
   }
   ndata_archive = nzip_open( ndata_filename );
   if (ndata_archive == NULL)
      WARN("Unable to open ndata from '%s'.", ndata_filename );

   /* Close lock. */
   SDL_mutexV(ndata_lock);

   /* Test version. */
   ndata_testVersion();

   return 0;
}


/**
 * @brief Test version to see if it matches.
 */
static void ndata_testVersion (void)
{
   int ret;
   uint32_t size;
   int version[3];
   char *buf;
   int diff;

   /* Parse version. */
   buf = ndata_read( "VERSION", &size );
   ret = naev_versionParse( version, buf, (int)size );
   free(buf);
   if (ret != 0) {
      WARN("Problem reading VERSION file from ndata!");
      return;
   }

   diff = naev_versionCompare( version );
   if (diff != 0) {
      WARN( "ndata version inconsistancy with this version of Naev!" );
      WARN( "Expected ndata version %d.%d.%d got %d.%d.%d.",
            VMAJOR, VMINOR, VREV, version[0], version[1], version[2] );

      if (ABS(diff) > 2)
         ERR( "Please get a compatible ndata version!" );

      if (ABS(diff) > 1)
         WARN( "Naev will probably crash now as the versions are probably not compatible." );
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

   /* If user enforces ndata filename, we'll respect that. */
   if (ndata_isndata(ndata_filename))
      return ndata_openFile();

   free(ndata_filename);
   ndata_filename = NULL;

   return 0;
}


/**
 * @brief Closes and cleans up the ndata file.
 */
void ndata_close (void)
{
   unsigned int i;

   /* Destroy the name. */
   if (ndata_arcName != NULL) {
      free(ndata_arcName);
      ndata_arcName = NULL;
   }

   /* Destroy the list. */
   if (ndata_fileList != NULL) {
      for (i=0; i<ndata_fileNList; i++)
         free(ndata_fileList[i]);

      free(ndata_fileList);
      ndata_fileList  = NULL;
      ndata_fileNList = 0;
   }

   /* Close the archive. */
   if (ndata_archive) {
      nzip_close(ndata_archive);
      ndata_archive = NULL;
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
 * @brief Gets the directory where ndata is loaded from.
 *
 *    @return Directory name that ndata is inside of.
 */
const char* ndata_getDirname(void)
{
   char *path;

   path = (char*)ndata_getPath();
   if (path != NULL)
      return nfile_dirname( path );

   switch (ndata_source) {
      case NDATA_SRC_LAIDOUT:
         return ".";
      case NDATA_SRC_DIRNAME:
         return ndata_dirname;
      case NDATA_SRC_NDATADEF:
         return nfile_dirname( strdup( NDATA_DEF ) );
      case NDATA_SRC_BINARY:
         return nfile_dirname( strdup( naev_binary() ) );
   }

   return NULL;
}


/**
 * @brief Checks to see if a file is in the NDATA.
 *    @param filename Name of the file to check.
 *    @return 1 if the file exists, 0 otherwise.
 */
int ndata_exists( const char* filename )
{
   char *buf, path[PATH_MAX];

   /* See if needs to load ndata archive. */
   if (ndata_archive == NULL) {

      /* Try to read the file as locally. */
      if (nfile_fileExists( filename ) && (ndata_source <= NDATA_SRC_LAIDOUT))
         return 1;

      /* We can try to use the dirname path. */
      if ((ndata_filename == NULL) && (ndata_dirname != NULL) &&
            (ndata_source <= NDATA_SRC_DIRNAME)) {
         nsnprintf( path, sizeof(path), "%s/%s", ndata_dirname, filename );
         if (nfile_fileExists( path ))
            return 1;
      }

      /* We can also try default location. */
      if (ndata_source <= NDATA_SRC_NDATADEF) {
         buf = strdup( NDATA_DEF );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(buf), filename );
         free(buf);
         if (nfile_fileExists( path ))
            return 1;
      }

      /* Try binary location. */
      if (ndata_source <= NDATA_SRC_BINARY) {
         buf = strdup( naev_binary() );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(buf), filename );
         free(buf);
         if (nfile_fileExists( path ))
            return 1;
      }

      return 0;
   }

   /* Try to get it from the archive. */
   return nzip_hasFile( ndata_archive, filename );
}


/**
 * @brief Reads a file from the ndata.
 *
 *    @param filename Name of the file to read.
 *    @param[out] filesize Stores the size of the file.
 *    @return The file data or NULL on error.
 */
void* ndata_read( const char* filename, uint32_t *filesize )
{
   char *buf, path[PATH_MAX];
   int nbuf;

   /* See if needs to load ndata archive. */
   if (ndata_archive == NULL) {

      /* Try to read the file as locally. */
      if (nfile_fileExists( filename ) && (ndata_source <= NDATA_SRC_LAIDOUT)) {
         buf = nfile_readFile( &nbuf, filename );
         if (buf != NULL) {
            ndata_loadedfile = 1;
            *filesize = nbuf;
            return buf;
         }
      }

      /* We can try to use the dirname path. */
      if ((ndata_filename == NULL) && (ndata_dirname != NULL) &&
            (ndata_source <= NDATA_SRC_DIRNAME)) {
         nsnprintf( path, sizeof(path), "%s/%s", ndata_dirname, filename );
         if (nfile_fileExists( path )) {
            buf = nfile_readFile( &nbuf, path );
            if (buf != NULL) {
               ndata_source = NDATA_SRC_DIRNAME;
               ndata_loadedfile = 1;
               *filesize = nbuf;
               return buf;
            }
         }
      }

      /* We can also try default location. */
      if (ndata_source <= NDATA_SRC_NDATADEF) {
         buf = strdup( NDATA_DEF );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(buf), filename );
         free(buf);
         if (nfile_fileExists( path )) {
            buf = nfile_readFile( &nbuf, path );
            if (buf != NULL) {
               ndata_source = NDATA_SRC_NDATADEF;
               ndata_loadedfile = 1;
               *filesize = nbuf;
               return buf;
            }
         }
      }

      /* Try binary location. */
      if (ndata_source <= NDATA_SRC_BINARY) {
         buf = strdup( naev_binary() );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(buf), filename );
         free(buf);
         if (nfile_fileExists( path )) {
            buf = nfile_readFile( &nbuf, path );
            if (buf != NULL) {
               ndata_source = NDATA_SRC_BINARY;
               ndata_loadedfile = 1;
               *filesize = nbuf;
               return buf;
            }
         }
      }

      /* Load the ndata archive. */
      ndata_openFile();
   }

   /* Wasn't able to open the file. */
   if (ndata_archive == NULL) {
      WARN("Unable to open file '%s': not found.", filename);
      *filesize = 0;
      return NULL;
   }

   /* Mark that we loaded a file. */
   ndata_loadedfile = 1;

   /* Get data from ndata archive. */
   return nzip_readFile( ndata_archive, filename, filesize );
}


/**
 * @brief Creates an rwops from a file in the ndata.
 *
 *    @param filename Name of the file to create rwops of.
 *    @return rwops that accesses the file in the ndata.
 */
SDL_RWops *ndata_rwops( const char* filename )
{
   char path[PATH_MAX], *tmp;
   SDL_RWops *rw;

   if (ndata_archive == NULL) {

      /* Try to open from file. */
      if (ndata_source <= NDATA_SRC_LAIDOUT) {
         rw = SDL_RWFromFile( filename, "rb" );
         if (rw != NULL) {
            ndata_loadedfile = 1;
            return rw;
         }
      }

      /* Try to open from dirname. */
      if ((ndata_filename == NULL) && (ndata_dirname != NULL) &&
            (ndata_source <= NDATA_SRC_DIRNAME)) {
         nsnprintf( path, sizeof(path), "%s/%s", ndata_dirname, filename );
         rw = SDL_RWFromFile( path, "rb" );
         if (rw != NULL) {
            ndata_source = NDATA_SRC_DIRNAME;
            ndata_loadedfile = 1;
            return rw;
         }
      }

      /* Try to open from def. */
      if (ndata_source <= NDATA_SRC_NDATADEF) {
         tmp = strdup( NDATA_DEF );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(tmp), filename );
         free(tmp);
         rw = SDL_RWFromFile( path, "rb" );
         if (rw != NULL) {
            ndata_source = NDATA_SRC_NDATADEF;
            ndata_loadedfile = 1;
            return rw;
         }
      }

      /* Try to open from binary. */
      if (ndata_source <= NDATA_SRC_BINARY) {
         tmp = strdup( naev_binary() );
         nsnprintf( path, sizeof(path), "%s/%s", nfile_dirname(tmp), filename );
         free(tmp);
         rw = SDL_RWFromFile( path, "rb" );
         if (rw != NULL) {
            ndata_source = NDATA_SRC_BINARY;
            ndata_loadedfile = 1;
            return rw;
         }
      }

      /* Load the ndata archive. */
      ndata_openFile();
   }

   /* Wasn't able to open the file. */
   if (ndata_archive == NULL) {
      WARN("Unable to open file '%s': not found.", filename);
      return NULL;
   }

   /* Mark that we loaded a file. */
   ndata_loadedfile = 1;

   return nzip_rwops( ndata_archive, filename );
}


/**
 * @brief Removes a common path from a list of files, if present.
 *
 *    @param list List of files to filter.
 *    @param nlist Number of files in the list.
 *    @param path Path to remove from the filenames.
 */
static char **stripPath( const char **list, int nlist, const char *path )
{
   int i, len;
   char **out, *buf;

   out = malloc(sizeof(char*) * nlist);
   len = strlen( path );

   /* Slash-terminate as needed. */
   if (strcmp(&path[len],"/")!=0) {
      len++;
      buf = malloc(len + 1);
      nsnprintf(buf, len+1,  "%s/", path );
   }
   else
      buf = strdup(path);

   for (i=0; i<nlist; i++) {
      if (strncmp(list[i],buf,len)==0)
         out[i] = strdup( &list[i][len] );
      else
         out[i] = strdup( list[i] );
   }

   free(buf);
   return out;
}


/**
 * @brief Filters a file list to match path.
 *
 *    @param list List to filter.
 *    @param nlist Members in list.
 *    @param path Path to filter.
 *    @param recursive Whether all children at any depth should be listed.
 *    @param[out] nfiles Files that match.
 */
static char** filterList( const char** list, int nlist,
      const char* path, uint32_t* nfiles, int recursive )
{
   char **filtered;
   int i, j, k, len;

   /* Maximum size by default. */
   filtered = malloc(sizeof(char*) * nlist);
   len = strlen( path );

   /* Filter list. */
   j = 0;
   for (i=0; i<nlist; i++) {
      /* Must match path. */
      if (strncmp(list[i], path, len)!=0)
         continue;

      /* Make sure there are no stray '/'. */
      for (k=len; list[i][k] != '\0'; k++)
         if (list[i][k] == '/')
            if (!recursive)
               break;

      if (list[i][k] != '\0')
         continue;

      /* Copy the file name without the path. */
      if (!recursive)
         filtered[j++] = strdup(&list[i][len]);
      else /* Recursive needs paths. */
         filtered[j++] = strdup(list[i]);
   }

   /* Return results. */
   *nfiles = j;
   return filtered;
}


/**
 * @brief Gets the list of files in the ndata.
 *
 * @note Strips the path.
 *
 *    @param path List files in path.
 *    @param nfiles Number of files found.
 *    @return List of files found.
 */
static char** ndata_listBackend( const char* path, uint32_t* nfiles, int recursive )
{
   (void) path;
   char **files, **tfiles, buf[PATH_MAX], *tmp;
   int n;
   char** (*nfile_readFunc) ( int* nfiles, const char* path, ... ) = NULL;

   if (recursive)
      nfile_readFunc = nfile_readDirRecursive;
   else
      nfile_readFunc = nfile_readDir;

   /* Already loaded the list. */
   if (ndata_fileList != NULL)
      return filterList( (const char**) ndata_fileList, ndata_fileNList, path, nfiles, recursive );

   /* See if can load from local directory. */
   if (ndata_archive == NULL) {

      /* Local search. */
      if (ndata_source <= NDATA_SRC_LAIDOUT) {
         files = nfile_readFunc( &n, path );
         if (files != NULL) {
            *nfiles = n;
            return files;
         }
      }

      /* Dirname search. */
      if ((ndata_filename == NULL) && (ndata_dirname != NULL) &&
            (ndata_source <= NDATA_SRC_DIRNAME)) {
         nsnprintf( buf, sizeof(buf), "%s/%s", ndata_dirname, path );
         tfiles = nfile_readFunc( &n, buf );
         files = stripPath( (const char**)tfiles, n, ndata_dirname );
         free(tfiles);
         if (files != NULL) {
            *nfiles = n;
            return files;
         }
      }

      /* NDATA_DEF. */
      if (ndata_source <= NDATA_SRC_NDATADEF) {
         tmp = strdup( NDATA_DEF );
         nsnprintf( buf, sizeof(buf), "%s/%s", nfile_dirname(tmp), path );
         tfiles = nfile_readFunc( &n, buf );
         files = stripPath( (const char**)tfiles, n, tmp );
         free(tmp);
         free(tfiles);
         if (files != NULL) {
            *nfiles = n;
            return files;
         }
      }

      /* Binary. */
      if (ndata_source <= NDATA_SRC_BINARY) {
         tmp = strdup( naev_binary() );
         nsnprintf( buf, sizeof(buf), "%s/%s", nfile_dirname(tmp), path );
         tfiles = nfile_readFunc( &n, buf );
         files = stripPath( (const char**)tfiles, n, nfile_dirname(tmp) );
         free(tmp);
         free(tfiles);
         if (files != NULL) {
            *nfiles = n;
            return files;
         }
      }

      /* Open ndata archive. */
      ndata_openFile();
   }

   /* Wasn't able to open the file. */
   if (ndata_archive == NULL) {
      *nfiles = 0;
      return NULL;
   }

   /* Load list. */
   ndata_fileList = nzip_listFiles( ndata_archive, &ndata_fileNList );

   return filterList( (const char**) ndata_fileList, ndata_fileNList, path, nfiles, recursive );
}

/**
 * @brief Gets a list of files in the ndata that are direct children of a path.
 *
 *    @sa ndata_listBackend
 */
char** ndata_list( const char* path, uint32_t* nfiles )
{
   return ndata_listBackend( path, nfiles, 0 );
}


/**
 * @brief Gets a list of files in the ndata below a certain path.
 *
 *    @sa ndata_listBackend
 */
char** ndata_listRecursive( const char* path, uint32_t* nfiles )
{
   return ndata_listBackend( path, nfiles, 1 );
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
void ndata_sortName( char **files, uint32_t nfiles )
{
   qsort( files, nfiles, sizeof(char*), ndata_sortFunc );
}



