/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nfile.c
 *
 * @brief Handles read/write abstractions to the users directory.
 *
 * @todo Add support for Windows and macOS.
 */
/** @cond */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <SDL3/SDL_stdinc.h>

#include <errno.h>
#include <libgen.h> /* dirname / basename */
#if HAS_POSIX
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* HAS_POSIX */
#if SDL_PLATFORM_WIN32
#include <windows.h>
#endif /* SDL_PLATFORM_WIN32 */
/** @endcond */

#include "nfile.h"

#include "array.h"
#include "conf.h"
#if SDL_PLATFORM_MACOS
#include "glue_macos.h"
#endif /* SDL_PLATFORM_MACOS */
#include "log.h"

#if HAS_UNIX && !SDL_PLATFORM_MACOS
//! http://n.ethz.ch/student/nevillm/download/libxdg-basedir/doc/basedir_8c_source.html

/**
 * Get value of an environment variable.
 * Sets @c errno to @c EINVAL if variable is not set or empty.
 *    @param name Name of environment variable.
 *    @return The environment variable or NULL if an error occurs.
 */
static const char *xdgGetEnv( const char *name )
{
   const char *env = SDL_getenv( name );
   if ( ( env != NULL ) && ( env[0] != '\0' ) )
      return env;
   /* What errno signifies missing env var? */
   errno = EINVAL;
   return NULL;
}

/**
 * Duplicate an environment variable.
 * Sets @c errno to @c ENOMEM if unable to allocate duplicate string.
 * Sets @c errno to @c EINVAL if variable is not set or empty.
 *    @return The duplicated string or NULL if an error occurs.
 */
static char *xdgEnvDup( const char *name )
{
   const char *env = xdgGetEnv( name );
   if ( env != NULL )
      return strdup( env );
   return NULL;
}

/**
 * Get a home directory from the environment or a fallback relative to @c
 * \$HOME. Sets @c errno to @c ENOMEM if unable to allocate duplicate string.
 * Sets @c errno to @c EINVAL if variable is not set or empty.
 *    @param envname Name of environment variable.
 *    @param relativefallback Path starting with "/" and relative to @c \$HOME
 * to use as fallback.
 *    @return The home directory path or @c NULL of an error occurs.
 */
static char *xdgGetRelativeHome( const char *envname,
                                 const char *relativefallback )
{
   char *relhome = xdgEnvDup( envname );
   if ( ( relhome == NULL ) && ( errno != ENOMEM ) ) {
      const char  *home;
      unsigned int homelen;
      errno = 0;
      home  = xdgGetEnv( "HOME" );
      if ( home == NULL )
         return NULL;
      homelen = strlen( home );
      unsigned int fallbacklength;
      fallbacklength = strlen( relativefallback );
      relhome        = malloc( homelen + fallbacklength + 1 );
      if ( relhome == NULL )
         return NULL;
      memcpy( relhome, home, homelen );
      memcpy( &relhome[homelen], relativefallback, fallbacklength + 1 );
      relhome[homelen + fallbacklength] = '\0'; /* Just in case. */
   }
   return relhome;
}
#endif /* HAS_UNIX && !SDL_PLATFORM_MACOS */

static char naev_configPath[PATH_MAX] = "\0"; /**< Store Naev's config path. */
/**
 * @brief Gets Naev's configuration path (for user preferences such as
 * "conf.lua")
 *
 *    @return The xdg configuration path.
 */
const char *nfile_configPath( void )
{
   if ( naev_configPath[0] == '\0' ) {
      /* Global override is set. */
      if ( conf.datapath ) {
         snprintf( naev_configPath, sizeof( naev_configPath ), "%s/",
                   conf.datapath );
         return naev_configPath;
      }
#if SDL_PLATFORM_MACOS
      if ( macos_configPath( naev_configPath, sizeof( naev_configPath ) ) !=
           0 ) {
         WARN( _( "Cannot determine config path, using current directory." ) );
         snprintf( naev_configPath, sizeof( naev_configPath ), "./naev/" );
      }
#elif HAS_UNIX
      char *path = xdgGetRelativeHome( "XDG_CONFIG_HOME", "/.config" );
      if ( path == NULL ) {
         WARN( _( "$XDG_CONFIG_HOME isn't set, using current directory." ) );
         path = strdup( "." );
      }

      snprintf( naev_configPath, sizeof( naev_configPath ), "%s/naev/", path );
      free( path );
#elif SDL_PLATFORM_WIN32
      char *path = SDL_getenv( "APPDATA" );
      if ( path == NULL ) {
         WARN( _( "%%APPDATA%% isn't set, using current directory." ) );
         path = ".";
      }
      snprintf( naev_configPath, sizeof( naev_configPath ), "%s/naev/", path );
#else
      // TODO just use SDL instead of custom implementations?
      char *prefpath = SDL_GetPrefPath( "Naev DevTeam", "Naev" );
      strncpy( naev_configPath, sizeof( naev_configPath ), prefpath );
      SDL_free( prefpath );
#endif
   }

   return naev_configPath;
}

