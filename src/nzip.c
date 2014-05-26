#include "nzip.h"

#include <zip.h>

#include "log.h"


void nzip_printError ( int err );


int nzip_isZip ( const char* filename )
{
   struct zip* arc;

   arc = nzip_open ( filename );

   if ( arc == NULL )
      return 0;

   nzip_close ( arc );

   return 1;
}

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

void nzip_close ( struct zip* arc )
{
   if ( zip_close ( arc ) ) {
      WARN ( "Error closing zip file" );
      WARN ( "%s", zip_strerror ( arc ) );
   }
}

int nzip_hasFile ( struct zip* arc, const char* filename )
{
   int flags = 0;
   return zip_name_locate ( arc, filename, flags ) >= 0;
}

void* nzip_readFile ( struct zip* arc, const char* filename, uint32_t* size )
{
   struct zip_file* file;
   struct zip_stat stats;
   void* data;
   int err;
   uint32_t read;
   int flags = 0;

   zip_stat_init ( &stats );
   err = zip_stat ( arc, filename, flags, &stats );

   if ( err ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      return NULL;
   }

   file = zip_fopen_index ( arc, stats.index, flags );

   if ( file == NULL ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      return NULL;
   }

   data = malloc ( stats.size );

   read = zip_fread ( file, data, stats.size );

   if ( read < stats.size ) {
      WARN ( "Error reading %s from archive", filename );
      WARN ( "%s", zip_strerror ( arc ) );
      return NULL;
   }

   zip_fclose ( file );

   *size = stats.size;
   return data;
}

char** nzip_listFiles ( struct zip* arc, uint32_t* nfiles )
{
   struct zip_stat stats;
   char** filelist;
   char** filelistshrunk;
   uint32_t i, j;
   int err;
   int flags = 0;

   *nfiles = zip_get_num_entries ( arc, flags );

   filelist = malloc ( sizeof ( char* ) * ( *nfiles ) );

   for ( i = 0, j = 0; i < *nfiles; i++ ) {
      zip_stat_init ( &stats );
      err = zip_stat_index ( arc, i, flags, &stats );

      if ( err ) {
         WARN ( "Error getting file list from archive" );
         WARN ( "%s", zip_strerror ( arc ) );
         return NULL;
      }

      // If the name ends with a forward slash, it's a directory
      if ( stats.name[strlen ( stats.name ) - 1] != '/' ) {
         filelist[j] = malloc ( sizeof ( char ) * strlen ( stats.name ) + 1 );
         strcpy ( filelist[j], stats.name );
         j++;
      }
   }

   *nfiles = j;

   filelistshrunk = realloc ( filelist, sizeof ( char* ) *j );

   if ( filelistshrunk == NULL )
      return filelist;

   else
      return filelistshrunk;
}

void nzip_printError ( int err )
{
   char *buf;
   int size;

   size = zip_error_to_str ( NULL, 0, err, 0 );
   buf = malloc ( sizeof ( char ) * size );
   zip_error_to_str ( buf, 50, err, 0 );
   WARN ( "%s", buf );
   free ( buf );
}

SDL_RWops* nzip_rwops ( struct zip* arc, const char* filename )
{
   void* data;
   uint32_t size;

   data = nzip_readFile ( arc, filename, &size );
   return SDL_RWFromMem ( data, size );
}
