/*
 * See Licensing and Copyright notice in naev.h
 */

#if USE_OPENAL


#include "music_openal.h"

#include <math.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_rwops.h"

#include <vorbis/vorbisfile.h>

#include "music.h"
#include "sound_openal.h"
#include "naev.h"
#include "log.h"
#include "pack.h"
#include "conf.h"


/**
 * @brief Default pre-amp in dB.
 */
#define RG_PREAMP_DB       0.0


/* Lock for OpenAL operations. */
#define soundLock()        SDL_mutexP(sound_lock)
#define soundUnlock()      SDL_mutexV(sound_lock)

/* Lock for all state/cond operations. */
#define musicLock()        SDL_mutexP(music_state_lock)
#define musicUnlock()      SDL_mutexV(music_state_lock)

/* Lock for all vorbisfile operations. */
#define musicVorbisLock()  SDL_mutexP(music_vorbis_lock)
#define musicVorbisUnlock() SDL_mutexV(music_vorbis_lock)


typedef enum music_cmd_e {
   MUSIC_CMD_NONE,
   MUSIC_CMD_KILL,
   MUSIC_CMD_STOP,
   MUSIC_CMD_PLAY,
   MUSIC_CMD_PAUSE,
   MUSIC_CMD_FADEIN,
   MUSIC_CMD_FADEOUT
} music_cmd_t;


typedef enum music_state_e {
   MUSIC_STATE_DEAD,
   MUSIC_STATE_STARTUP,
   MUSIC_STATE_IDLE,
   MUSIC_STATE_FADEIN,
   MUSIC_STATE_FADEOUT,
   MUSIC_STATE_PLAYING,
   MUSIC_STATE_PAUSED,
   /* Internal usage. */
   MUSIC_STATE_LOADING,
   MUSIC_STATE_STOPPING,
   MUSIC_STATE_PAUSING,
   MUSIC_STATE_RESUMING
} music_state_t;


static SDL_Thread *music_player = NULL; /**< Music player thread. */

/*
 * Playing buffers.
 */
static int music_bufSize            = 32*1024; /**< Size of music playing buffer. */
static char *music_buf              = NULL; /**< Music playing buffer. */


/*
 * Locks.
 */
extern SDL_mutex *sound_lock; /**< Global sound lock, used for all OpenAL calls. */
static SDL_mutex *music_vorbis_lock = NULL; /**< Lock for vorbisfile operations. */
static SDL_cond  *music_state_cond  = NULL; /**< Cond for thread to signal status updates. */
static SDL_mutex *music_state_lock  = NULL; /**< Lock for music state. */
static music_cmd_t   music_command  = MUSIC_CMD_NONE; /**< Target music state. */
static music_state_t music_state    = MUSIC_STATE_DEAD; /**< Current music state. */
static int music_forced             = 0; /**< Whether or not music is force stopped. */


/*
 * saves the music to ram in this structure
 */
typedef struct alMusic_ {
   char name[64]; /**< Song name. */
   SDL_RWops *rw; /**< RWops file reading from. */
   OggVorbis_File stream; /**< Vorbis file stream. */
   vorbis_info* info; /**< Information of the stream. */
   ALenum format; /**< Stream format. */
   /* Replaygain information. */
   ALfloat rg_scale_factor; /**< Scale factor. */
   ALfloat rg_max_scale; /**< Maximum scale factor before clipping. */
} alMusic;


/*
 * song currently playing
 */
static alMusic music_vorbis; /**< Current music. */
static ALuint music_buffer[2]; /**< Front and back buffer. */
ALuint music_source                    = 0; /**< Source associated to music. */


/*
 * volume
 */
static ALfloat music_vol     = 1.; /**< Current volume level (logarithmic). */
static ALfloat music_vol_lin = 1.; /**< Current volume level (linear). */


/*
 * prototypes
 */
#ifdef HAVE_OV_READ_FILTER
static void rg_filter( float **pcm, long channels, long samples, void *filter_param );
#endif /* HAVE_OV_READ_FILTER */
static void music_kill (void);
static int music_thread( void* unused );
static int stream_loadBuffer( ALuint buffer );


/**
 * @brief The music thread.
 *
 *    @param unused Unused.
 */
