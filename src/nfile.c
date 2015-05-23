/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nfile.c
 *
 * @brief Handles read/write abstractions to the users directory.
 *
 * @todo Add support for Windows and Mac OS X.
 */


#include "nfile.h"

#include "naev.h"
#include "conf.h"

#include <stdio.h>
#include "nstring.h"
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#if HAS_POSIX
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#endif /* HAS_POSIX */
#if HAS_WIN32
#include <windows.h>
#endif /* HAS_WIN32 */

#include "log.h"



#define BLOCK_SIZE      128*1024 /**< 128 kilobytes. */


/**
 * @brief Struct containing a file's name and stat structure.
 */
typedef struct filedata {
   char *name;
   struct stat stat;
} filedata_t;


static int nfile_sortCompare( const void *p1, const void *p2 );


#if HAS_UNIX
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
    char *path;

    if (naev_dataPath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_dataPath, PATH_MAX, "%s/", conf.datapath );
           return naev_dataPath;
        }
#if HAS_UNIX
        path = xdgGetRelativeHome( "XDG_DATA_HOME", "/.local/share" );
        if (path == NULL) {
            WARN("$XDG_DATA_HOME isn't set, using current directory.");
            path = strdup(".");
        }

        nsnprintf( naev_dataPath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN("%%APPDATA%% isn't set, using current directory.");
         path = ".";
      }
      nsnprintf( naev_dataPath, PATH_MAX, "%s/naev/", path );
#else
#error "Feature needs implementation on this Operating System for Naev to work."
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
    char *path;

    if (naev_configPath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_configPath, PATH_MAX, "%s/", conf.datapath );
           return naev_configPath;
        }
#if HAS_UNIX
        path = xdgGetRelativeHome( "XDG_CONFIG_HOME", "/.config" );
        if (path == NULL) {
            WARN("$XDG_CONFIG_HOME isn't set, using current directory.");
            path = strdup(".");
        }

        nsnprintf( naev_configPath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN("%%APPDATA%% isn't set, using current directory.");
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
    char *path;

    if (naev_cachePath[0] == '\0') {
        /* Global override is set. */
        if (conf.datapath) {
           nsnprintf( naev_cachePath, PATH_MAX, "%s/", conf.datapath );
           return naev_cachePath;
        }
#if HAS_UNIX
        path = xdgGetRelativeHome( "XDG_CACHE_HOME", "/.cache" );
        if (path == NULL) {
            WARN("$XDG_CACHE_HOME isn't set, using current directory.");
            path = strdup(".");
        }

        nsnprintf( naev_cachePath, PATH_MAX, "%s/naev/", path );

        if (path != NULL) {
            free (path);
        }
#elif HAS_WIN32
      path = SDL_getenv("APPDATA");
      if (path == NULL) {
         WARN("%%APPDATA%% isn't set, using current directory.");
         path = ".";
      }
      nsnprintf( naev_cachePath, PATH_MAX, "%s/naev/", path );
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
    }

    return naev_cachePath;
}


#if HAS_WIN32
static char dirname_buf[PATH_MAX];
#endif /* HAS_WIN32 */
/**
 * @brief Portable version of dirname.
 */
char* nfile_dirname( char *path )
{
#if HAS_POSIX
   return dirname( path );
#elif HAS_WIN32
   int i;
   for (i=strlen(path)-1; i>=0; i--)
      if ((path[i]=='\\') || (path[i]=='/'))
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
   if (opath[len - 1] == '/')
      opath[len - 1] = '\0';
   for (p=&opath[1]; p[0]!='\0'; p++) {
      if (p[0] == '/') {
         p[0] = '\0';
         if (!nfile_dirExists(opath)) {
            ret = MKDIR;
            if (ret)
               return ret;
         }
         p[0] = '/';
      }
   }
   if (!nfile_dirExists(opath)) { /* if path is not terminated with / */
      ret = MKDIR;
      if (ret)
         return ret;
   }

   return 0;
}
#undef MKDIR


/**
 * @brief Creates a directory if it doesn't exist.
 *
 *    @param path Path to create directory if it doesn't exist.
 *    @return 0 on success.
 */
