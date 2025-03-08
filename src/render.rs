use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use nalgebra::{Matrix3, Vector4};

// Use trait extension to give buffer support
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
        TextureUniform {
            texture: Matrix3::identity(),
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
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
        SolidUniform {
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        }
    }
}
