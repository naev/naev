/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nfile.h"

#include <string.h>
#ifdef LINUX
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

   if (strcmp(path,".")==0)
      strncpy(file,nfile_basePath(),PATH_MAX);
   else
      snprintf(file, PATH_MAX,"%s%s",nfile_basePath(),path);
   stat(file,&buf);
   if (!S_ISDIR(buf.st_mode))
      if (mkdir(file,S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
         WARN("Dir '%s' does not exist and unable to create", path);
         return -1;
      }
#endif /* LINUX */

   return 0;
}


