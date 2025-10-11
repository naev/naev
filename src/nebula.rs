use anyhow::Result;
use encase::ShaderType;
use glow::*;
use nalgebra::{Vector2, Vector3};
use palette::FromColor;
use palette::Hsv;
use palette::rgb::Srgb;
use std::os::raw::c_double;

use crate::rng;
use renderer::Uniform;
use renderer::buffer::{
    Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray, VertexArrayBuffer,
    VertexArrayBuilder,
};
use renderer::shader::{ProgramBuilder, Shader};
use renderer::texture::{Framebuffer, FramebufferBuilder};

#[allow(dead_code)]
pub const DEFAULT_HUE: f64 = 260.0;
const PUFF_BUFFER: f32 = 300.;

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
struct NebulaUniform {
    hue: f32,
    horizon: f32,
    eddy_scale: f32,
    elapsed: f32,
    nonuninformity: f32,
    volatility: f32,
    saturation: f32,
    camera: Vector2<f32>,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Default, bytemuck::Pod, bytemuck::Zeroable)]
struct Puff {
    data: [f32; 4],
    rand: [f32; 2], // Randomness
}
impl Puff {
    fn new(ctx: &renderer::Context, fg: bool) -> Self {
        let (x, y) = {
            let dims = ctx.dimensions.read().unwrap();
            let x = (dims.view_width + PUFF_BUFFER) * (rng::rng::<f32>() * 2.0 - 1.0);
            let y = (dims.view_height + PUFF_BUFFER) * (rng::rng::<f32>() * 2.0 - 1.0);
            (x, y)
        };
        let rx = rng::rng::<f32>() * 2000.0 - 1000.0;
        let ry = rng::rng::<f32>() * 2000.0 - 1000.0;
        let height = {
            let rnd = (0.2 * rng::rng::<f32>()).abs();
            1.0 + match fg {
                true => -rnd,
                false => rnd,
            }
        };
        let size = 8.0 + rng::rng::<f32>() * 20.0; // [8, 20] radius
        Self {
            data: [x, y, height, size],
            rand: [rx, ry],
        }
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
struct PuffUniform {
    screen: Vector2<f32>,
    offset: Vector3<f32>,
    colour: Vector3<f32>,
    elapsed: f32,
    scale: f32,
}

struct PuffLayer {
    data: Vec<Puff>,
    vertex_array: VertexArray,
    #[allow(dead_code)]
    buffer: Buffer,
}
impl PuffLayer {
    const ZERO: Puff = Puff {
        data: [0.0, 0.0, 0.0, 0.0],
        rand: [0.0, 0.0],
    };

    fn new(ctx: &renderer::Context, n: usize, fg: bool) -> Result<Self> {
        let mut data = vec![];
        for _ in 0..n {
            data.push(Puff::new(ctx, fg));
        }

        let puff_size = std::mem::size_of::<Puff>() as i32;
        let buffer = BufferBuilder::new(Some("Nebula Puff Buffer"))
            .usage(BufferUsage::Static)
            .data(match n {
                0 => bytemuck::cast_slice(&[Self::ZERO]), // Dummy data
                _ => bytemuck::cast_slice(&data),
            })
            .build(&ctx.gl)?;

        let vertex_array = VertexArrayBuilder::new(Some("Nebula Puff Vertex Array"))
            .buffers(&[
                VertexArrayBuffer {
                    buffer: &ctx.vbo_center,
                    size: 2,
                    stride: 0, // tightly packed
                    offset: 0,
                    divisor: 0,
                },
                VertexArrayBuffer {
                    buffer: &buffer,
                    size: 4,
                    stride: puff_size,
                    offset: 0,
                    divisor: 1, // Advances once per instance
                },
                VertexArrayBuffer {
                    buffer: &buffer,
                    size: 2,
                    stride: puff_size,
                    offset: std::mem::offset_of!(Puff, rand) as i32,
                    divisor: 1,
                },
            ])
            .build(ctx)?;

        Ok(PuffLayer {
            data,
            buffer,
            vertex_array,
        })
    }

    fn render(&self, ctx: &renderer::Context, data: &NebulaData) -> Result<()> {
        let gl = &ctx.gl;

        let count = self.data.len();

        data.shader_puff.use_program(gl);
        self.vertex_array.bind(ctx);
        data.puff_buffer.bind_base(ctx, 0);
        unsafe {
            gl.draw_arrays_instanced(glow::TRIANGLE_STRIP, 0, 4, count as i32);
        }
        VertexArray::unbind(ctx);
        data.puff_buffer.unbind(ctx);

        Ok(())
    }
}

struct NebulaData {
    density: f32,
    view: f32,       // How far the player can see
    dx: f32,         // Length scale (space coords) for tubulence/eddies we draw.
    speed: f32,      // speed of change
    scale: f32,      // How much to scale nebula
    nebu_scale: f32, // Downscaling of the nebula
    framebuffer: Framebuffer,
    uniform: NebulaUniform,
    buffer: Buffer,
    shader_bg: Shader,
    shader_overlay: Shader,
    shader_puff: Shader,
    puffs_bg: PuffLayer,
    puffs_fg: PuffLayer,
    puff_buffer: Buffer,
    puff_uniform: PuffUniform,
}

impl NebulaData {
    fn new(ctx: &renderer::Context) -> Result<Self> {
        let gl = &ctx.gl;
        let (vw, vh, scale) = {
            let dims = ctx.dimensions.read().unwrap();
            (dims.view_width, dims.view_height, dims.view_scale)
        };
        let nebu_scale = unsafe { naevc::conf.nebu_scale as f32 };
        let nw = (vw / nebu_scale).round();
        let nh = (vh / nebu_scale).round();
        let framebuffer = FramebufferBuilder::new(Some("Nebula Framebuffer"))
            .width(nw as usize)
            .height(nh as usize)
            .build(ctx)?;

        let uniform = NebulaUniform {
            nonuninformity: unsafe { naevc::conf.nebu_nonuniformity } as f32,
            camera: Vector2::new(nw * 0.5, nh * 0.5),
            ..Default::default()
        };

        let buffer = BufferBuilder::new(Some("Nebula Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&uniform.buffer()?)
            .build(gl)?;

        let shader_bg = ProgramBuilder::new(Some("Nebula Background Shader"))
            .uniform_buffer("NebulaData", 0)
            .vert_frag_file("nebula.vert", "nebula_background.frag")
            .build(gl)?;
        let shader_overlay = ProgramBuilder::new(Some("Nebula Overlay Shader"))
            .uniform_buffer("NebulaData", 0)
            .vert_frag_file("nebula.vert", "nebula_overlay.frag")
            .build(gl)?;
        let shader_puff = ProgramBuilder::new(Some("Nebula Puff Shader"))
            .uniform_buffer("PuffData", 0)
            .prepend(&format!("const float PUFF_BUFFER = {PUFF_BUFFER:.1};\n"))
            .vert_frag_file("nebula_puff.vert", "nebula_puff.frag")
            .build(gl)?;

        let puff_uniform = {
            PuffUniform {
                screen: Vector2::new(vw * 0.5, vh * 0.5),
                scale: unsafe { 1.0 / naevc::conf.zoom_far as f32 },
                ..Default::default()
            }
        };
        let puff_buffer = BufferBuilder::new(Some("Nebula Puff Uniform Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(&puff_uniform.buffer()?)
            .build(gl)?;

        Ok(NebulaData {
            density: 0.0,
            view: 0.0,
            dx: 0.0,
            speed: 0.0,
            scale,
            nebu_scale,
            framebuffer,
            uniform,
            buffer,
            shader_bg,
            shader_overlay,
            shader_puff,
            puffs_bg: PuffLayer::new(ctx, 0, false)?,
            puffs_fg: PuffLayer::new(ctx, 0, true)?,
            puff_buffer,
            puff_uniform,
        })
    }

    pub fn resize(&mut self, ctx: &renderer::Context) {
        let (vw, vh, scale) = {
            let dims = ctx.dimensions.read().unwrap();
            (dims.view_width, dims.view_height, dims.view_scale)
        };
        let nebu_scale = unsafe { naevc::conf.nebu_scale as f32 };
        let nw = (vw / nebu_scale).round();
        let nh = (vh / nebu_scale).round();
        if self.framebuffer.w != nw as usize || self.framebuffer.h != nh as usize {
            self.framebuffer = FramebufferBuilder::new(Some("Nebula Framebuffer"))
                .width(nw as usize)
                .height(nh as usize)
                .build(ctx)
                .unwrap();
            self.uniform.camera = Vector2::new(nw * 0.5, nh * 0.5);
        }

        self.scale = scale;
        self.nebu_scale = nebu_scale;
        self.puff_uniform.screen = Vector2::new(vw + 2.0 * PUFF_BUFFER, vh + 2.0 * PUFF_BUFFER);
    }

    pub fn render(&self, ctx: &renderer::Context) -> Result<()> {
        let gl = &ctx.gl;

        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear(glow::COLOR_BUFFER_BIT);
        }

        self.shader_bg.use_program(gl);
        self.buffer.bind_base(ctx, 0);
        ctx.vao_center.bind(ctx);
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }
        VertexArray::unbind(ctx);
        self.buffer.unbind(ctx);

        unsafe {
            let screen =
                std::num::NonZero::new(naevc::gl_screen.current_fbo).map(NativeFramebuffer);
            gl.bind_framebuffer(glow::READ_FRAMEBUFFER, Some(self.framebuffer.framebuffer));
            gl.bind_framebuffer(glow::DRAW_FRAMEBUFFER, screen);
            let (w, h) = (self.framebuffer.w as i32, self.framebuffer.h as i32);
            let dims = ctx.dimensions.read().unwrap();
            gl.blit_framebuffer(
                0,
                0,
                w,
                h,
                0,
                0,
                dims.pixels_width as i32,
                dims.pixels_height as i32,
                glow::COLOR_BUFFER_BIT,
                glow::LINEAR,
            );

            gl.bind_framebuffer(glow::FRAMEBUFFER, screen);
        }

        self.puffs_bg.render(ctx, self)
    }

    pub fn render_overlay(&self, ctx: &renderer::Context) -> Result<()> {
        let gl = &ctx.gl;

        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear(glow::COLOR_BUFFER_BIT);
        }

        self.shader_overlay.use_program(gl);
        self.buffer.bind_base(ctx, 0);
        ctx.vao_center.bind(ctx);
        unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }
        VertexArray::unbind(ctx);
        self.buffer.unbind(ctx);

        unsafe {
            let screen =
                std::num::NonZero::new(naevc::gl_screen.current_fbo).map(NativeFramebuffer);
            gl.bind_framebuffer(glow::FRAMEBUFFER, screen);
        }

        {
            let (vw, vh) = {
                let dims = ctx.dimensions.read().unwrap();
                (dims.view_width, dims.view_height)
            };
            // Copy over
            if let Some(ref tex) = self.framebuffer.texture {
                tex.draw(ctx, 0.0, 0.0, vw, vh)?
            };
        }

        self.puffs_fg.render(ctx, self)
    }

    pub fn update(&mut self, ctx: &renderer::Context, dt: f64) -> Result<()> {
        let dt = (dt as f32) * self.speed;
        self.uniform.elapsed += dt;
        self.puff_uniform.elapsed += dt;

        let (modifier, bonus) = unsafe {
            if !naevc::player.p.is_null() {
                (
                    (*naevc::player.p).stats.ew_detect as f32,
                    (*naevc::player.p).stats.nebu_visibility as f32,
                )
            } else {
                (1.0, 0.0)
            }
        };
        self.view = ((1600. - self.density) * modifier + bonus) * 4.0 * self.scale;

        let z = {
            let cam = renderer::camera::CAMERA.read().unwrap();
            let cam_pos = cam.pos();
            self.puff_uniform.offset.x = cam_pos.x as f32;
            self.puff_uniform.offset.y = cam_pos.y as f32;
            cam.zoom as f32
        };
        let s = self.scale * self.nebu_scale * 4.0; // 16.0 is a correction term to be same as before
        self.uniform.horizon = self.view * z / s;
        self.uniform.eddy_scale = self.dx * z / s;
        self.puff_uniform.offset.z = z;

        // Write updates to uniform buffer
        self.buffer.write(ctx, &self.uniform.buffer()?)?;

        // Update and writes to puff buffer
        self.puff_buffer.write(ctx, &self.puff_uniform.buffer()?)?;

        Ok(())
    }

    pub fn setup(
        &mut self,
        ctx: &renderer::Context,
        density: f32,
        volatility: f32,
        hue: f32,
    ) -> Result<()> {
        self.dx = 25e3 / density.powf(1.0 / 3.0) * self.scale;
        self.density = density;
        self.speed = (2.0 * density + 200.0) / 10e3; // Faster at higher density

        let saturation = unsafe { naevc::conf.nebu_saturation as f32 };
        let value = saturation * 0.5 + 0.5;

        unsafe {
            // Ambient
            let col = Srgb::from_color(Hsv::new(360.0 * hue, saturation, value)).into_linear(); // Same as col_hsv2rgb
            naevc::gltf_lightAmbient(
                3.0 * col.red as f64,
                3.0 * col.green as f64,
                3.0 * col.blue as f64,
            );
            naevc::gltf_lightIntensity(0.5);

            // Trails
            let col =
                Srgb::from_color(Hsv::new(360.0 * hue, 0.7 * saturation, value)).into_linear();
            naevc::spfx_setNebulaColour(col.red as f64, col.green as f64, col.blue as f64);
        }

        // General uniform data
        self.uniform.hue = hue;
        self.uniform.elapsed = 0.0;
        self.uniform.volatility = volatility;
        self.uniform.saturation = saturation;

        // Puffs
        let n = (density / 4.0).round() as usize;
        self.puffs_bg = PuffLayer::new(ctx, n, false)?;
        self.puffs_fg = PuffLayer::new(ctx, n, true)?;
        let col = Srgb::from_color(Hsv::new(360.0 * hue, 0.95 * saturation, value)).into_linear();
        self.puff_uniform.colour = Vector3::new(col.red, col.green, col.blue);

        self.update(ctx, 0.0)
    }
}

use std::sync::{LazyLock, Mutex};
static NEBULA: LazyLock<Mutex<NebulaData>> = LazyLock::new(|| {
    let ctx = renderer::Context::get();
    Mutex::new(NebulaData::new(ctx).unwrap())
});

#[unsafe(no_mangle)]
pub extern "C" fn nebu_init() {}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_resize() {
    let ctx = renderer::Context::get();
    let mut neb = NEBULA.lock().unwrap();
    neb.resize(ctx);
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_exit() {}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_render(_dt: f64) {
    let neb = NEBULA.lock().unwrap();
    let ctx = renderer::Context::get();
    let _ = neb.render(ctx);
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_renderOverlay(_dt: f64) {
    let neb = NEBULA.lock().unwrap();
    let ctx = renderer::Context::get();
    let _ = neb.render_overlay(ctx);
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_update(dt: f64) {
    let mut neb = NEBULA.lock().unwrap();
    let ctx = renderer::Context::get();
    let _ = neb.update(ctx, dt);
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_getSightRadius() -> c_double {
    let neb = NEBULA.lock().unwrap();
    neb.view as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_prep(density: c_double, volatility: c_double, hue: c_double) {
    let mut neb = NEBULA.lock().unwrap();
    let ctx = renderer::Context::get();
    let _ = neb.setup(ctx, density as f32, volatility as f32, hue as f32);
}

#[unsafe(no_mangle)]
pub extern "C" fn nebu_updateColour() {}
