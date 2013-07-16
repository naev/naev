/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file sound_sdlmix.c
 *
 * @brief SDL_mixer backend for sound.
 */


#if USE_SDLMIX


#include "sound_sdlmix.h"

#include "naev.h"

#include <sys/stat.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mixer.h"

#include "sound_priv.h"
#include "log.h"
#include "ndata.h"
#include "music.h"
#include "physics.h"
#include "conf.h"


/*
 * Global sound properties.
 */
static double sound_curVolume = 1.; /**< Current sound volume. */
static double sound_speedVolume = 1.; /**< Speed volume. */
static unsigned char sound_mixVolume = 0; /**< Actual in-game used volume. */
static double sound_pos[3]; /**< Position of listener. */
static double sound_speed = 1.; /**< Speed of the sound. */


/*
 * Groups.
 */
typedef struct mixGroup_s {
   int id; /**< ID of the group. */
   int start; /**< Start channel of the group. */
   int end; /**< End channel of the group. */
   int speed; /**< Whether or not affected by pitch. */
   double volume; /**< Volume of the group. */
} mixGroup_t;
static mixGroup_t *groups     = NULL; /**< Allocated Mixer groups. */
static int ngroups            = 0; /**< Number of allocated Mixer groups. */
static int group_idgen        = 0; /**< Current group ID generator. */
static int group_pos          = 0; /**< Current group position pointer. */


/*
 * prototypes
 */
/* General. */
static void print_MixerVersion (void);
/* Voices. */
static int sound_mix_updatePosVoice( alVoice *v, double x, double y );
static void voice_mix_markStopped( int channel );
static void sound_mix_volumeUpdate (void);


/**
 * @brief Initializes the sound subsystem.
 *
 *    @return 0 on success.
 */
int sound_mix_init (void)
{
   SDL_InitSubSystem(SDL_INIT_AUDIO);
   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT , 2, 1024) < 0) {
      WARN("Opening Audio: %s", Mix_GetError());
      DEBUG();
      return -1;
   }
   Mix_AllocateChannels( conf.snd_voices );

   /* Reset some variables. */
   group_pos = 0;

   /* Debug magic. */
   print_MixerVersion();

   /* Finish function. */
   Mix_ChannelFinished( voice_mix_markStopped );

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
#if SDL_VERSION_ATLEAST(2,0,0)
   const char *drvname = SDL_GetCurrentAudioDriver();
   strncpy( device, drvname, PATH_MAX );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_AudioDriverName(device, PATH_MAX);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Version itself. */
   DEBUG("SDL_Mixer Started: %d Hz %s", frequency,
         (channels == 2) ? "Stereo" : "Mono" );
   /* Check if major/minor version differ. */
   if ((linked->major*100 + linked->minor) > compiled.major*100 + compiled.minor)
      WARN("SDL_Mixer is newer than compiled version");
   if ((linked->major*100 + linked->minor) < compiled.major*100 + compiled.minor)
      WARN("SDL_Mixer is older than compiled version.");
   /* Print other debug info. */
   DEBUG("Renderer: %s",device);
   DEBUG("Version: %d.%d.%d [compiled: %d.%d.%d]",
         compiled.major, compiled.minor, compiled.patch,
         linked->major, linked->minor, linked->patch);
   DEBUG();
}


/**
 * @brief Cleans up after the sound subsytem.
 */
void sound_mix_exit (void)
{
   /* Free groups. */
   if (groups != NULL)
      free(groups);
   groups  = NULL;
   ngroups = 0;

   /* Close the audio. */
   Mix_CloseAudio();
}


/**
 * @brief Plays a sound with SDL_mixer.
 *
 *    @param v Voice to play sound.
 *    @param s Sound to play.
 *    @return 0 on success.
 */
int sound_mix_play( alVoice *v, alSound *s )
{
   if (sound_speed > SOUND_SPEED_PLAY_LIMIT)
      return 0;

   v->u.mix.channel = Mix_PlayChannel( -1, s->u.mix.buf, 0 );
   if (v->u.mix.channel >= 0)
      Mix_Volume( v->u.mix.channel, sound_mixVolume );

   /* Check to see if played. */
   /*
   if (v->channel < 0) {
      WARN("Unable to play sound: %s", Mix_GetError());
      return 1;
   }
   */

   return 0;
}


