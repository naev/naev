/*
 * See Licensing and Copyright notice in naev.h
 */

#if USE_OPENAL


#include "sound_openal.h"

#include "naev.h"

#include <math.h>
#include <sys/stat.h>

#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_endian.h"

#include "music_openal.h"
#include "sound.h"
#include "ndata.h"
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
/*
 * Wav loading.
 */
static int sound_al_wavReadCmp( SDL_RWops *rw, const char *cmp, int bytes );
static int sound_al_wavGetLen32( SDL_RWops *rw, uint32_t *out );
static int sound_al_wavGetLen16( SDL_RWops *rw, uint16_t *out );
/*
 * General.
 */
static ALuint sound_al_getSource (void);
static int sound_al_loadWav( alSound *snd, SDL_RWops *rw );
static int sound_al_loadOgg( alSound *snd, OggVorbis_File *vf );


/*
 * Vorbis stuff.
 */
static size_t ovpack_read( void *ptr, size_t size, size_t nmemb, void *datasource )
{  
   SDL_RWops *rw = datasource;
   return (size_t) rw->read( rw, ptr, size, nmemb );
}
static int ovpack_seek( void *datasource, ogg_int64_t offset, int whence )
{  
   SDL_RWops *rw = datasource;
   return rw->seek( rw, offset, whence );
}
static int ovpack_close( void *datasource )
{  
   SDL_RWops *rw = datasource;
   return rw->close( rw );
}
static long ovpack_tell( void *datasource )
{  
   SDL_RWops *rw = datasource;
   return rw->seek( rw, 0, SEEK_CUR );
}
ov_callbacks sound_al_ovcall = {
   .read_func = ovpack_read,
   .seek_func = ovpack_seek,
   .close_func = ovpack_close,
   .tell_func = ovpack_tell
}; /**< Vorbis call structure to handl rwops. */


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

   /* Set the distance model */
   alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED );

   /* Allocate source for music. */
   alGenSources( 1, &music_source );

   /* Start allocating the sources - music has already taken his */
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

   /* Check for errors. */
   al_checkErr();

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
}


/**
 * @brief Compares some bytes in a wav type structure.
 *
 *    @brief Returns 0 on success.
 */
static int sound_al_wavReadCmp( SDL_RWops *rw, const char *cmp, int bytes )
{
   int len;
   char buf[4];

   len = SDL_RWread( rw, buf, bytes, 1 );
   if (len != 1) {
      WARN("Did not read expected bytes.");
      return -1;
   }
   return memcmp( buf, cmp, bytes );
}


/**
 * @brief Gets a 32 bit length in a wav type structure.
 */
static int sound_al_wavGetLen32( SDL_RWops *rw, uint32_t *out )
{
   int len;

   len = SDL_RWread( rw, out, 4, 1 );
   if (len != 1) {
      WARN("Did not read expected bytes.");
      return -1;
   }
   /* Little endian by default. */
   *out = SDL_SwapLE32(*out);

   return 0;
}


/**
 * @brief Gets a 16 bit length in a wav type structure.
 */
static int sound_al_wavGetLen16( SDL_RWops *rw, uint16_t *out )
{
   int len;

   len = SDL_RWread( rw, out, 2, 1 );
   if (len != 1) {
      WARN("Did not read expected bytes.");
      return -1;
   }
   /* Little endian by default. */
   *out = SDL_SwapLE16(*out);

   return 0;
}


/**
 * @brief Loads a wav file from the rw if possible.
 *
 * @note Closes the rw.
 *
 *    @param snd Sound to load wav into.
 *    @param rw Data for the wave.
 */
