/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nfile.c
 *
 * @brief Handles read/write abstractions to the users directory.
 *
 * @todo add support for windows and mac os.
 */


#include "nfile.h"

#include "naev.h"

#include <stdio.h>
#include <string.h>
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



static char naev_base[PATH_MAX] = "\0"; /**< Stores naev's base path. */
/**
 * @brief Gets naev's base path (for saves and such).
 *
 *    @return The base path to naev.
 */
char* nfile_basePath (void)
{
   char *home;

   if (naev_base[0] == '\0') {
#if HAS_UNIX
      home = getenv("HOME");
      if (home == NULL) {
         WARN("$HOME isn't set, using current directory.");
         home = ".";
      }
#ifdef PREFSDIR_DEF
      snprintf( naev_base, PATH_MAX, "%s/%s/", home, PREFSDIR_DEF );
#else
      snprintf( naev_base, PATH_MAX, "%s/.naev/", home );
#endif
#elif HAS_WIN32
      home = getenv("APPDATA");
      if (home == NULL) {
         WARN("%%APPDATA%% isn't set, using current directory.");
         home = ".";
      }
      snprintf( naev_base, PATH_MAX, "%s/naev/", home );
#else
#error "Feature needs implementation on this Operating System for NAEV to work."
#endif
   }

   return naev_base;
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
   snprintf( dirname_buf, MIN(sizeof(dirname_buf), (size_t)(i+1)),  path );
   return dirname_buf;
#else
#error "Functionality not implemented for your OS."
#endif /* HAS_POSIX */
}


/**
 * @brief Creates a directory if it doesn't exist.
 *
 * Uses relative paths to basePath.
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

#if HAS_POSIX
   struct stat buf;
   int ret;

   ret = stat(file,&buf);
   /* Check to see if there was a messed up error. */
   if (ret && (errno != ENOENT)) {
      WARN("Unable to stat '%s': %s", file, strerror(errno));
      return -1;
   }
   /* Normal error/doesn't exist. */
   else if ((ret && (errno == ENOENT)) || !S_ISDIR(buf.st_mode))
      if (mkdir(file, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
         WARN("Dir '%s' does not exist and unable to create: %s", file, strerror(errno));
         return -1;
      }
#elif HAS_WIN32
   DIR *d;

   d = opendir(file);
   if (d==NULL) {
      if (!CreateDirectory(file, NULL))  {
         WARN("Dir '%s' does not exist and unable to create: %s", file, strerror(errno));
         return -1;
      }
   }
   else {
      closedir(d);
   }
#else
#error "Feature needs implementation on this Operating System for NAEV to work."
#endif

   return 0;
}


/**
 * @brief Checks to see if a file exists.
 *
 *    @param path printf formatted string pointing to the file to check for existance.
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
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   if (nfile_fileExists(file)) {
      snprintf(backup, PATH_MAX, "%s.backup", file);

      /* Open files. */
      f_in  = fopen( file, "r" );
      f_out = fopen( backup, "w" );
      if ((f_in==NULL) || (f_out==NULL)) {
         WARN( "Failure to create back up of '%s': %s", file, strerror(errno) );
         if (f_in!=NULL)
            fclose(f_in);
         return -1;
      }

      /* Copy data over. */
      do {
         lr = fread( buf, 1, sizeof(buf), f_in );
         lw = fwrite( buf, 1, lr, f_out );
         if (lr != lw) {
            WARN( "Failure to create back up of '%s': %s", file, strerror(errno) );
            fclose( f_in );
            fclose( f_out );
            return -1;
         }
      } while (lr > 0);

      /* Close files. */
      fclose( f_in );
      fclose( f_out );

   }
   return 0;
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
   va_list ap;

   (*nfiles) = 0;
   if (path == NULL)
      return NULL;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

   int i,j,k, n;
   DIR *d;
   struct dirent *dir;
   char *name;
   int mfiles;
   struct stat sb;
   time_t *tt, *ft;
   char **tfiles;

   mfiles      = 128;
   tfiles      = malloc(sizeof(char*)*mfiles);
   tt          = malloc(sizeof(time_t)*mfiles);

   d = opendir(base);
   if (d == NULL) {
      free(tt);
      free(tfiles);
      return NULL;
   }

   /* Get the file list */
   while ((dir = readdir(d)) != NULL) {
      name = dir->d_name;

      /* Skip hidden directories */
      if (name[0] == '.')
         continue;

      /* Stat the file */
      snprintf( file, PATH_MAX, "%s/%s", base, name );
      if (stat(file, &sb) == -1)
         continue; /* Unable to stat */

      /* Enough memory? */
      if ((*nfiles)+1 > mfiles) {
         mfiles += 128;
         tfiles = realloc( tfiles, sizeof(char*) * mfiles );
         tt = realloc( tt, sizeof(time_t) * mfiles );
      }

      /* Write the information */
      tfiles[(*nfiles)] = strdup(name);
      tt[(*nfiles)] = sb.st_mtime;
      (*nfiles)++;
   }

   closedir(d);

   /* Sort by last changed date */
   if ((*nfiles) > 0) {

      /* Need to allocate some stuff */
      files = malloc( sizeof(char*) * (*nfiles) );
      ft = malloc( sizeof(time_t) * (*nfiles) );

      /* Fill the list */
      for (i=0; i<(*nfiles); i++) {
         n = -1;

         /* Get next lowest */
         for (j=0; j<(*nfiles); j++) {

            /* Is lower? */
            if ((n == -1) || (tt[j] > tt[n])) {

               /* Check if it's already there */
               for (k=0; k<i; k++)
                  if (strcmp(files[k],tfiles[j])==0)
                     break;

               /* New lowest */
               if (k>=i)
                  n = j;
            }
         }

         files[i] = tfiles[n];
         ft[i] = tt[n];
      }
      free(ft);
   }
   else
      files = NULL;

   /* Free temporary stuff */
   free(tfiles);
   free(tt);

   /* found nothing */
   if ((*nfiles) == 0) {
      free(files);
      files = NULL;
   }

   return files;
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
   buf = malloc( len );
   if (buf == NULL) {
      WARN("Out of Memory!");
      fclose(file);
      *filesize = 0;
      return NULL;
   }

   /* Read the file. */
   n = 0;
   while (n < len) {
      pos = fread( &buf[n], 1, len-n, file );
      if (pos <= 0) {
         WARN("Error occurred while reading '%s': %s", base, strerror(errno));
         fclose(file);
         *filesize = 0;
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

