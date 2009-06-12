/*
 * See Licensing and Copyright notice in naev.h
 */

#if USE_OPENAL


#include "sound_openal.h"

#include <math.h>
#include <sys/stat.h>

#include <AL/alc.h>
#include <AL/alut.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "sound.h"
#include "ndata.h"
#include "naev.h"
#include "log.h"
#include "pack.h"


/*
 * this file controls all the routines for using a virtual voice
 * wrapper system around the openal library to get 3d sound.
 *
 * currently it is only using positional sound and no doppler effect
 */

/*
 * SOUND OVERVIEW
 *
 *
 * We use a priority virtual voice system with pre-allocated buffers.
 *
 * Naming:
 *    buffer - sound sample
 *    source - openal object that plays sound
 *    voice - virtual object that wants to play sound
 *
 *
 * First we allocate all the buffers based on what we find inside the
 * datafile.
 * Then we allocate all the possible sources (giving the music system
 * what it needs).
 * Now we allow the user to dynamically create voices, these voices will
 * always try to grab a source from the source pool.  If they can't they
 * will pretend to play the buffer.
 * Every so often we'll check to see if the important voices are being
 * played and take away the sources from the lesser ones.
 */


/* sound parameters - TODO make it variable per source */
#define SOUND_ROLLOFF_FACTOR  1.
#define SOUND_REFERENCE_DIST  500.
#define SOUND_MAX_DIST        1000.
#define SOUND_MAX_SOURCES     256


#define soundLock()     SDL_mutexP(sound_lock)
#define soundUnlock()   SDL_mutexV(sound_lock)


/*
 * global sound lock
 */
SDL_mutex *sound_lock = NULL;


/*
 * global device and contex
 */
static ALCcontext *al_context = NULL;
static ALCdevice *al_device   = NULL;


/*
 * struct to hold all the sources and currently attached voice
 */
static ALuint *source_stack   = NULL; /* and it's stack */
static int source_nstack      = 0;
static ALuint *source_active  = NULL;
static int source_nactive     = 0;
static int source_mstack      = 0;


/*
 * volume
 */
static ALfloat svolume        = 1.;


/*
 * Prototypes.
 */
static ALuint sound_al_getSource (void);


/**
 * @brief Initializes the sound subsystem.
 *
 *    @return 0 on success.
 */
int sound_al_init (void)
{
   int ret;
   ALenum err;
   const ALchar* dev;

   /* Default values. */
   ret = 0;

   /* we'll need a mutex */
   sound_lock = SDL_CreateMutex();
   soundLock();

   /* initialize alut - i think it's worth it */
   alutInitWithoutContext(NULL,NULL);

   /* Get the sound device. */
   dev = alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );

   /* opening the default device */
   al_device = alcOpenDevice(NULL);
   if (al_device == NULL) {
      WARN("Unable to open default sound device");
      ret = -1;
      goto snderr_dev;
   }

   /* create the OpenAL context */
   al_context = alcCreateContext( al_device, NULL );
   if (sound_lock == NULL) {
      WARN("Unable to create OpenAL context");
      ret = -2;
      goto snderr_ctx;
   }

   /* clear the errors */
   alGetError();

   /* set active context */
   if (alcMakeContextCurrent( al_context )==AL_FALSE) {
      WARN("Failure to set default context");
      ret = -4;
      goto snderr_act;
   }

   /* set the distance model */
   alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED );

   /* start allocating the sources - music has already taken his */
   alGetError(); /* another error clear */
   source_mstack = 0;
   while (((err=alGetError())==AL_NO_ERROR) && (source_nstack < SOUND_MAX_SOURCES)) {
      if (source_mstack < source_nstack+1) { /* allocate more memory */
         source_mstack += 32;
         source_stack = realloc( source_stack, sizeof(ALuint) * source_mstack );
      }
      alGenSources( 1, &source_stack[source_nstack] );
      source_nstack++;
   }
   /* Reduce ram usage. */
   source_mstack = source_nstack;
   source_stack  = realloc( source_stack, sizeof(ALuint) * source_mstack );
   source_active = malloc( sizeof(ALuint) * source_mstack );

   /* we can unlock now */
   soundUnlock();

   /* debug magic */
   DEBUG("OpenAL: %s", dev);
   DEBUG("Renderer: %s", alGetString(AL_RENDERER));
   DEBUG("Version: %s", alGetString(AL_VERSION));
   DEBUG();

   return 0;

   /*
    * error handling
    */
