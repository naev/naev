

#include "music.h"

#include "SDL.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include "main.h"
#include "log.h"
#include "pack.h"


#define MUSIC_STOPPED		(1<<1)
#define MUSIC_PLAYING		(1<<2)
#define MUSIC_KILL			(1<<9)
#define music_is(f)			(music_state & f)
#define music_set(f)			(music_state |= f)
#define music_rm(f)			(music_state ^= f)

#define MUSIC_PREFIX			"snd/music/"
#define MUSIC_SUFFIX			".ogg"

#define BUFFER_SIZE			(4096*8)


/*
 * global sound mutex
 */
extern SDL_mutex *sound_lock;


/*
 * saves the music to ram in this structure
 */
typedef struct {
	char name[32]; /* name */
	Packfile file;
	OggVorbis_File stream;
	vorbis_info* info;
	ALenum format;
} alMusic;


/*
 * song currently playing
 */

static SDL_mutex *music_vorbis_lock;
static alMusic music_vorbis;
static ALuint music_buffer[2]; /* front and back buffer */
static ALuint music_source = 0;


/*
 * what is available
 */
static char** music_selection = NULL;
static int nmusic_selection = 0;


/*
 * volume
 */
static ALfloat mvolume = 1.;


/*
 * vorbis stuff
 */
static size_t ovpack_read( void *ptr, size_t size, size_t nmemb, void *datasource )
{	return (ssize_t) pack_read( datasource, ptr, size*nmemb );	} /* pack_read wrapper */
static int ovpack_retneg (void) { return -1; } /* must return -1 */
static int ovpack_retzero (void) { return 0; } /* must return 0 */
ov_callbacks ovcall = {
	.read_func = ovpack_read,
	.seek_func = (int(*)(void*,ogg_int64_t,int)) ovpack_retneg,
	.close_func = (int(*)(void*)) ovpack_retzero,
	.tell_func =(long(*)(void*)) ovpack_retneg
};


/*
 * prototypes
 */
static int stream_loadBuffer( ALuint buffer );
static int music_find (void);
static int music_loadOGG( const char *filename );
static void music_free (void);


/*
 * the thread
 */
static unsigned int music_state = 0;
int music_thread( void* unused )
{
	(void)unused;

	int active; /* active buffer */
	ALint stat;

	/* main loop */
	while (!music_is(MUSIC_KILL)) {
		
		if (music_is(MUSIC_PLAYING)) {
			if (music_vorbis.file.end == 0)
				music_rm(MUSIC_PLAYING);
			else {

				music_rm(MUSIC_STOPPED);

				SDL_mutexP( music_vorbis_lock ); /* lock the mutex */
				SDL_mutexP( sound_lock );

				/* start playing current song */
				active = 0; /* load first buffer */
				if (stream_loadBuffer( music_buffer[active] )) music_rm(MUSIC_PLAYING);
				alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

				/* start playing with buffer loaded */
				alSourcePlay( music_source );

				active = 1; /* load second buffer */
				if (stream_loadBuffer( music_buffer[active] )) music_rm(MUSIC_PLAYING);
				alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

				SDL_mutexV( sound_lock );

				active = 0; /* dive into loop */
			}
			while (music_is(MUSIC_PLAYING)) {

				SDL_mutexP( sound_lock );

				alGetSourcei( music_source, AL_BUFFERS_PROCESSED, &stat );
				if (stat > 0) {

					/* refill active buffer */
					alSourceUnqueueBuffers( music_source, 1, &music_buffer[active] );
					if (stream_loadBuffer( music_buffer[active] )) music_rm(MUSIC_PLAYING);
					alSourceQueueBuffers( music_source, 1, &music_buffer[active] );

					active = 1 - active;
				}
				
				SDL_mutexV( sound_lock );

				SDL_Delay(0);
			}

			SDL_mutexP( sound_lock );

			alSourceStop( music_source );
			alSourceUnqueueBuffers( music_source, 2, music_buffer );

			SDL_mutexV( sound_lock );
			SDL_mutexV( music_vorbis_lock );
		}

		music_set(MUSIC_STOPPED);
		SDL_Delay(0); /* we must not kill resources */
	}

	return 0;
}
static int stream_loadBuffer( ALuint buffer )
{
	int size, section, result;
	char data[BUFFER_SIZE]; /* buffer to hold the data */

	size = 0;
	while (size < BUFFER_SIZE) { /* fille up the entire data buffer */

		result = ov_read( &music_vorbis.stream, /* stream */
				data + size,            /* data */
				BUFFER_SIZE - size,     /* amount to read */
				0,                      /* big endian? */
				2,                      /* 16 bit */
				1,                      /* signed */
				&section );             /* current bitstream */

		if (result == 0) return 1;
		else if (result == OV_HOLE) {
			WARN("OGG: Vorbis hole detected in music!");
			return 0;
		}
		else if (result == OV_EBADLINK) {
			WARN("OGG: Invalid stream section or corrupt link in music!");
			return -1;
		}

		size += result;
		if (size == BUFFER_SIZE) break; /* buffer is full */
	}
	/* load the buffer up */
	alBufferData( buffer, music_vorbis.format,
			data, BUFFER_SIZE, music_vorbis.info->rate );

	return 0;
}


