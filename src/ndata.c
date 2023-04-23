/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file ndata.c
 *
 * @brief Wrappers to set up and access game assets (mounted via PhysicsFS).
 *        We choose our underlying directories in ndata_setupWriteDir() and ndata_setupReadDirs().
 *        However, conf.c code may have seeded the search path based on command-line arguments.
 */
/** @cond */
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#if WIN32
#include <windows.h>
#endif /* WIN32 */

#include "physfs.h"
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "ndata.h"

#include "array.h"
#include "conf.h"
#include "env.h"
#if MACOS
#include "glue_macos.h"
#endif /* MACOS */
#include "log.h"
#include "nfile.h"
#include "nstring.h"
#include "plugin.h"

/*
 * Prototypes.
 */
static void ndata_testVersion (void);
static int ndata_found (void);
static int ndata_enumerateCallback( void* data, const char* origdir, const char* fname );

/**
 * @brief Checks to see if the physfs search path is enough to find game data.
 */
static int ndata_found( void )
{
   /* Verify that we can find VERSION and start.xml.
    * This is arbitrary, but these are among the hard dependencies to self-identify and start.
    */
   return PHYSFS_exists( "VERSION" ) && PHYSFS_exists( START_DATA_PATH );
}

/**
 * @brief Test version to see if it matches.
 */
static void ndata_testVersion (void)
{
   size_t size;
   char *buf, cbuf[PATH_MAX];
   int diff;

   if (!ndata_found())
      ERR( _("Unable to find game data. You may need to install, specify a datapath, or run using naev.sh (if developing).") );

   /* Parse version. */
   buf = ndata_read( "VERSION", &size );
   for (size_t i=0; i<MIN(size,PATH_MAX-1); i++)
      cbuf[i] = buf[i];
   cbuf[MIN(size-1,PATH_MAX-1)] = '\0';
   diff = naev_versionCompare( cbuf );
   if (diff != 0) {
      WARN( _("ndata version inconsistency with this version of Naev!") );
      WARN( _("Expected ndata version %s got %s."), naev_version( 0 ), cbuf );
      if (ABS(diff) > 2)
         ERR( _("Please get a compatible ndata version!") );
      if (ABS(diff) > 1)
         WARN( _("Naev will probably crash now as the versions are probably not compatible.") );
   }
   free( buf );
}

/**
 * @brief Gets Naev's data path (for user data such as saves and screenshots)
 */
void ndata_setupWriteDir (void)
{
   /* Global override is set. */
   if (conf.datapath) {
      PHYSFS_setWriteDir( conf.datapath );
      return;
   }
#if MACOS
   /* For historical reasons predating physfs adoption, this case is different. */
   PHYSFS_setWriteDir( PHYSFS_getPrefDir( ".", "org.naev.Naev" ) );
#else
   PHYSFS_setWriteDir( PHYSFS_getPrefDir( ".", "naev" ) );
#endif /* MACOS */
   if (PHYSFS_getWriteDir() == NULL) {
      WARN(_("Cannot determine data path, using current directory."));
      PHYSFS_setWriteDir( "./naev/" );
   }
}

/**
 * @brief Sets up the PhysicsFS search path.
 */
void ndata_setupReadDirs (void)
{
   char buf[ PATH_MAX ];

   if ( conf.ndata != NULL && PHYSFS_mount( conf.ndata, NULL, 1 ) )
      LOG(_("Added datapath from conf.lua file: %s"), conf.ndata);

#if MACOS
   if ( !ndata_found() && macos_isBundle() && macos_resourcesPath( buf, PATH_MAX-4 ) >= 0 && strncat( buf, "/dat", 4 ) ) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }
#endif /* MACOS */

   if ( !ndata_found() && env.isAppImage && nfile_concatPaths( buf, PATH_MAX, env.appdir, PKGDATADIR, "dat" ) >= 0 ) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   if (!ndata_found() && nfile_concatPaths( buf, PATH_MAX, PKGDATADIR, "dat" ) >= 0) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   if (!ndata_found() && nfile_concatPaths( buf, PATH_MAX, PHYSFS_getBaseDir(), "dat" ) >= 0) {
      LOG(_("Trying default datapath: %s"), buf);
      PHYSFS_mount( buf, NULL, 1 );
   }

   PHYSFS_mount( PHYSFS_getWriteDir(), NULL, 0 );

   /* Load plugins I guess. */
   plugin_init();

   ndata_testVersion();
}