/**
 * @brief Updates the position of a voice.
 *
 *    @param v Voice to update.
 *    @param x New X position for the voice.
 *    @param y New Y position for the voice.
 *    @return 0 on success.
 */
static int sound_mix_updatePosVoice( alVoice *v, double x, double y )
{
   double angle, dist;
   double px, py;
   double d;
   int idist;

   /* Get relative position. */
   px = x - sound_pos[0];
   py = y - sound_pos[1];

   /* Exact calculations. */
   angle = sound_pos[2] - ANGLE(px,py)/M_PI*180.;
   dist = MOD(px,py);

   /* Need to make sure distance doesn't overflow. */
   d = CLAMP( 0., 1., (dist - 50.) / 2500. );
   d = 255. * sqrt(d);
   idist = MIN( (int)d, 255);

   /* Panning also gets modulated at low distance. */
   if (idist < 10)
      angle *= d/10.;

   /* Try to play the song. */
   if (Mix_SetPosition( v->u.mix.channel, (Sint16)angle, (Uint8)idist) < 0) {
      WARN("Unable to set sound position: %s", Mix_GetError());
      return -1;
   }

   return 0;
}


/**
 * @brief Not needed.
 */
void sound_mix_update (void)
{
}


/**
 * @brief Plays a sound based on position.
 *
 *    @param v Voice to play sound.
 *    @param s Sound to play.
 *    @param px X position of the sound.
 *    @param py Y position of the sound.
 *    @param vx X velocity of the sound.
 *    @param vy Y velocity of the sound.
 *    @return 0 on success.
 */
int sound_mix_playPos( alVoice *v, alSound *s,
      double px, double py, double vx, double vy )
{
   (void) vx;
   (void) vy;

   if (sound_speed > SOUND_SPEED_PLAY_LIMIT)
      return 0;

   /* Get the channel. */
   v->u.mix.channel = Mix_PlayChannel( -1, s->u.mix.buf, 0 );
   if (v->u.mix.channel < 0)
      return -1;
   else
      Mix_Volume( v->u.mix.channel, sound_mixVolume );

   /* Update the voice. */
   if (sound_mix_updatePosVoice( v, px, py ))
      return -1;

   return 0;
}


/**
 * @brief Updates the position of a voice.
 *
 *    @param v Identifier of the voice to update.
 *    @param px New X position of the sound.
 *    @param py New Y position of the sound.
 *    @param vx New X velocity of the sound.
 *    @param vy New Y velocity of the sound.
 */
int sound_mix_updatePos( alVoice *v,
      double px, double py, double vx, double vy )
{
   (void) vx;
   (void) vy;

   /* Update the voice. */
   if (sound_mix_updatePosVoice( v, px, py ))
      return -1;

   return 0;
}


/**
 * @brief Pauses all the sounds.
 */
void sound_mix_pause (void)
{
   Mix_Pause(-1);
}


/**
 * @brief Resumes all the sounds.
 */
void sound_mix_resume (void)
{
   Mix_Resume(-1);
}


/**
 * @brief Stops a voice from playing.
 *
 *    @param voice Identifier of the voice to stop.
 */
void sound_mix_stop( alVoice *v )
{
   Mix_FadeOutChannel( v->u.mix.channel, 100 );
}


/**
 * @brief Updates the sound listener.
 *
 *    @param dir Direction listener is facing.
 *    @param px X position of the listener.
 *    @param py Y position of the listener.
 *    @param vx X velocity of the listener.
 *    @param vy Y velocity of the listener.
 *    @return 0 on success.
 *
 * @sa sound_playPos
 */
int sound_mix_updateListener( double dir, double px, double py,
      double vx, double vy )
{
   (void) vx;
   (void) vy;

   sound_pos[0] = px;
   sound_pos[1] = py;
   sound_pos[2] = dir/M_PI*180.;

   return 0;
}


/**
 * @brief Updates the volume.
 */
