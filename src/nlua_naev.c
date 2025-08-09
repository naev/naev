/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_naev.c
 *
 * @brief Contains Naev generic Lua bindings.
 */
/** @cond */
#include <lauxlib.h>
#include <time.h>

#include <SDL3/SDL_timer.h>

#include "naev.h"
/** @endcond */

#include "nlua_naev.h"

#include "array.h"
#include "console.h"
#include "debug.h"
#include "difficulty.h"
#include "event.h"
#include "hook.h"
#include "info.h"
#include "input.h"
#include "land.h"
#include "menu.h"
#include "nlua_misn.h"
#include "nlua_system.h"
#include "nluadef.h"
#include "pause.h"
#include "player.h"
#include "plugin.h"

static int cache_table = LUA_NOREF; /* No reference. */

/* Naev methods. */
static int naevL_version( lua_State *L );
static int naevL_versionTest( lua_State *L );
static int naevL_language( lua_State *L );
static int naevL_lastplayed( lua_State *L );
static int naevL_date( lua_State *L );
static int naevL_ticks( lua_State *L );
static int naevL_ticksGame( lua_State *L );
static int naevL_clock( lua_State *L );
static int naevL_fps( lua_State *L );
static int naevL_keyGet( lua_State *L );
static int naevL_keyEnable( lua_State *L );
static int naevL_keyEnableAll( lua_State *L );
static int naevL_keyDisableAll( lua_State *L );
static int naevL_eventStart( lua_State *L );
static int naevL_eventReload( lua_State *L );
static int naevL_missionList( lua_State *L );
static int naevL_missionStart( lua_State *L );
static int naevL_missionTest( lua_State *L );
static int naevL_missionReload( lua_State *L );
static int naevL_shadersReload( lua_State *L );
static int naevL_isSimulation( lua_State *L );
static int naevL_conf( lua_State *L );
static int naevL_confSet( lua_State *L );
static int naevL_cache( lua_State *L );
static int naevL_trigger( lua_State *L );
static int naevL_claimTest( lua_State *L );
static int naevL_plugins( lua_State *L );
static int naevL_menuInfo( lua_State *L );
static int naevL_menuSmall( lua_State *L );
static int naevL_isPaused( lua_State *L );
static int naevL_pause( lua_State *L );
static int naevL_unpause( lua_State *L );
static int naevL_hasTextInput( lua_State *L );
static int naevL_setTextInput( lua_State *L );
static int naevL_shipstats( lua_State *L );
static int naevL_unit( lua_State *L );
static int naevL_quadtreeParams( lua_State *L );
static int naevL_difficulty( lua_State *L );
static int naevL_difficultyLevel( lua_State *L );
#if DEBUGGING
static int naevL_debugTrails( lua_State *L );
static int naevL_debugCollisions( lua_State *L );
#endif /* DEBUGGING */

static const luaL_Reg naev_methods[] = {
   { "version", naevL_version },
   { "versionTest", naevL_versionTest },
   { "language", naevL_language },
   { "lastplayed", naevL_lastplayed },
   { "date", naevL_date },
   { "ticks", naevL_ticks },
   { "ticksGame", naevL_ticksGame },
   { "clock", naevL_clock },
   { "fps", naevL_fps },
   { "keyGet", naevL_keyGet },
   { "keyEnable", naevL_keyEnable },
   { "keyEnableAll", naevL_keyEnableAll },
   { "keyDisableAll", naevL_keyDisableAll },
   { "eventStart", naevL_eventStart },
   { "eventReload", naevL_eventReload },
   { "missionList", naevL_missionList },
   { "missionStart", naevL_missionStart },
   { "missionTest", naevL_missionTest },
   { "missionReload", naevL_missionReload },
   { "shadersReload", naevL_shadersReload },
   { "isSimulation", naevL_isSimulation },
   { "conf", naevL_conf },
   { "confSet", naevL_confSet },
   { "cache", naevL_cache },
   { "trigger", naevL_trigger },
   { "claimTest", naevL_claimTest },
   { "plugins", naevL_plugins },
   { "menuInfo", naevL_menuInfo },
   { "menuSmall", naevL_menuSmall },
   { "isPaused", naevL_isPaused },
   { "pause", naevL_pause },
   { "unpause", naevL_unpause },
   { "hasTextInput", naevL_hasTextInput },
   { "setTextInput", naevL_setTextInput },
   { "shipstats", naevL_shipstats },
   { "unit", naevL_unit },
   { "quadtreeParams", naevL_quadtreeParams },
   { "difficulty", naevL_difficulty },
   { "difficultyLevel", naevL_difficultyLevel },
#if DEBUGGING
   { "debugTrails", naevL_debugTrails },
   { "debugCollisions", naevL_debugCollisions },
#endif         /* DEBUGGING */
   { 0, 0 } }; /**< Naev Lua methods. */

