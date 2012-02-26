/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file music.c
 *
 * @brief Controls all the music playing.
 */


#include "music.h"

#include "naev.h"

#include "SDL.h"

#include "music_sdlmix.h"
#include "music_openal.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_var.h"
#include "nlua_music.h"
#include "log.h"
#include "ndata.h"
#include "conf.h"
#include "nstring.h"


#define MUSIC_PREFIX       "snd/music/" /**< Prefix of where tho find musics. */
#define MUSIC_SUFFIX       ".ogg" /**< Suffix of musics. */

#define MUSIC_LUA_PATH     "snd/music.lua" /**<  Lua music control file. */


#define CHUNK_SIZE         32 /**< Size of a chunk to allocate. */


int music_disabled = 0; /**< Whether or not music is disabled. */


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
static int music_runLua( const char *situation );


/*
 * The current music.
 */
static char *music_name       = NULL; /**< Current music name. */
static unsigned int music_start = 0; /**< Music start playing time. */
static double music_timer     = 0.; /**< Music timer. */


/*
 * Function pointers for backend.
 */
/* Init/exit. */
int  (*music_sys_init) (void)    = NULL;
void (*music_sys_exit) (void)    = NULL;
/* Loading. */
int  (*music_sys_load) ( const char* name, SDL_RWops *rw ) = NULL;
void (*music_sys_free) (void)    = NULL;
 /* Music control. */
int  (*music_sys_volume)( const double vol ) = NULL;
double (*music_sys_getVolume) (void) = NULL;
double (*music_sys_getVolumeLog) (void) = NULL;
void (*music_sys_play) (void)    = NULL;
void (*music_sys_stop) (void)    = NULL;
void (*music_sys_pause) (void)   = NULL;
void (*music_sys_resume) (void)  = NULL;
void (*music_sys_setPos) ( double sec ) = NULL;
int  (*music_sys_isPlaying) (void) = NULL;


/*
 * prototypes
 */
/* music stuff */
static int music_find (void);
static void music_free (void);
/* Lua stuff */
static int music_luaInit (void);
static void music_luaQuit (void);


/**
 * @brief Updates the music.
 */
void music_update( double dt )
{
   char buf[PATH_MAX];

   if (music_disabled)
      return;

   /* Timer stuff. */
   if (music_timer > 0.) {
      music_timer -= dt;
      if (music_timer <= 0.)
         music_runchoose = 1;
   }

   /* Lock music and see if needs to update. */
   SDL_mutexP(music_lock);
   if (music_runchoose == 0) {
      SDL_mutexV(music_lock);
      return;
   }
   music_runchoose = 0;
   strncpy(buf, music_situation, PATH_MAX);
   buf[ PATH_MAX-1 ] = '\0';
   SDL_mutexV(music_lock);
   music_runLua( buf );

   /* Make sure music is playing. */
   if (!music_isPlaying())
      music_choose("idle");
}


/**
 * @brief Runs the Lua music choose function.
 *
 *    @param situation Situation in to choose music for.
 *    @return 0 on success.
 */
static int music_runLua( const char *situation )
{
   int errf;
   lua_State *L;

   if (music_disabled)
      return 0;

   L = music_lua;

#if DEBUGGING
   lua_pushcfunction(L, nlua_errTrace);
   errf = -3;
#else /* DEBUGGING */
   errf = 0;
#endif /* DEBUGGING */

   /* Run the choose function in Lua. */
   lua_getglobal( L, "choose" );
   if (situation != NULL)
      lua_pushstring( L, situation );
   else
      lua_pushnil( L );
   if (lua_pcall(L, 1, 0, errf)) { /* error has occurred */
      WARN("Error while choosing music: %s", lua_tostring(L,-1));
      lua_pop(L,1);
   }
#if DEBUGGING
   lua_pop(L,1);
#endif /* DEBUGGING */

   return 0;
}


/**
 * @brief Initializes the music subsystem.
 *
 *    @return 0 on success.
 */
