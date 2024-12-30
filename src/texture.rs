#![allow(dead_code)]
use anyhow::Result;
use glow::*;
use nalgebra::Vector4;
use sdl2 as sdl;
use sdl2::image::ImageRWops;
use std::boxed::Box;
use std::os::raw::{c_char, c_uint};
use std::sync::{Arc, LazyLock, Mutex, MutexGuard, OnceLock, Weak};
use std::thread::ThreadId;

use crate::ngl::CONTEXT;
use crate::{log, ndata};

pub struct TextureData {
    path: String,
    texture: glow::Texture,
    w: usize,
    h: usize,
    is_srgb: bool,
    is_sdf: bool,
    vmax: f64, // For SDF
}

static TEXTURE_DATA: LazyLock<Mutex<Vec<Weak<TextureData>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));

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

        if is_srgb {
            todo!();
        }

        if is_sdf {
            todo!();
        }

        unsafe {
            let internalformat = match is_srgb {
                true => match has_alpha {
                    true => glow::SRGB_ALPHA,
                    false => glow::SRGB,
                },
                false => match has_alpha {
                    true => glow::RGB,
                    false => glow::RGBA,
                },
            };
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
            match tex.upgrade() {
                Some(t) => {
                    if t.path == path {
                        return Ok(t);
                    }
                }
                None => (),
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
    path: String,

    // Sprites
    sx: usize,
    sy: usize,
    sw: f64,
    sh: f64,
    srw: f64,
    srh: f64,

    // Data
    texture: Arc<TextureData>,
    sampler: glow::Sampler,
}

impl Texture {}

pub enum AddressMode {}

pub struct TextureBuilder {
    path: Option<String>,
    w: usize,
    h: usize,
    sx: usize,
    sy: usize,
    is_srgb: bool,
    is_sdf: bool,
    border_value: Option<Vector4<f32>>,
}

impl TextureBuilder {
    pub fn new(path: Option<&str>) -> Self {
        TextureBuilder {
            path: match path {
                Some(p) => Some(String::from(p)),
                None => None,
            },
            w: 0,
            h: 0,
            sx: 1,
            sy: 1,
            is_srgb: true,
            is_sdf: false,
            border_value: None,
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

    pub fn border(mut self, border_value: Option<Vector4<f32>>) -> Self {
        self.border_value = border_value;
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
                gl.sampler_parameter_i32(sampler, glow::TEXTURE_MIN_FILTER, glow::LINEAR as i32);
                gl.sampler_parameter_i32(sampler, glow::TEXTURE_MAG_FILTER, glow::LINEAR as i32);
                if let Some(border) = &self.border_value {
                    gl.sampler_parameter_f32_slice(
                        sampler,
                        glow::TEXTURE_BORDER_COLOR,
                        border.as_slice(),
                    );
                    gl.sampler_parameter_i32(
                        sampler,
                        glow::TEXTURE_WRAP_S,
                        glow::CLAMP_TO_BORDER as i32,
                    );
                    gl.sampler_parameter_i32(
                        sampler,
                        glow::TEXTURE_WRAP_T,
                        glow::CLAMP_TO_BORDER as i32,
                    );
                } else {
                    gl.sampler_parameter_i32(sampler, glow::TEXTURE_WRAP_S, glow::REPEAT as i32);
                    gl.sampler_parameter_i32(sampler, glow::TEXTURE_WRAP_T, glow::REPEAT as i32);
                }
            }

            let (w, h) = (texture.w, texture.h);
            let (sx, sy) = (self.sx, self.sy);
            let sw = (w as f64) / (sx as f64);
            let sh = (h as f64) / (sy as f64);
            let srw = sw / (w as f64);
            let srh = sh / (h as f64);

            Ok(Texture {
                path: self.path.unwrap(),
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

#[no_mangle]
pub extern "C" fn gl_newImage_(cpath: *const c_char, flags: c_uint) -> *mut naevc::glTexture {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = CONTEXT.get().unwrap();
    match TextureBuilder::new(Some(path.to_str().unwrap())).build(&ctx.gl) {
        Ok(tex) => {
            unsafe { Arc::increment_strong_count(Arc::into_raw(tex.texture.clone())) }
            Box::into_raw(Box::new(tex)) as *mut naevc::glTexture
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn tex_tex_(ctex: *mut naevc::glTexture) -> naevc::GLuint {
    let tex = unsafe { Box::from_raw(ctex as *mut Texture) };
    tex.texture.texture.0.into()
}

#[no_mangle]
pub extern "C" fn tex_sampler(ctex: *mut naevc::glTexture) -> naevc::GLuint {
    let tex = unsafe { Box::from_raw(ctex as *mut Texture) };
    tex.sampler.0.into()
}
