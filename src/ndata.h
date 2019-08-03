/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NDATA_H
#  define NDATA_H


#include <stdint.h>

#include "SDL.h"


/*
 * Define various paths
 */
#define PLANET_GFX_SPACE_PATH    "dat/gfx/planet/space/" /**< Location of planet space graphics. */
#define PLANET_GFX_EXTERIOR_PATH "dat/gfx/planet/exterior/" /**< Location of planet exterior graphics (when landed). */
#define GFX_PATH                 "dat/gfx/" /**< Location of the graphics root. */
#define GUI_GFX_PATH             "dat/gfx/gui/" /**< Location of the GUI graphics. */
#define PORTRAIT_GFX_PATH        "dat/gfx/portraits/" /**< Location of the portrait graphics. */
#define SHIP_GFX_PATH            "dat/gfx/ship/" /**< Location of ship graphics. */
#define OUTFIT_GFX_PATH          "dat/gfx/outfit/" /**< Path to outfit graphics. */
#define SPFX_GFX_PATH            "dat/gfx/spfx/" /**< location of the graphic */
#define FACTION_LOGO_PATH        "dat/gfx/logo/" /**< Path to logo gfx. */
#define SOUND_PATH               "dat/snd/sounds/" /**< Location of the sounds. */
#define COMMODITY_GFX_PATH          "dat/gfx/commodity/" /**< Path to commodities graphics. */
#define MAP_DECORATOR_GFX_PATH   "dat/gfx/map/"

#define FACTION_DATA_PATH        "dat/faction.xml" /**< Faction xml file. */
#define MISSION_DATA_PATH        "dat/mission.xml" /**< Path to missions XML. */
#define EVENT_DATA_PATH          "dat/event.xml" /**< Path to events XML. */
#define SPFX_DATA_PATH           "dat/spfx.xml" /**< Location of the spfx datafile. */
#define DTYPE_DATA_PATH          "dat/damagetype.xml" /**< Location of the spfx datafile. */
#define COMMODITY_DATA_PATH      "dat/commodity.xml" /**< Commodity XML file. */
#define FLEET_DATA_PATH          "dat/fleet.xml" /**< Where to find fleet data. */
#define TECH_DATA_PATH           "dat/tech.xml"   /**< XML file containing techs. */
#define DIFF_DATA_PATH           "dat/unidiff.xml" /**< Unidiff XML file. */
#define ASTERO_DATA_PATH         "dat/asteroids.xml" /**< Asteroid types XML file. */
#define MAP_DECORATOR_DATA_PATH  "dat/map.xml" /**< Commodity XML file. */

#define MISSION_LUA_PATH         "dat/missions/" /**< Path to Lua files. */
#define EVENT_LUA_PATH           "dat/events/" /**< Path to Lua files. */
#define OUTFIT_DATA_PATH         "dat/outfits/" /**< Path to outfits. */
#define MAP_DATA_PATH            "dat/outfits/maps/" /**< Path to maps. */
#define PLANET_DATA_PATH         "dat/assets/" /**< Path to planets. */
#define SYSTEM_DATA_PATH         "dat/ssys/" /**< Path to systems. */
#define SHIP_DATA_PATH           "dat/ships/" /**< Path to ships. */

#define LANDING_DATA_PATH        "dat/landing.lua" /**< Lua script containing landing data. */

#define NEBULA_PATH              "nebula/" /**< Path to nebula files. */

#define MUSIC_PATH               "dat/snd/music/" /**< Prefix of where to find musics. */
#define MUSIC_LUA_PATH           "dat/snd/music.lua" /**< Lua music control file. */

#define START_DATA_PATH          "dat/start.xml" /**< Path to module start file. */

#define FONT_DEFAULT_PATH        "dat/font.ttf" /**< Default font path. */
#define FONT_MONOSPACE_PATH      "dat/mono.ttf" /**< Default monospace font path. */

#define LUA_INCLUDE_PATH         "dat/scripts/" /**< Path for Lua includes. */
#define AI_PATH                  "dat/ai/" /**< Location of the AI files. */

/*
 * ndata open/close
 */
int ndata_open (void);
void ndata_close (void);

/*
 * General.
 */
int ndata_check( const char* path );
int ndata_setPath( const char* path );
const char* ndata_getDirname(void);
const char* ndata_getPath (void);
const char* ndata_name (void);

/*
 * Individual file functions.
 */
int ndata_exists( const char* filename );
void* ndata_read( const char* filename, size_t *filesize );
char** ndata_list( const char *path, size_t* nfiles );
char** ndata_listRecursive( const char *path, size_t* nfiles );
void ndata_sortName( char **files, size_t nfiles );


/*
 * RWops.
 */
SDL_RWops *ndata_rwops( const char* filename );


#endif /* NDATA_H */

