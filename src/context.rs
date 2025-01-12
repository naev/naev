#![allow(dead_code)]
use anyhow::Result;
use glow::*;
use sdl2 as sdl;
use sdl2::image::ImageRWops;
use std::ops::Deref;
use std::sync::{Arc, Mutex, MutexGuard, OnceLock};
use std::thread::ThreadId;

use crate::buffer::{
    Buffer, BufferBuilder, BufferUsage, VertexArray, VertexArrayBuffer, VertexArrayBuilder,
};
use crate::{log, ndata};

pub static CONTEXT: OnceLock<Context> = OnceLock::new();

pub struct Context {
    pub sdlvid: sdl::VideoSubsystem,
    pub gl: glow::Context,
    pub window: sdl::video::Window,
    pub gl_context: sdl::video::GLContext,
    main_thread: ThreadId,

    // Useful "globals"
    pub vbo_square: Buffer,
    pub vao_square: VertexArray,
}
unsafe impl Send for Context {}
unsafe impl Sync for Context {}

/// Wrapper for a Context MutexGuard
pub struct ContextWrap<'sc, 'ctx>(MutexGuard<'sc, &'ctx Context>);
impl<'sc, 'ctx> ContextWrap<'sc, 'ctx> {
    fn new(guard: MutexGuard<'sc, &'ctx Context>) -> Self {
        guard.window.gl_make_current(&guard.gl_context).unwrap();
        ContextWrap(guard)
    }
}
impl<'ctx> Deref for ContextWrap<'_, 'ctx> {
    type Target = &'ctx Context;
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl Drop for ContextWrap<'_, '_> {
    fn drop(&mut self) {
        self.0.sdlvid.gl_release_current_context().unwrap();
    }
}

/// Wrapper for thread safe OpenGL context
pub struct SafeContext<'ctx> {
    ctx: Arc<Mutex<&'ctx Context>>,
}
impl<'ctx> SafeContext<'ctx> {
    pub fn new(ctx: &'ctx Context) -> Self {
        ctx.sdlvid.gl_release_current_context().unwrap();
        SafeContext {
            ctx: Arc::new(Mutex::new(ctx)),
        }
    }
    pub fn lock(&self) -> ContextWrap<'_, 'ctx> {
        let guard = self.ctx.lock().unwrap();
        ContextWrap::new(guard)
    }
}
impl Drop for SafeContext<'_> {
    fn drop(&mut self) {
        let guard = self.ctx.lock().unwrap();
        guard.window.gl_make_current(&guard.gl_context).unwrap();
    }
}

impl Context {
    #[rustfmt::skip]
    const DATA_SQUARE: [f32;8] = [ 0., 0.,
                                   1., 0.,
                                   0., 1.,
                                   1., 1., ];

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

    pub fn new(sdlvid: sdl::VideoSubsystem) -> Result<&'static Self> {
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

        let (window, gl_context) = match Self::create_context(&sdlvid, &gl_attr, 4, 6) {
            Ok(v) => v,
            _ => match Self::create_context(&sdlvid, &gl_attr, 3, 3) {
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

        // Initialize some useful globals
        let vbo_square = BufferBuilder::new()
            .usage(BufferUsage::Static)
            .data_f32(&Self::DATA_SQUARE)
            .build(&gl)?;
        let vao_square = VertexArrayBuilder::new()
            .buffers(&[VertexArrayBuffer {
                buffer: &vbo_square,
                size: 2,
                stride: 0,
                offset: 0,
            }])
            .build(&gl)?;

        let ctx = Context {
            sdlvid,
            window,
            gl_context,
            gl,
            main_thread: std::thread::current().id(),
            vbo_square,
            vao_square,
        };
        let _ = CONTEXT.set(ctx);
        Ok(CONTEXT.get().unwrap())
    }

    fn is_main_thread(&self) -> bool {
        self.main_thread == std::thread::current().id()
    }
}
