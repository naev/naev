#![allow(dead_code, unused_variables)]
use anyhow::Result;
use glow::*;
use nalgebra::Vector4;
use sdl2 as sdl;
use sdl2::image::ImageRWops;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_double, c_int, c_uint};
use std::sync::{Arc, LazyLock, Mutex, Weak};

use crate::ndata;
use crate::ngl::CONTEXT;

static TEXTURE_DATA: LazyLock<Mutex<Vec<Weak<TextureData>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));

pub struct TextureData {
    path: String,
    texture: glow::Texture,
    w: usize,
    h: usize,
    is_srgb: bool,
    is_sdf: bool,
    vmax: f64, // For SDF
}
impl Drop for TextureData {
    fn drop(&mut self) {
        let ctx = CONTEXT.get().unwrap();
        unsafe { ctx.gl.delete_texture(self.texture) };
    }
}

impl TextureData {
    const SDL_FORMAT: sdl::pixels::PixelFormatEnum = sdl::pixels::PixelFormatEnum::ABGR8888;

    fn new_tex(
        gl: &glow::Context,
        path: &str,
        is_srgb: bool,
        is_sdf: bool,
    ) -> Result<(glow::Texture, usize, usize), String> {
        let texture = unsafe { gl.create_texture()? };

        let imgdata = ndata::read(path).unwrap();

        let rw = sdl2::rwops::RWops::from_bytes(&imgdata)?;
        let mut surface = rw.load()?;
        let surfmt = surface.pixel_format_enum();
        let has_alpha = surfmt.into_masks()?.amask > 0;
        if surfmt != Self::SDL_FORMAT {
            surface = surface.convert_format(Self::SDL_FORMAT)?;
        }
        let (w, h) = (surface.width(), surface.height());

        if is_sdf {
            //todo!();
        }

        let internalformat = match is_srgb {
            true => match has_alpha {
                true => glow::SRGB_ALPHA,
                false => glow::SRGB,
            },
            false => match has_alpha {
                true => glow::RGBA,
                false => glow::RGB,
            },
        };
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
            // TODO is this pitch correct?
            gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, surface.pitch().min(8) as i32);
            surface.with_lock(|data| {
                let gldata = glow::PixelUnpackData::Slice(Some(data));
                gl.tex_image_2d(
                    glow::TEXTURE_2D,
                    0,
                    internalformat as i32,
                    w as i32,
                    h as i32,
                    0,
                    glow::RGBA,
                    glow::UNSIGNED_BYTE,
                    gldata,
                );
            });
            gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 4);
            gl.bind_texture(glow::TEXTURE_2D, None);
        }

        Ok((texture, w as usize, h as usize))
    }

    fn new(gl: &glow::Context, path: &str, is_srgb: bool) -> Result<Arc<Self>> {
        let mut textures = TEXTURE_DATA.lock().unwrap();
        for tex in textures.iter() {
            if let Some(t) = tex.upgrade() {
                if t.path == path {
                    return Ok(t);
                }
            }
        }

        let (texture, w, h) =
            Self::new_tex(gl, path, is_srgb, false).map_err(|e| anyhow::anyhow!(e))?;

        let tex = Arc::new(TextureData {
            path: String::from(path),
            w,
            h,
            texture,
            is_srgb,
            is_sdf: false,
            vmax: 1.,
        });

        textures.push(Arc::downgrade(&tex));
        Ok(tex)
    }

    fn new_sdf(gl: &glow::Context, path: &str) -> Result<Arc<Self>> {
        todo!();
    }
}

pub struct Texture {
    pub path: String,
    name: CString, // TODO remove when not needed

    // Sprites
    pub sx: usize,
    pub sy: usize,
    pub sw: f64,
    pub sh: f64,
    pub srw: f64,
    pub srh: f64,

    // Data
    pub texture: Arc<TextureData>,
    pub sampler: glow::Sampler,
}
impl Drop for Texture {
    fn drop(&mut self) {
        let ctx = CONTEXT.get().unwrap();
        unsafe { ctx.gl.delete_sampler(self.sampler) };
    }
}
impl Texture {
    fn try_clone(&self) -> Result<Self> {
        let ctx = CONTEXT.get().unwrap();
        let gl = &ctx.gl;
        let sampler = unsafe { gl.create_sampler() }.map_err(|e| anyhow::anyhow!(e))?;

        // Copy necessaryparameters over
        for param in vec![
            glow::TEXTURE_WRAP_S,
            glow::TEXTURE_WRAP_T,
            glow::TEXTURE_MIN_FILTER,
            glow::TEXTURE_MAG_FILTER,
        ] {
            unsafe {
                let val = gl.get_sampler_parameter_i32(self.sampler, param);
                gl.sampler_parameter_i32(sampler, param, val);
            }
        }

        Ok(Texture {
            path: self.path.clone(),
            name: self.name.clone(),
            sx: self.sx,
            sy: self.sy,
            sw: self.sw,
            sh: self.sh,
            srw: self.srw,
            srh: self.srh,
            texture: self.texture.clone(),
            sampler,
        })
    }
}

