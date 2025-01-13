#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::*;
use nalgebra::Vector2;
use std::os::raw::c_double;

use crate::buffer::{Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray};
use crate::check_for_gl_error;
use crate::context;
use crate::shader::{Shader, ShaderBuilder};
use crate::texture::{Framebuffer, FramebufferBuilder};

pub const DEFAULT_HUE: f64 = 260.0;

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

        let shader_bg_vertex = shader_bg.get_attrib(gl, "vertex")?;
        let shader_bg_uniform = shader_bg.get_uniform_block(gl, "NebulaData")?;
        let shader_overlay_vertex = shader_overlay.get_attrib(gl, "vertex")?;
        let shader_overlay_uniform = shader_bg.get_uniform_block(gl, "NebulaData")?;

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
        self.buffer
            .write(ctx, self.uniform.buffer()?.into_inner().as_slice())?;
        self.buffer.bind(ctx);
        unsafe {
            gl.bind_buffer_base(
                glow::UNIFORM_BUFFER,
                self.shader_bg_uniform,
                Some(self.buffer.buffer),
            );

            ctx.vao_center.bind(ctx);
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
        Ok(())
    }

    pub fn render_overlay(&self, ctx: &context::Context) -> Result<()> {
        let gl = &ctx.gl;

        self.framebuffer.bind(ctx);
        unsafe {
            gl.clear(glow::COLOR_BUFFER_BIT);
        }

        self.shader_overlay.use_program(gl);
        // Already updated when rendering background
        //self.buffer.write(ctx, self.uniform.buffer()?.into_inner().as_slice())?;
        self.buffer.bind(ctx);
        unsafe {
            gl.bind_buffer_base(
                glow::UNIFORM_BUFFER,
                self.shader_overlay_uniform,
                Some(self.buffer.buffer),
            );

            ctx.vao_center.bind(ctx);
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
        Ok(())
    }

    pub fn update(&mut self, dt: f64) {
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

        // TODO puff updates, maybe can get away with shader magic
    }

    pub fn setup(&mut self, density: f32, volatility: f32, hue: f32) {
        self.density = density;
        self.speed = (2.0 * density + 200.0) / 10e3; // Faster at higher density
        self.dx = 15e3 / density.powf(1.0 / 3.0);

        let saturation = unsafe { naevc::conf.nebu_saturation as f32 };
        let _value = saturation * 0.5 + 0.5;
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

        self.uniform.hue = hue;
        self.uniform.elapsed = 0.0;
        self.uniform.volatility = volatility;
        self.uniform.saturation = saturation;

        //if density > 0.0;

        self.update(0.0);
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
    neb.update(dt);
}

#[no_mangle]
pub extern "C" fn nebu_getSightRadius() -> c_double {
    let neb = NEBULA.lock().unwrap();
    neb.view as c_double
}

#[no_mangle]
pub extern "C" fn nebu_prep(density: c_double, volatility: c_double, hue: c_double) {
    let mut neb = NEBULA.lock().unwrap();
    neb.setup(density as f32, volatility as f32, hue as f32);
}

#[no_mangle]
pub extern "C" fn nebu_updateColour() {}
