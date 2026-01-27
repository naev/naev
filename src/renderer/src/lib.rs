use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::*;
use helpers::atomicfloat::AtomicF32;
use naev_core::start;
use nalgebra::{Matrix3, Matrix4, Point3, Vector2, Vector3, Vector4};
use physics::vec2::Vec2;
use sdl3 as sdl;
use std::ffi::CStr;
use std::ops::Deref;
use std::sync::{Arc, Mutex, MutexGuard, OnceLock, RwLock, atomic::AtomicBool, atomic::Ordering};

pub mod buffer;
pub mod camera;
pub mod colour;
pub mod luagfx;
pub mod sdf;
pub mod shader;
pub mod texture;

/// Some hardcoded paths to search for things
pub const GFX_PATH: &str = "gfx/";

use crate::buffer::{
    Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray, VertexArrayBuffer,
    VertexArrayBuilder,
};
use crate::shader::{ProgramBuilder, Shader};
use nlog::{debug, info, warn, warn_err};

const MIN_WIDTH: u32 = 1280;
const MIN_HEIGHT: u32 = 720;
const MIN_WIDTH_F32: f32 = MIN_WIDTH as f32;
const MIN_HEIGHT_F32: f32 = MIN_HEIGHT as f32;
pub(crate) static VIEW_WIDTH: AtomicF32 = AtomicF32::new(0.0);
pub(crate) static VIEW_HEIGHT: AtomicF32 = AtomicF32::new(0.0);
static DEBUG: AtomicBool = AtomicBool::new(false);

fn debug_callback(source: u32, msg_type: u32, id: u32, severity: u32, msg: &str) {
    let s_source = match source {
        glow::DEBUG_SOURCE_API => "api",
        glow::DEBUG_SOURCE_WINDOW_SYSTEM => "window_system",
        glow::DEBUG_SOURCE_SHADER_COMPILER => "shader_compiler",
        glow::DEBUG_SOURCE_THIRD_PARTY => "third_party",
        glow::DEBUG_SOURCE_APPLICATION => "application",
        glow::DEBUG_SOURCE_OTHER => "other",
        _ => &format!("{source}"),
    };
    let s_type = match msg_type {
        glow::DEBUG_TYPE_ERROR => "error",
        glow::DEBUG_TYPE_DEPRECATED_BEHAVIOR => "deprecated_behavior",
        glow::DEBUG_TYPE_UNDEFINED_BEHAVIOR => "undefined_behavior",
        glow::DEBUG_TYPE_PORTABILITY => "portability",
        glow::DEBUG_TYPE_PERFORMANCE => "performance",
        glow::DEBUG_TYPE_MARKER => "marker",
        glow::DEBUG_TYPE_PUSH_GROUP => "push_group",
        glow::DEBUG_TYPE_POP_GROUP => "pop_group",
        glow::DEBUG_TYPE_OTHER => "other",
        _ => &format!("{msg_type}"),
    };
    let s_id = format!("{id}");
    let s_severity = match severity {
        glow::DEBUG_SEVERITY_LOW => "low",
        glow::DEBUG_SEVERITY_MEDIUM => "medium",
        glow::DEBUG_SEVERITY_HIGH => "high",
        glow::DEBUG_SEVERITY_NOTIFICATION => "notification",
        _ => &format!("{severity}"),
    };

    if severity == glow::DEBUG_SEVERITY_LOW {
        debug!(
            "OpenGL debug( source={s_source}, type={s_type}, id={s_id}, severity={s_severity} ): {msg}"
        );
    } else {
        warn!(
            "OpenGL debug( source={s_source}, type={s_type}, id={s_id}, severity={s_severity} ): {msg}"
        );
    }
}

/// Implements buffer writing for Uniforms
pub trait Uniform {
    fn buffer(&self) -> Result<Vec<u8>>;
}
impl<T: ShaderSize + encase::internal::WriteInto> Uniform for T {
    fn buffer(&self) -> Result<Vec<u8>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer.into_inner())
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct TextureUniform {
    pub texture: Matrix3<f32>,
    pub transform: Matrix3<f32>,
    pub colour: Vector4<f32>,
}
impl Default for TextureUniform {
    fn default() -> Self {
        Self {
            texture: Matrix3::identity(),
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        }
    }
}

