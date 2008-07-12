/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file music.c
 *
 * @brief Controls all the music playing.
 */


#include "music.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include "nlua.h"
#include "nluadef.h"
#include "misn_lua.h"
#include "naev.h"
#include "log.h"
#include "pack.h"


#define MUSIC_PREFIX       "snd/music/" /**< Prefix of where tho find musics. */
#define MUSIC_SUFFIX       ".ogg" /**< Suffix of musics. */

#define MUSIC_LUA_PATH     "snd/music.lua" /**<  Lua music control file. */


int music_disabled = 0; /**< Whether or not music is disabled. */


/*
 * global music lua
 */
static lua_State *music_lua = NULL; /**< The Lua music control state. */
/* functions */
static int musicL_load( lua_State* L );
static int musicL_play( lua_State* L );
static int musicL_stop( lua_State* L );
static const luaL_reg music_methods[] = {
   { "load", musicL_load },
   { "play", musicL_play },
   { "stop", musicL_stop },
   {0,0}
}; /**< Music specific methods. */


/*
 * what is available
 */
static char** music_selection = NULL; /**< Available music selection. */
static int nmusic_selection = 0; /**< Size of available music selection. */


/*
 * The current music.
 */
static void *music_data = NULL; /**< Current music data. */
static SDL_RWops *music_rw = NULL; /**< Current music RWops. */
static Mix_Music *music_music = NULL; /**< Current music. */


/*
 * prototypes
 */
/* music stuff */
static int music_find (void);
static void music_free (void);
/* lua stuff */
static void music_rechoose (void);
static int music_luaInit (void);
static void music_luaQuit (void);


/**
 * @fn int music_init (void)
 *
 * @brief Initializes the music subsystem.
 *
 *    @return 0 on success.
 */
int music_init (void)
{
   if (music_disabled) return 0;

   if (music_find() < 0) return -1;
   if (music_luaInit() < 0) return -1;
   music_volume(0.8);
   return 0;
}


/**
 * @fn void music_exit (void)
 *
 * @brief Exits the music subsystem.
 */
void music_exit (void)
{
   music_free();
}


/**
 * @fn static void music_free (void)
 *
 * @brief Frees the current playing music.
 */
static void music_free (void)
{
   if (music_music != NULL) {
      Mix_HookMusicFinished(NULL);
      Mix_HaltMusic();
      Mix_FreeMusic(music_music);
      /*SDL_FreeRW(music_rw);*/ /* FreeMusic frees it itself */
      free(music_data);
      music_music = NULL;
      music_rw = NULL;
      music_data = NULL;
   }
}


/**
 * @fn static int music_find (void)
 *
 * @brief Internal music loading routines.
 *
 *    @return 0 on success.
 */
static int music_find (void)
{
   char** files;
   uint32_t nfiles,i;
   char tmp[64];
   int len;

   if (music_disabled) return 0;

   /* get the file list */
   files = pack_listfiles( data, &nfiles );

   /* load the profiles */
   for (i=0; i<nfiles; i++)
      if ((strncmp( files[i], MUSIC_PREFIX, strlen(MUSIC_PREFIX))==0) &&
            (strncmp( files[i] + strlen(files[i]) - strlen(MUSIC_SUFFIX),
                      MUSIC_SUFFIX, strlen(MUSIC_SUFFIX))==0)) {

         /* grow the selection size */
         music_selection = realloc( music_selection, ++nmusic_selection*sizeof(char*));

         /* remove the prefix and suffix */
         len = strlen(files[i]) - strlen(MUSIC_SUFFIX MUSIC_PREFIX);
         strncpy( tmp, files[i] + strlen(MUSIC_PREFIX), len );
         tmp[MIN(len,64-1)] = '\0';
         
         /* give it the new name */
         music_selection[nmusic_selection-1] = strdup(tmp);
      }

   /* free the char* allocated by pack */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   DEBUG("Loaded %d song%c", nmusic_selection, (nmusic_selection==1)?' ':'s');

   return 0;
}


/**
 * @fn int music_volume( const double vol )
 *
 * @brief Sets the music volume.
 *
 *    @param vol Volume to set to (between 0 and 1).
 *    @return 0 on success.
 */
