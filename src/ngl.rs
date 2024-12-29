use anyhow::Result;
use glow::*;
use nalgebra::Vector2;
use sdl2 as sdl;
use sdl2::image::ImageRWops;
use std::ffi::CStr;
use std::os::raw::{c_char, c_uint};
use std::sync::{Arc, LazyLock, Mutex, MutexGuard, OnceLock};
use std::thread::ThreadId;

use crate::{log, ndata};

pub static CONTEXT: OnceLock<Context> = OnceLock::new();

pub struct Context {
    pub gl: glow::Context,
    pub window: sdl::video::Window,
    pub gl_context: sdl::video::GLContext,
    main_thread: ThreadId,
}
unsafe impl Send for Context {}
unsafe impl Sync for Context {}

impl Context {
    fn new(
        window: sdl::video::Window,
        gl_context: sdl::video::GLContext,
        gl: glow::Context,
    ) -> Self {
        Context {
            gl,
            window,
            gl_context,
            main_thread: std::thread::current().id(),
        }
    }

    fn is_main_thread(&self) -> bool {
        self.main_thread == std::thread::current().id()
    }
}

pub struct SafeContext {
    ctx: Arc<Mutex<Context>>,
}

impl SafeContext {
    pub fn new(ctx: Context) -> Self {
        SafeContext {
            ctx: Arc::new(Mutex::new(ctx)),
        }
    }

    pub fn lock<'a>(&'a self) -> MutexGuard<'a, Context> {
        let guard = self.ctx.lock().unwrap();
        guard.window.gl_make_current(&guard.gl_context).unwrap();
        //guard.window.gl_set_context_to_current().unwrap();
        guard
    }
}

impl Drop for SafeContext {
    fn drop(&mut self) {
        let guard = self.ctx.lock().unwrap();
        guard.window.gl_make_current(&guard.gl_context).unwrap();
        //guard.window.gl_set_context_to_current().unwrap();
    }
}

//static CONTEXT: TextureContext =

fn create_context(
    sdlvid: &sdl::VideoSubsystem,
    gl_attr: &sdl::video::gl_attr::GLAttr,
    major: u8,
    minor: u8,
) -> Result<(sdl::video::Window, sdl::video::GLContext)> {
    let (width, height, resizable, borderless) = unsafe {
        (
            naevc::conf.width,
            naevc::conf.height,
            naevc::conf.notresizable == 0,
            naevc::conf.borderless != 0,
        )
    };
    gl_attr.set_context_version(major, minor);

    //() = crate::start::start().name.into_string()?.as_str();

    let mut wdwbuild = sdlvid.window(
        format!(
            "{} - {}",
            crate::APPNAME,
            crate::start::start().name.clone().into_string()?
        )
        .as_str(),
        width.max(naevc::RESOLUTION_W_MIN),
        height.max(naevc::RESOLUTION_H_MIN),
    );
    if resizable {
        wdwbuild.resizable();
    }
    if borderless {
        wdwbuild.borderless();
    }
    wdwbuild.opengl().position_centered().allow_highdpi();
    let mut window = wdwbuild.build()?;

    window
        .set_minimum_size(naevc::RESOLUTION_W_MIN, naevc::RESOLUTION_H_MIN)
        .unwrap_or_else(|_| log::warn("Unable to set minimum window size."));
    let gl_context = match window.gl_create_context() {
        Ok(ctx) => ctx,
        Err(e) => anyhow::bail!("Unable to create OpenGL context: {}", e),
    };

    // Try to load the icon.
    let filename = format!("{}{}", ndata::GFX_PATH, "icon.webp");
    match ndata::rwops(filename.as_str()) {
        Ok(rw) => match rw.load() {
            Ok(icon) => window.set_icon(icon),
            Err(e) => anyhow::bail!(e),
        },
        Err(e) => anyhow::bail!(e),
    }

    Ok((window, gl_context))
}