int music_init (void)
{
   if (music_disabled)
      return 0;

   if ((conf.sound_backend != NULL) &&
         (strcmp(conf.sound_backend,"sdlmix")==0)) {
#if USE_SDLMIX
      /*
       * SDL_mixer backend.
       */
      /* Init/exit. */
      music_sys_init = music_mix_init;
      music_sys_exit = music_mix_exit;
      /* Loading. */
      music_sys_load = music_mix_load;
      music_sys_free = music_mix_free;
      /* Music control. */
      music_sys_volume = music_mix_volume;
      music_sys_getVolume = music_mix_getVolume;
      music_sys_getVolumeLog = music_mix_getVolume;
      music_sys_load = music_mix_load;
      music_sys_play = music_mix_play;
      music_sys_stop = music_mix_stop;
      music_sys_pause = music_mix_pause;
      music_sys_resume = music_mix_resume;
      music_sys_setPos = music_mix_setPos;
      music_sys_isPlaying = music_mix_isPlaying;
#else /* USE_SDLMIX */
      WARN("SDL_mixer support not compiled in!");
      return -1;
#endif /* USE_SDLMIX */
   }
   else if ((conf.sound_backend != NULL) &&
         (strcmp(conf.sound_backend,"openal")==0)) {
#if USE_OPENAL
      /*
       * OpenAL backend.
       */
      /* Init/exit. */
      music_sys_init = music_al_init;
      music_sys_exit = music_al_exit;
      /* Loading. */
      music_sys_load = music_al_load;
      music_sys_free = music_al_free;
      /* Music control. */
      music_sys_volume = music_al_volume;
      music_sys_getVolume = music_al_getVolume;
      music_sys_getVolumeLog = music_al_getVolumeLog;
      music_sys_load = music_al_load;
      music_sys_play = music_al_play;
      music_sys_stop = music_al_stop;
      music_sys_pause = music_al_pause;
      music_sys_resume = music_al_resume;
      music_sys_setPos = music_al_setPos;
      music_sys_isPlaying = music_al_isPlaying;
#else /* USE_OPENAL */
      WARN("OpenAL support not compiled in!");
      return -1;
#endif /* USE_OPENAL*/
   }
   else {
      WARN("Unknown sound backend '%s'.", conf.sound_backend);
      return -1;
   }

   /* Start the subsystem. */
   if (music_sys_init())
      return -1;

   /* Load the music. */
   if (music_find() < 0)
      return -1;

   /* Start up Lua. */
   if (music_luaInit() < 0)
      return -1;

   /* Set the volume. */
   if ((conf.music > 1.) || (conf.music < 0.))
      WARN("Music has invalid value, clamping to [0:1].");
   music_volume(conf.music);

   /* Create the lock. */
   music_lock = SDL_CreateMutex();

   return 0;
}


/**
 * @brief Exits the music subsystem.
 */
void music_exit (void)
{
   if (music_disabled)
      return;

   /* Free the music. */
   music_free();

   /* Exit the subsystem. */
   music_sys_exit();

   /* Destroy the lock. */
   if (music_lock != NULL) {
      SDL_DestroyMutex(music_lock);
      music_lock = NULL;
   }

   /* Clean up Lua. */
   music_luaQuit();
}


/**
 * @brief Frees the current playing music.
 */
static void music_free (void)
{
   if (music_disabled)
      return;

   if (music_name != NULL) {
      free(music_name);
      music_name = NULL;
   }
   music_start = 0;

   music_sys_free();
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
   int suflen, flen;
   int nmusic;

   if (music_disabled)
      return 0;

   /* get the file list */
   files = ndata_list( MUSIC_PREFIX, &nfiles );

   /* load the profiles */
   nmusic = 0;
   suflen = strlen(MUSIC_SUFFIX);
   for (i=0; i<nfiles; i++) {
      flen = strlen(files[i]);
      if ((flen > suflen) &&
            strncmp( &files[i][flen - suflen], MUSIC_SUFFIX, suflen)==0) {

         /* grow the selection size */
         nmusic++;
      }

      /* Clean up. */
      free(files[i]);
   }

   DEBUG("Loaded %d song%c", nmusic, (nmusic==1)?' ':'s');

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
   if (music_disabled)
      return 0;

   return music_sys_volume( vol );
}


/**
 * @brief Gets the current music volume (linear).
 *
 *    @return The current music volume.
 */
double music_getVolume (void)
{
   if (music_disabled)
      return 0.;

   return music_sys_getVolume();
}


/**
 * @brief Gets the current music volume (logarithmic).
 *
 *    @return The current music volume.
 */
double music_getVolumeLog(void)
{
   if (music_disabled)
      return 0.;
   return music_sys_getVolumeLog();
}


