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


#include "nfile.h"

#include "naev.h"
#include "conf.h"

#include "nstring.h"
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if HAS_POSIX
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#endif /* HAS_POSIX */
#if HAS_MACOS
#include "glue_macos.h"
#endif /* HAS_MACOS */
#if HAS_WIN32
#include <windows.h>
#endif /* HAS_WIN32 */

#include "array.h"
#include "log.h"


/**
 * @TODO try to use SDL_GetPrefPath and SDL_GetBasePath when possible.
 */


#define BLOCK_SIZE      128*1024 /**< 128 kilobytes. */


/**
 * @brief Struct containing a file's name and stat structure.
 */
typedef struct filedata {
   char *name;
   struct stat stat;
} filedata_t;


static int nfile_sortCompare( const void *p1, const void *p2 );


#if HAS_UNIX && !HAS_MACOS
//! http://n.ethz.ch/student/nevillm/download/libxdg-basedir/doc/basedir_8c_source.html

/**
 * Get value of an environment variable.
 * Sets @c errno to @c EINVAL if variable is not set or empty.
 *    @param name Name of environment variable.
 *    @return The environment variable or NULL if an error occurs.
 */
static char* xdgGetEnv(const char *name)
{
    char *env = SDL_getenv(name);
    if ((env != NULL) && (env[0] != '\0'))
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
static char* xdgEnvDup(const char *name)
{
    const char *env;
    env = xdgGetEnv( name );
    if (env != NULL)
        return strdup(env);
     return NULL;
}

/**
 * Get a home directory from the environment or a fallback relative to @c \$HOME.
 * Sets @c errno to @c ENOMEM if unable to allocate duplicate string.
 * Sets @c errno to @c EINVAL if variable is not set or empty.
 *    @param envname Name of environment variable.
 *    @param relativefallback Path starting with "/" and relative to @c \$HOME to use as fallback.
 *    @return The home directory path or @c NULL of an error occurs.
 */
static char * xdgGetRelativeHome( const char *envname, const char *relativefallback )
{
    char *relhome;
    relhome = xdgEnvDup(envname);
    if ((relhome == NULL) && (errno != ENOMEM)) {
        errno = 0;
        const char *home;
        unsigned int homelen;
        home = xdgGetEnv( "HOME" );
        if (home == NULL)
            return NULL;
        homelen = strlen(home);
        unsigned int fallbacklength;
        fallbacklength = strlen( relativefallback );
        relhome = malloc( homelen + fallbacklength + 1 );
        if (relhome == NULL)
           return NULL;
        memcpy( relhome, home, homelen );
        memcpy( &relhome[ homelen ], relativefallback, fallbacklength + 1 );
        relhome[ homelen + fallbacklength ] = '\0'; /* Just in case. */
    }
    return relhome;
}
#endif

static char naev_dataPath[PATH_MAX] = "\0"; /**< Store Naev's data path. */
/**
 * @brief Gets Naev's data path (for user data such as saves and screenshots)
 *
 *    @return The xdg data path.
 */
const char* nfile_dataPath (void)
{
    if (naev_dataPath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_dataPath, PATH_MAX, "%s/", conf.datapath );
           return naev_dataPath;
        }
#if HAS_MACOS
        if (macos_dataPath( naev_dataPath, PATH_MAX ) != 0) {
           WARN(_("Cannot determine data path, using current directory."));
           nsnprintf( naev_dataPath, PATH_MAX, "./naev/" );
        }
#elif HAS_UNIX
        char *path = xdgGetRelativeHome( "XDG_DATA_HOME", "/.local/share" );
        if (path == NULL) {
            WARN(_("$XDG_DATA_HOME isn't set, using current directory."));
            path = strdup(".");
        }

        nsnprintf( naev_dataPath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      char *path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN(_("%%APPDATA%% isn't set, using current directory."));
         path = ".";
      }
      nsnprintf( naev_dataPath, PATH_MAX, "%s/naev/", path );
#else
#error _("Feature needs implementation on this Operating System for Naev to work.")
#endif
    }

    return naev_dataPath;
}


static char naev_configPath[PATH_MAX] = "\0"; /**< Store Naev's config path. */
/**
 * @brief Gets Naev's config path (for user preferences such as conf.lua)
 *
 *    @return The xdg config path.
 */
