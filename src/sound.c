/*
 * See Licensing and Copyright notice in naev.h
 */



#include "sound.h"

#include <sys/stat.h>

#include <AL/alc.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "music.h"


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


#define SOUND_PREFIX "snd/sounds/"
#define SOUND_SUFFIX ".wav"


/*
 * gives the buffers a name
 */
typedef struct alSound_ {
   char *name; /* buffer's name */
   ALuint buffer; /* assosciated OpenAL buffer */
} alSound;


#define VOICE_PLAYING      (1<<0)   /* voice is playing */
#define VOICE_LOOPING      (1<<1)   /* voice is looping */
#define VOICE_DONE         (1<<2)   /* voice is done - must remove */
#define voice_set(v,f)     ((v)->flags |= f)
#define voice_is(v,f)      ((v)->flags & f)


/*
 * global sound lock
 */
SDL_mutex *sound_lock = NULL;


/*
 * global device and contex
 */
static ALCcontext *al_context = NULL;
static ALCdevice *al_device = NULL;

/*
 * music player thread to assure streaming is perfect
 */
static SDL_Thread *music_player = NULL;


/*
 * list of sounds available (all preloaded into a buffer)
 */
static alSound *sound_list = NULL;
static int nsound_list = 0;


/*
 * struct to hold all the sources and currently attached voice
 */
typedef struct VoiceSource_ {
   ALuint source; /* allocated source */
   ALuint voice; /* voice id */
} VoiceSource;
static VoiceSource *source_stack = NULL; /* and it's stack */
static int source_nstack = 0;


/*
 * virtual voice
 */
struct alVoice { /* declared in header */
   alVoice *next; /* yes it's a linked list */

   ALuint id; /* unique id for the voice */

   ALuint source; /* source itself, 0 if not set */
   ALuint buffer; /* buffer */

   int priority; /* base priority */

   double px, py; /* position */
   double vx, vy; /* velocity */

   unsigned int start; /* time started in ms */
   unsigned int flags; /* flags to set properties */
};
static alVoice *voice_start = NULL;
static alVoice *voice_end = NULL;


/*
 * volume
 */
static ALfloat svolume = 0.3;


/*
 * prototypes
 */
static int sound_makeList (void);
static int sound_load( ALuint *buffer, char *filename );
static void sound_free( alSound *snd );
static int voice_getSource( alVoice* voc );


/*
 * initializes the sound subsystem
 */
int sound_init (void)
{
   int mem, ret;
   ALenum err;

   ret = 0;

   /* we'll need a mutex */
   sound_lock = SDL_CreateMutex();
   SDL_mutexP( sound_lock );

   const ALchar* device = alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );

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
      ret =  -4;
      goto snderr_act;
   }

   /* set the master gain */
   alListenerf( AL_GAIN, 1. );

   /* set the distance model */
   alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED );

   /* we can unlock now */
   SDL_mutexV( sound_lock );

   /* start the music server */
   music_init();

#if 0
   /* start allocating the sources - music has already taken his */
   alGetError(); /* another error clear */
   mem = 0;
   while (((err=alGetError())==AL_NO_ERROR) && (source_nstack < 128)) {
      if (mem < source_nstack+1) { /* allocate more memory */
         mem += 32;
         source_stack = realloc( source_stack, sizeof(VoiceSource) * mem );
      }
      alGenSources( 1, &source_stack[source_nstack].source );
      source_stack[source_nstack].voice = 0;
      source_nstack++;
   }
   /* use minimal ram */
   source_stack = realloc( source_stack, sizeof(VoiceSource) * source_nstack );
#endif

   /* debug magic */                                 
   DEBUG("OpenAL: %s", device);                      
   DEBUG("Sources: %d", source_nstack );
   DEBUG("Renderer: %s", alGetString(AL_RENDERER));  
   DEBUG("Version: %s", alGetString(AL_VERSION));    
   DEBUG();

   /* load up all the sounds */
   sound_makeList();
   music_makeList(); /* and music */

   /* now start the music thread */
   music_player = SDL_CreateThread( music_thread, NULL );

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
   SDL_mutexV( sound_lock );
   SDL_DestroyMutex( sound_lock );
   sound_lock = NULL;
   ERR("Sound failed to initialize");
   return ret;
}
/*
 * cleans up after the sound subsytem
 */
