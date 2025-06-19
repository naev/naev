#![allow(dead_code, unused_variables)]
use crate::render::Uniform;
use anyhow::Result;
use glow::*;
use nalgebra::{Matrix3, Vector4};
use sdl2 as sdl;
use sdl2::image::ImageRWops;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use std::num::NonZero;
use std::os::raw::{c_char, c_double, c_float, c_int, c_uint};
use std::sync::{Arc, LazyLock, Mutex, MutexGuard, Weak};

use crate::context::Context;
use crate::log::warn_err;
use crate::{buffer, context, gettext, ndata, render};
use crate::{warn, warn_err};

static TEXTURE_DATA: LazyLock<Mutex<Vec<Weak<TextureData>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));

// Temporary hack until image-rs significantly increases performance...
pub fn surface_to_image(sur: sdl::surface::Surface) -> Result<image::DynamicImage> {
    //let has_alpha = sur.pixel_format_enum().supports_alpha();
    let sur = sur
        .convert_format(sdl::pixels::PixelFormatEnum::RGBA32)
        .map_err(|e| anyhow::anyhow!(e))?;
    let (w, h) = sur.size();
    // TODO this always converts to rgba so we store more memory and such...
    sur.with_lock(|data| {
        let buf = image::ImageBuffer::<image::Rgba<u8>, _>::from_vec(w, h, data.to_vec()).unwrap();
        Ok(image::DynamicImage::ImageRgba8(buf))
    })
}

#[allow(clippy::upper_case_acronyms)]
#[derive(Clone, Copy, Debug)]
pub enum TextureFormat {
    RGB,
    RGBA,
    SRGB,
    SRGBA,
    Depth,
}
impl TextureFormat {
    pub fn auto(has_alpha: bool, is_srgb: bool) -> i32 {
        (match is_srgb {
            true => match has_alpha {
                true => glow::SRGB_ALPHA,
                false => glow::SRGB,
            },
            false => match has_alpha {
                true => glow::RGBA,
                false => glow::RGB,
            },
        }) as i32
    }

    pub fn to_gl(self) -> i32 {
        (match self {
            Self::RGB => glow::RGB,
            Self::RGBA => glow::RGBA,
            Self::SRGB => glow::SRGB,
            Self::SRGBA => glow::SRGB_ALPHA,
            Self::Depth => glow::DEPTH_COMPONENT,
        }) as i32
    }

    pub fn is_srgb(self) -> bool {
        match self {
            Self::RGB | Self::RGBA | Self::Depth => false,
            Self::SRGB | Self::SRGBA => true,
        }
    }
}

#[derive(Debug)]
pub struct TextureData {
    name: Option<String>,
    texture: glow::Texture,
    w: usize,
    h: usize,
    is_srgb: bool,
    is_sdf: bool,
    mipmaps: bool,
    vmax: f64, // For SDF
}
impl Drop for TextureData {
    fn drop(&mut self) {
        context::MESSAGE_QUEUE
            .lock()
            .unwrap()
            .push(context::Message::DeleteTexture(self.texture));
        // TODO remove from TEXTURE_DATA ideally...
    }
}

