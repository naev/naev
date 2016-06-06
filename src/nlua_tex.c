/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_tex.c
 *
 * @brief Handles the Lua texture bindings.
 */

#include "nlua_tex.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "ndata.h"


/* Texture metatable methods. */
static int texL_close( lua_State *L );
static int texL_open( lua_State *L );
static int texL_dim( lua_State *L );
static int texL_sprites( lua_State *L );
static int texL_spriteFromDir( lua_State *L );
static const luaL_reg texL_methods[] = {
   { "__gc", texL_close },
   { "open", texL_open },
   { "dim", texL_dim },
   { "sprites", texL_sprites },
   { "spriteFromDir", texL_spriteFromDir },
   {0,0}
}; /**< Texture metatable methods. */




/**
 * @brief Loads the texture library.
 *
 *    @param L State to load texture library into.
 *    @return 0 on success.
 */
int nlua_loadTex( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Create the metatable */
   luaL_newmetatable(L, TEX_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, texL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, TEX_METATABLE);

   return 0;
}


/**
 * @brief Lua bindings to interact with OpenGL textures.
 *
 * This will allow you to load textures.
 *
 * An example would be:
 * @code
 * t  = tex.open( GFX_PATH"foo/bar.png" ) -- Loads the texture
 * w,h, sw,sh = t:dim()
 * sprites, sx,sy = t:sprites()
 * @endcode
 *
 * @luamod tex
 */
/**
 * @brief Gets texture at index.
 *
 *    @param L Lua state to get texture from.
 *    @param ind Index position to find the texture.
 *    @return Texture found at the index in the state.
 */
glTexture* lua_totex( lua_State *L, int ind )
{
   return *((glTexture**) lua_touserdata(L,ind));
}
/**
 * @brief Gets texture at index or raises error if there is no texture at index.
 *
 *    @param L Lua state to get texture from.
 *    @param ind Index position to find texture.
 *    @return Texture found at the index in the state.
 */
glTexture* luaL_checktex( lua_State *L, int ind )
{
   if (lua_istex(L,ind))
      return lua_totex(L,ind);
   luaL_typerror(L, ind, TEX_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a texture on the stack.
 *
 *    @param L Lua state to push texture into.
 *    @param texture Texture to push.
 *    @return Newly pushed texture.
 */
glTexture** lua_pushtex( lua_State *L, glTexture *texture )
{
   glTexture **t;
   t = (glTexture**) lua_newuserdata(L, sizeof(glTexture*));
   *t = texture;
   luaL_getmetatable(L, TEX_METATABLE);
   lua_setmetatable(L, -2);
   return t;
}
/**
 * @brief Checks to see if ind is a texture.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a texture.
 */
int lua_istex( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, TEX_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees the texture.
 *
 *    @luaparam t Texture to free.
 * @luafunc __gc( t )
 */
static int texL_close( lua_State *L )
{
   /* Free texture. */
   gl_freeTexture( luaL_checktex( L, 1 ) );

   return 0;
}


/**
 * @brief Opens a texture.
 *
 * @usage t = tex.open( "no_sprites.png" )
 * @usage t = tex.open( "spritesheet.png", 6, 6 )
 *
 *    @luaparam path Path to open.
 *    @luaparam sx Optional number of x sprites (defaults 1).
 *    @luaparam sy Optional number of y sprites (defaults 1).
 *    @luareturn The opened texture or nil on error.
 * @luafunc open( path, sx, sy )
 */
static int texL_open( lua_State *L )
{
   const char *path;
   glTexture *tex;
   int sx, sy;

   /* Defaults. */
   sx = 0;
   sy = 0;

   /* Get path. */
   path = luaL_checkstring( L, 1 );
   if (lua_gettop(L)>1) {
      sx = luaL_checkinteger(L,2);
      sy = luaL_checkinteger(L,3);
      if ((sx < 0 ) || (sy < 0))
         NLUA_ERROR( L, "Spritesheet dimensions must be positive" );
   }

   /* Push new texture. */
   if ((sx <=0 ) || (sy <= 0))
      tex = gl_newImage( path, 0 );
   else
      tex = gl_newSprite( path, sx, sy, 0 );
   if (tex == NULL)
      return 0;
   lua_pushtex( L, tex );
   return 1;
}


/**
 * @brief Gets the dimensions of the texture.
 *
 * @usage w,h, sw,sh = t:dim()
 *
 *    @luaparam t Texture to get dimensions of.
 *    @luareturn The width and height of the total image followed by the width and height of the sprites.
 * @luafunc dim( t )
 */
static int texL_dim( lua_State *L )
{
   glTexture *tex;

   /* Get texture. */
   tex = luaL_checktex( L, 1 );

   /* Get all 4 values. */
   lua_pushnumber( L, tex->w  );
   lua_pushnumber( L, tex->h  );
   lua_pushnumber( L, tex->sw );
   lua_pushnumber( L, tex->sh );
   return 4;
}


/**
 * @brief Gets the number of sprites in the texture.
 *
 * @usage sprites, sx,sy = t:sprites()
 *
 *    @luaparam t Texture to get sprites of.
 *    @luareturn The total number of sprites followed by the number of X sprites and the number of Y sprites.
 * @luafunc sprites( t )
 */
static int texL_sprites( lua_State *L )
{
   glTexture *tex;

   /* Get texture. */
   tex = luaL_checktex( L, 1 );

   /* Get sprites. */
   lua_pushnumber( L, tex->sx * tex->sy );
   lua_pushnumber( L, tex->sx );
   lua_pushnumber( L, tex->sy );
   return 3;
}


/**
 * @brief Gets the sprite that corresponds to a direction.
 *
 * @usage sx, sy = t:spriteFromdir( math.pi )
 *
 *    @luaparam t Texture to get sprite of.
 *    @luaparam a Direction to have sprite facing (in degrees).
 *    @luaparam b Whether radians should be used instead of degrees.
 *    @luareturn x and y positions of the sprite.
 * @luafunc spriteFromDir( t, a )
 */
static int texL_spriteFromDir( lua_State *L )
{
   double a;
   glTexture *tex;
   int sx, sy;

   /* Params. */
   tex = luaL_checktex( L, 1 );

   /* Use radians if requested, otherwise convert to degrees. */
   if (lua_gettop(L) > 2 && (lua_toboolean(L, 3)))
      a = luaL_checknumber( L, 2 );
   else
      a = luaL_checknumber( L, 2 ) / 180. * M_PI;

   /* Calculate with parameter sanity.. */
   if ((a >= 2.*M_PI) || (a < 0.)) {
      a = fmod( a, 2.*M_PI );
      if (a < 0.)
         a += 2.*M_PI;
   }
   gl_getSpriteFromDir( &sx, &sy, tex, a );

   /* Return. */
   lua_pushinteger( L, sx+1 );
   lua_pushinteger( L, sy+1 );
   return 2;
}