#[derive(Clone, Copy)]
pub enum AddressMode {
    ClampToEdge = 0,
    Repeat = 1,
    MirrorRepeat = 2,
    ClampToBorder = 3,
}
impl AddressMode {
    pub fn to_gl(self) -> i32 {
        (match self {
            AddressMode::ClampToEdge => glow::CLAMP_TO_EDGE,
            AddressMode::Repeat => glow::REPEAT,
            AddressMode::MirrorRepeat => glow::MIRRORED_REPEAT,
            AddressMode::ClampToBorder => glow::CLAMP_TO_BORDER,
        }) as i32
    }
}

#[derive(Clone, Copy)]
pub enum FilterMode {
    Nearest = 0,
    Linear = 1,
}
impl FilterMode {
    pub fn to_gl(self) -> i32 {
        (match self {
            FilterMode::Nearest => glow::NEAREST,
            FilterMode::Linear => glow::LINEAR,
        }) as i32
    }
}

pub struct TextureBuilder {
    path: Option<String>,
    w: usize,
    h: usize,
    sx: usize,
    sy: usize,
    is_srgb: bool,
    is_sdf: bool,
    border_value: Option<Vector4<f32>>,
    address_u: AddressMode,
    address_v: AddressMode,
    mag_filter: FilterMode,
    min_filter: FilterMode,
    mipmaps: bool,
}

impl TextureBuilder {
    pub fn new(path: Option<&str>) -> Self {
        TextureBuilder {
            path: path.map(String::from),
            w: 0,
            h: 0,
            sx: 1,
            sy: 1,
            is_srgb: true,
            is_sdf: false,
            border_value: None,
            address_u: AddressMode::Repeat,
            address_v: AddressMode::Repeat,
            mag_filter: FilterMode::Linear,
            min_filter: FilterMode::Linear,
            mipmaps: false,
        }
    }

    pub fn srgb(mut self, enable: bool) -> Self {
        self.is_srgb = enable;
        self
    }

    pub fn sdf(mut self, enable: bool) -> Self {
        self.is_sdf = enable;
        self
    }

    pub fn width(mut self, w: usize) -> Self {
        self.w = w;
        self
    }

    pub fn height(mut self, h: usize) -> Self {
        self.h = h;
        self
    }

    pub fn sx(mut self, sx: usize) -> Self {
        self.sx = sx;
        self
    }

    pub fn sy(mut self, sy: usize) -> Self {
        self.sy = sy;
        self
    }

    pub fn address_mode(self, mode: AddressMode) -> Self {
        self.address_mode_u(mode).address_mode_v(mode)
    }

    pub fn address_mode_u(mut self, mode: AddressMode) -> Self {
        self.address_u = mode;
        self
    }

    pub fn address_mode_v(mut self, mode: AddressMode) -> Self {
        self.address_v = mode;
        self
    }

    pub fn filter(self, mode: FilterMode) -> Self {
        self.min_filter(mode).mag_filter(mode)
    }

    pub fn min_filter(mut self, mode: FilterMode) -> Self {
        self.min_filter = mode;
        self
    }

    pub fn mag_filter(mut self, mode: FilterMode) -> Self {
        self.mag_filter = mode;
        self
    }

    pub fn border(mut self, border_value: Option<Vector4<f32>>) -> Self {
        self.border_value = border_value;
        self.address_mode(AddressMode::ClampToBorder)
    }

    pub fn mipmaps(mut self, enable: bool) -> Self {
        self.mipmaps = enable;
        self
    }

    pub fn build(self, gl: &glow::Context) -> Result<Texture> {
        if let Some(path) = &self.path {
            let texture = match self.is_sdf {
                true => TextureData::new_sdf(gl, path.as_str()),
                false => TextureData::new(gl, path.as_str(), self.is_srgb),
            }?;

            let sampler = unsafe { gl.create_sampler() }.map_err(|e| anyhow::anyhow!(e))?;
            unsafe {
                gl.sampler_parameter_i32(
                    sampler,
                    glow::TEXTURE_MIN_FILTER,
                    self.min_filter.to_gl(),
                );
                gl.sampler_parameter_i32(
                    sampler,
                    glow::TEXTURE_MAG_FILTER,
                    self.mag_filter.to_gl(),
                );
                if let Some(border) = &self.border_value {
                    gl.sampler_parameter_f32_slice(
                        sampler,
                        glow::TEXTURE_BORDER_COLOR,
                        border.as_slice(),
                    );
                }
                gl.sampler_parameter_i32(sampler, glow::TEXTURE_WRAP_S, self.address_u.to_gl());
                gl.sampler_parameter_i32(sampler, glow::TEXTURE_WRAP_T, self.address_v.to_gl());
            }

            let (w, h) = (texture.w, texture.h);
            let (sx, sy) = (self.sx, self.sy);
            let sw = (w as f64) / (sx as f64);
            let sh = (h as f64) / (sy as f64);
            let srw = sw / (w as f64);
            let srh = sh / (h as f64);

            let name = CString::new(path.as_str())?;

            Ok(Texture {
                path: self.path.unwrap(),
                name,
                sx,
                sy,
                sw,
                sh,
                srw,
                srh,
                texture,
                sampler,
            })
        } else {
            todo!();
        }
    }
}

