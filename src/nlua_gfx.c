/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_gfx.c
 *
 * @brief Handles the rendering of graphics on the screen.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_gfx.h"

#include "array.h"
#include "font.h"
#include "gltf.h"
#include "log.h"
#include "nlua_canvas.h"
#include "nlua_colour.h"
#include "nlua_font.h"
#include "nlua_shader.h"
#include "nlua_tex.h"
#include "nlua_transform.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "opengl.h"
#include "render.h"

/* GFX methods. */
static int gfxL_dim( lua_State *L );
static int gfxL_screencoords( lua_State *L );
static int gfxL_renderTex( lua_State *L );
static int gfxL_renderTexRaw( lua_State *L );
static int gfxL_renderTexScale( lua_State *L );
static int gfxL_renderTexH( lua_State *L );
static int gfxL_renderRect( lua_State *L );
static int gfxL_renderRectH( lua_State *L );
static int gfxL_renderCircle( lua_State *L );
static int gfxL_renderCircleH( lua_State *L );
static int gfxL_renderLinesH( lua_State *L );
static int gfxL_clearDepth( lua_State *L );
static int gfxL_fontSize( lua_State *L );
/* TODO get rid of printDim and print in favour of printfDim and printf */
static int gfxL_printfDim( lua_State *L );
static int gfxL_printfWrap( lua_State *L );
static int gfxL_printRestoreClear( lua_State *L );
static int gfxL_printRestoreLast( lua_State *L );
static int gfxL_printf( lua_State *L );
static int gfxL_printH( lua_State *L );
static int gfxL_printDim( lua_State *L );
static int gfxL_print( lua_State *L );
static int gfxL_printText( lua_State *L );
static int gfxL_setBlendMode( lua_State *L );
static int gfxL_setScissor( lua_State *L );
static int gfxL_lightAmbient( lua_State *L );
static int gfxL_lightAmbientGet( lua_State *L );
static int gfxL_lightIntensity( lua_State *L );
static int gfxL_lightIntensityGet( lua_State *L );
static int gfxL_screenshot( lua_State *L );
static int gfxL_glVersion( lua_State *L );

static const luaL_Reg gfxL_methods[] = {
   /* Information. */
   { "dim", gfxL_dim },
   { "screencoords", gfxL_screencoords },
   /* Render stuff. */
   { "renderTex", gfxL_renderTex },
   { "renderTexRaw", gfxL_renderTexRaw },
   { "renderTexScale", gfxL_renderTexScale },
   { "renderTexH", gfxL_renderTexH },
   { "renderRect", gfxL_renderRect },
   { "renderRectH", gfxL_renderRectH },
   { "renderCircle", gfxL_renderCircle },
   { "renderCircleH", gfxL_renderCircleH },
   { "renderLinesH", gfxL_renderLinesH },
   /* Printing. */
   { "clearDepth", gfxL_clearDepth },
   { "fontSize", gfxL_fontSize },
   { "printfDim", gfxL_printfDim },
   { "printfWrap", gfxL_printfWrap },
   { "printRestoreClear", gfxL_printRestoreClear },
   { "printRestoreLast", gfxL_printRestoreLast },
   { "printf", gfxL_printf },
   { "printH", gfxL_printH },
   { "printDim", gfxL_printDim },
   { "print", gfxL_print },
   { "printText", gfxL_printText },
   /* 3D rendering. */
   { "lightAmbient", gfxL_lightAmbient },
   { "lightAmbientGet", gfxL_lightAmbientGet },
   { "lightIntensity", gfxL_lightIntensity },
   { "lightIntensityGet", gfxL_lightIntensityGet },
   /* Misc. */
   { "setBlendMode", gfxL_setBlendMode },
   { "setScissor", gfxL_setScissor },
   { "screenshot", gfxL_screenshot },
   { "glVersion", gfxL_glVersion },
   { 0, 0 } }; /**< GFX methods. */

/**
 * @brief Loads the graphics library.
 *
 *    @param env Environment to load graphics library into.
 *    @return 0 on success.
 */
int nlua_loadGFX( nlua_env env )
{
   /* Register the values */
   nlua_register( env, "gfx", gfxL_methods, 0 );

   /* We also load the texture, colour, font, and transform modules as
    * dependencies. */
   nlua_loadCol( env );
   nlua_loadTex( env );
   nlua_loadFont( env );
   nlua_loadTransform( env );
   nlua_loadShader( env );
   nlua_loadCanvas( env );

   return 0;
}

/**
 * @brief Lua bindings to interact with rendering and the Naev graphical
 * environment.
 *
 * An example would be:
 * @code
 * t  = tex.open( GFX_PATH"foo/bar.png" ) -- Loads the texture
 * gfx.renderTex( t, 0., 0. ) -- Draws texture at origin
 * @endcode
 *
 * @luamod gfx
 */

/**
 * @brief Gets the dimensions of the Naev window.
 *
 * @usage screen_w, screen_h = gfx.dim()
 *
 *    @luatparam[opt=false] boolean internalsize Whether or not to consider the
 * GUI modifications to the screen size.
 *    @luatreturn number The width of the Naev window.
 *    @luatreturn number The height of the Naev window.
 *    @luatreturn scale The scaling factor.
 * @luafunc dim
 */
