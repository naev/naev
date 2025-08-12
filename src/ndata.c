/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file ndata.c
 *
 * @brief Wrappers to set up and access game assets (mounted via PhysicsFS).
 *        We choose our underlying directories in ndata_setupWriteDir() and
 * ndata_setupReadDirs(). However, conf.c code may have seeded the search path
 * based on command-line arguments.
 */
/** @cond */
#include <stdlib.h>
#if SDL_PLATFORM_WIN32
#include <windows.h>
#endif /* SDL_PLATFORM_WIN32 */

#include "physfs.h"
#include <SDL3/SDL_stdinc.h>
/** @endcond */

#include "ndata.h"

#include "array.h"
#if SDL_PLATFORM_MACOS
#include "glue_macos.h"
#endif /* SDL_PLATFORM_MACOS */
#include "log.h"
#include "nstring.h"
#include "physfs_archiver_blacklist.c"

/*
 * Prototypes.
 */
static int ndata_enumerateCallback( void *data, const char *origdir,
                                    const char *fname );

/**
 * @brief Reads a file from the ndata (will be NUL terminated).
 *
 *    @param path Path of the file to read.
 *    @param[out] filesize Stores the size of the file.
 *    @return The file data or NULL on error.
 */
void *ndata_read( const char *path, size_t *filesize )
{
   char         *buf;
   PHYSFS_file  *file;
   PHYSFS_sint64 len, n;
   PHYSFS_Stat   path_stat;

   if ( !PHYSFS_stat( path, &path_stat ) ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      *filesize = 0;
      return NULL;
   }
   if ( path_stat.filetype != PHYSFS_FILETYPE_REGULAR ) {
      WARN( _( "Error occurred while opening '%s': It is not a regular file" ),
            path );
      *filesize = 0;
      return NULL;
   }

   /* Open file. */
   file = PHYSFS_openRead( path );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. TODO: Don't assume this is always possible? */
   len = PHYSFS_fileLength( file );
   if ( len == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len + 1 );
   if ( buf == NULL ) {
      WARN( _( "Out of Memory" ) );
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while ( n < len ) {
      size_t pos = PHYSFS_readBytes( file, &buf[n], len - n );
      if ( pos == 0 ) {
         WARN( _( "Error occurred while reading '%s': %s" ), path,
               _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
         PHYSFS_close( file );
         *filesize = 0;
         free( buf );
         return NULL;
      }
      n += pos;
   }

   /* Close the file. */
   PHYSFS_close( file );

   *filesize = len;
   return buf;
}

/**
 * @brief Lists all the visible files in a directory, at any depth.
 *
 * Will sort by path, and (unlike underlying PhysicsFS) make sure to list each
 * file path only once.
 *
 *    @return Array of (allocated) file paths relative to base_dir.
 */
char **ndata_listRecursive( const char *path )
{
   char **files = array_create( char * );
   PHYSFS_enumerate( path, ndata_enumerateCallback, &files );
   /* Ensure unique. PhysicsFS can enumerate a path twice if it's in multiple
    * components of a union. */
   qsort( files, array_size( files ), sizeof( char * ), strsort );
   for ( int i = 0; i + 1 < array_size( files ); i++ )
      if ( strcmp( files[i], files[i + 1] ) == 0 ) {
         free( files[i] );
         array_erase( &files, &files[i], &files[i + 1] );
         i--; /* We're not done checking for dups of files[i]. */
      }
   for ( int i = 0; i < array_size( files ); i++ ) {
      if ( blacklist_isBlacklisted( files[i] ) ) {
         free( files[i] );
         array_erase( &files, &files[i], &files[i + 1] );
         i--; /* We're not done checking for dups of files[i]. */
      }
   }
   return files;
}

/**
 * @brief The PHYSFS_EnumerateCallback for ndata_listRecursive
 */
static int ndata_enumerateCallback( void *data, const char *origdir,
                                    const char *fname )
{
   char       *path;
   const char *fmt;
   size_t      dir_len;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   fmt     = dir_len && origdir[dir_len - 1] == '/' ? "%s%s" : "%s/%s";
   SDL_asprintf( &path, fmt, origdir, fname );
   if ( !PHYSFS_stat( path, &stat ) ) {
      WARN( _( "PhysicsFS: Cannot stat %s: %s" ), path,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      free( path );
   } else if ( stat.filetype == PHYSFS_FILETYPE_REGULAR )
      array_push_back( (char ***)data, path );
   else if ( stat.filetype == PHYSFS_FILETYPE_DIRECTORY ) {
      PHYSFS_enumerate( path, ndata_enumerateCallback, data );
      free( path );
   } else
      free( path );
   return PHYSFS_ENUM_OK;
}

/**
 * @brief Backup a file, if it exists.
 *
 *    @param path PhysicsFS relative pathname to back up.
 *    @return 0 on success, or if file does not exist, -1 on error.
 */
int ndata_backupIfExists( const char *path )
{
   char backup[PATH_MAX];

   if ( path == NULL )
      return -1;

   if ( !PHYSFS_exists( path ) )
      return 0;

   snprintf( backup, sizeof( backup ), "%s.backup", path );

   return ndata_copyIfExists( path, backup );
}

/**
 * @brief Copy a file, if it exists.
 *
 *    @param file1 PhysicsFS relative pathname to copy from.
 *    @param file2 PhysicsFS relative pathname to copy to.
 *    @return 0 on success, or if file1 does not exist, -1 on error.
 */
int ndata_copyIfExists( const char *file1, const char *file2 )
{
   PHYSFS_File  *f_in, *f_out;
   char          buf[8 * 1024];
   PHYSFS_sint64 lr, lw;

   if ( file1 == NULL )
      return -1;

   /* Check if input file exists */
   if ( !PHYSFS_exists( file1 ) )
      return 0;

   /* Open files. */
   f_in  = PHYSFS_openRead( file1 );
   f_out = PHYSFS_openWrite( file2 );
   if ( ( f_in == NULL ) || ( f_out == NULL ) ) {
      WARN( _( "Failure to copy '%s' to '%s': %s" ), file1, file2,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      if ( f_in != NULL )
         PHYSFS_close( f_in );
      return -1;
   }

   /* Copy data over. */
   do {
      lr = PHYSFS_readBytes( f_in, buf, sizeof( buf ) );
      if ( lr == -1 )
         goto err;
      else if ( !lr ) {
         if ( PHYSFS_eof( f_in ) )
            break;
         goto err;
      }

      lw = PHYSFS_writeBytes( f_out, buf, lr );
      if ( lr != lw )
         goto err;
   } while ( lr > 0 );

   /* Close files. */
   PHYSFS_close( f_in );
   PHYSFS_close( f_out );

   return 0;

err:
   WARN( _( "Failure to copy '%s' to '%s': %s" ), file1, file2,
         _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
   PHYSFS_close( f_in );
   PHYSFS_close( f_out );

   return -1;
}

/**
 * @brief Sees if a file matches an extension.
 *
 *    @param path Path to check extension of.
 *    @param ext Extension to check.
 *    @return 1 on match, 0 otherwise.
 */
int ndata_matchExt( const char *path, const char *ext )
{
   int i;
   /* Find the dot. */
   for ( i = strlen( path ) - 1; i > 0; i-- )
      if ( path[i] == '.' )
         break;
   if ( i <= 0 )
      return 0;
   return strcmp( &path[i + 1], ext ) == 0;
}

/**
 * @brief Tries to see if a file is in a default path before seeing if it is an
 * absolute path.
 *
 *    @param[out] path Path found.
 *    @param len Length of path.
 *    @param default_path Default path to look in.
 *    @param filename Name of the file to look for.
 *    @return 1 if found.
 */
int ndata_getPathDefault( char *path, int len, const char *default_path,
                          const char *filename )
{
   PHYSFS_Stat path_stat;
   snprintf( path, len, "%s%s", default_path, filename );
   if ( PHYSFS_stat( path, &path_stat ) &&
        ( path_stat.filetype == PHYSFS_FILETYPE_REGULAR ) )
      return 1;
   snprintf( path, len, "%s", filename );
   if ( PHYSFS_stat( path, &path_stat ) &&
        ( path_stat.filetype == PHYSFS_FILETYPE_REGULAR ) )
      return 1;
   return 0;
}
