/*
 * See Licensing and Copyright notice in naev.h
 */

#if USE_OPENAL


#include "sound_openal.h"

#include "naev.h"

#include <math.h>
#include <sys/stat.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_endian.h"

#include "music_openal.h"
#include "sound.h"
#include "ndata.h"
#include "log.h"
#include "conf.h"


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
 * 1) First we allocate all the buffers based on what we find inside the
 * datafile.
 * 2) Then we allocate all the possible sources (giving the music system
 * what it needs).
 * 3) Now we allow the user to dynamically create voices, these voices will
 * always try to grab a source from the source pool.  If they can't they
 * will pretend to play the buffer.
 * 4) Every so often we'll check to see if the important voices are being
 * played and take away the sources from the lesser ones.
 *
 *
 * EFX
 *
 * We use multiple effects, namely:
 *
 * - Air absorption factor
 * - Reverb
 */


#define SOUND_FADEOUT         100


#define soundLock()     SDL_mutexP(sound_lock)
#define soundUnlock()   SDL_mutexV(sound_lock)


/*
 * Global sound lock.
 */
SDL_mutex *sound_lock = NULL; /**< Global sound lock, always lock this before
                                   using any OpenAL functions. */


/*
 * Global device and context.
 */
static ALCcontext *al_context = NULL; /**< OpenAL context. */
static ALCdevice *al_device   = NULL; /**< OpenAL device. */
static ALfloat svolume        = 1.; /**< Sound global volume (logarithmic). */
static ALfloat svolume_lin    = 1.; /**< Sound global volume (linear). */
static ALfloat svolume_speed  = 1.; /**< Sound global volume modulator for speed. */
alInfo_t al_info; /**< OpenAL context info. */


/*
 * struct to hold all the sources and currently attached voice
 */
static ALuint *source_stack   = NULL; /**< Free source pool. */
static ALuint *source_total   = NULL; /**< Total source pool. */
static ALuint *source_all     = NULL; /**< All the sources. */
static int source_nstack      = 0; /**< Number of free sources in the pool. */
static int source_ntotal      = 0; /**< Number of general use sources. */
static int source_nall        = 0; /**< Total number of sources. */
static int source_mstack      = 0; /**< Memory allocated for sources in the pool. */


/*
 * EFX stuff.
 */
static ALuint efx_directSlot  = 0; /**< Direct 3d source slot. */
static ALuint efx_reverb      = 0; /**< Reverb effect. */
static ALuint efx_echo        = 0; /**< Echo effect. */


/*
 * Sound speed.
 */
static double sound_speed     = 1.; /**< Sound speed. */


/**
 * @brief Group implementation similar to SDL_Mixer.
 */
typedef struct alGroup_s {
   int id; /**< Group ID. */

   /* Sources. */
   ALuint *sources; /**< Sources in the group. */
   int nsources; /**< Number of sources in the group. */

   voice_state_t state; /**< Currently global group state. */
   int fade_timer; /**< Fadeout timer. */
   int speed; /**< Whether or not pitch affects. */
   double volume; /**< Volume of the group. */
} alGroup_t;
static alGroup_t *al_groups = NULL; /**< Created groups. */
static int al_ngroups       = 0; /**< Number of created groups. */
static int al_groupidgen    = 0; /**< Used to create group IDs. */


/*
 * Prototypes.
 */
/*
 * Loading.
 */
static const char* vorbis_getErr( int err );
static int al_enableEFX (void);
/*
 * General.
 */
static ALuint sound_al_getSource (void);
static int al_playVoice( alVoice *v, alSound *s,
      ALfloat px, ALfloat py, ALfloat vx, ALfloat vy, ALint relative );
static int sound_al_loadWav( alSound *snd, SDL_RWops *rw );
static int sound_al_loadOgg( alSound *snd, OggVorbis_File *vf );
/*
 * Pausing.
 */
static void al_pausev( ALint n, ALuint *s );
static void al_resumev( ALint n, ALuint *s );
/*
 * Groups.
 */
static alGroup_t *sound_al_getGroup( int group );
/*
 * Misc.
 */
static void sound_al_volumeUpdate (void);


/*
 * Vorbis stuff.
 */
static size_t ovpack_read( void *ptr, size_t size, size_t nmemb, void *datasource )
{
   SDL_RWops *rw = datasource;
   return (size_t) SDL_RWread( rw, ptr, size, nmemb );
}
static int ovpack_seek( void *datasource, ogg_int64_t offset, int whence )
{
   SDL_RWops *rw = datasource;
   return SDL_RWseek( rw, offset, whence );
}
static int ovpack_close( void *datasource )
{
   SDL_RWops *rw = datasource;
   return SDL_RWclose( rw );
}
static int ovpack_closeFake( void *datasource )
{
   (void) datasource;
   return 0;
}
static long ovpack_tell( void *datasource )
{
   SDL_RWops *rw = datasource;
   return SDL_RWseek( rw, 0, SEEK_CUR );
}
ov_callbacks sound_al_ovcall = {
   .read_func  = ovpack_read,
   .seek_func  = ovpack_seek,
   .close_func = ovpack_close,
   .tell_func  = ovpack_tell
}; /**< Vorbis call structure to handle rwops. */
ov_callbacks sound_al_ovcall_noclose = {
   .read_func  = ovpack_read,
   .seek_func  = ovpack_seek,
   .close_func = ovpack_closeFake,
   .tell_func  = ovpack_tell
}; /**< Vorbis call structure to handl rwops without closing. */