snderr_act:
   alcDestroyContext( al_context );
snderr_ctx:
   al_context = NULL;
   alcCloseDevice( al_device );
snderr_dev:
   al_device = NULL;
   soundUnlock();
   SDL_DestroyMutex( sound_lock );
   sound_lock = NULL;
   WARN("Error initializing sound: %d", alGetError());
   return ret;
}


/**
 * @brief Cleans up after the sound subsytem.
 */
void sound_al_exit (void)
{
   /* Clean up the sources. */
   if (source_stack) {
      alDeleteSources( source_nstack, source_stack );
      alDeleteSources( source_nactive, source_active );
   }

   /* Free stacks. */
   if (source_stack != NULL)
      free(source_stack);
   if (source_active != NULL)
      free(source_active);
   source_stack      = NULL;
   source_nstack     = 0;
   source_active     = NULL;
   source_nactive    = 0;
   source_mstack     = 0;

   /* Clean up context and such. */
   if (sound_lock) {
      soundLock();

      if (al_context) {
         alcMakeContextCurrent(NULL);
         alcDestroyContext( al_context );
      }
      if (al_device) alcCloseDevice( al_device );

      soundUnlock();
      SDL_DestroyMutex( sound_lock );
   }

   /* bye bye alut */
   alutExit();
}


/**
 * @brief Loads the sound.
 */
int sound_al_load( alSound *snd, const char *filename )
{
   void* wavdata;
   unsigned int size;
   ALenum err;

   /* get the file data buffer from packfile */
   wavdata = ndata_read( filename, &size );

   soundLock();

   /* bind to OpenAL buffer */
   snd->u.al.buf = alutCreateBufferFromFileImage( wavdata, size );
   if ((snd->u.al.buf) == AL_NONE) {
      WARN("OpenAL failed to load %s: %s", filename,
            alutGetErrorString(alutGetError()));
      free(wavdata);
      return -1;
   }
   /*alGenBuffers( 1, buffer );
   alBufferData( *buffer, AL_FORMAT_MONO16, wavdata, size, 22050 );*/

   /* errors? */
   if ((err = alGetError()) != AL_NO_ERROR) {
      WARN("OpenAL error '%d' loading sound '%s'.", err, filename );
      free(wavdata);
      return 0;
   }

   soundUnlock();

   /* finish */
   free(wavdata);
   return 0;
}


/**
 * @brief Frees the source.
 */
void sound_al_free( alSound *snd )
{
   soundLock();

   /* free the stuff */
   alDeleteBuffers( 1, &snd->u.al.buf );

   soundUnlock();
}


/**
 * @brief Sets all the sounds volume to vol
 */
int sound_al_volume( double vol )
{
   svolume = (ALfloat) vol;

   return 0;
}


/**
 * @brief Gets the current volume level.
 */
double sound_al_getVolume (void)
{
   return svolume;
}


/**
 * @brief Gets a free OpenAL source.
 */
static ALuint sound_al_getSource (void)
{
   ALuint source;

   /* Make sure there's enough. */
   if (source_nstack <= 0)
      return 0;

   /* Pull one off the stack. */
   source_nstack--;
   source = source_stack[source_nstack];

   /* Throw it on the active stack. */
   source_active[source_nactive] = source;
   source_nactive++;

   return source;
}


/**
 * @brief Plays a sound.
 *
 *    @param v Voice to play sound.
 *    @param s Sound to play.
 *    @return 0 on success.
 */
int sound_al_play( alVoice *v, alSound *s )
{
   soundLock();

   /* Set up the source and buffer. */
   v->u.al.source = sound_al_getSource();
   if (v->u.al.source == 0)
      return -1;
   v->u.al.buffer = s->u.al.buf;

   /* Do not do positional sound. */
   alSourcei( v->u.al.source, AL_SOURCE_RELATIVE, AL_FALSE );

   /* Start playing. */
   alSourcePlay( v->u.al.source );

   soundUnlock();

   return 0;
}


