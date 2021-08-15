/*
 * See Licensing and Copyright notice in naev.h
 */

/** @cond */
#include <math.h>
#include <vorbis/vorbisfile.h>
#include "SDL.h"
#include "SDL_rwops.h"
#include "SDL_thread.h"

#include "naev.h"
/** @endcond */

#include "music_openal.h"

#include "conf.h"
#include "log.h"
#include "music.h"
#include "sound_openal.h"


/**
 * @brief Default pre-amp in dB.
 */
#define RG_PREAMP_DB       0.0


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
} music_state_t;


static SDL_Thread *music_player = NULL; /**< Music player thread. */

/*
 * Playing buffers.
 */
static const int music_bufSize      = SOUND_BUFFER_SIZE*1024; /**< Size of music playing buffer. */
static char *music_buf              = NULL; /**< Music playing buffer. */


/*
 * Locks.
 */
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
   SDL_RWops *rw; /**< RWops file reading from. */
   OggVorbis_File stream; /**< Vorbis file stream. */
   vorbis_info* info; /**< Information of the stream. */
   ALenum format; /**< Stream format. */
   /* Replaygain information. */
   ALfloat rg_scale_factor; /**< Scale factor. */
   ALfloat rg_max_scale; /**< Maximum scale factor before clipping. */
} alMusic;


typedef struct MusicData_s {
   music_state_t state;
   int active; /* active buffer */
   ALint alstate;
   ALuint removed[2];
   ALenum value;
   ALfloat gain;
   int fadein_start;
   uint32_t fade, fade_timer;
} MusicData;


/*
 * song currently playing
 */
static alMusic music_vorbis; /**< Current music. */
static ALuint music_buffer[2]; /**< Front and back buffer. */
static ALuint music_source = 0; /**< Source associated to music. */


/*
 * volume
 */
static ALfloat music_vol     = 1.; /**< Current volume level (logarithmic). */
static ALfloat music_vol_lin = 1.; /**< Current volume level (linear). */


/*
 * prototypes
 */
static void rg_filter( float **pcm, long channels, long samples, void *filter_param );
static void music_kill (void);
static int music_thread( void* unused );
static int stream_loadBuffer( ALuint buffer );


/*
 * Internal stuff.
 */
static int mal_stop( MusicData *m )
{
   /* Notify of stopped. */
   if (music_state == MUSIC_STATE_IDLE)
      return 0;
   else {
      soundLock();

      /* Stop and remove buffers. */
      alSourceStop( music_source );
      alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &m->value );
      if (m->value > 0)
         alSourceUnqueueBuffers( music_source, m->value, m->removed );
      /* Clear timer. */
      m->fade_timer = 0;

      /* Reset volume. */
      alSourcef( music_source, AL_GAIN, music_vol );

      al_checkErr();
      soundUnlock();

      music_state = MUSIC_STATE_IDLE;
      if (!music_forced)
         music_rechoose();
   }
   return 0;
}
static int mal_load( MusicData *m )
{
   int ret;

   /* Load buffer and start playing. */
   m->active = 0; /* load first buffer */
   ret = stream_loadBuffer( music_buffer[m->active] );
   soundLock();
   alSourceQueueBuffers( music_source, 1, &music_buffer[m->active] );

   /* Special case NULL file or error. */
   if (ret < 0) {
      soundUnlock();
      return -1;
   }
   /* Force volume level. */
   alSourcef( music_source, AL_GAIN, (m->fadein_start) ? 0. : music_vol );

   /* Start playing. */
   alSourcePlay( music_source );

   /* Check for errors. */
   al_checkErr();

   soundUnlock();
   /* Special case of a very short song. */
   if (ret > 1) {
      m->active = -1;
      return 0;
   }

   /* Load second buffer. */
   m->active = 1;
   ret = stream_loadBuffer( music_buffer[m->active] );
   if (ret < 0)
      m->active = -1;
   else {
      soundLock();
      alSourceQueueBuffers( music_source, 1, &music_buffer[m->active] );
      /* Check for errors. */
      al_checkErr();
      soundUnlock();
      m->active = 1 - m->active;
   }
   return 0;
}
static int mal_play( MusicData *m )
{
   /* Set appropriate state. */
   if (music_state == MUSIC_STATE_PAUSED) {
      soundLock();
      alSourcePlay( music_source );
      alSourcef( music_source, AL_GAIN, music_vol );
      /* Check for errors. */
      al_checkErr();
      soundUnlock();
   }
   else if (music_state == MUSIC_STATE_FADEIN)
      m->fade_timer = SDL_GetTicks() - MUSIC_FADEIN_DELAY;
   else
      mal_load( m );
   /* Disable fadein. */
   m->fadein_start = 0;
   music_state = MUSIC_STATE_PLAYING;
   return 0;
}
static int mal_fadeout( MusicData *m )
{
   if (music_state == MUSIC_STATE_IDLE)
      return 0;
   /* Set timer. */
   music_state = MUSIC_STATE_FADEOUT;
   m->fade_timer = SDL_GetTicks();
   music_state = MUSIC_STATE_FADEOUT;
   return 0;
}
static int mal_fadein( MusicData *m )
{
   if ((music_state == MUSIC_STATE_FADEIN) ||
         (music_state == MUSIC_STATE_PLAYING))
      return 0;
   mal_load( m );
   /* Set timer. */
   m->fade_timer = SDL_GetTicks();
   m->fadein_start = 1;
   music_state = MUSIC_STATE_FADEIN;
   return 0;
}
static int mal_pause( MusicData *m )
{
   (void) m;
   if ((music_state == MUSIC_STATE_PLAYING) ||
         (music_state == MUSIC_STATE_FADEIN)) {
      soundLock();
      alSourcePause( music_source );
      /* Check for errors. */
      al_checkErr();
      soundUnlock();

      music_state = MUSIC_STATE_PAUSED;
   }
   return 0;
}


