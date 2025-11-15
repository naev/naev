/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl_render.c
 *
 * @brief This file handles the openGL rendering routines.
 *
 * There are two coordinate systems: relative and absolute.
 *
 * Relative:
 *  * Everything is drawn relative to the player, if it doesn't fit on screen
 *    it is clipped.
 *  * Origin (0., 0.) would be on top of the player.
 *
 * Absolute:
 *  * Everything is drawn in "screen coordinates".
 *  * (0., 0.) is bottom left.
 *  * (SCREEN_W, SCREEN_H) is top right.
 *
 * Note that the game actually uses a third type of coordinates for when using
 *  raw commands.  In this third type, the (0.,0.) is actually in middle of the
 *  screen.  (-SCREEN_W/2.,-SCREEN_H/2.) is bottom left and
 *  (+SCREEN_W/2.,+SCREEN_H/2.) is top right.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "opengl_render.h"

#include "camera.h"
#include "opengl.h"

#define OPENGL_RENDER_VBO_SIZE 256 /**< Size of VBO. */

static gl_vbo *gl_renderVBO          = 0; /**< VBO for rendering stuff. */
gl_vbo        *gl_squareVBO          = 0;
static gl_vbo *gl_squareEmptyVBO     = 0;
gl_vbo        *gl_circleVBO          = 0;
static gl_vbo *gl_lineVBO            = 0;
static gl_vbo *gl_triangleVBO        = 0;
static int     gl_renderVBOtexOffset = 0; /**< VBO texture offset. */
static int     gl_renderVBOcolOffset = 0; /**< VBO colour offset. */

void gl_beginSolidProgram( mat4 projection, const glColour *c )
{
   glUseProgram( shaders.solid.program );
   glEnableVertexAttribArray( shaders.solid.vertex );
   gl_uniformColour( shaders.solid.colour, c );
   gl_uniformMat4( shaders.solid.projection, &projection );
}

void gl_endSolidProgram( void )
{
   glDisableVertexAttribArray( shaders.solid.vertex );
   glUseProgram( 0 );
   gl_checkErr();
}

#if 0
/**
 * @brief Renders a rectangle.
 *
 *    @param x X position to render rectangle at.
 *    @param y Y position to render rectangle at.
 *    @param w Rectangle width.
 *    @param h Rectangle height.
 *    @param c Rectangle colour.
 */
void gl_renderRect( double x, double y, double w, double h, const glColour *c )
{
   /* Set the vertex. */
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, x, y, w, h );

   gl_renderRectH( &projection, c, 1 );
}
#endif

/**
 * @brief Renders a rectangle.
 *
 *    @param x X position to render rectangle at.
 *    @param y Y position to render rectangle at.
 *    @param w Rectangle width.
 *    @param h Rectangle height.
 *    @param c Rectangle colour.
 */
void gl_renderRectEmpty( double x, double y, double w, double h,
                         const glColour *c )
{
   // TODO probably replace with gl_renderRectEmptyThick as it handles DPI
   // scaling better
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, x, y, w, h );

   gl_renderRectH( &projection, c, 0 );
}

void gl_renderRectEmptyThick( double x, double y, double w, double h, double b,
                              const glColour *c )
{
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, x, y, w, h );

   glUseProgram( shaders.outline.program );
   glEnableVertexAttribArray( shaders.outline.vertex );
   glUniform2f( shaders.outline.border, ( w - 2. * b ) / w,
                ( h - 2. * b ) / h );
   gl_uniformColour( shaders.outline.colour, c );
   gl_uniformMat4( shaders.outline.projection, &projection );

   gl_vboActivateAttribOffset( gl_squareVBO, shaders.outline.vertex, 0, 2,
                               GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glDisableVertexAttribArray( shaders.outline.vertex );
   glUseProgram( 0 );
   gl_checkErr();
}

void gl_renderRectHalf( double x, double y, double w, double h,
                        const glColour *c )
{
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, x, y, w, h );
   gl_beginSolidProgram( projection, c );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.solid.vertex, 0, 2,
                               GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 3 );
   gl_endSolidProgram();
}

/**
 * @brief Renders a rectangle.
 *
 *    @param H Transformation matrix to apply.
 *    @param filled Whether or not to fill.
 *    @param c Rectangle colour.
 */
void gl_renderRectH( const mat4 *H, const glColour *c, int filled )
{
   gl_beginSolidProgram( *H, c );
   if ( filled ) {
      gl_vboActivateAttribOffset( gl_squareVBO, shaders.solid.vertex, 0, 2,
                                  GL_FLOAT, 0 );
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   } else {
      gl_vboActivateAttribOffset( gl_squareEmptyVBO, shaders.solid.vertex, 0, 2,
                                  GL_FLOAT, 0 );
      glDrawArrays( GL_LINE_STRIP, 0, 5 );
   }
   gl_endSolidProgram();
}