static int music_thread( void* unused )
{
   (void)unused;

   int ret;
   int active = 0; /* active buffer */
   ALint state;
   ALuint removed[2];
   ALenum value;
   music_state_t cur_state;
   ALfloat gain;
   int fadein_start = 0;
   uint32_t fade, fade_timer = 0;

   while (1) {
      /* Handle states. */
      musicLock();

      /* Handle new command. */
      switch (music_command) {
         case MUSIC_CMD_KILL:
            if (music_state != MUSIC_STATE_IDLE)
               music_state = MUSIC_STATE_STOPPING;
            else
               music_state = MUSIC_STATE_DEAD;

            /* Does not clear command. */
            break;

         case MUSIC_CMD_STOP:
            /* Notify of stopped. */
            if (music_state == MUSIC_STATE_IDLE)
               SDL_CondBroadcast( music_state_cond );
            else
               music_state = MUSIC_STATE_STOPPING;
            break;

         case MUSIC_CMD_PLAY:
            /* Set appropriate state. */
            if (music_state == MUSIC_STATE_PAUSING)
               music_state = MUSIC_STATE_RESUMING;
            else if (music_state == MUSIC_STATE_FADEIN)
               fade_timer = SDL_GetTicks() - MUSIC_FADEIN_DELAY;
            else
               music_state = MUSIC_STATE_LOADING;
            /* Disable fadein. */
            fadein_start = 0;
            /* Clear command. */
            music_command = MUSIC_CMD_NONE;
            SDL_CondBroadcast( music_state_cond );
            break;

         case MUSIC_CMD_FADEOUT:
            /* Notify of stopped. */
            if (music_state != MUSIC_STATE_IDLE) {
               music_state = MUSIC_STATE_FADEOUT;
               /* Set timer. */
               fade_timer = SDL_GetTicks();
            }
            /* Clear command. */
            music_command = MUSIC_CMD_NONE;
            SDL_CondBroadcast( music_state_cond );
            break;

         case MUSIC_CMD_FADEIN:
            if ((music_state == MUSIC_STATE_FADEIN) ||
                  (music_state == MUSIC_STATE_PLAYING))
               SDL_CondBroadcast( music_state_cond );
            else {
               music_state = MUSIC_STATE_LOADING;
               /* Set timer. */
               fade_timer = SDL_GetTicks();
               fadein_start = 1;
            }
            /* Clear command. */
            music_command = MUSIC_CMD_NONE;
            break;

         case MUSIC_CMD_PAUSE:
            if (music_state == MUSIC_STATE_PAUSED)
               SDL_CondBroadcast( music_state_cond );
            else if ((music_state == MUSIC_STATE_PLAYING) ||
                  (music_state == MUSIC_STATE_FADEIN))
               music_state = MUSIC_STATE_PAUSING;
            music_command = MUSIC_CMD_NONE;
            break;

         case MUSIC_CMD_NONE:
            break;
      }
      cur_state = music_state;
      musicUnlock();

      /* Main processing loop. */
      switch (cur_state) {
         /* Basically send a message that thread is up and running. */
         case MUSIC_STATE_STARTUP:
            musicLock();
            music_state = MUSIC_STATE_IDLE;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /* We died. */
         case MUSIC_STATE_DEAD:
            musicLock();
            music_state = MUSIC_STATE_DEAD;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            return 0;
            break;

         /* Delays at the end. */
         case MUSIC_STATE_PAUSED:
         case MUSIC_STATE_IDLE:
            break;

         /* Resumes the paused song. */
         case MUSIC_STATE_RESUMING:
            soundLock();
            alSourcePlay( music_source );
            alSourcef( music_source, AL_GAIN, music_vol );
            /* Check for errors. */
            al_checkErr();
            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_PLAYING;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /* Pause the song. */
         case MUSIC_STATE_PAUSING:
            soundLock();
            alSourcePause( music_source );
            /* Check for errors. */
            al_checkErr();
            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_PAUSED;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /* Stop song setting to IDLE. */
         case MUSIC_STATE_STOPPING:
            soundLock();

            /* Stop and remove buffers. */
            alSourceStop( music_source );
            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &value );
            if (value > 0)
               alSourceUnqueueBuffers( music_source, value, removed );
            /* Clear timer. */
            fade_timer = 0;

            /* Reset volume. */
            alSourcef( music_source, AL_GAIN, music_vol );

            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_IDLE;
            SDL_CondBroadcast( music_state_cond );
            if (!music_forced)
               music_rechoose();
            musicUnlock();
            break;

         /* Load the song. */
         case MUSIC_STATE_LOADING:
            /* Load buffer and start playing. */
            active = 0; /* load first buffer */
            ret = stream_loadBuffer( music_buffer[active] );
            soundLock();
            alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

            /* Special case NULL file or error. */
            if (ret < 0) {
               soundUnlock();
               /* Force state to stopped. */
               musicLock();
               music_state = MUSIC_STATE_IDLE;
               SDL_CondBroadcast( music_state_cond );
               if (!music_forced)
                  music_rechoose();
               musicUnlock();
               break;
            }
            /* Force volume level. */
            alSourcef( music_source, AL_GAIN, (fadein_start) ? 0. : music_vol );

            /* Start playing. */
            alSourcePlay( music_source );

            /* Check for errors. */
            al_checkErr();

            soundUnlock();
            /* Special case of a very short song. */
            if (ret > 1) {
               active = -1;

               musicLock();
               if (fadein_start)
                  music_state = MUSIC_STATE_FADEIN;
               else
                  music_state = MUSIC_STATE_PLAYING;
               SDL_CondBroadcast( music_state_cond );
               musicUnlock();
               break;
            }

            /* Load second buffer. */
            active = 1;
            ret = stream_loadBuffer( music_buffer[active] );
            if (ret < 0)
               active = -1;
            else {
               soundLock();
               alSourceQueueBuffers( music_source, 1, &music_buffer[active] );
               /* Check for errors. */
               al_checkErr();
               soundUnlock();
               active = 1 - active;
            }

            musicLock();
            if (fadein_start)
               music_state = MUSIC_STATE_FADEIN;
            else
               music_state = MUSIC_STATE_PLAYING;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /* Fades in the music. */
         case MUSIC_STATE_FADEOUT:
         case MUSIC_STATE_FADEIN:
            /* See if must still fade. */
            fade = SDL_GetTicks() - fade_timer;
            if (cur_state == MUSIC_STATE_FADEIN) {
               if (fade < MUSIC_FADEIN_DELAY) {
                  gain = (ALfloat)fade / (ALfloat)MUSIC_FADEIN_DELAY;
                  soundLock();
                  alSourcef( music_source, AL_GAIN, gain*music_vol );
                  /* Check for errors. */
                  al_checkErr();
                  soundUnlock();
               }
               /* No need to fade anymore. */
               else {
                  /* Set volume to normal level. */
                  soundLock();
                  alSourcef( music_source, AL_GAIN, music_vol );
                  /* Check for errors. */
                  al_checkErr();
                  soundUnlock();

                  /* Change state to playing. */
                  musicLock();
                  music_state = MUSIC_STATE_PLAYING;
                  musicUnlock();
               }
            }
            else if (cur_state == MUSIC_STATE_FADEOUT) {
               if (fade < MUSIC_FADEOUT_DELAY) {
                  gain = 1. - (ALfloat)fade / (ALfloat)MUSIC_FADEOUT_DELAY;
                  soundLock();
                  alSourcef( music_source, AL_GAIN, gain*music_vol );
                  /* Check for errors. */
                  al_checkErr();
                  soundUnlock();
               }
               else {
                  /* Music should stop. */
                  musicLock();
                  music_state = MUSIC_STATE_STOPPING;
                  musicUnlock();
                  break;
               }
            }

            /* Purpose fallthrough. */

         /* Play the song if needed. */
         case MUSIC_STATE_PLAYING:
            /* Special case where file has ended. */
            if (active < 0) {
               soundLock();
               alGetSourcei( music_source, AL_SOURCE_STATE, &state );

               if (state == AL_STOPPED) {
                  alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &value );
                  if (value > 0)
                     alSourceUnqueueBuffers( music_source, value, removed );
                  soundUnlock();

                  musicLock();
                  music_state = MUSIC_STATE_IDLE;
                  if (!music_forced)
                     music_rechoose();
                  musicUnlock();
                  break;
               }

               soundUnlock();

               break;
            }

            soundLock();

            /* See if needs another buffer set. */
            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &state );
            if (state > 0) {
               /* refill active buffer */
               alSourceUnqueueBuffers( music_source, 1, removed );
               ret = stream_loadBuffer( music_buffer[active] );
               if (ret < 0)
                  active = -1;
               else {
                  alSourceQueueBuffers( music_source, 1, &music_buffer[active] );
                  active = 1 - active;
               }
            }

            /* Check for errors. */
            al_checkErr();

            soundUnlock();
      }

      /* Global thread delay. */
      SDL_Delay(0);

   }

   return 0;
}