/**
 * @brief Initializes the sound subsystem.
 *
 *    @return 0 on success.
 */
int sound_al_init (void)
{
   int ret;
   ALuint s;
   ALint freq;
   ALint attribs[4] = { 0, 0, 0, 0 };

   /* Default values. */
   ret = 0;

   /* we'll need a mutex */
   sound_lock = SDL_CreateMutex();
   soundLock();

   /* opening the default device */
   al_device = alcOpenDevice(NULL);
   if (al_device == NULL) {
      WARN(_("Unable to open default sound device"));
      ret = -1;
      goto snderr_dev;
   }

   /* Query EFX extension. */
   if (conf.al_efx) {
      al_info.efx = alcIsExtensionPresent( al_device, "ALC_EXT_EFX" );
      if (al_info.efx == AL_TRUE) {
         attribs[0] = ALC_MAX_AUXILIARY_SENDS;
         attribs[1] = 4;
      }
   }
   else
      al_info.efx = AL_FALSE;

   /* Create the OpenAL context */
   al_context = alcCreateContext( al_device, attribs );
   if (al_context == NULL) {
      WARN(_("Unable to create OpenAL context"));
      ret = -2;
      goto snderr_ctx;
   }

   /* Clear the errors */
   alGetError();

   /* Set active context */
   if (alcMakeContextCurrent( al_context )==AL_FALSE) {
      WARN(_("Failure to set default context"));
      ret = -4;
      goto snderr_act;
   }

   /* Get context information. */
   alcGetIntegerv( al_device, ALC_FREQUENCY, sizeof(freq), &freq );

   /* Try to enable EFX. */
   if (al_info.efx == AL_TRUE) {
      al_enableEFX();
   }
   else {
      al_info.efx_reverb = AL_FALSE;
      al_info.efx_echo   = AL_FALSE;
   }

   /* Allocate source for music. */
   alGenSources( 1, &music_source );

   /* Check for errors. */
   al_checkErr();

   /* Start allocating the sources - music has already taken his */
   source_nstack  = 0;
   source_mstack  = 0;
   while (source_nstack < conf.snd_voices) {
      if (source_mstack < source_nstack+1) { /* allocate more memory */
         if (source_mstack == 0)
            source_mstack = conf.snd_voices;
         else
            source_mstack *= 2;
         source_stack = realloc( source_stack, sizeof(ALuint) * source_mstack );
      }
      alGenSources( 1, &s );
      source_stack[source_nstack] = s;

      /* How OpenAL distance model works:
       *
       * Clamped:
       *  gain = distance_function( CLAMP( AL_REFERENCE_DISTANCE, AL_MAX_DISTANCE, distance ) );
       *
       * Distance functions:
       *                                       AL_REFERENCE_DISTANCE
       *  * Inverse = ------------------------------------------------------------------------------
       *              AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR ( distance - AL_REFERENCE_DISTANCE )
       *
       *             1 - AL_ROLLOFF_FACTOR ( distance - AL_REFERENCE_DISTANCE )
       *  * Linear = ----------------------------------------------------------
       *                      AL_MAX_DISTANCE - AL_REFERENCE_DISTANCE
       *
       *                  /       distance        \ -AL_ROLLOFF_FACTOR
       *  * Exponential = | --------------------- |
       *                  \ AL_REFERENCE_DISTANCE /
       *
       *
       * Some values:
       *
       *  model    falloff  reference   100     1000    5000   10000
       *  linear     1        500      1.000   0.947   0.526   0.000
       *  inverse    1        500      1.000   0.500   0.100   0.050
       *  exponent   1        500      1.000   0.500   0.100   0.050
       *  inverse   0.5       500      1.000   0.667   0.182   0.095
       *  exponent  0.5       500      1.000   0.707   0.316   0.223
       *  inverse    2        500      1.000   0.333   0.052   0.026
       *  exponent   2        500      1.000   0.250   0.010   0.003
       */
      alSourcef( s, AL_REFERENCE_DISTANCE, 500. ); /* Close distance to clamp at (doesn't get louder). */
      alSourcef( s, AL_MAX_DISTANCE,       25000. ); /* Max distance to clamp at (doesn't get quieter). */
      alSourcef( s, AL_ROLLOFF_FACTOR,     1. ); /* Determines how it drops off. */

      /* Set the filter. */
      if (al_info.efx == AL_TRUE)
         alSource3i( s, AL_AUXILIARY_SEND_FILTER, efx_directSlot, 0, AL_FILTER_NULL );

      /* Check for error. */
      if (alGetError() == AL_NO_ERROR)
         source_nstack++;
      else
         break;
   }
   /* Reduce ram usage. */
   source_mstack = source_nstack;
   source_stack  = realloc( source_stack, sizeof(ALuint) * source_mstack );
   /* Copy allocated sources to total stack. */
   source_ntotal = source_mstack;
   source_total  = malloc( sizeof(ALuint) * source_mstack );
   memcpy( source_total, source_stack, sizeof(ALuint) * source_mstack );
   /* Copy allocated sources to all stack. */
   source_nall   = source_mstack;
   source_all    = malloc( sizeof(ALuint) * source_mstack );
   memcpy( source_all, source_stack, sizeof(ALuint) * source_mstack );

   /* Set up how sound works. */
   alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED ); /* Clamping is fundamental so it doesn't sound like crap. */
   alDopplerFactor( 1. );
   sound_al_env( SOUND_ENV_NORMAL, 0. );

   /* Check for errors. */
   al_checkErr();

   /* we can unlock now */
   soundUnlock();

   /* debug magic */
   DEBUG(_("OpenAL started: %d Hz"), freq);
   DEBUG(_("Renderer: %s"), alGetString(AL_RENDERER));
   if (al_info.efx)
      DEBUG(_("Version: %s with EFX %d.%d"), alGetString(AL_VERSION),
            al_info.efx_major, al_info.efx_minor);
   else
      DEBUG(_("Version: %s without EFX"), alGetString(AL_VERSION));
   DEBUG("");

   return ret;

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
 * @brief Enables the OpenAL EFX extension.
 *
 *    @return 0 on success.
 */
