use crate::TextureUniform;
use crate::{Context, Vec2, camera, colour};
use mlua::{UserData, UserDataMethods, UserDataRef};
use nalgebra::Matrix3;

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
               texture,
               transform,
               colour: colour.into(),
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
               texture,
               transform,
               colour: colour.into(),
            };
            Ok(tex.draw_ex(Context::get(), &data)?)
         },
      );
   }
}

pub fn open_gfx(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<LuaGfx>()?;
   Ok(proxy)
}