static int gfxL_dim( lua_State *L )
{
   if ( lua_isboolean( L, 1 ) ) {
      lua_pushnumber( L, SCREEN_W );
      lua_pushnumber( L, SCREEN_H );
   } else {
      lua_pushnumber( L, gl_screen.nw );
      lua_pushnumber( L, gl_screen.nh );
   }
   lua_pushnumber( L, gl_screen.scale );
   return 3;
}

/**
 * @brief Gets the screen coordinates from game coordinates.
 *
 *    @luatparam Vec2 Vector of coordinates to transnform.
 *    @luatparam[opt=false] boolean Whether or not to invert y axis.
 *    @luatreturn Vec2 Transformed vector.
 * @luafunc screencoords
 */
static int gfxL_screencoords( lua_State *L )
{
   vec2        screen;
   double      x, y;
   const vec2 *game   = luaL_checkvector( L, 1 );
   int         invert = lua_toboolean( L, 2 );
   gl_gameToScreenCoords( &x, &y, game->x, game->y );
   if ( invert )
      y = SCREEN_H - y;
   vec2_cset( &screen, x, y );
   lua_pushvector( L, screen );
   return 1;
}

/**
 * @brief Renders a texture.
 *
 * This function has variable parameters depending on how you want to render.
 *
 * @usage gfx.renderTex( tex, 0., 0. ) -- Render tex at origin
 * @usage gfx.renderTex( tex, 0., 0., col ) -- Render tex at origin with colour
 * col
 * @usage gfx.renderTex( tex, 0., 0., 4, 3 ) -- Render sprite at position 4,3
 * (top-left is 1,1)
 * @usage gfx.renderTex( tex, 0., 0., 4, 3, col ) -- Render sprite at position
 * 4,3 (top-left is 1,1) with colour col
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam number pos_x X position to render texture at.
 *    @luatparam number pos_y Y position to render texture at.
 *    @luatparam[opt=0] int sprite_x X sprite to render.
 *    @luatparam[opt=0] int sprite_y Y sprite to render.
 *    @luatparam[opt] Colour colour Colour to use when rendering.
 * @luafunc renderTex
 */
static int gfxL_renderTex( lua_State *L )
{
   const glTexture *tex;
   const glColour  *col;
   double           x, y;
   int              sx, sy;

   /* Parameters. */
   tex = luaL_checktex( L, 1 );
   x   = luaL_checknumber( L, 2 );
   y   = luaL_checknumber( L, 3 );
   if ( lua_isnumber( L, 4 ) ) {
      sx  = luaL_checkinteger( L, 4 ) - 1;
      sy  = luaL_checkinteger( L, 5 ) - 1;
      col = luaL_optcolour( L, 6, &cWhite );
   } else {
      sx  = 0;
      sy  = 0;
      col = luaL_optcolour( L, 4, &cWhite );
   }

   /* Some safety checking. */
#if DEBUGGING
   if ( sx < 0 || sx >= tex->sx )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (X "
                            "position) sprite: %d > %d." ),
                         tex->name, sx + 1, tex->sx );
   if ( sy < 0 || sy >= tex->sy )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (Y "
                            "position) sprite: %d > %d." ),
                         tex->name, sy + 1, tex->sy );
#endif /* DEBUGGING */

   /* Render. */
   gl_renderStaticSprite( tex, x, y, sx, sy, col );

   return 0;
}

/**
 * @brief DEPRECATED: Renders a texture, scaled. Ultimately, love.graphics
 * should handle this.
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam number pos_x X position to render texture at.
 *    @luatparam number pos_y Y position to render texture at.
 *    @luatparam number bw Width to scale to.
 *    @luatparam number bh Height to scale to.
 *    @luatparam[opt=0] int sprite_x X sprite to render.
 *    @luatparam[opt=0] int sprite_y Y sprite to render.
 *    @luatparam[opt] Colour colour Colour to use when rendering.
 * @luafunc renderTexScale
 */
static int gfxL_renderTexScale( lua_State *L )
{
   glTexture *tex;
   glColour  *col;
   double     x, y, bw, bh;
   int        sx, sy;

   /* Parameters. */
   col = NULL;
   tex = luaL_checktex( L, 1 );
   x   = luaL_checknumber( L, 2 );
   y   = luaL_checknumber( L, 3 );
   bw  = luaL_checknumber( L, 4 );
   bh  = luaL_checknumber( L, 5 );
   if ( lua_isnumber( L, 6 ) ) {
      sx = luaL_checkinteger( L, 6 ) - 1;
      sy = luaL_checkinteger( L, 7 ) - 1;
      if ( lua_iscolour( L, 8 ) )
         col = luaL_checkcolour( L, 8 );
   } else {
      sx = 0;
      sy = 0;
      if ( lua_iscolour( L, 6 ) )
         col = luaL_checkcolour( L, 4 );
   }

   /* Some safety checking. */
#if DEBUGGING
   if ( sx >= tex->sx )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (X "
                            "position) sprite: %d > %d." ),
                         tex->name, sx + 1, tex->sx );
   if ( sx >= tex->sx )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (Y "
                            "position) sprite: %d > %d." ),
                         tex->name, sy + 1, tex->sy );