static int al_enableEFX (void)
{
   /* Issues with ALSOFT 1.19.1 crashes so we work around it.
    * TODO: Disable someday. */
   DEBUG( "'%s'", alGetString(AL_VERSION));
   if (strcmp(alGetString(AL_VERSION), "1.1 ALSOFT 1.19.1")==0) {
      DEBUG(_("Crashing ALSOFT version detected, disabling EFX"));
      al_info.efx = AL_FALSE;
      return -1;
   }

   /* Get general information. */
   alcGetIntegerv( al_device, ALC_MAX_AUXILIARY_SENDS, 1, &al_info.efx_auxSends );
   alcGetIntegerv( al_device, ALC_EFX_MAJOR_VERSION, 1, &al_info.efx_major );
   alcGetIntegerv( al_device, ALC_EFX_MINOR_VERSION, 1, &al_info.efx_minor );

   /* Get function pointers. */
   nalGenAuxiliaryEffectSlots  = alGetProcAddress( "alGenAuxiliaryEffectSlots" );
   nalDeleteAuxiliaryEffectSlots = alGetProcAddress( "alDeleteAuxiliaryEffectSlots" );
   nalIsAuxiliaryEffectSlot    = alGetProcAddress( "alIsAuxiliaryEffectSlot" );
   nalAuxiliaryEffectSloti     = alGetProcAddress( "alAuxiliaryEffectSloti" );
   nalAuxiliaryEffectSlotiv    = alGetProcAddress( "alAuxiliaryEffectSlotiv" );
   nalAuxiliaryEffectSlotf     = alGetProcAddress( "alAuxiliaryEffectSlotf" );
   nalAuxiliaryEffectSlotfv    = alGetProcAddress( "alAuxiliaryEffectSlotfv" );
   nalGetAuxiliaryEffectSloti  = alGetProcAddress( "alGetAuxiliaryEffectSloti" );
   nalGetAuxiliaryEffectSlotiv = alGetProcAddress( "alGetAuxiliaryEffectSlotiv" );
   nalGetAuxiliaryEffectSlotf  = alGetProcAddress( "alGetAuxiliaryEffectSlotf" );
   nalGetAuxiliaryEffectSlotfv = alGetProcAddress( "alGetAuxiliaryEffectSlotfv" );
   nalGenFilters               = alGetProcAddress( "alGenFilters" );
   nalDeleteFilters            = alGetProcAddress( "alDeleteFilters" );
   nalFilteri                  = alGetProcAddress( "alFilteri" );
   nalFilteriv                 = alGetProcAddress( "alFilteriv" );
   nalFilterf                  = alGetProcAddress( "alFilterf" );
   nalFilterfv                 = alGetProcAddress( "alFilterfv" );
   nalGenEffects               = alGetProcAddress( "alGenEffects" );
   nalDeleteEffects            = alGetProcAddress( "alDeleteEffects" );
   nalEffecti                  = alGetProcAddress( "alEffecti" );
   nalEffectiv                 = alGetProcAddress( "alEffectiv" );
   nalEffectf                  = alGetProcAddress( "alEffectf" );
   nalEffectfv                 = alGetProcAddress( "alEffectfv" );
   if (!nalGenAuxiliaryEffectSlots || !nalDeleteAuxiliaryEffectSlots ||
         !nalIsAuxiliaryEffectSlot ||
         !nalAuxiliaryEffectSloti || !nalAuxiliaryEffectSlotiv ||
         !nalAuxiliaryEffectSlotf || !nalAuxiliaryEffectSlotfv ||
         !nalGetAuxiliaryEffectSloti || !nalGetAuxiliaryEffectSlotiv ||
         !nalGetAuxiliaryEffectSlotf || !nalGetAuxiliaryEffectSlotfv ||
         !nalGenFilters || !nalDeleteFilters ||
         !nalFilteri || !nalFilteriv || !nalFilterf || !nalFilterfv ||
         !nalGenEffects || !nalDeleteEffects ||
         !nalEffecti || !nalEffectiv || !nalEffectf || !nalEffectfv) {
      DEBUG(_("OpenAL EFX functions not found, disabling EFX."));
      al_info.efx = AL_FALSE;
      return -1;
   }

   /* Create auxiliary slot. */
   nalGenAuxiliaryEffectSlots( 1, &efx_directSlot );

   /* Create reverb effect. */
   nalGenEffects( 1, &efx_reverb );
   nalEffecti( efx_reverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB );
   if(alGetError() != AL_NO_ERROR) {
      DEBUG(_("OpenAL Reverb not found, disabling."));
      al_info.efx_reverb = AL_FALSE;
      nalDeleteEffects( 1, &efx_reverb );
   }
   else {
      al_info.efx_reverb = AL_TRUE;

      /* Set Reverb parameters. */
      /*nalEffectf( efx_reverb, AL_REVERB_DECAY_TIME, 15. );*/
   }

   /* Create echo effect. */
   nalGenEffects( 1, &efx_echo );
   nalEffecti( efx_echo, AL_EFFECT_TYPE, AL_EFFECT_ECHO );
   if(alGetError() != AL_NO_ERROR) {
      DEBUG(_("OpenAL Echo not found, disabling."));
      al_info.efx_echo = AL_FALSE;
      nalDeleteEffects( 1, &efx_echo );
   }
   else {
      al_info.efx_echo = AL_TRUE;

      /* Set Echo parameters. */
      nalEffectf( efx_echo, AL_ECHO_DELAY, 0.207 );
   }

   /* Set up the listener. */
   alListenerf( AL_METERS_PER_UNIT, 5. );

   /* Check for errors. */
   al_checkErr();

   return 0;
}


