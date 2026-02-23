use crate::colour::Colour;
use crate::texture::{AddressMode, FilterMode, Texture, TextureBuilder, TextureFormat};
use crate::{Context, ContextWrapper};
use anyhow::Result;
use glow::*;
use mlua::{MetaMethod, UserData, UserDataMethods, UserDataRef};
use nalgebra::Vector4;
use nlog::warn_err;
use std::num::NonZero;
use std::ops::Deref;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};

pub struct FramebufferC {
   fb: glow::NativeFramebuffer,
   w: usize,
   h: usize,
}
#[allow(clippy::large_enum_variant)]
pub enum FramebufferTarget {
   Screen,
   Framebuffer(Framebuffer),
   FramebufferC(FramebufferC),
}
impl FramebufferTarget {
   pub fn from_gl(fb: u32, w: usize, h: usize) -> Self {
      FramebufferTarget::FramebufferC(FramebufferC {
         fb: glow::NativeFramebuffer(NonZero::new(fb).unwrap()),
         w,
         h,
      })
   }

   pub fn dimensions(&self) -> (usize, usize) {
      match self {
         Self::Screen => todo!(),
         Self::Framebuffer(fb) => (fb.w, fb.h),
         Self::FramebufferC(fb) => (fb.w, fb.h),
      }
   }

   pub fn bind(&self, ctx: &Context) {
      let fb = match self {
         Self::Screen => None,
         Self::Framebuffer(fb) => Some(fb.framebuffer),
         Self::FramebufferC(fb) => Some(fb.fb),
      };
      unsafe {
         ctx.gl.bind_framebuffer(glow::FRAMEBUFFER, fb);
      }
   }

   pub fn unbind(&self, ctx: &Context) {
      unsafe {
         ctx.gl.bind_framebuffer(
            glow::FRAMEBUFFER,
            NonZero::new(naevc::gl_screen.current_fbo).map(glow::NativeFramebuffer),
         );
      }
   }
}

#[derive(Debug)]
pub struct Framebuffer {
   pub framebuffer: glow::Framebuffer,
   pub w: usize,
   pub h: usize,
   pub texture: Option<Texture>,
   pub depth: Option<Texture>,
}
impl Drop for Framebuffer {
   fn drop(&mut self) {
      crate::message_push(crate::Message::DeleteFramebuffer(self.framebuffer));
   }
}
impl Framebuffer {
   pub fn bind(&self, ctx: &Context) {
      self.bind_gl(&ctx.gl)
   }

   pub fn bind_gl(&self, gl: &glow::Context) {
      unsafe {
         gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.framebuffer));
      }
   }

   pub fn unbind(ctx: &Context) {
      Framebuffer::unbind_gl(&ctx.gl)
   }
   pub fn unbind_gl(gl: &glow::Context) {
      unsafe {
         gl.bind_framebuffer(
            glow::FRAMEBUFFER,
            NonZero::new(naevc::gl_screen.current_fbo).map(glow::NativeFramebuffer),
         );
      }
   }

   pub fn into_texture(mut self) -> Result<Texture> {
      match self.texture.take() {
         Some(tex) => Ok(tex),
         None => anyhow::bail!("unable to remove texture from framebuffer"),
      }
   }

   pub fn screenshot(&self, ctx: &Context) -> Result<image::DynamicImage> {
      let mut data: Vec<u8> = vec![0; (self.w * self.h * 3) as usize];
      let gl = &ctx.gl;
      self.bind(ctx);
      unsafe {
         gl.read_pixels(
            0,
            0,
            self.w as i32,
            self.h as i32,
            glow::RGB,
            glow::UNSIGNED_BYTE,
            glow::PixelPackData::Slice(Some(&mut data)),
         );
      }
      Self::unbind(&ctx);
      let img = match image::RgbImage::from_vec(self.w as u32, self.h as u32, data) {
         Some(img) => image::DynamicImage::ImageRgb8(img).flipv(),
         None => anyhow::bail!("failed to create ImageBuffer"),
      };
      Ok(img)
   }
}

pub struct FramebufferBuilder {
   name: Option<String>,
   w: usize,
   h: usize,
   texture: bool,
   depth: bool,
   filter: FilterMode,
   address_mode: AddressMode,
}

impl FramebufferBuilder {
   pub fn new(name: Option<&str>) -> Self {
      FramebufferBuilder {
         name: name.map(String::from),
         w: 0,
         h: 0,
         texture: true,
         depth: false,
         filter: FilterMode::Linear,
         address_mode: AddressMode::ClampToEdge,
      }
   }

   pub fn width(mut self, width: usize) -> Self {
      self.w = width;
      self
   }

   pub fn height(mut self, height: usize) -> Self {
      self.h = height;
      self
   }