#endif /* DEBUGGING */

   /* Render. */
   gl_renderScaleSprite( tex, x, y, sx, sy, bw, bh, col );

   return 0;
}

/**
 * @brief Renders a texture using the core render function.
 *
 * This function is far more complex than renderTex, however it allows much
 *  more fine grained control over the entire render process and puts you
 *  closer to the actual OpenGL calls.
 *
 * @usage gfx.renderTexRaw( tex, 0., 0., 100., 100., 1, 1, 0., 0., 0.5, 0.5 ) --
 * Renders the bottom quarter of the sprite 1,1 of the image.
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam number pos_x X position to render texture at.
 *    @luatparam number pos_y Y position to render texture at.
 *    @luatparam number pos_w Width of the image on screen.
 *    @luatparam number pos_h Height of the image on screen.
 *    @luatparam[opt=1] number sprite_x X sprite to render.
 *    @luatparam[opt=1] number sprite_y Y sprite to render.
 *    @luatparam[opt=0.] number tex_x X sprite texture offset as [0.:1.].
 *    @luatparam[opt=0.] number tex_y Y sprite texture offset as [0.:1.].
 *    @luatparam[opt=1.] number tex_w Sprite width to display as [-1.:1.]. Note
 * if negative, it will flip the image horizontally.
 *    @luatparam[opt=1.] number tex_h Sprite height to display as [-1.:1.] Note
 * if negative, it will flip the image vertically.
 *    @luatparam[opt] Colour colour Colour to use when rendering.
 *    @luatparam[opt] number angle Angle to rotate in radians.
 * @luafunc renderTexRaw
 */
static int gfxL_renderTexRaw( lua_State *L )
{
   glTexture      *t;
   const glColour *col;
   double          px, py, pw, ph, tx, ty, tw, th;
   double          angle;
   int             sx, sy;

   /* Parameters. */
   t     = luaL_checktex( L, 1 );
   px    = luaL_checknumber( L, 2 );
   py    = luaL_checknumber( L, 3 );
   pw    = luaL_checknumber( L, 4 );
   ph    = luaL_checknumber( L, 5 );
   sx    = luaL_optinteger( L, 6, 1 ) - 1;
   sy    = luaL_optinteger( L, 7, 1 ) - 1;
   tx    = luaL_optnumber( L, 8, 0. );
   ty    = luaL_optnumber( L, 9, 0. );
   tw    = luaL_optnumber( L, 10, 1. );
   th    = luaL_optnumber( L, 11, 1. );
   col   = luaL_optcolour( L, 12, &cWhite );
   angle = luaL_optnumber( L, 13, 0. );

   /* Some safety checking. */
#if DEBUGGING
   if ( sx >= t->sx )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (X "
                            "position) sprite: %d > %d." ),
                         t->name, sx + 1, t->sx );
   if ( sx >= t->sx )
      return NLUA_ERROR( L,
                         _( "Texture '%s' trying to render out of bounds (Y "
                            "position) sprite: %d > %d." ),
                         t->name, sy + 1, t->sy );
#endif /* DEBUGGING */

   /* Translate as needed. */
   tx = ( tx * t->sw + t->sw * (double)( sx ) ) / t->w;
   tw = tw * t->srw;
   if ( tw < 0 )
      tx -= tw;
   ty = ( ty * t->sh + t->sh * ( t->sy - (double)sy - 1 ) ) / t->h;
   th = th * t->srh;
   if ( th < 0 )
      ty -= th;

   gl_renderTexture( t, px, py, pw, ph, tx, ty, tw, th, col, angle );
   return 0;
}

/**
 * @brief Renders a texture using a transformation matrix.
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam Shader shader Shader to use when rendering.
 *    @luatparam Transformation H Transformation matrix to use.
 *    @luatparam[opt=white] Colour colour Colour to use or white if not set.
 * @luafunc renderTexH
 */
static int gfxL_renderTexH( lua_State *L )
{
   glTexture      *t;
   const glColour *col;
   LuaShader_t    *shader;
   mat4           *H, *TH, ID;

   ID = mat4_identity();

   /* Parameters. */
   t      = luaL_checktex( L, 1 );
   shader = luaL_checkshader( L, 2 );
   H      = luaL_checktransform( L, 3 );
   col    = luaL_optcolour( L, 4, &cWhite );
   TH     = luaL_opttransform( L, 5, &ID );

   glUseProgram( shader->program );

   /* Set the vertex. */
   glEnableVertexAttribArray( shader->VertexPosition );
   gl_vboActivateAttribOffset( gl_paneVBO, shader->VertexPosition, 0, 2,
                               GL_FLOAT, 0 );

   /* Set up texture vertices if necessary. */
   if ( shader->VertexTexCoord >= 0 ) {
      gl_uniformMat4( shader->ViewSpaceFromLocal, TH );
      glEnableVertexAttribArray( shader->VertexTexCoord );
      gl_vboActivateAttribOffset( gl_paneVBO, shader->VertexTexCoord, 0, 2,
                                  GL_FLOAT, 0 );
   }

   /* Set the texture(s). */
   glBindTexture( GL_TEXTURE_2D, t->texture );
   glUniform1i( shader->MainTex, 0 );
   for ( int i = 0; i < array_size( shader->tex ); i++ ) {
      LuaTexture_t *lt = &shader->tex[i];
      glActiveTexture( lt->active );
      glBindTexture( GL_TEXTURE_2D, lt->texid );
      glUniform1i( lt->uniform, lt->value );
   }
   glActiveTexture( GL_TEXTURE0 );

   /* Set shader uniforms. */
   gl_uniformColour( shader->ConstantColour, col );
   gl_uniformMat4( shader->ClipSpaceFromLocal, H );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shader->VertexPosition );
   if ( shader->VertexTexCoord >= 0 )
      glDisableVertexAttribArray( shader->VertexTexCoord );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );

   return 0;
}