void sound_exit (void)
{
   int i;
   /* free the sounds */
   for (i=0; i<nsound_list; i++)
      sound_free( &sound_list[i] );
   free( sound_list );
   sound_list = NULL;
   nsound_list = 0;

   /* must stop the music before killing it, then thread should commit suicide */
   if (music_player) {
      music_stop();
      music_kill();
      SDL_WaitThread( music_player, NULL );
      music_exit();
   }

   /* clean up context and such */
   if (sound_lock) {
      SDL_mutexP( sound_lock );

      if (al_context) {
         alcMakeContextCurrent(NULL);
         alcDestroyContext( al_context );
      }
      if (al_device) alcCloseDevice( al_device );

      SDL_mutexV( sound_lock );
      SDL_DestroyMutex( sound_lock );
   }
}


/*
 * gets the buffer to sound of name
 */
ALuint sound_get( char* name ) 
{
   if (sound_lock == NULL) return 0;

   int i;
   for (i=0; i<nsound_list; i++)
      if (strcmp(name, sound_list[i].name)==0)
         return sound_list[i].buffer;
   WARN("Sound '%s' not found in sound list", name);
   return 0;
}


/*
 * makes the list of available sounds
 */
static int sound_makeList (void)
{
   if (sound_lock == NULL) return 0;

   char** files;
   uint32_t nfiles,i;
   char tmp[64];
   int len;
   int mem;

   /* get the file list */
   files = pack_listfiles( data, &nfiles );

   /* load the profiles */
   mem = 0;
   for (i=0; i<nfiles; i++)
      if ((strncmp( files[i], SOUND_PREFIX, strlen(SOUND_PREFIX))==0) &&
            (strncmp( files[i] + strlen(files[i]) - strlen(SOUND_SUFFIX),
                      SOUND_SUFFIX, strlen(SOUND_SUFFIX))==0)) {

         /* grow the selection size */
         nsound_list++;
         if (nsound_list > mem) { /* we must grow */
            mem += 32; /* we'll overallocate most likely */
            sound_list = realloc( sound_list, mem*sizeof(alSound));
         }

         /* remove the prefix and suffix */
         len = strlen(files[i]) - strlen(SOUND_SUFFIX SOUND_PREFIX);
         strncpy( tmp, files[i] + strlen(SOUND_PREFIX), len );
         tmp[len] = '\0';

         /* give it the new name */
         sound_list[nsound_list-1].name = strdup(tmp);
         sound_load( &sound_list[nsound_list-1].buffer, files[i] );
      }
   /* shrink to minimum ram usage */
   sound_list = realloc( sound_list, nsound_list*sizeof(alSound));

   /* free the char* allocated by pack */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   DEBUG("Loaded %d sound%s", nsound_list, (nsound_list==1)?"":"s");

   return 0;
}


/*
 * loads a sound into the sound_list
 */
static int sound_load( ALuint *buffer, char *filename )
{
   if (sound_lock == NULL) return 0;

   void* wavdata;
   unsigned int size;
   ALenum err;

   /* get the file data buffer from packfile */
   wavdata = pack_readfile( DATA, filename, &size );

   SDL_mutexP( sound_lock );

   /* bind to OpenAL buffer */
   alGenBuffers( 1, buffer );
   alBufferData( *buffer, AL_FORMAT_MONO16, wavdata, size, 22050 );

   /* errors? */
   if ((err = alGetError()) != AL_NO_ERROR) {
      WARN("OpenAL error '%d' loading sound '%s'.", err, filename );
      return 0;
   }

   SDL_mutexV( sound_lock );

   /* finish */
   free( wavdata );
   return 0;
}
static void sound_free( alSound *snd )
{
   if (sound_lock == NULL) return;

   SDL_mutexP( sound_lock );

   /* free the stuff */
   if (snd->name) free(snd->name);
   alDeleteBuffers( 1, &snd->buffer );

   SDL_mutexV( sound_lock );
}


/*
 * updates the sounds and prioritizes sounds
 */
void sound_update (void)
{
   ALint stat;
   alVoice *voice, *prev, *next;

   if (sound_lock == NULL) return; /* sound system is off */
   if (voice_start == NULL) return; /* no voices */

   SDL_mutexP( sound_lock );

   /* update sounds */
   prev = NULL;
   voice = voice_start;
   do {
      next = voice->next;
      if (!voice_is(voice, VOICE_DONE)) { /* still working */

         /* update position */
         alSource3f( voice->source, AL_POSITION,
               voice->px, voice->py, 0. );
   /*    alSource3f( voice->source, AL_VELOCITY,
               voice->vx, voice->vy, 0. );*/
      }
      else { /* delete them */
         if (voice->source != 0) { /* source must exist */

            /* stop it if playing */
            alGetSourcei( voice->source, AL_SOURCE_STATE, &stat );
            if (stat == AL_PLAYING) alSourceStop( voice->source );

            /* clear it and get rid of it */
            alDeleteSources( 1, &voice->source );
            voice->source = 0;
         }

         /* delete from linked list */
         if (prev == NULL) /* was the first member */
            voice_start = voice->next;
         else /* not first member */
            prev->next = voice->next;
         if (voice_end == voice) /* last voice in linked list */
            voice_end = prev;
         free(voice);
      }
      prev = voice;
      voice = next;
   } while (voice != NULL);

   SDL_mutexV( sound_lock );
}


