/*
 * See Licensing and Copyright notice in naev.h
 */


#include "env.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "config.h"
#include "nstring.h"


env_t env;


void env_detect( char **argv )
{
   static short once = 0;
   assert( once == 0 );
   once = 1;

   env.appimage = getenv( "APPIMAGE" );
   if ( env.appimage != NULL ) {
      env.isAppImage = 1;
      env.argv0      = getenv( "ARGV0" );
      env.appdir     = getenv( "APPDIR" );
   }
   else {
      env.isAppImage = 0;
      env.argv0      = argv[ 0 ];
   }
}


/**
 * @brief Sets an environment variable.
 */
int nsetenv( const char *name, const char *value, int overwrite )
{
#if HAVE_DECL_SETENV
   return setenv( name, value, overwrite );
#else /* HAVE_DECL_SETENV */
   if (!overwrite) {
#if HAVE_DECL__PUTENV_S
      size_t envsize = 0;
      int errcode = getenv_s( &envsize, NULL, 0, name );
      if (errcode || envsize)
         return errcode;
#else /* HAVE_DECL__PUTENV_S */
      const char *envval = getenv( name );
      if (envval != NULL)
         return 0;
#endif /* HAVE_DECL__PUTENV_S */
   }
#if HAVE_DECL__PUTENV_S
   return _putenv_s(name, value);
#else /* HAVE_DECL__PUTENV_S */
   char buf[PATH_MAX];
   nsnprintf( buf, sizeof(buf), "%s=%s", name, value );
   return putenv( buf );
#endif /* HAVE_DECL__PUTENV_S */
#endif /* HAVE_DECL_SETENV */
}