#ifdef HAVE_OV_READ_FILTER
/**
 * @brief This is the filter function for the decoded Ogg Vorbis stream.
 *
 * base on:
 * vgfilter.c (c) 2007,2008 William Poetra Yoga Hadisoeseno
 * based on:
 * vgplay.c 1.0 (c) 2003 John Morton
 */
static void rg_filter( float **pcm, long channels, long samples, void *filter_param )
{
   int i, j;
   float cur_sample;
   alMusic *param       = filter_param;
   float scale_factor   = param->rg_scale_factor;
   float max_scale      = param->rg_max_scale;

   /* Apply the gain, and any limiting necessary */
   if (scale_factor > max_scale) {
      for(i = 0; i < channels; i++)
         for(j = 0; j < samples; j++) {
            cur_sample = pcm[i][j] * scale_factor;
            /*
             * This is essentially the scaled hard-limiting algorithm
             * It looks like the soft-knee to me
             * I haven't found a better limiting algorithm yet...
             */
            if (cur_sample < -0.5)
               cur_sample = tanh((cur_sample + 0.5) / (1-0.5)) * (1-0.5) - 0.5;
            else if (cur_sample > 0.5)
               cur_sample = tanh((cur_sample - 0.5) / (1-0.5)) * (1-0.5) + 0.5;
            pcm[i][j] = cur_sample;
         }
   }
   else if (scale_factor > 0.0)
      for(i = 0; i < channels; i++)
         for(j = 0; j < samples; j++)
            pcm[i][j] *= scale_factor;
}
#endif /* HAVE_OV_READ_FILTER */


