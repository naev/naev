/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file sound.c
 *
 * @brief Handles all the sound details.
 */


#include "sound.h"

#include <sys/stat.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mixer.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "music.h"
#include "physics.h"


#define SOUND_CHANNEL_MAX  256 /**< Number of sound channels to allocate. Overkill. */

#define SOUND_PREFIX "snd/sounds/" /**< Prefix of where to find sounds. */
#define SOUND_SUFFIX ".wav" /**< Suffix of sounds. */


/*
 * Global sound properties.
 */
int sound_disabled = 0; /**< Whether sound is disabled. */
double sound_defVolume = 0.4; /**< Sound default volume. */
static int sound_reserved = 0; /**< Amount of reserved channels. */
static double sound_pos[3]; /**< Position of listener. */


/**
 * @struct alSound
 *
 * @brief Contains a sound buffer.
 */
typedef struct alSound_ {
   char *name; /**< Buffer's name. */
   Mix_Chunk *buffer; /**< Buffer data. */
} alSound;


/**
 * @typedef voice_state_t
 * @brief The state of a voice.
 * @sa alVoice
 */
typedef enum voice_state_ {
   VOICE_STOPPED, /**< Voice is stopped. */
   VOICE_PLAYING, /**< Voice is playing. */
   VOICE_DESTROY  /**< Voice should get destroyed asap. */
} voice_state_t;


/**
 * @struct alVoice
 *
 * @brief Represents a voice in the game.
 *
 * A voice would be any object that is creating sound.
 */
typedef struct alVoice_ {
   struct alVoice_ *prev; /**< Linked list previous member. */
   struct alVoice_ *next; /**< Linked list next member. */

   int id; /**< Identifier of the voice. */
   double pos[2]; /**< Position of the voice. */
   int channel; /**< Channel currently in use. */
   unsigned int state; /**< Current state of the sound. */
} alVoice;


/*
 * list of sounds available (all preloaded into a buffer)
 */
static int voice_genid = 0; /**< Voice identifier generator. */
static alSound *sound_list = NULL; /**< List of available sounds. */
static int sound_nlist = 0; /**< Number of available sounds. */


/*
 * voice linked list.
 */
static alVoice *voice_active = NULL; /**< Active voices. */
static alVoice *voice_pool = NULL; /**< Pool of free voices. */


/*
 * prototypes
 */
/* General. */
static void print_MixerVersion (void);
static int sound_makeList (void);
static Mix_Chunk *sound_load( char *filename );
static void sound_free( alSound *snd );
/* Voices. */
static void voice_markStopped( int channel );
static alVoice* voice_new (void);
static int voice_add( alVoice* v );
static alVoice* voice_get( int id );


/**
 * @brief Initializes the sound subsystem.
 *
 *    @return 0 on success.
 */
int sound_init (void)
{
   if (sound_disabled && music_disabled) return 0;

   SDL_InitSubSystem(SDL_INIT_AUDIO);
   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT , 2, 1024) < 0) {
      WARN("Opening Audio: %s", Mix_GetError());
      DEBUG();
      sound_disabled = 1; /* Just disable sound then. */
      music_disabled = 1;
      return -1;
   }
   Mix_AllocateChannels(SOUND_CHANNEL_MAX);

   /* Debug magic. */
   print_MixerVersion();

   if (!sound_disabled) {
      /* load up all the sounds */
      sound_makeList();
      sound_volume(sound_defVolume);

      /* Finish function. */
      Mix_ChannelFinished( voice_markStopped );
   }

   /* Initialize the music */
   music_init();

   return 0;
}

/**
 * @brief Prints the current and compiled SDL_Mixer versions.
 */
