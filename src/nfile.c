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

#include <string.h>
#include <stdarg.h>
#ifdef LINUX
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <stdio.h> 
#include <errno.h>
#endif /* LINUX */

#include "naev.h"
#include "log.h"



/**
 * @fn char* nfile_basePath (void)
 *
 * @brief Gets naev's base path (for saves and such).
 *
 *    @return The base path to naev.
 */
static char naev_base[128] = "\0"; /**< Stores naev's base path. */
char* nfile_basePath (void)
{
   char *home;

   if (naev_base[0] == '\0') {
#ifdef LINUX
      home = getenv("HOME");
      snprintf(naev_base,PATH_MAX,"%s/.naev/",home);
#else
#error "Needs implementation."
#endif /* LINUX */
   }
   
   return naev_base;
}


/**
 * @fn int nfile_dirMakeExist( const char* path )
 *
 * @brief Creates a directory if it doesn't exist.
 *
 * Uses relative paths to basePath.
 *
 *    @param path Path to create directory if it doesn't exist.
 *    @return 0 on success.
 */
int nfile_dirMakeExist( const char* path )
{
   char file[PATH_MAX];

#ifdef LINUX
   struct stat buf;

   if (strcmp(path,".")==0) {
      strncpy(file,nfile_basePath(),PATH_MAX);
      file[PATH_MAX-1] = '\0';
   }
   else
      snprintf(file, PATH_MAX,"%s%s",nfile_basePath(),path);
   stat(file,&buf);
   if (!S_ISDIR(buf.st_mode))
      if (mkdir(file,S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
         WARN("Dir '%s' does not exist and unable to create", path);
         return -1;
      }
#else
#error "Needs implementation."
#endif /* LINUX */

   return 0;
}


/**
 * @fn int nfile_fileExists( const char* path, ... )
 *
 * @brief Checks to see if a file exists.
 *
 *    @param path printf formatted string pointing to the file to check for existance.
 *    @return 1 if file exists, 0 if it doesn't or -1 on error.
 */
int nfile_fileExists( const char* path, ... )
{
   char file[PATH_MAX], name[PATH_MAX];
   va_list ap;
   size_t l;

   l = 0;
   if (path == NULL) return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(name, PATH_MAX-l, path, ap);
      l = strlen(name);
      va_end(ap);
   }

   snprintf(file, PATH_MAX,"%s%s",nfile_basePath(),name);
#ifdef LINUX
   struct stat buf;

   if (stat(file,&buf)==0) /* stat worked, file must exist */
      return 1;

#else /* LINUX */
#error "Needs implementation."
#endif
   return 0;
}


/**
 * @fn char** nfile_readDir( int* nfiles, const char* path )
 *
 * @brief Lists all the visible files in a directory.
 *
 * Should also sort by last modified but that's up to the OS in question.
 * Paths are relative to base directory.
 *
 *    @param[out] nfiles Returns how many files there are.
 *    @param path Directory to read.
 */
char** nfile_readDir( int* nfiles, const char* path )
{
   char file[PATH_MAX];
   char **files;

   snprintf( file, PATH_MAX, "%s%s", nfile_basePath(), path );

#ifdef LINUX
   int i,j,k, n;
   DIR *d;
   struct dirent *dir;
   char *name;
   int mfiles;
   struct stat sb;
   time_t *tt, *ft;
   char **tfiles;

   (*nfiles) = 0;
   mfiles = 100;
   tfiles = malloc(sizeof(char*)*mfiles);
   tt = malloc(sizeof(time_t)*mfiles);

   d = opendir(file);
   if (d == NULL)
      return NULL;

   /* Get the file list */
   while ((dir = readdir(d)) != NULL) {
      name = dir->d_name;

      /* Skip hidden directories */
      if (name[0] == '.')
         continue;

      /* Stat the file */
      snprintf( file, PATH_MAX, "%s%s/%s", nfile_basePath(), path, name );
      if (stat(file, &sb) == -1)
         continue; /* Unable to stat */

      /* Enough memory? */
      if ((*nfiles)+1 > mfiles) {
         mfiles += 100;
         tfiles = realloc( files, sizeof(char*) * mfiles );
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

#else /* LINUX */
#error "Needs implementation."
#endif /* LINUX */

   /* found nothing */
   if ((*nfiles) == 0) {
      free(files);
      files = NULL;
   }

   return files;
}


