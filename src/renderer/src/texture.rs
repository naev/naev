use anyhow::Context as AnyhowContext;
use anyhow::Result;
use glow::*;
use log::{warn, warn_err};
use nalgebra::{Matrix3, Vector4};
use sdl3 as sdl;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use std::num::NonZero;
use std::os::raw::{c_char, c_double, c_float, c_int, c_uint};
use std::sync::{atomic::AtomicU32, Arc, LazyLock, Mutex, MutexGuard, Weak};

use crate::buffer;
use crate::{
    Context, ContextWrapper, TextureSDFUniform, TextureScaleUniform, TextureUniform, Uniform,
};

/// All the shared texture data to look up
pub static TEXTURE_DATA: LazyLock<Mutex<Vec<Weak<TextureData>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));
/// Counter for how many textures were destroyed
pub static GC_COUNTER: AtomicU32 = AtomicU32::new(0);
/// Number of destroyed textures to start garbage collecting the cache
pub const GC_THRESHOLD: u32 = 128;

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

    pub fn to_gl(self) -> u32 {
        match self {
            Self::RGB => glow::RGB,
            Self::RGBA => glow::RGBA,
            Self::SRGB => glow::RGB,
            Self::SRGBA => glow::RGBA,
            Self::Depth => glow::DEPTH_COMPONENT,
        }
    }

    pub fn from_gl(val: u32) -> Self {
        match val {
            glow::RGB => Self::RGB,
            glow::RGBA => Self::RGBA,
            glow::SRGB => Self::SRGB,
            glow::SRGB_ALPHA => Self::SRGBA,
            glow::DEPTH_COMPONENT => Self::Depth,
            _ => Self::RGB,
        }
    }

    pub fn to_sized_gl(self) -> i32 {
        (match self {
            Self::RGB => glow::RGB,
            Self::RGBA => glow::RGBA,
            Self::SRGB => glow::SRGB,
            Self::SRGBA => glow::SRGB_ALPHA,
            Self::Depth => glow::DEPTH_COMPONENT32,
        }) as i32
    }

    pub fn is_srgb(self) -> bool {
        match self {
            Self::RGB | Self::RGBA | Self::Depth => false,
            Self::SRGB | Self::SRGBA => true,
        }
    }
}

struct TextureSearch<'a> {
    name: &'a str,
    srgb: bool,
    flipv: bool,
    mipmaps: bool,
    sdf: bool,
}

#[derive(Debug)]
pub struct TextureData {
    /// Name of the texture
    name: Option<String>,
    /// Underlying OpenGL texture
    texture: glow::Texture,
    /// Width of the texture
    pub w: usize,
    /// Height of the texture
    pub h: usize,
    /// Whether or not the image is in SRGB format
    srgb: bool,
    /// Whether or not the image is a Signed Distance Function
    sdf: bool,
    /// Whether or not the texture has mipmaps
    mipmaps: bool,
    /// Maximum value for Signed Distance Function images
    vmax: f32,
    /// Whether or not it was flipped
    flipv: bool,
}
impl Drop for TextureData {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteTexture(self.texture));
    }
}