const char* nfile_configPath (void)
{
    if (naev_configPath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_configPath, PATH_MAX, "%s/", conf.datapath );
           return naev_configPath;
        }
#if HAS_MACOS
        if (macos_configPath( naev_configPath, PATH_MAX ) != 0) {
           WARN(_("Cannot determine config path, using current directory."));
           nsnprintf( naev_configPath, PATH_MAX, "./naev/" );
        }
#elif HAS_UNIX
        char *path = xdgGetRelativeHome( "XDG_CONFIG_HOME", "/.config" );
        if (path == NULL) {
            WARN(_("$XDG_CONFIG_HOME isn't set, using current directory."));
            path = strdup(".");
        }

        nsnprintf( naev_configPath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      char *path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN(_("%%APPDATA%% isn't set, using current directory."));
         path = ".";
      }
      nsnprintf( naev_configPath, PATH_MAX, "%s/naev/", path );
#else
#error "Feature needs implementation on this Operating System for Naev to work."
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
const char* nfile_cachePath (void)
{
    if (naev_cachePath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_cachePath, PATH_MAX, "%s/", conf.datapath );
           return naev_cachePath;
        }
#if HAS_MACOS
        if (macos_cachePath( naev_cachePath, PATH_MAX ) != 0) {
           WARN(_("Cannot determine cache path, using current directory."));
           nsnprintf( naev_cachePath, PATH_MAX, "./naev/" );
        }
#elif HAS_UNIX
        char *path = xdgGetRelativeHome( "XDG_CACHE_HOME", "/.cache" );
        if (path == NULL) {
            WARN(_("$XDG_CACHE_HOME isn't set, using current directory."));
            path = strdup(".");
        }

        nsnprintf( naev_cachePath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      char *path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN(_("%%APPDATA%% isn't set, using current directory."));
         path = ".";
      }
      nsnprintf( naev_cachePath, PATH_MAX, "%s/naev/", path );
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
    }

    return naev_cachePath;
}


static char dirname_buf[PATH_MAX];
/**
 * @brief Portable version of dirname.
 *
 * Unlike most implementations of dirname, this does not modify path, instead
 * it returns a modified buffer. The contents of the returned pointer can change
 * in subsequent calls.
 */
const char *_nfile_dirname( const char *path )
{
#if HAS_POSIX
   strncpy( dirname_buf, path, sizeof(dirname_buf) );
   dirname_buf[PATH_MAX-1]='\0';
   return dirname( dirname_buf );
#elif HAS_WIN32
   int i;
   for (i=strlen(path)-1; i>=0; i--)
      if (nfile_isSeparator( path[i] ))
         break;

   /* Nothing found. */
   if (i<=0)
      return path;

   /* New dirname. */
   nsnprintf( dirname_buf, MIN(sizeof(dirname_buf), (size_t)(i+1)),  path );
   return dirname_buf;
#else
#error "Functionality not implemented for your OS."
#endif /* HAS_POSIX */
}


#if HAS_POSIX
#define MKDIR mkdir( opath, mode )
static int mkpath( const char *path, mode_t mode )
#elif HAS_WIN32
#define MKDIR !CreateDirectory( opath, NULL )
static int mkpath( const char *path )
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
{
   char opath[PATH_MAX];
   char *p;
   size_t len;
   int ret;

   if (path == NULL)
      return 0;

   strncpy( opath, path, sizeof(opath) );
   opath[ PATH_MAX-1 ] = '\0';
   len = strlen(opath);

   p = &opath[len-1];
   if (nfile_isSeparator(p[0])) {
      p[0] = '\0';
      p--;
   }

   // Traverse up until we find a directory that exists.
   for (; p >= opath; p--) {
      if (nfile_isSeparator(p[0])) {
         p[0] = '\0';
         if (nfile_dirExists(opath)) {
            p[0] = '/';
            break;
         }
         p[0] = '/';
      }
   }
   // This skips the directory that exists, or puts us
   // back at the start if the loop fell through.
   p++;

   // Traverse down, creating directories.
   for (; p[0] != '\0'; p++) {
      if (nfile_isSeparator(p[0])) {
         p[0] = '\0';
         ret = MKDIR;
         if (ret)
            return ret;
         p[0] = '/';
      }
   }

   // Create the final directory.
   if (!nfile_dirExists(opath)) {
      ret = MKDIR;
      if (ret)
         return ret;
   }

   return 0;
}
#undef MKDIR


