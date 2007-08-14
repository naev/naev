

#include "conf.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "main.h"
#include "log.h"
#include "player.h"
#include "opengl.h"


#define	conf_loadInt(n,i)		\
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
	i = (int)lua_tonumber(L, -1); \
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
 * sets the default configuration
 */
void conf_setDefaults (void)
{
	/* global */
	data = DATA_DEF;
	/* opengl */
	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.fullscreen = 0;
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
	int i;

	lua_State *L = luaL_newstate();
	if (luaL_dofile(L, file) == 0) { /* configuration file exists */

		/* global */
		conf_loadString("data",data);

		/* opengl properties*/
		conf_loadInt("width",gl_screen.w);
		conf_loadInt("height",gl_screen.h);
		conf_loadBool("fullscreen",gl_screen.fullscreen);
		conf_loadBool("showfps",show_fps);
		conf_loadInt("maxfps",max_fps);

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
		return 1;
	}

	lua_close(L);
	return 0;
}