impl TextureData {
    /// Creates a new TextureData of size w x h without any data.
    fn new(ctx: &context::Context, format: TextureFormat, w: usize, h: usize) -> Result<Self> {
        let gl = &ctx.gl;
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
                format.to_gl(),
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
            is_srgb: format.is_srgb(),
            is_sdf: false,
            mipmaps: false,
            vmax: 1.,
        })
    }

    /// Checks to see if a TextureData exists.
    fn exists(name: &str) -> Option<Arc<Self>> {
        let textures = TEXTURE_DATA.lock().unwrap();
        Self::exists_textures(name, &textures)
    }

    /// Checks to see if a TextureData exists.
    fn exists_textures(
        name: &str,
        textures: &MutexGuard<'_, Vec<Weak<TextureData>>>,
    ) -> Option<Arc<Self>> {
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
            mipmaps: false,
            vmax: 1.,
        })
    }

    /// Creates a new TextureData from an image wrapper
    fn from_image(
        ctx: &context::Context,
        name: Option<&str>,
        img: &image::DynamicImage,
    ) -> Result<Self> {
        let gl = &ctx.gl;
        let texture = unsafe { gl.create_texture().map_err(|e| anyhow::anyhow!(e)) }?;

        let has_alpha = img.color().has_alpha();
        let (w, h) = (img.width(), img.height());
        //let imgdata = img.flipv().to_rgba8().into_raw();
        let imgdata = match has_alpha {
            true => img.flipv().to_rgba8().into_raw(),
            false => img.flipv().to_rgb8().into_raw(),
        };

        let is_srgb = true;

        let internalformat = TextureFormat::auto(has_alpha, is_srgb);
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
            let gldata = glow::PixelUnpackData::Slice(Some(imgdata.as_slice()));
            let fmt = match has_alpha {
                true => glow::RGBA,
                false => glow::RGB,
            };
            gl.tex_image_2d(
                glow::TEXTURE_2D,
                0,
                internalformat,
                w as i32,
                h as i32,
                0,
                fmt,
                glow::UNSIGNED_BYTE,
                gldata,
            );
            #[cfg(debug_assertions)]
            gl.object_label(glow::TEXTURE, texture.0.into(), name);
            gl.bind_texture(glow::TEXTURE_2D, None);
        }

        Ok(TextureData {
            name: name.map(String::from),
            w: w as usize,
            h: h as usize,
            texture,
            is_srgb,
            is_sdf: false,
            mipmaps: false,
            vmax: 1.,
        })
    }

    fn generate_mipmap(&mut self, gl: &glow::Context) -> Result<()> {
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(self.texture));
            gl.generate_mipmap(glow::TEXTURE_2D);
            gl.bind_texture(glow::TEXTURE_2D, None);
        }
        self.mipmaps = true;
        Ok(())
    }
}

#[derive(Debug)]
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
    pub mipmaps: bool,
}
impl Drop for Texture {
    fn drop(&mut self) {
        context::MESSAGE_QUEUE
            .lock()
            .unwrap()
            .push(context::Message::DeleteSampler(self.sampler));
    }
}
impl Texture {
    pub fn try_clone(&self) -> Result<Self> {
        let ctx = Context::get().unwrap();
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
        unsafe {
            // Has to be called after it is initialized with any call
            #[cfg(debug_assertions)]
            gl.object_label(glow::SAMPLER, sampler.0.into(), self.path.clone());
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
            mipmaps: self.mipmaps,
        })
    }

    pub fn bind(&self, ctx: &context::Context, idx: u32) {
        let gl = &ctx.gl;
        unsafe {
            gl.active_texture(glow::TEXTURE0 + idx);
            gl.bind_texture(glow::TEXTURE_2D, Some(self.texture.texture));
            gl.bind_sampler(idx, Some(self.sampler));
        }
    }

    pub fn unbind(ctx: &context::Context) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, None);
            gl.bind_sampler(0, None); // TODO handle index?
        }
    }

    pub fn draw(&self, ctx: &context::Context, x: f32, y: f32, w: f32, h: f32) -> Result<()> {
        let dims = ctx.dimensions.lock().unwrap();
        #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             w,  0.0,  x,
            0.0,  h,   y,
            0.0, 0.0, 1.0,
        );
        let uniform = render::TextureUniform {
            transform,
            ..Default::default()
        };
        self.draw_ex(ctx, &uniform)
    }

    pub fn draw_ex(&self, ctx: &context::Context, uniform: &render::TextureUniform) -> Result<()> {
        let gl = &ctx.gl;
        ctx.program_texture.use_program(gl);
        self.bind(ctx, 0);
        ctx.vao_square.bind(ctx);

        ctx.buffer_texture
            .bind_write_base(ctx, &uniform.buffer()?, 0)?;
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }

        Texture::unbind(ctx);
        buffer::VertexArray::unbind(ctx);
        ctx.buffer_texture.unbind(ctx);

        Ok(())
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
    MipmapLinear = 2,
}
impl FilterMode {
    pub fn to_gl(self) -> i32 {
        (match self {
            FilterMode::Nearest => glow::NEAREST,
            FilterMode::Linear => glow::LINEAR,
            FilterMode::MipmapLinear => glow::LINEAR_MIPMAP_LINEAR,
        }) as i32
    }
}