#[repr(C)]
#[derive(Default, Debug, Copy, Clone, ShaderType)]
pub struct TextureSDFUniform {
    pub m: f32,
    pub outline: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct TextureScaleUniform {
    pub texture: Matrix3<f32>,
    pub transform: Matrix3<f32>,
    pub colour: Vector4<f32>,
    pub scale: f32,
    pub radius: f32,
}
impl Default for TextureScaleUniform {
    fn default() -> Self {
        Self {
            texture: Matrix3::identity(),
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
            scale: 1.0,
            radius: 4.0,
        }
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct SolidUniform {
    pub transform: Matrix3<f32>,
    pub colour: Vector4<f32>,
}
impl Default for SolidUniform {
    fn default() -> Self {
        Self {
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        }
    }
}

#[allow(clippy::enum_variant_names)] // Remove when we add other messages
#[derive(Clone)]
pub(crate) enum Message {
    DeleteBuffer(glow::NativeBuffer),
    DeleteVertexArray(glow::NativeVertexArray),
    DeleteProgram(glow::NativeProgram),
    DeleteTexture(glow::NativeTexture),
    DeleteSampler(glow::NativeSampler),
    DeleteFramebuffer(glow::NativeFramebuffer),
}
impl Message {
    fn execute(self, ctx: &Context) {
        match self {
            Self::DeleteBuffer(buf) => unsafe {
                ctx.gl.delete_buffer(buf);
            },
            Self::DeleteVertexArray(vao) => unsafe {
                ctx.gl.delete_vertex_array(vao);
            },
            Self::DeleteProgram(pgm) => unsafe {
                ctx.gl.delete_program(pgm);
            },
            Self::DeleteTexture(tex) => unsafe {
                ctx.gl.delete_texture(tex);
                // Some simple garbage collection for when there are too many dead references
                if texture::GC_COUNTER.fetch_add(1, Ordering::SeqCst) > texture::GC_THRESHOLD {
                    let data = &mut texture::TEXTURE_DATA.lock().unwrap();
                    data.retain(|x| x.strong_count() > 0);
                    texture::GC_COUNTER.store(0, Ordering::Relaxed);
                }
            },
            Self::DeleteSampler(smp) => unsafe {
                ctx.gl.delete_sampler(smp);
            },
            Self::DeleteFramebuffer(buf) => unsafe {
                ctx.gl.delete_framebuffer(buf);
            },
        }
    }
}

static CONTEXT: OnceLock<Context> = OnceLock::new();
static MESSAGE_QUEUE: Mutex<Vec<Message>> = Mutex::new(vec![]);
pub(crate) fn message_push(msg: Message) {
    MESSAGE_QUEUE.lock().unwrap().push(msg);
}

#[derive(Clone, Debug)]
pub struct Dimensions {
    pub pixels_width: u32,        // In real pixels
    pub pixels_height: u32,       // In real pixels
    pub window_width: u32,        // In window units
    pub window_height: u32,       // In window units
    pub view_width: f32,          // In viewport scaled pixels
    pub view_height: f32,         // In viewport scaled pixels
    pub view_scale: f32,          // Viewport scaling value
    pub projection: Matrix3<f32>, // Projection matrix for drawing
}

#[rustfmt::skip]
pub const fn ortho3(left: f32, right: f32, bottom: f32, top: f32) -> Matrix3<f32> {
    Matrix3::new(
        2.0/(right-left), 0.0,              -(right+left)/(right-left),
        0.0,              2.0/(top-bottom), -(top+bottom)/(top-bottom),
        0.0,              0.0,              1.0,
    )
}

#[rustfmt::skip]
pub const fn ortho4(left: f32, right: f32, bottom: f32, top: f32, near: f32, far: f32) -> Matrix4<f32> {
    Matrix4::new(
        2.0/(right-left),   0.0,  0.0, -(right+left)/(right-left),
        0.0,    2.0/(top-bottom), 0.0, -(top+bottom)/(top-bottom),
        0.0, 0.0, -2.0 / (far - near), -(far+near)/(far-near),
        0.0,        0.0,          0.0, 1.0,
    )
}

pub fn look_at4(eye: &Vector3<f32>, target: &Vector3<f32>, up: &Vector3<f32>) -> Matrix4<f32> {
    nalgebra::Isometry3::look_at_rh(&Point3::from(*eye), &Point3::from(*target), up)
        .to_homogeneous()
}

impl Dimensions {
    fn new(window: &sdl::video::Window) -> Self {
        let (pixels_width, pixels_height) = window.size_in_pixels();
        let (window_width, window_height) = window.size();
        let (dwscale, dhscale) = (
            (window_width as f32) / (pixels_width as f32),
            (window_height as f32) / (pixels_height as f32),
        );
        let scalefactor = unsafe { naevc::conf.scalefactor as f32 };
        let scale = f32::max(dwscale, dhscale) / scalefactor;
        let (view_width, view_height, view_scale) = {
            let (vw, vh) = (
                (pixels_width as f32) * scale,
                (pixels_height as f32) * scale,
            );
            if vw < MIN_WIDTH_F32 || vh < MIN_HEIGHT_F32 {
                info!("Screen size is too small, upscaling...");
                let scalew = MIN_WIDTH_F32 / vw;
                let scaleh = MIN_HEIGHT_F32 / vh;
                let scale = scale * f32::max(scalew, scaleh);
                (
                    (window_width as f32) * scale,
                    (window_height as f32) * scale,
                    scale,
                )
            } else {
                (vw, vh, scale)
            }
        };
        let projection = ortho3(0.0, view_width, 0.0, view_height);

        // This function is private, so we can update here.
        VIEW_WIDTH.store(view_width, Ordering::Relaxed);
        VIEW_HEIGHT.store(view_height, Ordering::Relaxed);

        // TODO remove the C stuff
        unsafe {
            naevc::gl_screen.rw = pixels_width as i32;
            naevc::gl_screen.rh = pixels_height as i32;
            naevc::gl_screen.dwscale = dwscale as f64;
            naevc::gl_screen.dhscale = dhscale as f64;
            naevc::gl_screen.scale = view_scale as f64;
            naevc::gl_screen.nw = view_width.round() as i32;
            naevc::gl_screen.nh = view_height.round() as i32;
            naevc::gl_screen.w = naevc::gl_screen.nw;
            naevc::gl_screen.h = naevc::gl_screen.nh;
            naevc::gl_screen.wscale = 1.0;
            naevc::gl_screen.hscale = 1.0;
            naevc::gl_screen.mxscale = naevc::gl_screen.w as f64 / naevc::gl_screen.rw as f64;
            naevc::gl_screen.myscale = naevc::gl_screen.h as f64 / naevc::gl_screen.rh as f64;
        }

        Dimensions {
            window_width,
            window_height,
            pixels_width,
            pixels_height,
            view_width,
            view_height,
            view_scale,
            projection,
        }
    }
}

pub struct Context {
    pub sdlvid: sdl::VideoSubsystem,
    pub gl: glow::Context,
    pub window: Mutex<sdl::video::Window>,
    pub gl_context: sdl::video::GLContext,
    // We should be able to get rid of this mutex when fully moved to Rust
    pub dimensions: RwLock<Dimensions>,

    // Useful "globals"
    pub program_texture: Shader,
    pub buffer_texture: Buffer,
    pub program_texture_sdf: Shader,
    pub buffer_texture_sdf: Buffer,
    pub program_texture_scale: Shader,
    pub buffer_texture_scale: Buffer,
    pub program_solid: Shader,
    pub buffer_solid: Buffer,
    pub vbo_square: Buffer,
    pub vao_square: VertexArray,
    pub vbo_center: Buffer,
    pub vao_center: VertexArray,
    pub vbo_triangle: Buffer,
    pub vao_triangle: VertexArray,

    // Some subsystems
    pub sdf: sdf::SdfRenderer,

    // To be phased out when moved to rust
    pub vao_core: glow::VertexArray,
}
// The issue is the SDL structs use raw pointers. If they wrapped in AtomicPtr, it would be
// Send+Sync safe, but it is not.
// See https://github.com/vhspace/sdl3-rs/issues/196
unsafe impl Sync for Context {}
unsafe impl Send for Context {}

/// Wrapper for a Context MutexGuard
pub struct ContextGuard<'sc, 'ctx>(MutexGuard<'sc, &'ctx Context>);
impl<'sc, 'ctx> ContextGuard<'sc, 'ctx> {
    fn new(guard: MutexGuard<'sc, &'ctx Context>) -> Self {
        guard
            .window
            .lock()
            .unwrap()
            .gl_make_current(&guard.gl_context)
            .unwrap();
        ContextGuard(guard)
    }
}
impl<'ctx> Deref for ContextGuard<'_, 'ctx> {
    type Target = &'ctx Context;
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl Drop for ContextGuard<'_, '_> {
    fn drop(&mut self) {
        self.0.sdlvid.gl_release_current_context().unwrap();
    }
}

/// Wrapper for thread safe OpenGL context
#[derive(Clone)]
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
    pub fn lock(&self) -> ContextGuard<'_, 'ctx> {
        let guard = self.ctx.lock().unwrap();
        ContextGuard::new(guard)
    }
    pub fn into_wrap(self) -> ContextWrapper<'ctx> {
        ContextWrapper::Safe(self)
    }
}
impl Drop for SafeContext<'_> {
    fn drop(&mut self) {
        let guard = self.ctx.lock().unwrap();
        guard
            .window
            .lock()
            .unwrap()
            .gl_make_current(&guard.gl_context)
            .unwrap();
    }
}