int music_volume( const double vol )
{
   if (music_disabled) return 0;

   return Mix_VolumeMusic(MIX_MAX_VOLUME*vol);
}


/**
 * @fn void music_load( const char* name )
 *
 * @brief Loads the music by name.
 *
 *    @param name Name of the file to load.
 */
void music_load( const char* name )
{
   unsigned int size;
   char filename[PATH_MAX];

   if (music_disabled) return;

   music_free();

   /* Load the data */
   snprintf( filename, PATH_MAX, MUSIC_PREFIX"%s"MUSIC_SUFFIX, name); 
   music_data = pack_readfile( DATA, filename, &size );
   music_rw = SDL_RWFromMem(music_data, size);
   music_music = Mix_LoadMUS_RW(music_rw);
   if (music_music == NULL)
      WARN("SDL_Mixer: %s", Mix_GetError());

   Mix_HookMusicFinished(music_rechoose);
}


/**
 * @fn void music_play (void)
 *
 * @brief Plays the loaded music.
 */
void music_play (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeInMusic( music_music, 0, 500 ) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/**
 * @fn void music_stop (void)
 *
 * @brief Stops the loaded music.
 */
void music_stop (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeOutMusic(500) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/*
 * music lua stuff
 */
/**
 * @fn static int music_luaInit (void)
 *
 * @brief Initialize the music Lua control system.
 *
 *    @return 0 on success.
 */
static int music_luaInit (void)
{
   char *buf;
   uint32_t bufsize;

   if (music_lua != NULL)
      music_luaQuit();

   music_lua = nlua_newState();

   luaL_openlibs(music_lua);

   lua_loadSpace(music_lua,1); /* space and time are readonly */
   lua_loadTime(music_lua,1);
   lua_loadRnd(music_lua);
   lua_loadVar(music_lua,1); /* also read only */
   lua_loadMusic(music_lua,0); /* write it */

   /* load the actual lua music code */
   buf = pack_readfile( DATA, MUSIC_LUA_PATH, &bufsize );
   if (luaL_dobuffer(music_lua, buf, bufsize, MUSIC_LUA_PATH) != 0) {
      ERR("Error loading music file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check",
            MUSIC_LUA_PATH, lua_tostring(music_lua,-1) );
      return -1;
   }
   free(buf);

   return 0;
}


/**
 * @fn static void music_luaQuit (void)
 *
 * @brief Quits the music Lua contrtol system.
 */
static void music_luaQuit (void)
{
   if (music_lua == NULL)
      return;

   lua_close(music_lua);
   music_lua = NULL;
}


/**
 * @fn int lua_loadMusic( lua_State *L, int read_only )
 *
 * @brief Loads the music functions into a lua_State.
 *
 *    @param L Lua State to load the music functions into.
 *    @param read_only Load the write functions?
 *    @return 0 on success.
 */
int lua_loadMusic( lua_State *L, int read_only )
{
   (void)read_only; /* future proof */
   luaL_register(L, "music", music_methods);
   return 0;
}


/**
 * @fn int music_choose( char* situation )
 *
 * @brief Actually runs the music stuff, based on situation.
 *
 *    @param situation Choose a new music to play.
 *    @return 0 on success.
 */
int music_choose( char* situation )
{
   if (music_disabled) return 0;

   lua_getglobal( music_lua, "choose" );
   lua_pushstring( music_lua, situation );
   if (lua_pcall(music_lua, 1, 0, 0)) { /* error has occured */
      WARN("Error while choosing music: %s", (char*) lua_tostring(music_lua,-1));
      return -1;
   }

   return 0;
}


/**
 * @fn static void music_rechoose (void)
 *
 * @brief Attempts to rechoose the music.
 */
static void music_rechoose (void)
{
   music_choose("idle");
}


/*
 * the music lua functions
 */
static int musicL_load( lua_State *L )
{
   char* str;

   /* check parameters */
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L,1)) str = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   music_load( str );
   return 0;
}
static int musicL_play( lua_State *L )
{
   (void)L;
   music_play();
   return 0;
}
static int musicL_stop( lua_State *L )
{
   (void)L;
   music_stop();
   return 0;
}