pub enum TextureSource {
    Path(String),
    Image(image::DynamicImage),
    TextureData(Arc<TextureData>),
    Raw(glow::NativeTexture),
    Empty(TextureFormat),
}
impl TextureSource {
    fn to_texture_data(
        &self,
        ctx: &context::Context,
        w: usize,
        h: usize,
        mipmaps: bool,
        name: Option<&str>,
    ) -> Result<Arc<TextureData>> {
        if let TextureSource::TextureData(tex) = self {
            // Purpose dropout, don't want to cache again here
            return Ok(tex.clone());
        };

        let mut textures = TEXTURE_DATA.lock().unwrap();

        // Try to load from name if possible
        if let Some(name) = name {
            if let Some(t) = TextureData::exists_textures(name, &textures) {
                // TODO we would actually have to make sure it has mipmaps if we want them...
                return Ok(t);
            }
        }

        // Failed to find in cache, load a new
        let tex = Arc::new({
            let mut inner = match self {
                TextureSource::Path(path) => {
                    //let bytes = ndata::read(path.as_str())?;
                    //let img = image::load_from_memory(&bytes)?;
                    let rw = ndata::rwops(path.as_str()).map_err(|e| anyhow::anyhow!(e))?;
                    let sur = rw.load().map_err(|e| anyhow::anyhow!(e))?;
                    let img = surface_to_image(sur)?;
                    TextureData::from_image(ctx, name, &img)?
                }
                TextureSource::Image(img) => TextureData::from_image(ctx, name, img)?,
                TextureSource::Raw(tex) => TextureData::from_raw(*tex, w, h)?,
                TextureSource::Empty(fmt) => TextureData::new(ctx, *fmt, w, h)?,
                TextureSource::TextureData(tex) => unreachable!(),
            };
            if mipmaps {
                inner.generate_mipmap(&ctx.gl)?;
            }
            inner
        });

        // Add weak reference to cache
        if name.is_some() {
            textures.push(Arc::downgrade(&tex));
        }

        Ok(tex)
    }
    fn to_texture_data_safe<'a>(
        &self,
        sctx: &'a context::ContextWrapper<'a>,
        w: usize,
        h: usize,
        mipmaps: bool,
        name: Option<&str>,
    ) -> Result<Arc<TextureData>> {
        if let TextureSource::TextureData(tex) = self {
            // Purpose dropout, don't want to cache again here
            return Ok(tex.clone());
        };

        let mut textures = TEXTURE_DATA.lock().unwrap();

        // Try to load from name if possible
        if let Some(name) = name {
            if let Some(t) = TextureData::exists_textures(name, &textures) {
                // TODO we would actually have to make sure it has mipmaps if we want them...
                return Ok(t);
            }
        }

        // Failed to find in cache, load a new
        let tex = Arc::new({
            let mut inner = match self {
                TextureSource::Path(path) => {
                    //let bytes = ndata::read(path.as_str())?;
                    //let img = image::load_from_memory(&bytes)?;
                    let rw = ndata::rwops(path.as_str()).map_err(|e| anyhow::anyhow!(e))?;
                    let sur = rw.load().map_err(|e| anyhow::anyhow!(e))?;
                    let img = surface_to_image(sur)?;
                    let ctx = &sctx.lock();
                    TextureData::from_image(ctx, name, &img)?
                }
                TextureSource::Image(img) => {
                    let ctx = &sctx.lock();
                    TextureData::from_image(ctx, name, img)?
                }
                TextureSource::Raw(tex) => TextureData::from_raw(*tex, w, h)?,
                TextureSource::Empty(fmt) => {
                    let ctx = &sctx.lock();
                    TextureData::new(ctx, *fmt, w, h)?
                }
                TextureSource::TextureData(tex) => unreachable!(),
            };
            if mipmaps {
                let ctx = &sctx.lock();
                inner.generate_mipmap(&ctx.gl)?;
            }
            inner
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
    format: TextureFormat,
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
            source: TextureSource::Empty(TextureFormat::SRGBA),
            format: TextureFormat::SRGBA,
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

    pub fn empty(mut self, fmt: TextureFormat) -> Self {
        self.source = TextureSource::Empty(fmt);
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

    pub fn build(self, ctx: &context::Context) -> Result<Texture> {
        let gl = &ctx.gl;
        /* TODO handle SDF. */
        let texture =
            self.source
                .to_texture_data(ctx, self.w, self.h, self.mipmaps, self.name.as_deref())?;
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
            #[cfg(debug_assertions)]
            gl.object_label(glow::SAMPLER, sampler.0.into(), self.name.clone());
        }

        let (w, h) = (texture.w, texture.h);
        let (sx, sy) = (self.sx, self.sy);
        let sw = (w as f64) / (sx as f64);
        let sh = (h as f64) / (sy as f64);
        let srw = sw / (w as f64);
        let srh = sh / (h as f64);

        Ok(Texture {
            path: self.name.clone(),
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
            mipmaps: self.mipmaps,
        })
    }

    pub fn build_safe<'a>(self, sctx: &'a context::ContextWrapper<'a>) -> Result<Texture> {
        /* TODO handle SDF. */
        let texture = self.source.to_texture_data_safe(
            sctx,
            self.w,
            self.h,
            self.mipmaps,
            self.name.as_deref(),
        )?;

        let ctx = &sctx.lock();
        let gl = &ctx.gl;
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
            #[cfg(debug_assertions)]
            gl.object_label(glow::SAMPLER, sampler.0.into(), self.name.clone());
        }

        let (w, h) = (texture.w, texture.h);
        let (sx, sy) = (self.sx, self.sy);
        let sw = (w as f64) / (sx as f64);
        let sh = (h as f64) / (sy as f64);
        let srw = sw / (w as f64);
        let srh = sh / (h as f64);

        Ok(Texture {
            path: self.name.clone(),
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
            mipmaps: self.mipmaps,
        })
    }
}

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
            Self::Framebuffer(fb) => (fb.texture.texture.w, fb.texture.texture.h),
            Self::FramebufferC(fb) => (fb.w, fb.h),
        }
    }

    pub fn bind(&self, ctx: &context::Context) {
        let fb = match self {
            Self::Screen => None,
            Self::Framebuffer(fb) => Some(fb.framebuffer),
            Self::FramebufferC(fb) => Some(fb.fb),
        };
        unsafe {
            ctx.gl.bind_framebuffer(glow::FRAMEBUFFER, fb);
        }
    }
}