impl TextureData {
    /// Creates a new TextureData of size w x h without any data.
    fn new(ctx: &Context, format: TextureFormat, w: usize, h: usize) -> Result<Self> {
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
                format.to_sized_gl(),
                w as i32,
                h as i32,
                0,
                format.to_gl(),
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
            srgb: format.is_srgb(),
            sdf: false,
            mipmaps: false,
            vmax: 1.,
            flipv: false,
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

    /// Searches to see if a TextureData exists, not only considering name
    fn search_textures(
        s: &TextureSearch,
        textures: &MutexGuard<'_, Vec<Weak<TextureData>>>,
    ) -> Option<Arc<Self>> {
        for tex in textures.iter() {
            if let Some(t) = tex.upgrade() {
                if let Some(tname) = &t.name {
                    if s.name == tname
                        && s.srgb == t.srgb
                        && s.flipv == t.flipv
                        && s.mipmaps == t.mipmaps
                        && s.sdf == t.sdf
                    {
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
            srgb: true,
            sdf: false,
            mipmaps: false,
            vmax: 1.,
            flipv: false,
        })
    }

    /// Creates a new TextureData from an image wrapper
    fn from_image(
        ctx: &Context,
        name: Option<&str>,
        img: image::DynamicImage,
        flipv: bool,
        srgb: bool,
    ) -> Result<Self> {
        let gl = &ctx.gl;

        let has_alpha = img.color().has_alpha();
        let img = match flipv {
            true => img.flipv(),
            false => img,
        };

        let (imgdata, fmt): (image::DynamicImage, u32) = match has_alpha {
            //true => (img.to_rgba8().into_flat_samples(), glow::RGBA),
            //false => (img.to_rgb8().into_flat_samples(), glow::RGB),
            true => (img.into_rgba8().into(), glow::RGBA),
            false => (img.into_rgb8().into(), glow::RGB),
        };
        let (w, h) = (imgdata.width(), imgdata.height());

        let internalformat = TextureFormat::auto(has_alpha, srgb);
        let gldata = glow::PixelUnpackData::Slice(Some(imgdata.as_bytes()));

        let texture = unsafe {
            let texture = gl.create_texture().map_err(|e| anyhow::anyhow!(e))?;
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
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
            if gl.supports_debug() {
                gl.object_label(glow::TEXTURE, texture.0.into(), name);
            }
            gl.bind_texture(glow::TEXTURE_2D, None);
            texture
        };

        Ok(TextureData {
            name: name.map(String::from),
            w: w as usize,
            h: h as usize,
            texture,
            srgb,
            sdf: false,
            mipmaps: false,
            vmax: 1.,
            flipv,
        })
    }

    /// Creates a new TextureData from an image wrapper
    fn from_image_sdf(
        ctx: &Context,
        name: Option<&str>,
        img: image::DynamicImage,
        flipv: bool,
    ) -> Result<Self> {
        let gl = &ctx.gl;
        let texture = unsafe { gl.create_texture().map_err(|e| anyhow::anyhow!(e)) }?;

        let has_alpha = img.color().has_alpha();
        if !has_alpha {
            anyhow::bail!("Trying to create SDF from image without alpha!");
        }
        let img = match flipv {
            true => img.flipv(),
            false => img,
        };
        let (w, h) = (img.width(), img.height());
        let (imgdata, vmax) = {
            // Get only the alpha channel
            let mut rawdata: Vec<_> = img.into_luma_alpha8().pixels().map(|p| p.0[1]).collect();
            let mut vmax: f64 = 0.0;
            // Compute the distance transform
            let sdfdata = unsafe {
                let data = naevc::make_distance_mapbf(rawdata.as_mut_ptr(), w, h, &mut vmax);
                std::slice::from_raw_parts(data, (w * h) as usize)
            };
            (sdfdata, vmax)
        };

        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
            let (_prefix, floats, _suffix) = imgdata.align_to::<u8>();
            let gldata = glow::PixelUnpackData::Slice(Some(floats));
            gl.tex_image_2d(
                glow::TEXTURE_2D,
                0,
                glow::RED as i32,
                w as i32,
                h as i32,
                0,
                glow::RED,
                glow::FLOAT,
                gldata,
            );
            if gl.supports_debug() {
                gl.object_label(glow::TEXTURE, texture.0.into(), name);
            }
            gl.bind_texture(glow::TEXTURE_2D, None);
        }

        Ok(TextureData {
            name: name.map(String::from),
            w: w as usize,
            h: h as usize,
            texture,
            srgb: false,
            sdf: true,
            mipmaps: false,
            vmax: vmax as f32,
            flipv,
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
}
impl Drop for Texture {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteSampler(self.sampler));
    }
}
impl Texture {
    fn copy_sampler_params(gl: &glow::Context, dst: &glow::Sampler, src: &glow::Sampler) {
        for param in [
            glow::TEXTURE_WRAP_S,
            glow::TEXTURE_WRAP_T,
            glow::TEXTURE_MIN_FILTER,
            glow::TEXTURE_MAG_FILTER,
        ] {
            unsafe {
                let val = gl.get_sampler_parameter_i32(*src, param);
                gl.sampler_parameter_i32(*dst, param, val);
            }
        }
        // TODO copy border, but not possible atm because of
        // https://github.com/grovesNL/glow/issues/342
    }

    pub fn try_clone(&self) -> Result<Self> {
        let ctx = Context::get();
        let gl = &ctx.gl;
        let sampler = unsafe { gl.create_sampler() }.map_err(|e| anyhow::anyhow!(e))?;
        Self::copy_sampler_params(gl, &sampler, &self.sampler);
        if gl.supports_debug() {
            unsafe {
                gl.object_label(glow::SAMPLER, sampler.0.into(), self.path.clone());
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

    pub fn scale(&self, ctx: &Context, w: usize, h: usize) -> Result<Self> {
        self.scale_wrap(&ctx.as_wrap(), w, h)
    }

    pub fn scale_wrap(&self, wctx: &ContextWrapper, w: usize, h: usize) -> Result<Self> {
        let fbo = FramebufferBuilder::new(Some("Downscaler"))
            .width(w)
            .height(h)
            .build_wrap(wctx)?;

        let ctx = wctx.lock();
        let gl = &ctx.gl;
        fbo.bind_gl(gl);
        unsafe {
            gl.viewport(0, 0, w as i32, h as i32);
        }
        let scale = (w as f32 / self.texture.w as f32).min(h as f32 / self.texture.h as f32);
        let uniform = TextureScaleUniform {
            transform: crate::ortho3(0.0, 1.0, 0.0, 1.0),
            scale,
            ..Default::default()
        };
        ctx.program_texture_scale.use_program(gl);
        self.bind_gl(gl, 0);
        ctx.vao_square.bind_gl(gl);

        ctx.buffer_texture_scale
            .bind_write_base_gl(gl, &uniform.buffer()?, 0)?;
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }

        Texture::unbind_gl(gl);
        //buffer::VertexArray::unbind_wrap(ctx);
        unsafe {
            gl.bind_vertex_array(Some(ctx.vao_core));
        }
        ctx.buffer_texture_scale.unbind_gl(gl);

        Framebuffer::unbind_gl(gl);
        unsafe {
            gl.viewport(0, 0, naevc::gl_screen.rw, naevc::gl_screen.rh);
        }

        // Have to copy the parameters over
        let tex = fbo.into_texture()?;
        Self::copy_sampler_params(gl, &tex.sampler, &self.sampler);
        Ok(tex)
    }

    pub fn bind(&self, ctx: &Context, idx: u32) {
        self.bind_gl(&ctx.gl, idx)
    }

    pub fn bind_gl(&self, gl: &glow::Context, idx: u32) {
        unsafe {
            gl.active_texture(glow::TEXTURE0 + idx);
            gl.bind_texture(glow::TEXTURE_2D, Some(self.texture.texture));
            gl.bind_sampler(idx, Some(self.sampler));
        }
    }

    pub fn unbind(ctx: &Context) {
        Self::unbind_gl(&ctx.gl)
    }

    pub fn unbind_gl(gl: &glow::Context) {
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, None);
            gl.bind_sampler(0, None); // TODO handle index?
        }
    }

    pub fn draw(&self, ctx: &Context, x: f32, y: f32, w: f32, h: f32) -> Result<()> {
        let dims = ctx.dimensions.read().unwrap();
        #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             w,  0.0,  x,
            0.0,  h,   y,
            0.0, 0.0, 1.0,
        );
        let uniform = TextureUniform {
            transform,
            ..Default::default()
        };
        self.draw_ex(ctx, &uniform)
    }

    pub fn draw_ex(&self, ctx: &Context, uniform: &TextureUniform) -> Result<()> {
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

    pub fn draw_sdf_ex(
        &self,
        ctx: &Context,
        uniform: &TextureUniform,
        sdf: &TextureSDFUniform,
    ) -> Result<()> {
        let gl = &ctx.gl;
        ctx.program_texture_sdf.use_program(gl);
        self.bind(ctx, 0);
        ctx.vao_square.bind(ctx);

        ctx.buffer_texture_sdf
            .bind_write_base(ctx, &sdf.buffer()?, 1)?;
        ctx.buffer_texture
            .bind_write_base(ctx, &uniform.buffer()?, 0)?;
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }

        Texture::unbind(ctx);
        buffer::VertexArray::unbind(ctx);
        ctx.buffer_texture_sdf.unbind(ctx);
        ctx.buffer_texture.unbind(ctx);

        Ok(())
    }

    pub fn draw_scale(
        &self,
        ctx: &Context,
        x: f32,
        y: f32,
        w: f32,
        h: f32,
        scale: f32,
    ) -> Result<()> {
        let dims = ctx.dimensions.read().unwrap();
        #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             w,  0.0,  x,
            0.0,  h,   y,
            0.0, 0.0, 1.0,
        );
        let uniform = TextureScaleUniform {
            transform,
            scale,
            ..Default::default()
        };
        self.draw_scale_ex(ctx, &uniform)
    }

    pub fn draw_scale_ex(&self, ctx: &Context, uniform: &TextureScaleUniform) -> Result<()> {
        self.draw_scale_ex_wrap(&ctx.as_wrap(), uniform)
    }

    pub fn draw_scale_ex_wrap(
        &self,
        wctx: &ContextWrapper,
        uniform: &TextureScaleUniform,
    ) -> Result<()> {
        let ctx = &wctx.lock();
        let gl = &ctx.gl;
        ctx.program_texture_scale.use_program(gl);
        self.bind(ctx, 0);
        ctx.vao_square.bind(ctx);

        ctx.buffer_texture_scale
            .bind_write_base(ctx, &uniform.buffer()?, 0)?;
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }

        Texture::unbind(ctx);
        buffer::VertexArray::unbind(ctx);
        ctx.buffer_texture_scale.unbind(ctx);
        Ok(())
    }