#if 0
/**
 * @brief Renders a cross at a given position.
 *
 *    @param x X position to centre at.
 *    @param y Y position to centre at.
 *    @param r Radius of cross.
 *    @param c Colour to use.
 */
void gl_renderCross( double x, double y, double r, const glColour *c )
{
   glUseProgram( shaders.crosshairs.program );
   glUniform1f( shaders.crosshairs.paramf, 1. ); /* No outline. */
   gl_renderShader( x, y, r, r, 0., &shaders.crosshairs, c, 1 );
}
#endif

/**
 * @brief Renders a triangle at a given position.
 *
 *    @param x X position to centre at.
 *    @param y Y position to centre at.
 *    @param a Angle the triangle should "face" (right is 0.)
 *    @param s Scaling of the triangle.
 *    @param length Length deforming factor. Setting it to a value of other
 * than 1. moves away from an equilateral triangle.
 *    @param c Colour to use.
 */
void gl_renderTriangleEmpty( double x, double y, double a, double s,
                             double length, const glColour *c )
{
   mat4 projection = gl_view_matrix;
   mat4_translate_xy( &projection, x, y );
   if ( a != 0. )
      mat4_rotate2d( &projection, a );
   mat4_scale_xy( &projection, s * length, s );

   gl_beginSolidProgram( projection, c );
   gl_vboActivateAttribOffset( gl_triangleVBO, shaders.solid.vertex, 0, 2,
                               GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 4 );
   gl_endSolidProgram();
}

void gl_renderDepthRawH( GLuint depth, const mat4 *projection,
                         const mat4 *tex_mat )
{
   /* Depth testing is required for depth writing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_ALWAYS );
   glColorMask( GL_FALSE, GL_FALSE, GL_FALSE,
                GL_FALSE ); /* Don't draw colour. */

   glUseProgram( shaders.texture_depth_only.program );

   /* Bind the texture_depth_only. */
   glBindTexture( GL_TEXTURE_2D, depth );

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.texture_depth_only.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_depth_only.vertex,
                               0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformMat4( shaders.texture_depth_only.projection, projection );
   gl_uniformMat4( shaders.texture_depth_only.tex_mat, tex_mat );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_depth_only.vertex );
   glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
   glDepthFunc( GL_LESS );
   glDisable( GL_DEPTH_TEST );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );
}

void gl_renderDepthRaw( GLuint depth, uint8_t flags, double x, double y,
                        double w, double h, double tx, double ty, double tw,
                        double th, double angle )
{
   (void)flags;
   mat4 projection, tex_mat;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if ( angle == 0. ) {
      mat4_translate_scale_xy( &projection, x, y, w, h );
   } else {
      double hw = w * 0.5;
      double hh = h * 0.5;
      mat4_translate_xy( &projection, x + hw, y + hh );
      mat4_rotate2d( &projection, angle );
      mat4_translate_scale_xy( &projection, -hw, -hh, w, h );
   }

   /* Set the texture. */
   tex_mat = mat4_identity();
   mat4_translate_scale_xy( &tex_mat, tx, ty, tw, th );

   gl_renderDepthRawH( depth, &projection, &tex_mat );
}

void gl_renderTextureDepthRawH( GLuint texture, GLuint depth,
                                const mat4 *projection, const mat4 *tex_mat,
                                const glColour *c )
{
   /* Depth testing is required for depth writing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_ALWAYS );

   glUseProgram( shaders.texture_depth.program );

   /* Bind the texture_depth. */
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, depth );
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, texture );

   /* Must have colour for now. */
   if ( c == NULL )
      c = &cWhite;

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.texture_depth.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_depth.vertex, 0, 2,
                               GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColour( shaders.texture_depth.colour, c );
   gl_uniformMat4( shaders.texture_depth.projection, projection );
   gl_uniformMat4( shaders.texture_depth.tex_mat, tex_mat );
   glUniform1i( shaders.texture_depth.depth, 1 );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_depth.vertex );
   glDepthFunc( GL_LESS );
   glDisable( GL_DEPTH_TEST );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );
}