pub struct Framebuffer {
    pub framebuffer: glow::Framebuffer,
    pub w: usize,
    pub h: usize,
    pub texture: Texture,
    pub depth: Option<Texture>,
}
impl Drop for Framebuffer {
    fn drop(&mut self) {
        let ctx = Context::get().unwrap();
        unsafe { ctx.gl.delete_framebuffer(self.framebuffer) };
    }
}
impl Framebuffer {
    pub fn bind(&self, ctx: &context::Context) {
        unsafe {
            ctx.gl
                .bind_framebuffer(glow::FRAMEBUFFER, Some(self.framebuffer));
        }
    }

    pub fn unbind(ctx: &context::Context) {
        unsafe {
            ctx.gl.bind_framebuffer(glow::FRAMEBUFFER, None);
        }
    }
}

pub struct FramebufferBuilder {
    name: Option<String>,
    w: usize,
    h: usize,
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
            depth: false,
            filter: FilterMode::Linear,
            address_mode: AddressMode::ClampToBorder,
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

    pub fn build(self, ctx: &context::Context) -> Result<Framebuffer> {
        let gl = &ctx.gl;

        let texture = TextureBuilder::new()
            .width(self.w)
            .height(self.h)
            .filter(self.filter)
            .address_mode(self.address_mode)
            .build(ctx)?;

        let framebuffer = unsafe { gl.create_framebuffer().map_err(|e| anyhow::anyhow!(e)) }?;
        texture.bind(ctx, 0);
        unsafe {
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(framebuffer));
            gl.framebuffer_texture_2d(
                glow::FRAMEBUFFER,
                glow::COLOR_ATTACHMENT0,
                glow::TEXTURE_2D,
                Some(texture.texture.texture),
                0,
            );
            #[cfg(debug_assertions)]
            gl.object_label(glow::FRAMEBUFFER, framebuffer.0.into(), self.name);
        }

        let depth = if self.depth {
            let depth = TextureBuilder::new()
                .empty(TextureFormat::Depth)
                .width(self.w)
                .height(self.h)
                .filter(self.filter)
                .address_mode(self.address_mode)
                .build(ctx)?;
            depth.bind(ctx, 0);
            unsafe {
                gl.framebuffer_texture_2d(
                    glow::FRAMEBUFFER,
                    glow::DEPTH_ATTACHMENT,
                    glow::TEXTURE_2D,
                    Some(depth.texture.texture),
                    0,
                );
            }
            Some(depth)
        } else {
            None
        };

        let status = unsafe { gl.check_framebuffer_status(glow::FRAMEBUFFER) };
        if status != glow::FRAMEBUFFER_COMPLETE {
            anyhow::bail!("error setting up framebuffer");
        }

        unsafe {
            Texture::unbind(ctx);
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

// BELOW THIS IS THE C API CODE

struct Flags {
    maptrans: bool,
    mipmaps: bool,
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
            skipcache: (flags & naevc::OPENGL_TEX_SKIPCACHE) > 0,
            sdf: (flags & naevc::OPENGL_TEX_SDF) > 0,
            clamp_alpha: (flags & naevc::OPENGL_TEX_CLAMP_ALPHA) > 0,
            notsrgb: (flags & naevc::OPENGL_TEX_NOTSRGB) > 0,
        }
    }
}