/**
 * @brief Renders a rectangle.
 *
 * @usage gfx.renderRect( 10., 30,. 40., 40., col ) -- Renders a 40 side square
 * at position 10,30 of colour col
 * @usage gfx.renderRect( 10., 30,. 40., 40., col, True ) -- Renders a 40 side
 * empty square at position 10,30 of colour col
 *
 *    @luatparam number x X position to render at.
 *    @luatparam number y Y position to render at.
 *    @luatparam number w Width of the rectangle.
 *    @luatparam number h Height of the rectangle.
 *    @luatparam Colour col Colour to use.
 *    @luatparam[opt=0] int lw Width of the outline or 0 to fill.
 *    @luatparam[opt=0] int rx The corners curve length along x axis.
 *    @luatparam[opt=0] int ry The corners curve length along y axis.
 * @luafunc renderRect
 */
static int gfxL_renderRect( lua_State *L )
{
   glColour *col;
   double    x, y, w, h, lw, rx, ry;

   /* Parse parameters. */
   x   = luaL_checknumber( L, 1 );
   y   = luaL_checknumber( L, 2 );
   w   = luaL_checknumber( L, 3 );
   h   = luaL_checknumber( L, 4 );
   col = luaL_checkcolour( L, 5 );
   lw  = luaL_optnumber( L, 6, 0. );
   rx  = luaL_optnumber( L, 7, 0. );
   ry  = luaL_optnumber( L, 8, 0. );

   /* Render. */
   gl_renderRoundRect( x, y, w, h, lw, rx, ry, col );
   return 0;
}

/**
 * @brief Renders a rectangle given a transformation matrix.
 *
 *    @luatparam Transform H Transformation matrix to use.
 *    @luatparam[opt=white] Colour col Colour to use.
 *    @luatparam[opt=false] boolean empty Whether or not it should be empty.
 *    @luatparam[opt=0] int lw Width of the outline or 0 to fill.
 *    @luatparam[opt=0] int rx The corners curve length along x axis.
 *    @luatparam[opt=0] int ry The corners curve length along y axis.
 * @luafunc renderRectH
 */
static int gfxL_renderRectH( lua_State *L )
{
   /* Parse parameters. */
   const mat4     *H     = luaL_checktransform( L, 1 );
   const glColour *col   = luaL_optcolour( L, 2, &cWhite );
   int             empty = lua_toboolean( L, 3 );
   int             lw    = luaL_optinteger( L, 4, 0 );
   int             rx    = luaL_optinteger( L, 5, 0 );
   int             ry    = luaL_optinteger( L, 6, 0 );

   /* Render. */
   lw = empty ? lw : 0;
   gl_renderRectH( H, col, lw, rx, ry );

   return 0;
}

/**
 * @brief Renders a circle
 *
 *    @luatparam number x X position to render at.
 *    @luatparam number y Y position to render at.
 *    @luatparam number r Radius of the circle.
 *    @luatparam Colour col Colour to use.
 *    @luatparam[opt=0] int lw Width of the outline or 0 to fill.
 * @luafunc renderCircle
 */
static int gfxL_renderCircle( lua_State *L )
{
   glColour *col;
   double    x, y, r, lw;

   /* Parse parameters. */
   x   = luaL_checknumber( L, 1 );
   y   = luaL_checknumber( L, 2 );
   r   = luaL_checknumber( L, 3 );
   col = luaL_checkcolour( L, 4 );
   lw  = luaL_optinteger( L, 5, 0 );

   /* Render. */
   gl_renderCircle( x, y, r, col, lw );

   return 0;
}

/**
 * @brief Renders a circle given a transformation matrix.
 *
 *    @luatparam Transform H Transformation matrix to use.
 *    @luatparam[opt=white] Colour col Colour to use.
 *    @luatparam[opt=false] boolean empty Whether or not it should be empty.
 *    @luatparam[opt=0] int lw Width of the outline or 0 to fill.
 * @luafunc renderCircleH
 */
static int gfxL_renderCircleH( lua_State *L )
{

   /* Parse parameters. */
   const mat4     *H     = luaL_checktransform( L, 1 );
   const glColour *col   = luaL_optcolour( L, 2, &cWhite );
   int             empty = lua_toboolean( L, 3 );
   int             lw    = luaL_optinteger( L, 4, 0 );

   /* Render. */
   lw = empty ? lw : 0;
   gl_renderCircleH( H, col, lw );

   return 0;
}

