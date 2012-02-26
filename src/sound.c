/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file sound.c
 *
 * @brief Handles all the sound details.
 */


#include "sound.h"

#include "naev.h"

#include <sys/stat.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"

#include "sound_priv.h"
#include "sound_openal.h"
#include "sound_sdlmix.h"
#include "log.h"
#include "nstring.h"
#include "ndata.h"
#include "music.h"
#include "physics.h"
#include "conf.h"
#include "player.h"
#include "camera.h"


#define SOUND_PREFIX       "snd/sounds/" /**< Prefix of where to find sounds. */
#define SOUND_SUFFIX_WAV   ".wav" /**< Suffix of sounds. */
#define SOUND_SUFFIX_OGG   ".ogg" /**< Suffix of sounds. */


#define voiceLock()        SDL_LockMutex(voice_mutex)
#define voiceUnlock()      SDL_UnlockMutex(voice_mutex)


/*
 * Global sound properties.
 */
int sound_disabled            = 0; /**< Whether sound is disabled. */
static int sound_initialized  = 0; /**< Whether or not sound is initialized. */


/*
 * Sound list.
 */
static alSound *sound_list    = NULL; /**< List of available sounds. */
static int sound_nlist        = 0; /**< Number of available sounds. */


/*
 * Voices.
 */
static int voice_genid        = 0; /**< Voice identifier generator. */
alVoice *voice_active         = NULL; /**< Active voices. */
static alVoice *voice_pool    = NULL; /**< Pool of free voices. */
static SDL_mutex *voice_mutex = NULL; /**< Lock for voices. */


/*
 * Internally used sounds.
 */
static int snd_compression    = -1; /**< Compression sound. */
static int snd_compressionG   = -1; /**< Compression sound group. */
static double snd_compression_gain = 0.; /**< Current compression gain. */



/*
 * Function pointers for backends.
 */
/* Creation. */
int  (*sound_sys_init) (void)          = NULL;
void (*sound_sys_exit) (void)          = NULL;
 /* Sound creation. */
int  (*sound_sys_load) ( alSound *snd, const char *filename ) = NULL;
void (*sound_sys_free) ( alSound *snd ) = NULL;
 /* Sound settings. */
int  (*sound_sys_volume) ( const double vol ) = NULL;
double (*sound_sys_getVolume) (void)   = NULL;
double (*sound_sys_getVolumeLog) (void)   = NULL;
 /* Sound playing. */
int  (*sound_sys_play) ( alVoice *v, alSound *s )   = NULL;
int  (*sound_sys_playPos) ( alVoice *v, alSound *s,
      double px, double py, double vx, double vy ) = NULL;
int  (*sound_sys_updatePos) ( alVoice *v, double px, double py,
      double vx, double vy )           = NULL;
void (*sound_sys_updateVoice) ( alVoice *v ) = NULL;
 /* Sound management. */
void (*sound_sys_update) (void)        = NULL;
void (*sound_sys_stop) ( alVoice *v )  = NULL;
void (*sound_sys_pause) (void)         = NULL;
void (*sound_sys_resume) (void)        = NULL;
void (*sound_sys_setSpeed) ( double s ) = NULL;
void (*sound_sys_setSpeedVolume) ( double vol ) = NULL;
/* Listener. */
int (*sound_sys_updateListener) ( double dir, double px, double py,
      double vx, double vy )           = NULL;
/* Groups. */
int  (*sound_sys_createGroup) ( int size ) = NULL;
int  (*sound_sys_playGroup) ( int group, alSound *s, int once ) = NULL;
void (*sound_sys_stopGroup) ( int group ) = NULL;
void (*sound_sys_pauseGroup) ( int group ) = NULL;
void (*sound_sys_resumeGroup) ( int group ) = NULL;
void (*sound_sys_speedGroup) ( int group, int enable ) = NULL;
void (*sound_sys_volumeGroup) ( int group, double volume ) = NULL;
/* Env. */
int  (*sound_sys_env) ( SoundEnv_t env, double param ) = NULL;


/*
 * prototypes
 */
/* General. */
static int sound_makeList (void);
static int sound_load( alSound *snd, const char *filename );
static void sound_free( alSound *snd );
/* Voices. */


/**
 * @brief Initializes the sound subsystem.
 *
 *    @return 0 on success.
 */