pub enum ContextWrapperGuard<'sc, 'ctx> {
    Context(&'ctx Context),
    Safe(ContextGuard<'sc, 'ctx>),
}
impl<'ctx> Deref for ContextWrapperGuard<'_, 'ctx> {
    type Target = &'ctx Context;
    fn deref(&self) -> &Self::Target {
        match self {
            Self::Context(ctx) => ctx,
            Self::Safe(sctx) => sctx,
        }
    }
}
#[derive(Clone)]
pub enum ContextWrapper<'ctx> {
    Context(&'ctx Context),
    Safe(SafeContext<'ctx>),
}
impl<'ctx> ContextWrapper<'ctx> {
    pub fn lock(&self) -> ContextWrapperGuard<'_, 'ctx> {
        match self {
            Self::Context(ctx) => ContextWrapperGuard::Context(ctx),
            Self::Safe(sctx) => ContextWrapperGuard::Safe(sctx.lock()),
        }
    }
}
impl<'ctx> From<&'ctx Context> for ContextWrapper<'ctx> {
    fn from(ctx: &'ctx Context) -> Self {
        Self::Context(ctx)
    }
}
impl<'ctx> From<SafeContext<'ctx>> for ContextWrapper<'ctx> {
    fn from(ctx: SafeContext<'ctx>) -> Self {
        Self::Safe(ctx)
    }
}

