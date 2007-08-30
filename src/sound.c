

#include "sound.h"

#include <sys/stat.h>

#include <AL/alc.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "main.h"
#include "log.h"
#include "pack.h"
#include "music.h"


/* sometimes not set in al.h */
#ifndef AL_FORMAT_VORBIS_EXT
#define  AL_FORMAT_VORBIS_EXT 0x10003
#endif /* AL_FORMAT_VORBIS_EXT */


typedef struct {
	ALfloat x, y;
} alVector;


static ALCcontext *al_context = NULL;
static ALCdevice *al_device = NULL;

static SDL_Thread *music_player = NULL;


/*
 * initializes the sound subsystem
 */
int sound_init (void)
{
	/* opening the default device */
	al_device = alcOpenDevice(NULL);
	if (al_device == NULL) {
		WARN("Unable to open default sound device");
		return -1;
	}

	/* create the OpenAL context */
	al_context = alcCreateContext( al_device, NULL );
	if (al_context == NULL) {
		WARN("Unable to create OpenAL context");
		return -2;
	}

	/* we use the vorbis extension for AL to load it easy */
	if (alIsExtensionPresent("AL_EXT_vorbis")==AL_FALSE) {
		WARN("AL_EXT_vorbis not available in OpenAL context!");
		return -3;
	}

	if (alcMakeContextCurrent( al_context )==AL_FALSE) { /* set active context */
		WARN("Failure to set default context");
		return -4;
	}

	music_init();
	music_player = SDL_CreateThread( music_thread, NULL );

	return 0;
}
/*
 * cleans up after the sound subsytem
 */
void sound_exit (void)
{
	music_stop();
	music_kill();
	SDL_WaitThread( music_player, NULL );
	music_exit();

	if (al_context) {
		alcMakeContextCurrent(NULL);
		alcDestroyContext( al_context );
	}
	if (al_device) alcCloseDevice( al_device );
}


/*
 * loads a vorbis file
 */
ALuint sound_sndCreate( char* filename ) 
{
	void* ovdata;	
	unsigned int size;
	ALenum err;
	ALuint buf;

	/* get the file data buffer from packfile */
	ovdata = pack_readfile( DATA, filename, &size );

	/* bind to OpenAL buffer */
	alGenBuffers( 1, &buf );
	alBufferData( buf, AL_FORMAT_VORBIS_EXT, ovdata, size, 44800 );

	/* errors? */
	if ((err = alGetError()) != AL_NO_ERROR) {
		WARN("OpenAL error '%d' loading sound '%s'.", err, filename );
		return 0;
	}

	/* finish */
	free( ovdata );
	return buf;
}
void sound_sndFree( const ALuint snd )
{
	alDeleteBuffers( 1, &snd );
}