void gl_renderTextureDepthRaw( GLuint texture, GLuint depth, uint8_t flags,
                               double x, double y, double w, double h,
                               double tx, double ty, double tw, double th,
                               const glColour *c, double angle )
{
   (void)flags;
   mat4 projection, tex_mat;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if ( angle == 0. ) {
      mat4_translate_scale_xy( &projection, x, y, w, h );
   } else {
      double hw = w * 0.5;
      double hh = h * 0.5;
      mat4_translate_xy( &projection, x + hw, y + hh );
      mat4_rotate2d( &projection, angle );
      mat4_translate_scale_xy( &projection, -hw, -hh, w, h );
   }

   /* Set the texture. */
   tex_mat = mat4_identity();
   mat4_translate_scale_xy( &tex_mat, tx, ty, tw, th );

   gl_renderTextureDepthRawH( texture, depth, &projection, &tex_mat, c );
}

/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param sampler Sampler to use.
 *    @param projection Projection matrix to use.
 *    @param tex_mat Texture matrix to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderTextureRawH( GLuint texture, GLuint sampler,
                           const mat4 *projection, const mat4 *tex_mat,
                           const glColour *c )
{
   glUseProgram( shaders.texture.program );

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, texture );
   if ( sampler > 0 )
      glBindSampler( 0, sampler );

   /* Must have colour for now. */
   if ( c == NULL )
      c = &cWhite;

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.texture.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture.vertex, 0, 2,
                               GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColour( shaders.texture.colour, c );
   gl_uniformMat4( shaders.texture.projection, projection );
   gl_uniformMat4( shaders.texture.tex_mat, tex_mat );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture.vertex );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );
   glBindTexture( GL_TEXTURE_2D, 0 );
   if ( sampler > 0 )
      glBindSampler( 0, 0 );
}

/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param sampler Sampler to use.
 *    @param flags Texture flags,.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture. [0:1]
 *    @param ty Y position within the texture. [0:1]
 *    @param tw Texture width. [0:1]
 *    @param th Texture height. [0:1]
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the centre).
 */
void gl_renderTextureRaw( GLuint texture, GLuint sampler, uint8_t flags,
                          double x, double y, double w, double h, double tx,
                          double ty, double tw, double th, const glColour *c,
                          double angle )
{
   (void)flags;
   mat4 projection, tex_mat;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if ( angle == 0. ) {
      mat4_translate_scale_xy( &projection, x, y, w, h );
   } else {
      double hw, hh; /* Half width and height. */
      hw = w * 0.5;
      hh = h * 0.5;
      mat4_translate_xy( &projection, x + hw, y + hh );
      mat4_rotate2d( &projection, angle );
      mat4_translate_scale_xy( &projection, -hw, -hh, w, h );
   }

   /* Set the texture. */
   tex_mat = mat4_identity();
   mat4_translate_scale_xy( &tex_mat, tx, ty, tw, th );

   gl_renderTextureRawH( texture, sampler, &projection, &tex_mat, c );
}

#if 0
/**
 * @brief Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture. [0:1]
 *    @param ty Y position within the texture. [0:1]
 *    @param tw Texture width. [0:1]
 *    @param th Texture height. [0:1]
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the centre).
 */
void gl_renderTexture( const glTexture *texture, double x, double y, double w,
                       double h, double tx, double ty, double tw, double th,
                       const glColour *c, double angle )
{
   gl_renderTextureRaw( tex_tex( texture ), tex_sampler( texture ), 0, x, y, w,
                        h, tx, ty, tw, th, c, angle );
}

/**
 * @brief SDF Texture blitting backend.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen. (units pixels)
 *    @param y Y position of the texture on the screen. (units pixels)
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param c Colour to use (modifies texture colour).
 *    @param angle Rotation to apply (radians ccw around the centre).
 *    @param outline Thickness of the outline.
 */
void gl_renderSDF( const glTexture *texture, double x, double y, double w,
                   double h, const glColour *c, double angle, double outline )
{
   (void)outline; /* TODO handle outline. */
   double hw, hh; /* Half width and height */
   double sw, sh;
   mat4   projection, tex_mat;

   glUseProgram( shaders.texturesdf.program );

   /* Bind the texture. */
   glBindTexture( GL_TEXTURE_2D, tex_tex( texture ) );

   /* Must have colour for now. */
   if ( c == NULL )
      c = &cWhite;

   hw = w * 0.5;
   hh = h * 0.5;

   /* Set the vertex. */
   projection = gl_view_matrix;
   if ( angle == 0. ) {
      mat4_translate_scale_xy( &projection, x + hw, y + hh, hw, hh );
   } else {
      mat4_translate_xy( &projection, x + hw, y + hh );
      mat4_rotate2d( &projection, angle );
      mat4_scale_xy( &projection, hw, hh );
   }
   glEnableVertexAttribArray( shaders.texturesdf.vertex );
   gl_vboActivateAttribOffset( gl_circleVBO, shaders.texturesdf.vertex, 0, 2,
                               GL_FLOAT, 0 );

   /* Set the texture. */
   /* TODO we would want to pad the texture a bit to get nice marked borders,
    * but we have to actually pad the SDF first... */
   sw      = 0.; // 1./w;
   sh      = 0.; // 1./h;
   tex_mat = mat4_identity();
   mat4_scale_xy( &tex_mat, tex_srw( texture ) + 2. * sw,
                  tex_srh( texture ) + 2. * sh );
   mat4_translate_xy( &tex_mat, -sw, -sh );

   /* Set shader uniforms. */
   gl_uniformColour( shaders.texturesdf.colour, c );
   gl_uniformMat4( shaders.texturesdf.projection, &projection );
   gl_uniformMat4( shaders.texturesdf.tex_mat, &tex_mat );
   glUniform1f( shaders.texturesdf.m,
                ( 2.0 * tex_vmax( texture ) * ( w + 2. ) / tex_w( texture ) ) );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texturesdf.vertex );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );
}
#endif

