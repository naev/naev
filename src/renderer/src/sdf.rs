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

        Ok(Self {
            program_cross,
            buffer_cross,
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
}
