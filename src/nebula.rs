#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::*;
use nalgebra::{Vector2, Vector3};
use palette::rgb::Srgb;
use palette::FromColor;
use palette::Hsv;
use std::os::raw::c_double;

use crate::buffer::{
    Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray, VertexArrayBuffer,
    VertexArrayBuilder,
};
use crate::check_for_gl_error;
use crate::shader::{Shader, ShaderBuilder};
use crate::texture::{Framebuffer, FramebufferBuilder};
use crate::{context, rng};

pub const DEFAULT_HUE: f64 = 260.0;
pub const PUFF_BUFFER: f32 = 300.;

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
impl NebulaUniform {
    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Default, bytemuck::Pod, bytemuck::Zeroable)]
struct Puff {
    pos: [f32; 2],
    height: f32,
    size: f32,
    rand: [f32; 2], // Randomness
}
impl Puff {
    fn new(ctx: &context::Context, fg: bool) -> Self {
        let x = (ctx.view_width + 2.0 * PUFF_BUFFER) * rng::rngf32();
        let y = (ctx.view_height + 2.0 * PUFF_BUFFER) * rng::rngf32();
        let rx = rng::rngf32() * 2000.0 - 1000.0;
        let ry = rng::rngf32() * 2000.0 - 1000.0;
        let height = 0.2 * rng::rngf32();
        Self {
            pos: [x, y],
            height: 1.0
                + match fg {
                    true => height,
                    false => height.abs(),
                },
            size: rng::range(10, 32) as f32,
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
}
impl PuffUniform {
    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

struct PuffLayer {
    data: Vec<Puff>,
    buffer: Buffer,
    vertex_array: VertexArray,
}
impl PuffLayer {
    const ZERO: Puff = Puff {
        pos: [0.0, 0.0],
        height: 0.0,
        size: 0.0,
        rand: [0.0, 0.0],
    };

    fn new(ctx: &context::Context, n: usize, fg: bool) -> Result<Self> {
        let mut data = vec![];
        for _ in 0..n {
            data.push(Puff::new(ctx, fg));
        }

        let puff_size = std::mem::size_of::<Puff>();
        let buffer = BufferBuilder::new()
            .usage(BufferUsage::Static)
            .data(match n {
                0 => bytemuck::cast_slice(&[Self::ZERO]), // Dummy data
                _ => bytemuck::cast_slice(&data),
            })
            .build(&ctx.gl)?;

        let vertex_array = VertexArrayBuilder::new()
            .buffers(&[
                VertexArrayBuffer {
                    buffer: &ctx.vbo_square,
                    size: 2,
                    stride: 0, // tightly packed
                    offset: 0,
                    divisor: 0,
                },
                VertexArrayBuffer {
                    buffer: &buffer,
                    size: 4,
                    stride: puff_size as i32,
                    offset: 0,
                    divisor: 4, // Advances once per instance (aka 4 vertices)
                },
                VertexArrayBuffer {
                    buffer: &buffer,
                    size: 2,
                    stride: puff_size as i32,
                    offset: std::mem::offset_of!(Puff, rand) as i32,
                    divisor: 4,
                },
            ])
            .build(&ctx.gl)?;

        check_for_gl_error!(&ctx.gl, "Generating Nebula Puffs");

        Ok(PuffLayer {
            data,
            buffer,
            vertex_array,
        })
    }

    fn render(&self, ctx: &context::Context, data: &NebulaData) -> Result<()> {
        let gl = &ctx.gl;

        /*
        let count = self.data.len();

        data.shader_puff.use_program(gl);
        self.vertex_array.bind(ctx);
        data.puff_buffer.bind_base(ctx, 0);

        unsafe {
            gl.draw_arrays_instanced( glow::TRIANGLE_STRIP,
                0,
                4,
                count as i32,
            );
        }

        VertexArray::unbind(ctx);
        data.puff_buffer.unbind(ctx);
        */

        check_for_gl_error!(&gl, "Rendering Nebula Puffs");
        Ok(())
    }
}

struct NebulaData {
    density: f32,
    view: f32,  // How far the player can see
    dx: f32,    // Length scale (space coords) for tubulence/eddies we draw.
    speed: f32, // speed of change
    scale: f32, // How much to scale nebula
    framebuffer: Framebuffer,
    uniform: NebulaUniform,
    buffer: Buffer,
    shader_bg: Shader,
    shader_overlay: Shader,
    shader_puff: Shader,
    shader_bg_vertex: u32,
    shader_bg_uniform: u32,
    shader_overlay_vertex: u32,
    shader_overlay_uniform: u32,
    puffs_bg: PuffLayer,
    puffs_fg: PuffLayer,
    puff_buffer: Buffer,
    puff_uniform: PuffUniform,
}

impl NebulaData {
    fn new(ctx: &context::Context) -> Result<Self> {
        let gl = &ctx.gl;
        let (w, h) = unsafe { (naevc::gl_screen.w, naevc::gl_screen.h) };
        let framebuffer = FramebufferBuilder::new()
            .width(w as usize)
            .height(h as usize)
            .build(ctx)?;

        let mut uniform = NebulaUniform::default();
        uniform.nonuninformity = unsafe { naevc::conf.nebu_nonuniformity } as f32;
        uniform.camera = Vector2::new((w as f32) * 0.5, (h as f32) * 0.5);
        let scale = unsafe { naevc::conf.nebu_scale * naevc::gl_screen.scale } as f32;

        let buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(uniform.buffer()?.into_inner().as_slice())
            .build(gl)?;

        let shader_bg = ShaderBuilder::new(Some("Nebula Background Shader"))
            .vert_file("nebula.vert")
            .frag_file("nebula_background.frag")
            .build(gl)?;
        let shader_overlay = ShaderBuilder::new(Some("Nebula Overlay Shader"))
            .vert_file("nebula.vert")
            .frag_file("nebula_overlay.frag")
            .build(gl)?;
        let shader_puff = ShaderBuilder::new(Some("Nebula Puff Shader"))
            .vert_file("nebula_puff.vert")
            .frag_file("nebula_puff.frag")
            .build(gl)?;

        let shader_bg_vertex = shader_bg.get_attrib(gl, "vertex")?;
        let shader_bg_uniform = shader_bg.get_uniform_block(gl, "NebulaData")?;
        let shader_overlay_vertex = shader_overlay.get_attrib(gl, "vertex")?;
        let shader_overlay_uniform = shader_bg.get_uniform_block(gl, "NebulaData")?;

        let puff_uniform = PuffUniform::default();
        let puff_buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Static)
            .data(puff_uniform.buffer()?.into_inner().as_slice())
            .build(&gl)?;

        check_for_gl_error!(&gl, "Creating NebulaData");

        Ok(NebulaData {
            density: 0.0,
            view: 0.0,
            dx: 0.0,
            speed: 0.0,
            scale,
            framebuffer,
            uniform,
            buffer,
            shader_bg,
            shader_overlay,
            shader_puff,
            shader_bg_vertex,
            shader_bg_uniform,
            shader_overlay_vertex,
            shader_overlay_uniform,
            puffs_bg: PuffLayer::new(ctx, 0, false)?,
            puffs_fg: PuffLayer::new(ctx, 0, true)?,
            puff_buffer,
            puff_uniform,
        })
    }

    pub fn resize(&mut self, ctx: &context::Context) {
        let scale = unsafe { naevc::conf.nebu_scale * naevc::gl_screen.scale } as f32;
        let w = (unsafe { naevc::gl_screen.nw as f32 } / scale).round() as usize;
        let h = (unsafe { naevc::gl_screen.nh as f32 } / scale).round() as usize;
        if self.framebuffer.w == w && self.framebuffer.h == h {
            return;
        }

        self.scale = scale;
        self.framebuffer = FramebufferBuilder::new()
            .width(w)
            .height(h)
            .build(ctx)
            .unwrap();

        self.uniform.camera = Vector2::new(ctx.view_width * 0.5, ctx.view_height * 0.5);
    }

    pub fn render(&self, ctx: &context::Context) -> Result<()> {
        let gl = &ctx.gl;

        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear_color(0.0, 0.0, 0.0, 0.0);
            gl.clear(glow::COLOR_BUFFER_BIT);
        }

        self.shader_bg.use_program(gl);
        self.buffer.bind_base(ctx, self.shader_bg_uniform);
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
            gl.blit_framebuffer(
                0,
                0,
                w,
                h,
                0,
                0,
                ctx.window_width as i32,
                ctx.window_height as i32,
                glow::COLOR_BUFFER_BIT,
                glow::LINEAR,
            );

            gl.bind_framebuffer(glow::FRAMEBUFFER, screen);
        }
        check_for_gl_error!(&gl, "Rendering Nebula Background");

        self.puffs_bg.render(ctx, &self)
    }