/**
 * @see nfile_dirMakeExist
 */
int _nfile_dirMakeExist( const char *path )
{
   if ( path == NULL )
      return -1;

   /* Check if it exists. */
   if ( nfile_dirExists( path ) )
      return 0;

#if HAS_POSIX
   if ( mkpath( path, S_IRWXU | S_IRWXG | S_IRWXO ) < 0 ) {
#elif HAS_WIN32
   if ( mkpath( path ) < 0 ) {
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
      WARN( _( "Dir '%s' does not exist and unable to create: %s" ), path, strerror( errno ) );
      return -1;
   }

   return 0;
}


int _nfile_dirExists( const char *path )
{
   DIR *d;

   if (path == NULL)
      return -1;

   d = opendir( path );
   if ( d == NULL )
      return 0;
   closedir(d);
   return 1;
}


/**
 * @brief Checks to see if a file exists.
 *
 *    @param path string pointing to the file to check for existence.
 *    @return 1 if file exists, 0 if it doesn't or -1 on error.
 */
int _nfile_fileExists( const char *path )
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
int _nfile_backupIfExists( const char *path )
{
   char backup[ PATH_MAX ];

   if ( path == NULL )
      return -1;

   if ( !nfile_fileExists( path ) )
      return 0;

   nsnprintf(backup, PATH_MAX, "%s.backup", path);

   return nfile_copyIfExists( path, backup );
}


/**
 * @brief Copy a file, if it exists.
 *
 *    @param file1 Filename to copy from.
 *    @param file2 Filename to copy to.
 *    @return 0 on success, or if file1 does not exist, -1 on error.
 */
int nfile_copyIfExists( const char* file1, const char* file2 )
{
   FILE *f_in, *f_out;
   char buf[ 8*1024 ];
   size_t lr, lw;

   if (file1 == NULL)
      return -1;

   /* Check if input file exists */
   if (!nfile_fileExists(file1))
      return 0;

   /* Open files. */
   f_in  = fopen( file1, "rb" );
   f_out = fopen( file2, "wb" );
   if ((f_in==NULL) || (f_out==NULL)) {
      WARN( _("Failure to copy '%s' to '%s': %s"), file1, file2, strerror(errno) );
      if (f_in!=NULL)
         fclose(f_in);
      return -1;
   }

   /* Copy data over. */
   do {
      lr = fread( buf, 1, sizeof(buf), f_in );
      if (ferror(f_in))
         goto err;
      else if (!lr) {
         if (feof(f_in))
            break;
         goto err;
      }

      lw = fwrite( buf, 1, lr, f_out );
      if (ferror(f_out) || (lr != lw))
         goto err;
   } while (lr > 0);

   /* Close files. */
   fclose( f_in );
   fclose( f_out );

   return 0;

err:
   WARN( _("Failure to copy '%s' to '%s': %s"), file1, file2, strerror(errno) );
   fclose( f_in );
   fclose( f_out );

   return -1;
}


/**
 * @brief Lists all the visible files in a directory.
 *
 * Should also sort by last modified but that's up to the OS in question.
 * Paths are relative to base directory.
 *
 *    @param[out] nfiles Returns how many files there are.
 *    @param path Directory to read.
 */
char **_nfile_readDir( size_t *nfiles, const char *path )
{
   char file[ PATH_MAX ];
   char **files;
   filedata_t *filedata;

   ( *nfiles ) = 0;
   if ( path == NULL )
      return NULL;

   size_t i;
   DIR *d;
   struct dirent *dir;
   char *name;
   size_t mfiles;
   struct stat sb;

   d = opendir( path );
   if ( d == NULL )
      return NULL;

   mfiles      = 128;
   filedata    = malloc(sizeof(filedata_t) * mfiles);

   /* Get the file list */
   while ((dir = readdir(d)) != NULL) {
      name = dir->d_name;

      /* Skip hidden directories */
      if (name[0] == '.')
         continue;

      /* Stat the file */
      nsnprintf( file, PATH_MAX, "%s/%s", path, name );
      if ( stat( file, &sb ) == -1 )
         continue; /* Unable to stat */

      /* Enough memory? */
      if ((*nfiles)+1 > mfiles) {
         mfiles *= 2;
         filedata = realloc( filedata, sizeof(filedata_t) * mfiles );
      }

      /* Write the information */
      filedata[(*nfiles)].name = strdup(name);
      filedata[(*nfiles)].stat = sb;
      (*nfiles)++;
   }

   closedir(d);

   /* Sort by last changed date */
   if ((*nfiles) > 0) {
      qsort(filedata, *nfiles, sizeof(filedata_t), nfile_sortCompare);

      files = malloc(sizeof(char*) * (*nfiles));
      for (i=0; i<(*nfiles); i++) {
         files[i] = strdup(filedata[i].name);
         free(filedata[i].name);
      }
   }
   else
      files = NULL;

   free(filedata);
   filedata = NULL;

   return files;
}


/**
 * @brief Lists all the visible files in a directory, at any depth.
 *
 * Should also sort by last modified but that's up to the OS in question.
 * Paths are relative to base directory.
 *
 *    @param base_dir Root of the search (not part of returned file paths).
 *    @param sub_dir Subdirectory being searched (included in returend file paths).
 *    @return Array of (allocated) file paths relative to base_dir.
 */
char **_nfile_readDirRecursive( char ***_files, const char *base_dir, const char *sub_dir )
{
   char child_path_buf[ PATH_MAX ];
   char *child_path;
   char **files;
   char **cur_path_contents;
   size_t n_cur_path_contents;

   if ( _files == NULL )
      files = array_create( char * );
   else
      files = *_files;

   cur_path_contents = nfile_readDir( &n_cur_path_contents, base_dir, sub_dir );

   for ( size_t i = 0; i < n_cur_path_contents; i += 1 ) {
      if ( sub_dir == NULL )
         child_path = cur_path_contents[ i ];
      else if ( nfile_concatPaths( child_path_buf, PATH_MAX, sub_dir, cur_path_contents[ i ] ) < 0 ) {
         WARN( _( "Error while opening %s/%s: Path is too long" ), sub_dir, cur_path_contents[ i ] );
         for ( ; i < n_cur_path_contents; i += 1 )
            free( cur_path_contents[ i ] );
         free( cur_path_contents );
         return NULL;
      }
      else {
         child_path = child_path_buf;
      }
      if ( nfile_dirExists( base_dir, child_path ) ) {
         /* Iterate over children. */
         _nfile_readDirRecursive( &files, base_dir, child_path );
      }
      else {
         array_push_back( &files, strdup( child_path ) );
      }
      free( cur_path_contents[ i ] );
   }

   /* Clean up. */
   free( cur_path_contents );

   if ( _files != NULL )
      *_files = files;

   return files;
}


/**
 * @brief Tries to read a file.
 *
 *    @param filesize Stores the size of the file.
 *    @param path Path of the file.
 *    @return The file data.
 */
char *_nfile_readFile( size_t *filesize, const char *path )
{
   int n;
   char *buf;
   FILE *file;
   int len;
   size_t pos;
   struct stat path_stat;

   if ( path == NULL ) {
      *filesize = 0;
      return NULL;
   }

   if ( stat( path, &path_stat ) ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path, strerror( errno ) );
      *filesize = 0;
      return NULL;
   }

   if ( !S_ISREG( path_stat.st_mode ) ) {
      WARN( _( "Error occurred while opening '%s': It is not a regular file" ), path );
      *filesize = 0;
      return NULL;
   }

   /* Open file. */
   file = fopen( path, "rb" );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path, strerror( errno ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. */
   if ( fseek( file, 0L, SEEK_END ) == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path, strerror( errno ) );
      fclose( file );
      *filesize = 0;
      return NULL;
   }
   len = ftell( file );
   if ( fseek( file, 0L, SEEK_SET ) == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path, strerror( errno ) );
      fclose( file );
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len+1 );
   if (buf == NULL) {
      WARN(_("Out of Memory"));
      fclose(file);
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while ( n < len ) {
      pos = fread( &buf[ n ], 1, len - n, file );
      if ( pos <= 0 ) {
         WARN( _( "Error occurred while reading '%s': %s" ), path, strerror( errno ) );
         fclose( file );
         *filesize = 0;
         free(buf);
         return NULL;
      }
      n += pos;
   }

   /* Close the file. */
   fclose(file);

   *filesize = len;
   return buf;
}