static char naev_cachePath[PATH_MAX] = "\0"; /**< Store Naev's cache path. */
/**
 * @brief Gets Naev's cache path (for cached data such as generated textures)
 *
 *    @return The xdg cache path.
 */
const char *nfile_cachePath( void )
{
   if ( naev_cachePath[0] == '\0' ) {
      /* Global override is set. */
      if ( conf.datapath ) {
         snprintf( naev_cachePath, sizeof( naev_cachePath ), "%s/",
                   conf.datapath );
         return naev_cachePath;
      }
#if SDL_PLATFORM_MACOS
      if ( macos_cachePath( naev_cachePath, sizeof( naev_cachePath ) ) != 0 ) {
         WARN( _( "Cannot determine cache path, using current directory." ) );
         snprintf( naev_cachePath, sizeof( naev_cachePath ), "./naev/" );
      }
#elif HAS_UNIX
      char *path = xdgGetRelativeHome( "XDG_CACHE_HOME", "/.cache" );
      if ( path == NULL ) {
         WARN( _( "$XDG_CACHE_HOME isn't set, using current directory." ) );
         path = strdup( "." );
      }

      snprintf( naev_cachePath, sizeof( naev_cachePath ), "%s/naev/", path );
      free( path );
#elif SDL_PLATFORM_WIN32
      char *path = SDL_getenv( "APPDATA" );
      if ( path == NULL ) {
         WARN( _( "%%APPDATA%% isn't set, using current directory." ) );
         path = ".";
      }
      snprintf( naev_cachePath, sizeof( naev_cachePath ), "%s/naev/", path );
#else
      // TODO just use SDL instead of custom implementations?
      char *prefpath = SDL_GetPrefPath( "Naev DevTeam", "Naev" );
      strncpy( naev_cachePath, sizeof( naev_cachePath ), prefpath );
      SDL_free( prefpath );
#endif
   }

   return naev_cachePath;
}

/**
 * @brief Creates a directory if it doesn't exist.
 *
 *    @param path Path to create directory if it doesn't exist.
 *    @return 0 on success.
 */
int nfile_dirMakeExist( const char *path )
{
   if ( path == NULL )
      return -1;

   /* Check if it exists. */
   if ( nfile_dirExists( path ) )
      return 0;

   if ( !SDL_CreateDirectory( path ) ) {
      WARN( _( "Dir '%s' does not exist and unable to create: %s" ), path,
            strerror( errno ) );
      return -1;
   }

   return 0;
}

/**
 * @brief Checks to see if a directory exists.
 *
 * @param path Path to directory
 * @return 1 on exists, 0 otherwise
 */
int nfile_dirExists( const char *path )
{
   DIR *d;

   if ( path == NULL )
      return -1;

   d = opendir( path );
   if ( d == NULL )
      return 0;
   closedir( d );
   return 1;
}

/**
 * @brief Checks to see if a file exists.
 *
 *    @param path string pointing to the file to check for existence.
 *    @return 1 if file exists, 0 if it doesn't or -1 on error.
 */
int nfile_fileExists( const char *path )
{
   struct stat buf;

   if ( path == NULL )
      return -1;

   if ( stat( path, &buf ) == 0 ) /* stat worked, file must exist */
      return 1;

   /* ANSI C89 compliant method here for reference. Not as precise as stat.
   FILE *f = fopen(file, "rb");
   if (f != NULL) {
      fclose(f);
      return 1;
   }
   */

   return 0;
}

/**
 * @brief Backup a file, if it exists.
 *
 *    @param path printf formatted string pointing to the file to backup.
 *    @return 0 on success, or if file does not exist, -1 on error.
 */
