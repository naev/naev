/*
 * See Licensing and Copyright notice in naev.h
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


#define MUSIC_PREFIX       "snd/music/"
#define MUSIC_SUFFIX       ".ogg"

#define MUSIC_LUA_PATH     "snd/music.lua"


int music_disabled = 0;


/*
 * global music lua
 */
static lua_State *music_lua = NULL;
/* functions */
static int musicL_load( lua_State* L );
static int musicL_play( lua_State* L );
static int musicL_stop( lua_State* L );
static const luaL_reg music_methods[] = {
   { "load", musicL_load },
   { "play", musicL_play },
   { "stop", musicL_stop },
   {0,0}
};


/*
 * what is available
 */
static char** music_selection = NULL;
static int nmusic_selection = 0;


/*
 * The current music.
 */
static void *music_data = NULL;
static SDL_RWops *music_rw = NULL;
static Mix_Music *music_music = NULL;


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


/*
 * init/exit
 */
int music_init (void)
{
   if (music_disabled) return 0;

   if (music_find() < 0) return -1;
   if (music_luaInit() < 0) return -1;
   music_volume(0.7);
   return 0;
}
void music_exit (void)
{
   music_free();
}


/*
 * Frees the current playing music.
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


/*
 * internal music loading routines
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


/*
 * music control functions
 */
int music_volume( const double vol )
{
   if (music_disabled) return 0;

   return Mix_VolumeMusic(MIX_MAX_VOLUME*vol);
}
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
void music_play (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeInMusic( music_music, 0, 500 ) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}
void music_stop (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeOutMusic(500) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/*
 * music lua stuff
 */
/* initialize */
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
      ERR("Error loading music file: %s",MUSIC_LUA_PATH);
      ERR("%s",lua_tostring(music_lua,-1));
      WARN("Most likely Lua file has improper syntax, please check");
      return -1;
   }
   free(buf);

   return 0;
}
/* destroy */
static void music_luaQuit (void)
{
   if (music_lua == NULL)
      return;

   lua_close(music_lua);
   music_lua = NULL;
}


/*
 * loads the music functions into a lua_State
 */
int lua_loadMusic( lua_State *L, int read_only )
{
   (void)read_only; /* future proof */
   luaL_register(L, "music", music_methods);
   return 0;
}


/*
 * actually runs the music stuff, based on situation
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


/*
 * Attempts to rechoose the music.
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