/**
 * @brief Plays a sound at a position.
 *
 *    @param v Voice to play sound.
 *    @param s Sound to play.
 *    @param px X position of the sound.
 *    @param py Y position of the sound.
 *    @param vx X velocity of the sound.
 *    @param vy Y velocity of the sound.
 *    @return 0 on success.
 */
int sound_al_playPos( alVoice *v, alSound *s,
            double px, double py, double vx, double vy )
{
   soundLock();

   /* Set up the source and buffer. */
   v->u.al.source = sound_al_getSource();
   if (v->u.al.source == 0)
      return -1;
   v->u.al.buffer = s->u.al.buf;

   /* Enable positional sound. */
   alSourcei( v->u.al.source, AL_SOURCE_RELATIVE, AL_TRUE );

   /* Distance model. */
   alSourcef( v->u.al.source, AL_ROLLOFF_FACTOR, SOUND_ROLLOFF_FACTOR );
   alSourcef( v->u.al.source, AL_MAX_DISTANCE, SOUND_MAX_DIST );
   alSourcef( v->u.al.source, AL_REFERENCE_DISTANCE, SOUND_REFERENCE_DIST );

   /* Update position. */
   v->u.al.pos[0] = px;
   v->u.al.pos[1] = py;
   v->u.al.vel[0] = vx;
   v->u.al.vel[1] = vy;

   /* Set up properties. */
   alSourcef( v->u.al.source, AL_GAIN, svolume );
   alSource3f( v->u.al.source, AL_POSITION,
         v->u.al.pos[0], v->u.al.pos[1], 0. );
   alSource3f( v->u.al.source, AL_VELOCITY,
         v->u.al.vel[0], v->u.al.vel[1], 0. );

   /* Start playing. */
   alSourcePlay( v->u.al.source );

   soundUnlock();

   return 0;
}


/**
 * @brief Updates the position of the sound.
 */
int sound_al_updatePos( alVoice *v,
            double px, double py, double vx, double vy )
{
   v->u.al.pos[0] = px;
   v->u.al.pos[1] = py;
   v->u.al.vel[0] = vx;
   v->u.al.vel[1] = vy;

   return 0;
}


/**
 * @brief Updates the voice.
 *
 *    @param v Voice to update.
 */
void sound_al_updateVoice( alVoice *v )
{
   ALint state;

   soundLock();

   /* Get status. */
   alGetSourcei( v->u.al.source, AL_SOURCE_STATE, &state );
   if (state == AL_STOPPED) {
      /* Put source back on the list. */
      source_stack[source_nstack] = v->u.al.source;
      source_nstack++;
      v->u.al.source = 0;

      /* Mark as stopped - erased next iteration. */
      v->state = VOICE_STOPPED;
      return;
   }

   /* Set up properties. */
   alSourcef( v->u.al.source, AL_GAIN, svolume );
   alSource3f( v->u.al.source, AL_POSITION,
         v->u.al.pos[0], v->u.al.pos[1], 0. );
   alSource3f( v->u.al.source, AL_VELOCITY,
         v->u.al.vel[0], v->u.al.vel[1], 0. );

   soundUnlock();
}


/**
 * @brief Stops playing sound.
 */
void sound_al_stop( alVoice* voice )
{
   soundLock();

   if (voice->u.al.source != 0)
      alSourceStop(voice->u.al.source);

   soundUnlock();
}


/**
 * @brief Pauses all sounds.
 */
void sound_al_pause (void)
{
}


/**
 * @brief Resumes all sounds.
 */
void sound_al_resume (void)
{
}


/**
 * @brief Updates the listener.
 */
int sound_al_updateListener( double dir, double px, double py,
      double vx, double vy )
{
   soundLock();

   /* set orientation */
   ALfloat ori[] = { cos(dir), sin(dir), 0.,  0., 0., 1. };
   alListenerfv( AL_ORIENTATION, ori );
   alListener3f( AL_POSITION, px, py, 1. );
   alListener3f( AL_VELOCITY, vx, vy, 0. );

   soundUnlock();

   return 0;
}


#endif /* USE_OPENAL */
