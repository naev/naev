/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/*
 * Define various paths
 */
#define SPOB_GFX_SPACE_PATH      "gfx/spob/space/" /**< Location of spob space graphics. */
#define SPOB_GFX_EXTERIOR_PATH   "gfx/spob/exterior/" /**< Location of spob exterior graphics (when landed). */
#define SPOB_GFX_COMM_PATH       "gfx/spob/comm/" /**< Location of spob communication graphics (when hailed). */
#define GFX_PATH                 "gfx/" /**< Location of the graphics root. */
#define OVERLAY_GFX_PATH         "gfx/overlays/" /**< Location of overlays such as those used with graphics and ships. */
#define GUI_GFX_PATH             "gfx/gui/" /**< Location of the GUI graphics. */
#define PORTRAIT_GFX_PATH        "gfx/portraits/" /**< Location of the portrait graphics. */
#define SHIP_GFX_PATH            "gfx/ship/" /**< Location of ship graphics. */
#define SHIP_3DGFX_PATH          "gfx/ship/3d/" /**< Location of ship 3d graphics. */
#define OUTFIT_GFX_PATH          "gfx/outfit/" /**< Path to outfit graphics. */
#define SPFX_GFX_PATH            "gfx/spfx/" /**< location of the graphic */
#define FACTION_LOGO_PATH        "gfx/logo/" /**< Path to logo gfx. */
#define SOUND_PATH               "snd/sounds/" /**< Location of the sounds. */
#define COMMODITY_GFX_PATH       "gfx/commodity/" /**< Path to commodities graphics. */
#define MAP_DECORATOR_GFX_PATH   "gfx/map/"
#define SHIP_POLYGON_PATH        "gfx/ship_polygon/" /**< Path to ship's collision polygon. */
#define OUTFIT_POLYGON_PATH      "gfx/outfit/space_polygon/" /**< Path to outfit collision polygon. */
#define ASTEROID_POLYGON_PATH    "gfx/spob/space/asteroid_polygon/" /**< Path to asteroid collision polygon. */

#define FACTION_DATA_PATH        "factions/" /**< Path to factions. */
#define MISSION_DATA_PATH        "missions/" /**< Path to missions XML. */
#define EVENT_DATA_PATH          "events/" /**< Path to events XML. */
#define UNIDIFF_DATA_PATH        "unidiff/" /**< Path to unidiff XML. */
#define SPFX_DATA_PATH           "spfx/" /**< Location of the spfx datafiles. */
#define DTYPE_DATA_PATH          "damagetype/" /**< Damage-type definitions. */
#define SP_DATA_PATH             "slots/" /**< Location of the sp datafile. */
#define COMMODITY_DATA_PATH      "commodities/" /**< Path to commodities. */
#define TECH_DATA_PATH           "tech/"   /**< Path totechs. */
#define ASTEROID_TYPES_DATA_PATH "asteroids/types/" /**< Asteroid types XML file location path. */
#define ASTEROID_GROUPS_DATA_PATH "asteroids/groups/" /**< Asteroid groups XML file location path. */
#define MAP_DECORATOR_DATA_PATH  "map_decorator/" /**< Where the map has background images. */
#define TRAIL_DATA_PATH          "trails/" /**< Trail types XML file. */

#define MISSION_LUA_PATH         "missions/" /**< Path to Lua files. */
#define EVENT_LUA_PATH           "events/" /**< Path to Lua files. */
#define OUTFIT_DATA_PATH         "outfits/" /**< Path to outfits. */
#define EFFECT_DATA_PATH         "effects/" /**< Path to effects. */
#define MAP_DATA_PATH            "outfits/maps/" /**< Path to maps. */
#define SPOB_DATA_PATH           "spob/" /**< Path to spobs. */
#define VIRTUALSPOB_DATA_PATH    "spob_virtual/" /**< Path to spobs. */
#define SYSTEM_DATA_PATH         "ssys/" /**< Path to systems. */
#define SHIP_DATA_PATH           "ships/" /**< Path to ships. */

#define LUA_COMMON_PATH          "common.lua" /**< Common Lua functions. */
#define LOADSCREEN_DATA_PATH     "loadscreen.lua" /**< Script for the load screen. */

#define MUSIC_PATH               "snd/music/" /**< Prefix of where to find musics. */
#define MUSIC_LUA_PATH           "snd/music.lua" /**< Lua music control file. */

#define START_DATA_PATH          "start.xml" /**< Path to module start file. */

/* Fonts should be defined in start.xml probably. */
/* Currently our fonts/Cabin-SemiBold.otf lacks many fairly standard glyphs, so we are falling back to
 * the monospace font which has better coverage.
 * TODO solve this issue in a sane way. */
#define FONT_PATH_PREFIX         "fonts/"
#define FONT_DEFAULT_PATH        N_("Cabin-SemiBold.otf,NanumBarunGothicBold.ttf,SourceCodePro-Semibold.ttf,IBMPlexSansJP-Medium.otf") /**< Default font path. */
#define FONT_MONOSPACE_PATH      N_("SourceCodePro-Semibold.ttf,D2CodingBold.ttf,IBMPlexSansJP-Medium.otf") /**< Default monospace font path. */

#define LUA_INCLUDE_PATH         "scripts/" /**< Path for Lua includes. */
#define AI_PATH                  "ai/" /**< Location of the AI files. */

#define GLSL_PATH                "glsl/"

#define AI_EQUIP_PATH            "factions/equip/generic.lua"
#define GUI_PATH                 "gui/"
#define FACTIONS_PATH            "factions/"
#define GETTEXT_PATH             "gettext/" /* Doesn't use ndata functions. */
#define GETTEXT_STATS_PATH       "gettext_stats/"
#define BACKGROUND_PATH          "bkg/"
#define INTRO_PATH               "intro"
#define RESCUE_PATH              "rescue.lua"
#define AUTOEQUIP_PATH           "autoequip.lua"
#define COMM_PATH                "comm.lua"
#define BOARD_PATH               "board.lua"
#define SAVE_UPDATER_PATH        "save_updater.lua"
#define DIFFICULTY_PATH          "difficulty/"

void ndata_setupWriteDir (void);
void ndata_setupReadDirs (void);
void* ndata_read( const char* filename, size_t *filesize );
char** ndata_listRecursive( const char *path );
int ndata_backupIfExists( const char *path );
int ndata_copyIfExists( const char *path1, const char *path2 );
int ndata_matchExt( const char *path, const char *ext );
int ndata_getPathDefault( char *path, int len, const char *default_path, const char *filename );
