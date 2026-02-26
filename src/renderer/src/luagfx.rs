use crate::TextureUniform;
use crate::Uniform as ContextUniform;
use crate::VertexArray;
use crate::colour::Colour;
use crate::framebuffer::{FramebufferBuilder, FramebufferWrap};
use crate::luashader;
use crate::sdf::{CircleHollowUniform, CircleUniform};
use crate::texture::Texture;
use crate::{Context, SolidUniform, Vec2, camera, colour};
use glow::HasContext;
use mlua::{BorrowedStr, MultiValue, UserData, UserDataMethods, UserDataRef, Variadic};
use nalgebra::{Matrix3, Vector2, Vector4};
use physics::transform2::Transform2;

/// Converts a blend-equation string to a GL constant.
fn blend_func_from_str(s: &str) -> mlua::Result<u32> {
   match s {
      "add" => Ok(glow::FUNC_ADD),
      "subtract" => Ok(glow::FUNC_SUBTRACT),
      "reverse_subtract" => Ok(glow::FUNC_REVERSE_SUBTRACT),
      "min" => Ok(glow::MIN),
      "max" => Ok(glow::MAX),
      _ => Err(mlua::Error::RuntimeError(format!(
         "unknown blend equation '{s}'"
      ))),
   }
}

/// Converts a blend-factor string to a GL constant.
fn blend_factor_from_str(s: &str) -> mlua::Result<u32> {
   match s {
      "zero" => Ok(glow::ZERO),
      "one" => Ok(glow::ONE),
      "src_color" | "src_colour" => Ok(glow::SRC_COLOR),
      "one_minus_src_color" | "one_minus_src_colour" => Ok(glow::ONE_MINUS_SRC_COLOR),
      "dst_color" | "dst_colour" => Ok(glow::DST_COLOR),
      "one_minus_dst_color" | "one_minus_dst_colour" => Ok(glow::ONE_MINUS_DST_COLOR),
      "src_alpha" => Ok(glow::SRC_ALPHA),
      "one_minus_src_alpha" => Ok(glow::ONE_MINUS_SRC_ALPHA),
      "dst_alpha" => Ok(glow::DST_ALPHA),
      "one_minus_dst_alpha" => Ok(glow::ONE_MINUS_DST_ALPHA),
      "constant_color" | "constant_colour" => Ok(glow::CONSTANT_COLOR),
      "one_minus_constant_color" | "one_minus_constant_colour" => {
         Ok(glow::ONE_MINUS_CONSTANT_COLOR)
      }
      "constant_alpha" => Ok(glow::CONSTANT_ALPHA),
      "one_minus_constant_alpha" => Ok(glow::ONE_MINUS_CONSTANT_ALPHA),
      "src_alpha_saturate" => Ok(glow::SRC_ALPHA_SATURATE),
      _ => Err(mlua::Error::RuntimeError(format!(
         "unknown blend factor '{s}'"
      ))),
   }
}

/// Empty struct
pub struct LuaGfx;

/*@
 * @brief Lua bindings to interact with rendering and the Naev graphical
 * environment.
 *
 * An example would be:
 * @code
 * t  = tex.open( "foo/bar.png" ) -- Loads the texture
 * gfx.renderTex( t, 0., 0. ) -- Draws texture at origin
 * @endcode
 *
 * @lua_mod gfx
 */
