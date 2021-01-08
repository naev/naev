/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file sound.c
 *
 * @brief Handles all the sound details.
 */


/** @cond */
#include <sys/stat.h>
#include "physfs.h"
#include "SDL.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"

#include "naev.h"
/** @endcond */

#include "sound.h"

#include "array.h"
#include "camera.h"
#include "conf.h"
#include "log.h"
#include "music.h"
#include "ndata.h"
#include "nstring.h"
#include "physics.h"
#include "player.h"
#include "sound_openal.h"


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

   /* Initialize sound backend. */
   ret = sound_al_init();
   if (ret != 0) {
      sound_disabled = 1;
      music_disabled = 1;
      WARN(_("Sound disabled."));
      return ret;
   }

   /* Create voice lock. */
   voice_mutex = SDL_CreateMutex();
   if (voice_mutex == NULL)
      WARN(_("Unable to create voice mutex."));

   /* Load available sounds. */
   ret = sound_makeList();
   if (ret != 0)
      return ret;

   /* Initialize music. */
   ret = music_init();
   if (ret != 0) {
      music_disabled = 1;
      WARN(_("Music disabled."));
   }

   /* Set volume. */
   if ((conf.sound > 1.) || (conf.sound < 0.))
      WARN(_("Sound has invalid value, clamping to [0:1]."));
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
   for (i=0; i<array_size(sound_list); i++)
      sound_free( &sound_list[i] );
   array_free( sound_list );

   /* Exit sound subsystem. */
   sound_al_exit();

   /* Sound is done. */
   sound_initialized = 0;
}


/**
 * @brief Gets the buffer to sound of name.
 *
 *    @param name Name of the sound to get the id of.
 *    @return ID of the sound matching name.
 */
int sound_get( const char* name )
{
   int i;

   if (sound_disabled)
      return 0;

   for (i=0; i<array_size(sound_list); i++)
      if (strcmp(name, sound_list[i].name)==0)
         return i;

   WARN(_("Sound '%s' not found in sound list"), name);
   return -1;
}


/**
 * @brief Gets the length of the sound buffer.
 *
 *    @param sound ID of the buffer to get the length of..
 *    @return The length of the buffer.
 */
double sound_getLength( int sound )
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

   if ((sound < 0) || (sound >= array_size(sound_list)))
      return -1;

   /* Gets a new voice. */
   v = voice_new();

   /* Get the sound. */
   s = &sound_list[sound];

   /* Try to play the sound. */
   if (sound_al_play( v, s ))
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

   if ((sound < 0) || (sound >= array_size(sound_list)))
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
   if (sound_al_playPos( v, s, px, py, vx, vy ))
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
 *    @param px New x position to update to.
 *    @param py New y position to update to.
 *    @param vx New x velocity of the sound.
 *    @param vy New y velocity of the sound.
 */
int sound_updatePos( int voice, double px, double py, double vx, double vy )
{
   alVoice *v;

   if (sound_disabled)
      return 0;

   v = voice_get(voice);
   if (v != NULL) {

      /* Update the voice. */
      if (sound_al_updatePos( v, px, py, vx, vy))
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
   sound_al_update();

   if (voice_active == NULL)
      return 0;

   voiceLock();

   /* The actual control loop. */
   for (v=voice_active; v!=NULL; v=v->next) {

      /* Run first to clear in same iteration. */
      sound_al_updateVoice( v );

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

   sound_al_pause();

   if (snd_compression >= 0)
      sound_al_pauseGroup( snd_compressionG );
}


/**
 * @brief Resumes all the sounds.
 */
void sound_resume (void)
{
   if (sound_disabled)
      return;

   sound_al_resume();

   if (snd_compression >= 0)
      sound_al_resumeGroup( snd_compressionG );
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
      sound_al_stop( v );
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
      sound_al_stop( v );
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

   return sound_al_updateListener( dir, px, py, vx, vy );
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
      sound_al_setSpeedVolume( 1.-v );
   }
   else if (playing) {
      if (snd_compression >= 0)
         sound_stopGroup( snd_compressionG ); /* Stop compression sound. */
      sound_al_setSpeedVolume( 1. ); /* Restore volume. */
   }
   snd_compression_gain = v;

   return sound_al_setSpeed( s );
}


/**
 * @brief Makes the list of available sounds.
 */
static int sound_makeList (void)
{
   char** files;
   size_t i;
   char path[PATH_MAX];
   char tmp[64];
   int len, suflen, flen;
   alSound *snd;

   if (sound_disabled)
      return 0;

   /* get the file list */
   files = PHYSFS_enumerateFiles( SOUND_PATH );

   /* Create the list. */
   sound_list = array_create( alSound );

   /* load the profiles */
   suflen = strlen(SOUND_SUFFIX_WAV);
   for (i=0; files[i]!=NULL; i++) {
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

      /* remove the suffix */
      len = flen - suflen;
      strncpy( tmp, files[i], len );
      tmp[len] = '\0';

      /* Load the sound. */
      snd = &array_grow( &sound_list );
      snd->name = strdup(tmp);
      nsnprintf( path, PATH_MAX, SOUND_PATH"%s", files[i] );
      snd->filename = strdup( path );
      if (sound_load( snd, path ))
         array_erase( &sound_list, snd, snd+1 ); /* Song not actually added. */
   }

   DEBUG( n_("Loaded %d Sound", "Loaded %d Sounds", array_size(sound_list)), array_size(sound_list) );

   /* Clean up. */
   PHYSFS_freeList( files );

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

   return sound_al_volume( vol );
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

   return sound_al_getVolume();
}


/**
 * @brief Gets the current sound volume (logarithmic).
 *
 *    @return The current sound volume level.
 */
double sound_getVolumeLog (void)
{
   if (sound_disabled)
      return 0.;

   return sound_al_getVolumeLog();
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

   return sound_al_load( snd, filename );
}


/**
 * @brief Frees the sound.
 *
 *    @param snd Sound to free.
 */
static void sound_free( alSound *snd )
{
   /* Free general stuff. */
   free(snd->name);
   free(snd->filename);

   /* Free internals. */
   sound_al_free(snd);
}


/**
 * @brief Creates a sound group.
 *
 *    @param size Size of the group.
 *    @return ID of the group created on success, 0 on error.
 */
int sound_createGroup( int size )
{
   if (sound_disabled)
      return 0;

   return sound_al_createGroup( size );
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

   if ((sound < 0) || (sound >= array_size(sound_list)))
      return -1;

   return sound_al_playGroup( group, &sound_list[sound], once );
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

   sound_al_stopGroup( group );
}


/**
 * @brief Pauses all the sounds in a group.
 *
 *    @param group Group to pause sounds.
 */
void sound_pauseGroup( int group )
{
   if (sound_disabled)
      return;

   sound_al_pauseGroup( group );
}


/**
 * @brief Resumes all the sounds in a group.
 *
 *    @param group Group to resume sounds.
 */
void sound_resumeGroup( int group )
{
   if (sound_disabled)
      return;

   sound_al_resumeGroup( group );
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

   sound_al_speedGroup( group, enable );
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

   sound_al_volumeGroup( group, volume );
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

   return sound_al_env( env, param );
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

