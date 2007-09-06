

#include "conf.h"

#include <unistd.h> /* getopt */
#include <string.h> /* strdup */
#include <getopt.h> /* getopt_long */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "main.h"
#include "log.h"
#include "player.h"
#include "input.h"
#include "opengl.h"
#include "music.h"


#define	conf_loadInt(n,i)		\
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
	i = (int)lua_tonumber(L, -1); \
	lua_remove(L,-1); \
}

#define	conf_loadFloat(n,f)    \
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
	f = (double)lua_tonumber(L, -1); \
	lua_remove(L,-1); \
}

#define	conf_loadBool(n,b)	\
lua_getglobal(L, n); \
if (lua_isnumber(L, -1)) \
	if ((int)lua_tonumber(L, -1) == 1) { \
		b = 1; \
		lua_remove(L,-1); \
	}

#define	conf_loadString(n,s)	\
	lua_getglobal(L, n);	\
if (lua_isstring(L, -1)) {	\
	s = strdup((char*)lua_tostring(L, -1));	\
	lua_remove(L,-1);	\
}


/* from main.c */
extern int show_fps;
extern int max_fps;
extern int indjoystick;
extern char* namjoystick;
/* from player.c */
extern const char *keybindNames[]; /* keybindings */


/*
 * prototypes
 */
static void print_usage( char **argv );


/*
 * prints usage
 */
static void print_usage( char **argv )
{
	LOG("Usage: %s [OPTION]", argv[0]);
	LOG("Options are:");
	LOG("   -f, --fullscreen      fullscreen");
	LOG("   -F n, --fps n         limit frames per second");
	LOG("   -d s, --data s        set the data file to be s");
	/*LOG("   -w n                  set width to n");
	  LOG("   -h n                  set height to n");*/
	LOG("   -j n, --joystick n    use joystick n");
	LOG("   -J s, --Joystick s    use joystick whose name contains s");
	LOG("   -m f, --music f       sets the music volume to f");
	LOG("   -s f, --sound f       sets the sound volume to f");
	LOG("   -h, --help            display this message and exit");
	LOG("   -v, --version         print the version and exit");
}



/*
 * sets the default configuration
 */
void conf_setDefaults (void)
{
	/* global */
	data = DATA_DEF;
	/* opengl */
	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.flags = 0;
	/* joystick */
	indjoystick = -1;
	namjoystick = NULL;
	/* input */
	input_setDefault();
}


/*
 * parses the config file
 */