/**
 * @brief Loads a buffer.
 *
 *    @param buffer Buffer to load.
 */
static int stream_loadBuffer( ALuint buffer )
{
   int ret, size, section, result;

   musicVorbisLock();

   /* Make sure music is valid. */
   if (music_vorbis.rw == NULL) {
      musicVorbisUnlock();
      return -1;
   }

   ret  = 0;
   size = 0;
   while (size < music_bufSize) { /* file up the entire data buffer */

#ifdef HAVE_OV_READ_FILTER
      result = ov_read_filter(
            &music_vorbis.stream,   /* stream */
            &music_buf[size],       /* data */
            music_bufSize - size,   /* amount to read */
            VORBIS_ENDIAN,          /* big endian? */
            2,                      /* 16 bit */
            1,                      /* signed */
            &section,               /* current bitstream */
            rg_filter,              /* filter function */
            &music_vorbis );        /* filter parameter */
#else /* HAVE_OV_READ_FILTER */
      result = ov_read(
            &music_vorbis.stream,   /* stream */
            &music_buf[size],       /* data */
            music_bufSize - size,   /* amount to read */
            VORBIS_ENDIAN,          /* big endian? */
            2,                      /* 16 bit */
            1,                      /* signed */
            &section );             /* current bitstream */
#endif /* HAVE_OV_READ_FILTER */

      /* End of file. */
      if (result == 0) {
         if (size == 0) {
            musicVorbisUnlock();
            return -2;
         }
         ret = 1;
         break;
      }
      /* Hole error. */
      else if (result == OV_HOLE) {
         musicVorbisUnlock();
         WARN("OGG: Vorbis hole detected in music!");
         return 0;
      }
      /* Bad link error. */
      else if (result == OV_EBADLINK) {
         musicVorbisUnlock();
         WARN("OGG: Invalid stream section or corrupt link in music!");
         return -1;
      }

      size += result;
   }

   musicVorbisUnlock();

   /* load the buffer up */
   soundLock();
   alBufferData( buffer, music_vorbis.format,
         music_buf, size, music_vorbis.info->rate );
   soundUnlock();

   return ret;
}


/**
 * @brief Initializes the OpenAL music subsystem.
 */