    pub fn render_overlay(&self, ctx: &context::Context) -> Result<()> {
        let gl = &ctx.gl;

        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear(glow::COLOR_BUFFER_BIT);
        }

        self.shader_overlay.use_program(gl);
        self.buffer.bind_base(ctx, self.shader_overlay_uniform);
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

        // Copy over
        self.framebuffer
            .texture
            .draw(ctx, 0.0, 0.0, ctx.view_width, ctx.view_height)?;

        check_for_gl_error!(&gl, "Rendering Nebula Overlay");

        self.puffs_fg.render(ctx, &self)
    }

    pub fn update(&mut self, ctx: &context::Context, dt: f64) -> Result<()> {
        let dt = (dt as f32) * self.speed;
        self.uniform.elapsed += dt;

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
        self.view = (1600. - self.density) * modifier + bonus;

        let z = crate::camera::CAMERA.lock().unwrap().zoom as f32;
        self.uniform.horizon = self.view * z / self.scale;
        self.uniform.eddy_scale = self.dx * z / self.scale;

        // Write updates to uniform buffer
        self.buffer
            .write(ctx, self.uniform.buffer()?.into_inner().as_slice())?;

        // TODO puff updates, maybe can get away with shader magic

        Ok(())
    }

    pub fn setup(
        &mut self,
        ctx: &context::Context,
        density: f32,
        volatility: f32,
        hue: f32,
    ) -> Result<()> {
        self.density = density;
        self.speed = (2.0 * density + 200.0) / 10e3; // Faster at higher density
        self.dx = 15e3 / density.powf(1.0 / 3.0);

        let saturation = unsafe { naevc::conf.nebu_saturation as f32 };
        let value = saturation * 0.5 + 0.5;

        /*
        /* Set up ambient colour. */
        col_hsv2rgb( &col, nebu_hue * 360., saturation, value );
        gltf_lightAmbient( 3.0 * col.r, 3.0 * col.g, 3.0 * col.b );
        gltf_lightIntensity( 0.5 );

        /* Also set the hue for trails */
        col_hsv2rgb( &col, nebu_hue * 360., 0.7 * conf.nebu_saturation, value );
        spfx_setNebulaColour( col.r, col.g, col.b );

        /* Also set the hue for puffs. */
        col_hsv2rgb( &col, nebu_hue * 360., 0.95 * conf.nebu_saturation, value );
        glUseProgram( shaders.nebula_puff.program );
        glUniform3f( shaders.nebula_puff.nebu_col, col.r, col.g, col.b );
         */

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

            // Puffs
            //col_hsv2rgb( &col, nebu_hue * 360., 0.95 * conf.nebu_saturation, value );
            //glUseProgram( shaders.nebula_puff.program );
            //glUniform3f( shaders.nebula_puff.nebu_col, col.r, col.g, col.b );
        }

        self.uniform.hue = hue;
        self.uniform.elapsed = 0.0;
        self.uniform.volatility = volatility;
        self.uniform.saturation = saturation;

        let n = (density / 4.0).round() as usize;
        self.puffs_bg = PuffLayer::new(ctx, n, false)?;
        self.puffs_fg = PuffLayer::new(ctx, n, true)?;

        self.update(ctx, 0.0)
    }
}