static void print_MixerVersion (void)
{
   int frequency;
   Uint16 format;
   int channels;
   SDL_version compiled;
   const SDL_version *linked;
   char device[PATH_MAX];

   /* Query stuff. */
   Mix_QuerySpec(&frequency, &format, &channels);
   MIX_VERSION(&compiled);
   linked = Mix_Linked_Version();
   SDL_AudioDriverName(device, PATH_MAX);

   /* Version itself. */
   DEBUG("SDL_Mixer: %d.%d.%d [compiled: %d.%d.%d]", 
         compiled.major, compiled.minor, compiled.patch,
         linked->major, linked->minor, linked->patch);
   /* Check if major/minor version differ. */
   if ((linked->major*100 + linked->minor) > compiled.major*100 + compiled.minor)
      WARN("SDL_Mixer is newer then compiled version");
   if ((linked->major*100 + linked->minor) < compiled.major*100 + compiled.minor)
      WARN("SDL_Mixer is older then compiled version.");
   /* Print other debug info. */
   DEBUG("Driver: %s",device);
   DEBUG("Format: %d Hz %s", frequency, (channels == 2) ? "Stereo" : "Mono");
   DEBUG();
}


/**
 * @brief Cleans up after the sound subsytem.
 */
void sound_exit (void)
{
   int i;
   alVoice *v;

   /* Close the audio. */
   Mix_CloseAudio();

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

   /* free the sounds */
   for (i=0; i<sound_nlist; i++)
      sound_free( &sound_list[i] );
   free( sound_list );
   sound_list = NULL;
   sound_nlist = 0;

   music_exit();
}


/**
 * @brief Gets the buffer to sound of name.
 *
 *    @param name Name of the sound to get it's id.
 *    @return ID of the sound matching name.
 */
int sound_get( char* name ) 
{
   int i;

   if (sound_disabled) return 0;

   for (i=0; i<sound_nlist; i++)
      if (strcmp(name, sound_list[i].name)==0) {
         return i;
      }
   WARN("Sound '%s' not found in sound list", name);
   return -1;
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

   if (sound_disabled) return 0;

   if ((sound < 0) || (sound > sound_nlist))
      return -1;

   v->channel = Mix_PlayChannel( -1, sound_list[sound].buffer, 0 );
   
   if (v->channel < 0)
      WARN("Unable to play sound: %s", Mix_GetError());

   v->state = VOICE_PLAYING;
   v->id = ++voice_genid;
   voice_add(v);

   return v->id;
}


/**
 * @brief Plays a sound based on position.
 *
 *    @param sound Sound to play.
 *    @param x X position of the sound.
 *    @param y Y position of the sound.
 *    @return 0 on success.
 */
int sound_playPos( int sound, double x, double y )
{
   alVoice *v;
   double angle, dist;
   double px, py;
   int idist;

   if (sound_disabled) return 0;

   if ((sound < 0) || (sound > sound_nlist))
      return -1;

   /* Gets a new voice. */
   v = voice_new();

   v->pos[0] = x;
   v->pos[1] = y;

   px = v->pos[0] - sound_pos[0];
   py = v->pos[1] - sound_pos[1];

   angle = sound_pos[2] - ANGLE(px,py)/M_PI*180.;
   dist = MOD(px,py);

   v->channel = Mix_PlayChannel( -1, sound_list[sound].buffer, 0 );

   if (v->channel < 0) {
      WARN("Unable to play sound: %s", Mix_GetError());
      return -1;
   }

   /* Need to make sure distance doesn't overflow. */
   idist = (int) dist / 13.;
   if (idist > 255) idist = 255;
   if (Mix_SetPosition( v->channel, (Sint16)angle, (Uint8)idist) < 0) {
      WARN("Unable to set sound position: %s", Mix_GetError());
      return -1;
   }

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
int sound_updatePos( int voice, double x, double y )
{
   alVoice *v;
   double angle, dist;
   double px, py;

   if (sound_disabled) return 0;

   v = voice_get(voice);
   if (v != NULL) {
      v->pos[0] = x;
      v->pos[1] = y;

      px = x - sound_pos[0];
      py = y - sound_pos[1];

      angle = sound_pos[2] - ANGLE(px,py)/M_PI*180.;
      dist = MOD(px,py);

      if (Mix_SetPosition( v->channel, (int)angle, (int)dist / 10 ) < 0) {
         WARN("Unable to set sound position: %s", Mix_GetError());
         return -1;
      }
   }

   return 0;
}


/**
 * @brief Updates the sonuds removing obsolete ones and such.
 *
 *    @return 0 on success.
 */
int sound_update (void)
{
   alVoice *v, *tv;

   /* Update music if needed. */
   music_update();

   if (sound_disabled) return 0;

   if (voice_active == NULL) return 0;

   /* The actual control loop. */
   for (v=voice_active; v!=NULL; v=v->next) {

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
         v->channel = 0;
         if (v->next != NULL)
            v->next->prev = v;

         /* Avoid loop blockage. */
         v = (tv != NULL) ? tv->next : voice_active;
         if (v == NULL) break;
      }
   }

   return 0;
}