int conf_loadConfig ( const char* file )
{
	int i = 0;
	double d = 0.;

	lua_State *L = luaL_newstate();
	if (luaL_dofile(L, file) == 0) { /* configuration file exists */

		/* global */
		conf_loadString("data",data);

		/* opengl properties*/
		conf_loadInt("width",gl_screen.w);
		conf_loadInt("height",gl_screen.h);
		conf_loadBool("fullscreen",i);
		if (i) { gl_screen.flags |= OPENGL_FULLSCREEN; i = 0; }
		conf_loadBool("aa",i);
		if (i) {
			gl_screen.flags |= OPENGL_AA_POINT | OPENGL_AA_LINE | OPENGL_AA_POLYGON;
			i = 0; }
		conf_loadBool("aa_point",i);
		if (i) { gl_screen.flags |= OPENGL_AA_POINT; i = 0; }
		conf_loadBool("aa_line",i);
		if (i) { gl_screen.flags |= OPENGL_AA_LINE; i = 0; }
		conf_loadBool("aa_polygon",i);
		if (i) { gl_screen.flags |= OPENGL_AA_POLYGON; i = 0; }

		/* FPS */
		conf_loadBool("showfps",show_fps);
		conf_loadInt("maxfps",max_fps);

		/* sound */
		conf_loadFloat("sound",d);
		if (d) { sound_volume(d); d = 0.; }
		conf_loadFloat("music",d);
		if (d) { music_volume(d); d = 0.; }


		/* joystick */
		lua_getglobal(L, "joystick");
		if (lua_isnumber(L, -1)) {
			indjoystick = (int)lua_tonumber(L, -1);
			lua_remove(L,-1);
		}
		else if (lua_isstring(L, -1)) {
			namjoystick = strdup((char*)lua_tostring(L, -1));
			lua_remove(L,-1);
		}

		/* grab the keybindings if there are any */
		char *str;
		int type, key, reverse;
		for (i=0; strcmp(keybindNames[i],"end"); i++) {
			lua_getglobal(L, keybindNames[i]);
			str = NULL;
			key = -1;
			reverse = 0;
			if (lua_istable(L, -1)) { /* it's a table */
				/* gets the event type */
				lua_pushstring(L, "type");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1))
					str = (char*)lua_tostring(L, -1);

				/* gets the key */
				lua_pushstring(L, "key");
				lua_gettable(L, -3);
				if (lua_isnumber(L, -1))
					key = (int)lua_tonumber(L, -1);

				/* is reversed, only useful for axis */
				lua_pushstring(L, "reverse");
				lua_gettable(L, -4);
				if (lua_isnumber(L, -1))
					reverse = 1;

				if (key != -1 && str != NULL) { /* keybind is valid */
					/* get type */
					if (strcmp(str,"null")==0) type = KEYBIND_NULL;
					else if (strcmp(str,"keyboard")==0) type = KEYBIND_KEYBOARD;
					else if (strcmp(str,"jaxis")==0) type = KEYBIND_JAXIS;
					else if (strcmp(str,"jbutton")==0) type = KEYBIND_JBUTTON;
					else {
						WARN("Unkown keybinding of type %s", str);
						continue;
					}
					/* set the keybind */
					input_setKeybind( (char*)keybindNames[i], type, key, reverse );
				}
				else WARN("Malformed keybind in %s", file);              

				/* clean up after table stuff */
				lua_remove(L,-1);
				lua_remove(L,-1);
				lua_remove(L,-1);
				lua_remove(L,-1);
			}
		}
	}
	else { /* failed to load the config file */
		DEBUG("config file '%s' not found", file);
		lua_close(L);
		return 1;
	}

	lua_close(L);
	return 0;
}


/*
 * parses the CLI options
 */
void conf_parseCLI( int argc, char** argv )
{
	static struct option long_options[] = {
		{ "fullscreen", no_argument, 0, 'f' },
		{ "fps", required_argument, 0, 'F' },
		{ "data", required_argument, 0, 'd' },
		{ "joystick", required_argument, 0, 'j' },
		{ "Joystick", required_argument, 0, 'J' },
		{ "music", required_argument, 0, 'm' },
		{ "sound", required_argument, 0, 's' },
		{ "help", no_argument, 0, 'h' }, 
		{ "version", no_argument, 0, 'v' },
		{ NULL, 0, 0, 0 } };
	int option_index = 0;
	int c = 0;
	while ((c = getopt_long(argc, argv,
			"fF:d:J:j:V:hv",
			long_options, &option_index)) != -1) {
		switch (c) {
			case 'f':
				gl_screen.flags |= OPENGL_FULLSCREEN;
				break;
			case 'F':
				max_fps = atoi(optarg);
				break;
			case 'd': 
				data = strdup(optarg);
				break;
			case 'j':
				indjoystick = atoi(optarg);
				break;
			case 'J':
				namjoystick = strdup(optarg);
				break;
			case 'm':
				music_volume( atof(optarg) );
				break;
			case 's':
				sound_volume( atof(optarg) );
				break;

			case 'v':
				LOG(APPNAME": version %d.%d.%d", VMAJOR, VMINOR, VREV);
			case 'h':
				print_usage(argv);
				exit(EXIT_SUCCESS);
		}
	}
}


/* 
 * saves the current configuration
 */
int conf_saveConfig (void)
{
	/* TODO */
	return 0;
}