struct Flags {
    maptrans: bool,
    mipmaps: bool,
    vflip: bool,
    skipcache: bool,
    sdf: bool,
    clamp_alpha: bool,
    notsrgb: bool,
}
impl Flags {
    fn from(flags: c_uint) -> Self {
        Flags {
            maptrans: (flags & naevc::OPENGL_TEX_MAPTRANS) > 0,
            mipmaps: (flags & naevc::OPENGL_TEX_MIPMAPS) > 0,
            vflip: (flags & naevc::OPENGL_TEX_VFLIP) > 0,
            skipcache: (flags & naevc::OPENGL_TEX_SKIPCACHE) > 0,
            sdf: (flags & naevc::OPENGL_TEX_SDF) > 0,
            clamp_alpha: (flags & naevc::OPENGL_TEX_CLAMP_ALPHA) > 0,
            notsrgb: (flags & naevc::OPENGL_TEX_NOTSRGB) > 0,
        }
    }
}

macro_rules! capi_tex {
    ($funcname: ident, $field: tt) => {
        #[no_mangle]
        pub extern "C" fn $funcname(ctex: *mut Texture) -> c_double {
            let tex = unsafe { &*ctex };
            tex.texture.$field as f64
        }
    };
}
macro_rules! capi {
    ($funcname: ident, $field: tt) => {
        #[no_mangle]
        pub extern "C" fn $funcname(ctex: *mut Texture) -> c_double {
            let tex = unsafe { &*ctex };
            tex.$field as f64
        }
    };
}

capi_tex!(tex_w_, w);
capi_tex!(tex_h_, h);
capi!(tex_sx_, sx);
capi!(tex_sy_, sy);
capi!(tex_sw_, sw);
capi!(tex_sh_, sh);
capi!(tex_srw_, srw);
capi!(tex_srh_, srh);
capi_tex!(tex_vmax_, vmax);

#[no_mangle]
pub extern "C" fn gl_initTextures_() -> c_int {
    0
}

#[no_mangle]
pub extern "C" fn gl_exitTextures_() {}

#[no_mangle]
pub extern "C" fn gl_texExistsOrCreate_(
    path: *const c_char,
    flags: c_uint,
    sx: c_int,
    sy: c_int,
    created: *mut c_int,
) -> *mut Texture {
    todo!();
}

#[no_mangle]
pub extern "C" fn gl_loadImageData_(
    data: *mut f32,
    w: c_int,
    h: c_int,
    sx: c_int,
    sy: c_int,
    name: *const c_char,
) -> *mut Texture {
    todo!();
}

#[no_mangle]
pub extern "C" fn gl_newImage_(cpath: *const c_char, flags: c_uint) -> *mut Texture {
    gl_newSprite_(cpath, 1, 1, flags)
}

#[no_mangle]
pub extern "C" fn gl_newSprite_(
    cpath: *const c_char,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = CONTEXT.get().unwrap();
    let flags = Flags::from(cflags);

    let mut builder = TextureBuilder::new(Some(path.to_str().unwrap()))
        .sx(sx as usize)
        .sy(sy as usize)
        .srgb(!flags.notsrgb)
        .mipmaps(flags.mipmaps);

    if flags.clamp_alpha {
        builder = builder.border(Some(Vector4::<f32>::new(0., 0., 0., 0.)));
    }

    match builder.build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gl_newSpriteRWops_(
    path: *const c_char,
    rw: *mut naevc::SDL_RWops,
    sx: c_int,
    sy: c_int,
    flags: c_uint,
) -> *mut Texture {
    todo!();
}

#[no_mangle]
pub extern "C" fn gl_dupTexture_(ctex: *mut Texture) -> *mut Texture {
    let tex = unsafe { &*ctex };
    unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
    Box::into_raw(Box::new(tex.try_clone().unwrap()))
}

#[no_mangle]
pub extern "C" fn gl_rawTexture_(
    ctex: *mut Texture,
    tex: naevc::GLuint,
    w: c_double,
    h: c_double,
) -> *mut Texture {
    todo!();
}

#[no_mangle]
pub extern "C" fn gl_freeTexture_(ctex: *mut Texture) {
    let _ = unsafe { Box::from_raw(ctex as *mut Texture) };
    // The texture should get dropped now
}

#[no_mangle]
pub extern "C" fn tex_tex_(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.texture.texture.0.into()
}

#[no_mangle]
pub extern "C" fn tex_sampler(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.sampler.0.into()
}

#[no_mangle]
pub extern "C" fn tex_name_(ctex: *mut Texture) -> *const c_char {
    let tex = unsafe { &*ctex };
    tex.name.as_ptr()
}

#[no_mangle]
pub extern "C" fn tex_isSDF_(ctex: *mut Texture) -> c_int {
    let tex = unsafe { &*ctex };
    tex.texture.is_sdf as i32
}