/**
 * @brief Stops a voice from playing.
 *
 *    @param voice Identifier of the voice to stop.
 */
void sound_stop( int voice )
{
   alVoice *v;

   if (sound_disabled) return;

   v = voice_get(voice);
   if (v != NULL) {
      Mix_HaltChannel(v->channel);
      v->state = VOICE_STOPPED;
   }

}


/**
 * @brief Updates the sound listener.
 *
 *    @param dir Direction listener is facing.
 *    @param x X position of the listener.
 *    @param y Y position of the listener.
 *    @return 0 on success.
 *
 * @sa sound_playPos
 */
int sound_updateListener( double dir, double x, double y )
{
   if (sound_disabled) return 0;

   sound_pos[0] = x;
   sound_pos[1] = y;
   sound_pos[2] = dir/M_PI*180.;
   return 0;
}


/**
 * @brief Makes the list of available sounds.
 */
static int sound_makeList (void)
{
   char** files;
   uint32_t nfiles,i;
   char tmp[64];
   int len;
   int mem;

   if (sound_disabled) return 0;

   /* get the file list */
   files = pack_listfiles( data, &nfiles );

   /* load the profiles */
   mem = 0;
   for (i=0; i<nfiles; i++)
      if ((strncmp( files[i], SOUND_PREFIX, strlen(SOUND_PREFIX))==0) &&
            (strncmp( files[i] + strlen(files[i]) - strlen(SOUND_SUFFIX),
                      SOUND_SUFFIX, strlen(SOUND_SUFFIX))==0)) {

         /* grow the selection size */
         sound_nlist++;
         if (sound_nlist > mem) { /* we must grow */
            mem += 32; /* we'll overallocate most likely */
            sound_list = realloc( sound_list, mem*sizeof(alSound));
         }

         /* remove the prefix and suffix */
         len = strlen(files[i]) - strlen(SOUND_SUFFIX SOUND_PREFIX);
         strncpy( tmp, files[i] + strlen(SOUND_PREFIX), len );
         tmp[len] = '\0';

         /* give it the new name */
         sound_list[sound_nlist-1].name = strdup(tmp);
         sound_list[sound_nlist-1].buffer = sound_load( files[i] );
      }
   /* shrink to minimum ram usage */
   sound_list = realloc( sound_list, sound_nlist*sizeof(alSound));

   /* free the char* allocated by pack */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   DEBUG("Loaded %d sound%s", sound_nlist, (sound_nlist==1)?"":"s");

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
   if (sound_disabled) return 0;
   return Mix_Volume( -1, MIX_MAX_VOLUME*vol);
}


/**
 * @brief Loads a sound into the sound_list.
 *
 *    @param filename Name fo the file to load.
 *    @return The SDL_Mixer of the loaded chunk.
 *
 * @sa sound_makeList
 */