int nfile_dirMakeExist( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Check if it exists. */
   if (nfile_dirExists(file))
      return 0;

#if HAS_POSIX
   if (mkpath(file, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
#elif HAS_WIN32
   if (mkpath(file) < 0) {
#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
      WARN("Dir '%s' does not exist and unable to create: %s", file, strerror(errno));
      return -1;
   }

   return 0;
}


/**
 * @brief Checks to see if a directory exists.
 */
int nfile_dirExists( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   DIR *d;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   d = opendir(file);
   if (d==NULL)
      return 0;
   closedir(d);
   return 1;
}


/**
 * @brief Checks to see if a file exists.
 *
 *    @param path printf formatted string pointing to the file to check for existence.
 *    @return 1 if file exists, 0 if it doesn't or -1 on error.
 */
int nfile_fileExists( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   struct stat buf;

   if (path == NULL)
      return -1;
   va_start(ap, path);
   vsnprintf(file, PATH_MAX, path, ap);
   va_end(ap);

   if (stat(file,&buf)==0) /* stat worked, file must exist */
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
int nfile_backupIfExists( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   char backup[PATH_MAX];
   FILE *f_in, *f_out;
   char buf[ 8*1024 ];
   size_t lr, lw;

   if (path == NULL)
      return -1;

   va_start(ap, path);
   vsnprintf(file, PATH_MAX, path, ap);
   va_end(ap);

   if (!nfile_fileExists(file))
      return 0;

   nsnprintf(backup, PATH_MAX, "%s.backup", file);

   /* Open files. */
   f_in  = fopen( file, "rb" );
   f_out = fopen( backup, "wb" );
   if ((f_in==NULL) || (f_out==NULL)) {
      WARN( "Failure to create back up of '%s': %s", file, strerror(errno) );
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
   WARN( "Failure to create back up of '%s': %s", file, strerror(errno) );
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
char** nfile_readDir( int* nfiles, const char* path, ... )
{
   char file[PATH_MAX], base[PATH_MAX];
   char **files;
   filedata_t *filedata;
   va_list ap;

   (*nfiles) = 0;
   if (path == NULL)
      return NULL;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

   int i;
   DIR *d;
   struct dirent *dir;
   char *name;
   int mfiles;
   struct stat sb;

   d = opendir(base);
   if (d == NULL)
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
      nsnprintf( file, PATH_MAX, "%s/%s", base, name );
      if (stat(file, &sb) == -1)
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
 *    @param[out] nfiles Returns how many files there are.
 *    @param path Directory to read.
 */
char** nfile_readDirRecursive( int* nfiles, const char* path, ... )
{
   char **tfiles, **out, **cfiles, *buf, base[PATH_MAX];
   int i, j, ls, mfiles, tmp, cn;
   va_list ap;

   va_start(ap, path);
   vsnprintf( base, PATH_MAX, path, ap );
   va_end(ap);

   mfiles  = 128;
   out     = malloc(sizeof(char*)*mfiles);
   tfiles  = nfile_readDir( &tmp, base );
   *nfiles = 0;

   for (i=0; i<tmp; i++) {
      ls  = strlen(base) + strlen(tfiles[i]) + 1;
      buf = malloc(ls);
      nsnprintf( buf, ls, "%s%s", path, tfiles[i] );
      if (nfile_dirExists(buf)) {
         /* Append slash if necessary. */
         if (strcmp(&buf[ls-1],"/")!=0) {
            buf = realloc( buf, (ls+1) );
            nsnprintf( buf, ls+1, "%s%s/", path, tfiles[i] );
         }

         /* Iterate over children. */
         cfiles = nfile_readDirRecursive( &cn, buf );
         for (j=0; j<cn; j++) {
            if ((*nfiles+1) > mfiles) {
               mfiles *= 2;
               out = realloc( out, sizeof(char*)*mfiles );
            }
            out[(*nfiles)++] = cfiles[j];
         }
         free(cfiles);
      }
      else {
         if ((*nfiles+1) > mfiles) {
            mfiles *= 2;
            out = realloc( out, sizeof(char*)*mfiles );
         }
         out[(*nfiles)++] = strdup( buf );
      }

      /* Clean up. */
      free(tfiles[i]);
      free(buf);
   }

   free(tfiles);
   return out;
}

/**
 * @brief Tries to read a file.
 *
 *    @param filesize Stores the size of the file.
 *    @param path Path of the file.
 *    @return The file data.
 */
char* nfile_readFile( int* filesize, const char* path, ... )
{
   int n;
   char base[PATH_MAX];
   char *buf;
   FILE *file;
   int len;
   size_t pos;
   va_list ap;

   if (path == NULL) {
      *filesize = 0;
      return NULL;
   }
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Open file. */
   file = fopen( base, "rb" );
   if (file == NULL) {
      WARN("Error occurred while opening '%s': %s", base, strerror(errno));
      *filesize = 0;
      return NULL;
   }

   /* Get file size. */
   if (fseek( file, 0L, SEEK_END ) == -1) {
      WARN("Error occurred while seeking '%s': %s", base, strerror(errno));
      fclose(file);
      *filesize = 0;
      return NULL;
   }
   len = ftell(file);
   if (fseek( file, 0L, SEEK_SET ) == -1) {
      WARN("Error occurred while seeking '%s': %s", base, strerror(errno));
      fclose(file);
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len+1 );
   if (buf == NULL) {
      WARN("Out of Memory!");
      fclose(file);
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while (n < len) {
      pos = fread( &buf[n], 1, len-n, file );
      if (pos <= 0) {
         WARN("Error occurred while reading '%s': %s", base, strerror(errno));
         fclose(file);
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
int nfile_touch( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   FILE *f;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Try to open the file, C89 compliant, but not as precise as stat. */
   f = fopen(file, "a+b");
   if (f == NULL) {
      WARN("Unable to touch file '%s': %s", file, strerror(errno));
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
int nfile_writeFile( const char* data, int len, const char* path, ... )
{
   int n;
   char base[PATH_MAX];
   FILE *file;
   size_t pos;
   va_list ap;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Open file. */
   file = fopen( base, "wb" );
   if (file == NULL) {
      WARN("Error occurred while opening '%s': %s", base, strerror(errno));
      return -1;
   }

   /* Write the file. */
   n = 0;
   while (n < len) {
      pos = fwrite( &data[n], 1, len-n, file );
      if (pos <= 0) {
         WARN("Error occurred while writing '%s': %s", base, strerror(errno));
         fclose(file);  /* don't care about further errors */
         return -1;
      }
      n += pos;
   }

   /* Close the file. */
   if (fclose(file) == EOF) {
      WARN("Error occurred while closing '%s': %s", base, strerror(errno));
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
int nfile_delete( const char* file )
{
   if (unlink(file)) {
      WARN( "Error deleting file %s",file );
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
      WARN("Can not rename non existant file %s",oldname);
      return -1;
   }
   if (newname == NULL) {
      WARN("Can not rename to NULL file name");
      return -1;
   }
   if (nfile_fileExists( newname )) {
      WARN("Error renaming %s to %s. %s already exists",oldname,newname,newname);
      return -1;
   }
   if (rename(oldname,newname))
      WARN("Error renaming %s to %s",oldname,newname);
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