/*
 * sets all the sounds volume to vol
 */
void sound_volume( const double vol )
{
   if (sound_lock == NULL) return;

   svolume = (ALfloat) vol;
}


/*
 * attempts to alloc a source for a voice
 */
static int voice_getSource( alVoice* voc )
{
   if (sound_lock == NULL) return -1;

   int ret;
   ALenum err;

   ret = 0; /* default return */

   SDL_mutexP( sound_lock );

   /* try to grab a source */
   voc->source = 0;
   alGenSources( 1, &voc->source );
   err = alGetError();
   if (err != AL_NO_ERROR) {
      voc->source = 0;
      ret = 1;
   }
   else { /* set the properties */
      alSourcei( voc->source, AL_BUFFER, voc->buffer );

      /* distance model */
      alSourcef( voc->source, AL_ROLLOFF_FACTOR, SOUND_ROLLOFF_FACTOR );
      alSourcef( voc->source, AL_MAX_DISTANCE, SOUND_MAX_DIST );
      alSourcef( voc->source, AL_REFERENCE_DISTANCE, SOUND_REFERENCE_DIST );

      alSourcei( voc->source, AL_SOURCE_RELATIVE, AL_FALSE );
      alSourcef( voc->source, AL_GAIN, svolume );
      alSource3f( voc->source, AL_POSITION, voc->px, voc->py, 0. );
      /* alSource3f( voc->source, AL_VELOCITY, voc->vx, voc->vy, 0. ); */
      if (voice_is( voc, VOICE_LOOPING ))
         alSourcei( voc->source, AL_LOOPING, AL_TRUE );
      else
         alSourcei( voc->source, AL_LOOPING, AL_FALSE );

      /* try to play the source */
      alSourcePlay( voc->source );
      err = alGetError();
      if (err == AL_NO_ERROR) voice_set( voc, VOICE_PLAYING );
      else ret = 2;
   }

   SDL_mutexV( sound_lock );

   return ret;
}


/*
 * creates a dynamic moving voice
 */
alVoice* sound_addVoice( int priority, double px, double py,
      double vx, double vy, const ALuint buffer, const int looping )
{
   (void) vx;
   (void) vy;
   alVoice *voc;

   if (sound_lock == NULL) return NULL;

   
   /* allocate the voice */
   voc = malloc(sizeof(alVoice));

   /* set the data */
   voc->next = NULL;
   voc->priority = priority;
   voc->start = SDL_GetTicks();
   voc->buffer = buffer;
   voc->flags = 0;
   if (looping!=0) voice_set( voc, VOICE_LOOPING );
   voc->px = px;
   voc->py = py;
/* voc->vx = vx;
   voc->vy = vy; */

   voice_getSource( voc );

   /* add to the linked list */
   if (voice_start == NULL) {
      voice_start = voc;
      voice_end = voc;
   }
   else {
      if (voice_end != NULL)
         voice_end->next = voc;
      voice_end = voc;
   }

   return voc;
}

/*
 * deletes the voice
 */
void sound_delVoice( alVoice* voice )
{
   if (sound_lock == NULL) return;

   voice_set(voice, VOICE_DONE);
}


/*
 * updates voice position, should be run once per frame
 */
void voice_update( alVoice* voice, double px, double py,
      double vx, double vy )
{
   (void) vx;
   (void) vy;

   if (sound_lock == NULL) return;

   voice->px = px;
   voice->py = py;
/* voice->vx = vx;
   voice->vy = vy;*/
}


/*
 * sets the listener
 */
void sound_listener( double dir, double px, double py,
      double vx, double vy )
{
   (void) vx;
   (void) vy;

   if (sound_lock == NULL) return;

   SDL_mutexP( sound_lock );

   /* set orientation */
   ALfloat ori[] = { 0., 0., 0.,  0., 0., 1. };
   ori[0] = cos(dir);
   ori[1] = sin(dir);
   alListenerfv( AL_ORIENTATION, ori );
   alListener3f( AL_POSITION, px, py, 1. );
/* alListener3f( AL_VELOCITY, vx, vy, 0. );*/

   SDL_mutexV( sound_lock );
}