int music_al_init (void)
{
   ALfloat v[] = { 0., 0., 0. };

   /* Create threading mechanisms. */
   music_state_cond  = SDL_CreateCond();
   music_state_lock  = SDL_CreateMutex();
   music_vorbis_lock = SDL_CreateMutex();
   music_vorbis.rw   = NULL; /* indication it's not loaded */

   /* Create the buffer. */
   music_bufSize     = conf.al_bufsize * 1024;
   music_buf         = malloc( music_bufSize );

   soundLock();

   /* music_source created in sound_al_init. */

   /* Generate buffers and sources. */
   alGenBuffers( 2, music_buffer );

   /* Set up OpenAL properties. */
   alSourcef(  music_source, AL_GAIN, music_vol );
   alSourcei(  music_source, AL_SOURCE_RELATIVE, AL_TRUE );
   alSourcefv( music_source, AL_POSITION, v );
   alSourcefv( music_source, AL_VELOCITY, v );

   /* Check for errors. */
   al_checkErr();

   /* Set state to none. */
   music_state = 0;

   soundUnlock();

   /*
    * Start up thread and have it inform us when it already reaches the main loop.
    */
   musicLock();
   music_state  = MUSIC_STATE_STARTUP;
   music_player = SDL_CreateThread( music_thread,
#if SDL_VERSION_ATLEAST(1,3,0)
         "music_thread",
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
         NULL );
   SDL_CondWait( music_state_cond, music_state_lock );
   musicUnlock();

   return 0;
}

/**
 * @brief Frees the music.
 */
void music_al_exit (void)
{
   /* Kill the thread. */
   music_kill();

   soundLock();

   /* Free the music. */
   alDeleteBuffers( 2, music_buffer );
   alDeleteSources( 1, &music_source );

   /* Check for errors. */
   al_checkErr();

   soundUnlock();

   /* Free the buffer. */
   if (music_buf != NULL)
      free(music_buf);
   music_buf = NULL;

   /* Destroy the mutex. */
   SDL_DestroyMutex( music_vorbis_lock );
   SDL_DestroyMutex( music_state_lock );
   SDL_DestroyCond( music_state_cond );
}


/**
 * @brief Internal music loading routines.
 */
int music_al_load( const char* name, SDL_RWops *rw )
{
   int rg;
   ALfloat track_gain_db, track_peak;
   vorbis_comment *vc;
   char *tag = NULL;

   musicVorbisLock();

   /* set the new name */
   strncpy( music_vorbis.name, name, 64 );

   /* Load new ogg. */
   music_vorbis.rw = rw;
   if (ov_open_callbacks( music_vorbis.rw, &music_vorbis.stream,
            NULL, 0, sound_al_ovcall ) < 0) {
      WARN("Song '%s' does not appear to be a vorbis bitstream.", name);
      musicUnlock();
      return -1;
   }
   music_vorbis.info = ov_info( &music_vorbis.stream, -1 );

   /* Get Replaygain information. */
   vc             = ov_comment( &music_vorbis.stream, -1 );
   track_gain_db  = 0.;
   track_peak     = 1.;
   rg             = 0;
   if ((tag = vorbis_comment_query(vc, "replaygain_track_gain", 0))) {
      track_gain_db  = atof(tag);
      rg             = 1;
   }
   if ((tag = vorbis_comment_query(vc, "replaygain_track_peak", 0))) {
      track_peak     = atof(tag);
      rg             = 1;
   }
   music_vorbis.rg_scale_factor = pow(10.0, (track_gain_db + RG_PREAMP_DB)/20);
   music_vorbis.rg_max_scale = 1.0 / track_peak;
   if (!rg)
      DEBUG("Song '%s' has no replaygain information.", name );

   /* Set the format */
   if (music_vorbis.info->channels == 1)
      music_vorbis.format = AL_FORMAT_MONO16;
   else
      music_vorbis.format = AL_FORMAT_STEREO16;

   musicVorbisUnlock();

   return 0;
}


/**
 * @brief Frees the music.
 */
void music_al_free (void)
{
   /* Stop music if needed. */
   musicLock();
   if (music_state != MUSIC_STATE_IDLE) {
      music_command = MUSIC_CMD_STOP;
      music_forced  = 1;
      while (1) {
         SDL_CondWait( music_state_cond, music_state_lock );
         if (music_state == MUSIC_STATE_IDLE) {
            music_forced = 0;
            break;
         }
      }
   }
   musicUnlock();

   musicVorbisLock();

   if (music_vorbis.rw != NULL) {
      ov_clear( &music_vorbis.stream );
      music_vorbis.rw = NULL; /* somewhat officially ended */
   }

   musicVorbisUnlock();
}