/**
 * @brief Tries to create the file if it doesn't exist.
 *
 *    @param path Path of the file to create.
 */
int _nfile_touch( const char *path )
{
   FILE *f;

   if (path == NULL)
      return -1;

   /* Try to open the file, C89 compliant, but not as precise as stat. */
   f = fopen( path, "a+b" );
   if ( f == NULL ) {
      WARN( _( "Unable to touch file '%s': %s" ), path, strerror( errno ) );
      return -1;
   }

   fclose(f);
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
int _nfile_writeFile( const char *data, size_t len, const char *path )
{
   size_t n;
   FILE *file;
   size_t pos;

   if ( path == NULL )
      return -1;

   /* Open file. */
   file = fopen( path, "wb" );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path, strerror( errno ) );
      return -1;
   }

   /* Write the file. */
   n = 0;
   while ( n < len ) {
      pos = fwrite( &data[ n ], 1, len - n, file );
      if ( pos <= 0 ) {
         WARN( _( "Error occurred while writing '%s': %s" ), path, strerror( errno ) );
         fclose( file ); /* don't care about further errors */
         return -1;
      }
      n += pos;
   }

   /* Close the file. */
   if ( fclose( file ) == EOF ) {
      WARN( _( "Error occurred while closing '%s': %s" ), path, strerror( errno ) );
      return -1;
   }

   return 0;
}


