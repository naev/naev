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
#include "naev.h"
#include "log.h"
#include "pack.h"


#define MUSIC_STOPPED      (1<<1)
#define MUSIC_PLAYING      (1<<2)
#define MUSIC_PAUSED       (1<<3)
#define MUSIC_KILL         (1<<9)
#define music_is(f)        (music_state & (f))
#define music_set(f)       (music_state |= (f))
#define music_rm(f)        (music_state &= ~(f))

#define BUFFER_SIZE        (4096*8)

#define soundLock()        SDL_mutexP(sound_lock)
#define soundUnlock()      SDL_mutexV(sound_lock)

#define musicLock()        SDL_mutexP(music_vorbis_lock)
#define musicUnlock()      SDL_mutexV(music_vorbis_lock)


/*
 * global sound mutex
 */
extern SDL_mutex *sound_lock;
static unsigned int music_state = 0; /**< Current music state. */
static SDL_Thread *music_player = NULL;


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
static SDL_mutex *music_vorbis_lock    = NULL; /**< Music lock. */
static alMusic music_vorbis; /**< Current music. */
static ALuint music_buffer[2]; /**< Front and back buffer. */
static ALuint music_source             = 0; /**< Source assosciated to music. */


/*
 * volume
 */
static ALfloat music_vol = 1.; /**< Current volume level. */


/*
 * vorbis stuff
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
ov_callbacks ovcall = {
   .read_func = ovpack_read,
   .seek_func = ovpack_seek,
   .close_func = ovpack_close,
   .tell_func = ovpack_tell
}; /**< Vorbis call structure to handl rwops. */


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

   int active; /* active buffer */
   ALint state;

   /* main loop */
   while (!music_is(MUSIC_KILL)) {

      if (music_is(MUSIC_PLAYING)) {
         if (music_vorbis.rw == NULL)
            music_rm(MUSIC_PLAYING);
         else {

            music_rm(MUSIC_STOPPED);

            musicLock(); /* lock the mutex */
            soundLock();

            /* start playing current song */
            active = 0; /* load first buffer */
            if (stream_loadBuffer( music_buffer[active] ))
               music_rm(MUSIC_PLAYING);
            alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

            /* start playing with buffer loaded */
            alSourcePlay( music_source );

            active = 1; /* load second buffer */
            if (stream_loadBuffer( music_buffer[active] ))
               music_rm(MUSIC_PLAYING);
            alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

            soundUnlock();

            active = 0; /* dive into loop */
         }
         while (music_is(MUSIC_PLAYING)) {

            soundLock();

            alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &state );
            if (state > 0) {

               /* refill active buffer */
               alSourceUnqueueBuffers( music_source, 1, &music_buffer[active] );
               if (stream_loadBuffer( music_buffer[active] )) music_rm(MUSIC_PLAYING);
               alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

               active = 1 - active;
            }
            
            soundUnlock();

            SDL_Delay(0);
         }

         soundLock();

         alSourceStop( music_source );
         alSourceUnqueueBuffers( music_source, 2, music_buffer );

         soundUnlock();
         musicUnlock();
      }

      music_set(MUSIC_STOPPED);
      SDL_Delay(0); /* we must not kill resources */
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
   int size, section, result;
   char dat[BUFFER_SIZE]; /* buffer to hold the data */

   size = 0;
   while (size < BUFFER_SIZE) { /* fille up the entire data buffer */

      result = ov_read( &music_vorbis.stream, /* stream */
            dat + size,             /* data */
            BUFFER_SIZE - size,     /* amount to read */
            0,                      /* big endian? */
            2,                      /* 16 bit */
            1,                      /* signed */
            &section );             /* current bitstream */

      /* End of file. */
      if (result == 0)
         return 1;
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
   /* load the buffer up */
   alBufferData( buffer, music_vorbis.format,
         dat, BUFFER_SIZE, music_vorbis.info->rate );

   return 0;
}


/*
 * init/exit
 */