/**
 * @brief Loads the Naev Lua library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadNaev( nlua_env *env )
{
   nlua_register( env, "naev", naev_methods, 0 );

   /* Create cache. */
   if ( cache_table == LUA_NOREF ) {
      lua_newtable( naevL );
      cache_table = luaL_ref( naevL, LUA_REGISTRYINDEX );
   }

   return 0;
}

/**
 * @brief Naev generic Lua bindings.
 *
 * @luamod naev
 */

/**
 * @brief Gets the version of Naev and the save game.
 *
 * @usage game_version, save_version = naev.version()
 *
 *    @luatreturn string The version of the game.
 *    @luatreturn string Version of current loaded save or nil if not loaded.
 * @luafunc version
 */
static int naevL_version( lua_State *L )
{
   lua_pushstring( L, naev_version( 0 ) );
   if ( player.loaded_version == NULL )
      lua_pushnil( L );
   else
      lua_pushstring( L, player.loaded_version );
   return 2;
}

/**
 * @brief Test a version string.
 *
 *    @luatparam string ver Version to test.
 *    @luatparam string req Requirement condition.
 *    @luatreturn boolean Whether or not the version meets the requirements.
 * @luafunc versionTest
 */
static int naevL_versionTest( lua_State *L )
{
   const char *s1 = luaL_checkstring( L, 1 );
   const char *s2 = luaL_checkstring( L, 2 );
   lua_pushboolean( L, naev_versionMatchReq( s1, s2 ) );
   return 1;
}

/**
 * @brief Gets the current language locale.
 *
 *    @luatreturn string Current language locale (such as "en" for English, "de"
 * for German, or "ja" for Japanese).
 * @luafunc language
 */
static int naevL_language( lua_State *L )
{
   lua_pushstring( L, gettext_getLanguage() );
   return 1;
}

/**
 * @brief Gets how many days it has been since the player last played Naev.
 *
 *    @luatreturn number Number of days since the player last played.
 *    @luatreturn number Number of days since any of the save games were played.
 * @luafunc lastplayed
 */
static int naevL_lastplayed( lua_State *L )
{
   const double TO_DAYS = 1. / ( 3600. * 24. ); /* Convert to days. */
   double       d       = difftime( time( NULL ), player.last_played );
   double       g       = difftime( time( NULL ), conf.last_played );
   lua_pushnumber( L, d * TO_DAYS );
   lua_pushnumber( L, g * TO_DAYS );
   return 2;
}

/**
 * @brief Equivalent to os.date from standard Lua.
 *
 * @luafunc date
 */
static int naevL_date( lua_State *L )
{
   const char *s = luaL_optstring( L, 1, "%c" );
   time_t      t = luaL_opt( L, (time_t)luaL_checknumber, 2, time( NULL ) );
   struct tm  *stm;
   if ( *s == '!' ) { /* UTC? */
      stm = gmtime( &t );
      s++; /* skip `!' */
   } else
      stm = localtime( &t );
   if ( stm == NULL ) /* invalid date? */
      lua_pushnil( L );
   else if ( strcmp( s, "*t" ) == 0 ) {
      lua_createtable( L, 0, 9 ); /* 9 = number of fields */
#define setfield( L, key, value )                                              \
   do {                                                                        \
      lua_pushinteger( L, value );                                             \
      lua_setfield( L, -2, key );                                              \
   } while ( 0 )
#define setboolfield( L, key, value )                                          \
   do {                                                                        \
      if ( value >= 0 ) {                                                      \
         lua_pushinteger( L, value );                                          \
         lua_setfield( L, -2, key );                                           \
      }                                                                        \
   } while ( 0 )
      setfield( L, "sec", stm->tm_sec );
      setfield( L, "min", stm->tm_min );
      setfield( L, "hour", stm->tm_hour );
      setfield( L, "day", stm->tm_mday );
      setfield( L, "month", stm->tm_mon + 1 );
      setfield( L, "year", stm->tm_year + 1900 );
      setfield( L, "wday", stm->tm_wday + 1 );
      setfield( L, "yday", stm->tm_yday + 1 );
      setboolfield( L, "isdst", stm->tm_isdst );
#undef setfield
#undef setboolfield
   } else {
      char        cc[3];
      luaL_Buffer b;
      cc[0] = '%';
      cc[2] = '\0';
      luaL_buffinit( L, &b );
      for ( ; *s; s++ ) {
         if ( *s != '%' || *( s + 1 ) == '\0' ) /* no conversion specifier? */
            luaL_addchar( &b, *s );
         else {
            size_t reslen;
            char buff[200]; /* should be big enough for any conversion result */
            cc[1]  = *( ++s );
            reslen = strftime( buff, sizeof( buff ), cc, stm );
            luaL_addlstring( &b, buff, reslen );
         }
      }
      luaL_pushresult( &b );
   }
   return 1;
}