static gl_vbo *vbo_lines = NULL;
/**
 * @brief Renders a polyline or set of line segments.
 *
 * @usage gfx.renderLinesH( 50,30, 70,70 )
 * @usage gfx.renderLinesH( vec2.new(), vec2.new(100,100), vec2.new(200,100) )
 *
 *    @luatparam Transform H Transform to use when rendering.
 *    @luatparam Colour Colour to use when drawing.
 *    @luatparam number|Vec2 Either a set of x/y coordinates or 2D vector.
 * @luafunc renderLinesH
 */
static int gfxL_renderLinesH( lua_State *L )
{
   GLfloat         buf[256 * 2];
   int             i = 3;
   GLuint          n = 0;
   const mat4     *H = luaL_checktransform( L, 1 );
   const glColour *c = luaL_optcolour( L, 2, &cWhite );

   if ( vbo_lines == NULL )
      vbo_lines = gl_vboCreateDynamic( 256 * sizeof( GLfloat ) * 2, NULL );

   while ( !lua_isnoneornil( L, i ) ) {
      if ( n >= 256 ) {
         WARN( _( "Trying to draw too many lines in one call!" ) );
         n = 256;
         break;
      }

      if ( lua_isnumber( L, i ) ) {
         double x       = luaL_checknumber( L, i );
         double y       = luaL_checknumber( L, i + 1 );
         buf[2 * n + 0] = x;
         buf[2 * n + 1] = y;
         n++;
         i += 2;
      } else {
         const vec2 *v  = luaL_checkvector( L, i );
         buf[2 * n + 0] = v->x;
         buf[2 * n + 1] = v->y;
         n++;
         i += 1;
      }
   }

   glUseProgram( shaders.lines.program );

   gl_vboData( vbo_lines, sizeof( GLfloat ) * 2 * n, buf );
   glEnableVertexAttribArray( shaders.lines.vertex );
   gl_vboActivateAttribOffset( vbo_lines, shaders.lines.vertex, 0, 2, GL_FLOAT,
                               2 * sizeof( GLfloat ) );

   gl_uniformColour( shaders.lines.colour, c );
   gl_uniformMat4( shaders.lines.projection, H );

   glDrawArrays( GL_LINE_STRIP, 0, n );
   glUseProgram( 0 );

   /* Check for errors. */
   gl_checkErr();

   return 0;
}

/**
 * @brief Clears the depth buffer.
 */
static int gfxL_clearDepth( lua_State *L )
{
   (void)L;
   glClear( GL_DEPTH_BUFFER_BIT );
   return 0;
}

/**
 * @brief Gets the size of the font.
 *
 *    @luatparam boolean small Whether or not to get the size of the small font.
 *    @luatreturn[opt=false] The size in pixels of the font.
 * @luafunc fontSize
 */
static int gfxL_fontSize( lua_State *L )
{
   int small = lua_toboolean( L, 1 );
   lua_pushnumber( L, small ? gl_smallFont.h : gl_defFont.h );
   return 1;
}

/**
 * @brief Gets the size of the text to print.
 *
 * @usage len = gfx.printDim( nil, _("Hello World!") ) -- Length of string with
 * normal font
 * @usage height = gfx.printDim( true, _([["Longer text"]]), 20 ) -- Dimensions
 * of text block
 *
 *    @luatparam boolean small Whether or not to use the small font.
 *    @luatparam string str Text to calculate length of.
 *    @luatparam[opt] int width Optional parameter to indicate it is a block of
 * text and to use this width.
 * @luafunc printDim
 */
static int gfxL_printDim( lua_State *L )
{
   const char *str;
   int         width;
   glFont     *font;

   /* Parse parameters. */
   font  = lua_toboolean( L, 1 ) ? &gl_smallFont : &gl_defFont;
   str   = luaL_checkstring( L, 2 );
   width = luaL_optinteger( L, 3, 0 );

   /* Print length. */
   if ( width == 0 )
      lua_pushnumber( L, gl_printWidthRaw( font, str ) );
   else
      lua_pushnumber( L, gl_printHeightRaw( font, width, str ) );
   return 1;
}

/**
 * @brief Gets the size of the text to print.
 *
 *    @luatparam font font Font to use.
 *    @luatparam string str Text to calculate length of.
 *    @luatparam[opt] int width Optional parameter to indicate it is a block of
 * text and to use this width.
 * @luafunc printfDim
 */
static int gfxL_printfDim( lua_State *L )
{
   const char *str;
   int         width;
   glFont     *font;

   /* Parse parameters. */
   font  = luaL_checkfont( L, 1 );
   str   = luaL_checkstring( L, 2 );
   width = luaL_optinteger( L, 3, 0 );

   /* Print length. */
   if ( width == 0 )
      lua_pushnumber( L, gl_printWidthRaw( font, str ) );
   else
      lua_pushnumber( L, gl_printHeightRaw( font, width, str ) );
   return 1;
}

/**
 * @brief Gets the wrap for text.
 *
 *    @luatparam font font Font to use.
 *    @luatparam string str Text to calculate length of.
 *    @luatparam int width Width to wrap at.
 *    @luatreturn table A table containing pairs of text and their width.
 *    @luatreturn number Maximum width of all the lines.
 * @luafunc printfWrap
 */