static void sound_mix_volumeUpdate (void)
{
   int i, j;
   mixGroup_t *g;
   double v;
   unsigned char cv;

   /* Set volume for all sources. */
   Mix_Volume( -1, sound_mixVolume );
   /* Set volume for groups. */
   for (j=0; j<ngroups; j++) {
      g = &groups[j];
      v = sound_curVolume * g->volume;
      if (g->speed)
         v *= sound_speedVolume;
      cv = (unsigned char) (MIX_MAX_VOLUME*v);
      for (i=g->start; i<=g->end; i++)
         Mix_Volume( i, cv );
   }
}


/**
 * @brief Sets the volume.
 *
 *    @param vol Volume to set to.
 *    @return 0 on success.
 */
int sound_mix_volume( const double vol )
{
   /* Calculate volume. */
   sound_curVolume = CLAMP(0., 1., vol);
   sound_mixVolume = (unsigned char) (MIX_MAX_VOLUME * CLAMP(0., 1., sound_speedVolume*sound_curVolume));
   /* Update volume. */
   sound_mix_volumeUpdate();
   return 0;
}


/**
 * @brief Gets the current sound volume.
 *
 *    @return The current sound volume level.
 */
double sound_mix_getVolume (void)
{
   return sound_curVolume;
}


/**
 * @brief Loads a sound into the sound_list.
 *
 *    @param filename Name of the file to load.
 *    @return The SDL_Mixer of the loaded chunk.
 *
 * @sa sound_makeList
 */
int sound_mix_load( alSound *s, const char *filename )
{
   SDL_RWops *rw;
   int freq, bytes, channels;
   Uint16 format;

   /* get the file data buffer from packfile */
   rw = ndata_rwops( filename );

   /* bind to buffer */
   s->u.mix.buf = Mix_LoadWAV_RW( rw, 1 );
   if (s->u.mix.buf == NULL) {
      DEBUG("Unable to load sound '%s': %s", filename, Mix_GetError());
      return -1;
   }

   /* Get spec. */
   Mix_QuerySpec( &freq, &format, &channels );
   switch (format) {
      case AUDIO_U8:
      case AUDIO_S8:
         bytes = 1;
         break;
      default:
         bytes = 2;
         break;
   }

   /* Set length. */
   s->length = (double)s->u.mix.buf->alen / (double)(freq*bytes*channels);

   return 0;
}


/**
 * @brief Frees the sound.
 *
 *    @param snd Sound to free.
 */
void sound_mix_free( alSound *snd )
{
   Mix_FreeChunk(snd->u.mix.buf);
   snd->u.mix.buf = NULL;
}


/**
 * @brief Creates a sound group.
 *
 *    @param tag Identifier of the group to create.
 *    @param start Where to start creating the group.
 *    @param size Size of the group.
 *    @param ID of the group on success, otherwise 0.
 */
int sound_mix_createGroup( int size )
{
   int ret;
   mixGroup_t *g;

   /* Create new group. */
   ngroups++;
   groups      = realloc( groups, sizeof(mixGroup_t) * ngroups );
   g           = &groups[ ngroups-1 ];
   g->volume   = 1.;
   g->speed    = 1;

   /* Reserve channels. */
   ret = Mix_ReserveChannels( group_pos + size );
   if (ret != group_pos + size) {
      WARN("Unable to reserve sound channels: %s", Mix_GetError());
      return -1;
   }

   /* Get a new ID. */
   g->id    = ++group_idgen;

   /* Set group struct. */
   g->start = group_pos;
   g->end   = group_pos+size-1;

   /* Create group. */
   ret = Mix_GroupChannels( g->start, g->end, g->id );
   if (ret != size) {
      WARN("Unable to create sound group: %s", Mix_GetError());
      ngroups--;
      return -1;
   }

   /* Add to stack. */
   group_pos += size;

   return g->id;
}


/**
 * @brief Gets a group by ID.
 */
static mixGroup_t* sound_mix_getGroup( int group )
{
   int i;
   for (i=0; i<ngroups; i++)
      if (groups[i].id == group)
         return &groups[i];
   WARN("Group '%d' not found.", group);
   return NULL;
}


/**
 * @brief Plays a sound in a group.
 *
 *    @param group Group to play sound in.
 *    @param sound Sound to play.
 *    @param once Whether to play only once.
 *    @return 0 on success.
 */