impl Context {
    #[rustfmt::skip]
    const DATA_SQUARE: [f32;8] = [ 0., 0.,
                                   1., 0.,
                                   0., 1.,
                                   1., 1., ];
    #[rustfmt::skip]
    const DATA_CENTER: [f32;8] = [-1.,-1.,
                                   1.,-1.,
                                  -1., 1.,
                                   1., 1., ];

    // cos/sin are not constant or we would want
    //vertex[0]      = 0.5 * cos( 4. * M_PI / 3. );
    //vertex[1]      = 0.5 * sin( 4. * M_PI / 3. );
    //vertex[2]      = 0.5 * cos( 0. );
    //vertex[3]      = 0.5 * sin( 0. );
    //vertex[4]      = 0.5 * cos( 2. * M_PI / 3. );
    //vertex[5]      = 0.5 * sin( 2. * M_PI / 3. );
    //vertex[6]      = vertex[0];
    //vertex[7]      = vertex[1];
    #[rustfmt::skip]
    const DATA_TRIANGLE: [f32;8] = [
        -0.25, -0.433_012_7,
         0.5,   0.0,
        -0.25,  0.433_012_7,
        -0.25, -0.433_012_7];

    pub fn get() -> &'static Self {
        CONTEXT.get().expect("No context!")
    }

    pub fn as_safe_wrap(&self) -> ContextWrapper<'_> {
        self.as_safe().into_wrap()
    }

    pub fn as_safe(&self) -> SafeContext<'_> {
        SafeContext::new(self)
    }

    pub fn as_wrap(&self) -> ContextWrapper<'_> {
        ContextWrapper::Context(self)
    }

    fn create_context(
        sdlvid: &sdl::VideoSubsystem,
        gl_attr: &sdl::video::gl_attr::GLAttr,
        major: u8,
        minor: u8,
    ) -> Result<(sdl::video::Window, sdl::video::GLContext)> {
        let (width, height, resizable, fullscreen) = unsafe {
            (
                naevc::conf.width,
                naevc::conf.height,
                naevc::conf.notresizable == 0,
                naevc::conf.fullscreen != 0,
            )
        };
        gl_attr.set_context_version(major, minor);
        //gl_attr.set_share_with_current_context(true);

        let mut wdwbuild = sdlvid.window(
            format!(
                "{} - {}",
                naev_core::APPNAME,
                start::start().name.clone().into_string()?
            )
            .as_str(),
            width.max(MIN_WIDTH),
            height.max(MIN_HEIGHT),
        );
        // Issue documented below
        if resizable {
            wdwbuild.resizable();
        }
        if fullscreen {
            wdwbuild.fullscreen();
        }
        let mut window = wdwbuild
            .opengl()
            .position_centered()
            .high_pixel_density()
            .build()?;

        // It seems like setting the build time flag can give smaller windows than we want, at
        // least on Wayland. Workaround is to set RESIZABLE flag after creation.
        // https://github.com/libsdl-org/SDL/issues/13344
        //if resizable {
        //    window.set_resizable(true);
        //}

        window
            .set_minimum_size(MIN_WIDTH, MIN_HEIGHT)
            .unwrap_or_else(|err| {
                warn_err!(anyhow::Error::new(err).context("unable to set minimum window size."))
            });
        let gl_context = match window.gl_create_context() {
            Ok(ctx) => ctx,
            Err(e) => anyhow::bail!("Unable to create OpenGL context: {}", e),
        };

        // Try to load the icon.
        fn set_icon(window: &mut sdl::video::Window) -> Result<()> {
            let filename = 'filename: {
                for imageformat in texture::FORMATS {
                    for ext in imageformat.extensions_str() {
                        let path = format!("{}{}.{}", GFX_PATH, "icon", ext);
                        if ndata::exists(&path) {
                            break 'filename path;
                        }
                    }
                }
                anyhow::bail!("icon not found")
            };
            let rw = ndata::iostream(&filename)?;
            let img = {
                let mut img = image::ImageReader::new(std::io::BufReader::new(rw))
                    .with_guessed_format()?
                    .decode()?
                    .into_rgba8();
                // See crate::texture::TextureData::from_image()
                for p in img.pixels_mut() {
                    if p.0[3] == 0 {
                        p.0 = [0u8; 4];
                    }
                }
                img
            };
            let mut data = img.into_flat_samples();
            let sur = sdl::surface::Surface::from_data(
                &mut data.samples,
                data.layout.width,
                data.layout.height,
                data.layout.height_stride as u32,
                sdl::pixels::PixelFormat::RGBA32,
            )?;
            window.set_icon(sur);
            Ok(())
        }
        if let Err(e) = set_icon(&mut window) {
            warn!("Unable to set window icon: {e}");
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

        // Focus behaviour.
        sdl::hint::set_video_minimize_on_focus_loss(minimize);

        // Set up the attributes.
        let gl_attr = sdlvid.gl_attr();
        gl_attr.set_context_profile(sdl::video::GLProfile::Core);
        gl_attr.set_double_buffer(true);
        if fsaa > 1 {
            gl_attr.set_multisample_buffers(1);
            gl_attr.set_multisample_samples(fsaa);
        }
        gl_attr.set_framebuffer_srgb_compatible(true);
        gl_attr.set_context_flags().forward_compatible().set();
        #[cfg(debug_assertions)]
        gl_attr.set_context_flags().debug().set();

        // Try older versions in order
        let gl_backup = || -> Result<(sdl::video::Window, sdl::video::GLContext)> {
            for (major, minor) in [(4, 5), (4, 4), (4, 3), (4, 2), (4, 1), (4, 0), (3, 3)] {
                match Self::create_context(&sdlvid, &gl_attr, major, minor) {
                    Ok(v) => {
                        warn!("Falling back to OpenGL {major}.{minor} context!");
                        return Ok(v);
                    }
                    _ => continue,
                }
            }
            anyhow::bail!("Failed to create OpenGL context!")
        };
        let (window, gl_context) = {
            match Self::create_context(&sdlvid, &gl_attr, 4, 6) {
                Ok(v) => v,
                _ => gl_backup()?,
            }
        };
        let mut gl = unsafe {
            glow::Context::from_loader_function(|s| match sdlvid.gl_get_proc_address(s) {
                Some(f) => f as *const _,
                None => std::ptr::null(),
            })
        };
        DEBUG.store(gl.supports_debug(), Ordering::Relaxed);

        // Final touches
        sdlvid
            .gl_set_swap_interval(match vsync {
                true => 1,
                false => 0,
            })
            .unwrap_or_else(|err| {
                warn_err!(anyhow::Error::msg(err).context("unable to set OpenGL swap interval"))
            });

        match gl_attr.framebuffer_srgb_compatible() {
            true => (),
            false => warn("unable to set framebuffer to SRGB!"),
        };

        #[cfg(debug_assertions)]
        if gl.supports_debug() && unsafe { naevc::conf.devmode != 0 } {
            match gl_attr.context_flags().has_debug() {
                true => unsafe {
                    gl.enable(glow::DEBUG_OUTPUT);
                    gl.debug_message_callback(debug_callback);
                    // Hide stuff that is useful for RenderDoc
                    for msg_type in [
                        glow::DEBUG_TYPE_PUSH_GROUP,
                        glow::DEBUG_TYPE_POP_GROUP,
                        glow::DEBUG_TYPE_MARKER,
                    ] {
                        gl.debug_message_control(
                            glow::DEBUG_SOURCE_APPLICATION,
                            msg_type,
                            glow::DONT_CARE,
                            &[],
                            false,
                        );
                    }
                    // Notifications about putting stuff in GPU memory, which we want
                    gl.debug_message_control(
                        glow::DONT_CARE,
                        glow::DONT_CARE,
                        glow::DEBUG_SEVERITY_NOTIFICATION,
                        &[],
                        false,
                    );
                    // NVIDIA warnings about recompiling shaders, don't think we can do much here
                    gl.debug_message_control(
                        glow::DEBUG_SOURCE_API,
                        glow::DEBUG_TYPE_PERFORMANCE,
                        glow::DONT_CARE,
                        // 131218 -> recompiling shaders
                        // 131186 -> Buffer performance warning: ... being copied/moved from VIDEO memory to HOST memory.
                        &[131218, 131186],
                        false,
                    );
                },
                false => warn!("unable to set OpenGL debug mode!"),
            };
        }

        unsafe {
            naevc::gl_screen.window = window.raw() as *mut naevc::SDL_Window;
            naevc::gl_screen.context = gl_context.raw() as *mut naevc::SDL_GLContextState;
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

        // Modern OpenGL requires at least one VAO and the C code uses the same one
        let vao_core = unsafe {
            let vao = gl.create_vertex_array().map_err(|e| anyhow::anyhow!(e))?;
            gl.bind_vertex_array(Some(vao));
            if gl.supports_debug() {
                gl.object_label(
                    glow::VERTEX_ARRAY,
                    vao.0.into(),
                    Some("C Core Vertex Array"),
                );
            }
            vao
        };

        // Initialize some useful globals
        // The texture shader
        let program_texture = ProgramBuilder::new(Some("Texture Shader"))
            .uniform_buffer("texturedata", 0)
            .wgsl_file("texture.wgsl")
            .sampler("texsampler", 0)
            .build(&gl)?;
        let buffer_texture = BufferBuilder::new(Some("Texture Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&TextureUniform::default().buffer()?)
            .build(&gl)?;
        // SDF texture shader
        let program_texture_sdf = ProgramBuilder::new(Some("SDF Texture Shader"))
            .uniform_buffer("texturedata", 0)
            .uniform_buffer("sdfdata", 1)
            .wgsl_file("texture_sdf.wgsl")
            .sampler("texsampler", 0)
            .build(&gl)?;
        let buffer_texture_sdf = BufferBuilder::new(Some("SDF Texture Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&TextureSDFUniform::default().buffer()?)
            .build(&gl)?;
        // Downscaling texture shader
        let program_texture_scale = ProgramBuilder::new(Some("Scaling Texture Shader"))
            .uniform_buffer("TextureData", 0)
            .vert_frag_file_single("rust_magic.glsl")
            .sampler("sampler", 0)
            .build(&gl)?;
        let buffer_texture_scale = BufferBuilder::new(Some("Scaling Texture Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&TextureScaleUniform::default().buffer()?)
            .build(&gl)?;
        // The solid shader
        let program_solid = ProgramBuilder::new(Some("Solid Shader"))
            .uniform_buffer("soliddata", 0)
            .wgsl_file("solid.wgsl")
            .build(&gl)?;
        let buffer_solid = BufferBuilder::new(Some("Solid Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&SolidUniform::default().buffer()?)
            .build(&gl)?;

        // Square VBO
        let vbo_square = BufferBuilder::new(Some("Square VBO"))
            .usage(BufferUsage::Static)
            .data_f32(&Self::DATA_SQUARE)
            .build(&gl)?;
        let vao_square = VertexArrayBuilder::new(Some("Square Vertex Array"))
            .buffers(&[VertexArrayBuffer {
                buffer: &vbo_square,
                size: 2,
                stride: 0, // tightly packed
                offset: 0,
                divisor: 0,
            }])
            .build_gl(&gl)?;

        // Center VBO
        let vbo_center = BufferBuilder::new(Some("Center VBO"))
            .usage(BufferUsage::Static)
            .data_f32(&Self::DATA_CENTER)
            .build(&gl)?;
        let vao_center = VertexArrayBuilder::new(Some("Center Vertex Array"))
            .buffers(&[VertexArrayBuffer {
                buffer: &vbo_center,
                size: 2,
                stride: 0, // tightly packed
                offset: 0,
                divisor: 0,
            }])
            .build_gl(&gl)?;

        // Triangle VBO
        let vbo_triangle = BufferBuilder::new(Some("Triangle VBO"))
            .usage(BufferUsage::Static)
            .data_f32(&Self::DATA_TRIANGLE)
            .build(&gl)?;
        let vao_triangle = VertexArrayBuilder::new(Some("Triangle Vertex Array"))
            .buffers(&[VertexArrayBuffer {
                buffer: &vbo_triangle,
                size: 2,
                stride: 0, // tightly packed
                offset: 0,
                divisor: 0,
            }])
            .build_gl(&gl)?;

        // Load up initial dimensions
        let dimensions = Dimensions::new(&window);

        // Set up the OpenGL state
        unsafe {
            gl.enable(glow::FRAMEBUFFER_SRGB);
            gl.bind_vertex_array(Some(vao_core)); // Set default C VAO
            gl.disable(glow::DEPTH_TEST);
            gl.enable(glow::BLEND);
            // We use SDF shaders for most shapes, but star trails & map routes are thin & anti-aliased.
            gl.enable(glow::LINE_SMOOTH);
            gl.blend_equation(glow::FUNC_ADD);
            gl.blend_func_separate(
                glow::SRC_ALPHA,
                glow::ONE_MINUS_SRC_ALPHA,
                glow::ONE,
                glow::ONE_MINUS_SRC_ALPHA,
            );
            gl.clear_color(0.0, 0.0, 0.0, 0.0);
            gl.clear(glow::COLOR_BUFFER_BIT | glow::DEPTH_BUFFER_BIT);
            // image-rs uses tight packing
            gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 1);
            gl.pixel_store_i32(glow::PACK_ALIGNMENT, 1);

            gl.viewport(
                0,
                0,
                dimensions.pixels_width as i32,
                dimensions.pixels_height as i32,
            );
        }
        let sdf = sdf::SdfRenderer::new(&gl)?;
        let ctx = Context {
            sdlvid,
            window: Mutex::new(window),
            gl_context,
            gl,
            dimensions: RwLock::new(dimensions),
            program_texture,
            buffer_texture,
            program_texture_sdf,
            buffer_texture_sdf,
            program_texture_scale,
            buffer_texture_scale,
            program_solid,
            buffer_solid,
            vbo_square,
            vao_square,
            vbo_center,
            vao_center,
            vbo_triangle,
            vao_triangle,
            sdf,
            vao_core,
        };
        let _ = CONTEXT.set(ctx);
        Ok(CONTEXT.get().unwrap())
    }

    pub fn resize(&self) -> Result<()> {
        let dims = {
            let wdw = self.window.lock().unwrap();
            Dimensions::new(&wdw)
        };
        let gl = &self.gl;
        let (vw, vh) = (dims.pixels_width as i32, dims.pixels_height as i32);
        unsafe {
            gl.viewport(0, 0, vw, vh);
            naevc::gl_view_matrix = naevc::mat4_ortho(
                0.0,
                dims.view_width.into(),
                0.0,
                dims.view_height.into(),
                -1.0,
                1.0,
            );
            naevc::conf.width = dims.window_width;
            naevc::conf.height = dims.window_height;
        }
        *self.dimensions.write().unwrap() = dims;
        Ok(())
    }

    pub fn execute_messages(&self) {
        let mut queue = MESSAGE_QUEUE.lock().unwrap();
        for msg in queue.drain(..) {
            msg.execute(self);
        }
    }

    pub fn draw_rect(&self, x: f32, y: f32, w: f32, h: f32, colour: Vector4<f32>) -> Result<()> {
        let dims = self.dimensions.read().unwrap();
        #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             w,  0.0,  x,
            0.0,  h,   y,
            0.0, 0.0, 1.0,
        );
        let uniform = SolidUniform { transform, colour };
        self.draw_rect_ex(&uniform)
    }

    pub fn draw_rect_ex(&self, uniform: &SolidUniform) -> Result<()> {
        let gl = &self.gl;
        self.program_solid.use_program(gl);
        self.vao_square.bind(self);

        self.buffer_solid
            .bind_write_base(self, &uniform.buffer()?, 0)?;
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }
        VertexArray::unbind(self);
        self.buffer_solid.unbind(self);

        Ok(())
    }

    /// Takes a screenshot of the game
    pub fn screenshot(&self, filename: &str) -> Result<()> {
        let dims = self.dimensions.read().unwrap();
        let (w, h) = (dims.pixels_width, dims.pixels_height);
        let mut data: Vec<u8> = vec![0; (w * h * 3) as usize];
        let gl = &self.gl;
        unsafe {
            gl.read_pixels(
                0,
                0,
                w as i32,
                h as i32,
                glow::RGB,
                glow::UNSIGNED_BYTE,
                glow::PixelPackData::Slice(Some(&mut data)),
            );
        }
        let img = match image::RgbImage::from_vec(w, h, data) {
            Some(img) => image::DynamicImage::ImageRgb8(img).flipv(),
            None => anyhow::bail!("failed to create ImageBuffer"),
        };
        let mut writer = ndata::physfs::File::open(filename, ndata::physfs::Mode::Write)?;
        Ok(img.write_to(&mut writer, image::ImageFormat::Png)?)
    }

    /// Converts a point in game coordinates to screen coordinates
    pub fn game_to_screen_coords(&self, pos: Vector2<f64>) -> Vector2<f64> {
        camera::CAMERA.read().unwrap().game_to_screen_coords(pos)
    }

    /// Converts a point in game coordinates to screen coordinates
    pub fn game_to_screen_coords_yflip(&self, pos: Vector2<f64>) -> Vector2<f64> {
        camera::CAMERA
            .read()
            .unwrap()
            .game_to_screen_coords_yflip(pos)
    }

    /// Converts a point in screen coordinates to game coordinates
    pub fn screen_to_game_coords(&self, pos: Vector2<f64>) -> Vector2<f64> {
        camera::CAMERA.read().unwrap().screen_to_game_coords(pos)
    }

    /// Converts a point from game to screen coordinates and makes sure it is in range
    pub fn game_to_screen_coords_inrange(&self, pos: Vector2<f64>, r: f64) -> Option<Vector2<f64>> {
        let view_width = crate::VIEW_WIDTH.load(Ordering::Relaxed) as f64;
        let view_height = crate::VIEW_HEIGHT.load(Ordering::Relaxed) as f64;
        let screen = camera::CAMERA.read().unwrap().game_to_screen_coords(pos);
        if screen.x < -r || screen.y < -r || screen.x > view_width + r || screen.y > view_height + r
        {
            None
        } else {
            Some(screen)
        }
    }

    /// Converts a point from game to screen coordinates and makes sure it is in range
    pub fn game_to_screen_coords_inrange_yflip(
        &self,
        pos: Vector2<f64>,
        r: f64,
    ) -> Option<Vector2<f64>> {
        let view_width = crate::VIEW_WIDTH.load(Ordering::Relaxed) as f64;
        let view_height = crate::VIEW_HEIGHT.load(Ordering::Relaxed) as f64;
        let screen = camera::CAMERA
            .read()
            .unwrap()
            .game_to_screen_coords_yflip(pos);
        if screen.x < -r || screen.y < -r || screen.x > view_width + r || screen.y > view_height + r
        {
            None
        } else {
            Some(screen)
        }
    }
}

// C API
use std::os::raw::{c_char, c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderRect(
    x: c_double,
    y: c_double,
    w: c_double,
    h: c_double,
    c: *mut Vector4<f32>,
) {
    let ctx = Context::get();
    let colour = unsafe { *c };
    let _ = ctx.draw_rect(x as f32, y as f32, w as f32, h as f32, colour);
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_resize() {
    {
        let ctx = CONTEXT.get().unwrap();
        let _ = ctx.resize();
    }
    unsafe { naevc::gl_resize_c() };
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_supportsDebug() -> std::os::raw::c_int {
    match DEBUG.load(Ordering::Relaxed) {
        true => 1,
        false => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_defViewport() {
    unsafe {
        naevc::gl_view_matrix = naevc::mat4_ortho(
            0.0,
            VIEW_WIDTH.load(Ordering::Relaxed).into(),
            0.0,
            VIEW_HEIGHT.load(Ordering::Relaxed).into(),
            -1.0,
            1.0,
        );
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_screenshot(cpath: *mut c_char) {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = Context::get();
    match ctx.screenshot(path.to_str().unwrap()) {
        Ok(_) => (),
        Err(e) => {
            warn!("Failed to take a screenshot: {e}");
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_toggleFullscreen() {
    let ctx = Context::get();
    let mut wdw = ctx.window.lock().unwrap();
    let fullscreen = wdw.fullscreen_state() != sdl::video::FullscreenType::Off;
    match wdw.set_fullscreen(!fullscreen) {
        Ok(()) => unsafe {
            naevc::conf.fullscreen = !fullscreen as c_int;
        },
        Err(e) => {
            warn_err!(e);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_setFullscreen(enable: c_int) {
    let ctx = Context::get();
    let mut wdw = ctx.window.lock().unwrap();
    match wdw.set_fullscreen(enable != 0) {
        Ok(()) => unsafe {
            naevc::conf.fullscreen = enable as c_int;
        },
        Err(e) => {
            warn_err!(e);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_gameToScreenCoords(
    nx: *mut c_double,
    ny: *mut c_double,
    bx: c_double,
    by: c_double,
) {
    let p = Vector2::new(bx, by);
    let v = camera::CAMERA.read().unwrap().game_to_screen_coords(p);
    unsafe {
        *nx = v.x;
        *ny = v.y;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_screenToGameCoords(
    nx: *mut c_double,
    ny: *mut c_double,
    bx: c_int,
    by: c_int,
) {
    let p = Vector2::new(bx as c_double, by as c_double);
    let v = camera::CAMERA.read().unwrap().screen_to_game_coords(p);
    unsafe {
        *nx = v.x;
        *ny = v.y;
    }
}