/**
 * @brief The music thread.
 *
 *    @param unused Unused.
 */
static int music_thread( void* unused )
{
   (void)unused;

   int ret;
   MusicData m;
   memset( &m, 0, sizeof(MusicData) );

   while (1) {
      /* Handle states. */
      musicLock();

      /* Handle new command. */
      switch (music_command) {
         case MUSIC_CMD_KILL:
            mal_stop( &m );
            music_state = MUSIC_STATE_DEAD;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            return 0;

         case MUSIC_CMD_STOP:    mal_stop( &m ); break;
         case MUSIC_CMD_PLAY:    mal_play( &m ); break;
         case MUSIC_CMD_FADEOUT: mal_fadeout( &m ); break;
         case MUSIC_CMD_FADEIN:  mal_fadein( &m ); break;
         case MUSIC_CMD_PAUSE:   mal_pause( &m ); break;

         case MUSIC_CMD_NONE:
            break;
      }
      m.state = music_state;
      if (music_command != MUSIC_CMD_NONE) {
         music_command = MUSIC_CMD_NONE;
         SDL_CondBroadcast( music_state_cond );
      }

      musicUnlock();

      /* Main processing loop. */
      switch (m.state) {
         /* Basically send a message that thread is up and running. */
         case MUSIC_STATE_STARTUP:
            musicLock();
            music_state = MUSIC_STATE_IDLE;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /* Delays at the end. */
         case MUSIC_STATE_DEAD:
         case MUSIC_STATE_PAUSED:
         case MUSIC_STATE_IDLE:
            break;

         /* Fades in the music. */
         case MUSIC_STATE_FADEOUT:
         case MUSIC_STATE_FADEIN:
            /* See if must still fade. */
            m.fade = SDL_GetTicks() - m.fade_timer;
            if (m.state == MUSIC_STATE_FADEIN) {
               if (m.fade < MUSIC_FADEIN_DELAY) {
                  m.gain = (ALfloat)m.fade / (ALfloat)MUSIC_FADEIN_DELAY;
                  soundLock();
                  alSourcef( music_source, AL_GAIN, m.gain*music_vol );
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
            else if (m.state == MUSIC_STATE_FADEOUT) {
               if (m.fade < MUSIC_FADEOUT_DELAY) {
                  m.gain = 1. - (ALfloat)m.fade / (ALfloat)MUSIC_FADEOUT_DELAY;
                  soundLock();
                  alSourcef( music_source, AL_GAIN, m.gain*music_vol );
                  /* Check for errors. */
                  al_checkErr();
                  soundUnlock();
               }
               else {
                  mal_stop( &m );
                  break;
               }
            }
            /* fallthrough */

         /* Play the song if needed. */
         case MUSIC_STATE_PLAYING:
            /* Special case where file has ended. */
            if (m.active < 0) {
               soundLock();
               alGetSourcei( music_source, AL_SOURCE_STATE, &m.alstate );

               if (m.alstate == AL_STOPPED) {
                  alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &m.value );
                  if (m.value > 0)
                     alSourceUnqueueBuffers( music_source, m.value, m.removed );
                  al_checkErr();
                  soundUnlock();

                  musicLock();
                  music_state = MUSIC_STATE_IDLE;
                  if (!music_forced)
                     music_rechoose();
                  musicUnlock();
                  break;
               }

               al_checkErr();
               soundUnlock();

               break;
            }

            al_checkErr();
            soundLock();

            /* See if needs another buffer set. */
            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &m.alstate );
            if (m.alstate > 0) {
               /* refill active buffer */
               alSourceUnqueueBuffers( music_source, 1, m.removed );
               ret = stream_loadBuffer( music_buffer[m.active] );
               if (ret < 0)
                  m.active = -1;
               else {
                  alSourceQueueBuffers( music_source, 1, &music_buffer[m.active] );
                  m.active = 1 - m.active;
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
      for (i = 0; i < channels; i++)
         for (j = 0; j < samples; j++) {
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
      for (i = 0; i < channels; i++)
         for (j = 0; j < samples; j++)
            pcm[i][j] *= scale_factor;
}


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
      result = ov_read_filter(
            &music_vorbis.stream,   /* stream */
            &music_buf[size],       /* data */
            music_bufSize - size,   /* amount to read */
            (SDL_BYTEORDER == SDL_BIG_ENDIAN),
            2,                      /* 16 bit */
            1,                      /* signed */
            &section,               /* current bitstream */
            rg_filter,              /* filter function */
            &music_vorbis );        /* filter parameter */

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
         WARN(_("OGG: Vorbis hole detected in music!"));
         return 0;
      }
      /* Bad link error. */
      else if (result == OV_EBADLINK) {
         musicVorbisUnlock();
         WARN(_("OGG: Invalid stream section or corrupt link in music!"));
         return -1;
      }

      size += result;
   }

   musicVorbisUnlock();

   /* load the buffer up */
   soundLock();
   alBufferData( buffer, music_vorbis.format,
         music_buf, size, music_vorbis.info->rate );
   al_checkErr();
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
   music_buf         = malloc( music_bufSize );

   soundLock();

   /* Allocate source for music. */
   alGenSources( 1, &music_source );

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
         "music_thread",
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

   /* Load new ogg. */
   music_vorbis.rw = rw;
   if (ov_open_callbacks( music_vorbis.rw, &music_vorbis.stream,
            NULL, 0, sound_al_ovcall ) < 0) {
      WARN(_("Song '%s' does not appear to be a Vorbis bitstream."), name);
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
      DEBUG(_("Song '%s' has no replaygain information."), name );

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

      SDL_CondWait( music_state_cond, music_state_lock );
      if (music_state == MUSIC_STATE_IDLE)
         music_forced = 0;
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
   SDL_CondWait( music_state_cond, music_state_lock );

   musicUnlock();
}


/**
 * @brief Tells the music thread to stop playing.
 */
void music_al_stop (void)
{
   musicLock();

   music_command = MUSIC_CMD_FADEOUT;
   SDL_CondWait( music_state_cond, music_state_lock );

   musicUnlock();
}


/**
 * @brief Tells the music thread to pause.
 */
void music_al_pause (void)
{
   musicLock();

   music_command = MUSIC_CMD_PAUSE;
   SDL_CondWait( music_state_cond, music_state_lock );

   musicUnlock();
}


/**
 * @brief Tells the music thread to resume.
 */
void music_al_resume (void)
{
   musicLock();

   music_command = MUSIC_CMD_PLAY;
   SDL_CondWait( music_state_cond, music_state_lock );

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
      WARN(_("Unable to seek Vorbis file."));
}


/**
 * @brief Checks to see if the music is playing.
 */
int music_al_isPlaying (void)
{
   int ret;

   musicLock();

   if ((music_state == MUSIC_STATE_PLAYING) ||
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
   ret = SDL_CondWaitTimeout( music_state_cond, music_state_lock, 3000 );

   /* Timed out, just slaughter the thread. */
   if (ret == SDL_MUTEX_TIMEDOUT)
      WARN(_("Music thread did not exit when asked, ignoring..."));

   musicUnlock();
}