int nfile_backupIfExists( const char *path )
{
   char backup[PATH_MAX];

   if ( path == NULL )
      return -1;

   if ( !nfile_fileExists( path ) )
      return 0;

   snprintf( backup, sizeof( backup ), "%s.backup", path );

   return nfile_copyIfExists( path, backup );
}

/**
 * @brief Copy a file, if it exists.
 *
 *    @param file1 Filename to copy from.
 *    @param file2 Filename to copy to.
 *    @return 0 on success, or if file1 does not exist, -1 on error.
 */
int nfile_copyIfExists( const char *file1, const char *file2 )
{
   FILE  *f_in, *f_out;
   char   buf[8 * 1024];
   size_t lr, lw;

   if ( file1 == NULL )
      return -1;

   /* Check if input file exists */
   if ( !nfile_fileExists( file1 ) )
      return 0;

   /* Open files. */
   f_in  = fopen( file1, "rb" );
   f_out = fopen( file2, "wb" );
   if ( ( f_in == NULL ) || ( f_out == NULL ) ) {
      WARN( _( "Failure to copy '%s' to '%s': %s" ), file1, file2,
            strerror( errno ) );
      if ( f_in != NULL )
         fclose( f_in );
      if ( f_out != NULL )
         fclose( f_out );
      return -1;
   }

   /* Copy data over. */
   do {
      lr = fread( buf, 1, sizeof( buf ), f_in );
      if ( ferror( f_in ) )
         goto err;
      else if ( !lr ) {
         if ( feof( f_in ) )
            break;
         goto err;
      }

      lw = fwrite( buf, 1, lr, f_out );
      if ( ferror( f_out ) || ( lr != lw ) )
         goto err;
   } while ( lr > 0 );

   /* Close files. */
   fclose( f_in );
   fclose( f_out );

   return 0;

err:
   WARN( _( "Failure to copy '%s' to '%s': %s" ), file1, file2,
         strerror( errno ) );
   fclose( f_in );
   fclose( f_out );

   return -1;
}

/**
 * @brief Tries to read a file.
 *
 *    @param filesize Stores the size of the file.
 *    @param path Path of the file.
 *    @return The file data.
 */
char *nfile_readFile( size_t *filesize, const char *path )
{
   int         n;
   char       *buf;
   FILE       *file;
   int         len;
   size_t      pos;
   struct stat path_stat;

   if ( path == NULL ) {
      *filesize = 0;
      return NULL;
   }

   if ( stat( path, &path_stat ) ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            strerror( errno ) );
      *filesize = 0;
      return NULL;
   }

   if ( !S_ISREG( path_stat.st_mode ) ) {
      WARN( _( "Error occurred while opening '%s': It is not a regular file" ),
            path );
      *filesize = 0;
      return NULL;
   }

   /* Open file. */
   file = fopen( path, "rb" );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            strerror( errno ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. */
   if ( fseek( file, 0L, SEEK_END ) == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            strerror( errno ) );
      fclose( file );
      *filesize = 0;
      return NULL;
   }
   len = ftell( file );
   if ( fseek( file, 0L, SEEK_SET ) == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            strerror( errno ) );
      fclose( file );
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len + 1 );
   if ( buf == NULL ) {
      WARN( _( "Out of Memory" ) );
      fclose( file );
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while ( n < len ) {
      pos = fread( &buf[n], 1, len - n, file );
      if ( pos <= 0 ) {
         WARN( _( "Error occurred while reading '%s': %s" ), path,
               strerror( errno ) );
         fclose( file );
         *filesize = 0;
         free( buf );
         return NULL;
      }
      n += pos;
   }

   /* Close the file. */
   fclose( file );

   *filesize = len;
   return buf;
}

/**
 * @brief Tries to create the file if it doesn't exist.
 *
 *    @param path Path of the file to create.
 */
int nfile_touch( const char *path )
{
   FILE *f;

   if ( path == NULL )
      return -1;

   /* Try to open the file, C89 compliant, but not as precise as stat. */
   f = fopen( path, "a+b" );
   if ( f == NULL ) {
      WARN( _( "Unable to touch file '%s': %s" ), path, strerror( errno ) );
      return -1;
   }

   fclose( f );
   return 0;
}

/**
 * @brief Tries to write a file.
 *
 *    @param data Pointer to the data to write.
 *    @param len The size of data.
 *    @param path Path of the file.
 *    @return 0 on success, -1 on error.
 */