/**
 * @brief Texture blitting backend for interpolated texture.
 *
 *    @param ta Texture id A to blit.
 *    @param tb Texture id B to blit.
 *    @param sa Sampler for A.
 *    @param sb Sampler for B.
 *    @param inter Amount of interpolation to do.
 *    @param projection Projection matrix to use.
 *    @param tex_mat Texture matrix to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderTextureInterpolateRawH( GLuint ta, GLuint tb, GLuint sa,
                                      GLuint sb, double inter,
                                      const mat4 *projection,
                                      const mat4 *tex_mat, const glColour *c )
{
   /* Corner cases. */
   if ( inter >= 1. )
      return gl_renderTextureRawH( ta, sa, projection, tex_mat, c );
   else if ( inter <= 0. )
      return gl_renderTextureRawH( tb, sb, projection, tex_mat, c );

   /* Must have colour for now. */
   if ( c == NULL )
      c = &cWhite;

   glUseProgram( shaders.texture_interpolate.program );

   /* Bind the textures. */
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, tb );
   if ( sb > 0 )
      glBindSampler( 1, sb );
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, ta );
   if ( sa > 0 )
      glBindSampler( 0, sa );
   /* Always end with TEXTURE0 active. */

   /* Set the vertex. */
   glEnableVertexAttribArray( shaders.texture_interpolate.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture_interpolate.vertex,
                               0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   glUniform1i( shaders.texture_interpolate.sampler1, 0 );
   glUniform1i( shaders.texture_interpolate.sampler2, 1 );
   gl_uniformColour( shaders.texture_interpolate.colour, c );
   glUniform1f( shaders.texture_interpolate.inter, inter );
   gl_uniformMat4( shaders.texture_interpolate.projection, projection );
   gl_uniformMat4( shaders.texture_interpolate.tex_mat, tex_mat );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture_interpolate.vertex );

   if ( sb > 0 )
      glBindSampler( 1, 0 );
   if ( sa > 0 )
      glBindSampler( 0, 0 );

   /* anything failed? */
   gl_checkErr();

   glUseProgram( 0 );
}

/**
 * @brief Texture blitting backend for interpolated texture.
 *
 * Value blitted is  ta*inter + tb*(1.-inter).
 *
 *    @param ta Texture A to blit.
 *    @param tb Texture B to blit.
 *    @param inter Amount of interpolation to do.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param w Width on the screen. (units pixels)
 *    @param h Height on the screen. (units pixels)
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param tw Texture width.
 *    @param th Texture height.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderTextureInterpolate( const glTexture *ta, const glTexture *tb,
                                  double inter, double x, double y, double w,
                                  double h, double tx, double ty, double tw,
                                  double th, const glColour *c )
{
   mat4 projection, tex_mat;

   /* Case no need for interpolation. */
   if ( tb == NULL )
      return gl_renderTexture( ta, x, y, w, h, tx, ty, tw, th, c, 0. );
   else if ( ta == NULL )
      return gl_renderTexture( tb, x, y, w, h, tx, ty, tw, th, c, 0. );

   projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, x, y, w, h );
   tex_mat = mat4_identity();
   mat4_translate_scale_xy( &tex_mat, tx, ty, tw, th );

   return gl_renderTextureInterpolateRawH( tex_tex( ta ), tex_tex( tb ),
                                           tex_sampler( ta ), tex_sampler( tb ),
                                           inter, &projection, &tex_mat, c );
}

/**
 * @brief Blits a sprite, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSprite( const glTexture *sprite, double bx, double by, int sx,
                      int sy, const glColour *c )
{
   double x, y, w, h, tx, ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx, by );

   /* Scaled sprite dimensions. */
   w = tex_sw( sprite ) * z;
   h = tex_sh( sprite ) * z;

   /* Check if inbounds. */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* Correct location. */
   x -= w * 0.5;
   y -= h * 0.5;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   gl_renderTexture( sprite, x, y, w, h, tx, ty, tex_srw( sprite ),
                     tex_srh( sprite ), c, 0. );
}