/**
 * @brief Loads the music by name.
 *
 *    @param name Name of the file to load.
 */
int music_load( const char* name )
{
   SDL_RWops *rw;
   char filename[PATH_MAX];

   if (music_disabled)
      return 0;

   /* Free current music if needed. */
   music_free();

   /* Load new music. */
   music_name  = strdup(name);
   music_start = SDL_GetTicks();
   nsnprintf( filename, PATH_MAX, MUSIC_PREFIX"%s"MUSIC_SUFFIX, name);
   rw = ndata_rwops( filename );
   if (rw == NULL) {
      WARN("Music '%s' not found.", filename);
      return -1;
   }
   music_sys_load( name, rw );

   return 0;
}


/**
 * @brief Plays the loaded music.
 */
void music_play (void)
{
   if (music_disabled) return;

   music_sys_play();
}


/**
 * @brief Stops the loaded music.
 */
void music_stop (void)
{
   if (music_disabled) return;

   music_sys_stop();
}


/**
 * @brief Pauses the music.
 */
void music_pause (void)
{
   if (music_disabled) return;

   music_sys_pause();
}


/**
 * @brief Resumes the music.
 */
void music_resume (void)
{
   if (music_disabled) return;

   music_sys_resume();
}


/**
 * @brief Checks to see if the music is playing.
 *
 *    @return 0 if music isn't playing, 1 if is playing.
 */
int music_isPlaying (void)
{
   if (music_disabled)
      return 0; /* Always not playing when music is off. */

   return music_sys_isPlaying();
}


/**
 * @brief Gets the name of the current playing song.
 *
 *    @return Name of the current playing song.
 */
const char *music_playingName (void)
{
   if (music_disabled)
      return NULL;

   return music_name;
}


/**
 * @brief Gets the time since the music started playing.
 *
 *    @return The time since the music started playing.
 */
double music_playingTime (void)
{
   if (music_disabled)
      return 0.;

   return (double)(SDL_GetTicks() - music_start) / 1000.;
}


/**
 * @brief Sets the music to a position in seconds.
 *
 *    @param sec Position to go to in seconds.
 */
void music_setPos( double sec )
{
   if (music_disabled)
      return;

   music_sys_setPos( sec );
}


/*
 * music Lua stuff
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

   if (music_disabled)
      return 0;

   if (music_lua != NULL)
      music_luaQuit();

   music_lua = nlua_newState();
   nlua_loadBasic(music_lua);
   nlua_loadStandard(music_lua,1);
   nlua_loadMusic(music_lua,0); /* write it */

   /* load the actual Lua music code */
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
 * @brief Quits the music Lua control system.
 */
static void music_luaQuit (void)
{
   if (music_disabled)
      return;

   if (music_lua == NULL)
      return;

   lua_close(music_lua);
   music_lua = NULL;
}


/**
 * @brief Actually runs the music stuff, based on situation.
 *
 *    @param situation Choose a new music to play.
 *    @return 0 on success.
 */
int music_choose( const char* situation )
{
   if (music_disabled)
      return 0;

   music_timer = 0.;
   music_runLua( situation );

   return 0;
}



/**
 * @brief Actually runs the music stuff, based on situation after a delay.
 *
 *    @param situation Choose a new music to play after delay.
 *    @param delay Delay in seconds to delay the rechoose.
 *    @return 0 on success.
 */
int music_chooseDelay( const char* situation, double delay )
{
   if (music_disabled)
      return 0;

   /* Lock so it doesn't run in between an update. */
   SDL_mutexP(music_lock);
   music_timer       = delay;
   music_runchoose   = 0;
   strncpy(music_situation, situation, PATH_MAX);
   music_situation[ PATH_MAX-1 ] = '\0';
   SDL_mutexV(music_lock);

   return 0;
}


/**
 * @brief Attempts to rechoose the music.
 *
 * DO NOT CALL MIX_* FUNCTIONS FROM WITHIN THE CALLBACKS!
 */
void music_rechoose (void)
{
   if (music_disabled)
      return;

   /* Lock so it doesn't run in between an update. */
   SDL_mutexP(music_lock);
   music_timer       = 0.;
   music_runchoose   = 1;
   strncpy(music_situation, "idle", PATH_MAX);
   music_situation[ PATH_MAX-1 ] = '\0';
   SDL_mutexV(music_lock);
}

