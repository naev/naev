#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::*;
use nalgebra::Matrix3;

use crate::buffer::{Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray};
use crate::context;
use crate::shader::{Shader, ShaderBuilder};
use crate::texture::{Framebuffer, FramebufferBuilder};

pub const DEFAULT_HUE: f64 = 260.0;

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
struct NebulaUniform {
    horizon: f32,
    eddy_scale: f32,
    elapsed: f32,
    nonuninformity: f32,
    transform: Matrix3<f32>,
}
impl NebulaUniform {
    pub fn new() -> Self {
        NebulaUniform {
            ..Default::default()
        }
    }
    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

struct NebulaData {
    hue: f32,
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
    shader_bg_vertex: u32,
    shader_bg_uniform: u32,
    shader_overlay_vertex: u32,
    shader_overlay_uniform: u32,
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

        let buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(uniform.buffer()?.into_inner().as_slice())
            .build(gl)?;

        let shader_bg = ShaderBuilder::new(Some("Nebula Background Shader"))
            .vert_file("nebula.vert")
            .frag_file("nebula_background.frag")
            .build(ctx)?;

        let shader_overlay = ShaderBuilder::new(Some("Nebula Overlay Shader"))
            .vert_file("nebula.vert")
            .frag_file("nebula_overlay.frag")
            .build(ctx)?;

        let shader_bg_vertex = shader_bg.get_attrib(gl, "vertex")?;
        let shader_bg_uniform = shader_bg.get_uniform_block(gl, "nebula_data")?;
        let shader_overlay_vertex = shader_overlay.get_attrib(gl, "vertex")?;
        let shader_overlay_uniform = shader_bg.get_uniform_block(gl, "nebula_data")?;

        Ok(NebulaData {
            hue: 0.0,
            density: 0.0,
            view: 0.0,
            dx: 0.0,
            speed: 0.0,
            scale: 0.0,
            framebuffer,
            uniform,
            buffer,
            shader_bg,
            shader_overlay,
            shader_bg_vertex,
            shader_bg_uniform,
            shader_overlay_vertex,
            shader_overlay_uniform,
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
            .width(w as usize)
            .height(h as usize)
            .build(ctx)
            .unwrap();

        //self.uniform.transform =
    }

    pub fn render(&self, ctx: &context::Context) -> Result<()> {
        let gl = &ctx.gl;
        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear(glow::COLOR_BUFFER_BIT | glow::DEPTH_BUFFER_BIT);
        }

        self.shader_bg.use_program(gl);
        self.buffer
            .write(ctx, self.uniform.buffer()?.into_inner().as_slice())?;
        self.buffer.bind(ctx);
        unsafe {
            gl.bind_buffer_base(
                glow::UNIFORM_BUFFER,
                self.shader_bg_uniform,
                Some(self.buffer.buffer),
            );

            ctx.vao_square.bind(ctx);
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
        }
        VertexArray::unbind(ctx);
        self.buffer.unbind(ctx);

        // Copy over
        Ok(())
    }

    pub fn render_overlay(&self, ctx: &context::Context) {
        let gl = &ctx.gl;
        self.shader_overlay.use_program(gl);
    }

    pub fn update(&mut self, dt: f64) {
        let dt = (dt as f32) * self.speed;
        self.uniform.elapsed += dt as f32;

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
        self.view = ((1600. - self.density) * modifier + bonus) as f32;

        let z = crate::camera::CAMERA.lock().unwrap().zoom as f32;
        self.uniform.horizon = self.view * z / self.scale;
        self.uniform.eddy_scale = self.dx * z / self.scale;

        // TODO puff updates, maybe can get away with shader magic
    }
}

use std::sync::{LazyLock, Mutex};
static NEBULA: LazyLock<Mutex<NebulaData>> = LazyLock::new(|| {
    let ctx = context::CONTEXT.get().unwrap();
    Mutex::new(NebulaData::new(&ctx).unwrap())
});

pub fn resize() {
    let ctx = context::CONTEXT.get().unwrap();
    let mut neb = NEBULA.lock().unwrap();
    neb.resize(&ctx);
}

pub fn render() {
    let neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    let _ = neb.render(&ctx);
}

pub fn render_overlay() {
    let neb = NEBULA.lock().unwrap();
    let ctx = context::CONTEXT.get().unwrap();
    neb.render_overlay(&ctx);
}

pub fn update(dt: f64) {
    let mut neb = NEBULA.lock().unwrap();
    neb.update(dt);
}
