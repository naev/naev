/*
 * See Licensing and Copyright notice in naev.h
 */

#if USE_OPENAL


#include "music_openal.h"

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_rwops.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include "music.h"
#include "sound_openal.h"
#include "naev.h"
#include "log.h"
#include "pack.h"


#define BUFFER_SIZE        (4096*8)

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
   MUSIC_CMD_PAUSE
} music_cmd_t;


typedef enum music_state_e {
   MUSIC_STATE_DEAD,
   MUSIC_STATE_STARTUP,
   MUSIC_STATE_IDLE,
   MUSIC_STATE_PLAYING,
   MUSIC_STATE_PAUSED,
   /* Internal usage. */
   MUSIC_STATE_LOADING,
   MUSIC_STATE_STOPPING,
   MUSIC_STATE_PAUSING,
   MUSIC_STATE_RESUMING
} music_state_t;


static SDL_Thread *music_player = NULL;

/*
 * Locks.
 */
extern SDL_mutex *sound_lock; /**< Global sound lock, used for all OpenAL calls. */
static SDL_mutex *music_vorbis_lock = NULL; /**< Lock for vorbisfile operations. */
static SDL_cond  *music_state_cond = NULL; /**< Cond for thread to signal status updates. */
static SDL_mutex *music_state_lock = NULL; /**< Lock for music state. */
static music_cmd_t   music_command = MUSIC_CMD_NONE; /**< Target music state. */
static music_state_t music_state = MUSIC_STATE_DEAD; /**< Current music state. */


/*
 * saves the music to ram in this structure
 */
typedef struct alMusic_ {
   char name[64]; /* name */
   SDL_RWops *rw;
   OggVorbis_File stream;
   vorbis_info* info;
   ALenum format;
} alMusic;


/*
 * song currently playing
 */
static alMusic music_vorbis; /**< Current music. */
static ALuint music_buffer[2]; /**< Front and back buffer. */
ALuint music_source                    = 0; /**< Source assosciated to music. */


/*
 * volume
 */
static ALfloat music_vol = 1.; /**< Current volume level. */