/**
 * @brief Deletes a file.
 *
 *    @param file File to delete.
 *    @return 0 on success.
 */
int _nfile_delete( const char *file )
{
   if (unlink(file)) {
      WARN( _("Error deleting file %s"),file );
      return -1;
   }
   return 0;
}

/**
 * @brief Renames a file.
 *
 *    @param oldname Old name of the file.
 *    @param newname New name to set the file to.
 *    @return 0 on success.
 */
int nfile_rename( const char* oldname, const char* newname )
{
   if (!nfile_fileExists(oldname)) {
      WARN(_("Can not rename non existant file %s"),oldname);
      return -1;
   }
   if (newname == NULL) {
      WARN(_("Can not rename to NULL file name"));
      return -1;
   }
   if (nfile_fileExists( newname )) {
      WARN(_("Error renaming %s to %s. %s already exists"),oldname,newname,newname);
      return -1;
   }
   if (rename(oldname,newname))
      WARN(_("Error renaming %s to %s"),oldname,newname);
   return 0;
}


/**
 * @brief qsort compare function for files.
 */
static int nfile_sortCompare( const void *p1, const void *p2 )
{
   filedata_t *f1, *f2;

   f1 = (filedata_t*) p1;
   f2 = (filedata_t*) p2;

   if (f1->stat.st_mtime > f2->stat.st_mtime)
      return -1;
   else if (f1->stat.st_mtime < f2->stat.st_mtime)
      return +1;

   return strcmp( f1->name, f2->name );
}

/**
 * @brief Checks to see if a character is used to separate files in a path.
 *
 *    @param c Character to check.
 *    @return 1 if is a separator, 0 otherwise.
 */
int nfile_isSeparator( uint32_t c )
{
   if (c == '/')
      return 1;
#if HAS_WIN32
   else if (c == '\\')
      return 1;
#endif /* HAS_WIN32 */
   return 0;
}


int _nfile_concatPaths( char buf[static 1], int maxLength, const char path[static 1], ... )
{
   char *bufPos;
   char *bufEnd;
   const char *section;
   va_list ap;

   bufPos = buf;
   bufEnd = buf + maxLength;
   va_start( ap, path );
   section = path;

#if DEBUGGING
   if ( section == NULL )
      WARN( _( "First argument to nfile_concatPaths was NULL. This is probably an error." ) );
#endif

   do {
      // End of arg list?
      if ( section == NULL )
         break;

      if ( bufPos > buf ) {
         // Make sure there's a path seperator.
         if ( bufPos[ -1 ] != '/' ) {
            bufPos[ 0 ] = '/';
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
   } while ( bufPos-- < bufEnd ); // Rewind after compare so we're pointing at the NULL character.
   va_end( ap );

   // Did we run out of space?
   if ( section != NULL )
      return -1;

   return bufPos - buf;
}