int sound_init (void)
{
   int ret;

   /* See if sound is disabled. */
   if (conf.nosound) {
      sound_disabled = 1;
      music_disabled = 1;
   }

   /* Parse conf. */
   if (sound_disabled && music_disabled)
      return 0;

   /* Choose sound system. */
   if ((sound_sys_init == NULL) && (conf.sound_backend != NULL) &&
         (strcmp(conf.sound_backend,"openal")==0)) {
#if USE_OPENAL
      /*
       * OpenAL Sound.
       */
      /* Creation. */
      sound_sys_init       = sound_al_init;
      sound_sys_exit       = sound_al_exit;
      /* Sound Creation. */
      sound_sys_load       = sound_al_load;
      sound_sys_free       = sound_al_free;
      /* Sound settings. */
      sound_sys_volume     = sound_al_volume;
      sound_sys_getVolume  = sound_al_getVolume;
      sound_sys_getVolumeLog = sound_al_getVolumeLog;
      /* Sound playing. */
      sound_sys_play       = sound_al_play;
      sound_sys_playPos    = sound_al_playPos;
      sound_sys_updatePos  = sound_al_updatePos;
      sound_sys_updateVoice = sound_al_updateVoice;
      /* Sound management. */
      sound_sys_update     = sound_al_update;
      sound_sys_stop       = sound_al_stop;
      sound_sys_pause      = sound_al_pause;
      sound_sys_resume     = sound_al_resume;
      sound_sys_setSpeed   = sound_al_setSpeed;
      sound_sys_setSpeedVolume = sound_al_setSpeedVolume;
      /* Listener. */
      sound_sys_updateListener = sound_al_updateListener;
      /* Groups. */
      sound_sys_createGroup = sound_al_createGroup;
      sound_sys_playGroup  = sound_al_playGroup;
      sound_sys_stopGroup  = sound_al_stopGroup;
      sound_sys_pauseGroup = sound_al_pauseGroup;
      sound_sys_resumeGroup = sound_al_resumeGroup;
      sound_sys_speedGroup = sound_al_speedGroup;
      sound_sys_volumeGroup = sound_al_volumeGroup;
      /* Env. */
      sound_sys_env        = sound_al_env;
#else /* USE_OPENAL */
      WARN("OpenAL support not compiled in!");
#endif /* USE_OPENAL */
   }
   if ((sound_sys_init == NULL) && (conf.sound_backend != NULL) &&
         (strcmp(conf.sound_backend,"sdlmix")==0)) {
#if USE_SDLMIX
      /*
       * SDL_mixer Sound.
       */
      /* Creation. */
      sound_sys_init       = sound_mix_init;
      sound_sys_exit       = sound_mix_exit;
      /* Sound Creation. */
      sound_sys_load       = sound_mix_load;
      sound_sys_free       = sound_mix_free;
      /* Sound settings. */
      sound_sys_volume     = sound_mix_volume;
      sound_sys_getVolume  = sound_mix_getVolume;
      sound_sys_getVolumeLog = sound_mix_getVolume;
      /* Sound playing. */
      sound_sys_play       = sound_mix_play;
      sound_sys_playPos    = sound_mix_playPos;
      sound_sys_updatePos  = sound_mix_updatePos;
      sound_sys_updateVoice = sound_mix_updateVoice;
      /* Sound management. */
      sound_sys_update     = sound_mix_update;
      sound_sys_stop       = sound_mix_stop;
      sound_sys_pause      = sound_mix_pause;
      sound_sys_resume     = sound_mix_resume;
      sound_sys_setSpeed   = sound_mix_setSpeed;
      sound_sys_setSpeedVolume = sound_mix_setSpeedVolume;
      /* Listener. */
      sound_sys_updateListener = sound_mix_updateListener;
      /* Groups. */
      sound_sys_createGroup = sound_mix_createGroup;
      sound_sys_playGroup  = sound_mix_playGroup;
      sound_sys_stopGroup  = sound_mix_stopGroup;
      sound_sys_pauseGroup = sound_mix_pauseGroup;
      sound_sys_resumeGroup = sound_mix_resumeGroup;
      sound_sys_speedGroup = sound_mix_speedGroup;
      sound_sys_volumeGroup = sound_mix_volumeGroup;
      /* Env. */
      sound_sys_env        = sound_mix_env;
#else /* USE_SDLMIX */
      WARN("SDL_mixer support not compiled in!");
#endif /* USE_SDLMIX */
   }
   if (sound_sys_init == NULL) {
      WARN("Unknown/Unavailable sound backend '%s'.", conf.sound_backend);
      sound_disabled = 1;
      WARN("Sound disabled.");
      music_disabled = 1;
      return 0;
   }

   /* Initialize sound backend. */
   ret = sound_sys_init();
   if (ret != 0) {
      sound_disabled = 1;
      music_disabled = 1;
      WARN("Sound disabled.");
      return ret;
   }

   /* Create voice lock. */
   voice_mutex = SDL_CreateMutex();
   if (voice_mutex == NULL)
      WARN("Unable to create voice mutex.");

   /* Load available sounds. */
   ret = sound_makeList();
   if (ret != 0)
      return ret;

   /* Initialize music. */
   ret = music_init();
   if (ret != 0) {
      music_disabled = 1;
      WARN("Music disabled.");
   }

   /* Set volume. */
   if ((conf.sound > 1.) || (conf.sound < 0.))
      WARN("Sound has invalid value, clamping to [0:1].");
   sound_volume(conf.sound);

   /* Initialized. */
   sound_initialized = 1;

   /* Load compression noise. */
   snd_compression = sound_get( "compression" );
   if (snd_compression >= 0) {
      snd_compressionG = sound_createGroup( 1 );
      sound_speedGroup( snd_compressionG, 0 );
   }


   return 0;
}