/*
 * init/exit
 */
int music_init()
{
	music_vorbis_lock = SDL_CreateMutex();
	music_find();
	music_vorbis.file.end = 0; /* indication it's not loaded */

	SDL_mutexP( sound_lock );

	alGenBuffers( 2, music_buffer );
	alGenSources( 1, &music_source );
	alSourcef( music_source, AL_GAIN, mvolume );
	alSourcef( music_source, AL_ROLLOFF_FACTOR, 0. );
	alSourcei( music_source, AL_SOURCE_RELATIVE, AL_TRUE );

	SDL_mutexV( sound_lock );

	return 0;
}
void music_exit()
{
	int i;

	/* free the music */
	alDeleteBuffers( 2, music_buffer );
	alDeleteSources( 1, &music_source );
	music_free();

	/* free selection */
	for (i=0; i<nmusic_selection; i++)
		free(music_selection[i]);
	free(music_selection);

	SDL_DestroyMutex( music_vorbis_lock );
}


/*
 * internal music loading routines
 */
static int music_loadOGG( const char *filename )
{
	/* free currently loaded ogg */
	music_free();

	SDL_mutexP( music_vorbis_lock );
	
	/* load new ogg */
	pack_open( &music_vorbis.file, DATA, filename );
	ov_open_callbacks( &music_vorbis.file, &music_vorbis.stream, NULL, 0, ovcall );
	music_vorbis.info = ov_info( &music_vorbis.stream, -1);

	/* set the format */
	if (music_vorbis.info->channels == 1) music_vorbis.format = AL_FORMAT_MONO16;
	else music_vorbis.format = AL_FORMAT_STEREO16;

	SDL_mutexV( music_vorbis_lock );

	return 0;
}
static int music_find (void)
{
	char** files;
	uint32_t nfiles,i;
	char tmp[64];
	int len;

	/* get the file list */
	files = pack_listfiles( data, &nfiles );

	/* load the profiles */
	for (i=0; i<nfiles; i++)
		if ((strncmp( files[i], MUSIC_PREFIX, strlen(MUSIC_PREFIX))==0) &&
				(strncmp( files[i] + strlen(files[i]) - strlen(MUSIC_SUFFIX),
							 MUSIC_SUFFIX, strlen(MUSIC_SUFFIX))==0)) {

			/* grow the selection size */
			music_selection = realloc( music_selection, ++nmusic_selection*sizeof(char*));

			/* remove the prefix and suffix */
			len = strlen(files[i]) - strlen(MUSIC_SUFFIX MUSIC_PREFIX);
			strncpy( tmp, files[i] + strlen(MUSIC_PREFIX), len );
			tmp[len] = '\0';
			
			/* give it the new name */
			music_selection[nmusic_selection-1] = strdup(tmp);
		}

	/* free the char* allocated by pack */
	for (i=0; i<nfiles; i++)
		free(files[i]);
	free(files);

	DEBUG("Loaded %d song%c", nmusic_selection, (nmusic_selection==1)?' ':'s');

	return 0;
}
static void music_free (void)
{
	SDL_mutexP( music_vorbis_lock );

	if (music_vorbis.file.end != 0) {
		ov_clear( &music_vorbis.stream );
		pack_close( &music_vorbis.file );
		music_vorbis.file.end = 0; /* somewhat officially ended */
	}

	SDL_mutexV( music_vorbis_lock );
}


/*
 * music control functions
 */
void music_volume( const double vol )
{
	/* sanity check */
	ALfloat fvol = ABS(vol);
	if (fvol > 1.) fvol = 1.;

	mvolume = fvol;

	/* only needed if playing */
	if (music_set(MUSIC_PLAYING)) {
		SDL_mutexP( sound_lock );
		alSourcef( music_source, AL_GAIN, fvol );
		SDL_mutexV( sound_lock );
	}
}
void music_load( const char* name )
{
	int i;
	char tmp[64];

	music_stop();
	while (!music_is(MUSIC_STOPPED)) SDL_Delay(0);

	for (i=0; i<nmusic_selection; i++)
		if (strcmp(music_selection[i], name)==0) {
			snprintf( tmp, 64, MUSIC_PREFIX"%s"MUSIC_SUFFIX, name );
			music_loadOGG(tmp);
			return;
		}
	WARN("Requested load song '%s' but it can't be found in the music stack",name);
}
void music_play (void)
{
	music_set(MUSIC_PLAYING);
}
void music_stop (void)
{
	music_rm(MUSIC_PLAYING);
}
void music_kill (void)
{
	music_set(MUSIC_KILL);
}

