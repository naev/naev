/*
 * See Licensing and Copyright notice in naev.h
 */

#include <lualib.h>

#include "constants.h"

#include "ndata.h"
#include "nlua.h"

constants CTS = {
   .PHYSICS_SPEED_DAMP    = 3., /* Default before 0.13.0. */
   .STEALTH_MIN_DIST      = 1000.,
   .EW_JUMP_BONUS_RANGE   = 2500.,
   .EW_ASTEROID_DIST      = 7.5e3,
   .EW_JUMPDETECT_DIST    = 7.5e3,
   .EW_SPOBDETECT_DIST    = 20e3,
   .PILOT_DISABLED_ARMOUR = 0.1, /* 0 before 0.13.0 */
};

int constants_init( void )
{
   const char *file = "constants.lua";
   char       *buf;
   size_t      size;

   buf = ndata_read( file, &size );
   if ( buf == NULL ) {
      WARN( _( "File '%s' not found!" ), file );
      return -1;
   }

   lua_State *L = luaL_newstate();
   luaL_openlibs( L );

   if ( ( nlua_loadbuffer( L, buf, size, file ) || lua_pcall( L, 0, 1, 0 ) ) ) {
      WARN( _( "Failed to parse '%s':\n%s" ), file, lua_tostring( L, -1 ) );
      lua_close( L );
      return -1;
   }
#define CT_DBL( NAME )                                                         \
   do {                                                                        \
      lua_getfield( L, -1, ( #NAME ) );                                        \
      if ( !lua_isnoneornil( L, -1 ) ) {                                       \
         CTS.NAME = lua_tonumber( L, -1 );                                     \
      }                                                                        \
      lua_pop( L, 1 );                                                         \
   } while ( 0 )

   CT_DBL( PHYSICS_SPEED_DAMP );
   CT_DBL( STEALTH_MIN_DIST );

   CT_DBL( EW_JUMP_BONUS_RANGE );
   CT_DBL( EW_ASTEROID_DIST );
   CT_DBL( EW_JUMPDETECT_DIST );
   CT_DBL( EW_SPOBDETECT_DIST );

   CT_DBL( PILOT_DISABLED_ARMOUR );

#undef CT_DBL

   lua_close( L );
   return 0;
}