/*
 * prototypes
 */
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
   int active; /* active buffer */
   ALint state;
   ALenum value;
   music_state_t cur_state;

   while (1) {

      /* Handle states. */
      musicLock();

      /* Handle new command. */
      switch (music_command) {
         case MUSIC_CMD_KILL:
            if (music_state != MUSIC_STATE_IDLE)
               music_state = MUSIC_STATE_STOPPING;
            else {
               music_state = MUSIC_STATE_DEAD;
            }
            /* Does not clear command. */
            break;

         case MUSIC_CMD_STOP:
            /* Notify of stopped. */
            if (music_state == MUSIC_STATE_IDLE)
               SDL_CondBroadcast( music_state_cond );
            else
               music_state = MUSIC_STATE_STOPPING;
            music_command = MUSIC_CMD_NONE;
            break;

         case MUSIC_CMD_PLAY:
            if (music_state == MUSIC_STATE_PAUSING)
               music_state = MUSIC_STATE_RESUMING;
            else
               music_state = MUSIC_STATE_LOADING;
            music_command = MUSIC_CMD_NONE;
            break;

         case MUSIC_CMD_PAUSE:
            music_state = MUSIC_STATE_PAUSING;
            music_command = MUSIC_CMD_NONE;
            break;

         case MUSIC_CMD_NONE:
            break;
      }
      cur_state = music_state;
      musicUnlock();


      /*
       * Main processing loop.
       */
      switch (cur_state) {
         /*
          * Basically send a message that thread is up and running.
          */
         case MUSIC_STATE_STARTUP:
            musicLock();
            music_state = MUSIC_STATE_IDLE;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /*
          * We died.
          */
         case MUSIC_STATE_DEAD:
            return 0;
            break;

         /*
          * Delays at the end.
          */
         case MUSIC_STATE_PAUSED:
         case MUSIC_STATE_IDLE:
            break;

         /*
          * Resumes the paused song.
          */
         case MUSIC_STATE_RESUMING:
            soundLock();
            alSourcePlay( music_source );
            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_PLAYING;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /*
          * Pause the song.
          */
         case MUSIC_STATE_PAUSING:
            soundLock();
            alSourcePause( music_source );
            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_PAUSED;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /*
          * Stop song setting to IDLE.
          */
         case MUSIC_STATE_STOPPING:
            soundLock();

            /* Stop and remove buffers. */
            alSourceStop( music_source );
            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &value );
            if (value > 0)
               alSourceUnqueueBuffers( music_source, value, music_buffer );

            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_IDLE;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /*
          * Load the song.
          */
         case MUSIC_STATE_LOADING:
            soundLock();

            /* Load buffer and start playing. */
            active = 0; /* load first buffer */
            ret = stream_loadBuffer( music_buffer[active] );
            alSourceQueueBuffers( music_source, 1, &music_buffer[active] );
            /* Special case NULL file or error. */
            if (ret < 0) {
               soundUnlock();
               /* Force state to stopped. */
               musicLock();
               music_state = MUSIC_STATE_IDLE;
               SDL_CondBroadcast( music_state_cond );
               musicUnlock();
               break;
            }
            /* Start playing. */
            alSourcePlay( music_source );
            /* Special case of a very short song. */
            if (ret > 1) {
               active = -1;
               soundUnlock();

               /* Check for errors. */
               al_checkErr();

               musicLock();
               music_state = MUSIC_STATE_PLAYING;
               SDL_CondBroadcast( music_state_cond );
               musicUnlock();
               break;
            }

            /* Load second buffer. */
            active = 1;
            ret = stream_loadBuffer( music_buffer[active] );
            if (ret < 0) {
               active = -1;
            }
            else {
               alSourceQueueBuffers( music_source, 1, &music_buffer[active] );
               active = 1 - active;
            }

            /* Check for errors. */
            al_checkErr();

            soundUnlock();

            musicLock();
            music_state = MUSIC_STATE_PLAYING;
            SDL_CondBroadcast( music_state_cond );
            musicUnlock();
            break;

         /*
          * Play the song if needed.
          */
         case MUSIC_STATE_PLAYING:

            /* Special case where file has ended. */
            if (active < 0) {
               soundLock();
               alGetSourcei( music_source, AL_SOURCE_STATE, &state );
               soundUnlock();

               if (state == AL_STOPPED) {
                  musicLock();
                  music_state = MUSIC_STATE_IDLE;
                  music_rechoose();
                  musicUnlock();
                  break;
               }

               break;
            }

            soundLock();

            /* See if needs another buffer set. */
            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &state );
            if (state > 0) {

               /* refill active buffer */
               alSourceUnqueueBuffers( music_source, 1, &music_buffer[active] );
               ret = stream_loadBuffer( music_buffer[active] );
               if (ret < 0) {
                  active = -1;
               }
               else {
                  alSourceQueueBuffers( music_source, 1, &music_buffer[active] );
                  active = 1 - active;
               }
            }

            /* Check for errors. */
            al_checkErr();

            soundUnlock();
      }

      /*
       * Global thread delay.
       */
      SDL_Delay(0);

   }

   return 0;
}



/**
 * @brief Loads a buffer.
 *
 *    @param buffer Buffer to load.
 */