   pub fn texture(mut self, enable: bool) -> Self {
      self.texture = enable;
      self
   }

   pub fn depth(mut self, enable: bool) -> Self {
      self.depth = enable;
      self
   }

   pub fn filter(mut self, mode: FilterMode) -> Self {
      self.filter = mode;
      self
   }

   pub fn address_mode(mut self, mode: AddressMode) -> Self {
      self.address_mode = mode;
      self
   }

   pub fn name(mut self, name: Option<&str>) -> Self {
      self.name = name.map(String::from);
      self
   }

   pub fn build(self, ctx: &Context) -> Result<Framebuffer> {
      let wctx: ContextWrapper = ctx.into();
      self.build_wrap(&wctx)
   }

   pub fn build_wrap(self, ctx: &ContextWrapper) -> Result<Framebuffer> {
      let texture = if self.texture {
         let name = self.name.as_ref().map(|name| format!("{name}-Texture"));
         let texture = TextureBuilder::new()
            .name(name.as_deref())
            .width(Some(self.w))
            .height(Some(self.h))
            .filter(self.filter)
            .address_mode(self.address_mode)
            .build_wrap(ctx)?;
         Some(texture)
      } else {
         None
      };

      let depth = if self.depth {
         let name = self.name.as_ref().map(|name| format!("{name}-Depth"));
         let depth = TextureBuilder::new()
            .name(name.as_deref())
            .empty(TextureFormat::Depth)
            .width(Some(self.w))
            .height(Some(self.h))
            .filter(self.filter)
            .address_mode(self.address_mode)
            .build_wrap(ctx)?;
         Some(depth)
      } else {
         None
      };

      let lctx = ctx.lock();
      let gl = &lctx.gl;

      let framebuffer = unsafe { gl.create_framebuffer().map_err(|e| anyhow::anyhow!(e)) }?;
      unsafe {
         gl.bind_framebuffer(glow::FRAMEBUFFER, Some(framebuffer));
         if gl.supports_debug() {
            gl.object_label(glow::FRAMEBUFFER, framebuffer.0.into(), self.name);
         }
      }

      if let Some(ref texture) = texture {
         texture.bind_gl(gl, 0);
         unsafe {
            gl.framebuffer_texture_2d(
               glow::FRAMEBUFFER,
               glow::COLOR_ATTACHMENT0,
               glow::TEXTURE_2D,
               Some(texture.texture.texture),
               0,
            );
         }
      }

      if let Some(ref depth) = depth {
         depth.bind_gl(gl, 0);
         unsafe {
            gl.framebuffer_texture_2d(
               glow::FRAMEBUFFER,
               glow::DEPTH_ATTACHMENT,
               glow::TEXTURE_2D,
               Some(depth.texture.texture),
               0,
            );
         }
      };

      let status = unsafe { gl.check_framebuffer_status(glow::FRAMEBUFFER) };
      if status != glow::FRAMEBUFFER_COMPLETE {
         anyhow::bail!("error setting up framebuffer");
      }

      unsafe {
         Texture::unbind_gl(gl);
         gl.bind_framebuffer(
            glow::FRAMEBUFFER,
            NonZero::new(naevc::gl_screen.current_fbo).map(glow::NativeFramebuffer),
         );
      }

      Ok(Framebuffer {
         framebuffer,
         w: self.w,
         h: self.h,
         texture,
         depth,
      })
   }
}

static PREVIOUS_FBO_SET: AtomicBool = AtomicBool::new(false);
static PREVIOUS_FBO: AtomicU32 = AtomicU32::new(0);
static WAS_SCISSORED: AtomicBool = AtomicBool::new(false);
fn reset() {
   if PREVIOUS_FBO_SET.load(Ordering::Relaxed) {
      let prev = PREVIOUS_FBO.load(Ordering::Relaxed);
      let ctx = Context::get();
      let gl = &ctx.gl;
      unsafe {
         naevc::gl_screen.current_fbo = prev;
         PREVIOUS_FBO.store(0, Ordering::Relaxed);
         if WAS_SCISSORED.load(Ordering::Relaxed) {
            gl.enable(SCISSOR_TEST);
            WAS_SCISSORED.store(false, Ordering::Relaxed);
         }
         gl.viewport(0, 0, naevc::gl_screen.rw, naevc::gl_screen.rh);
         if let Some(prev) = NonZero::new(prev) {
            gl.bind_framebuffer(FRAMEBUFFER, Some(glow::NativeFramebuffer(prev)));
         } else {
            gl.bind_framebuffer(FRAMEBUFFER, None);
         }
      }
   }
}

#[derive(Clone, Debug)]
pub struct FramebufferWrap(Arc<Framebuffer>);
impl FramebufferWrap {
   pub fn new(fb: Framebuffer) -> Self {
      Self(Arc::new(fb))
   }

