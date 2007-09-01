

#include "sound.h"

#include <sys/stat.h>

#include <AL/alc.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "main.h"
#include "log.h"
#include "pack.h"
#include "music.h"


#define SOUND_PREFIX	"snd/sounds/"
#define SOUND_SUFFIX ".wav"


typedef struct {
	char *name;
	ALuint buffer;
} alSound;


/*
 * global device and contex
 */
static ALCcontext *al_context = NULL;
static ALCdevice *al_device = NULL;

/*
 * music player thread to assure streaming is perfect
 */
static SDL_Thread *music_player = NULL;


/*
 * list of sounds available (all preloaded into a buffer)
 */
static alSound *sound_list = NULL;
static int nsound_list = 0;


/*
 * prototypes
 */
static int sound_makeList (void);
static int sound_load( ALuint *buffer, char *filename );
static void sound_free( alSound *snd );


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

	/* set active context */
	if (alcMakeContextCurrent( al_context )==AL_FALSE) {
		WARN("Failure to set default context");
		return -4;
	}

	/* load up all the sounds */
	sound_makeList();
	
	/* start the music server */
	music_init();
	music_player = SDL_CreateThread( music_thread, NULL );

	return 0;
}
/*
 * cleans up after the sound subsytem
 */
void sound_exit (void)
{
	int i;
	/* free the sounds */
	for (i=0; i<nsound_list; i++)
		sound_free( &sound_list[i] );
	free( sound_list );
	sound_list = NULL;
	nsound_list = 0;

	/* must stop the music before killing it, then thread should commit suicide */
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
 * gets the buffer to sound of name
 */
ALuint sound_get( char* name ) 
{
	int i;
	for (i=0; i<nsound_list; i++)
		if (strcmp(name, sound_list[i].name)==0)
			return sound_list[i].buffer;
	WARN("Sound '%s' not found in sound list", name);
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

	/* get the file list */
	files = pack_listfiles( data, &nfiles );

	/* load the profiles */
	for (i=0; i<nfiles; i++)
		if ((strncmp( files[i], SOUND_PREFIX, strlen(SOUND_PREFIX))==0) &&
				(strncmp( files[i] + strlen(files[i]) - strlen(SOUND_SUFFIX),
							 SOUND_SUFFIX, strlen(SOUND_SUFFIX))==0)) {

			/* grow the selection size */
			sound_list = realloc( sound_list, ++nsound_list*sizeof(alSound));

			/* remove the prefix and suffix */
			len = strlen(files[i]) - strlen(SOUND_SUFFIX SOUND_PREFIX);
			strncpy( tmp, files[i] + strlen(SOUND_PREFIX), len );
			tmp[len] = '\0';

			/* give it the new name */
			sound_list[nsound_list-1].name = strdup(tmp);
			sound_load( &sound_list[nsound_list-1].buffer, files[i] );
		}

	/* free the char* allocated by pack */
	for (i=0; i<nfiles; i++)
		free(files[i]);
	free(files);

	DEBUG("Loaded %d sound%c", nsound_list, (nsound_list==1)?' ':'s');

	return 0;
}


/*
 * loads a sound into the sound_list
 */
static int sound_load( ALuint *buffer, char *filename )
{
	void* wavdata;
	unsigned int size;
	ALenum err;

	/* get the file data buffer from packfile */
	wavdata = pack_readfile( DATA, filename, &size );

	/* bind to OpenAL buffer */
	alGenBuffers( 1, buffer );
	alBufferData( *buffer, AL_FORMAT_MONO16, wavdata, size, 22050 );

	/* errors? */
	if ((err = alGetError()) != AL_NO_ERROR) {
		WARN("OpenAL error '%d' loading sound '%s'.", err, filename );
		return 0;
	}

	/* finish */
	free( wavdata );
	return 0;
}
static void sound_free( alSound *snd )
{
	if (snd->name) free(snd->name);
	alDeleteBuffers( 1, &snd->buffer );
}


/*
 * creates a dynamic moving source
 */
ALuint sound_dynSource( double px, double py, double vx, double vy, int looping )
{
	ALuint tmp;

	alGenSources( 1, &tmp );
	
	alSourcei( tmp, AL_SOURCE_RELATIVE, AL_TRUE );
	alSource3f( tmp, AL_POSITION, px, py, 0. );
	alSource3f( tmp, AL_VELOCITY, vx, vy, 0. );

	if (looping) alSourcei( tmp, AL_LOOPING, AL_TRUE );

	return tmp;
}


