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
#include "log.h"
#include "md5.h"
#include "nxml.h"
#include "pack.h"
#include "nfile.h"


#define NDATA_FILENAME  "ndata"
#ifndef NDATA_DEF
#define NDATA_DEF       NDATA_FILENAME
#endif /* NDATA_DEF */

#define XML_START_ID    "Start"  /**< XML document tag of module start file. */
#define START_DATA      "dat/start.xml" /**< Path to module start file. */


/*
 * Packfile.
 */
static char* ndata_filename = NULL; /**< Packfile name. */
static Packcache_t *ndata_cache = NULL; /**< Actual packfile. */
static char* ndata_packName = NULL; /**< Name of the ndata module. */

/*
 * File list.
 */
static const char **ndata_fileList = NULL; /**< List of files in the packfile. */
static uint32_t ndata_fileNList = 0; /**< Number of files in ndata_fileList. */


/**
 * @brief Checks to see if path is a ndata file.
 *
 *    @param path Path to check to see if it's an ndata file.
 *    @return 1 if it is an ndata file, 0 else.
 */
int ndata_check( char* path )
{
   return pack_check( path );
}


/**
 * @brief Sets the current ndata path to use.
 *
 *    @param path Path to set.
 */
int ndata_setPath( char* path )
{
   if (ndata_filename != NULL)
      free(ndata_filename);
   ndata_filename = (path == NULL) ? NULL : strdup(path);
   return 0;
}


/**
 * @brief Opens a packfile if needed.
 */
static int ndata_openPackfile (void)
{
   int i;
   char **files;
   int nfiles;
   size_t len;

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
         len = strlen(NDATA_FILENAME);
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

   /* Open the cache. */
   ndata_cache = pack_openCache( ndata_filename );
   if (ndata_cache == NULL)
      WARN("Unable to create Packcache from '%s'.", ndata_filename );

   return 0;
}


/**
 * @brief Opens the ndata file.
 *
 *    @return 0 on success.
 */
int ndata_open (void)
{
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
 */
void* ndata_read( const char* filename, uint32_t *filesize )
{
   if (ndata_cache == NULL)
      ndata_openPackfile();
   return pack_readfileCached( ndata_cache, filename, filesize );
}


/**
 * @brief Gets the list of files in the ndata.
 */
const char** ndata_list( const char* path, uint32_t* nfiles )
{
   (void) path;

   if (ndata_fileList != NULL) {
      *nfiles = ndata_fileNList;
      return ndata_fileList;
   }

   if (ndata_cache == NULL)
      ndata_openPackfile();

   ndata_fileList = pack_listfilesCached( ndata_cache, &ndata_fileNList );
   *nfiles = ndata_fileNList;
   return ndata_fileList;
}