   pub fn into_raw(self) -> *const Framebuffer {
      Arc::into_raw(self.0)
   }

   /// # Safety
   /// Same as Arc::from_raw.
   pub unsafe fn from_raw(ptr: *const Framebuffer) -> Self {
      let fb: Arc<Framebuffer> = unsafe { Arc::from_raw(ptr) };
      Self(fb)
   }
}
impl Deref for FramebufferWrap {
   type Target = Framebuffer;

   fn deref(&self) -> &Self::Target {
      &self.0
   }
}

/*@
 * @brief Lua bindings to interact with canvass.
 *
 * @note The API here is designed to be compatible with that of "LÖVE".
 *
 * @luamod canvas
 */
impl UserData for FramebufferWrap {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      methods.add_meta_function(MetaMethod::ToString, |_, this: UserDataRef<Self>| {
         Ok(format!("Canvas( {} )", this.framebuffer.0))
      });
      /*@
       * @brief Opens a new canvas.
       *
       *    @luatparam number width Width of the new canvas.
       *    @luatparam number height Height of the new canvas.
       *    @luatparam[opt=false] boolean depth Whether or not to add a depth channel
       * to the canvas.
       *    @luatreturn Canvas New canvas object.
       * @luafunc new
       */
      methods.add_function(
         "new",
         |_, (w, h, depth): (usize, usize, Option<bool>)| -> mlua::Result<Self> {
            static CANVAS_ID: AtomicU32 = AtomicU32::new(1);
            let id = CANVAS_ID
               .fetch_update(Ordering::SeqCst, Ordering::SeqCst, |x| Some(x + 1))
               .unwrap_or(0);
            Ok(FramebufferWrap::new(
               FramebufferBuilder::new(Some(&format!("nlua_canvas_{id}")))
                  .width(w)
                  .height(h)
                  .depth(depth.unwrap_or(false))
                  .build(Context::get())?,
            ))
         },
      );

      /*@
       * @brief Sets the active canvas.
       *
       *    @luatparam Canvas|nil arg Either a canvas object or nil to disable.
       * @luafunc set
       * @todo Add actual graphics state maintenance! For now, just disable the
       * scissor region.
       */
      methods.add_function(
         "set",
         |_, c: Option<UserDataRef<Self>>| -> mlua::Result<()> {
            if let Some(canvas) = c {
               let ctx = Context::get();
               let gl = &ctx.gl;
               if !PREVIOUS_FBO_SET.load(Ordering::Relaxed) {
                  PREVIOUS_FBO.store(unsafe { naevc::gl_screen.current_fbo }, Ordering::Relaxed);
                  PREVIOUS_FBO_SET.store(true, Ordering::Relaxed);
                  WAS_SCISSORED.store(unsafe { gl.is_enabled(SCISSOR_TEST) }, Ordering::Relaxed);
               }
               unsafe {
                  naevc::gl_screen.current_fbo = canvas.framebuffer.0.into();
                  gl.disable(SCISSOR_TEST);
                  gl.viewport(0, 0, canvas.w as i32, canvas.h as i32);
                  naevc::render_needsReset();
               }
               canvas.bind_gl(gl);
            } else {
               reset();
            }
            Ok(())
         },
      );

      /*@
       * @brief Gets the size of the canvas.
       *
       *    @luatreturn number Width of the canvas.
       *    @luatreturn number Height of the canvas.
       * @luafunc dims
       */
      methods.add_method("dims", |_, canvas, ()| -> mlua::Result<(usize, usize)> {
         Ok((canvas.w, canvas.h))
      });

      /*@
       * @brief Gets the texture associated with the canvas.
       *
       *    @luatparam Canvas canvas Canvas to get the texture from.
       *    @luatreturn Tex Texture associated with the canvas.
       * @luafunc getTex
       */
      methods.add_method("getTex", |_, canvas, ()| -> mlua::Result<Texture> {
         if let Some(tex) = &canvas.texture {
            Ok(tex.try_clone()?)
         } else {
            Err(mlua::Error::RuntimeError(
               "framebuffer has no texture".to_string(),
            ))
         }
      });

      /*@
       * @brief Clears a canvas.
       *
       *    @luatparam Canvas canvas Canvas to clear.
       *    @luatparam Colour col Colour to clear to.
       * @luafunc clear
       */
      methods.add_method(
         "clear",
         |_, _canvas, colour: Option<Colour>| -> mlua::Result<()> {
            let colour = colour.unwrap_or_else(|| Colour::new_alpha(0., 0., 0., 0.));
            let ctx = Context::get();
            let gl = &ctx.gl;
            let v: Vector4<f32> = colour.into();
            unsafe {
               gl.clear_color(v.x, v.y, v.z, v.w);
               gl.clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
               gl.clear_color(0.0, 0.0, 0.0, 0.0);
            }
            Ok(())
         },
      );
   }
}