    pub fn into_ptr(self) -> *mut Self {
        unsafe { Arc::increment_strong_count(Arc::into_raw(self.texture.clone())) }
        Box::into_raw(Box::new(self))
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

/// Loads an SVG file into a DynamicImage
fn svg_to_img(path: &str, w: Option<usize>, h: Option<usize>) -> Result<image::DynamicImage> {
    use resvg::{tiny_skia, usvg};

    // Load the SVG
    // TODO add ndata-based href resolves and such
    let svg_data = ndata::read(path)?;
    let opt = usvg::Options {
        resources_dir: None, // TODO add and such
        ..Default::default()
    };
    let tree = usvg::Tree::from_data(&svg_data, &opt)?;

    // Render the SVG
    let (iw, ih) = tree.size().to_int_size().dimensions();
    let transform = {
        let sx = match w {
            Some(w) => w as f32 / iw as f32,
            None => 1.0,
        };
        let sy = match h {
            Some(h) => h as f32 / ih as f32,
            None => sx,
        };
        tiny_skia::Transform::from_scale(sx, sy)
    };
    let mut pixmap = tiny_skia::Pixmap::new(iw, ih).context("unable to create pixbuf")?;
    resvg::render(&tree, transform, &mut pixmap.as_mut());

    // Convert to image and return
    Ok(image::RgbaImage::from_vec(iw, ih, pixmap.take())
        .context("unable to create RgbaImage from Pixmap")?
        .into())
}

pub enum TextureSource {
    Path(String),
    Image(image::DynamicImage),
    TextureData(Arc<TextureData>),
    Raw(glow::NativeTexture),
    Empty(TextureFormat),
}
impl TextureSource {
    #[allow(clippy::too_many_arguments)]
    fn into_texture_data(
        self,
        sctx: &ContextWrapper,
        w: usize,
        h: usize,
        srgb: bool,
        flipv: bool,
        mipmaps: bool,
        sdf: bool,
        name: Option<&str>,
    ) -> Result<Arc<TextureData>> {
        if let TextureSource::TextureData(tex) = self {
            // Purpose dropout, don't want to cache again here
            return Ok(tex.clone());
        };

        let mut textures = TEXTURE_DATA.lock().unwrap();

        // Try to see if a texture that matches everything already exists
        if let Some(name) = name {
            let search = TextureSearch {
                name,
                srgb,
                flipv,
                mipmaps,
                sdf,
            };
            if let Some(t) = TextureData::search_textures(&search, &textures) {
                return Ok(t);
            }
        }

        // Failed to find in cache, load a new
        let tex = Arc::new({
            let mut inner = match self {
                TextureSource::Path(path) => {
                    let img = {
                        let cpath = ndata::simplify_path(&path)?;
                        if std::path::Path::new(&cpath)
                            .extension()
                            .and_then(|s| s.to_str())
                            == Some("svg")
                        {
                            svg_to_img(&cpath, Some(w), Some(h))?
                        } else {
                            let rw = ndata::iostream(&cpath)?;
                            image::ImageReader::with_format(
                                std::io::BufReader::new(rw),
                                image::ImageFormat::from_path(path)?,
                            )
                            //let img = image::ImageReader::new(std::io::BufReader::new(rw)).with_guessed_format()?
                            .decode()?
                        }
                    };
                    let ctx = &sctx.lock();
                    match sdf {
                        true => TextureData::from_image_sdf(ctx, name, img, flipv)?,
                        false => TextureData::from_image(ctx, name, img, flipv, srgb)?,
                    }
                }
                TextureSource::Image(img) => {
                    let ctx = &sctx.lock();
                    match sdf {
                        true => TextureData::from_image_sdf(ctx, name, img, flipv)?,
                        false => TextureData::from_image(ctx, name, img, flipv, srgb)?,
                    }
                }
                TextureSource::Raw(tex) => TextureData::from_raw(tex, w, h)?,
                TextureSource::Empty(fmt) => {
                    let ctx = &sctx.lock();
                    TextureData::new(ctx, fmt, w, h)?
                }
                TextureSource::TextureData(_) => unreachable!(),
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
    source: Option<TextureSource>,
    w: usize,
    h: usize,
    sx: usize,
    sy: usize,
    is_srgb: bool,
    is_sdf: bool,
    is_flipv: bool,
    border_value: Option<Vector4<f32>>,
    address_u: AddressMode,
    address_v: AddressMode,
    mag_filter: FilterMode,
    min_filter: FilterMode,
    mipmaps: bool,
}

impl Default for TextureBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl TextureBuilder {
    pub fn new() -> Self {
        TextureBuilder {
            name: None,
            source: Some(TextureSource::Empty(TextureFormat::SRGBA)),
            w: 0,
            h: 0,
            sx: 1,
            sy: 1,
            is_srgb: true,
            is_sdf: false,
            is_flipv: true, // We flip by so it matches the viewport, but not the 3D textures
            border_value: None,
            address_u: AddressMode::Repeat,
            address_v: AddressMode::Repeat,
            mag_filter: FilterMode::Linear,
            min_filter: FilterMode::Linear,
            mipmaps: false,
        }
    }

    pub fn name(mut self, name: Option<&str>) -> Self {
        self.name = name.map(String::from);
        self
    }

    pub fn path(mut self, path: &str) -> Self {
        self.source = Some(TextureSource::Path(String::from(path)));
        self.name = Some(String::from(path));
        self
    }

    pub fn empty(mut self, fmt: TextureFormat) -> Self {
        self.source = Some(TextureSource::Empty(fmt));
        self
    }

    pub fn image(mut self, img: &image::DynamicImage) -> Self {
        self.source = Some(TextureSource::Image(img.clone()));
        self
    }

    pub fn texture_data(mut self, data: &Arc<TextureData>) -> Self {
        self.source = Some(TextureSource::TextureData(data.clone()));
        self
    }

    pub fn native_texture(mut self, tex: glow::NativeTexture) -> Self {
        self.source = Some(TextureSource::Raw(tex));
        self
    }

    pub fn srgb(mut self, enable: bool) -> Self {
        self.is_srgb = enable;
        self
    }

    pub fn sdf(mut self, enable: bool) -> Self {
        self.is_sdf = enable;
        self.border(Some(Vector4::from([0., 0., 0., 0.])))
    }

    pub fn flipv(mut self, enable: bool) -> Self {
        self.is_flipv = enable;
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
        match border_value {
            Some(_) => self.address_mode(AddressMode::ClampToBorder),
            None => self,
        }
    }

    pub fn mipmaps(mut self, enable: bool) -> Self {
        self.mipmaps = enable;
        self
    }

    pub fn build(self, ctx: &Context) -> Result<Texture> {
        let wctx: ContextWrapper = ctx.into();
        self.build_wrap(&wctx)
    }

    pub fn build_wrap(mut self, sctx: &ContextWrapper) -> Result<Texture> {
        let source = self
            .source
            .take()
            .ok_or(anyhow::anyhow!("texture source not specified"))?;

        // Some checks
        if self.is_sdf {
            match source {
                TextureSource::TextureData(_) | TextureSource::Raw(_) | TextureSource::Empty(_) => {
                    anyhow::bail!("SDF must use Path of Image as source!")
                }
                _ => (),
            }
        }

        let texture = source.into_texture_data(
            sctx,
            self.w,
            self.h,
            self.is_srgb,
            self.is_flipv,
            self.mipmaps,
            self.is_sdf,
            self.name.as_deref(),
        )?;

        // Create the sampler
        let sampler = {
            let ctx = &sctx.lock();
            let gl = &ctx.gl;
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
                if gl.supports_debug() {
                    gl.object_label(glow::SAMPLER, sampler.0.into(), self.name.clone());
                }
            }
            sampler
        };

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

pub struct Framebuffer {
    pub framebuffer: glow::Framebuffer,
    pub w: usize,
    pub h: usize,
    pub texture: Option<Texture>,
    pub depth: Option<Texture>,
}
impl Drop for Framebuffer {
    fn drop(&mut self) {
        let ctx = Context::get();
        unsafe { ctx.gl.delete_framebuffer(self.framebuffer) };
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
                .width(self.w)
                .height(self.h)
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
                .width(self.w)
                .height(self.h)
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

// BELOW THIS IS THE C API CODE

#[allow(dead_code)]
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
            if ctex.is_null() {
                warn!("Received NULL texture!");
                return 0.0;
            }
            let tex = unsafe { &*ctex };
            tex.texture.$field as f64
        }
    };
}
macro_rules! capi {
    ($funcname: ident, $field: tt) => {
        #[unsafe(no_mangle)]
        pub extern "C" fn $funcname(ctex: *mut Texture) -> c_double {
            if ctex.is_null() {
                warn!("Received NULL texture!");
                return 0.0;
            }
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
    let ctx = Context::get(); /* Lock early. */

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
        Ok(tex) => tex.into_ptr(),
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
    _cflags: c_uint,
) -> *mut Texture {
    let ctx = Context::get(); /* Lock early. */
    let name = unsafe { CStr::from_ptr(cname) };
    //let flags = Flags::from(cflags);

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
        Ok(tex) => tex.into_ptr(),
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
    let ctx = Context::get(); /* Lock early. */
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
        Ok(tex) => tex.into_ptr(),
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
    rw: *mut naevc::SDL_IOStream,
    sx: c_int,
    sy: c_int,
    cflags: c_uint,
) -> *mut Texture {
    let ctx = Context::get(); /* Lock early. */
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
            let rw = unsafe {
                sdl::iostream::IOStream::from_ll(rw as *mut sdl::sys::iostream::SDL_IOStream)
            };
            let img = image::ImageReader::new(std::io::BufReader::new(rw))
                .with_guessed_format()
                .unwrap()
                .decode()
                .unwrap();
            builder.image(&img)
        }
    };

    let out = match builder.build(ctx) {
        Ok(tex) => tex.into_ptr(),
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
    let ctx = Context::get(); /* Lock early. */
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
        Ok(tex) => tex.into_ptr(),
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
    tex.texture.sdf as i32
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_isTrans(_ctex: *mut Texture, _x: c_int, _y: c_int) -> c_int {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_hasTrans(_ctex: *mut Texture) -> c_int {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn tex_setTex(ctex: *mut Texture, texture: naevc::GLuint) {
    if ctex.is_null() {
        return;
    }
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
    if ctex.is_null() {
        return;
    }

    let ctx = Context::get();
    let colour = match c.is_null() {
        true => Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        false => unsafe { *c },
    };
    let dims = ctx.dimensions.read().unwrap();
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
    #[rustfmt::skip]
    let texture: Matrix3<f32> = Matrix3::new(
        tw as f32, 0.0,       tx as f32,
        0.0,      th as f32, ty as f32,
        0.0,       0.0,       1.0,
    );
    let data = TextureUniform {
        texture,
        transform,
        colour,
    };

    let tex = unsafe { &*ctex };
    let _ = tex.draw_ex(ctx, &data);
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderSDF(
    ctex: *mut Texture,
    x: c_double,
    y: c_double,
    w: c_double,
    h: c_double,
    c: *mut Vector4<f32>,
    angle: c_double,
    outline: c_double,
) {
    let ctx = Context::get();
    let colour = match c.is_null() {
        true => Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        false => unsafe { *c },
    };
    let dims = ctx.dimensions.read().unwrap();
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
    let data = TextureUniform {
        transform,
        colour,
        ..Default::default()
    };
    let tex = unsafe { &*ctex };
    let sdf = TextureSDFUniform {
        m: 2.0 * tex.texture.vmax * (w as f32 + 2.) / tex.texture.w as f32,
        outline: outline as f32,
    };
    let _ = tex.draw_sdf_ex(ctx, &data, &sdf);
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderScaleAspectMagic(
    ctex: *mut Texture,
    bx: c_double,
    by: c_double,
    bw: c_double,
    bh: c_double,
) {
    if ctex.is_null() {
        return;
    }
    let ctx = Context::get();
    let tex = unsafe { &*ctex };
    let tw = tex.texture.w as f32;
    let th = tex.texture.h as f32;
    let w = bw as f32;
    let h = bh as f32;
    let scale = (w / tw).min(h / th);
    let nw = scale * tw;
    let nh = scale * th;
    let x = bx as f32 + (w - nw) * 0.5;
    let y = by as f32 + (h - nh) * 0.5;

    // TODO Disable for now, reactivate in a better way with Texture::scale caching into framebuffers in
    // the widgets
    let _ = tex.draw(ctx, x, y, nw, nh);
    //let _ = tex.draw_scale(ctx, x, y, nw, nh, scale);
}