int sound_mix_playGroup( int group, alSound *s, int once )
{
   int ret, channel;
   double v;
   unsigned char cv;
   mixGroup_t *g;

   /* Get the channel. */
   channel = Mix_GroupAvailable(group);
   if (channel == -1) {
      channel = Mix_GroupOldest(group);
      if (channel == -1) {
         WARN("Group '%d' has no free channels!", group);
         return -1;
      }
   }

   /* Play the sound. */
   ret = Mix_PlayChannel( channel, s->u.mix.buf, (once == 0) ? -1 : 0 );
   if (ret < 0) {
      WARN("Unable to play sound %s for group %d: %s",
            s->name, group, Mix_GetError());
      return -1;
   }

   g = sound_mix_getGroup( group );
   if (g == NULL) {
      WARN("Group '%d' does not exist!", group);
      return 0;
   }
   v = sound_curVolume * g->volume;
   if (g->speed)
      v *= sound_speedVolume;
   cv = (unsigned char) (MIX_MAX_VOLUME*v);
   Mix_Volume( channel, cv );

   return 0;
}


/**
 * @brief Stops all the sounds in a group.
 *
 *    @param group Group to stop all its sounds.
 */
void sound_mix_stopGroup( int group )
{
   Mix_FadeOutGroup(group, 100);
}


/**
 * @brief Pauses all the sounds in a group.
 */
void sound_mix_pauseGroup( int group )
{
   int i;
   mixGroup_t *g;
   g = sound_mix_getGroup( group );
   if (g==NULL)
      return;

   for (i=g->start; i<=g->end; i++) {
      if (Mix_Playing(i))
         Mix_Pause(i);
   }
}


/**
 * @brief Pauses all the sounds in a group.
 */
void sound_mix_resumeGroup( int group )
{
   int i;
   mixGroup_t *g;
   g = sound_mix_getGroup( group );
   if (g==NULL)
      return;

   for (i=g->start; i<=g->end; i++) {
      if (Mix_Paused(i))
         Mix_Resume(i);
   }
}


/**
 * @brief Sets whether or not speed affects the group.
 */
void sound_mix_speedGroup( int group, int enable )
{
   mixGroup_t *g;
   g = sound_mix_getGroup( group );
   if (g==NULL)
      return;
   g->speed = enable;
}


/**
 * @brief Sets the volume of a group.
 */
void sound_mix_volumeGroup( int group, double volume )
{
   int i;
   mixGroup_t *g;
   double v;
   unsigned char cv;

   g = sound_mix_getGroup( group );
   if (g==NULL)
      return;

   g->volume = CLAMP( 0., 1., volume );
   v = sound_curVolume*g->volume;
   if (g->speed)
      v *= sound_speedVolume;
   cv = (unsigned char) (MIX_MAX_VOLUME*v);
   for (i=g->start; i<=g->end; i++)
      Mix_Volume( i, cv );
}


/**
 * Unsupported.
 */
void sound_mix_setSpeed( double s )
{
   sound_speed = s;
}


/**
 * @brief Sets the speed volume.
 */
void sound_mix_setSpeedVolume( double vol )
{
   sound_speedVolume = CLAMP( 0., 1., vol );
   sound_mix_volumeUpdate();
}


/**
 * @brief Does nothing atm.
 *
 *    @param v Unused.
 */
void sound_mix_updateVoice( alVoice *v )
{
   (void) v;
}


/**
 * @brief Marks the voice to which channel belongs to as stopped.
 *
 * DO NOT CALL MIX_* FUNCTIONS FROM CALLBACKS!
 */
static void voice_mix_markStopped( int channel )
{
   alVoice *v;

   voice_lock();
   for (v=voice_active; v!=NULL; v=v->next)
      if (v->u.mix.channel == channel) {
         v->state = VOICE_STOPPED;
         break;
      }
   voice_unlock();
}


/**
 * @brief SDL_mixer doesn't handle this.
 */
int sound_mix_env( SoundEnv_t env, double param )
{
   (void) env;
   (void) param;
   return 0;
}


#endif /* USE_SDLMIX */
