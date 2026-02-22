use crate::colour::Colour;
use crate::{
   Buffer, BufferBuilder, BufferTarget, BufferUsage, Context, ProgramBuilder, Shader, Uniform,
   VertexArray,
};
use anyhow::Result;
use encase::ShaderType;
use glow::HasContext;
use nalgebra::{Matrix3, Vector2, Vector4};
use nlog::warn_err;
use physics::transform2::Transform2;

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct RectHollowUniform {
   pub transform: Transform2,
   pub colour: Colour,
   pub dims: Vector2<f32>,
   pub border: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct CrossUniform {
   pub transform: Transform2,
   pub colour: Colour,
   pub radius: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct CircleUniform {
   pub transform: Transform2,
   pub colour: Colour,
   pub radius: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct CircleHollowUniform {
   pub transform: Transform2,
   pub colour: Colour,
   pub radius: f32,
   pub border: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct TriangleHollowUniform {
   pub transform: Transform2,
   pub colour: Colour,
   pub scale: Vector2<f32>,
   pub border: f32,
}

macro_rules! draw_sdf_func_ex {
   ($funcname: ident, $vao: tt, $uniform: ty, $program: tt, $buffer: tt) => {
      pub fn $funcname(&self, ctx: &Context, uniform: &$uniform) -> Result<()> {
         let gl = &ctx.gl;
         self.$program.use_program(gl);
         ctx.$vao.bind(ctx);
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
   program_rect_hollow: Shader,
   buffer_rect_hollow: Buffer,
   program_cross: Shader,
   buffer_cross: Buffer,
   program_circle: Shader,
   buffer_circle: Buffer,
   program_circle_hollow: Shader,
   buffer_circle_hollow: Buffer,
   program_triangle_hollow: Shader,
   buffer_triangle_hollow: Buffer,
}
impl SdfRenderer {
   pub fn new(gl: &glow::Context) -> Result<Self> {
      let program_rect_hollow = ProgramBuilder::new(Some("Rect Hollow Shader"))
         .uniform_buffer("rectdata", 0)
         .wgsl_file("rect_hollow.wgsl")
         .build(gl)?;
      let buffer_rect_hollow = BufferBuilder::new(Some("Rect Hollow Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&RectHollowUniform::default().buffer()?)
         .build(gl)?;

      let program_cross = ProgramBuilder::new(Some("Cross Shader"))
         .uniform_buffer("crossdata", 0)
         .wgsl_file("cross.wgsl")
         .build(gl)?;
      let buffer_cross = BufferBuilder::new(Some("Cross Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&CrossUniform::default().buffer()?)
         .build(gl)?;

      let program_circle = ProgramBuilder::new(Some("Circle Shader"))
         .uniform_buffer("circledata", 0)
         .wgsl_file("circle.wgsl")
         .build(gl)?;
      let buffer_circle = BufferBuilder::new(Some("Circle Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&CircleUniform::default().buffer()?)
         .build(gl)?;

      let program_circle_hollow = ProgramBuilder::new(Some("Circle Hollow Shader"))
         .uniform_buffer("circledata", 0)
         .wgsl_file("circle_hollow.wgsl")
         .build(gl)?;
      let buffer_circle_hollow = BufferBuilder::new(Some("Circle Hollow Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&CircleHollowUniform::default().buffer()?)
         .build(gl)?;

      let program_triangle_hollow = ProgramBuilder::new(Some("Triangle Hollow Shader"))
         .uniform_buffer("triangledata", 0)
         .wgsl_file("triangle_hollow.wgsl")
         .build(gl)?;
      let buffer_triangle_hollow = BufferBuilder::new(Some("Triangle Hollow Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&TriangleHollowUniform::default().buffer()?)
         .build(gl)?;

      Ok(Self {
         program_rect_hollow,
         buffer_rect_hollow,
         program_cross,
         buffer_cross,
         program_circle,
         buffer_circle,
         program_circle_hollow,
         buffer_circle_hollow,
         program_triangle_hollow,
         buffer_triangle_hollow,
      })
   }

   #[allow(clippy::too_many_arguments)]
   pub fn draw_rect_hollow(
      &self,
      ctx: &Context,
      x: f32,
      y: f32,
      w: f32,
      h: f32,
      b: f32,
      colour: Colour,
   ) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
      let transform: Matrix3<f32> = dims.projection * Matrix3::new(
          w,  0.0,  x,
         0.0,  h,   y,
         0.0, 0.0, 1.0,
      );
      let uniform = RectHollowUniform {
         transform: transform.into(),
         colour,
         dims: Vector2::new(w, h),
         border: b,
      };
      self.draw_rect_hollow_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(
      draw_rect_hollow_ex,
      vao_square,
      RectHollowUniform,
      program_rect_hollow,
      buffer_rect_hollow
   );

   pub fn draw_cross(&self, ctx: &Context, x: f32, y: f32, r: f32, colour: Colour) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
      let transform: Matrix3<f32> = dims.projection * Matrix3::new(
          r,  0.0,  x,
         0.0,  r,   y,
         0.0, 0.0, 1.0,
      );
      let uniform = CrossUniform {
         transform: transform.into(),
         colour,
         radius: r,
      };
      self.draw_cross_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(
      draw_cross_ex,
      vao_center,
      CrossUniform,
      program_cross,
      buffer_cross
   );

   pub fn draw_circle(&self, ctx: &Context, x: f32, y: f32, r: f32, colour: Colour) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
      let transform: Matrix3<f32> = dims.projection * Matrix3::new(
          r,  0.0,  x,
         0.0,  r,   y,
         0.0, 0.0, 1.0,
      );
      let uniform = CircleUniform {
         transform: transform.into(),
         colour,
         radius: r,
      };
      self.draw_circle_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(
      draw_circle_ex,
      vao_center,
      CircleUniform,
      program_circle,
      buffer_circle
   );

   pub fn draw_circle_hollow(
      &self,
      ctx: &Context,
      x: f32,
      y: f32,
      r: f32,
      colour: Colour,
      b: f32,
   ) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      #[rustfmt::skip]
      let transform: Matrix3<f32> = dims.projection * Matrix3::new(
          r,  0.0,  x,
         0.0,  r,   y,
         0.0, 0.0, 1.0,
      );
      let uniform = CircleHollowUniform {
         transform: transform.into(),
         colour,
         radius: r,
         border: b,
      };
      self.draw_circle_hollow_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(
      draw_circle_hollow_ex,
      vao_center,
      CircleHollowUniform,
      program_circle_hollow,
      buffer_circle_hollow
   );

   #[allow(clippy::too_many_arguments)]
   pub fn draw_triangle_hollow(
      &self,
      ctx: &Context,
      x: f32,
      y: f32,
      angle: f32,
      scale: f32,
      length: f32,
      colour: Colour,
      b: f32,
   ) -> Result<()> {
      let dims = ctx.dimensions.read().unwrap();
      let r = scale;
      let (q, l) = if length * 1.5 > 2.0 {
         let s = 2.0 / (length * 1.5);
         (r * s, r * 2.0)
      } else {
         (r, r * length)
      };
      let c = angle.cos();
      let s = angle.sin();
      #[rustfmt::skip]
      let transform: Matrix3<f32> = dims.projection * Matrix3::new(
          c,   s,   x,
         -s,   c,   y,
         0.0, 0.0, 1.0,
      ) * Matrix3::new(
          r ,0.0, 0.0,
         0.0,  r,  0.0,
         0.0, 0.0, 1.0,
      );
      let uniform = TriangleHollowUniform {
         transform: transform.into(),
         colour,
         scale: Vector2::new(q, -l),
         border: b,
      };
      self.draw_triangle_hollow_ex(ctx, &uniform)
   }
   draw_sdf_func_ex!(
      draw_triangle_hollow_ex,
      vao_center,
      TriangleHollowUniform,
      program_triangle_hollow,
      buffer_triangle_hollow
   );
}

use std::ffi::{c_double, c_int};

fn get_col(c: *const Vector4<f32>) -> Colour {
   match c.is_null() {
      true => Colour::default(),
      false => unsafe { *c }.into(),
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderCross(x: c_double, y: c_double, r: c_double, c: *const Vector4<f32>) {
   let ctx = Context::get();
   let colour = get_col(c);
   if let Err(e) = ctx
      .sdf
      .draw_cross(ctx, x as f32, y as f32, r as f32, colour)
   {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderCircle(
   x: c_double,
   y: c_double,
   r: c_double,
   c: *const Vector4<f32>,
   filled: c_int,
) {
   let ctx = Context::get();
   let colour = get_col(c);
   let x = x as f32;
   let y = y as f32;
   let r = r as f32;
   let res = if filled != 0 {
      ctx.sdf.draw_circle(ctx, x, y, r, colour)
   } else {
      ctx.sdf.draw_circle_hollow(ctx, x, y, r, colour, 0.0)
   };
   if let Err(e) = res {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderRectEmpty(
   x: c_double,
   y: c_double,
   w: c_double,
   h: c_double,
   c: *const Vector4<f32>,
) {
   let ctx = Context::get();
   let colour = get_col(c);
   if let Err(e) = ctx
      .sdf
      .draw_rect_hollow(ctx, x as f32, y as f32, w as f32, h as f32, 0.0, colour)
   {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderRectEmptyThick(
   x: c_double,
   y: c_double,
   w: c_double,
   h: c_double,
   b: c_double,
   c: *const Vector4<f32>,
) {
   let ctx = Context::get();
   let colour = get_col(c);
   if let Err(e) = ctx.sdf.draw_rect_hollow(
      ctx,
      x as f32,
      y as f32,
      w as f32,
      h as f32,
      b as f32 - 1.0,
      colour,
   ) {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderRectH(t: *const Matrix3<f32>, c: *const Vector4<f32>, filled: c_int) {
   let ctx = Context::get();
   let transform = unsafe { *t };
   let colour = get_col(c);
   let ret = if filled != 0 {
      let uniform = crate::SolidUniform {
         transform: transform.into(),
         colour,
      };
      ctx.draw_rect_ex(&uniform)
   } else {
      let uniform = RectHollowUniform {
         transform: transform.into(),
         colour,
         dims: Vector2::new(0.0, 0.0),
         border: 0.0,
      };
      ctx.sdf.draw_rect_hollow_ex(ctx, &uniform)
   };
   if let Err(e) = ret {
      warn_err!(e);
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_renderTriangleEmpty(
   x: c_double,
   y: c_double,
   a: c_double,
   s: c_double,
   length: c_double,
   c: *const Vector4<f32>,
) {
   let ctx = Context::get();
   let colour = get_col(c);
   if let Err(e) = ctx.sdf.draw_triangle_hollow(
      ctx,
      x as f32,
      y as f32,
      a as f32,
      s as f32,
      length as f32,
      colour,
      0.0,
   ) {
      warn_err!(e);
   }
}
