/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SPACE_H
#  define NLUA_SPACE_H


#include "lua.h"

#include "space.h"


#define PLANET_METATABLE   "Planet"
#define SYSTEM_METATABLE   "System"
#define VECTOR_METATABLE   "Vec2"


/*
 * Lua wrappers.
 */
typedef struct LuaPlanet_s {
   Planet *p;
} LuaPlanet;
typedef struct LuaSystem_s {
   StarSystem *s;
} LuaSystem;
typedef struct LuaVector_s {
   Vector2d vec;
} LuaVector;


/* 
 * Load the space library.
 */
int lua_loadSpace( lua_State *L, int readonly );
int lua_loadVector( lua_State *L );

/*
 * Planet operations.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind );
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet );
int lua_isplanet( lua_State *L, int ind );

/*
 * System operations.
 */
LuaSystem* lua_tosystem( lua_State *L, int ind );
LuaSystem* lua_pushsystem( lua_State *L, LuaSystem sys );
int lua_issystem( lua_State *L, int ind );

/*
 * Vector operations.
 */
LuaVector* lua_tovector( lua_State *L, int ind );
LuaVector* lua_pushvector( lua_State *L, LuaVector vec );
int lua_isvector( lua_State *L, int ind );


#endif /* NLUA_SPACE_H */


