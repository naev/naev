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

#include "log.h"
#include "nlua.h"
#include "nlua_bkg.h"
#include "nlua_linopt.h"
#include "nlua_music.h"
#include "nlua_tk.h"

static nlua_env *naevpedia_env = NULL; /**< Naevpedia environment. */

/**
 * @brief Try to open Naevpedia path.
 */
int naevpedia_open( const char *path )
{
   char buf[STRMAX_SHORT];
   int  status;

   if ( naevpedia_env == NULL ) {
      naevpedia_env = nlua_newEnv( "naevpedia" );
      nlua_loadStandard( naevpedia_env );
      nlua_loadBackground( naevpedia_env );
      nlua_loadMusic( naevpedia_env );
      nlua_loadTk( naevpedia_env );
      nlua_loadLinOpt( naevpedia_env );
   }

   snprintf( buf, sizeof( buf ), "require('naevpedia').open('%s')", path );
   status = nlua_dobufenv( naevpedia_env, buf, strlen( buf ), buf );
   if ( status ) {
      WARN( _( "Naevpedia '%s' Lua error:\n%s" ), path,
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }
   return 0;
}