/**
 * @brief Blits a sprite, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew Scaling of width.
 *    @param scaleh Scaling of height.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteScale( const glTexture *sprite, double bx, double by,
                           double scalew, double scaleh, int sx, int sy,
                           const glColour *c )
{
   double x, y, w, h, tx, ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx, by );

   /* Scaled sprite dimensions. */
   w = tex_sw( sprite ) * z * scalew;
   h = tex_sh( sprite ) * z * scaleh;

   /* check if inbounds */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* Correct location. */
   x -= w * 0.5;
   y -= h * 0.5;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   gl_renderTexture( sprite, x, y, w, h, tx, ty, tex_srw( sprite ),
                     tex_srh( sprite ), c, 0. );
}

/**
 * @brief Blits a sprite, position is relative to the player with rotation.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param angle Angle to rotate when rendering.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteRotate( const glTexture *sprite, double bx, double by,
                            double angle, int sx, int sy, const glColour *c )
{
   double x, y, w, h, tx, ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx, by );

   /* Scaled sprite dimensions. */
   w = tex_sw( sprite ) * z;
   h = tex_sh( sprite ) * z;

   /* check if inbounds */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* Correct location. */
   x -= w * 0.5;
   y -= h * 0.5;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   gl_renderTexture( sprite, x, y, w, h, tx, ty, tex_srw( sprite ),
                     tex_srh( sprite ), c, angle );
}

/**
 * @brief Blits a sprite, position is relative to the player with scaling and
 * rotation.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew Scaling of width.
 *    @param scaleh Scaling of height.
 *    @param angle Angle to rotate when rendering.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteScaleRotate( const glTexture *sprite, double bx, double by,
                                 double scalew, double scaleh, double angle,
                                 int sx, int sy, const glColour *c )
{
   double x, y, w, h, tx, ty, z;

   /* Translate coords. */
   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, bx, by );

   /* Scaled sprite dimensions. */
   w = tex_sw( sprite ) * z * scalew;
   h = tex_sh( sprite ) * z * scaleh;

   /* check if inbounds */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* Correct location. */
   x -= w * 0.5;
   y -= h * 0.5;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   gl_renderTexture( sprite, x, y, w, h, tx, ty, tex_srw( sprite ),
                     tex_srh( sprite ), c, angle );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteInterpolate( const glTexture *sa, const glTexture *sb,
                                 double inter, double bx, double by, int sx,
                                 int sy, const glColour *c )
{
   gl_renderSpriteInterpolateScale( sa, sb, inter, bx, by, 1., 1., sx, sy, c );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param scalew X scale factor.
 *    @param scaleh Y scale factor.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderSpriteInterpolateScale( const glTexture *sa, const glTexture *sb,
                                      double inter, double bx, double by,
                                      double scalew, double scaleh, int sx,
                                      int sy, const glColour *c )
{
   double x, y, w, h, tx, ty, z;

   /* Translate coords. */
   gl_gameToScreenCoords( &x, &y, bx, by );

   /* Scaled sprite dimensions. */
   z = cam_getZoom();
   w = tex_sw( sa ) * z * scalew;
   h = tex_sh( sa ) * z * scaleh;

   /* check if inbounds */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* Correct location. */
   x -= w * 0.5;
   y -= h * 0.5;

   /* texture coords */
   tx = tex_sw( sa ) * (double)( sx ) / tex_w( sa );
   ty = tex_sh( sa ) * ( tex_sy( sa ) - (double)sy - 1 ) / tex_h( sa );

   gl_renderTextureInterpolate( sa, sb, inter, x, y, w, h, tx, ty,
                                tex_srw( sa ), tex_srh( sa ), c );
}

/**
 * @brief Blits a sprite, position is in absolute screen coordinates.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSprite( const glTexture *sprite, double bx, double by,
                            int sx, int sy, const glColour *c )
{
   double x, y, tx, ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   /* actual blitting */
   gl_renderTexture( sprite, x, y, tex_sw( sprite ), tex_sh( sprite ), tx, ty,
                     tex_srw( sprite ), tex_srh( sprite ), c, 0. );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSpriteInterpolate( const glTexture *sa, const glTexture *sb,
                                       double inter, double bx, double by,
                                       int sx, int sy, const glColour *c )
{
   gl_renderStaticSpriteInterpolateScale( sa, sb, inter, bx, by, 1., 1., sx, sy,
                                          c );
}

/**
 * @brief Blits a sprite interpolating, position is relative to the player.
 *
 * Since position is in "game coordinates" it is subject to all
 * sorts of position transformations.
 *
 * Interpolation is:  sa*inter + sb*1.-inter)
 *
 *    @param sa Sprite A to blit.
 *    @param sb Sprite B to blit.
 *    @param inter Amount to interpolate.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param scalew X scale factor.
 *    @param scaleh Y scale factor.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStaticSpriteInterpolateScale( const glTexture *sa,
                                            const glTexture *sb, double inter,
                                            double bx, double by, double scalew,
                                            double scaleh, int sx, int sy,
                                            const glColour *c )
{
   double x, y, w, h, tx, ty;

   x = bx;
   y = by;

   /* Scaled sprite dimensions. */
   w = tex_sw( sa ) * scalew;
   h = tex_sh( sa ) * scaleh;

   /* check if inbounds */
   if ( ( x < -w ) || ( x > SCREEN_W + w ) || ( y < -h ) ||
        ( y > SCREEN_H + h ) )
      return;

   /* texture coords */
   tx = tex_sw( sa ) * (double)( sx ) / tex_w( sa );
   ty = tex_sh( sa ) * ( tex_sy( sa ) - (double)sy - 1 ) / tex_h( sa );

   gl_renderTextureInterpolate( sa, sb, inter, x, y, w, h, tx, ty,
                                tex_srw( sa ), tex_srh( sa ), c );
}

