#![allow(dead_code, unused_variables)]
use anyhow::Result;
use glow::*;
use nalgebra::Vector4;
use sdl2 as sdl;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_double, c_float, c_int, c_uint};
use std::sync::{Arc, LazyLock, Mutex, Weak};

use crate::ngl::CONTEXT;
use crate::{formatx, warn};
use crate::{gettext, ndata};

static TEXTURE_DATA: LazyLock<Mutex<Vec<Weak<TextureData>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));

pub struct TextureData {
    name: Option<String>,
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
    /// Creates a new TextureData of size w x h without any data.
    fn new(gl: &glow::Context, w: usize, h: usize) -> Result<Self> {
        if w == 0 || h == 0 {
            return Err(anyhow::anyhow!(
                "Trying to create TextureData without width or height"
            ));
        }
        let texture = unsafe { gl.create_texture().map_err(|e| anyhow::anyhow!(e)) }?;
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
            gl.tex_image_2d(
                glow::TEXTURE_2D,
                0,
                glow::SRGB_ALPHA as i32,
                w as i32,
                h as i32,
                0,
                glow::RGBA,
                glow::UNSIGNED_BYTE,
                glow::PixelUnpackData::Slice(None),
            );
            gl.bind_texture(glow::TEXTURE_2D, None);
        }

        Ok(TextureData {
            name: None,
            w,
            h,
            texture,
            is_srgb: true,
            is_sdf: false,
            vmax: 1.,
        })
    }

    /// Checks to see if a TextureData exists.
    fn exists(name: &str) -> Option<Arc<Self>> {
        let textures = TEXTURE_DATA.lock().unwrap();
        for tex in textures.iter() {
            if let Some(t) = tex.upgrade() {
                if let Some(tname) = &t.name {
                    if tname == name {
                        return Some(t);
                    }
                }
            }
        }
        None
    }

    /// Creates a new TextureData from
    fn from_raw(raw: glow::NativeTexture, w: usize, h: usize) -> Result<Self> {
        Ok(TextureData {
            name: None,
            w,
            h,
            texture: raw,
            is_srgb: true,
            is_sdf: false,
            vmax: 1.,
        })
    }

    /// Creates a new TextureData from an image wrapper
    fn from_image(
        gl: &glow::Context,
        name: Option<&str>,
        img: &image::DynamicImage,
    ) -> Result<Self> {
        let texture = unsafe { gl.create_texture().map_err(|e| anyhow::anyhow!(e)) }?;

        let has_alpha = img.color().has_alpha();
        let (w, h) = (img.width(), img.height());
        let imgdata = img.flipv().to_rgba8().into_raw();
        //let imgdata = img.to_rgba8().into_raw();

        let is_srgb = true;

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
            //gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, surface.pitch().min(8) as i32);
            let gldata = glow::PixelUnpackData::Slice(Some(imgdata.as_slice()));
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
            //gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 4);
            gl.bind_texture(glow::TEXTURE_2D, None);
        }

        Ok(TextureData {
            name: name.map(String::from),
            w: w as usize,
            h: h as usize,
            texture,
            is_srgb,
            is_sdf: false,
            vmax: 1.,
        })
    }
}

