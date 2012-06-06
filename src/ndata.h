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
#define PLANET_GFX_SPACE_PATH    "gfx/planet/space/" /**< Location of planet space graphics. */
#define PLANET_GFX_EXTERIOR_PATH "gfx/planet/exterior/" /**< Location of planet exterior graphics (when landed). */
#define GUI_GFX_PATH             "gfx/gui/" /**< Location of the GUI graphics. */
#define SHIP_GFX_PATH            "gfx/ship/" /**< Location of ship graphics. */
#define OUTFIT_GFX_PATH          "gfx/outfit/" /**< Path to outfit graphics. */
#define SPFX_GFX_PATH            "gfx/spfx/" /**< location of the graphic */
#define FACTION_LOGO_PATH        "gfx/logo/" /**< Path to logo gfx. */

#define FACTION_DATA_PATH        "dat/faction.xml" /**< Faction xml file. */
#define MISSION_DATA_PATH        "dat/mission.xml" /**< Path to missions XML. */
#define EVENT_DATA_PATH          "dat/event.xml" /**< Path to events XML. */
#define SPFX_DATA_PATH           "dat/spfx.xml" /**< Location of the spfx datafile. */
#define DTYPE_DATA_PATH          "dat/damagetype.xml" /**< Location of the spfx datafile. */
#define COMMODITY_DATA_PATH      "dat/commodity.xml" /**< Commodity XML file. */
#define FLEET_DATA_PATH          "dat/fleet.xml" /**< Where to find fleet data. */
#define TECH_DATA_PATH           "dat/tech.xml"   /**< XML file containing techs. */
#define DIFF_DATA_PATH           "dat/unidiff.xml" /**< Unidiff XML file. */

#define MISSION_LUA_PATH         "dat/missions/" /**< Path to Lua files. */
#define EVENT_LUA_PATH           "dat/events/" /**< Path to Lua files. */
#define OUTFIT_DATA_PATH         "dat/outfits/" /**< Path to outfits. */
#define MAP_DATA_PATH            "dat/outfits/maps/" /**< Path to maps. */
#define PLANET_DATA_PATH         "dat/assets/" /**< Path to planets. */
#define SYSTEM_DATA_PATH         "dat/ssys/" /**< Path to systems. */
#define SHIP_DATA_PATH           "dat/ships/" /**< Path to ships. */

#define LANDING_DATA_PATH        "dat/landing.lua" /**< Lua script containing landing data. */

#define NEBULA_PATH              "nebula/" /**< Path to nebula files. */

#define MUSIC_PATH               "snd/music/" /**< Prefix of where to find musics. */
#define MUSIC_LUA_PATH           "snd/music.lua" /**< Lua music control file. */

#define START_DATA_PATH          "dat/start.xml" /**< Path to module start file. */

#define FONT_DEFAULT_PATH        "dat/font.ttf" /**< Default font path. */
#define OMSG_FONT_DEFAULT_PATH   "dat/mono.ttf" /**< Default font path. */

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
void* ndata_read( const char* filename, uint32_t *filesize );
char** ndata_list( const char *path, uint32_t* nfiles );
char** ndata_listRecursive( const char *path, uint32_t* nfiles );
void ndata_sortName( char **files, uint32_t nfiles );


/*
 * RWops.
 */
SDL_RWops *ndata_rwops( const char* filename );


#endif /* NDATA_H */

