/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file music.c
 *
 * @brief Controls all the music playing.
 */

/** @cond */
#include "physfsrwops.h"
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "music.h"

#include "conf.h"
#include "log.h"
#include "music_openal.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_audio.h"
#include "nlua_var.h"
#include "nluadef.h"
#include "nstring.h"
#include "sound.h"

#define MUSIC_SUFFIX       ".ogg" /**< Suffix of musics. */

int music_disabled = 0; /**< Whether or not music is disabled. */

/*
 * Handle if music should run Lua script.  Must be locked to ensure same
 *  behaviour always.
 */
static int music_runchoose = 0; /**< Whether or not music should run the choose function. */

/*
 * global music lua
 */
static nlua_env music_env     = LUA_NOREF; /**< The Lua music control env. */
static int music_lua_update   = LUA_NOREF;
static int music_lua_choose   = LUA_NOREF;
static int music_lua_play     = LUA_NOREF;
static int music_lua_stop     = LUA_NOREF;
static int music_lua_pause    = LUA_NOREF;
static int music_lua_resume   = LUA_NOREF;
static int music_lua_info     = LUA_NOREF;

/* functions */
static int music_runLua( const char *situation );

/*
 * The current music.
 */
static char *music_name       = NULL; /**< Current music name. */
static unsigned int music_start = 0; /**< Music start playing time. */
static int music_temp_disabled= 0; /**< Music is temporarily disabled. */
static int music_temp_repeat  = 0; /**< Music is repeating. */
static char *music_temp_repeatname = NULL; /**< Repeating song name. */

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
   if (music_disabled)
      return;

   if (music_temp_disabled)
      return;

   /* Run the choose function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_update );
   lua_pushnumber( naevL, dt );
   if (nlua_pcall(music_env, 1, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "update", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}

/**
 * @brief Runs the Lua music choose function.
 *
 *    @param situation Situation in to choose music for.
 *    @return 0 on success.
 */
static int music_runLua( const char *situation )
{
   if (music_disabled)
      return 0;

   if (music_temp_disabled)
      return 0;

   if (music_temp_repeat) {
      music_load( music_temp_repeatname );
      return 0;
   }

   /* Run the choose function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_choose );
   if (situation != NULL)
      lua_pushstring( naevL, situation );
   else
      lua_pushnil( naevL );
   if (nlua_pcall(music_env, 1, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "choose", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

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

   /* Load the music. */
   if (music_find() < 0)
      return -1;

   /* Start up Lua. */
   if (music_luaInit() < 0)
      return -1;

   /* Set the volume. */
   if ((conf.music > 1.) || (conf.music < 0.))
      WARN(_("Music has invalid value, clamping to [0:1]."));
   music_volume(conf.music);

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
}

/**
 * @brief Internal music loading routines.
 *
 *    @return 0 on success.
 */
static int music_find (void)
{
   char** files;
   int suflen;
   int nmusic;

   if (music_disabled)
      return 0;

   /* get the file list */
   files = PHYSFS_enumerateFiles( MUSIC_PATH );

   /* load the profiles */
   nmusic = 0;
   suflen = strlen(MUSIC_SUFFIX);
   for (size_t i=0; files[i]!=NULL; i++) {
      int flen = strlen(files[i]);
      if ((flen > suflen) &&
            strncmp( &files[i][flen - suflen], MUSIC_SUFFIX, suflen)==0) {

         /* grow the selection size */
         nmusic++;
      }
   }

   DEBUG( n_("Loaded %d Song", "Loaded %d Songs", nmusic ), nmusic );

   /* More clean up. */
   PHYSFS_freeList(files);

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

   return music_al_volume( vol );
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

   return music_al_getVolume();
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
   return music_al_getVolumeLog();
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

   /* Determine the filename. */
   if (name[0]=='/')
      snprintf( filename, sizeof(filename), "%s", &name[1]);
   else
      snprintf( filename, sizeof(filename), MUSIC_PATH"%s"MUSIC_SUFFIX, name);

   /* Load new music. */
   music_name  = strdup(name);
   music_start = SDL_GetTicks();
   rw = PHYSFSRWOPS_openRead( filename );
   if (rw == NULL) {
      WARN(_("Music '%s' not found."), filename);
      return -1;
   }
   music_al_load( name, rw );

   return 0;
}

/**
 * @brief Plays the loaded music.
 */