use mlua::{Value, ffi};
use std::ffi::{c_char, c_int, c_void};
pub fn open_canvas(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<FramebufferWrap>()?;

   if let mlua::Value::Nil = lua.named_registry_value("push_canvas")? {
      let push_canvas = lua.create_function(|lua, canvas: mlua::LightUserData| {
         let canvas = canvas.0 as *mut Framebuffer;
         if canvas.is_null() {
            Err(mlua::Error::RuntimeError(
               "push_canvas received NULL".to_string(),
            ))
         } else {
            let canvas = unsafe { FramebufferWrap::from_raw(canvas) };
            lua.create_userdata(canvas)
         }
      })?;
      lua.set_named_registry_value("push_canvas", push_canvas)?;

      let get_canvas =
         lua.create_function(|_, mut ud: mlua::UserDataRefMut<FramebufferWrap>| {
            let canvas: *mut FramebufferWrap = &mut *ud;
            Ok(Value::LightUserData(mlua::LightUserData(
               canvas as *mut c_void,
            )))
         })?;
      lua.set_named_registry_value("get_canvas", get_canvas)?;

      let get_canvas_fbo = lua.create_function(
         |_, ud: mlua::UserDataRef<FramebufferWrap>| -> mlua::Result<i32> {
            Ok(Into::<u32>::into(ud.framebuffer.0) as i32)
         },
      )?;
      lua.set_named_registry_value("get_canvas_fbo", get_canvas_fbo)?;
   }

   Ok(proxy)
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_checkcanvas(L: *mut mlua::lua_State, idx: c_int) -> *mut FramebufferWrap {
   unsafe {
      let canvas = lua_tocanvas(L, idx);
      if canvas.is_null() {
         ffi::luaL_typerror(L, idx, c"canvas".as_ptr() as *const c_char);
      }
      canvas
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_iscanvas(L: *mut mlua::lua_State, idx: c_int) -> c_int {
   !lua_tocanvas(L, idx).is_null() as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_pushcanvas(L: *mut mlua::lua_State, canvas: *mut FramebufferWrap) {
   unsafe {
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_canvas".as_ptr());
      ffi::lua_pushlightuserdata(L, canvas as *mut c_void);
      ffi::lua_call(L, 1, 1);
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_tocanvas(L: *mut mlua::lua_State, idx: c_int) -> *mut FramebufferWrap {
   unsafe {
      let idx = ffi::lua_absindex(L, idx);
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_canvas".as_ptr());
      ffi::lua_pushvalue(L, idx);
      let canvas = match ffi::lua_pcall(L, 1, 1, 0) {
         ffi::LUA_OK => ffi::lua_touserdata(L, -1) as *mut FramebufferWrap,
         _ => std::ptr::null_mut(),
      };
      ffi::lua_pop(L, 1);
      canvas
   }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_checkcanvasfbo(L: *mut mlua::lua_State, idx: c_int) -> i32 {
   unsafe {
      let idx = ffi::lua_absindex(L, idx);
      ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_canvas_fbo".as_ptr());
      ffi::lua_pushvalue(L, idx);
      let canvas = match ffi::lua_pcall(L, 1, 1, 0) {
         ffi::LUA_OK => ffi::lua_tointeger(L, -1) as i32,
         _ => -1,
      };
      ffi::lua_pop(L, 1);
      canvas
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn canvas_new(w: c_int, h: c_int) -> *const Framebuffer {
   static CANVAS_ID: AtomicU32 = AtomicU32::new(1);
   let id = CANVAS_ID
      .fetch_update(Ordering::SeqCst, Ordering::SeqCst, |x| Some(x + 1))
      .unwrap_or(0);
   let fb = match FramebufferBuilder::new(Some(&format!("C_nlua_canvas_{id}")))
      .width(w as usize)
      .height(h as usize)
      .build(Context::get())
   {
      Ok(fb) => fb,
      Err(e) => {
         warn_err!(e);
         return std::ptr::null_mut();
      }
   };
   FramebufferWrap::new(fb).into_raw()
}

#[unsafe(no_mangle)]
pub extern "C" fn canvas_fbo(fb: *const Framebuffer) -> naevc::GLuint {
   let fb = unsafe { &*fb };
   fb.framebuffer.0.into()
}

#[unsafe(no_mangle)]
pub extern "C" fn canvas_tex(fb: *const Framebuffer) -> *mut Texture {
   let fb = unsafe { &*fb };
   if let Some(tex) = &fb.texture
      && let Ok(tex) = tex.try_clone()
   {
      tex.into_ptr()
   } else {
      std::ptr::null_mut()
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn canvas_reset() {
   reset();
}