pub struct Texture {
    pub path: Option<String>,
    name: Option<CString>, // TODO remove when not needed

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
    pub flipv: bool,
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
        for param in [
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
            flipv: self.flipv,
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

pub enum TextureSource {
    Path(String),
    Image(image::DynamicImage),
    TextureData(Arc<TextureData>),
    Raw(glow::NativeTexture),
    None,
}
impl TextureSource {
    fn to_texture_data(
        &self,
        gl: &glow::Context,
        w: usize,
        h: usize,
        name: Option<&str>,
    ) -> Result<Arc<TextureData>> {
        if let TextureSource::TextureData(tex) = self {
            // Purpose dropout, don't want to cache again here
            return Ok(tex.clone());
        };

        let mut textures = TEXTURE_DATA.lock().unwrap();

        // Try to load from name if possible
        if let Some(name) = name {
            for tex in textures.iter() {
                if let Some(t) = tex.upgrade() {
                    if let Some(tname) = &t.name {
                        if tname == name {
                            return Ok(t);
                        }
                    }
                }
            }
        }

        // Failed to find in cache, load a new
        let tex = Arc::new(match self {
            TextureSource::Path(path) => {
                let bytes = ndata::read(path.as_str())?;
                let img = image::load_from_memory(&bytes)?;
                TextureData::from_image(gl, name, &img)?
            }
            TextureSource::Image(img) => TextureData::from_image(gl, name, img)?,
            TextureSource::Raw(tex) => TextureData::from_raw(*tex, w, h)?,
            TextureSource::None => TextureData::new(gl, w, h)?,
            TextureSource::TextureData(tex) => unreachable!(),
        });

        // Add weak reference to cache
        if name.is_some() {
            textures.push(Arc::downgrade(&tex));
        }

        Ok(tex)
    }
}

pub struct TextureBuilder {
    name: Option<String>,
    source: TextureSource,
    w: usize,
    h: usize,
    sx: usize,
    sy: usize,
    is_srgb: bool,
    is_sdf: bool,
    flipv: bool,
    border_value: Option<Vector4<f32>>,
    address_u: AddressMode,
    address_v: AddressMode,
    mag_filter: FilterMode,
    min_filter: FilterMode,
    mipmaps: bool,
}

impl TextureBuilder {
    pub fn new() -> Self {
        TextureBuilder {
            name: None,
            source: TextureSource::None,
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
            flipv: false,
        }
    }

    pub fn name(mut self, name: Option<&str>) -> Self {
        self.name = name.map(String::from);
        self
    }

    pub fn path(mut self, path: &str) -> Self {
        self.source = TextureSource::Path(String::from(path));
        self.name = Some(String::from(path));
        self
    }

    pub fn image(mut self, img: &image::DynamicImage) -> Self {
        self.source = TextureSource::Image(img.clone());
        self
    }

    pub fn texture_data(mut self, data: &Arc<TextureData>) -> Self {
        self.source = TextureSource::TextureData(data.clone());
        self
    }

    pub fn native_texture(mut self, tex: glow::NativeTexture) -> Self {
        self.source = TextureSource::Raw(tex);
        self
    }

    pub fn srgb(mut self, enable: bool) -> Self {
        self.is_srgb = enable;
        self
    }

    pub fn sdf(mut self, enable: bool) -> Self {
        self.is_sdf = enable;
        self
    }

    pub fn flipv(mut self, enable: bool) -> Self {
        self.flipv = enable;
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
        /* TODO handle SDF. */
        let texture = self
            .source
            .to_texture_data(gl, self.w, self.h, self.name.as_deref())?;
        let sampler = unsafe { gl.create_sampler() }.map_err(|e| anyhow::anyhow!(e))?;
        unsafe {
            gl.sampler_parameter_i32(sampler, glow::TEXTURE_MIN_FILTER, self.min_filter.to_gl());
            gl.sampler_parameter_i32(sampler, glow::TEXTURE_MAG_FILTER, self.mag_filter.to_gl());
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

        Ok(Texture {
            path: self.name.clone().map(String::from),
            name: self.name.map(|s| CString::new(s.as_str()).unwrap()),
            sx,
            sy,
            sw,
            sh,
            srw,
            srh,
            texture,
            sampler,
            flipv: self.flipv,
        })
    }
}

struct Flags {
    maptrans: bool,
    mipmaps: bool,
    flipv: bool,
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
            flipv: (flags & naevc::OPENGL_TEX_VFLIP) > 0,
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

capi_tex!(tex_w, w);
capi_tex!(tex_h, h);
capi!(tex_sx, sx);
capi!(tex_sy, sy);
capi!(tex_sw, sw);
capi!(tex_sh, sh);
capi!(tex_srw, srw);
capi!(tex_srh, srh);
capi_tex!(tex_vmax, vmax);

#[no_mangle]
pub extern "C" fn gl_initTextures() -> c_int {
    0
}

#[no_mangle]
pub extern "C" fn gl_exitTextures() {}

#[no_mangle]
pub extern "C" fn gl_texExistsOrCreate(
    cpath: *const c_char,
    cflags: c_uint,
    sx: c_int,
    sy: c_int,
    created: *mut c_int,
) -> *mut Texture {
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */

    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);
    let mut builder = TextureBuilder::new()
        .sx(sx as usize)
        .sy(sy as usize)
        .flipv(flags.flipv)
        .srgb(!flags.notsrgb)
        .mipmaps(flags.mipmaps);

    if flags.clamp_alpha {
        builder = builder.border(Some(Vector4::<f32>::new(0., 0., 0., 0.)));
    }

    let pathname = path.to_str().unwrap();
    builder = match TextureData::exists(pathname) {
        Some(tex) => {
            unsafe {
                *created = 0;
            }
            builder.texture_data(&tex)
        }
        None => {
            unsafe {
                *created = 1;
            }
            builder.path(pathname)
        }
    };

    match builder.build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gl_loadImageData(
    data: *const c_float,
    w: c_int,
    h: c_int,
    sx: c_int,
    sy: c_int,
    cname: *const c_char,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    let name = unsafe { CStr::from_ptr(cname) };
    let flags = Flags::from(cflags);

    let mut builder = TextureBuilder::new()
        .name(Some(name.to_str().unwrap()))
        .sx(sx as usize)
        .sy(sy as usize)
        .flipv(flags.flipv)
        .width(w as usize)
        .height(h as usize);

    if !data.is_null() {
        let rawdata = unsafe { std::slice::from_raw_parts(data, (w * h * 4) as usize) };
        let buf = match image::ImageBuffer::<image::Rgba<f32>, Vec<f32>>::from_raw(
            w as u32,
            h as u32,
            rawdata.to_vec(),
        ) {
            Some(val) => val,
            None => {
                warn!("unable to load image");
                return std::ptr::null_mut();
            }
        };
        let img = image::DynamicImage::ImageRgba32F(buf);
        builder = builder.image(&img);
    }

    match builder.build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => {
            warn!("unable to create texture");
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn gl_newImage(cpath: *const c_char, flags: c_uint) -> *mut Texture {
    gl_newSprite(cpath, 1, 1, flags)
}

#[no_mangle]
pub extern "C" fn gl_newSprite(
    cpath: *const c_char,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);

    let mut builder = TextureBuilder::new()
        .path(path.to_str().unwrap())
        .sx(sx as usize)
        .sy(sy as usize)
        .flipv(flags.flipv)
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
pub extern "C" fn gl_newSpriteRWops(
    cpath: *const c_char,
    rw: *mut naevc::SDL_RWops,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);

    let mut builder = TextureBuilder::new()
        .sx(sx as usize)
        .sy(sy as usize)
        .flipv(flags.flipv)
        .srgb(!flags.notsrgb)
        .mipmaps(flags.mipmaps);

    if flags.clamp_alpha {
        builder = builder.border(Some(Vector4::<f32>::new(0., 0., 0., 0.)));
    }

    let pathname = path.to_str().unwrap();
    builder = match TextureData::exists(pathname) {
        Some(tex) => builder.texture_data(&tex),
        None => {
            let rw = unsafe { sdl::rwops::RWops::from_ll(rw as *mut sdl::sys::SDL_RWops) };
            let img = image::ImageReader::new(std::io::BufReader::new(rw))
                .with_guessed_format()
                .unwrap()
                .decode()
                .unwrap();
            builder.image(&img)
        }
    };

    match builder.build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gl_dupTexture(ctex: *mut Texture) -> *mut Texture {
    if ctex.is_null() {
        return ctex;
    }
    let tex = unsafe { &*ctex };
    unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
    Box::into_raw(Box::new(tex.try_clone().unwrap()))
}

#[no_mangle]
pub extern "C" fn gl_rawTexture(
    cpath: *mut c_char,
    tex: naevc::GLuint,
    w: c_double,
    h: c_double,
) -> *mut Texture {
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    let pathname: Option<&str> = {
        if cpath.is_null() {
            None
        } else {
            let path = unsafe { CStr::from_ptr(cpath) };
            Some(path.to_str().unwrap())
        }
    };
    let mut builder = TextureBuilder::new()
        .width(w as usize)
        .height(h as usize)
        .name(pathname);

    builder = match pathname {
        Some(pathname) => match TextureData::exists(pathname) {
            Some(tex) => builder.texture_data(&tex),
            None => {
                let tex = glow::NativeTexture(std::num::NonZero::new(tex).unwrap());
                builder.native_texture(tex)
            }
        },
        None => {
            let tex = glow::NativeTexture(std::num::NonZero::new(tex).unwrap());
            builder.native_texture(tex)
        }
    };

    match builder.build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gl_freeTexture(ctex: *mut Texture) {
    if !ctex.is_null() {
        let _ = unsafe { Box::from_raw(ctex) };
    }
    // The texture should get dropped now
}

#[no_mangle]
pub extern "C" fn tex_tex(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.texture.texture.0.into()
}

#[no_mangle]
pub extern "C" fn tex_sampler(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.sampler.0.into()
}

#[no_mangle]
pub extern "C" fn tex_name(ctex: *mut Texture) -> *const c_char {
    let tex = unsafe { &*ctex };
    match &tex.name {
        Some(name) => name.as_ptr(),
        None => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn tex_isSDF(ctex: *mut Texture) -> c_int {
    let tex = unsafe { &*ctex };
    tex.texture.is_sdf as i32
}

#[no_mangle]
pub extern "C" fn gl_isTrans(ctex: *mut Texture, x: c_int, y: c_int) -> c_int {
    // TODO
    0
}

#[no_mangle]
pub extern "C" fn tex_hasTrans(ctex: *mut Texture) -> c_int {
    // TODO
    0
}

#[no_mangle]
pub extern "C" fn tex_setTex(ctex: *mut Texture, texture: naevc::GLuint) {
    let tex = unsafe { &mut *ctex };
    let ntex = glow::NativeTexture(std::num::NonZero::new(texture).unwrap());
    tex.texture = Arc::new(TextureData::from_raw(ntex, tex.texture.w, tex.texture.h).unwrap());
}

#[no_mangle]
pub extern "C" fn tex_setVFLIP(ctex: *mut Texture, flip: c_int) {
    let tex = unsafe { &mut *ctex };
    tex.flipv = flip != 0;
}

#[no_mangle]
pub extern "C" fn tex_flags(ctex: *mut Texture) -> c_uint {
    let tex = unsafe { &*ctex };
    let mut flags: c_uint = 0;
    if tex.flipv {
        flags |= naevc::OPENGL_TEX_VFLIP;
    }
    flags
}