static int gfxL_printfWrap( lua_State *L )
{
   const char         *s;
   int                 width, maxw;
   glFont             *font;
   glPrintLineIterator iter;
   int                 linenum;

   /* Parse parameters. */
   font  = luaL_checkfont( L, 1 );
   s     = luaL_checkstring( L, 2 );
   width = luaL_checkinteger( L, 3 );
   if ( width < 0 )
      return NLUA_ERROR( L, _( "width has to be a positive value." ) );

   /* Process output into table. */
   lua_newtable( L );
   gl_printLineIteratorInit( &iter, font, s, width );
   linenum = 1;
   maxw    = 0;
   while ( gl_printLineIteratorNext( &iter ) ) {
      maxw = MAX( maxw, iter.l_width );

      /* Create entry of form { string, width } in the table. */
      lua_newtable( L ); /* t, t */
      lua_pushlstring( L, &s[iter.l_begin],
                       iter.l_end - iter.l_begin ); /* t, t, s */
      lua_rawseti( L, -2, 1 );                      /* t, t */
      lua_pushinteger( L, iter.l_width );           /* t, t, n */
      lua_rawseti( L, -2, 2 );                      /* t, t */
      lua_rawseti( L, -2, linenum++ );              /* t */
   }

   /* Push max width. */
   lua_pushinteger( L, maxw );

   return 2;
}

/**
 * @brief Clears the saved internal colour state.
 * @luafunc printRestoreClear
 */
static int gfxL_printRestoreClear( lua_State *L )
{
   (void)L;
   gl_printRestoreClear();
   return 0;
}

/**
 * @brief Restores the last saved internal colour state.
 * @luafunc printRestoreLast
 */
static int gfxL_printRestoreLast( lua_State *L )
{
   (void)L;
   gl_printRestoreLast();
   return 0;
}

/**
 * @brief Prints text on the screen using a font.
 *
 * @usage gfx.printf( font, _("Hello World!"), 50, 50, colour.new("Red") ) --
 * Displays text in red at 50,50.
 *
 *    @luatparam font font Font to use.
 *    @luatparam string str String to print.
 *    @luatparam number x X position to print at.
 *    @luatparam number y Y position to print at.
 *    @luatparam Colour col Colour to print text.
 *    @luatparam[opt] int max Maximum width to render up to.
 *    @luatparam[opt] boolean center Whether or not to center it.
 * @luafunc printf
 */
static int gfxL_printf( lua_State *L )
{
   glFont     *font;
   const char *str;
   double      x, y;
   glColour   *col;
   int         max, mid;

   /* Parse parameters. */
   font = luaL_checkfont( L, 1 );
   str  = luaL_checkstring( L, 2 );
   x    = luaL_checknumber( L, 3 );
   y    = luaL_checknumber( L, 4 );
   col  = luaL_checkcolour( L, 5 );
   max  = luaL_optinteger( L, 6, 0 );
   mid  = lua_toboolean( L, 7 );

   /* Render. */
   if ( mid )
      gl_printMidRaw( font, max, x, y, col, 0., str );
   else if ( max > 0 )
      gl_printMaxRaw( font, max, x, y, col, 0., str );
   else
      gl_printRaw( font, x, y, col, 0., str );
   return 0;
}

/**
 * @brief Prints text on the screen using a font with a transformation matirx.
 *
 *    @luatparam Transform H transformation matrix to use.
 *    @luatparam font font Font to use.
 *    @luatparam string str String to print.
 *    @luatparam[opt=white] Colour col Colour to print text.
 *    @luatparam[opt=0] number outline How big to make an outline.
 * @luafunc printH
 */
static int gfxL_printH( lua_State *L )
{
   const mat4     *H;
   glFont         *font;
   const char     *str;
   const glColour *col;
   double          outline;

   /* Parse parameters. */
   H       = luaL_checktransform( L, 1 );
   font    = luaL_checkfont( L, 2 );
   str     = luaL_checkstring( L, 3 );
   col     = luaL_optcolour( L, 4, &cWhite );
   outline = luaL_optnumber( L, 5, 0. );

   /* Render. */
   gl_printRawH( font, H, col, outline, str );
   return 0;
}

/**
 * @brief Prints text on the screen.
 *
 * @usage gfx.print( nil, _("Hello World!"), 50, 50, colour.new("Red") ) --
 * Displays text in red at 50,50.
 * @usage gfx.print( true, _("Hello World!"), 50, 50, col, 100 ) -- Displays
 * text to a maximum of 100 pixels wide.
 * @usage gfx.print( true, str, 50, 50, col, 100, true ) -- Displays centered
 * text to a maximum of 100 pixels.
 *
 *    @luatparam boolean small Whether or not to use a small font.
 *    @luatparam string str String to print.
 *    @luatparam number x X position to print at.
 *    @luatparam number y Y position to print at.
 *    @luatparam Colour col Colour to print text.
 *    @luatparam[opt] int max Maximum width to render up to.
 *    @luatparam[opt] boolean center Whether or not to center it.
 * @luafunc print
 */