static Mix_Chunk* sound_load( char *filename )
{
   void* wavdata;
   unsigned int size;
   SDL_RWops *rw;
   Mix_Chunk *buffer;

   if (sound_disabled) return NULL;

   /* get the file data buffer from packfile */
   wavdata = pack_readfile( DATA, filename, &size );
   rw = SDL_RWFromMem(wavdata, size);

   /* bind to OpenAL buffer */
   buffer = Mix_LoadWAV_RW(rw,1);

   if (buffer == NULL)
      DEBUG("Unable to load sound '%s': %s", filename, Mix_GetError());

   /* finish */
   free( wavdata );
   return buffer;
}


/**
 * @brief Frees the sound.
 *
 *    @param snd Sound to free.
 */
static void sound_free( alSound *snd )
{
   /* free the stuff */
   if (snd->name) {
      free(snd->name);
      snd->name = NULL;
   }
   Mix_FreeChunk(snd->buffer);
   snd->buffer = NULL;
}


/**
 * @brief Reserves num channels.
 *
 *    @param num Number of channels to reserve.
 *    @return 0 on success.
 */
int sound_reserve( int num )
{
   int ret;

   if (sound_disabled) return 0;

   sound_reserved += num;
   ret = Mix_ReserveChannels(num);

   if (ret != sound_reserved) {
      WARN("Unable to reserve %d channels: %s", sound_reserved, Mix_GetError());
      return -1;
   }

   return 0;
}


/**
 * @brief Creates a sound group.
 *
 *    @param tag Identifier of the group to creat.
 *    @param start Where to start creating the group.
 *    @param size Size of the group.
 *    @return 0 on success.
 */
int sound_createGroup( int tag, int start, int size )
{
   int ret;

   if (sound_disabled) return 0;

   ret = Mix_GroupChannels( start, start+size-1, tag );

   if (ret != size) {
      WARN("Unable to create sound group: %s", Mix_GetError());
      return -1;
   }

   return 0;
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
   int ret, channel;

   if (sound_disabled) return 0;

   channel = Mix_GroupAvailable(group);
   if (channel == -1) {
      WARN("Group '%d' has no free channels!", group);
      return -1;
   }

   ret = Mix_PlayChannel( channel, sound_list[sound].buffer,
         (once == 0) ? -1 : 0 );
   if (ret < 0) {
      WARN("Unable to play sound %d for group %d: %s",
            sound, group, Mix_GetError());
      return -1;
   }

   return 0;
}


/**
 * @brief Stops all the sounds in a group.
 *
 *    @param group Group to stop all it's sounds.
 */
void sound_stopGroup( int group )
{
   if (sound_disabled) return;

   Mix_HaltGroup(group);
}


/**
 * @brief Marks the voice to which channel belongs to as stopped.
 *
 * DO NOT CALL MIX_* FUNCTIONS FROM CALLBACKS!
 */
static void voice_markStopped( int channel )
{
   alVoice *v;

   for (v=voice_active; v!=NULL; v=v->next)
      if (v->channel == channel) {
         v->state = VOICE_STOPPED;
         break;
      }
}


/**
 * @brief Gets a new voice ready to be used.
 *
 *    @return New voice ready to use.
 */
static alVoice* voice_new (void)
{
   alVoice *v;

   /* No free voices, allocate a new one. */
   if (voice_pool == NULL) {
      v = malloc(sizeof(alVoice));
      memset(v, 0, sizeof(alVoice));
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
static int voice_add( alVoice* v )
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
   tv = voice_active;
   v->next = tv;
   v->prev = NULL;
   voice_active = v;
   if (tv != NULL)
      tv->prev = v;
   return 0;
}


/**
 * @brief Gets a voice by identifier.
 *
 *    @param id Identifier to look for.
 *    @return Voice matching identifier or NULL if not found.
 */
static alVoice* voice_get( int id )
{
   alVoice *v;

   if (voice_active==NULL) return NULL;

   for (v=voice_active; v!=NULL; v=v->next)
      if (v->id == id) {
         return v;
      }

   return NULL;
}