void music_play( const char *filename )
{
   if (music_disabled) return;

   /* Run the play function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_play );
   if (filename != NULL)
      lua_pushstring( naevL, filename );
   else
      lua_pushnil( naevL );
   if (nlua_pcall(music_env, 1, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "play", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}

/**
 * @brief Stops the loaded music.
 */
void music_stop (void)
{
   if (music_disabled) return;

   /* Run the stop function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_stop );
   if (nlua_pcall(music_env, 0, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "stop", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}

/**
 * @brief Pauses the music.
 */
void music_pause (void)
{
   if (music_disabled) return;

   /* Run the pause function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_pause );
   if (nlua_pcall(music_env, 0, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "pause", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}

/**
 * @brief Resumes the music.
 */
void music_resume (void)
{
   if (music_disabled) return;

   /* Run the resume function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_resume );
   if (nlua_pcall(music_env, 0, 0)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "resume", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}


static MusicInfo_t minfo;
MusicInfo_t* music_info (void)
{
   if (minfo.name) {
      free( minfo.name );
      minfo.name = NULL;
   }

   /* Run the info function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_info );
   if (nlua_pcall(music_env, 0, 3)) { /* error has occurred */
      WARN(_("Error while running music function '%s': %s"), "info", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

   minfo.playing  = lua_toboolean(naevL,-3);
   minfo.name     = strdup(luaL_optstring(naevL,-2,NULL));
   minfo.pos      = luaL_optnumber(naevL,-1,-1);

   lua_pop(naevL,3);

   return &minfo;
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

   return music_al_isPlaying();
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

   music_al_setPos( sec );
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
   size_t bufsize;

   if (music_disabled)
      return 0;

   if (music_env != LUA_NOREF)
      music_luaQuit();

   /* Reset the environment. */
   music_env = nlua_newEnv();
   nlua_loadStandard(music_env);
   nlua_loadAudio(music_env);

   /* load the actual Lua music code */
   buf = ndata_read( MUSIC_LUA_PATH, &bufsize );
   if (nlua_dobufenv(music_env, buf, bufsize, MUSIC_LUA_PATH) != 0) {
      ERR(_("Error loading music file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check"),
            MUSIC_LUA_PATH, lua_tostring(naevL,-1) );
      return -1;
   }
   free(buf);

   /* Set up comfort functions. */
   music_lua_choose = nlua_refenvtype( music_env, "choose", LUA_TFUNCTION );
   music_lua_update = nlua_refenvtype( music_env, "update", LUA_TFUNCTION );
   music_lua_play   = nlua_refenvtype( music_env, "play", LUA_TFUNCTION );
   music_lua_stop   = nlua_refenvtype( music_env, "stop", LUA_TFUNCTION );
   music_lua_pause  = nlua_refenvtype( music_env, "pause", LUA_TFUNCTION );
   music_lua_resume = nlua_refenvtype( music_env, "resume", LUA_TFUNCTION );
   music_lua_info   = nlua_refenvtype( music_env, "info", LUA_TFUNCTION );

   /* Free repeatname. */
   free( music_temp_repeatname );

   return 0;
}

/**
 * @brief Quits the music Lua control system.
 */
static void music_luaQuit (void)
{
   if (music_disabled)
      return;

   nlua_freeEnv(music_env);
   music_env         = LUA_NOREF;
   music_lua_choose  = LUA_NOREF;
   music_lua_update  = LUA_NOREF;
   music_lua_play    = LUA_NOREF;
   music_lua_stop    = LUA_NOREF;
   music_lua_pause   = LUA_NOREF;
   music_lua_resume  = LUA_NOREF;
   music_lua_info    = LUA_NOREF;
}

/**
 * @brief Actually runs the music stuff, based on situation.
 *
 *    @param situation Choose a new music to play.
 *    @return 0 on success.
 */
int music_choose( const char* situation )
{
   if (music_disabled || sound_disabled)
      return 0;

   music_temp_repeat = 0;
   music_temp_disabled = 0;
   music_runLua( situation );

   return 0;
}

/**
 * @brief Attempts to rechoose the music.
 */
void music_rechoose (void)
{
   if (music_disabled)
      return;

   if (music_temp_disabled)
      return;

   /* Lock so it doesn't run in between an update. */
   music_runchoose   = 1;
   music_temp_disabled = 0;
}

/**
 * @brief Temporarily disables the music.
 */
void music_tempDisable( int disable )
{
   music_temp_disabled = disable;
}

/**
 * @brief Temporarily makse the music repeat.
 */
void music_repeat( int repeat )
{
   if (music_disabled)
      return;

   if (music_name == NULL) {
      WARN(_("Trying to set music on repeat when no music is loaded!"));
      return;
   }

   music_temp_repeat = repeat;
   free( music_temp_repeatname );
   music_temp_repeatname = strdup( music_name );
}