/**
 * @brief Reads a file from the ndata (will be NUL terminated).
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
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
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
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. TODO: Don't assume this is always possible? */
   len = PHYSFS_fileLength( file );
   if ( len == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
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
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
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
   char **files = array_create( char * );
   PHYSFS_enumerate( path, ndata_enumerateCallback, &files );
   /* Ensure unique. PhysicsFS can enumerate a path twice if it's in multiple components of a union. */
   qsort( files, array_size(files), sizeof(char*), strsort );
   for (int i=0; i+1<array_size(files); i++)
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
   size_t dir_len;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   SDL_asprintf( &path, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat )) {
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
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

/**
 * @brief Backup a file, if it exists.
 *
 *    @param path PhysicsFS relative pathname to back up.
 *    @return 0 on success, or if file does not exist, -1 on error.
 */
int ndata_backupIfExists( const char *path )
{
   char backup[ PATH_MAX ];

   if (path == NULL)
      return -1;

   if (!PHYSFS_exists( path ))
      return 0;

   snprintf(backup, sizeof(backup), "%s.backup", path);

   return ndata_copyIfExists( path, backup );
}

/**
 * @brief Copy a file, if it exists.
 *
 *    @param file1 PhysicsFS relative pathname to copy from.
 *    @param file2 PhysicsFS relative pathname to copy to.
 *    @return 0 on success, or if file1 does not exist, -1 on error.
 */
int ndata_copyIfExists( const char* file1, const char* file2 )
{
   PHYSFS_File *f_in, *f_out;
   char buf[ 8*1024 ];
   PHYSFS_sint64 lr, lw;

   if (file1 == NULL)
      return -1;

   /* Check if input file exists */
   if (!PHYSFS_exists(file1))
      return 0;

   /* Open files. */
   f_in  = PHYSFS_openRead( file1 );
   f_out = PHYSFS_openWrite( file2 );
   if ((f_in==NULL) || (f_out==NULL)) {
      WARN( _("Failure to copy '%s' to '%s': %s"), file1, file2,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      if (f_in!=NULL)
         PHYSFS_close(f_in);
      return -1;
   }

   /* Copy data over. */
   do {
      lr = PHYSFS_readBytes( f_in, buf, sizeof(buf) );
      if (lr == -1)
         goto err;
      else if (!lr) {
         if (PHYSFS_eof( f_in ))
            break;
         goto err;
      }

      lw = PHYSFS_writeBytes( f_out, buf, lr );
      if (lr != lw)
         goto err;
   } while (lr > 0);

   /* Close files. */
   PHYSFS_close( f_in );
   PHYSFS_close( f_out );

   return 0;

err:
   WARN( _("Failure to copy '%s' to '%s': %s"), file1, file2,
         _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
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
   for (i=strlen(path)-1; i>0; i--)
      if (path[i] == '.')
         break;
   if (i<=0)
      return 0;
   return strcmp( &path[i+1], ext )==0;
}

/**
 * @brief Tries to see if a file is in a default path before seeing if it is an absolute path.
 *
 *    @param[out] path Path found.
 *    @param len Length of path.
 *    @param default_path Default path to look in.
 *    @param filename Name of the file to look for.
 *    @return 1 if found.
 */
int ndata_getPathDefault( char *path, int len, const char *default_path, const char *filename )
{
   PHYSFS_Stat path_stat;
   snprintf( path, len, "%s%s", default_path, filename );
   if (PHYSFS_stat( path, &path_stat ) && (path_stat.filetype == PHYSFS_FILETYPE_REGULAR))
      return 1;
   snprintf( path, len, "%s", filename );
   if (PHYSFS_stat( path, &path_stat ) && (path_stat.filetype == PHYSFS_FILETYPE_REGULAR))
      return 1;
   return 0;
}
