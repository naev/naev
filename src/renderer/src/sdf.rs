use crate::{
   Buffer, BufferBuilder, BufferTarget, BufferUsage, Context, ProgramBuilder, Shader,
   TextureUniform, Uniform, VertexArray,
};
use anyhow::Result;
use encase::ShaderType;
use glow::HasContext;
use nalgebra::{Matrix3, Vector4};

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct CrossUniform {
   pub transform: Matrix3<f32>,
   pub colour: Vector4<f32>,
   pub radius: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct CircleUniform {
   pub transform: Matrix3<f32>,
   pub colour: Vector4<f32>,
   pub radius: f32,
}

macro_rules! draw_sdf_func_ex {
   ($funcname: ident, $uniform: ty, $program: tt, $buffer: tt) => {
      pub fn $funcname(&self, ctx: &Context, uniform: &$uniform) -> Result<()> {
         let gl = &ctx.gl;
         self.$program.use_program(gl);
         ctx.vao_center.bind(ctx);
         self.$buffer.bind_write_base(ctx, &uniform.buffer()?, 0)?;
         unsafe {
            gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
         }
         VertexArray::unbind(ctx);
         self.$buffer.unbind(ctx);
         Ok(())
      }
   };
}

pub struct SdfRenderer {
   program_cross: Shader,
   buffer_cross: Buffer,
   program_circle: Shader,
   buffer_circle: Buffer,
}
impl SdfRenderer {
   pub fn new(gl: &glow::Context) -> Result<Self> {
      let program_cross = ProgramBuilder::new(Some("Cross Shader"))
         .uniform_buffer("crossdata", 0)
         .wgsl_file("cross.wgsl")
         .build(gl)?;
      let buffer_cross = BufferBuilder::new(Some("Cross Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&TextureUniform::default().buffer()?)
         .build(gl)?;

      let program_circle = ProgramBuilder::new(Some("Circle Shader"))
         .uniform_buffer("circledata", 0)
         .wgsl_file("circle.wgsl")
         .build(gl)?;
      let buffer_circle = BufferBuilder::new(Some("Circle Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&TextureUniform::default().buffer()?)
         .build(gl)?;

      Ok(Self {
         program_cross,
         buffer_cross,
         program_circle,
         buffer_circle,
      })
   }

   pub fn draw_cross(
      &self,
      ctx: &Context,
      x: f32,
      y: f32,
      r: f32,
      colour: Vector4<f32>,
   ) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             r,  0.0,  x,
            0.0,  r,   y,
            0.0, 0.0, 1.0,
        );
      let uniform = CrossUniform {
         transform,
         colour,
         radius: r,
      };
      self.draw_cross_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(draw_cross_ex, CrossUniform, program_cross, buffer_cross);

   pub fn draw_circle(
      &self,
      ctx: &Context,
      x: f32,
      y: f32,
      r: f32,
      colour: Vector4<f32>,
   ) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
        let transform: Matrix3<f32> = dims.projection * Matrix3::new(
             r,  0.0,  x,
            0.0,  r,   y,
            0.0, 0.0, 1.0,
        );
      let uniform = CircleUniform {
         transform,
         colour,
         radius: r,
      };
      self.draw_circle_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(draw_circle_ex, CircleUniform, program_circle, buffer_circle);
}

use std::ffi::c_double; //{c_double, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderCross(x: c_double, y: c_double, r: c_double, c: *const Vector4<f32>) {
   let ctx = Context::get();
   let colour = match c.is_null() {
      true => Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
      false => unsafe { *c },
   };
   let _ = ctx
      .sdf
      .draw_cross(ctx, x as f32, y as f32, r as f32, colour);
}

/*
#[unsafe(no_mangle)]
pub extern "C" fn gl_renderCircle(
    x: c_double,
    y: c_double,
    r: c_double,
    c: *const Vector4<f32>,
    _filled: c_int,
) {
    let ctx = Context::get();
    let colour = match c.is_null() {
        true => Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        false => unsafe { *c },
    };
    let _ = ctx
        .sdf
        .draw_circle(&ctx, x as f32, y as f32, r as f32, colour);
}
*/