/**
 * @brief Blits a scaled sprite, position is in absolute screen coordinates.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param bw Width of sprite to render at.
 *    @param bh Height of sprite to render at.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScaleSprite( const glTexture *sprite, double bx, double by,
                           int sx, int sy, double bw, double bh,
                           const glColour *c )
{
   double x, y, tx, ty;

   x = bx;
   y = by;

   /* texture coords */
   tx = tex_sw( sprite ) * (double)( sx ) / tex_w( sprite );
   ty = tex_sh( sprite ) * ( tex_sy( sprite ) - (double)sy - 1 ) /
        tex_h( sprite );

   /* actual blitting */
   gl_renderTexture( sprite, x, y, bw, bh, tx, ty, tex_srw( sprite ),
                     tex_srh( sprite ), c, 0. );
}

/**
 * @brief Blits a texture scaling it.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScale( const glTexture *texture, double bx, double by, double bw,
                     double bh, const glColour *c )
{
   double x, y, tx, ty;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* texture dimensions */
   tx = ty = 0.;

   /* Actual blitting. */
   gl_renderTexture( texture, x, y, bw, bh, tx, ty, tex_srw( texture ),
                     tex_srh( texture ), c, 0. );
}

/**
 * @brief Blits a texture scaling it to fit a rectangle, but conserves aspect
 * ratio.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderScaleAspect( const glTexture *texture, double bx, double by,
                           double bw, double bh, const glColour *c )
{
   double scale;
   double nw, nh;

   scale = MIN( bw / tex_w( texture ), bh / tex_h( texture ) );

   nw = scale * tex_w( texture );
   nh = scale * tex_h( texture );

   bx += ( bw - nw ) * 0.5;
   by += ( bh - nh ) * 0.5;

   gl_renderScale( texture, bx, by, nw, nh, c );
}

#if 0
/**
 * @brief Blits a texture scaling it to fit a rectangle, but conserves aspect
 * ratio using expensive filtering with the "Magic" Kernel. Should ideally be
 * cached and not run every frame.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 */
void gl_renderScaleAspectMagic( const glTexture *texture, double bx, double by,
                                double bw, double bh )
{
   double scale;
   double nw, nh;

   scale = MIN( bw / tex_w( texture ), bh / tex_h( texture ) );
   if ( scale < gl_screen.scale )
      return gl_renderScaleAspect( texture, bx, by, bw, bh, NULL );

   nw = scale * tex_w( texture );
   nh = scale * tex_h( texture );

   bx += ( bw - nw ) * 0.5;
   by += ( bh - nh ) * 0.5;

   /* Set the vertex. */
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, bx, by, bw, bh );

   glUseProgram( shaders.resize.program );
   glBindTexture( GL_TEXTURE_2D, tex_tex( texture ) );
   GLuint sampler = tex_sampler( texture );
   if ( sampler > 0 )
      glBindSampler( 0, sampler );
   glEnableVertexAttribArray( shaders.resize.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.resize.vertex, 0, 2,
                               GL_FLOAT, 0 );

   mat4 tex_mat = mat4_identity();

   glUniform1f( shaders.resize.u_scale, scale );
   glUniform1f( shaders.resize.u_radius, 8.0 );
   gl_uniformMat4( shaders.resize.projection, &projection );
   gl_uniformMat4( shaders.resize.tex_mat, &tex_mat );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glDisableVertexAttribArray( shaders.resize.vertex );
   glUseProgram( 0 );
   glBindTexture( GL_TEXTURE_2D, 0 );
   if ( sampler > 0 )
      glBindSampler( 0, 0 );
   gl_checkErr();
}
#endif