/**
 * @brief Cleans up after the sound subsytem.
 */
void sound_exit (void)
{
   int i;
   alVoice *v;

   /* Nothing to disable. */
   if (sound_disabled || !sound_initialized)
      return;

   /* Exit music subsystem. */
   music_exit();

   if (voice_mutex != NULL) {
      voiceLock();
      /* free the voices. */
      while (voice_active != NULL) {
         v = voice_active;
         voice_active = v->next;
         free(v);
      }
      while (voice_pool != NULL) {
         v = voice_pool;
         voice_pool = v->next;
         free(v);
      }
      voiceUnlock();

      /* Destroy voice lock. */
      SDL_DestroyMutex(voice_mutex);
      voice_mutex = NULL;
   }

   /* free the sounds */
   for (i=0; i<sound_nlist; i++)
      sound_free( &sound_list[i] );
   free( sound_list );
   sound_list = NULL;
   sound_nlist = 0;

   /* Exit sound subsystem. */
   sound_sys_exit();

   /* Sound is done. */
   sound_initialized = 0;
}


/**
 * @brief Gets the buffer to sound of name.
 *
 *    @param name Name of the sound to get the id of.
 *    @return ID of the sound matching name.
 */
int sound_get( char* name )
{
   int i;

   if (sound_disabled)
      return 0;

   for (i=0; i<sound_nlist; i++)
      if (strcmp(name, sound_list[i].name)==0)
         return i;

   WARN("Sound '%s' not found in sound list", name);
   return -1;
}


/**
 * @brief Gets the length of the sound buffer.
 *
 *    @param id ID of the buffer to get the length of..
 *    @return The length of the buffer.
 */
double sound_length( int sound )
{
   if (sound_disabled)
      return 0.;

   return sound_list[sound].length;
}


/**
 * @brief Plays the sound in the first available channel.
 *
 *    @param sound Sound to play.
 *    @return Voice identifier on success.
 */
int sound_play( int sound )
{
   alVoice *v;
   alSound *s;

   if (sound_disabled)
      return 0;

   if ((sound < 0) || (sound >= sound_nlist))
      return -1;

   /* Gets a new voice. */
   v = voice_new();

   /* Get the sound. */
   s = &sound_list[sound];

   /* Try to play the sound. */
   if (sound_sys_play( v, s ))
      return -1;

   /* Set state and add to list. */
   v->state = VOICE_PLAYING;
   v->id = ++voice_genid;
   voice_add(v);

   return v->id;
}


/**
 * @brief Plays a sound based on position.
 *
 *    @param sound Sound to play.
 *    @param px X position of the sound.
 *    @param py Y position of the sound.
 *    @param vx X velocity of the sound.
 *    @param vy Y velocity of the sound.
 *    @return Voice identifier on success.
 */
