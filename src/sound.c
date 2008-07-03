/*
 * See Licensing and Copyright notice in naev.h
 */



#include "sound.h"

#include <sys/stat.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_thread.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "music.h"
#include "physics.h"


#define SOUND_CHANNEL_MAX  256 /* Overkill */

#define SOUND_PREFIX "snd/sounds/"
#define SOUND_SUFFIX ".wav"


/*
 * Global sound properties.
 */
int sound_disabled = 0; /* Whether sound is disabled */
static int sound_reserved = 0; /* Amount of reserved channels */
static double sound_pos[3]; /* Position of listener. */


/*
 * gives the buffers a name
 */
typedef struct alSound_ {
   char *name; /* buffer's name */
   Mix_Chunk *buffer;
} alSound;


/*
 * list of sounds available (all preloaded into a buffer)
 */
static alSound *sound_list = NULL;
static int sound_nlist = 0;


/*
 * prototypes
 */
static int sound_makeList (void);
static Mix_Chunk *sound_load( char *filename );
static void sound_free( alSound *snd );


/*
 * initializes the sound subsystem
 */
int sound_init (void)
{
   int frequency;
   Uint16 format;
   int channels;
   SDL_version compile_version;
   const SDL_version *link_version;

   if (sound_disabled) return 0;

   SDL_InitSubSystem(SDL_INIT_AUDIO);
   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT , 2, 1024) < 0) {
      WARN("Opening Audio: %s", Mix_GetError());
      return -1;
   }
   Mix_AllocateChannels(SOUND_CHANNEL_MAX);

   /* debug magic */
   Mix_QuerySpec(&frequency, &format, &channels);
   MIX_VERSION(&compile_version);
   link_version = Mix_Linked_Version();
   DEBUG("SDL_Mixer: %d.%d.%d [compiled: %d.%d.%d]", 
         compile_version.major, compile_version.minor, compile_version.patch,
         link_version->major, link_version->minor, link_version->patch);
   DEBUG("Format: %d Hz %s", frequency, (channels == 2) ? "Stereo" : "Mono");
   DEBUG();

   /* load up all the sounds */
   sound_makeList();
   sound_volume(0.4);

   /* Initialize the music */
   music_init();

   return 0;
}


/*
 * cleans up after the sound subsytem
 */
void sound_exit (void)
{
   int i;

   /* free the sounds */
   for (i=0; i<sound_nlist; i++)
      sound_free( &sound_list[i] );
   free( sound_list );
   sound_list = NULL;
   sound_nlist = 0;

   music_exit();
}


/*
 * Gets the buffer to sound of name.
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


/*
 * Plays the sound.
 */
int sound_play( int sound )
{
   int channel;

   if (sound_disabled) return 0;

   if ((sound < 0) || (sound > sound_nlist))
      return -1;

   channel = Mix_PlayChannel( -1, sound_list[sound].buffer, 0 );
   
   if (channel < 0)
      WARN("Unable to play sound: %s", Mix_GetError());

   return 0;
}


int sound_playPos( int sound, double x, double y )
{
   int channel;
   double angle, dist;
   double px,py;

   if (sound_disabled) return 0;

   if ((sound < 0) || (sound > sound_nlist))
      return -1;

   px = x - sound_pos[0];
   py = y - sound_pos[1];

   angle = sound_pos[2] - ANGLE(px,py)/M_PI*180.;
   dist = MOD(px,py);

   channel = Mix_PlayChannel( -1, sound_list[sound].buffer, 0 );

   if (channel < 0) {
      WARN("Unable to play sound: %s", Mix_GetError());
      return -1;
   }

   if (Mix_SetPosition( channel, (int)angle, (int)dist / 10 ) < 0) {
      WARN("Unable to set sound position: %s", Mix_GetError());
      return -1;
   }

   return 0;
}


int sound_updateListener( double dir, double x, double y )
{
   sound_pos[0] = x;
   sound_pos[1] = y;
   sound_pos[2] = dir/M_PI*180.;
   return 0;
}


/*
 * makes the list of available sounds
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


/*
 * Sets the volume.
 */
int sound_volume( const double vol )
{
   if (sound_disabled) return 0;
   return Mix_Volume( -1, MIX_MAX_VOLUME*vol);
}


/*
 * loads a sound into the sound_list
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


/*
 * Reserves num channels.
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


/*
 * Creates a sound group.
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


/*
 * Plays a sound in a group.
 */
int sound_playGroup( int group, int sound, int once )
{
   int ret, channel;

   if (sound_disabled) return 0;

   channel = Mix_GroupAvailable(group);

   ret = Mix_PlayChannel( channel, sound_list[sound].buffer,
         (once == 0) ? -1 : 0 );

   if (ret < 0) {
      WARN("Unable to play sound %d for group %d: %s",
            sound, group, Mix_GetError());
      return -1;
   }
   
   return 0;
}


/*
 * Stops all the sounds in a group.
 */
void sound_stopGroup( int group )
{
   if (sound_disabled) return;

   Mix_FadeOutGroup(group, 100);
}


