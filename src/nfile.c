/*
 * See Licensing and Copyright notice in naev.h
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



/*
 * returns naev's base path
 */
static char naev_base[128] = "\0";
char* nfile_basePath (void)
{
   char *home;

   if (naev_base[0] == '\0') {
#ifdef LINUX
      home = getenv("HOME");
#else
#error "Needs implementation."
#endif /* LINUX */
      snprintf(naev_base,PATH_MAX,"%s/.naev/",home);
   }
   
   return naev_base;
}


/*
 * checks if a directory exists, and creates it if it doesn't
 * based on naev_base
 */
int nfile_dirMakeExist( char* path )
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


/*
 * checks if a file exists
 */
int nfile_fileExists( char* path, ... )
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


/*
 * lists all the files in a dir (besidse . and ..)
 */
char** nfile_readDir( int* nfiles, char* path )
{
   char file[PATH_MAX];
   char **files;

   snprintf( file, PATH_MAX, "%s%s", nfile_basePath(), path );

#ifdef LINUX
   DIR *d;
   struct dirent *dir;
   char *name;
   int mfiles;

   (*nfiles) = 0;
   mfiles = 100;
   files = malloc(sizeof(char*)*mfiles);

   d = opendir(file);
   if (d == NULL) {
      return NULL;
   }

   while ((dir = readdir(d)) != NULL) {
      name = dir->d_name;

      if ((strcmp(name,".")==0) || (strcmp(name,"..")==0))
         continue;

      if ((*nfiles)+1 > mfiles) {
         mfiles += 100;
         files = realloc( files, sizeof(char*) * mfiles );
      }

      files[(*nfiles)] = strdup(name);
      (*nfiles)++;
   }

   closedir(d);
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