impl UserData for LuaGfx {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Gets the dimensions of the Naev window.
       *
       * @usage screen_w, screen_h = gfx.dim()
       *
       * GUI modifications to the screen size.
       *    @luatreturn number The width of the Naev window.
       *    @luatreturn number The height of the Naev window.
       *    @luatreturn scale The scaling factor.
       * @luafunc dim
       */
      methods.add_function("dim", |_, ()| -> mlua::Result<(f32, f32, f32)> {
         let ctx = Context::get();
         let dims = ctx.dimensions.read().unwrap();
         Ok((dims.view_width, dims.view_height, dims.view_scale))
      });
      /*@
       * @brief Gets the screen coordinates from game coordinates.
       *
       *    @luatparam Vec2 Vector of coordinates to transform.
       *    @luatreturn Vec2 Transformed vector.
       * @luafunc screencoords
       */
      methods.add_function("screencoords", |_, pos: Vec2| -> mlua::Result<Vec2> {
         Ok(camera::CAMERA
            .read()
            .unwrap()
            .game_to_screen_coords_yflip(pos.into())
            .into())
      });
      /*@
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
      methods.add_function(
         "renderTex",
         |_,
          (tex, x, y, sx, sy, col): (
            UserDataRef<crate::texture::Texture>,
            f32,
            f32,
            Option<usize>,
            Option<usize>,
            Option<colour::Colour>,
         )|
          -> mlua::Result<()> {
            let sx = sx.unwrap_or(1) - 1;
            let sy = sy.unwrap_or(1) - 1;
            let w = tex.texture.w as f32;
            let h = tex.texture.h as f32;
            #[rustfmt::skip]
            let transform: Matrix3<f32> = Matrix3::new(
               w,   0.0,  x,
               0.0,  h,   y,
               0.0, 0.0, 1.0,
            );
            let tw = tex.sw as f32;
            let th = tex.sh as f32;
            let tx = tw * (sx as f32) / w;
            let ty = th * ((tex.sy as f32) - (sy as f32) - 1.0) / h;
            #[rustfmt::skip]
            let texture: Matrix3<f32> = Matrix3::new(
               tw,  0.0, tx,
               0.0, th,  ty,
               0.0, 0.0, 1.0,
            );
            let colour = col.unwrap_or(colour::WHITE);
            let data = TextureUniform {
               texture: texture.into(),
               transform: transform.into(),
               colour,
            };
            Ok(tex.draw_ex(Context::get(), &data)?)
         },
      );
      /*@
       * @brief Renders a texture using the core render function.
       *
       * This function is far more complex than `renderTex`, however it allows much
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
      #[allow(clippy::type_complexity)]
      methods.add_function(
         "renderTexRaw",
         |_,
          (tex, x, y, w, h, sx, sy, tx, ty, tw, th, col, angle): (
            UserDataRef<crate::texture::Texture>,
            f32,
            f32,
            f32,
            f32,
            Option<usize>,
            Option<usize>,
            Option<f32>,
            Option<f32>,
            Option<f32>,
            Option<f32>,
            Option<colour::Colour>,
            Option<f32>,
         )|
          -> mlua::Result<()> {
            let sx = sx.unwrap_or(1) - 1;
            let sy = sy.unwrap_or(1) - 1;
            let tx = tx.unwrap_or(0.0);
            let ty = ty.unwrap_or(0.0);
            let tw = tw.unwrap_or(1.0);
            let th = th.unwrap_or(1.0);
            #[rustfmt::skip]
            let transform: Matrix3<f32> = {
               if let Some(angle) = angle {
                  let hw = 0.5 * w;
                  let hh = 0.5 * h;
                  let c = angle.cos();
                  let s = angle.sin();
                  Matrix3::new(
                     1.0, 0.0, x + hw,
                     0.0, 1.0, y + hh,
                     0.0, 0.0, 1.0,
                  ) * Matrix3::new(
                     c,   -s,  0.0,
                     s,    c,  0.0,
                     0.0, 0.0, 1.0,
                  ) * Matrix3::new(
                     w,   0.0, -hw,
                     0.0,  h,  -hh,
                     0.0, 0.0, 1.0,
                  )
               } else {
                  Matrix3::new(
                     w,   0.0,  x,
                     0.0,  h,   y,
                     0.0, 0.0, 1.0,
                  )
               }
            };
            let sw = tex.sw as f32;
            let sh = tex.sh as f32;
            let mut tx = tx * sw + sw * (sx as f32) / tex.texture.w as f32;
            let mut ty =
               ty * sh + sh * ((tex.sy as f32) - (sy as f32) - 1.0) / tex.texture.h as f32;
            let tw = tw * tex.srw as f32;
            let th = th * tex.srh as f32;
            if tw < 0.0 {
               tx -= sw;
            }
            if th < 0.0 {
               ty -= sh;
            }
            #[rustfmt::skip]
            let texture: Matrix3<f32> = Matrix3::new(
               tw,  0.0, tx,
               0.0, th,  ty,
               0.0, 0.0, 1.0,
            );
            let colour = col.unwrap_or(colour::WHITE);
            let data = TextureUniform {
               texture: texture.into(),
               transform: transform.into(),
               colour,
            };
            Ok(tex.draw_ex(Context::get(), &data)?)
         },
      );
      /*@
       * @brief Renders a texture using a transformation matrix and a custom shader.
       *
       *    @luatparam Tex tex Texture to render.
       *    @luatparam Shader shader Shader to use when rendering.
       *    @luatparam Transform H Transformation matrix (ClipSpaceFromLocal).
       *    @luatparam[opt=white] Colour colour
       *    @luatparam[opt=identity] Transform TH Texture-space transform
       * (ViewSpaceFromLocal for the tex-coord channel).
       * @luafunc renderTexH
       */
      methods.add_function(
         "renderTexH",
         |_,
          (tex, shader, transform, col, transform_texture): (
            UserDataRef<Texture>,
            UserDataRef<luashader::LuaShader>,
            Transform2,
            Option<Colour>,
            Option<Transform2>,
         )|
          -> mlua::Result<()> {
            let ctx = Context::get();
            let gl = &ctx.gl;
            let colour = col.unwrap_or(colour::WHITE);
            let transform: Matrix3<f32> = transform.into();
            let transform_texture: Matrix3<f32> =
               transform_texture.unwrap_or(Transform2::new()).into();

            let shader = &*shader;
            let (w, h) = {
               let dims = ctx.dimensions.read().unwrap();
               (dims.view_width, dims.view_height)
            };

            let uniform = luashader::UniformBlock {
               view_space_from_local: transform_texture.into(),
               clip_space_from_local: transform.into(),
               constant_colour: colour.into(),
               love_screensize: Vector4::new(w, h, 1.0, 0.0),
            };

            shader.shader.use_program(gl);
            ctx.vao_square.bind(ctx);

            shader.buffer.bind_write_base(
               ctx,
               &uniform.buffer()?,
               luashader::LuaShader::UNIFORM_BLOCK,
            )?;

            // Bind any extra textures the shader registered via `send`
            for slot in &shader.textures {
               if slot.texid != 0 {
                  unsafe {
                     gl.active_texture(slot.active);
                     gl.bind_texture(
                        glow::TEXTURE_2D,
                        Some(glow::NativeTexture(
                           std::num::NonZero::new(slot.texid).unwrap(),
                        )),
                     );
                     gl.uniform_1_i32(Some(&slot.uniform), slot.value);
                     gl.bind_sampler(slot.active - glow::TEXTURE0, Some(tex.sampler));
                  }
               }
            }

            // MainTex is bound last
            if let Some(_maintex) = shader.maintex {
               tex.bind(ctx, 0);
            }

            unsafe {
               gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);

               VertexArray::unbind(ctx);
               gl.use_program(None);
            }
            Ok(())
         },
      );
      /*@
       * @brief Renders a rectangle.
       *
       * @usage gfx.renderRect( 10., 30., 40., 40., col ) -- filled square
       * @usage gfx.renderRect( 10., 30., 40., 40., col, true ) -- hollow square
       *
       *    @luatparam number x X position.
       *    @luatparam number y Y position.
       *    @luatparam number w Width.
       *    @luatparam number h Height.
       *    @luatparam Colour col Colour to use.
       *    @luatparam[opt=false] boolean empty Whether to draw hollow.
       * @luafunc renderRect
       */
      methods.add_function(
         "renderRect",
         |_,
          (x, y, w, h, col, empty): (f32, f32, f32, f32, Colour, Option<bool>)|
          -> mlua::Result<()> {
            let ctx = Context::get();
            let ret = if empty.unwrap_or(false) {
               ctx.sdf.draw_rect_hollow(ctx, x, y, w, h, 0.0, col)
            } else {
               ctx.draw_rect(x, y, w, h, col)
            };
            Ok(ret?)
         },
      );
      /*@
       * @brief Renders a rectangle given a transformation matrix.
       *
       *    @luatparam Transform H Transformation matrix.
       *    @luatparam[opt=white] Colour col Colour to use.
       *    @luatparam[opt=false] boolean empty Whether to draw hollow.
       * @luafunc renderRectH
       */
      methods.add_function(
         "renderRectH",
         |_,
          (transform, col, empty): (Transform2, Option<Colour>, Option<bool>)|
          -> mlua::Result<()> {
            let ctx = Context::get();
            let col = col.unwrap_or(colour::WHITE);
            let dims = ctx.dimensions.read().unwrap();
            let transform: Matrix3<f32> = transform.into();
            let transform = dims.projection * transform;
            if empty.unwrap_or(false) {
               let uniform = crate::sdf::RectHollowUniform {
                  transform: transform.into(),
                  colour: col,
                  dims: Vector2::new(0.0, 0.0),
                  border: 0.0,
               };
               Ok(ctx.sdf.draw_rect_hollow_ex(ctx, &uniform)?)
            } else {
               let uniform = SolidUniform {
                  transform: transform.into(),
                  colour: col,
               };
               Ok(ctx.draw_rect_ex(&uniform)?)
            }
         },
      );
      /*@
       * @brief Renders a circle.
       *
       *    @luatparam number x X position.
       *    @luatparam number y Y position.
       *    @luatparam number r Radius.
       *    @luatparam Colour col Colour to use.
       *    @luatparam[opt=false] boolean empty Whether to draw hollow.
       * @luafunc renderCircle
       */
      methods.add_function(
         "renderCircle",
         |_, (x, y, r, col, empty): (f32, f32, f32, Colour, Option<bool>)| -> mlua::Result<()> {
            let ctx = Context::get();
            let res = if empty.unwrap_or(false) {
               ctx.sdf.draw_circle_hollow(ctx, x, y, r, col, 0.0)
            } else {
               ctx.sdf.draw_circle(ctx, x, y, r, col)
            };
            Ok(res?)
         },
      );
      /*@
       * @brief Renders a circle given a transformation matrix.
       *
       *    @luatparam Transform H Transformation matrix.
       *    @luatparam[opt=white] Colour col Colour to use.
       *    @luatparam[opt=false] boolean empty Whether to draw hollow.
       * @luafunc renderCircleH
       */
      methods.add_function(
         "renderCircleH",
         |_,
          (transform, col, empty): (Transform2, Option<Colour>, Option<bool>)|
          -> mlua::Result<()> {
            let ctx = Context::get();
            let col = col.unwrap_or(colour::WHITE);
            let transform: Matrix3<f32> = transform.into();
            let dims = ctx.dimensions.read().unwrap();
            // TODO handle shearing and different x/y scaling
            let radius = transform.m11;
            let transform = dims.projection * transform;
            if empty.unwrap_or(false) {
               let uniform = CircleHollowUniform {
                  transform: transform.into(),
                  colour: col,
                  radius,
                  border: 0.0,
               };
               Ok(ctx.sdf.draw_circle_hollow_ex(ctx, &uniform)?)
            } else {
               let uniform = CircleUniform {
                  transform: transform.into(),
                  colour: col,
                  radius,
               };
               Ok(ctx.sdf.draw_circle_ex(ctx, &uniform)?)
            }
         },
      );
      /*@
       * @brief Renders a polyline or set of line segments.
       *
       * @usage gfx.renderLinesH( H, col, 50,30, 70,70 )
       * @usage gfx.renderLinesH( H, col, vec2.new(), vec2.new(100,100) )
       *
       *    @luatparam Transform H Transform to use when rendering.
       *    @luatparam Colour col Colour to draw with.
       *    @luatparam number|Vec2 ... Coordinates as x,y pairs or Vec2 values.
       * @luafunc renderLinesH
       */
      methods.add_function("renderLinesH", |_, ()| -> mlua::Result<()> {
         Err(mlua::Error::RuntimeError("unimplemented".to_string()))
      });
      /*@
       * @brief Clears the depth buffer.
       *
       * @luafunc clearDepth
       */
      methods.add_function("clearDepth", |_, ()| -> mlua::Result<()> {
         let ctx = Context::get();
         unsafe {
            ctx.gl.clear(glow::DEPTH_BUFFER_BIT);
         }
         Ok(())
      });
      /*@
       * @brief Gets the size of the default or small font.
       *
       *    @luatparam[opt=false] boolean small Whether to query the small font.
       *    @luatreturn number Height in pixels.
       * @luafunc fontSize
       */
      methods.add_function("fontSize", |_, small: Option<bool>| -> mlua::Result<f32> {
         if small.unwrap_or(false) {
            Ok(unsafe { naevc::gl_smallFont.h as f32 })
         } else {
            Ok(unsafe { naevc::gl_defFont.h as f32 })
         }
      });
      /*@
       * @brief Gets the rendered width (or block height) of a string using a
       * font object.
       *
       *    @luatparam font font Font to use.
       *    @luatparam string str Text to measure.
       *    @luatparam[opt] int width Block width; if given, returns height instead.
       *    @luatreturn number Width or height in pixels.
       * @luafunc printfDim
       */
      methods.add_function(
         "printfDim",
         |_, (_font, _str, _width): (mlua::Value, mlua::Value, Option<i32>)| -> mlua::Result<f32> {
            // TODO
            Ok(0.0)
         },
      );
      /*@
       * @brief Gets the (x, y) position at which text would stop printing.
       *
       *    @luatparam font font Font to use.
       *    @luatparam string str Text.
       *    @luatparam int width Maximum width.
       *    @luatreturn number x
       *    @luatreturn number y
       * @luafunc printfEnd
       */
      methods.add_function(
         "printfEnd",
         |_, (_font, _str, _width): (mlua::Value, mlua::Value, i32)| -> mlua::Result<(i32, i32)> {
            // TODO
            Ok((0, 0))
         },
      );
      /*@
       * @brief Wraps text into lines at the given width.
       *
       *    @luatparam font font Font to use.
       *    @luatparam string str Text to wrap.
       *    @luatparam int width Maximum line width.
       *    @luatreturn table Table of {string, width} pairs.
       *    @luatreturn number Maximum line width.
       * @luafunc printfWrap
       */
      methods.add_function(
         "printfWrap",
         |lua,
          (_font, _str, _width): (mlua::Value, mlua::Value, i32)|
          -> mlua::Result<(mlua::Table, i32)> {
            // TODO
            Ok((lua.create_table()?, 0))
         },
      );
      /*@
       * @brief Clears the saved internal colour state.
       * @luafunc printRestoreClear
       */
      methods.add_function("printRestoreClear", |_, ()| -> mlua::Result<()> {
         // TODO
         Ok(())
      });
      /*@
       * @brief Restores the last saved internal colour state.
       * @luafunc printRestoreLast
       */
      methods.add_function("printRestoreLast", |_, ()| -> mlua::Result<()> {
         // TODO
         Ok(())
      });
      /*@
       * @brief Prints text using a font object.
       *
       *    @luatparam font font Font to use.
       *    @luatparam string str String to print.
       *    @luatparam number x X position.
       *    @luatparam number y Y position.
       *    @luatparam Colour col Colour.
       *    @luatparam[opt] int max Maximum render width.
       *    @luatparam[opt] boolean centre Whether to centre.
       * @luafunc printf
       */
      methods.add_function(
         "printf",
         |_,
          (_font, _str, _x, _y, _col, _max, _centre): (
            mlua::Value,
            mlua::Value,
            f32,
            f32,
            mlua::Value,
            Option<i32>,
            Option<bool>,
         )|
          -> mlua::Result<()> {
            // TODO
            Ok(())
         },
      );
      /*@
       * @brief Prints text using a transformation matrix.
       *
       *    @luatparam Transform H Transformation matrix.
       *    @luatparam font font Font to use.
       *    @luatparam string str String to print.
       *    @luatparam[opt=white] Colour col Colour.
       *    @luatparam[opt=0] number outline Outline width.
       * @luafunc printH
       */
      methods.add_function(
         "printH",
         |_,
          (_h, _font, _str, _col, _outline): (
            mlua::Value,
            mlua::Value,
            mlua::Value,
            Option<mlua::Value>,
            Option<f32>,
         )|
          -> mlua::Result<()> {
            // TODO
            Ok(())
         },
      );
      /*@
       * @brief Gets the rendered width (or block height) of a string.
       *
       *    @luatparam boolean small Whether to use the small font.
       *    @luatparam string str Text to measure.
       *    @luatparam[opt] int width Block width; if given, returns height instead.
       *    @luatreturn number Width or height in pixels.
       * @luafunc printDim
       */
      methods.add_function(
         "printDim",
         |_, (_small, _text, _width): (bool, BorrowedStr, Option<i32>)| -> mlua::Result<f32> {
            // TODO
            Ok(0.0)
         },
      );
      /*@
       * @brief Prints text on the screen.
       *
       *    @luatparam boolean small Whether to use the small font.
       *    @luatparam string str String to print.
       *    @luatparam number x X position.
       *    @luatparam number y Y position.
       *    @luatparam Colour col Colour.
       *    @luatparam[opt] int max Maximum width.
       *    @luatparam[opt] boolean centre Whether to centre.
       * @luafunc print
       */
      methods.add_function(
         "print",
         |_,
          (_small, _str, _x, _y, _col, _max, _centre): (
            bool,
            mlua::Value,
            f32,
            f32,
            mlua::Value,
            Option<i32>,
            Option<bool>,
         )|
          -> mlua::Result<()> {
            // TODO
            Ok(())
         },
      );
      /*@
       * @brief Prints a block of text on the screen.
       *
       *    @luatparam boolean small Whether to use the small font.
       *    @luatparam string str String to print.
       *    @luatparam number x X position.
       *    @luatparam number y Y position.
       *    @luatparam number w Block width.
       *    @luatparam number h Block height.
       *    @luatparam Colour col Colour.
       *    @luatparam[opt=0] number line_height Line height override.
       * @luafunc printText
       */
      methods.add_function(
         "printText",
         |_,
          (_small, _str, _x, _y, _w, _h, _col, _lh): (
            bool,
            mlua::Value,
            f32,
            f32,
            f32,
            f32,
            mlua::Value,
            Option<f32>,
         )|
          -> mlua::Result<()> {
            // TODO
            Ok(())
         },
      );
      /*@
       * @brief Sets the OpenGL blending mode.
       *
       * See https://love2d.org/wiki/love.graphics.setBlendMode
       *
       *    @luatparam string mode One of: `"alpha"`, `"replace"`, `"screen"`,
       * `"add"`, `"subtract"`, `"multiply"`, `"lighten"`, or `"darken"`.
       *    @luatparam[opt="alphamultiply"] string alphamode `"alphamultiply"` or
       * `"premultiplied"`.
       * @luafunc setBlendMode
       */
      methods.add_function(
         "setBlendMode",
         |_, (mode, alphamode): (BorrowedStr, Option<BorrowedStr>)| -> mlua::Result<()> {
            let alphamode = alphamode.as_deref().unwrap_or("alphamultiply");

            let mut func = glow::FUNC_ADD;
            let mut src_rgb = glow::ONE;
            let mut src_a = glow::ONE;
            let mut dst_rgb = glow::ZERO;
            let mut dst_a = glow::ZERO;

            // Set source factors based on alpha mode
            match alphamode {
               "alphamultiply" => {
                  src_rgb = glow::SRC_ALPHA;
                  src_a = glow::SRC_ALPHA;
               }
               "premultiplied" => {
                  if matches!(mode.as_ref(), "lighten" | "darken" | "multiply") {
                     return Err(mlua::Error::RuntimeError(format!(
                        "blend mode '{}' is incompatible with 'premultiplied'",
                        &*mode
                     )));
                  }
               }
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unknown alphamode '{}'",
                     alphamode
                  )));
               }
            }

            match mode.as_ref() {
               "alpha" => {
                  dst_rgb = glow::ONE_MINUS_SRC_ALPHA;
                  dst_a = glow::ONE_MINUS_SRC_ALPHA;
               }
               "multiply" => {
                  src_rgb = glow::DST_COLOR;
                  src_a = glow::DST_COLOR;
               }
               "subtract" => {
                  func = glow::FUNC_REVERSE_SUBTRACT;
               }
               "add" => {
                  func = glow::FUNC_REVERSE_SUBTRACT;
                  src_a = glow::ZERO;
                  dst_rgb = glow::ONE;
                  dst_a = glow::ONE;
               }
               "lighten" => {
                  func = glow::MAX;
               }
               "darken" => {
                  func = glow::MIN;
               }
               "screen" => {
                  dst_rgb = glow::ONE_MINUS_SRC_COLOR;
                  dst_a = glow::ONE_MINUS_SRC_COLOR;
               }
               "replace" => { /* default values already set. */ }
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "unknown blend mode '{}'",
                     &*mode
                  )));
               }
            }

            let ctx = Context::get();
            let gl = &ctx.gl;
            unsafe {
               gl.blend_equation(func);
               gl.blend_func_separate(src_rgb, dst_rgb, src_a, dst_a);
               naevc::render_needsReset();
            }
            Ok(())
         },
      );
      /*@
       * @brief Sets the OpenGL blending state directly.
       *
       * See https://love2d.org/wiki/love.graphics.setBlendState (v0.12)
       *
       * @usage gfx.setBlendState( "add", "src_alpha", "one_minus_src_alpha" )
       * @usage gfx.setBlendState( "add", "add", "src_alpha", "one", "one_minus_src_alpha", "one" )
       *
       * @luafunc setBlendState
       */
      methods.add_function(
         "setBlendState",
         |_, args: mlua::Variadic<BorrowedStr>| -> mlua::Result<()> {
            let ctx = Context::get();
            let gl = &ctx.gl;
            let len = args.len();
            if len == 3 {
               let func_rgb = blend_func_from_str(&args[0])?;
               let src_rgb = blend_factor_from_str(&args[1])?;
               let dst_rgb = blend_factor_from_str(&args[2])?;
               unsafe {
                  gl.blend_equation(func_rgb);
                  gl.blend_func(src_rgb, dst_rgb);
               }
            } else if len == 6 {
               let func_rgb = blend_func_from_str(&args[0])?;
               let func_a = blend_func_from_str(&args[1])?;
               let src_rgb = blend_factor_from_str(&args[2])?;
               let src_a = blend_factor_from_str(&args[3])?;
               let dst_rgb = blend_factor_from_str(&args[4])?;
               let dst_a = blend_factor_from_str(&args[5])?;
               unsafe {
                  gl.blend_equation_separate(func_rgb, func_a);
                  gl.blend_func_separate(src_rgb, dst_rgb, src_a, dst_a);
               }
            } else {
               return Err(mlua::Error::RuntimeError(
                  "setBlendState: need 3 or 6 arguments".into(),
               ));
            }
            unsafe {
               naevc::render_needsReset();
            }
            Ok(())
         },
      );
      /*@
       * @brief Sets scissor clipping, with (0,0) at top-left.
       *
       * Calling with no parameters disables scissoring.
       *
       *    @luatparam number x X position of clipping rectangle.
       *    @luatparam number y Y position of clipping rectangle.
       *    @luatparam number width Width of clipping rectangle.
       *    @luatparam number height Height of clipping rectangle.
       * @luafunc setScissor
       */
      methods.add_function("setScissor", |_, args: Variadic<f32>| -> mlua::Result<()> {
         let ctx = Context::get();
         let gl = &ctx.gl;
         if args.is_empty() {
            // Disable scissoring and reset to full screen
            unsafe {
               gl.disable(glow::SCISSOR_TEST);
               let (rw, rh) = (naevc::gl_screen.rw, naevc::gl_screen.rh);
               gl.scissor(0, 0, rw, rh);
            }
         } else if args.len() == 4 {
            let x = args[0];
            let y = args[1];
            let w = args[2];
            let h = args[3];
            let (mxscale, myscale, rh) = unsafe {
               (
                  naevc::gl_screen.mxscale as f32,
                  naevc::gl_screen.myscale as f32,
                  naevc::gl_screen.rh as f32,
               )
            };
            let rx = x / mxscale;
            let ry = y / myscale;
            let rw = w / mxscale;
            let rheight = h / myscale;

            unsafe {
               gl.scissor(
                  rx as i32,
                  (rh - rheight - ry) as i32,
                  rw as i32,
                  rheight as i32,
               );
               gl.enable(glow::SCISSOR_TEST);
               naevc::render_needsReset()
            }
         }
         Ok(())
      });
      /*@
       * @brief Takes the current rendered screen and returns it as a canvas.
       *
       *    @luatreturn Canvas A canvas containing the screen data.
       * @luafunc screenshot
       */
      methods.add_function("screenshot", |_, ()| -> mlua::Result<FramebufferWrap> {
         let ctx = Context::get();
         let (w, h) = {
            let dims = ctx.dimensions.read().unwrap();
            (dims.pixels_width, dims.pixels_height)
         };

         let fb = FramebufferBuilder::new(Some("Screenshot Canvas"))
            .width(w as usize)
            .height(h as usize)
            .build(ctx)?;

         let gl = &ctx.gl;
         unsafe {
            // Bind screen as READ source
            gl.bind_framebuffer(glow::READ_FRAMEBUFFER, None);
            // Bind our new FB as DRAW destination
            gl.bind_framebuffer(glow::DRAW_FRAMEBUFFER, Some(fb.framebuffer));
            // Blit with vertical flip: src bottom=0 top=rh, dst bottom=tex_h top=0
            gl.blit_framebuffer(
               0,
               0,
               w as i32,
               h as i32,
               0,
               h as i32, // y flipped
               w as i32,
               0,
               glow::COLOR_BUFFER_BIT,
               glow::NEAREST,
            );
            // Restore current FBO
            gl.bind_framebuffer(
               glow::FRAMEBUFFER,
               std::num::NonZero::new(naevc::gl_screen.current_fbo).map(glow::NativeFramebuffer),
            );
         }
         Ok(FramebufferWrap::new(fb))
      });
      /*@
       * @brief Sets the ambient lighting.
       *
       *    @luatparam Colour|number r Colour or red channel.  For a Colour, the
       * alpha channel is used as a radiance multiplier.
       *    @luatparam[opt=r] number g Green channel.
       *    @luatparam[opt=r] number b Blue channel.
       *    @luatparam[opt] number strength Normalise (r,g,b) so total radiance =
       * strength.
       * @luafunc lightAmbient
       */
      methods.add_function("lightAmbient", |_, args: MultiValue| -> mlua::Result<()> {
         // TODO
         let _ = args;
         Ok(())
      });

      // ── lightAmbientGet ─────────────────────────────────────────────────────
      /*@
       * @brief Gets the current ambient lighting values.
       *
       *    @luatreturn number r
       *    @luatreturn number g
       *    @luatreturn number b
       * @luafunc lightAmbientGet
       */
      methods.add_function(
         "lightAmbientGet",
         |_, ()| -> mlua::Result<(f64, f64, f64)> {
            // TODO
            Ok((0.0, 0.0, 0.0))
         },
      );
      /*@
       * @brief Sets the intensity multiplier for non-ambient lights.
       *
       *    @luatparam number intensity Intensity multiplier.
       * @luafunc lightIntensity
       */
      methods.add_function("lightIntensity", |_, _intensity: f64| -> mlua::Result<()> {
         // TODO
         Ok(())
      });
      /*@
       * @brief Gets the current light intensity multiplier.
       *
       *    @luatreturn number Intensity.
       * @luafunc lightIntensityGet
       */
      methods.add_function("lightIntensityGet", |_, ()| -> mlua::Result<f64> {
         // TODO
         Ok(1.0)
      });
      /*@
       * @brief Gets the OpenGL version.
       *
       *    @luatreturn integer Major version.
       *    @luatreturn integer Minor version.
       *    @luatreturn integer GLSL version.
       * @luafunc glVersion
       */
      methods.add_function("glVersion", |_, ()| -> mlua::Result<(i32, i32, i32)> {
         // TODO add fields to Context
         let (major, minor, glsl) = unsafe {
            (
               naevc::gl_screen.major as i32,
               naevc::gl_screen.minor as i32,
               naevc::gl_screen.glsl as i32,
            )
         };
         Ok((major, minor, glsl))
      });
   }
}

pub fn open_gfx(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<LuaGfx>()?;
   Ok(proxy)
}