/**
 * @brief Gets the game seconds since the program started running.
 *
 * These are modified by whatever speed up the player has.
 *
 *    @luatreturn number The seconds since the application started running.
 * @luafunc ticksGame
 */
static int naevL_ticksGame( lua_State *L )
{
   lua_pushnumber( L, elapsed_time_mod );
   return 1;
}

/**
 * @brief Gets the seconds since the program started running.
 *
 * Useful for doing timing on Lua functions.
 *
 *    @luatreturn number The seconds since the application started running.
 * @luafunc ticks
 */
static int naevL_ticks( lua_State *L )
{
   lua_pushnumber( L, (double)SDL_GetPerformanceCounter() /
                         (double)SDL_GetPerformanceFrequency() );
   return 1;
}

/**
 * @brief Gets the approximate CPU processing time.
 *
 *    @luatreturn number Seconds elapsed since start of the process.
 * @luafunc clock
 */
static int naevL_clock( lua_State *L )
{
   lua_pushnumber( L, (double)clock() / (double)CLOCKS_PER_SEC );
   return 1;
}

/**
 * @brief Gets the current game FPS as displayed to the player.
 *
 *    @luatreturn number Current FPS as displayed to the player.
 * @luafunc fps
 */
static int naevL_fps( lua_State *L )
{
   lua_pushnumber( L, fps_current() );
   return 1;
}

/**
 * @brief Gets a human-readable name for the key bound to a function.
 *
 * @usage bindname = naev.keyGet( "accel" )
 *
 *    @luatparam string keyname Name of the keybinding to get value of. Valid
 * values are listed in src/input.c: keybind_info.
 * @luafunc keyGet
 */
static int naevL_keyGet( lua_State *L )
{
   char        buf[128];
   const char *keyname = luaL_checkstring( L, 1 );
   input_getKeybindDisplay( input_keyFromBrief( keyname ), buf, sizeof( buf ) );
   lua_pushstring( L, buf );
   return 1;
}

/**
 * @brief Disables or enables a specific keybinding.
 *
 * Use with caution, this can make the player get stuck.
 *
 * @usage naev.keyEnable( "accel", false ) -- Disables the acceleration key
 *    @luatparam string keyname Name of the key to disable (for example
 * "accel").
 *    @luatparam[opt=false] boolean enable Whether to enable or disable.
 * @luafunc keyEnable
 */
static int naevL_keyEnable( lua_State *L )
{
   const char *key    = luaL_checkstring( L, 1 );
   int         enable = lua_toboolean( L, 2 );

   input_toggleEnable( input_keyFromBrief( key ), enable );
   return 0;
}

/**
 * @brief Enables all inputs.
 *
 * @usage naev.keyEnableAll() -- Enables all inputs
 * @luafunc keyEnableAll
 */
static int naevL_keyEnableAll( lua_State *L )
{
   (void)L;
   input_enableAll();
   return 0;
}

/**
 * @brief Disables all inputs.
 *
 * @usage naev.keyDisableAll() -- Disables all inputs
 * @luafunc keyDisableAll
 */
static int naevL_keyDisableAll( lua_State *L )
{
   (void)L;
   input_disableAll();
   return 0;
}

/**
 * @brief Starts an event, does not start check conditions.
 *
 * @usage naev.eventStart( "Some Event" )
 *    @luatparam string evtname Name of the event to start.
 *    @luatreturn boolean true on success.
 * @luafunc eventStart
 */
static int naevL_eventStart( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   int         ret = event_start( str, NULL );

   if ( cli_isOpen() && landed )
      bar_regen();

   lua_pushboolean( L, !ret );
   return 1;
}

/**
 * @brief Lists all the missions in the game.
 *
 *    @luatreturn table A table of all the missions in the game, each as a
 * table.
 * @luafunc missionList
 */
