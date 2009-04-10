/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ndata.c
 *
 * @brief Wrapper to handle reading/writing the ndata file.
 *
 * Optimizes to minimize the opens and frees, plus tries to read from the
 *  filesystem instead always looking for a packfile.
 */

#include "ndata.h"

#include "naev.h"

#if HAS_WIN32
#include <windows.h>
#endif /* HAS_WIN32 */

#include "SDL.h"
#include "SDL_mutex.h"

#include "log.h"
#include "md5.h"
#include "nxml.h"
#include "pack.h"
#include "nfile.h"


#define NDATA_FILENAME  "ndata" /**< Generic ndata file name. */
#ifndef NDATA_DEF
#define NDATA_DEF       NDATA_FILENAME /**< Default ndata to use. */
#endif /* NDATA_DEF */

#define XML_START_ID    "Start"  /**< XML document tag of module start file. */
#define START_DATA      "dat/start.xml" /**< Path to module start file. */


/*
 * Packfile.
 */
static char* ndata_filename         = NULL; /**< Packfile name. */
static Packcache_t *ndata_cache     = NULL; /**< Actual packfile. */
static char* ndata_packName         = NULL; /**< Name of the ndata module. */
static SDL_mutex *ndata_lock        = NULL; /**< Lock for ndata creation. */

/*
 * File list.
 */
static const char **ndata_fileList  = NULL; /**< List of files in the packfile. */
static uint32_t ndata_fileNList     = 0; /**< Number of files in ndata_fileList. */


/*
 * Prototypes.
 */
static char** filterList( const char** list, int nlist,
      const char* path, uint32_t* nfiles );


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
   return pack_check( path );
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
   if (ndata_filename != NULL)
      free(ndata_filename);
   ndata_filename = (path == NULL) ? NULL : strdup(path);
   return 0;
}


/**
 * @brief Opens a packfile if needed.
 *
 *    @return 0 on success.
 */
static int ndata_openPackfile (void)
{
   int i;
   char **files;
   int nfiles;
   size_t len;

   /* Must be thread safe. */
   SDL_mutexP(ndata_lock);

   /* Was opened while locked. */
   if (ndata_cache != NULL) {
      SDL_mutexV(ndata_lock);
      return 0;
   }

   /*
    * Try to find the ndata file.
    */
   if (ndata_filename == NULL) {
      /* Check ndata with version appended. */
      if (nfile_fileExists("%s-%d.%d.%d", NDATA_FILENAME, VMAJOR, VMINOR, VREV )) {
         ndata_filename = malloc(PATH_MAX);
         snprintf( ndata_filename, PATH_MAX, "%s-%d.%d.%d",
               NDATA_FILENAME, VMAJOR, VMINOR, VREV );
      }
      /* Check default ndata. */
      else if (nfile_fileExists(NDATA_DEF))
         ndata_filename = strdup(NDATA_DEF);
      /* Try to open any ndata in path. */
      else {                                                                 
         files = nfile_readDir( &nfiles, "." );
         if (files != NULL) {
            len   = strlen(NDATA_FILENAME);
            for (i=0; i<nfiles; i++) {
               if (strncmp(files[i], NDATA_FILENAME, len)==0) {
                  /* Must be packfile. */
                  if (pack_check(files[i]))
                     continue;

                  ndata_filename = strdup(files[i]);
                  break;
               }
            }

            /* Clean up. */
            for (i=0; i<nfiles; i++)
               free(files[i]);
            free(files);
         }
      }
   }

   /* Open the cache. */
   if (nfile_fileExists( ndata_filename ) != 1) {
      WARN("Cannot find ndata file!");
      WARN("Please specify ndata file with -d or data in the conf file.");

      /* Window users get hand holding. */
#if HAS_WIN32
      SDL_Quit(); /* Should destroy window and give focus to MessageBox. */
      MessageBox( NULL,
            "ndata file not found.\n"
            "Please specify ndata file with -d or data in the conf file.",
            "Cannot find ndata file!",
            MB_OK | MB_ICONEXCLAMATION );
#endif /* HAS_WIN32 */

      exit(1);
   }
   ndata_cache = pack_openCache( ndata_filename );
   if (ndata_cache == NULL)
      WARN("Unable to create Packcache from '%s'.", ndata_filename );

   /* Close lock. */
   SDL_mutexV(ndata_lock);

   return 0;
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

   /* If user enforces ndata filename, we'll respect that. */
   if (ndata_filename != NULL)
      return ndata_openPackfile();

   return 0;
}