int music_al_init (void)
{
   music_vorbis_lock = SDL_CreateMutex();
   music_vorbis.rw = NULL; /* indication it's not loaded */

   soundLock();

   /* Generate buffers and sources. */
   alGenBuffers( 2, music_buffer );
   alGenSources( 1, &music_source );

   /* @brief Set up OpenAL properties. */
   alSourcef( music_source, AL_GAIN, music_vol );
   alSourcef( music_source, AL_ROLLOFF_FACTOR, 0. );
   alSourcei( music_source, AL_SOURCE_RELATIVE, AL_FALSE );

   soundUnlock();

   /* Start up the thread. */
   music_player = SDL_CreateThread( music_thread, NULL );

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

   /* Free the music. */
   alDeleteBuffers( 2, music_buffer );
   alDeleteSources( 1, &music_source );

   /* Destroy the mutex. */
   SDL_DestroyMutex( music_vorbis_lock );
}


/*
 * internal music loading routines
 */
int music_al_load( const char* name, SDL_RWops *rw )
{
   /* Lock the music. */
   musicLock();

   /* set the new name */
   strncpy( music_vorbis.name, name, 64 );
   
   /* Load new ogg. */
   music_vorbis.rw = rw;
   ov_open_callbacks( &music_vorbis.rw, &music_vorbis.stream, NULL, 0, ovcall );
   music_vorbis.info = ov_info( &music_vorbis.stream, -1);

   /* Set the format */
   if (music_vorbis.info->channels == 1)
      music_vorbis.format = AL_FORMAT_MONO16;
   else
      music_vorbis.format = AL_FORMAT_STEREO16;

   musicUnlock();

   return 0;
}


/**
 * @brief Frees the music.
 */
void music_al_free (void)
{
   musicLock();

   /* Stop music if needed. */
   music_al_stop();
   while (!music_is(MUSIC_STOPPED))
      SDL_Delay(0);

   if (music_vorbis.rw != NULL) {
      ov_clear( &music_vorbis.stream );
      music_vorbis.rw = NULL; /* somewhat officially ended */
   }

   musicUnlock();
}


/**
 * @brief Sets the volume.
 */
int music_al_volume( double vol )
{
   music_vol = vol;

   /* only needed if playing */
   if (music_set(MUSIC_PLAYING)) {
      soundLock();

      alSourcef( music_source, AL_GAIN, vol );

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

   if (!music_is(MUSIC_PLAYING))
      music_set(MUSIC_PLAYING);

   musicUnlock();
}


/**
 * @brief Tells the music thread to stop playing.
 */
void music_al_stop (void)
{
   musicLock();

   if (music_is(MUSIC_PLAYING))
      music_rm(MUSIC_PLAYING);

   musicUnlock();
}


/**
 * @brief Tells the music thread to pause.
 */
void music_al_pause (void)
{
   musicLock();

   if (music_is(MUSIC_PLAYING)) {
      music_rm(MUSIC_PLAYING);
      music_set(MUSIC_PAUSED);
   }

   musicUnlock();
}


/**
 * @brief Tells the music thread to resume.
 */
void music_al_resume (void)
{
   musicLock();

   if (music_is(MUSIC_PAUSED)) {
      music_rm(MUSIC_PAUSED);
      music_set(MUSIC_PLAYING);
   }

   musicUnlock();
}


/**
 * @brief Tells the music to seek to a position.
 */
void music_al_setPos( double sec )
{
   (void) sec;
   /** @todo implement seeking. */
}


/**
 * @brief Checks to see if the music is playing.
 */
int music_al_isPlaying (void)
{
   int ret;

   musicLock();

   ret = !!music_is(MUSIC_PLAYING);

   musicUnlock();

   return ret;
}


/**
 * @brief Tells the music thread to die.
 */
static void music_kill (void)
{
   musicLock();

   if (!music_is(MUSIC_KILL))
      music_set(MUSIC_KILL);

   musicUnlock();
}

#endif /* USE_OPENAL */