static int naevL_missionList( lua_State *L )
{
   const MissionData *misns = mission_list();
   lua_newtable( L );
   for ( int i = 0; i < array_size( misns ); i++ ) {
      misn_pushMissionData( L, &misns[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   return 1;
}

/**
 * @brief Starts a mission, does no check start conditions.
 *
 * @usage naev.missionStart( "Some Mission" )
 *    @luatparam string misnname Name of the mission to start.
 *    @luatreturn boolean true if mission was either accepted, or started
 * without misn.finish() getting called in create.
 *    @luatreturn boolean true whether or not the mission was accepted.
 * @luafunc missionStart
 */
static int naevL_missionStart( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   int         ret = mission_start( str, NULL );

   if ( cli_isOpen() && landed ) {
      bar_regen();
      misn_regen();
   }

   lua_pushboolean( L, ( ret == 0 ) || ( ret == 3 ) );
   lua_pushboolean( L, ( ret == 3 ) );
   return 2;
}

/**
 * @brief Tests a missions conditionals to see if it can be started by the
 * player.
 *
 * Note that this tests the Lua conditionals, not the create function, so it may
 * be possible that even though naev.missionTest returns true, the player can
 * still not start the mission.
 *
 * @usage naev.missionTest( "Some Mission" )
 *    @luatparam string misnname Name of the mission to test.
 *    @luatreturn boolean true if the mission can be can be started, or false
 * otherwise.
 * @luafunc missionTest
 */
static int naevL_missionTest( lua_State *L )
{
   lua_pushboolean( L, mission_test( luaL_checkstring( L, 1 ) ) );
   return 1;
}

/**
 * @brief Reloads an event's script, providing a convenient way to test and
 * hopefully not corrupt the game's state. Use with caution, and only during
 * development as a way to get quicker feedback.
 *
 * @usage naev.eventReload( "Some Event" )
 *    @luatparam string evtname Name of the event to start.
 *    @luatreturn boolean true on success.
 * @luafunc eventReload
 */
static int naevL_eventReload( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   int         ret = event_reload( str );

   lua_pushboolean( L, !ret );
   return 1;
}

/**
 * @brief Reloads a mission's script, providing a convenient way to test and
 * hopefully not corrupt the game's state. Use with caution, and only during
 * development as a way to get quicker feedback.
 *
 * @usage naev.missionReload( "Some Mission" )
 *    @luatparam string misnname Name of the mission to start.
 *    @luatreturn boolean true on success.
 * @luafunc missionReload
 */
static int naevL_missionReload( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   int         ret = mission_reload( str );

   lua_pushboolean( L, !ret );
   return 1;
}

/**
 * @brief Reloads all the Naev shaders excluding those created by the shader
 * library.
 *
 * @luafunc shadersReload
 */
static int naevL_shadersReload( lua_State *L )
{
   (void)L;
   shaders_unload();
   shaders_load();
   return 0;
}

/**
 * @brief Gets whether or not the universe is being simulated or not.
 *
 *    @luatreturn boolean true if the world is being simulated.
 * @luafunc isSimulation
 */
static int naevL_isSimulation( lua_State *L )
{
   lua_pushboolean( L, space_isSimulation() );
   return 1;
}

#define PUSH_STRING( L, name, value )                                          \
   lua_pushstring( L, name );                                                  \
   lua_pushstring( L, value );                                                 \
   lua_rawset( L, -3 )
#define PUSH_DOUBLE( L, name, value )                                          \
   lua_pushstring( L, name );                                                  \
   lua_pushnumber( L, value );                                                 \
   lua_rawset( L, -3 )
#define PUSH_INT( L, name, value )                                             \
   lua_pushstring( L, name );                                                  \
   lua_pushinteger( L, value );                                                \
   lua_rawset( L, -3 )
#define PUSH_BOOL( L, name, value )                                            \
   lua_pushstring( L, name );                                                  \
   lua_pushboolean( L, value );                                                \
   lua_rawset( L, -3 )
/**
 * @brief Gets the configuration information.
 *
 *    @luatreturn table Table of configuration values as they appear in the
 * configuration file.
 * @luafunc conf
 */
static int naevL_conf( lua_State *L )
{
   lua_newtable( L );
   PUSH_STRING( L, "data", conf.ndata );
   PUSH_STRING( L, "language", conf.language );
   PUSH_STRING( L, "difficulty", conf.difficulty );
   PUSH_INT( L, "fsaa", conf.fsaa );
   PUSH_BOOL( L, "vsync", conf.vsync );
   PUSH_INT( L, "width", conf.width );
   PUSH_INT( L, "height", conf.height );
   PUSH_DOUBLE( L, "scalefactor", conf.scalefactor );
   PUSH_DOUBLE( L, "nebu_scale", conf.nebu_scale );
   PUSH_BOOL( L, "fullscreen", conf.fullscreen );
   PUSH_BOOL( L, "notresizable", conf.notresizable );
   PUSH_BOOL( L, "minimize", conf.minimize );
   PUSH_DOUBLE( L, "colourblind_sim", conf.colourblind_sim );
   PUSH_DOUBLE( L, "colourblind_correct", conf.colourblind_correct );
   PUSH_INT( L, "colourblind_type", conf.colourblind_type );
   PUSH_DOUBLE( L, "game_speed", conf.game_speed );
   PUSH_DOUBLE( L, "bg_brightness", conf.bg_brightness );
   PUSH_DOUBLE( L, "nebu_nonuniformity", conf.nebu_nonuniformity );
   PUSH_DOUBLE( L, "nebu_saturation", conf.nebu_saturation );
   PUSH_DOUBLE( L, "gamma_correction", conf.gamma_correction );
   PUSH_BOOL( L, "low_memory", conf.low_memory );
   PUSH_BOOL( L, "showfps", conf.fps_show );
   PUSH_INT( L, "maxfps", conf.fps_max );
   PUSH_BOOL( L, "showpause", conf.pause_show );
   PUSH_BOOL( L, "al_efx", conf.al_efx );
   PUSH_BOOL( L, "nosound", conf.nosound );
   PUSH_DOUBLE( L, "sound", conf.sound );
   PUSH_DOUBLE( L, "music", conf.music );
   /* joystick */
   PUSH_INT( L, "mesg_visible", conf.mesg_visible );
   PUSH_DOUBLE( L, "map_overlay_opacity", conf.map_overlay_opacity );
   PUSH_BOOL( L, "big_icons", conf.big_icons );
   PUSH_INT( L, "repeat_delay", conf.repeat_delay );
   PUSH_INT( L, "repeat_freq", conf.repeat_freq );
   PUSH_BOOL( L, "zoom_manual", conf.zoom_manual );
   PUSH_DOUBLE( L, "zoom_far", conf.zoom_far );
   PUSH_DOUBLE( L, "zoom_near", conf.zoom_near );
   PUSH_DOUBLE( L, "zoom_speed", conf.zoom_speed );
   PUSH_INT( L, "font_size_console", conf.font_size_console );
   PUSH_INT( L, "font_size_intro", conf.font_size_intro );
   PUSH_INT( L, "font_size_def", conf.font_size_def );
   PUSH_INT( L, "font_size_small", conf.font_size_small );
   PUSH_BOOL( L, "redirect_file", conf.redirect_file );
   PUSH_INT( L, "doubletap_sensitivity", conf.doubletap_sens );
   PUSH_DOUBLE( L, "mouse_hide", conf.mouse_hide );
   PUSH_BOOL( L, "mouse_fly", conf.mouse_fly );
   PUSH_INT( L, "mouse_accel", conf.mouse_accel );
   PUSH_DOUBLE( L, "mouse_doubleclick", conf.mouse_doubleclick );
   PUSH_BOOL( L, "devmode", conf.devmode );
   PUSH_BOOL( L, "devautosave", conf.devautosave );
   PUSH_BOOL( L, "lua_enet", conf.lua_enet );
   PUSH_BOOL( L, "lua_repl", conf.lua_repl );
   PUSH_BOOL( L, "conf_nosave", conf.nosave );
   PUSH_STRING( L, "last_version", conf.lastversion );
   PUSH_BOOL( L, "translation_warning_seen", conf.translation_warning_seen );
   PUSH_BOOL( L, "fpu_except", conf.fpu_except );
   PUSH_STRING( L, "dev_data_dir", conf.dev_data_dir );
   PUSH_BOOL( L, "puzzle_skip", conf.puzzle_skip );
   return 1;
}
#undef PUSH_STRING
#undef PUSH_DOUBLE
#undef PUSH_INT
#undef PUSH_BOOL

/**
 * @brief Sets configuration variables. Note that not all are supported.
 *
 *    @luatparam string name Configuration variable name.
 *    @luatparam number|string value Value to set to.
 * @luafunc confSet
 */
static int naevL_confSet( lua_State *L )
{
   (void)L;
   /* TODO implement. */
   return NLUA_ERROR( L, _( "unimplemented" ) );
}

/**
 * @brief Gets the global Lua runtime cache. This is shared among all
 * environments and is cleared when the game is closed.
 *
 * @usage c = naev.cache()
 *
 *    @luatreturn table The Lua global cache.
 * @luafunc cache
 */
static int naevL_cache( lua_State *L )
{
   lua_rawgeti( L, LUA_REGISTRYINDEX, cache_table );
   return 1;
}

/**
 * @brief Triggers manually a hook stack. This is run deferred (next frame).
 * Meant mainly to be used with hook.custom, but can work with other hooks too
 * (if you know what you are doing).
 *
 * @note This will trigger all hooks waiting on a stack.
 *
 * @usage naev.trigger( "my_event", data ) -- data will be passed to the
 * receiving end
 *
 *    @luatparam string hookname Name of the hook to be run.
 *    @luaparam arg Parameter to pass to the hooks.
 * @see custom
 * @luafunc trigger
 */
static int naevL_trigger( lua_State *L )
{
   HookParam   hp[HOOK_MAX_PARAM];
   const char *hookname = luaL_checkstring( L, 1 );

   /* Set up hooks. */
   if ( !lua_isnoneornil( L, 2 ) ) {
      /* Since this doesn't get saved and is triggered by Lua code, we can
       * actually pass references here. */
      hp[0].type = HOOK_PARAM_REF;
      lua_pushvalue( L, 2 );
      hp[0].u.ref = luaL_ref( L, LUA_REGISTRYINDEX );
      hp[1].type  = HOOK_PARAM_SENTINEL;
   } else
      hp[0].type = HOOK_PARAM_SENTINEL;

   /* Run the deferred hooks. */
   hooks_runParamDeferred( hookname, hp );
   return 0;
}

/**
 * @brief Tests a claim of a system or strings.
 *
 * @usage if not naev.claimTest( { system.get("Gamma Polaris") } ) then
 * print("Failed to claim!") end
 *
 *    @luatparam System|String|{System,String...} params Table of
 * systems/strings to claim or a single system/string.
 *    @luatparam[opt=false] boolean inclusive Whether or not to allow the claim
 * to include other inclusive claims. Multiple missions/events can inclusively
 * claim the same system, but only one system can exclusively claim it.
 *    @luatreturn boolean true if it is possible to claim, false otherwise.
 * @luafunc claimTest
 */
static int naevL_claimTest( lua_State *L )
{
   int      inclusive = lua_toboolean( L, 2 );
   Claim_t *claim     = claim_create( !inclusive );

   if ( lua_istable( L, 1 ) ) {
      /* Iterate over table. */
      lua_pushnil( L );
      while ( lua_next( L, 1 ) != 0 ) {
         if ( lua_issystem( L, -1 ) )
            claim_addSys( claim, lua_tosystem( L, -1 ) );
         else if ( lua_isstring( L, -1 ) )
            claim_addStr( claim, lua_tostring( L, -1 ) );
         lua_pop( L, 1 );
      }
   } else if ( lua_issystem( L, 1 ) )
      claim_addSys( claim, lua_tosystem( L, 1 ) );
   else if ( lua_isstring( L, 1 ) )
      claim_addStr( claim, lua_tostring( L, 1 ) );
   else
      NLUA_INVALID_PARAMETER( L, 1 );

   /* Only test, but don't apply case. */
   lua_pushboolean( L, !claim_test( claim ) );
   claim_destroy( claim );
   return 1;
}

/**
 * @brief Gets the list of available plugins.
 *
 *    @luatreturn table Table containing the list of plugins.
 * @luafunc plugins
 */
static int naevL_plugins( lua_State *L )
{
   const plugin_t *plugins = plugin_list();
   lua_newtable( L );
   for ( int i = 0; i < array_size( plugins ); i++ ) {
      const plugin_t *plg = &plugins[i];
      lua_newtable( L );

#define STRING( x )                                                            \
   lua_pushstring( L, plg->x );                                                \
   lua_setfield( L, -2, #x )
#define INTEGER( x )                                                           \
   lua_pushinteger( L, plg->x );                                               \
   lua_setfield( L, -2, #x )
#define BOOL( x )                                                              \
   lua_pushboolean( L, plg->x );                                               \
   lua_setfield( L, -2, #x )

      STRING( name );
      STRING( author );
      STRING( version );
      STRING( description );
      STRING( compatibility );
      STRING( mountpoint );

      INTEGER( priority );

      BOOL( compatible );
      BOOL( total_conversion );

#undef BOOL
#undef INTEGER
#undef STRING

      lua_rawseti( L, -2, i + 1 );
   }
   return 1;
}

/**
 * @brief Opens the info menu window.
 *
 * Possible window targets are: <br />
 *  - "main" : Main window.<br />
 *  - "ship" : Ship info window.<br />
 *  - "weapons" : Weapon configuration window.<br />
 *  - "cargo" : Cargo view window.<br />
 *  - "missions" : Mission view window.<br />
 *  - "standings" : Standings view window.<br />
 *
 * @usage naev.menuInfo( "ship" ) -- Opens ship tab
 *
 *    @luatparam[opt="main"] string window parameter indicating the tab to open
 * at.
 * @luafunc menuInfo
 */
static int naevL_menuInfo( lua_State *L )
{
   const char *str;
   int         window;

   if ( menu_open )
      return 0;

   if ( lua_gettop( L ) > 0 )
      str = luaL_checkstring( L, 1 );
   else {
      /* No parameter. */
      menu_info( INFO_DEFAULT );
      return 0;
   }

   /* Parse string. */
   if ( SDL_strcasecmp( str, "main" ) == 0 )
      window = INFO_MAIN;
   else if ( SDL_strcasecmp( str, "ship" ) == 0 )
      window = INFO_SHIP;
   else if ( SDL_strcasecmp( str, "weapons" ) == 0 )
      window = INFO_WEAPONS;
   else if ( SDL_strcasecmp( str, "cargo" ) == 0 )
      window = INFO_CARGO;
   else if ( SDL_strcasecmp( str, "missions" ) == 0 )
      window = INFO_MISSIONS;
   else if ( SDL_strcasecmp( str, "standings" ) == 0 )
      window = INFO_STANDINGS;
   else
      return NLUA_ERROR( L, _( "Invalid window info name '%s'." ), str );

   /* Open window. */
   menu_info( window );

   return 0;
}

/**
 * @brief Opens the small menu window.
 *
 * @usage naev.menuSmall()
 *
 *    @luatparam[opt=false] boolean info Show the info button.
 *    @luatparam[opt=false] boolean options Show the options button.
 *    @luatparam[opt=false] boolean allowsave Allow saving the game from the
 * menu (either directly or by exiting the game from the menu).
 * @luafunc menuSmall
 */
static int naevL_menuSmall( lua_State *L )
{
   menu_small( 0, lua_toboolean( L, 1 ), lua_toboolean( L, 2 ),
               lua_toboolean( L, 3 ) );
   return 0;
}

/**
 * @brief Checks to see if the game is paused.
 *
 *    @luatreturn boolean Whether or not the game is currently paused.
 * @luafunc isPaused
 */
static int naevL_isPaused( lua_State *L )
{
   lua_pushboolean( L, paused );
   return 1;
}

/**
 * @brief Pauses the game.
 *
 * @luafunc pause
 */
static int naevL_pause( lua_State *L )
{
   (void)L;
   pause_game();
   return 0;
}

/**
 * @brief Unpauses the game.
 *
 * Can not be run while landed.
 *
 * @luafunc unpause
 */
static int naevL_unpause( lua_State *L )
{
   if ( landed )
      return NLUA_ERROR( L, _( "Unable to unpause the game when landed!" ) );
   unpause_game();
   return 0;
}

/**
 * @brief Checks to see if text inputting is enabled.
 *
 *    @luatreturn boolean Whether or not text inputting is enabled.
 * @luafunc hasTextInput
 */
static int naevL_hasTextInput( lua_State *L )
{
   lua_pushboolean( L, SDL_EventEnabled( SDL_EVENT_TEXT_INPUT ) );
   return 1;
}

/**
 * @brief Enables or disables text inputting.
 *
 *    @luatparam boolean enable Whether text input events should be enabled.
 *    @luatparam integer Text rectangle x position.
 *    @luatparam integer Text rectangle y position.
 *    @luatparam integer Text rectangle width.
 *    @luatparam integer Text rectangle height.
 * @luafunc setTextInput
 */
static int naevL_setTextInput( lua_State *L )
{
   if ( lua_toboolean( L, 1 ) ) {
      SDL_Rect input_pos;
      input_pos.x = luaL_checkinteger( L, 2 );
      input_pos.y = luaL_checkinteger( L, 3 );
      input_pos.w = luaL_checkinteger( L, 4 );
      input_pos.h = luaL_checkinteger( L, 5 );
      SDL_SetEventEnabled( SDL_EVENT_TEXT_INPUT, 1 );
      SDL_StartTextInput( gl_screen.window );
      SDL_SetTextInputArea( gl_screen.window, &input_pos, 0 );
   } else {
      SDL_StopTextInput( gl_screen.window );
      SDL_SetEventEnabled( SDL_EVENT_TEXT_INPUT, 0 );
   }
   return 0;
}

/**
 * @brief Gets the translated string corresponding to an in-game unit.
 * <ul>
 *  <li>name: Name of the ship stat.</li>
 *  <li>display: Untranslated display name of the ship stat, which you use to
 * show to the player.<li> <li>unit: Untranslated unit name (if applicable,
 * otherwise nil).</li> <li>inverted: Boolean value whether or not negative is
 * "better" for the player.</li>
 * </ul>
 *    @luatreturn Table with all the ship stats information.
 * @luafunc shipstats
 */
static int naevL_shipstats( lua_State *L )
{
   ss_exportLua( L );
   return 1;
}

static const char *unittbl[] = {
   "time",         _UNIT_TIME,   "per_time",     _UNIT_PER_TIME, "distance",
   _UNIT_DISTANCE, "speed",      _UNIT_SPEED,    "accel",        _UNIT_ACCEL,
   "energy",       _UNIT_ENERGY, "power",        _UNIT_POWER,    "angle",
   _UNIT_ANGLE,    "rotation",   _UNIT_ROTATION, "mass",         _UNIT_MASS,
   "cpu",          _UNIT_CPU,    "unit",         _UNIT_UNIT,     "percent",
   _UNIT_PERCENT,
};
/**
 * @brief Gets the translated string corresponding to an in-game unit.
 *    @luaparam[opt=nil] string str Name of the unit to get or nil to get a
 * table with all of them.
 *    @luareturn Translated string corresponding to the unit or table of all
 * strings if no parameter is passed.
 * @luafunc unit
 */
static int naevL_unit( lua_State *L )
{
   if ( lua_isnoneornil( L, 1 ) ) {
      lua_newtable( L );
      for ( unsigned int i = 0; i < sizeof( unittbl ) / sizeof( unittbl[0] );
            i += 2 ) {
         lua_pushstring( L, _( unittbl[i + 1] ) );
         lua_setfield( L, -2, unittbl[i] );
      }
      return 1;
   } else {
      const char *str = luaL_checkstring( L, 1 );
      for ( unsigned int i = 0; i < sizeof( unittbl ) / sizeof( unittbl[0] );
            i += 2 ) {
         if ( strcmp( unittbl[i], str ) == 0 ) {
            lua_pushstring( L, _( unittbl[i + 1] ) );
            return 1;
         }
      }
   }
   NLUA_INVALID_PARAMETER( L, 1 );
}

/**
 * @brief Modifies the Naev internal quadtree lookup parameters.
 *
 *    @luatparam number max_elem Maximum amount of elements to allow in a leaf
 * node.
 *    @luatparam number depth depth Maximum depth to allow.
 * @luafunc quadtreeParams
 */
static int naevL_quadtreeParams( lua_State *L )
{
   int max_elem = luaL_checkinteger( L, 1 );
   int depth    = luaL_checkinteger( L, 2 );
   pilot_quadtreeParams( max_elem, depth );
   return 0;
}

/**
 * @brief Gets information about the current difficulty setting.
 *
 *    @luatparam boolean internal Whether or not to get values using the
 * internal format, or a human readable format.
 *    @luatreturn string Name (untranslated) of the difficulty setting.
 *    @luatreturn table Table containing the ship stats (name is key, while
 * value is value).
 * @luafunc difficulty
 */
static int naevL_difficulty( lua_State *L )
{
   const Difficulty *dif = difficulty_cur();
   lua_pushstring( L, dif->name );
   ss_statsGetLuaTableList( L, dif->stats, lua_toboolean( L, 1 ) );
   return 2;
}

/**
 * @brief Gets the difficulty level settings.
 *
 *    @luatreturn number Difficulty level settings.
 * @luafunc difficultyLevel
 */
static int naevL_difficultyLevel( lua_State *L )
{
   const Difficulty *dif = difficulty_cur();
   lua_pushnumber( L, dif->level );
   return 1;
}

#if DEBUGGING
/**
 * @brief Toggles the trail emitters.
 *
 * @usage naev.debugTrails() -- Trail emitters are marked with crosses.
 * @usage naev.debugTrails(false) -- Remove the markers.
 *
 *    @luatparam[opt=true] boolean state Whether to set or unset markers.
 * @luafunc debugTrails
 */
static int naevL_debugTrails( lua_State *L )
{
   int state = ( lua_gettop( L ) > 0 ) ? lua_toboolean( L, 1 ) : 1;
   if ( state )
      debug_setFlag( DEBUG_MARK_EMITTER );
   else
      debug_rmFlag( DEBUG_MARK_EMITTER );
   return 0;
}

/**
 * @brief Toggles the collision polygons.
 *
 *    @luatparam[opt=true] boolean state Whether or not to show the collision
 * polygons.
 * @luafunc debugCollisions
 */
static int naevL_debugCollisions( lua_State *L )
{
   int state = ( lua_gettop( L ) > 0 ) ? lua_toboolean( L, 1 ) : 1;
   if ( state )
      debug_setFlag( DEBUG_MARK_COLLISION );
   else
      debug_rmFlag( DEBUG_MARK_COLLISION );
   return 0;
}
#endif /* DEBUGGING */
