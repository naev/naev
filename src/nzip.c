#include "nzip.h"

#include <zip.h>

#include "log.h"

/**
 * Private function prototypes
 */
void nzip_printError ( int err );
int nzip_rwopsClose ( struct SDL_RWops* context );




/**
 * @brief Tests a file for zip like qualities
 *
 *    @param filename Name of the archive to check.
 *    @return 0 if not zip, 1 if zip.
 */
int nzip_isZip ( const char* filename )
{
   struct zip* arc;

   arc = nzip_open ( filename );

   if ( arc == NULL )
      return 0;

   nzip_close ( arc );

   return 1;
}

/**
 * @brief Opens a zip file.
 *
 *    @param filename Name of the archive to open.
 *    @return The archive, or NULL on error.
 */
struct zip* nzip_open ( const char* filename )
{
   struct zip* arc;
   int flags = 0;
   int err = 0;

   arc = zip_open ( filename, flags, &err );

   if ( err != 0 || arc == NULL ) {
      WARN ( "Error opening zip file %s", filename );
      nzip_printError ( err );
   }

   return arc;
}

/**
 * @brief Closes a zip file and free's resources
 *
 *    @param arc Archive to close
 */
void nzip_close ( struct zip* arc )
{
   if ( zip_close ( arc ) ) {
      WARN ( "Error closing zip file" );
      WARN ( "%s", zip_strerror ( arc ) );
   }
}

/**
 * @brief Check to see if a file exists in an archive
 *
 *    @param arc Archive to look in
 *    @param filename File to look for
 *    @return 1 if found, 0 otherwise
 */
int nzip_hasFile ( struct zip* arc, const char* filename )
{
   int flags = 0;
   return zip_name_locate ( arc, filename, flags ) >= 0;
}

/**
 * @brief Read the contents of a file from an archive
 *
 *    @param arc Archive to look in
 *    @param filename File to read
 *    @param[out] size Size of returned buffer
 *    @return A pointer to the file contents in memory
 */
void* nzip_readFile ( struct zip* arc, const char* filename, uint32_t* size )
{
   struct zip_file* file;
   struct zip_stat stats;
   void* data;
   int err;
   uint32_t read;
   int flags = 0;

   // Get info about file
   zip_stat_init ( &stats );
   err = zip_stat ( arc, filename, flags, &stats );

   if ( err ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      return NULL;
   }

   // Open the file
   file = zip_fopen_index ( arc, stats.index, flags );

   if ( file == NULL ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      return NULL;
   }

   // Allocate based on reported size
   data = malloc ( stats.size );

   // Read file data to buffer
   read = zip_fread ( file, data, stats.size );

   // If we read less than the reported file size, something probably went wrong
   if ( read < stats.size ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      free ( data );
      zip_fclose ( file );
      return NULL;
   }

   // Close the file
   zip_fclose ( file );

   // Return data
   *size = stats.size;
   return data;
}

/**
 * @brief Return a list of all files in an archive
 *
 *    @param arc Archive to look through
 *    @param[out] nfiles Number of files found
 *    @return List of file names found
 */
char** nzip_listFiles ( struct zip* arc, uint32_t* nfiles )
{
   struct zip_stat stats;
   char **filelist, **shrunk;
   uint32_t i, j;
   int err;
   int flags = 0;

   // Total number of archive entries (includes directories)
   *nfiles = zip_get_num_entries ( arc, flags );

   filelist = malloc ( sizeof ( char* ) * ( *nfiles ) );

   // Get stats for each file, and store the name
   for ( i = 0, j = 0; i < *nfiles; i++ ) {
      zip_stat_init ( &stats );
      err = zip_stat_index ( arc, i, flags, &stats );

      if ( err ) {
         WARN ( "Error getting file list from archive" );
         WARN ( "%s", zip_strerror ( arc ) );
         free ( filelist );
         return NULL;
      }

      // If the name ends with a forward slash, it's a directory
      if (stats.name[strlen(stats.name) - 1] != '/')
         filelist[j++] = strdup(stats.name);
   }

   // Number of files excluding directories
   *nfiles = j;

   // Shrink file list to needed size
   shrunk = realloc( filelist, sizeof(char*) * j );
   if (shrunk != NULL)
      return shrunk;

   return filelist;
}

/**
 * @brief Print error message given a libzip error code
 *
 *    @param err The error code
 */
void nzip_printError ( int err )
{
   char *buf;
   int size;

   // Get length of message
   size = zip_error_to_str ( NULL, 0, err, 0 );
   // Get message
   buf = malloc ( sizeof ( char ) * size );
   zip_error_to_str ( buf, 50, err, 0 );

   WARN ( "%s", buf );

   free ( buf );
}

/**
 * @brief Close a rwop
 */
int nzip_rwopsClose ( struct SDL_RWops* context )
{
   free(context->hidden.unknown.data1);
   SDL_FreeRW(context);
   return 0;
}

/**
 * @brief Return SDL_RWops for a file in an archive. This version works on a copy of the file in memory.
 *
 *    @param arc Archive to look in
 *    @param filename File to get from archive
 *    @return RWops for filename
 */
SDL_RWops* nzip_rwops ( struct zip* arc, const char* filename )
{
   void* data;
   uint32_t size;
   SDL_RWops* rwops;

   data = nzip_readFile ( arc, filename, &size );
   rwops = SDL_RWFromMem ( data,size);
   rwops->close = nzip_rwopsClose;
   rwops->hidden.unknown.data1 = data;
   return rwops;
}