/**
 * @brief Closes and cleans up the ndata file.
 */
void ndata_close (void)
{
   /* Destroy the name. */
   if (ndata_packName != NULL) {
      free(ndata_packName);
      ndata_packName = NULL;
   }

   /* Destroy the list. */
   if (ndata_fileList != NULL) {
      /* No need to free memory since cache does that. */
      ndata_fileList = NULL;
      ndata_fileNList = 0;
   }

   /* Close the packfile. */
   if (ndata_cache) {
      pack_closeCache(ndata_cache);
      ndata_cache = NULL;
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
   char *buf;
   uint32_t size;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Already loaded. */
   if (ndata_packName != NULL)
      return ndata_packName;

   /* We'll just read it and parse it. */
   buf = ndata_read( START_DATA, &size );
   if (buf == NULL) {
      WARN("Unable to open '%s'.", START_DATA);
      return NULL;
   }
   doc = xmlParseMemory( buf, size );

   /* Make sure it's what we are looking for. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
      return NULL;
   }

   /* Check if node is valid. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"START_DATA"' file: does not contain elements");
      return NULL;
   }
   do {
      xmlr_strd(node, "name", ndata_packName);
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);

   /* Check if data if name is found. */
   if (ndata_packName == NULL)
      WARN("No ndata packname found.");

   return ndata_packName;
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
   char *buf;
   int nbuf;

   /* See if needs to load packfile. */
   if (ndata_cache == NULL) {

      /* Try to read the file as locally. */
      if (nfile_fileExists( filename )) {
         buf = nfile_readFile( &nbuf, filename );
         if (buf != NULL) {
            *filesize = nbuf;
            return buf;
         }
      }

      /* Load the packfile. */
      ndata_openPackfile();
   }

   /* Get data from packfile. */
   return pack_readfileCached( ndata_cache, filename, filesize );
}


/**
 * @brief Creates an rwops from a file in the ndata.
 *
 *    @param filename Name of the file to create rwops of.
 *    @return rwops that accesses the file in the ndata.
 */
SDL_RWops *ndata_rwops( const char* filename )
{
   SDL_RWops *rw;

   if (ndata_cache == NULL) {

      /* Try to open from file. */
      rw = SDL_RWFromFile( filename, "rb" );
      if (rw != NULL)
         return rw;

      /* Load the packfile. */
      ndata_openPackfile();
   }

   return pack_rwopsCached( ndata_cache, filename );
}


/**
 * @brief Filters a file list to match path.
 *
 *    @param list List to filter.
 *    @param nlist Members in list.
 *    @param path Path to filter.
 *    @param[out] nfiles Files that match.
 */
static char** filterList( const char** list, int nlist,
      const char* path, uint32_t* nfiles )
{
   char **filtered;
   int i, j, k;
   int len;

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
            break;
      if (list[i][k] != '\0')
         continue;

      /* Copy the file name without the path. */
      filtered[j++] = strdup(&list[i][len]);
   }

   /* Return results. */
   *nfiles = j;
   return filtered;
}


/**
 * @brief Gets the list of files in the ndata.
 *
 *    @param path List files in path.
 *    @param nfiles Number of files found.
 *    @return List of files found.
 */
char** ndata_list( const char* path, uint32_t* nfiles )
{
   (void) path;
   char **files;
   int n;

   /* Already loaded the list. */
   if (ndata_fileList != NULL)
      return filterList( ndata_fileList, ndata_fileNList, path, nfiles );

   /* See if can load from local directory. */
   if (ndata_cache == NULL) {
      files = nfile_readDir( &n, path );

      /* Found locally. */
      if (files != NULL) {
         *nfiles = n;
         return files;
      }

      /* Open packfile. */
      ndata_openPackfile();
   }

   /* Load list. */
   ndata_fileList = pack_listfilesCached( ndata_cache, &ndata_fileNList );

   return filterList( ndata_fileList, ndata_fileNList, path, nfiles );
}