int nfile_writeFile( const char *data, size_t len, const char *path )
{
   size_t n;
   FILE  *file;
   size_t pos;

   if ( path == NULL )
      return -1;

   /* Open file. */
   file = fopen( path, "wb" );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            strerror( errno ) );
      return -1;
   }

   /* Write the file. */
   n = 0;
   while ( n < len ) {
      pos = fwrite( &data[n], 1, len - n, file );
      if ( pos <= 0 ) {
         WARN( _( "Error occurred while writing '%s': %s" ), path,
               strerror( errno ) );
         fclose( file ); /* don't care about further errors */
         return -1;
      }
      n += pos;
   }

   /* Close the file. */
   if ( fclose( file ) == EOF ) {
      WARN( _( "Error occurred while closing '%s': %s" ), path,
            strerror( errno ) );
      return -1;
   }

   return 0;
}

/**
 * @brief Checks to see if a character is used to separate files in a path.
 *
 *    @param c Character to check.
 *    @return 1 if is a separator, 0 otherwise.
 */
int nfile_isSeparator( uint32_t c )
{
   if ( c == '/' )
      return 1;
#if SDL_PLATFORM_WIN32
   else if ( c == '\\' )
      return 1;
#endif /* SDL_PLATFORM_WIN32 */
   return 0;
}

int _nfile_concatPaths( char buf[static 1], int maxLength,
                        const char path[static 1], ... )
{
   char       *bufPos;
   char       *bufEnd;
   const char *section;
   va_list     ap;

   bufPos = buf;
   bufEnd = buf + maxLength;
   va_start( ap, path );
   section = path;

#if DEBUGGING
   if ( section == NULL )
      WARN( _( "First argument to nfile_concatPaths was NULL. This is probably "
               "an error." ) );
#endif /* DEBUGGING */

   do {
      // End of arg list?
      if ( section == NULL )
         break;

      if ( bufPos > buf ) {
         // Make sure there's a path seperator.
         if ( bufPos[-1] != '/' ) {
            bufPos[0] = '/';
            bufPos += 1;
         }
         // But not too many path seperators.
         if ( *section == '/' )
            section += 1;
      }

      // Copy this section
      bufPos = memccpy( bufPos, section, '\0', bufEnd - bufPos );
      if ( bufPos == NULL )
         break;

      // Next path section
      section = va_arg( ap, char * );
   } while (
      bufPos-- <
      bufEnd ); // Rewind after compare so we're pointing at the NULL character.
   va_end( ap );

   // Did we run out of space?
   if ( section != NULL )
      return -1;

   return bufPos - buf;
}

/**
 * @brief Simplifies the path removing things like ".." or consecutive "/".
 *
 *    @param path Path to simplify. Gets overwritten.
 *    @return 0 on success.
 */
int nfile_simplifyPath( char path[static 1] )
{
   char **dirnames = array_create( char * );
   char  *saveptr  = NULL;
   size_t n        = strlen( path );
   int    absolute = ( path[0] == '/' );
   char  *token    = SDL_strtok_r( path, "/", &saveptr );

   while ( token != NULL ) {
      /* Skip noop. */
      if ( ( strcmp( token, "" ) == 0 ) || ( strcmp( token, "." ) == 0 ) ) {
         token = SDL_strtok_r( NULL, "/", &saveptr );
         continue;
      }

      /* Go up if ".." */
      if ( strcmp( token, ".." ) == 0 ) {
         int dn = array_size( dirnames );
         if ( dn > 0 ) {
            free( dirnames[dn - 1] );
            array_erase( &dirnames, &dirnames[dn - 1], &dirnames[dn] );
         }
      } else {
         array_push_back( &dirnames, strdup( token ) );
      }

      /* On to the next one. */
      token = SDL_strtok_r( NULL, "/", &saveptr );
   }

   /* If nothing, we're empty. */
   if ( array_size( dirnames ) <= 0 ) {
      array_free( dirnames );
      path[0] = '\0';
      return 0;
   }

   /* Build back the path, assume 'path' is smaller than simplified one. */
   size_t s = 0;
   for ( int i = 0; i < array_size( dirnames ); i++ ) {
      char *ds = dirnames[i];
      if ( ( absolute || ( s > 0 ) ) && ( s < n ) )
         path[s++] = '/';
      for ( size_t j = 0; j < strlen( ds ); j++ )
         if ( s < n )
            path[s++] = ds[j];
      free( ds );
   }
   path[s] = '\0';
   array_free( dirnames );
   return 0;
}