/**
 * @brief Blits a texture to a position
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_renderStatic( const glTexture *texture, double bx, double by,
                      const glColour *c )
{
   double x, y;

   /* here we use absolute coords */
   x = bx;
   y = by;

   /* actual blitting */
   gl_renderTexture( texture, x, y, tex_sw( texture ), tex_sh( texture ), 0.,
                     0., tex_srw( texture ), tex_srh( texture ), c, 0. );
}

/**
 * @brief Renders a simple shader.
 *
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param r Rotation or 0. to disable.
 *    @param shd Shader to render.
 *    @param c Colour to use or NULL if not necessary.
 *    @param centre Whether or not to centre the shader on the position and use
 * [-1,1] coordinates or set bottom-left and use [0,1] coordinates.
 */
void gl_renderShader( double x, double y, double w, double h, double r,
                      const SimpleShader *shd, const glColour *c, int centre )
{
   mat4 projection = gl_view_matrix;
   mat4_translate_xy( &projection, x, y );
   if ( r != 0. )
      mat4_rotate2d( &projection, r );
   mat4_scale_xy( &projection, w, h );
   glUniform2f( shd->dimensions, w, h );
   gl_renderShaderH( shd, &projection, c, centre );
}

/**
 * @brief Renders a simple shader with a transformation.
 *
 *    @param shd Shader to render.
 *    @param H Transformation matrix.
 *    @param c Colour to use or NULL if not necessary.
 *    @param centre Whether or not to centre the shader on the position and use
 * [-1,1] coordinates or set bottom-left and use [0,1] coordinates.
 */
void gl_renderShaderH( const SimpleShader *shd, const mat4 *H,
                       const glColour *c, int centre )
{
   glEnableVertexAttribArray( shd->vertex );
   gl_vboActivateAttribOffset( centre ? gl_circleVBO : gl_squareVBO,
                               shd->vertex, 0, 2, GL_FLOAT, 0 );

   if ( c != NULL )
      gl_uniformColour( shd->colour, c );

   gl_uniformMat4( shd->projection, H );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glDisableVertexAttribArray( shd->vertex );
   glUseProgram( 0 );
   gl_checkErr();
}

/**
 * @brief Draws a circle.
 *
 *    @param cx X position of the centre in screen coordinates.
 *    @param cy Y position of the centre in screen coordinates.
 *    @param r Radius of the circle.
 *    @param c Colour to use.
 *    @param filled Whether or not it should be filled.
 */
void gl_renderCircle( double cx, double cy, double r, const glColour *c,
                      int filled )
{
   /* Set the vertex. */
   mat4 projection = gl_view_matrix;
   mat4_translate_scale_xy( &projection, cx, cy, r, r );

   /* Draw! */
   gl_renderCircleH( &projection, c, filled );
}

/**
 * @brief Draws a circle.
 *
 *    @param H Transformation matrix to draw the circle.
 *    @param c Colour to use.
 *    @param filled Whether or not it should be filled.
 */
void gl_renderCircleH( const mat4 *H, const glColour *c, int filled )
{
   // TODO handle shearing and different x/y scaling
   GLfloat r = H->m[0][0] / gl_view_matrix.m[0][0];

   glUseProgram( shaders.circle.program );
   glUniform2f( shaders.circle.dimensions, r, r );
   glUniform1i( shaders.circle.parami, filled );
   gl_renderShaderH( &shaders.circle, H, c, 1 );
}

/**
 * @brief Draws a line.
 *
 *    @param x1 X position of the first point in screen coordinates.
 *    @param y1 Y position of the first point in screen coordinates.
 *    @param x2 X position of the second point in screen coordinates.
 *    @param y2 Y position of the second point in screen coordinates.
 *    @param c Colour to use.
 */
void gl_renderLine( double x1, double y1, double x2, double y2,
                    const glColour *c )
{
   double a = atan2( y2 - y1, x2 - x1 );
   double s = hypotf( x2 - x1, y2 - y1 );

   glUseProgram( shaders.sdfsolid.program );
   glUniform1f( shaders.sdfsolid.paramf, 1. ); /* No outline. */
   gl_renderShader( ( x1 + x2 ) * 0.5, ( y1 + y2 ) * 0.5, s * 0.5 + 0.5, 1.0, a,
                    &shaders.sdfsolid, c, 1 );
}

