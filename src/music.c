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
#include "SDL_mutex.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_misn.h"
#include "naev.h"
#include "log.h"
#include "ndata.h"


#define MUSIC_PREFIX       "snd/music/" /**< Prefix of where tho find musics. */
#define MUSIC_SUFFIX       ".ogg" /**< Suffix of musics. */

#define MUSIC_LUA_PATH     "snd/music.lua" /**<  Lua music control file. */


#define CHUNK_SIZE         32 /**< Size of a chunk to allocate. */


int music_disabled = 0; /**< Whether or not music is disabled. */
double music_defVolume = 0.8; /**< Music default volume. */


/*
 * Handle if music should run Lua script.  Must be locked to ensure same
 *  behaviour always.
 */
static SDL_mutex *music_lock = NULL; /**< lock for music_runLua so it doesn't
                                          run twice in a row with weird
                                          results.
                                          DO NOT CALL MIX_* FUNCTIONS WHEN
                                          LOCKED!!! */
static int music_runchoose = 0; /**< Whether or not music should run the choose function. */
static char music_situation[PATH_MAX]; /**< What situation music is in. */


/*
 * global music lua
 */
static lua_State *music_lua = NULL; /**< The Lua music control state. */
/* functions */
static int music_runLua( char *situation );
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
 * @brief Updates the music.
 */
void music_update (void)
{
   char buf[PATH_MAX];

   if (music_disabled) return;

   /* Lock music and see if needs to update. */
   SDL_mutexP(music_lock);
   if (music_runchoose == 0) {
      SDL_mutexV(music_lock);
      return;
   }
   music_runchoose = 0;
   strncpy(buf, music_situation, PATH_MAX);
   SDL_mutexV(music_lock);

   music_runLua( buf );

}


/**
 * @brief Runs the Lua music choose function.
 *
 *    @param situation Situation in to choose music for.
 *    @return 0 on success.
 */
static int music_runLua( char *situation )
{
   if (music_disabled) return 0;

   /* Run the choose function in Lua. */
   lua_getglobal( music_lua, "choose" );
   if (situation != NULL)
      lua_pushstring( music_lua, situation );
   else
      lua_pushnil( music_lua );
   if (lua_pcall(music_lua, 1, 0, 0)) /* error has occured */
      WARN("Error while choosing music: %s", (char*) lua_tostring(music_lua,-1));

   return 0;
}


/**
 * @brief Initializes the music subsystem.
 *
 *    @return 0 on success.
 */
int music_init (void)
{
   if (music_disabled) return 0;

   if (music_find() < 0) return -1;
   if (music_luaInit() < 0) return -1;
   music_volume(music_defVolume);

   /* Create the lock. */
   music_lock = SDL_CreateMutex();

   return 0;
}


/**
 * @brief Exits the music subsystem.
 */
void music_exit (void)
{
   music_free();

   /* Destroy the lock. */
   if (music_lock != NULL) {
      SDL_DestroyMutex(music_lock);
      music_lock = NULL;
   }
}


/**
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
 * @brief Internal music loading routines.
 *
 *    @return 0 on success.
 */
static int music_find (void)
{
   char** files;
   uint32_t nfiles,i;
   char tmp[64];
   int len, suflen, flen;
   int mem;

   if (music_disabled) return 0;

   /* get the file list */
   files = ndata_list( MUSIC_PREFIX, &nfiles );

   /* load the profiles */
   mem = 0;
   suflen = strlen(MUSIC_SUFFIX);
   for (i=0; i<nfiles; i++) {
      flen = strlen(files[i]);
      if ((flen > suflen) &&
            strncmp( &files[i][flen - suflen], MUSIC_SUFFIX, suflen)==0) {

         /* grow the selection size */
         nmusic_selection++;
         if (nmusic_selection > mem) {
            mem += CHUNK_SIZE;
            music_selection = realloc( music_selection, sizeof(char*)*mem);
         }

         /* remove the prefix and suffix */
         len = flen - suflen;
         strncpy( tmp, files[i], len );
         tmp[MIN(len,64-1)] = '\0';
         
         /* give it the new name */
         music_selection[nmusic_selection-1] = strdup(tmp);
      }

      /* Clean up. */
      free(files[i]);
   }
   music_selection = realloc( music_selection, sizeof(char*)*nmusic_selection);

   DEBUG("Loaded %d song%c", nmusic_selection, (nmusic_selection==1)?' ':'s');

   /* More clean up. */
   free(files);

   return 0;
}


/**
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
   music_data = ndata_read( filename, &size );
   music_rw = SDL_RWFromMem(music_data, size);
   music_music = Mix_LoadMUS_RW(music_rw);
   if (music_music == NULL)
      WARN("SDL_Mixer: %s", Mix_GetError());

   Mix_HookMusicFinished(music_rechoose);
}


/**
 * @brief Plays the loaded music.
 */
void music_play (void)
{
   if (music_disabled) return;

   if (music_music == NULL) return;

   if (Mix_FadeInMusic( music_music, 0, 500 ) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/**
 * @brief Stops the loaded music.
 */
void music_stop (void)
{
   if (music_disabled) return;

   if (music_music == NULL) return;

   if (Mix_FadeOutMusic(500) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/**
 * @brief Pauses the music.
 */
void music_pause (void)
{
   if (music_disabled) return;

   if (music_music == NULL) return;

   Mix_PauseMusic();
}


/**
 * @brief Resumes the music.
 */
void music_resume (void)
{
   if (music_disabled) return;

   if (music_music == NULL) return;

   Mix_ResumeMusic();
}


/**
 * @brief Sets the music to a position in seconds.
 *
 *    @param sec Position to go to in seconds.
 */
void music_setPos( double sec )
{
   if (music_disabled) return;

   if (music_music == NULL) return;

   Mix_FadeInMusicPos( music_music, 1, 1000, sec );
}


/*
 * music lua stuff
 */
/**
 * @brief Initialize the music Lua control system.
 *
 *    @return 0 on success.
 */
static int music_luaInit (void)
{
   char *buf;
   uint32_t bufsize;

   if (music_disabled) return 0;

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
   buf = ndata_read( MUSIC_LUA_PATH, &bufsize );
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
 * @brief Quits the music Lua contrtol system.
 */
static void music_luaQuit (void)
{
   if (music_disabled) return;

   if (music_lua == NULL)
      return;

   lua_close(music_lua);
   music_lua = NULL;
}


/**
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
 * @brief Actually runs the music stuff, based on situation.
 *
 *    @param situation Choose a new music to play.
 *    @return 0 on success.
 */
int music_choose( char* situation )
{
   if (music_disabled) return 0;

   music_runLua( situation );

   return 0;
}


/**
 * @brief Attempts to rechoose the music.
 *
 * DO NOT CALL MIX_* FUNCTIONS FROM WITHIN THE CALLBACKS!
 */
static void music_rechoose (void)
{
   if (music_disabled) return;

   /* Lock so it doesn't run in between an update. */
   SDL_mutexP(music_lock);
   music_runchoose = 1;
   strncpy(music_situation, "idle", PATH_MAX);
   SDL_mutexV(music_lock);
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