/**
 * @brief Cleans up after the sound subsytem.
 */
void sound_al_exit (void)
{
   int i;

   soundLock();

   /* Free groups. */
   for (i=0; i<al_ngroups; i++) {
      if (al_groups[i].sources != NULL)
         free(al_groups[i].sources);
      al_groups[i].sources  = NULL;
      al_groups[i].nsources = 0;
   }
   if (al_groups != NULL)
      free(al_groups);
   al_groups  = NULL;
   al_ngroups = 0;

   /* Free stacks. */
   if (source_all != NULL) {
      alSourceStopv(   source_nall, source_all );
      alDeleteSources( source_nall, source_all );
      free(source_all);
   }
   source_all        = NULL;
   source_nall       = 0;
   if (source_total)
      free(source_total);
   source_total      = NULL;
   source_ntotal     = 0;
   if (source_stack != NULL)
      free(source_stack);
   source_stack      = NULL;
   source_nstack     = 0;
   source_mstack     = 0;

   /* Clean up EFX stuff. */
   if (al_info.efx == AL_TRUE) {
      nalDeleteAuxiliaryEffectSlots( 1, &efx_directSlot );
      if (al_info.efx_reverb == AL_TRUE)
         nalDeleteEffects( 1, &efx_reverb );
      if (al_info.efx_echo == AL_TRUE)
         nalDeleteEffects( 1, &efx_echo );
   }

   /* Clean up global stuff. */
   if (al_context) {
      alcMakeContextCurrent(NULL);
      alcDestroyContext( al_context );
   }
   if (al_device)
      alcCloseDevice( al_device );

   soundUnlock();
   SDL_DestroyMutex( sound_lock );
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
   SDL_AudioSpec wav_spec;
   Uint32 wav_length;
   Uint8 *wav_buffer;
   ALenum format;

   SDL_RWseek( rw, 0, SEEK_SET );

   /* Load WAV. */
   if (SDL_LoadWAV_RW( rw, 0, &wav_spec, &wav_buffer, &wav_length) == NULL) {
      WARN(_("SDL_LoadWav_RW failed: %s"), SDL_GetError());
      return -1;
   }

   /* Handle format. */
   switch (wav_spec.format) {
      case AUDIO_U8:
      case AUDIO_S8:
         format = (wav_spec.channels==1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
         break;
      case AUDIO_U16LSB:
      case AUDIO_S16LSB:
         format = (wav_spec.channels==1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
         break;
      case AUDIO_U16MSB:
      case AUDIO_S16MSB:
         WARN( _("Big endian WAVs unsupported!") );
         return -1;
      default:
         WARN( _("Invalid WAV format!") );
         return -1;
   }

   /* Load into openal. */
   soundLock();
   /* Create new buffer. */
   alGenBuffers( 1, &snd->u.al.buf );
   /* Put into the buffer. */
   alBufferData( snd->u.al.buf, format, wav_buffer, wav_length, wav_spec.freq );
   soundUnlock();

   /* Clean up. */
   free( wav_buffer );
   return 0;
}


/**
 * @brief Gets the vorbisfile error in human readable form..
 */
static const char* vorbis_getErr( int err )
{
   switch (err) {
      case OV_EREAD:       return _("A read from media returned an error.");
      case OV_EFAULT:      return _("Internal logic fault; indicates a bug or heap/stack corruption.");
      case OV_EIMPL:       return _("Feature not implemented.");
      case OV_EINVAL:      return _("Either an invalid argument, or incompletely initialized argument passed to libvorbisfile call");
      case OV_ENOTVORBIS:  return _("Bitstream is not Vorbis data.");
      case OV_EBADHEADER:  return _("Invalid Vorbis bitstream header.");
      case OV_EVERSION:    return _("Vorbis version mismatch.");
      case OV_EBADLINK:    return _("The given link exists in the Vorbis data stream, but is not decipherable due to garbacge or corruption.");
      case OV_ENOSEEK:     return _("The given stream is not seekable.");

      default: return _("Unknown vorbisfile error.");
   }
}


/**
 * @brief Loads an ogg file from a tested format if possible.
 *
 *    @param snd Sound to load ogg into.
 *    @param vf Vorbisfile containing the song.
 */
static int sound_al_loadOgg( alSound *snd, OggVorbis_File *vf )
{
   int ret;
   long i;
   int section;
   vorbis_info *info;
   ALenum format;
   ogg_int64_t len;
   char *buf;

   /* Finish opening the file. */
   ret = ov_test_open(vf);
   if (ret) {
      WARN(_("Failed to finish loading Ogg file: %s"), vorbis_getErr(ret) );
      return -1;
   }

   /* Get file information. */
   info   = ov_info( vf, -1 );
   format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
   len    = ov_pcm_total( vf, -1 ) * info->channels * 2;

   /* Allocate memory. */
   buf = malloc( len );

   /* Fill buffer. */
   i = 0;
   while (i < len) {
      /* Fill buffer with data in the 16 bit signed samples format. */
      i += ov_read( vf, &buf[i], len-i, VORBIS_ENDIAN, 2, 1, &section );
   }

   soundLock();
   /* Create new buffer. */
   alGenBuffers( 1, &snd->u.al.buf );
   /* Put into buffer. */
   alBufferData( snd->u.al.buf, format, buf, len, info->rate );
   soundUnlock();

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
   ALint freq, bits, channels, size;

   /* get the file data buffer from packfile */
   rw = ndata_rwops( filename );

   /* Check to see if it's an Ogg. */
   if (ov_test_callbacks( rw, &vf, NULL, 0, sound_al_ovcall_noclose )==0)
      ret = sound_al_loadOgg( snd, &vf );

   /* Otherwise try WAV. */
   else {
      /* Destroy the partially loaded vorbisfile. */
      ov_clear(&vf);

      /* Try to load Wav. */
      ret = sound_al_loadWav( snd, rw );
   }

   /* Close RWops. */
   SDL_RWclose(rw);

   /* Failed to load. */
   if (ret != 0) {
      WARN(_("Failed to load sound file '%s'."), filename);
      return ret;
   }

   soundLock();

   /* Get the length of the sound. */
   alGetBufferi( snd->u.al.buf, AL_FREQUENCY, &freq );
   alGetBufferi( snd->u.al.buf, AL_BITS, &bits );
   alGetBufferi( snd->u.al.buf, AL_CHANNELS, &channels );
   alGetBufferi( snd->u.al.buf, AL_SIZE, &size );
   if ((freq==0) || (bits==0) || (channels==0)) {
      WARN(_("Something went wrong when loading sound file '%s'."), filename);
      snd->length = 0;
   }
   else
      snd->length = (double)size / (double)(freq * (bits/8) * channels);

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
 * @brief Internal volume update function.
 */
static void sound_al_volumeUpdate (void)
{
   int i, j;
   alGroup_t *g;
   double v;

   soundLock();
   /* Do generic ones. */
   for (i=0; i<source_ntotal; i++)
      alSourcef( source_total[i], AL_GAIN, svolume*svolume_speed );
   /* Do specific groups. */
   for (i=0; i<al_ngroups; i++) {
      g = &al_groups[i];
      v = svolume * g->volume;
      if (g->speed)
         v *= svolume_speed;
      for (j=0; j<g->nsources; j++)
         alSourcef( g->sources[j], AL_GAIN, v );
   }
   soundUnlock();
}


/**
 * @brief Sets all the sounds volume to vol
 */
int sound_al_volume( double vol )
{
   /* Calculate volume level. */
   svolume_lin = vol;
   if (vol > 0.) /* Floor of -48 dB (0.00390625 amplitude) */
      svolume = (ALfloat) 1 / pow(2, (1 - vol) * 8);
   else
      svolume     = 0.;

   /* Update volume. */
   sound_al_volumeUpdate();

   return 0;
}


/**
 * @brief Gets the current volume level (linear).
 */
double sound_al_getVolume (void)
{
   return svolume_lin;
}


/**
 * @brief Gets the current volume level (logarithmic).
 */
double sound_al_getVolumeLog(void)
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

   return source;
}


/**
 * @brief Plays a voice.
 */
static int al_playVoice( alVoice *v, alSound *s,
      ALfloat px, ALfloat py, ALfloat vx, ALfloat vy, ALint relative )
{
   /* Must be below the limit. */
   if (sound_speed > SOUND_SPEED_PLAY_LIMIT)
      return 0;

   /* Set up the source and buffer. */
   v->u.al.source = sound_al_getSource();
   if (v->u.al.source == 0)
      return -1;
   v->u.al.buffer = s->u.al.buf;

   soundLock();

   /* Attach buffer. */
   alSourcei( v->u.al.source, AL_BUFFER, v->u.al.buffer );

   /* Enable positional sound. */
   alSourcei( v->u.al.source, AL_SOURCE_RELATIVE, relative );

   /* Update position. */
   v->u.al.pos[0] = px;
   v->u.al.pos[1] = py;
   v->u.al.pos[2] = 0.;
   v->u.al.vel[0] = vx;
   v->u.al.vel[1] = vy;
   v->u.al.vel[2] = 0.;

   /* Set up properties. */
   alSourcef(  v->u.al.source, AL_GAIN, svolume*svolume_speed );
   alSourcefv( v->u.al.source, AL_POSITION, v->u.al.pos );
   alSourcefv( v->u.al.source, AL_VELOCITY, v->u.al.vel );

   /* Defaults just in case. */
   alSourcei( v->u.al.source, AL_LOOPING, AL_FALSE );

   /* Start playing. */
   alSourcePlay( v->u.al.source );

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

   return 0;
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
   return al_playVoice( v, s, 0., 0., 0., 0., AL_TRUE );
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
   return al_playVoice( v, s, px, py, vx, vy, AL_FALSE );
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

   /* Invalid source, mark to delete. */
   if (v->u.al.source == 0) {
      v->state = VOICE_DESTROY;
      return;
   }

   soundLock();

   /* Get status. */
   alGetSourcei( v->u.al.source, AL_SOURCE_STATE, &state );
   if (state == AL_STOPPED) {

      /* Remove buffer so it doesn't start up again if resume is called. */
      alSourcei( v->u.al.source, AL_BUFFER, AL_NONE );

      /* Check for errors. */
      al_checkErr();

      soundUnlock();

      /* Put source back on the list. */
      source_stack[source_nstack] = v->u.al.source;
      source_nstack++;
      v->u.al.source = 0;

      /* Mark as stopped - erased next iteration. */
      v->state = VOICE_STOPPED;
      return;
   }

   /* Set up properties. */
   alSourcef(  v->u.al.source, AL_GAIN, svolume*svolume_speed );
   alSourcefv( v->u.al.source, AL_POSITION, v->u.al.pos );
   alSourcefv( v->u.al.source, AL_VELOCITY, v->u.al.vel );

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
      alSourceStop( voice->u.al.source );

   /* Check for errors. */
   al_checkErr();

   soundUnlock();
}


/**
 * @brief Pauses all sounds.
 */
void sound_al_pause (void)
{
   soundLock();
   al_pausev( source_ntotal, source_total );
   /* Check for errors. */
   al_checkErr();
   soundUnlock();
}


/**
 * @brief Resumes all sounds.
 */
void sound_al_resume (void)
{
   soundLock();
   al_resumev( source_ntotal, source_total );
   /* Check for errors. */
   al_checkErr();
   soundUnlock();
}


/**
 * @brief Set the playing speed.
 */
void sound_al_setSpeed( double s )
{
   int i, j;
   alGroup_t *g;

   soundLock();
   sound_speed = s; /* Set the speed. */
   /* Do all the groupless. */
   for (i=0; i<source_ntotal; i++)
      alSourcef( source_total[i], AL_PITCH, s );
   /* Do specific groups. */
   for (i=0; i<al_ngroups; i++) {
      g = &al_groups[i];
      if (!g->speed)
         continue;
      for (j=0; j<g->nsources; j++)
         alSourcef( g->sources[j], AL_PITCH, s );
   }
   /* Check for errors. */
   al_checkErr();
   soundUnlock();
}


/**
 * @brief Set the speed volume.
 */
void sound_al_setSpeedVolume( double vol )
{
   svolume_speed = vol;
   sound_al_volumeUpdate();
}


/**
 * @brief Updates the listener.
 */
int sound_al_updateListener( double dir, double px, double py,
      double vx, double vy )
{
   double c, s;
   ALfloat ori[6], pos[3], vel[3];

   c = cos(dir);
   s = sin(dir);

   soundLock();

   ori[0] = c;
   ori[1] = s;
   ori[2] = 0.;
   ori[3] = 0.;
   ori[4] = 0.;
   ori[5] = 1.;
   alListenerfv( AL_ORIENTATION, ori );
   pos[0] = px;
   pos[1] = py;
   pos[2] = 0.;
   alListenerfv( AL_POSITION, pos );
   vel[0] = vx;
   vel[1] = vy;
   vel[2] = 0.;
   alListenerfv( AL_VELOCITY, vel );

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

   return 0;
}


/**
 * @brief Creates a sound environment.
 */
int sound_al_env( SoundEnv_t env, double param )
{
   int i;
   ALuint s;
   ALfloat f;

   soundLock();
   switch (env) {
      case SOUND_ENV_NORMAL:
         /* Set global parameters. */
         alSpeedOfSound( 3433. );

         if (al_info.efx == AL_TRUE) {
            /* Disconnect the effect. */
            nalAuxiliaryEffectSloti( efx_directSlot,
                  AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL );

            /* Set per-source parameters. */
            for (i=0; i<source_ntotal; i++) {
               s = source_total[i];
               alSourcef( s, AL_AIR_ABSORPTION_FACTOR, 0. );
            }
         }
         break;

      case SOUND_ENV_NEBULA:
         f = param / 1000.;

         /* Set global parameters. */
         alSpeedOfSound( 3433./(1. + f*2.) );

         if (al_info.efx == AL_TRUE) {

            if (al_info.efx_reverb == AL_TRUE) {
               /* Tweak the reverb. */
               nalEffectf( efx_reverb, AL_REVERB_DECAY_TIME,    10. );
               nalEffectf( efx_reverb, AL_REVERB_DECAY_HFRATIO, 0.5 );

               /* Connect the effect. */
               nalAuxiliaryEffectSloti( efx_directSlot,
                     AL_EFFECTSLOT_EFFECT, efx_reverb );
            }

            /* Set per-source parameters. */
            for (i=0; i<source_ntotal; i++) {
               s = source_total[i];
               /* Value is from 0. (normal) to 10..
                * It represents the attenuation per meter. In this case it decreases by
                * 0.05*AB_FACTOR dB/meter where AB_FACTOR is the air absoprtion factor.
                * In our case each pixel represents 5 meters.
                */
               alSourcef( s, AL_AIR_ABSORPTION_FACTOR, 3.*f );
            }
         }
         break;
   }

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

   return 0;
}


/**
 * @brief Creates a group.
 */
int sound_al_createGroup( int size )
{
   int i, j, id;
   alGroup_t *g;

   /* Get new ID. */
   id = ++al_groupidgen;

   /* Grow group list. */
   al_ngroups++;
   al_groups = realloc( al_groups, sizeof(alGroup_t) * al_ngroups );
   g        = &al_groups[ al_ngroups-1 ];
   memset( g, 0, sizeof(alGroup_t) );
   g->id    = id;
   g->state = VOICE_PLAYING;
   g->speed = 1;
   g->volume = 1.;

   /* Allocate sources. */
   g->sources  = calloc( size, sizeof(ALuint) );
   g->nsources = size;

   /* Add some sources. */
   for (i=0; i<size; i++) {
      /* Make sure there's enough. */
      if (source_nstack <= 0)
         goto group_err;

      /* Pull one off the stack. */
      source_nstack--;
      g->sources[i] = source_stack[source_nstack];

      /* Disable EFX, they don't affect groups. */
      if (al_info.efx_reverb == AL_TRUE) {
         alSourcef(  g->sources[i], AL_AIR_ABSORPTION_FACTOR, 0. );
         alSource3i( g->sources[i], AL_AUXILIARY_SEND_FILTER,
               AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL );
      }

      /* Remove from total too. */
      for (j=0; j<source_ntotal; j++) {
         if (g->sources[i] == source_total[j]) {
            source_ntotal--;
            memmove( &source_total[j], &source_total[j+1],
                  sizeof(ALuint) * (source_ntotal - j) );
            break;
         }
      }
   }

   return id;

group_err:
   free(g->sources);
   al_ngroups--;
   return 0;
}


/**
 * @brief Gets a group by ID.
 */
static alGroup_t *sound_al_getGroup( int group )
{
   int i;
   for (i=0; i<al_ngroups; i++) {
      if (al_groups[i].id != group)
         continue;
      return &al_groups[i];
   }
   WARN(_("Group '%d' not found."), group);
   return NULL;
}


/**
 * @brief Plays a sound in a group.
 */
int sound_al_playGroup( int group, alSound *s, int once )
{
   int i, j;
   alGroup_t *g;
   ALint state;
   double v;

   for (i=0; i<al_ngroups; i++) {

      /* Find group. */
      if (al_groups[i].id != group)
         continue;

      g = &al_groups[i];
      g->state = VOICE_PLAYING;
      soundLock();
      for (j=0; j<g->nsources; j++) {
         alGetSourcei( g->sources[j], AL_SOURCE_STATE, &state );

         /* No free ones, just smash the last one. */
         if (j == g->nsources-1) {
            if (state != AL_STOPPED)
               alSourceStop( g->sources[j] );
         }
         /* Ignore playing/paused. */
         else if ((state == AL_PLAYING) || (state == AL_PAUSED))
            continue;

         /* Attach buffer. */
         alSourcei( g->sources[j], AL_BUFFER, s->u.al.buf );

         /* Do not do positional sound. */
         alSourcei( g->sources[j], AL_SOURCE_RELATIVE, AL_TRUE );

         /* See if should loop. */
         alSourcei( g->sources[j], AL_LOOPING, (once) ? AL_FALSE : AL_TRUE );

         /* Set volume. */
         v = svolume * g->volume;
         if (g->speed)
            v *= svolume_speed;
         alSourcef( g->sources[j], AL_GAIN, v );

         /* Start playing. */
         alSourcePlay( g->sources[j] );

         /* Check for errors. */
         al_checkErr();

         soundUnlock();
         return 0;
      }
      soundUnlock();

      WARN(_("Group '%d' has no free sounds."), group );

      /* Group matched but not found. */
      break;
   }

   if (i>=al_ngroups)
      WARN(_("Group '%d' not found."), group);

   return -1;
}


/**
 * @brief Stops a group.
 */
void sound_al_stopGroup( int group )
{
   alGroup_t *g;
   g = sound_al_getGroup( group );
   if (g == NULL)
      return;

   g->state      = VOICE_FADEOUT;
   g->fade_timer = SDL_GetTicks();
}


/**
 * @brief Pauses a group.
 */
void sound_al_pauseGroup( int group )
{
   alGroup_t *g;
   g = sound_al_getGroup( group );
   if (g == NULL)
      return;

   soundLock();
   al_pausev( g->nsources, g->sources );
   soundUnlock();
}


/**
 * @brief Resumes a group.
 */
void sound_al_resumeGroup( int group )
{
   alGroup_t *g;
   g = sound_al_getGroup( group );
   if (g == NULL)
      return;

   soundLock();
   al_resumev( g->nsources, g->sources );
   soundUnlock();
}


/**
 * @brief Sets the speed of the group.
 */
void sound_al_speedGroup( int group, int enable )
{
   alGroup_t *g;
   g = sound_al_getGroup( group );
   if (g == NULL)
      return;

   g->speed = enable;
}


/**
 * @brief Sets the volume of the group.
 */
void sound_al_volumeGroup( int group, double volume )
{
   alGroup_t *g;
   g = sound_al_getGroup( group );
   if (g == NULL)
      return;

   g->volume = volume;
}


/**
 * @brief Acts like alSourcePausev but with proper checks.
 */
static void al_pausev( ALint n, ALuint *s )
{
   int i;
   ALint state;

   for (i=0; i<n; i++) {
      alGetSourcei( s[i], AL_SOURCE_STATE, &state );
      if (state == AL_PLAYING)
         alSourcePause( s[i] );
   }
}


/**
 * @brief Acts like alSourcePlayv but with proper checks to just resume.
 */
static void al_resumev( ALint n, ALuint *s )
{
   int i;
   ALint state;

   for (i=0; i<n; i++) {
      alGetSourcei( s[i], AL_SOURCE_STATE, &state );
      if (state == AL_PAUSED)
         alSourcePlay( s[i] );
   }
}


/**
 * @brief Updates the group sounds.
 */
void sound_al_update (void)
{
   int i, j;
   alGroup_t *g;
   ALfloat d, v;
   unsigned int t, f;

   t = SDL_GetTicks();

   for (i=0; i<al_ngroups; i++) {
      g = &al_groups[i];
      /* Handle fadeout. */
      if (g->state != VOICE_FADEOUT)
         continue;

      /* Calculate fadeout. */
      f = t - g->fade_timer;
      if (f < SOUND_FADEOUT) {
         d = 1. - (ALfloat) f / (ALfloat) SOUND_FADEOUT;
         v = d * svolume * g->volume;
         if (g->speed)
            v *= svolume_speed;
         soundLock();
         for (j=0; j<g->nsources; j++)
            alSourcef( g->sources[j], AL_GAIN, v );
         /* Check for errors. */
         al_checkErr();
         soundUnlock();
      }
      /* Fadeout done. */
      else {
         soundLock();
         v = svolume * g->volume;
         if (g->speed)
            v *= svolume_speed;
         for (j=0; j<g->nsources; j++) {
            alSourceStop( g->sources[j] );
            alSourcei( g->sources[j], AL_BUFFER, AL_NONE );
            alSourcef( g->sources[j], AL_GAIN, v );
         }
         /* Check for errors. */
         al_checkErr();
         soundUnlock();

         /* Mark as done. */
         g->state = VOICE_PLAYING;
      }
   }
}


#ifdef DEBUGGING
/**
 * @brief Converts an OpenAL error to a string.
 *
 *    @param err Error to convert to string.
 *    @return String corresponding to the error.
 */
void al_checkHandleError( const char *func )
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
         errstr = _("a bad name (ID) was passed to an OpenAL function");
         break;
      case AL_INVALID_ENUM:
         errstr = _("an invalid enum value was passed to an OpenAL function");
         break;
      case AL_INVALID_VALUE:
         errstr = _("an invalid value was passed to an OpenAL function");
         break;
      case AL_INVALID_OPERATION:
         errstr = _("the requested operation is not valid");
         break;
      case AL_OUT_OF_MEMORY:
         errstr = _("the requested operation resulted in OpenAL running out of memory");
         break;

      default:
         errstr = _("unknown error");
         break;
   }
   WARN(_("OpenAL error [%s]: %s"), func, errstr);
}
#endif /* DEBUGGING */


#endif /* USE_OPENAL */