/**
 * @brief Sets up 2d clipping planes around a rectangle.
 *
 *    @param x X position of the rectangle.
 *    @param y Y position of the rectangle.
 *    @param w Width of the rectangle.
 *    @param h Height of the rectangle.
 */
void gl_clipRect( int x, int y, int w, int h )
{
   double rx, ry, rw, rh;
   rx = x / gl_screen.mxscale;
   ry = y / gl_screen.myscale;
   rw = w / gl_screen.mxscale;
   rh = h / gl_screen.myscale;
   glScissor( rx, ry, rw, rh );
   glEnable( GL_SCISSOR_TEST );
}

/**
 * @brief Clears the 2d clipping planes.
 */
void gl_unclipRect( void )
{
   glDisable( GL_SCISSOR_TEST );
   glScissor( 0, 0, gl_screen.rw, gl_screen.rh );
}

/**
 * @brief Initializes the OpenGL rendering routines.
 *
 *    @return 0 on success.
 */
int gl_initRender( void )
{
   GLfloat vertex[10];

   /* Initialize the VBO. */
   gl_renderVBO = gl_vboCreateStream(
      sizeof( GLfloat ) * OPENGL_RENDER_VBO_SIZE * ( 2 + 2 + 4 ), NULL );
   gl_renderVBOtexOffset = sizeof( GLfloat ) * OPENGL_RENDER_VBO_SIZE * 2;
   gl_renderVBOcolOffset =
      sizeof( GLfloat ) * OPENGL_RENDER_VBO_SIZE * ( 2 + 2 );

   vertex[0]    = 0.;
   vertex[1]    = 1.;
   vertex[2]    = 1.;
   vertex[3]    = 1.;
   vertex[4]    = 0.;
   vertex[5]    = 0.;
   vertex[6]    = 1.;
   vertex[7]    = 0.;
   gl_squareVBO = gl_vboCreateStatic( sizeof( GLfloat ) * 8, vertex );
   gl_vboLabel( gl_squareVBO, "C Square VBO" );

   vertex[0]    = -1.;
   vertex[1]    = -1.;
   vertex[2]    = 1.;
   vertex[3]    = -1.;
   vertex[4]    = -1.;
   vertex[5]    = 1.;
   vertex[6]    = 1.;
   vertex[7]    = 1.;
   gl_circleVBO = gl_vboCreateStatic( sizeof( GLfloat ) * 8, vertex );
   gl_vboLabel( gl_circleVBO, "C Circle VBO" );

   vertex[0]         = 0.;
   vertex[1]         = 0.;
   vertex[2]         = 1.;
   vertex[3]         = 0.;
   vertex[4]         = 1.;
   vertex[5]         = 1.;
   vertex[6]         = 0.;
   vertex[7]         = 1.;
   vertex[8]         = 0.;
   vertex[9]         = 0.;
   gl_squareEmptyVBO = gl_vboCreateStatic( sizeof( GLfloat ) * 10, vertex );
   gl_vboLabel( gl_squareEmptyVBO, "C Square Empty VBO" );

   vertex[0]  = 0.;
   vertex[1]  = 0.;
   vertex[2]  = 1.;
   vertex[3]  = 0.;
   gl_lineVBO = gl_vboCreateStatic( sizeof( GLfloat ) * 4, vertex );
   gl_vboLabel( gl_lineVBO, "C Line VBO" );

   vertex[0]      = 0.5 * cos( 4. * M_PI / 3. );
   vertex[1]      = 0.5 * sin( 4. * M_PI / 3. );
   vertex[2]      = 0.5 * cos( 0. );
   vertex[3]      = 0.5 * sin( 0. );
   vertex[4]      = 0.5 * cos( 2. * M_PI / 3. );
   vertex[5]      = 0.5 * sin( 2. * M_PI / 3. );
   vertex[6]      = vertex[0];
   vertex[7]      = vertex[1];
   gl_triangleVBO = gl_vboCreateStatic( sizeof( GLfloat ) * 8, vertex );
   gl_vboLabel( gl_triangleVBO, "C Triangle VBO" );

   gl_checkErr();

   return 0;
}

/**
 * @brief Cleans up the OpenGL rendering routines.
 */
void gl_exitRender( void )
{
   /* Destroy the VBO. */
   gl_vboDestroy( gl_renderVBO );
   gl_vboDestroy( gl_squareVBO );
   gl_vboDestroy( gl_circleVBO );
   gl_vboDestroy( gl_squareEmptyVBO );
   gl_vboDestroy( gl_lineVBO );
   gl_vboDestroy( gl_triangleVBO );
   gl_renderVBO = NULL;
}