macro_rules! capi_tex {
    ($funcname: ident, $field: tt) => {
        #[unsafe(no_mangle)]
        pub extern "C" fn $funcname(ctex: *mut Texture) -> c_double {
            let tex = unsafe { &*ctex };
            tex.texture.$field as f64
        }
    };
}
macro_rules! capi {
    ($funcname: ident, $field: tt) => {
        #[unsafe(no_mangle)]
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

#[unsafe(no_mangle)]
pub extern "C" fn gl_texExistsOrCreate(
    cpath: *const c_char,
    cflags: c_uint,
    sx: c_int,
    sy: c_int,
    created: *mut c_int,
) -> *mut Texture {
    let ctx = Context::get().unwrap(); /* Lock early. */

    unsafe {
        naevc::gl_contextSet();
    }

    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);
    let mut builder = TextureBuilder::new()
        .sx(sx as usize)
        .sy(sy as usize)
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

    let out = match builder.build(ctx) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        _ => std::ptr::null_mut(),
    };
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_loadImageData(
    data: *const c_float,
    w: c_int,
    h: c_int,
    sx: c_int,
    sy: c_int,
    cname: *const c_char,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = Context::get().unwrap(); /* Lock early. */
    let name = unsafe { CStr::from_ptr(cname) };
    let flags = Flags::from(cflags);

    unsafe {
        naevc::gl_contextSet();
    }

    let mut builder = TextureBuilder::new()
        .name(Some(name.to_str().unwrap()))
        .sx(sx as usize)
        .sy(sy as usize)
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
                unsafe {
                    naevc::gl_contextUnset();
                }
                return std::ptr::null_mut();
            }
        };
        let img = image::DynamicImage::ImageRgba32F(buf);
        builder = builder.image(&img);
    }

    let out = match builder.build(ctx) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        Err(e) => {
            warn_err!(e);
            std::ptr::null_mut()
        }
    };
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_newImage(cpath: *const c_char, flags: c_uint) -> *mut Texture {
    gl_newSprite(cpath, 1, 1, flags)
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_newSprite(
    cpath: *const c_char,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = Context::get().unwrap(); /* Lock early. */
    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);

    unsafe {
        naevc::gl_contextSet();
    }

    let mut builder = TextureBuilder::new()
        .path(path.to_str().unwrap())
        .sx(sx as usize)
        .sy(sy as usize)
        .srgb(!flags.notsrgb)
        .mipmaps(flags.mipmaps);

    if flags.clamp_alpha {
        builder = builder.border(Some(Vector4::<f32>::new(0., 0., 0., 0.)));
    }

    let out = match builder.build(ctx) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        Err(e) => {
            warn_err(e.context("unable to build texture for new sprite"));
            std::ptr::null_mut()
        }
    };
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_newSpriteRWops(
    cpath: *const c_char,
    rw: *mut naevc::SDL_RWops,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = Context::get().unwrap(); /* Lock early. */
    let path = unsafe { CStr::from_ptr(cpath) };
    let flags = Flags::from(cflags);
    unsafe {
        naevc::gl_contextSet();
    }

    let mut builder = TextureBuilder::new()
        .sx(sx as usize)
        .sy(sy as usize)
        .srgb(!flags.notsrgb)
        .mipmaps(flags.mipmaps);

    if flags.mipmaps {
        builder = builder.min_filter(FilterMode::MipmapLinear);
    }
    if flags.clamp_alpha {
        builder = builder.border(Some(Vector4::<f32>::new(0., 0., 0., 0.)));
    }

    let pathname = path.to_str().unwrap();
    builder = match TextureData::exists(pathname) {
        Some(tex) => builder.texture_data(&tex),
        None => {
            let rw = unsafe { sdl::rwops::RWops::from_ll(rw as *mut sdl::sys::SDL_RWops) };
            /* TODO support image when it's faster...
            let img = image::ImageReader::new(std::io::BufReader::new(rw))
                .with_guessed_format()
                .unwrap()
                .decode()
                .unwrap();
            */
            let img = match rw.load() {
                Ok(sur) => surface_to_image(sur).unwrap(),
                Err(e) => {
                    // SDL2 uses strings as errors...
                    //warn_err!(e, "unable to load image '{}'", pathname);
                    warn!("unable to load image '{}': {}", pathname, e);
                    return std::ptr::null_mut();
                }
            };
            builder.image(&img)
        }
    };

    let out = match builder.build(ctx) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        Err(e) => {
            warn_err(e.context("unable to build texture for new sprite from rwops"));
            std::ptr::null_mut()
        }
    };
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_dupTexture(ctex: *mut Texture) -> *mut Texture {
    if ctex.is_null() {
        return ctex;
    }
    unsafe {
        naevc::gl_contextSet();
    }
    let tex = unsafe { &*ctex };
    unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
    let out = Box::into_raw(Box::new(tex.try_clone().unwrap()));
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_rawTexture(
    cpath: *mut c_char,
    tex: naevc::GLuint,
    w: c_double,
    h: c_double,
) -> *mut Texture {
    let ctx = Context::get().unwrap(); /* Lock early. */
    unsafe {
        naevc::gl_contextSet();
    }
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
                let tex = glow::NativeTexture(NonZero::new(tex).unwrap());
                builder.native_texture(tex)
            }
        },
        None => {
            let tex = glow::NativeTexture(NonZero::new(tex).unwrap());
            builder.native_texture(tex)
        }
    };

    let out = match builder.build(ctx) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex))
        }
        Err(e) => {
            warn_err(e.context("unable to build texture for raw texture"));
            std::ptr::null_mut()
        }
    };
    unsafe {
        naevc::gl_contextUnset();
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_freeTexture(ctex: *mut Texture) {
    if !ctex.is_null() {
        let _ = unsafe { Box::from_raw(ctex) };
    }
    // The texture should get dropped now
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_tex(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.texture.texture.0.into()
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_sampler(ctex: *mut Texture) -> naevc::GLuint {
    let tex = unsafe { &*ctex };
    tex.sampler.0.into()
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_name(ctex: *mut Texture) -> *const c_char {
    let tex = unsafe { &*ctex };
    match &tex.name {
        Some(name) => name.as_ptr(),
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_isSDF(ctex: *mut Texture) -> c_int {
    let tex = unsafe { &*ctex };
    tex.texture.is_sdf as i32
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_isTrans(ctex: *mut Texture, x: c_int, y: c_int) -> c_int {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_hasTrans(ctex: *mut Texture) -> c_int {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_setTex(ctex: *mut Texture, texture: naevc::GLuint) {
    let tex = unsafe { &mut *ctex };
    let ntex = glow::NativeTexture(std::num::NonZero::new(texture).unwrap());
    tex.texture = Arc::new(TextureData::from_raw(ntex, tex.texture.w, tex.texture.h).unwrap());
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderTexture(
    ctex: *mut Texture,
    x: c_double,
    y: c_double,
    w: c_double,
    h: c_double,
    tx: c_double,
    ty: c_double,
    tw: c_double,
    th: c_double,
    c: *mut Vector4<f32>,
    angle: c_double,
) {
    let ctx = Context::get().unwrap();
    let colour = match c.is_null() {
        true => Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        false => unsafe { *c },
    };
    let dims = ctx.dimensions.lock().unwrap();
    #[rustfmt::skip]
    let transform: Matrix3<f32> = dims.projection * {
        if angle.abs() > 1e-5 {
            let hw = 0.5 * w as f32;
            let hh = 0.5 * h as f32;
            let c = angle.cos() as f32;
            let s = angle.sin() as f32;
            Matrix3::new(
                1.0, 0.0, x as f32 + hw,
                0.0, 1.0, y as f32 + hh,
                0.0, 0.0, 1.0,
            ) * Matrix3::new(
                 c,  -s,  0.0,
                 s,   c,  0.0,
                0.0, 0.0, 1.0,
            ) * Matrix3::new(
                w as f32, 0.0,      -hw,
                0.0,      h as f32, -hh,
                0.0,      0.0,      1.0,
            )
        } else {
            Matrix3::new(
                w as f32, 0.0,      x as f32,
                0.0,      h as f32, y as f32,
                0.0,      0.0,      1.0,
            )
        }
    };
    // Our coordinate system rust-side is inverted with respect to Lua and textures
    #[rustfmt::skip]
    let texture: Matrix3<f32> = Matrix3::new(
        tw as f32, 0.0,       tx as f32,
        0.0,      th as f32, ty as f32,
        0.0,       0.0,       1.0,
    );
    let data = render::TextureUniform {
        texture,
        transform,
        colour,
    };

    let tex = unsafe { &*ctex };
    let _ = tex.draw_ex(ctx, &data);
}
