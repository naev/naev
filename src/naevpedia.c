/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_shader.c
 *
 * @brief Handles shaders.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua.h"

int naevpedia_open( const char *path )
{
   char buf[STRMAX_SHORT];
   snprintf( buf, sizeof( buf ), "require('naevpedia').open('%s')", path );

   nlua_env env = nlua_newEnv();
   nlua_dobufenv( env, buf, strlen( buf ), path );
   nlua_freeEnv( env );
   return 0;
}