static int gfxL_print( lua_State *L )
{
   glFont     *font;
   const char *str;
   double      x, y;
   glColour   *col;
   int         max, mid;

   /* Parse parameters. */
   font = lua_toboolean( L, 1 ) ? &gl_smallFont : &gl_defFont;
   str  = luaL_checkstring( L, 2 );
   x    = luaL_checknumber( L, 3 );
   y    = luaL_checknumber( L, 4 );
   col  = luaL_checkcolour( L, 5 );
   max  = luaL_optinteger( L, 6, 0 );
   mid  = lua_toboolean( L, 7 );

   /* Render. */
   if ( mid )
      gl_printMidRaw( font, max, x, y, col, -1., str );
   else if ( max > 0 )
      gl_printMaxRaw( font, max, x, y, col, -1., str );
   else
      gl_printRaw( font, x, y, col, -1., str );
   return 0;
}

/**
 * @brief Prints a block of text on the screen.
 *
 * @usage gfx.printText( true, 100, 50, 50, 100, 100, col ) -- Displays a
 * 100x100 block of text
 *
 *    @luatparam boolean small Whether or not to use a small font.
 *    @luatparam string str String to print.
 *    @luatparam number x X position to print at.
 *    @luatparam number y Y position to print at.
 *    @luatparam number w Width of the block of text.
 *    @luatparam number h Height of the block of text.
 *    @luatparam Colour col Colour to print text.
 *    @luatparam line_height Height of each line to print.
 * @luafunc printText
 */
static int gfxL_printText( lua_State *L )
{
   glFont     *font;
   const char *str;
   int         w, h, lh;
   double      x, y;
   glColour   *col;

   /* Parse parameters. */
   font = lua_toboolean( L, 1 ) ? &gl_smallFont : &gl_defFont;
   str  = luaL_checkstring( L, 2 );
   x    = luaL_checknumber( L, 3 );
   y    = luaL_checknumber( L, 4 );
   w    = luaL_checkinteger( L, 5 );
   h    = luaL_checkinteger( L, 6 );
   col  = luaL_checkcolour( L, 7 );
   lh   = luaL_optinteger( L, 8, 0 );

   /* Render. */
   gl_printTextRaw( font, w, h, x, y, lh, col, -1., str );

   return 0;
}

/**
 * @brief Sets the OpenGL blending mode. See
 * https://love2d.org/wiki/love.graphics.setBlendMode as of version 0.10.
 *
 * @usage gfx.setBlendMode( "alpha", "premultiplied" )
 *
 *    @luatparam string mode One of: "alpha", "replace", "screen", "add",
 * "subtract", "multiply", "lighten", or "darken".
 *    @luatparam[opt="alphamultiply"] string alphamode Override to
 * "premultiplied" when drawing canvases; see
 * https://love2d.org/wiki/BlendAlphaMode.
 * @luafunc setBlendMode
 */
static int gfxL_setBlendMode( lua_State *L )
{
   const char *mode, *alphamode;
   GLenum      func, srcRGB, srcA, dstRGB, dstA;

   /* Parse parameters. */
   mode      = luaL_checkstring( L, 1 );
   alphamode = luaL_optstring( L, 2, "alphamultiply" );

   /* Translate to OpenGL enums. */
   func   = GL_FUNC_ADD;
   srcRGB = GL_ONE;
   srcA   = GL_ONE;
   dstRGB = GL_ZERO;
   dstA   = GL_ZERO;

   if ( !strcmp( alphamode, "alphamultiply" ) )
      srcRGB = srcA = GL_SRC_ALPHA;
   else if ( !strcmp( alphamode, "premultiplied" ) ) {
      if ( !strcmp( mode, "lighten" ) || !strcmp( mode, "darken" ) ||
           !strcmp( mode, "multiply" ) )
         NLUA_INVALID_PARAMETER( L, 2 );
   } else
      NLUA_INVALID_PARAMETER( L, 2 );

   if ( !strcmp( mode, "alpha" ) )
      dstRGB = dstA = GL_ONE_MINUS_SRC_ALPHA;
   else if ( !strcmp( mode, "multiply" ) )
      srcRGB = srcA = GL_DST_COLOR;
   else if ( !strcmp( mode, "subtract" ) )
      func = GL_FUNC_REVERSE_SUBTRACT;
   else if ( !strcmp( mode, "add" ) ) {
      func   = GL_FUNC_REVERSE_SUBTRACT;
      srcA   = GL_ZERO;
      dstRGB = dstA = GL_ONE;
   } else if ( !strcmp( mode, "lighten" ) )
      func = GL_MAX;
   else if ( !strcmp( mode, "darken" ) )
      func = GL_MIN;
   else if ( !strcmp( mode, "screen" ) )
      dstRGB = dstA = GL_ONE_MINUS_SRC_COLOR;
   else if ( strcmp( mode, "replace" ) )
      NLUA_INVALID_PARAMETER( L, 1 );

   glBlendEquation( func );
   glBlendFuncSeparate( srcRGB, dstRGB, srcA, dstA );
   gl_checkErr();

   render_needsReset();
   return 0;
}

/**
 * @brief Sets the scissor clipping.
 *
 * Calling setScissor with no parameters disables the clipping.
 *
 *    @luatparam number x X position of the clipping rectangle.
 *    @luatparam number y Y position of the clipping rectangle.
 *    @luatparam number width Width of the clipping rectangle.
 *    @luatparam number height Height of the clipping rectangle.
 * @luafunc setScissor
 */