pub fn init(sdlvid: &sdl::VideoSubsystem) -> Result<&'static Context> {
    let (minimize, fsaa, vsync) = unsafe {
        (
            naevc::conf.minimize != 0,
            naevc::conf.fsaa,
            naevc::conf.vsync != 0,
        )
    };

    /* Focus behaviour. */
    sdl::hint::set_video_minimize_on_focus_loss(minimize);

    /* Set up the attributes. */
    let gl_attr = sdlvid.gl_attr();
    gl_attr.set_context_profile(sdl::video::GLProfile::Core);
    gl_attr.set_double_buffer(true);
    if fsaa > 1 {
        gl_attr.set_multisample_buffers(1);
        gl_attr.set_multisample_samples(fsaa);
    }
    gl_attr.set_framebuffer_srgb_compatible(true);
    gl_attr.set_context_flags().forward_compatible().set();
    // TODO reenable debug mode
    // gl_attr.set_context_flags().debug().set();

    let (window, gl_context) = match create_context(sdlvid, &gl_attr, 4, 6) {
        Ok(v) => v,
        _ => match create_context(sdlvid, &gl_attr, 3, 3) {
            Ok(v) => v,
            _ => anyhow::bail!("Foo"),
        },
    };
    let gl = unsafe {
        glow::Context::from_loader_function(|s| sdlvid.gl_get_proc_address(s) as *const _)
    };

    // Final touches
    sdlvid
        .gl_set_swap_interval(match vsync {
            true => 1,
            false => 0,
        })
        .unwrap_or_else(|_| log::warn("Unable to set OpenGL swap interval!"));

    match gl_attr.framebuffer_srgb_compatible() {
        true => (),
        false => log::warn("Unable to set framebuffer to SRGB!"),
    };

    unsafe {
        naevc::gl_screen.window = window.raw() as *mut naevc::SDL_Window;
        naevc::gl_screen.context = gl_context.raw();
        (naevc::gl_screen.major, naevc::gl_screen.minor) = gl_attr.context_version();
        let major: i32 = naevc::gl_screen.major.into();
        let minor: i32 = naevc::gl_screen.minor.into();
        naevc::gl_screen.glsl = if major * 100 + minor * 10 > 320 {
            100 * major + 10 * minor
        } else {
            150
        };
        naevc::gl_screen.depth = gl_attr.depth_size();
        naevc::gl_screen.r = gl_attr.red_size();
        naevc::gl_screen.g = gl_attr.green_size();
        naevc::gl_screen.b = gl_attr.blue_size();
        naevc::gl_screen.a = gl_attr.alpha_size();
        naevc::gl_screen.depth =
            naevc::gl_screen.r + naevc::gl_screen.g + naevc::gl_screen.b + naevc::gl_screen.a;
        naevc::gl_screen.fsaa = gl_attr.multisample_samples();
        naevc::gl_screen.tex_max = gl.get_parameter_i32(glow::MAX_TEXTURE_SIZE);
        naevc::gl_screen.multitex_max = gl.get_parameter_i32(glow::MAX_TEXTURE_IMAGE_UNITS);
    }

    let ctx = Context::new(window, gl_context, gl);
    let _ = CONTEXT.set(ctx);
    Ok(&CONTEXT.get().unwrap())
}

pub struct Texture {
    path: String,

    // Dimensions
    w: usize,
    h: usize,

    // Sprites
    sx: usize,
    sy: usize,
    sw: f64,
    sh: f64,
    srw: f64,
    srh: f64,

    // Data
    texture: glow::Texture,
    alpha: Option<Vec<u8>>,
    vmax: f64,

    // Properties
    is_sdf: bool,
}

static TEXTURES: LazyLock<Mutex<Vec<Arc<Texture>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));

impl Texture {
    const SDL_FORMAT: sdl::pixels::PixelFormatEnum = sdl::pixels::PixelFormatEnum::ABGR8888;

    fn new_tex(
        gl: &glow::Context,
        path: &str,
        is_srgb: bool,
        is_sdf: bool,
    ) -> Result<(glow::Texture, f64, usize, usize), String> {
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

        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(texture));
            if naevc::gl_screen.scale != 1. {
                gl.tex_parameter_i32(
                    glow::TEXTURE_2D,
                    glow::TEXTURE_MIN_FILTER,
                    glow::LINEAR as i32,
                );
                gl.tex_parameter_i32(
                    glow::TEXTURE_2D,
                    glow::TEXTURE_MAG_FILTER,
                    glow::LINEAR as i32,
                );
            }
            gl.tex_parameter_i32(glow::TEXTURE_2D, glow::TEXTURE_WRAP_S, glow::REPEAT as i32);
            gl.tex_parameter_i32(glow::TEXTURE_2D, glow::TEXTURE_WRAP_T, glow::REPEAT as i32);
        };

        let vmax = 1.;
        if is_sdf {
            todo!();
        } else {
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
            }
        }

        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, None);
        }
        Ok((texture, vmax, w as usize, h as usize))
    }

    pub fn from_path(gl: &glow::Context, path: &str, sx: usize, sy: usize) -> Result<Arc<Self>> {
        let mut textures = TEXTURES.lock().unwrap();
        for tex in textures.iter() {
            if tex.path == path && tex.sx == sx && tex.sy == sy {
                return Ok(tex.clone());
            }
        }

        let (texture, vmax, w, h) =
            Self::new_tex(gl, path, true, false).map_err(|e| anyhow::anyhow!(e))?;

        let sw = (w as f64) / (sx as f64);
        let sh = (h as f64) / (sy as f64);

        let tex = Arc::new(Texture {
            path: String::from(path),
            w,
            h,
            sx,
            sy,
            sw,
            sh,
            srw: sw / (w as f64),
            srh: sh / (h as f64),
            texture,
            alpha: None,
            vmax,
            is_sdf: false,
        });

        textures.push(tex.clone());
        Ok(tex)
    }
}

#[no_mangle]
pub extern "C" fn gl_newImage_(cpath: *const c_char, flags: c_uint) -> *const naevc::glTexture {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = CONTEXT.get().unwrap();
    match Texture::from_path(&ctx.gl, path.to_str().unwrap(), 1, 1) {
        Ok(tex) => {
            let ptr = Arc::into_raw(tex) as *const naevc::glTexture;
            //Arc::increment_strong_count_in(ptr, System);
            ptr
        }
        _ => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn tex_tex_(ctex: *const naevc::glTexture) -> naevc::GLuint {
    let tex = unsafe { Arc::from_raw(ctex as *const Texture) };
    tex.texture.0.into()
}
