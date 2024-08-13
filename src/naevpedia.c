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
#include "nlua_camera.h"
#include "nlua_cli.h"
#include "nlua_colour.h"
#include "nlua_linopt.h"
#include "nlua_music.h"
#include "nlua_tex.h"
#include "nlua_tk.h"

int naevpedia_open( const char *path )
{
   char buf[STRMAX_SHORT];
   int  status;
   snprintf( buf, sizeof( buf ), "require('naevpedia').open('%s')", path );

   nlua_env env = nlua_newEnv();
   nlua_loadStandard( env );
   nlua_loadTex( env );
   nlua_loadCol( env );
   nlua_loadBackground( env );
   nlua_loadCLI( env );
   nlua_loadCamera( env );
   nlua_loadMusic( env );
   nlua_loadTk( env );
   nlua_loadLinOpt( env );
   status = nlua_dobufenv( env, buf, strlen( buf ), buf );
   if ( status ) {
      WARN( _( "Naevpedia '%s' Lua error:\n%s" ), path,
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }
   nlua_freeEnv( env );
   return 0;
}