static int gfxL_setScissor( lua_State *L )
{
   if ( lua_gettop( L ) > 0 ) {
      GLint   x = luaL_optinteger( L, 1, 0 );
      GLint   y = luaL_optinteger( L, 2, 0 );
      GLsizei w = luaL_optinteger( L, 3, 0 );
      GLsizei h = luaL_optinteger( L, 4, 0 );
      gl_clipRect( x, y, w, h );
      render_needsReset();
   } else
      gl_unclipRect();

   return 0;
}

/**
 * @brief Takes the current rendered game screen and returns it as a canvas.
 *
 *    @luatparam[opt=nil] Canvas c Canvas to use instead of creating a new one.
 *    @luatreturn Canvas A new canvas or parameter canvas if available with the
 * screen data in it.
 * @luafunc screenshot
 */
static int gfxL_screenshot( lua_State *L )
{
   LuaCanvas_t *lc;
   int          mustfree;

   /* Set up canvas or try to reuse. */
   if ( lua_iscanvas( L, 1 ) ) {
      lc       = luaL_checkcanvas( L, 1 );
      mustfree = 0;
   } else {
      lc = calloc( 1, sizeof( LuaCanvas_t ) );
      canvas_new( lc, gl_screen.rw, gl_screen.rh );
      mustfree = 1;
   }

   /* Copy over. */
   glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
   glBindFramebuffer( GL_DRAW_FRAMEBUFFER, lc->fbo );
   /* We flip it over because that seems to be what love2d API wants. */
   glBlitFramebuffer( 0, 0, gl_screen.rw, gl_screen.rh, 0, lc->tex->h,
                      lc->tex->w, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST );
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   /* Return new or old canvas. */
   lua_pushcanvas( L, *lc );
   if ( mustfree )
      free( lc );
   return 1;
}

/**
 * @brief Sets the ambient lighting.
 *
 *    @luatparam Colour|number r Colour or red channel to use for ambient
 * lighting. In the case of a colour, the alpha channel is used as the radiance
 * value of the ambient light.
 *    @luatparam[opt=r] number g Green channel to use for ambient lighting.
 *    @luatparam[opt=r] number b Blue channel to use  for ambient lighting.
 *    @luatparam[opt] number strength If defined, normalizes the values of r, g,
 * and b so that the total radiance is equal to strength. If not, the values of
 * r, g, and b are considered to be radiance values.
 * @luafunc lightAmbient
 */
static int gfxL_lightAmbient( lua_State *L )
{
   double r, g, b;
   if ( lua_iscolour( L, 1 ) ) {
      const glColour *c = lua_tocolour( L, 1 );
      double n = c->a * sqrt( pow2( c->r ) + pow2( c->g ) + pow2( c->b ) );
      /* Premultiply alpha. */
      r = c->r * n;
      g = c->g * n;
      b = c->b * n;
   } else {
      r = luaL_checknumber( L, 1 );
      g = luaL_optnumber( L, 2, r );
      b = luaL_optnumber( L, 3, r );
      if ( !lua_isnoneornil( L, 4 ) ) {
         double n = luaL_checknumber( L, 4 ) /
                    sqrt( pow2( r ) + pow2( g ) + pow2( b ) );
         r *= n;
         g *= n;
         b *= n;
      }
   }
   gltf_lightAmbient( r, g, b );
   return 0;
}

/**
 * @brief Gets the ambient lighting values.
 *
 *    @luatreturn number r Red colour value.
 *    @luatreturn number g Green colour value.
 *    @luatreturn number b Blue colour value.
 * @luafunc lightAmbientGet
 */
static int gfxL_lightAmbientGet( lua_State *L )
{
   double r, g, b;
   gltf_lightAmbientGet( &r, &g, &b );
   lua_pushnumber( L, r );
   lua_pushnumber( L, g );
   lua_pushnumber( L, b );
   return 3;
}

/**
 * @brief Sets the intensity of the main lights excluding ambient lighting.
 * Multiplies the default radiosity values.
 *
 *    @luatparam number intensity Intensity to set the lights to.
 * @luafunc lightIntensity
 */
static int gfxL_lightIntensity( lua_State *L )
{
   gltf_lightIntensity( luaL_checknumber( L, 1 ) );
   return 0;
}

/**
 * @brief Gets the light intensity.
 *
 *    @luatreturn number Light intensity.
 * @luafunc lightIntensityGet
 */
static int gfxL_lightIntensityGet( lua_State *L )
{
   lua_pushnumber( L, gltf_lightIntensityGet() );
   return 1;
}

/**
 * @brief Gets the OpenGL version.
 *
 *    @luatreturn integer Major OpenGL version.
 *    @luatreturn integer Minor OpenGL version.
 *    @luatreturn integer GLSL version.
 * @luafunc glVersion
 */
static int gfxL_glVersion( lua_State *L )
{
   lua_pushinteger( L, gl_screen.major );
   lua_pushinteger( L, gl_screen.minor );
   lua_pushinteger( L, gl_screen.glsl );
   return 3;
}