static int stream_loadBuffer( ALuint buffer )
{
   int ret, size, section, result;
   char dat[BUFFER_SIZE]; /* buffer to hold the data */

   musicVorbisLock();

   ret  = 0;
   size = 0;
   while (size < BUFFER_SIZE) { /* fille up the entire data buffer */

      result = ov_read( &music_vorbis.stream, /* stream */
            &dat[size],             /* data */
            BUFFER_SIZE - size,     /* amount to read */
            VORBIS_ENDIAN,          /* big endian? */
            2,                      /* 16 bit */
            1,                      /* signed */
            &section );             /* current bitstream */

      /* End of file. */
      if (result == 0) {
         if (size == 0)
            return -2;
         ret = 1;
         break;
      }
      /* Hole error. */
      else if (result == OV_HOLE) {
         WARN("OGG: Vorbis hole detected in music!");
         return 0;
      }
      /* Bad link error. */
      else if (result == OV_EBADLINK) {
         WARN("OGG: Invalid stream section or corrupt link in music!");
         return -1;
      }

      size += result;
   }

   musicVorbisUnlock();

   /* load the buffer up */
   alBufferData( buffer, music_vorbis.format,
         dat, size, music_vorbis.info->rate );

   return ret;
}


/**
 * @brief Initializes the OpenAL music subsystem.
 */
int music_al_init (void)
{
   music_state_cond  = SDL_CreateCond();
   music_state_lock  = SDL_CreateMutex();
   music_vorbis_lock = SDL_CreateMutex();
   music_vorbis.rw = NULL; /* indication it's not loaded */

   soundLock();

   /* music_source created in sound_al_init. */

   /* Generate buffers and sources. */
   alGenBuffers( 2, music_buffer );

   /* Set up OpenAL properties. */
   alSourcef( music_source, AL_GAIN, music_vol );
   alSourcef( music_source, AL_ROLLOFF_FACTOR, 0. );
   alSourcei( music_source, AL_SOURCE_RELATIVE, AL_FALSE );

   /* Check for errors. */
   al_checkErr();

   /* Set state to none. */
   music_state = 0;

   soundUnlock();

   /*
    * Start up thread and have it inform us when it already reaches the main loop.
    */
   musicLock();
   music_state = MUSIC_STATE_STARTUP;
   music_player = SDL_CreateThread( music_thread, NULL );
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
   music_stop();
   music_kill();
   SDL_WaitThread( music_player, NULL );

   soundLock();

   /* Free the music. */
   alDeleteBuffers( 2, music_buffer );
   alDeleteSources( 1, &music_source );

   soundUnlock();

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
      while (1) {
         SDL_CondWait( music_state_cond, music_state_lock );
         if (music_state == MUSIC_STATE_IDLE)
            break;
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
   music_vol = vol;

   /* only needed if playing */
   if (music_al_isPlaying()) {
      soundLock();

      alSourcef( music_source, AL_GAIN, vol );

      /* Check for errors. */
      al_checkErr();
 
      soundUnlock();
   }

   return 0;
}


/**
 * @brief Gets the volume.
 */
double music_al_getVolume (void)
{
   return music_vol;
}


/**
 * @brief Tells the music thread to play.
 */
void music_al_play (void)
{
   musicLock();

   music_command = MUSIC_CMD_PLAY;

   musicUnlock();
}


/**
 * @brief Tells the music thread to stop playing.
 */
void music_al_stop (void)
{
   musicLock();

   music_command = MUSIC_CMD_STOP;

   musicUnlock();
}


/**
 * @brief Tells the music thread to pause.
 */
void music_al_pause (void)
{
   musicLock();

   music_command = MUSIC_CMD_PAUSE;

   musicUnlock();
}


/**
 * @brief Tells the music thread to resume.
 */
void music_al_resume (void)
{
   musicLock();

   music_command = MUSIC_CMD_PLAY;

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

   if (ret != 0) {
      WARN("Unable to seek vorbis file.");
   }
}


/**
 * @brief Checks to see if the music is playing.
 */
int music_al_isPlaying (void)
{
   int ret;

   musicLock();

   if (music_state == MUSIC_STATE_PLAYING)
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
   musicLock();

   music_command = MUSIC_CMD_KILL;

   musicUnlock();
}

#endif /* USE_OPENAL */
