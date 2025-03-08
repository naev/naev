use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use nalgebra::{Matrix3, Vector4};

#[repr(C)]
#[derive(Debug, Copy, Clone, ShaderType)]
pub struct TextureUniform {
    pub texture: Matrix3<f32>,
    pub transform: Matrix3<f32>,
    pub colour: Vector4<f32>,
}
impl TextureUniform {
    pub fn buffer(&self) -> Result<Vec<u8>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer.into_inner())
    }
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
impl SolidUniform {
    pub fn buffer(&self) -> Result<Vec<u8>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer.into_inner())
    }
}
impl Default for SolidUniform {
    fn default() -> Self {
        SolidUniform {
            transform: Matrix3::identity(),
            colour: Vector4::<f32>::from([1.0, 1.0, 1.0, 1.0]),
        }
    }
}