static int sound_al_loadWav( alSound *snd, SDL_RWops *rw )
{
   int len;
   uint32_t i;
   char magic[4], *buf;
   ALenum format;
   uint32_t filelen, chunklen, rate, unused32;
   uint16_t compressed, channels, align, unused16;

   /* Some initialization. */
   compressed = 0;
   channels   = 0;

   /* Check RIFF header. */
   if (sound_al_wavReadCmp( rw, "RIFF", 4 )) {
      WARN("RIFF header not found.");
      goto wav_err;
   }
   /* Get file length. */
   if (sound_al_wavGetLen32( rw, &filelen )) {
      WARN("Unable to get WAVE length.");
      goto wav_err;
   }
   /* Check WAVE header. */
   if (sound_al_wavReadCmp( rw, "WAVE", 4 )) {
      WARN("WAVE header not found.");
      goto wav_err;
   }

   /*
    * Chunk information header.
    */
   /* Check chunk header. */
   if (sound_al_wavReadCmp( rw, "fmt ", 4 )) {
      WARN("Chunk header 'fmt ' header not found.");
      goto wav_err;
   }
   /* Get chunk length. */
   if (sound_al_wavGetLen32( rw, &chunklen )) {
      WARN("Unable to get WAVE chunk length.");
      goto wav_err;
   }
   i = 0;

   /* Get compression. */
   if (sound_al_wavGetLen16( rw, &compressed )) {
      WARN("Unable to get WAVE chunk compression type.");
      goto wav_err;
   }
   if (compressed != 0x0001) {
      WARN("Unsupported WAVE chunk compression '0x%04x'.", compressed);
      goto wav_err;
   }
   i += 2;

   /* Get channels. */
   if (sound_al_wavGetLen16( rw, &channels )) {
      WARN("Unable to get WAVE chunk channels.");
      goto wav_err;
   }
   i += 2;

   /* Get sample rate. */
   if (sound_al_wavGetLen32( rw, &rate )) {
      WARN("Unable to get WAVE chunk sample rate.");
      goto wav_err;
   }
   i += 4;

   /* Get average bytes. */
   if (sound_al_wavGetLen32( rw, &unused32 )) {
      WARN("Unable to get WAVE chunk average byte rate.");
      goto wav_err;
   }
   i += 4;

   /* Get block align. */
   if (sound_al_wavGetLen16( rw, &unused16 )) {
      WARN("Unable to get WAVE chunk block align.");
      goto wav_err;
   }
   i += 2;

   /* Get significant bits. */
   if (sound_al_wavGetLen16( rw, &align )) {
      WARN("Unable to get WAVE chunk significant bits.");
      goto wav_err;
   }
   align /= channels;
   i += 2;

   /* Seek to end. */
   SDL_RWseek( rw, chunklen-i, SEEK_CUR );


   /* Read new header. */
   len = SDL_RWread( rw, magic, 4, 1 );
   if (len != 1) {
      WARN("Unable to read chunk header.");
      goto wav_err;
   }

   /* Skip fact. */
   if (memcmp( magic, "fact", 4)==0) {
      /* Get chunk length. */
      if (sound_al_wavGetLen32( rw, &chunklen )) {
         WARN("Unable to get WAVE chunk data length.");
         goto wav_err;
      }

      /* Seek to end of chunk. */
      SDL_RWseek( rw, chunklen, SEEK_CUR );

      /* Read new header. */
      len = SDL_RWread( rw, magic, 4, 1 );
      if (len != 1) {
         WARN("Unable to read chunk header.");
         goto wav_err;
      }
   }

   /* Should be chunk header now. */
   if (memcmp( magic, "data", 4)) {
      WARN("Unable to find WAVE 'data' chunk header.");
      goto wav_err;
   }

   /*
    * Chunk data header.
    */
   /* Get chunk length. */
   if (sound_al_wavGetLen32( rw, &chunklen )) {
      WARN("Unable to get WAVE chunk data length.");
      goto wav_err;
   }

   /* Load the chunk data. */
   buf = malloc( chunklen );
   i = 0;
   while (i < chunklen) {
      i += SDL_RWread( rw, &buf[i], 1, chunklen-i );
   }

   /* Calculate format. */
   if (channels == 2) {
      if (align == 16)
         format = AL_FORMAT_STEREO16;
      else if (align == 8)
         format = AL_FORMAT_STEREO8;
      else {
         WARN("Unsupported byte alignment (%d) in WAVE file.", align);
         goto chunk_err;
      }
   }
   else if (channels == 1) {
      if (align == 16)
         format = AL_FORMAT_MONO16;
      else if (align == 8)
         format = AL_FORMAT_MONO8;
      else {
         WARN("Unsupported byte alignment (%d) in WAVE file.", align);
         goto chunk_err;
      }
   }
   else {
      WARN("Unsupported number of channels (%d) in WAVE file.", channels);
      goto chunk_err;
   }

   /* Close file. */
   SDL_RWclose(rw);

   /* Create new buffer. */
   alGenBuffers( 1, &snd->u.al.buf );

   /* Put into the buffer. */
   alBufferData( snd->u.al.buf, format, buf, chunklen, rate );

   free(buf);
   return 0;

chunk_err:
   free(buf);
wav_err:
   SDL_RWclose(rw);
   return -1;
}