int sound_playPos( int sound, double px, double py, double vx, double vy )
{
   alVoice *v;
   alSound *s;
   Pilot *p;
   double cx, cy, dist;
   int target;

   if (sound_disabled)
      return 0;

   if ((sound < 0) || (sound >= sound_nlist))
      return -1;

   target = cam_getTarget();

   /* Following a pilot. */
   p = pilot_get(target);
   if (target && (p != NULL)) {
      if (!pilot_inRange( p, px, py ))
         return 0;
   }
   /* Set to a position. */
   else {
      cam_getPos(&cx, &cy);
      dist = pow2(px - cx) + pow2(py - cy);
      if (dist > pilot_sensorRange())
         return 0;
   }

   /* Gets a new voice. */
   v = voice_new();

   /* Get the sound. */
   s = &sound_list[sound];

   /* Try to play the sound. */
   if (sound_sys_playPos( v, s, px, py, vx, vy ))
      return -1;

   /* Actually add the voice to the list. */
   v->state = VOICE_PLAYING;
   v->id = ++voice_genid;
   voice_add(v);

   return v->id;
}


/**
 * @brief Updates the position of a voice.
 *
 *    @param voice Identifier of the voice to update.
 *    @param x New x position to update to.
 *    @param y New y position to update to.
 */
int sound_updatePos( int voice, double px, double py, double vx, double vy )
{
   alVoice *v;

   if (sound_disabled)
      return 0;

   v = voice_get(voice);
   if (v != NULL) {

      /* Update the voice. */
      if (sound_sys_updatePos( v, px, py, vx, vy))
         return -1;
   }

   return 0;
}


/**
 * @brief Updates the sounds removing obsolete ones and such.
 *
 *    @return 0 on success.
 */
int sound_update( double dt )
{
   alVoice *v, *tv;

   /* Update music if needed. */
   music_update(dt);

   if (sound_disabled)
      return 0;

   /* System update. */
   sound_sys_update();

   if (voice_active == NULL)
      return 0;

   voiceLock();

   /* The actual control loop. */
   for (v=voice_active; v!=NULL; v=v->next) {

      /* Run first to clear in same iteration. */
      sound_sys_updateVoice( v );

      /* Destroy and toss into pool. */
      if ((v->state == VOICE_STOPPED) || (v->state == VOICE_DESTROY)) {

         /* Remove from active list. */
         tv = v->prev;
         if (tv == NULL) {
            voice_active = v->next;
            if (voice_active != NULL)
               voice_active->prev = NULL;
         }
         else {
            tv->next = v->next;
            if (tv->next != NULL)
               tv->next->prev = tv;
         }

         /* Add to free pool. */
         v->next = voice_pool;
         v->prev = NULL;
         voice_pool = v;
         if (v->next != NULL)
            v->next->prev = v;

         /* Avoid loop blockage. */
         v = (tv != NULL) ? tv->next : voice_active;
         if (v == NULL)
            break;
      }
   }

   voiceUnlock();

   return 0;
}


/**
 * @brief Pauses all the sounds.
 */
void sound_pause (void)
{
   if (sound_disabled)
      return;

   sound_sys_pause();

   if (snd_compression >= 0)
      sound_sys_pauseGroup( snd_compressionG );
}


/**
 * @brief Resumes all the sounds.
 */
void sound_resume (void)
{
   if (sound_disabled)
      return;

   sound_sys_resume();

   if (snd_compression >= 0)
      sound_sys_resumeGroup( snd_compressionG );
}


/**
 * @brief Stops all the playing voices.
 */
void sound_stopAll (void)
{
   alVoice *v;

   if (sound_disabled)
      return;

   /* Make sure there are voices. */
   if (voice_active==NULL)
      return;

   voiceLock();
   for (v=voice_active; v!=NULL; v=v->next) {
      sound_sys_stop( v );
      v->state = VOICE_STOPPED;
   }
   voiceUnlock();
}


/**
 * @brief Stops a voice from playing.
 *
 *    @param voice Identifier of the voice to stop.
 */
void sound_stop( int voice )
{
   alVoice *v;

   if (sound_disabled)
      return;

   v = voice_get(voice);
   if (v != NULL) {
      sound_sys_stop( v );
      v->state = VOICE_STOPPED;
   }

}


/**
 * @brief Updates the sound listener.
 *
 *    @param dir Direction listener is facing.
 *    @param px X position of the listener.
 *    @param py Y position of the listener.
 *    @param vx X velocity of the listener.
 *    @param vy Y velocity of the listener.
 *    @return 0 on success.
 *
 * @sa sound_playPos
 */
int sound_updateListener( double dir, double px, double py,
      double vx, double vy )
{
   if (sound_disabled)
      return 0;

   return sound_sys_updateListener( dir, px, py, vx, vy );
}


/**
 * @brief Sets the speed to play the sound at.
 *
 *    @param s Speed to play sound at.
 */