/**
 * @brief Sets the volume.
 */
int music_al_volume( double vol )
{
   soundLock();

   music_vol_lin = vol;
   if (vol > 0.) /* Floor of -48 dB (0.00390625 amplitude) */
      music_vol = 1 / pow(2, (1 - vol) * 8 );
   else
      music_vol = 0.;

   /* only needed if playing */
   if (music_al_isPlaying()) {

      alSourcef( music_source, AL_GAIN, music_vol );

      /* Check for errors. */
      al_checkErr();
   }

   soundUnlock();

   return 0;
}


/**
 * @brief Gets the volume (linear).
 */
double music_al_getVolume (void)
{
   return music_vol_lin;
}


/**
 * @brief Gets the volume (logarithmic).
 */
double music_al_getVolumeLog(void)
{
   return music_vol;
}


/**
 * @brief Tells the music thread to play.
 */
void music_al_play (void)
{
   musicLock();

   music_command = MUSIC_CMD_FADEIN;
   while (1) {
      SDL_CondWait( music_state_cond, music_state_lock );
      if (music_isPlaying())
         break;
   }

   musicUnlock();
}


/**
 * @brief Tells the music thread to stop playing.
 */
void music_al_stop (void)
{
   musicLock();

   music_command = MUSIC_CMD_FADEOUT;
   while (1) {
      SDL_CondWait( music_state_cond, music_state_lock );
      if ((music_state == MUSIC_STATE_IDLE) ||
            (music_state == MUSIC_STATE_FADEOUT))
         break;
   }

   musicUnlock();
}


/**
 * @brief Tells the music thread to pause.
 */
void music_al_pause (void)
{
   musicLock();

   music_command = MUSIC_CMD_PAUSE;
   while (1) {
      SDL_CondWait( music_state_cond, music_state_lock );
      if ((music_state == MUSIC_STATE_IDLE) ||
            (music_state == MUSIC_STATE_PAUSED))
         break;
   }

   musicUnlock();
}


/**
 * @brief Tells the music thread to resume.
 */
void music_al_resume (void)
{
   musicLock();

   music_command = MUSIC_CMD_PLAY;
   while (1) {
      SDL_CondWait( music_state_cond, music_state_lock );
      if (music_isPlaying())
         break;
   }

   musicUnlock();
}


/**
 * @brief Tells the music to seek to a position.
 */
void music_al_setPos( double sec )
{
   int ret;

   musicVorbisLock();

   ret = 0;
   if (music_vorbis.rw != NULL)
      ret = ov_time_seek( &music_vorbis.stream, sec );

   musicVorbisUnlock();

   if (ret != 0)
      WARN("Unable to seek vorbis file.");
}


/**
 * @brief Checks to see if the music is playing.
 */
int music_al_isPlaying (void)
{
   int ret;

   musicLock();

   if ((music_state == MUSIC_STATE_PLAYING) ||
         (music_state == MUSIC_STATE_LOADING) ||
         (music_state == MUSIC_STATE_RESUMING) ||
         (music_state == MUSIC_STATE_FADEIN) ||
         (music_state == MUSIC_STATE_FADEOUT) ||
         (music_state == MUSIC_STATE_PAUSED))
      ret = 1;
   else
      ret = 0;

   musicUnlock();

   return ret;
}


/**
 * @brief Tells the music thread to die.
 */
static void music_kill (void)
{
   int ret;
   musicLock();

   music_command = MUSIC_CMD_KILL;
   music_forced  = 1;
   while (1) {
      ret = SDL_CondWaitTimeout( music_state_cond, music_state_lock, 3000 );

      /* Timed out, just slaughter the thread. */
      if (ret == SDL_MUTEX_TIMEDOUT) {
#if SDL_VERSION_ATLEAST(2,0,0)
         WARN("Music thread did not exit when asked, ignoring...");
#else /* SDL_VERSION_ATLEAST(2,0,0) */
         WARN("Music thread did not exit when asked, slaughtering...");
         SDL_KillThread( music_player );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
         break;
      }

      /* Ended properly, breaking. */
      if (music_state == MUSIC_STATE_DEAD)
         break;
   }

   musicUnlock();
}

#endif /* USE_OPENAL */