use std::sync::{LazyLock, Mutex};
static NEBULA: LazyLock<Mutex<NebulaData>> = LazyLock::new(|| {
    let ctx = context::CONTEXT.get().unwrap();
    Mutex::new(NebulaData::new(ctx).unwrap())
});

#[no_mangle]
pub extern "C" fn nebu_init() {}

#[no_mangle]
pub extern "C" fn nebu_resize() {
    let ctx = context::CONTEXT.get().unwrap();
    let mut neb = NEBULA.lock().unwrap();
    neb.resize(ctx);
}

#[no_mangle]
pub extern "C" fn nebu_exit() {}

#[no_mangle]
pub extern "C" fn nebu_render(_dt: f64) {
    let neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    let _ = neb.render(ctx);
}

#[no_mangle]
pub extern "C" fn nebu_renderOverlay(_dt: f64) {
    let neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    let _ = neb.render_overlay(ctx);
}

#[no_mangle]
pub extern "C" fn nebu_update(dt: f64) {
    let mut neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    let _ = neb.update(ctx, dt);
}

#[no_mangle]
pub extern "C" fn nebu_getSightRadius() -> c_double {
    let neb = NEBULA.lock().unwrap();
    neb.view as c_double
}

#[no_mangle]
pub extern "C" fn nebu_prep(density: c_double, volatility: c_double, hue: c_double) {
    let mut neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    let _ = neb.setup(ctx, density as f32, volatility as f32, hue as f32);
}

#[no_mangle]
pub extern "C" fn nebu_updateColour() {}