void sound_setSpeed( double s )
{
   double v;
   int playing;

   if (sound_disabled)
      return;

   /* We implement the brown noise here. */
   playing = (snd_compression_gain > 0.);
   if (player.tc_max > 2.)
      v = CLAMP( 0, 1., MAX( (s-2)/10., (s-2) / (player.tc_max-2) ) );
   else
      v = CLAMP( 0, 1., (s-2)/10. );

   if (v > 0.) {
      if (snd_compression >= 0) {
         if (!playing)
            sound_playGroup( snd_compressionG, snd_compression, 0 ); /* Start playing only if it's not playing. */
         sound_volumeGroup( snd_compressionG, v );
      }
      sound_sys_setSpeedVolume( 1.-v );
   }
   else if (playing) {
      if (snd_compression >= 0)
         sound_stopGroup( snd_compressionG ); /* Stop compression sound. */
      sound_sys_setSpeedVolume( 1. ); /* Restore volume. */
   }
   snd_compression_gain = v;

   return sound_sys_setSpeed( s );
}


/**
 * @brief Makes the list of available sounds.
 */
static int sound_makeList (void)
{
   char** files;
   uint32_t nfiles,i;
   char path[PATH_MAX];
   char tmp[64];
   int len, suflen, flen;
   int mem;

   if (sound_disabled)
      return 0;

   /* get the file list */
   files = ndata_list( SOUND_PREFIX, &nfiles );

   /* load the profiles */
   mem = 0;
   suflen = strlen(SOUND_SUFFIX_WAV);
   for (i=0; i<nfiles; i++) {
      flen = strlen(files[i]);

      /* Must be longer than suffix. */
      if (flen < suflen) {
         free(files[i]);
         continue;
      }

      /* Make sure is wav or ogg. */
      if ((strncmp( &files[i][flen - suflen], SOUND_SUFFIX_WAV, suflen)!=0) &&
            (strncmp( &files[i][flen - suflen], SOUND_SUFFIX_OGG, suflen)!=0)) {
         free(files[i]);
         continue;
      }

      /* grow the selection size */
      sound_nlist++;
      if (sound_nlist > mem) { /* we must grow */
         mem += 32; /* we'll overallocate most likely */
         sound_list = realloc( sound_list, mem*sizeof(alSound));
      }

      /* remove the suffix */
      len = flen - suflen;
      strncpy( tmp, files[i], len );
      tmp[len] = '\0';

      /* Load the sound. */
      sound_list[sound_nlist-1].name = strdup(tmp);
      nsnprintf( path, PATH_MAX, SOUND_PREFIX"%s", files[i] );
      if (sound_load( &sound_list[sound_nlist-1], path ))
         sound_nlist--; /* Song not actually added. */

      /* Clean up. */
      free(files[i]);
   }
   /* shrink to minimum ram usage */
   sound_list = realloc( sound_list, sound_nlist*sizeof(alSound));

   DEBUG("Loaded %d sound%s", sound_nlist, (sound_nlist==1)?"":"s");

   /* More clean up. */
   free(files);

   return 0;
}


/**
 * @brief Sets the volume.
 *
 *    @param vol Volume to set to.
 *    @return 0 on success.
 */
int sound_volume( const double vol )
{
   if (sound_disabled)
      return 0;

   return sound_sys_volume( vol );
}


/**
 * @brief Gets the current sound volume (linear).
 *
 *    @return The current sound volume level.
 */
double sound_getVolume (void)
{
   if (sound_disabled)
      return 0.;

   return sound_sys_getVolume();
}


/**
 * @brief Gets the current sound volume (logarithmic).
 *
 *    @return The current sound volume level.
 */
double sound_getVolumeLog(void)
{
   if (sound_disabled)
      return 0.;

   return sound_sys_getVolumeLog();
}


/**
 * @brief Loads a sound into the sound_list.
 *
 *    @param snd Sound to load into.
 *    @param filename Name fo the file to load.
 *    @return 0 on success.
 *
 * @sa sound_makeList
 */
static int sound_load( alSound *snd, const char *filename )
{
   if (sound_disabled)
      return -1;

   return sound_sys_load( snd, filename );
}


/**
 * @brief Frees the sound.
 *
 *    @param snd Sound to free.
 */
static void sound_free( alSound *snd )
{
   /* Free general stuff. */
   if (snd->name) {
      free(snd->name);
      snd->name = NULL;
   }

   /* Free internals. */
   sound_sys_free(snd);
}