/**
 * @brief Loads an ogg file from a tested format if possible.
 *
 *    @param snd Sound to load ogg into.
 *    @param vf Vorbisfile containing the song.
 */
static int sound_al_loadOgg( alSound *snd, OggVorbis_File *vf )
{
   long i;
   int section;
   vorbis_info *info;
   ALenum format;
   ogg_int64_t len;
   char *buf;

   /* Finish opening the file. */
   if (ov_test_open(vf)) {
      WARN("Failed to finish loading OGG file.");
      return -1;
   }

   /* Get file information. */
   info   = ov_info( vf, -1 );
   format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
   len    = ov_pcm_total( vf, -1 ) * info->channels * 2;

   /* Create new buffer. */
   alGenBuffers( 1, &snd->u.al.buf );

   /* Allocate memory. */
   buf = malloc( len );

   /* Fill buffer. */
   i = 0;
   while (i < len) {
      /* Fill buffer with data in the 16 bit signed samples format. */
      i += ov_read( vf, &buf[i], len-i, VORBIS_ENDIAN, 2, 1, &section );
   }

   /* Put into buffer. */
   alBufferData( snd->u.al.buf, format, buf, len, info->rate );

   /* Clean up. */
   free(buf);
   ov_clear(vf);

   return 0;
}


/**
 * @brief Loads the sound.
 *
 *    @param snd Sound to load.
 *    @param filename Name of the file to load into sound.
 */
int sound_al_load( alSound *snd, const char *filename )
{
   int ret;
   SDL_RWops *rw;
   OggVorbis_File vf;

   /* get the file data buffer from packfile */
   rw = ndata_rwops( filename );

   soundLock();

   /* Check to see if it's an OGG. */
   if (ov_test_callbacks( rw, &vf, NULL, 0, sound_al_ovcall )==0) {
      ret = sound_al_loadOgg( snd, &vf );
   }
   /* Otherwise try WAV. */
   else {
      /* Destroy the partially loaded vorbisfile. */
      ov_clear(&vf);

      /* Reopen packfile. */
      rw = ndata_rwops( filename );

      /* Try to load Wav. */
      ret = sound_al_loadWav( snd, rw );
   }

   /* Failed to load. */
   if (ret != 0) {
      soundUnlock();
      WARN("Failed to load sound file '%s'.", filename);
      return ret;
   }

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

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

   /* Check for errors. */
   al_checkErr();

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

   /* Check for errors. */
   al_checkErr();

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

   /* Check for errors. */
   al_checkErr();

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

   /* Check for errors. */
   al_checkErr();

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

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

   return 0;
}


#ifndef al_checkError
/**
 * @brief Converts an OpenAL error to a string.
 *
 *    @param err Error to convert to string.
 *    @return String corresponding to the error.
 */
void al_checkErr (void)
{
   ALenum err;
   const char *errstr;

   /* Get the possible error. */
   err = alGetError();

   /* No error. */
   if (err == AL_NO_ERROR)
      return;

   /* Get the message. */
   switch (err) {
      case AL_INVALID_NAME:
         errstr = "a bad name (ID) was passed to an OpenAL function";
         break;
      case AL_INVALID_ENUM:
         errstr = "an invalid enum value was passed to an OpenAL function";
         break;
      case AL_INVALID_VALUE:
         errstr = "an invalid value was passed to an OpenAL function";
         break;
      case AL_INVALID_OPERATION:
         errstr = "the requested operation is not valid";
         break;
      case AL_OUT_OF_MEMORY:
         errstr = "the requested operation resulted in OpenAL running out of memory";
         break;

      default:
         errstr = "unknown error";
         break;
   }
   WARN("OpenAL error: %s", errstr);
}
#endif /* al_checkError */


#endif /* USE_OPENAL */