/**
 * @brief Creates a sound group.
 *
 *    @param start Where to start creating the group.
 *    @param size Size of the group.
 *    @return ID of the group created on success, 0 on error.
 */
int sound_createGroup( int size )
{
   if (sound_disabled)
      return 0;

   return sound_sys_createGroup( size );
}


/**
 * @brief Plays a sound in a group.
 *
 *    @param group Group to play sound in.
 *    @param sound Sound to play.
 *    @param once Whether to play only once.
 *    @return 0 on success.
 */
int sound_playGroup( int group, int sound, int once )
{
   if (sound_disabled)
      return 0;

   if ((sound < 0) || (sound >= sound_nlist))
      return -1;

   return sound_sys_playGroup( group, &sound_list[sound], once );
}


/**
 * @brief Stops all the sounds in a group.
 *
 *    @param group Group to stop all its sounds.
 */
void sound_stopGroup( int group )
{
   if (sound_disabled)
      return;

   sound_sys_stopGroup( group );
}


/**
 * @brief Pauses all the sounds in a group.
 *
 *    @param Group to pause sounds.
 */
void sound_pauseGroup( int group )
{
   if (sound_disabled)
      return;

   sound_sys_pauseGroup( group );
}


/**
 * @brief Resumes all the sounds in a group.
 *
 *    @param Group to resume sounds.
 */
void sound_resumeGroup( int group )
{
   if (sound_disabled)
      return;

   sound_sys_resumeGroup( group );
}


/**
 * @brief Sets whether or not the speed affects a group.
 *
 *    @param group Group to set if speed affects it.
 *    @param enable Whether or not speed affects the group.
 */
void sound_speedGroup( int group, int enable )
{
   if (sound_disabled)
      return;

   sound_sys_speedGroup( group, enable );
}


/**
 * @brief Sets the volume of a group.
 *
 *    @param group Group to set the volume of.
 *    @param volume Volume to set to in the [0-1] range.
 */
void sound_volumeGroup( int group, double volume )
{
   if (sound_disabled)
      return;

   sound_sys_volumeGroup( group, volume );
}


/**
 * @brief Sets up the sound environment.
 *
 *    @param env Type of environment to set up.
 *    @param param Environment parameter.
 *    @return 0 on success.
 */
int sound_env( SoundEnv_t env, double param )
{
   if (sound_disabled)
      return 0;

   return sound_sys_env( env, param );
}


/**
 * @brief Locks the voices.
 */
void voice_lock (void)
{
   voiceLock();
}


/**
 * @brief Unlocks the voices.
 */
void voice_unlock (void)
{
   voiceUnlock();
}


/**
 * @brief Gets a new voice ready to be used.
 *
 *    @return New voice ready to use.
 */
alVoice* voice_new (void)
{
   alVoice *v;

   /* No free voices, allocate a new one. */
   if (voice_pool == NULL) {
      v = calloc( 1, sizeof(alVoice) );
      voice_pool = v;
      return v;
   }

   /* First free voice. */
   v = voice_pool; /* We do not touch the next nor prev, it's still in the pool. */
   return v;
}


/**
 * @brief Adds a voice to the active voice stack.
 *
 *    @param v Voice to add to the active voice stack.
 *    @return 0 on success.
 */
int voice_add( alVoice* v )
{
   alVoice *tv;

   /* Remove from pool. */
   if (v->prev != NULL) {
      tv = v->prev;
      tv->next = v->next;
      if (tv->next != NULL)
         tv->next->prev = tv;
   }
   else { /* Set pool to be the next. */
      voice_pool = v->next;
      if (voice_pool != NULL)
         voice_pool->prev = NULL;
   }

   /* Insert to the front of active voices. */
   voiceLock();
   tv = voice_active;
   v->next = tv;
   v->prev = NULL;
   voice_active = v;
   if (tv != NULL)
      tv->prev = v;
   voiceUnlock();
   return 0;
}


/**
 * @brief Gets a voice by identifier.
 *
 *    @param id Identifier to look for.
 *    @return Voice matching identifier or NULL if not found.
 */
alVoice* voice_get( int id )
{
   alVoice *v;

   /* Make sure there are voices. */
   if (voice_active==NULL)
      return NULL;

   voiceLock();
   for (v=voice_active; v!=NULL; v=v->next)
      if (v->id == id)
         break;
   voiceUnlock();

   return v;
}

