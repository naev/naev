#![allow(dead_code)]
use anyhow::Result;
use glow::*;

use crate::context;
use crate::context::{Context, CONTEXT};
use crate::{formatx, gettext, warn};

pub struct Buffer {
    pub buffer: glow::Buffer,
    datalen: usize, // in u8
    target: u32,
    usage: u32,
}
impl Buffer {
    pub fn write(&self, ctx: &Context, data: &[u8]) -> Result<()> {
        if data.len() != self.datalen {
            anyhow::bail!("buffer data length mismatch!");
        }
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, Some(self.buffer));
            gl.buffer_data_u8_slice(self.target, data, self.usage);
            gl.bind_buffer(self.target, None);
        }
        Ok(())
    }
    /// Simply binds the buffer to the context
    pub fn bind(&self, ctx: &context::Context) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, Some(self.buffer));
        }
    }
    /// Binds the buffer and connects it to the uniform in the shader
    pub fn bind_base(&self, ctx: &context::Context, idx: u32) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, Some(self.buffer));
            gl.bind_buffer_base(glow::UNIFORM_BUFFER, idx, Some(self.buffer));
        }
    }
    /// Unbinds the buffer
    pub fn unbind(&self, ctx: &context::Context) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, None);
            // TODO why is this necessary? Shouldn't be, but gives issues otherwise
            // Maybe remove when C code is cleaned up...
            gl.bind_buffer_base(glow::UNIFORM_BUFFER, 0, None);
        }
    }
}
impl Drop for Buffer {
    fn drop(&mut self) {
        let ctx = CONTEXT.get().unwrap();
        unsafe {
            ctx.gl.delete_buffer(self.buffer);
        }
    }
}

pub enum BufferUsage {
    Stream,
    Dynamic,
    Static,
}
impl BufferUsage {
    pub fn to_gl(&self) -> u32 {
        match self {
            Self::Stream => glow::STREAM_DRAW,
            Self::Dynamic => glow::DYNAMIC_DRAW,
            Self::Static => glow::STATIC_DRAW,
        }
    }
}

pub enum BufferTarget {
    Array,
    Uniform,
}
impl BufferTarget {
    pub fn to_gl(&self) -> u32 {
        match self {
            Self::Array => glow::ARRAY_BUFFER,
            Self::Uniform => glow::UNIFORM_BUFFER,
        }
    }
}

pub struct BufferBuilder<'a> {
    target: BufferTarget,
    usage: BufferUsage,
    data: &'a [u8],
}
impl<'a> BufferBuilder<'a> {
    pub fn new() -> Self {
        BufferBuilder {
            target: BufferTarget::Array,
            usage: BufferUsage::Stream,
            data: Default::default(),
        }
    }

    pub fn target(mut self, target: BufferTarget) -> Self {
        self.target = target;
        self
    }

    pub fn usage(mut self, usage: BufferUsage) -> Self {
        self.usage = usage;
        self
    }

    pub fn data(mut self, data: &'a [u8]) -> Self {
        self.data = data;
        self
    }

    pub fn data_f32(self, data: &'a [f32]) -> Self {
        self.data(unsafe {
            std::slice::from_raw_parts(data.as_ptr() as *const u8, std::mem::size_of_val(data))
        })
    }

    pub fn build(self, gl: &glow::Context) -> Result<Buffer> {
        if self.data.is_empty() {
            anyhow::bail!("BufferBuilder has no data");
        }

        let buffer = unsafe { gl.create_buffer().map_err(|e| anyhow::anyhow!(e))? };

        let target = self.target.to_gl();
        let usage = self.usage.to_gl();
        unsafe {
            gl.bind_buffer(target, Some(buffer));
            gl.buffer_data_u8_slice(target, self.data, usage);
            gl.bind_buffer(target, None);
        }

        /*
        let size = match self.target {
            BufferTarget::Uniform => {
                unsafe{ gl.get_active_uniform_block_parameter_i32( shader.program, buffer, glow::UNIFORM_BLOCK_DATA_SIZE ) as usize }
            },
            _ => 0,
        };
        */

        Ok(Buffer {
            buffer,
            datalen: self.data.len(),
            target,
            usage,
        })
    }
}

pub struct VertexArray {
    pub vertex_array: glow::VertexArray,
}
impl Drop for VertexArray {
    fn drop(&mut self) {
        let ctx = CONTEXT.get().unwrap();
        unsafe {
            ctx.gl.delete_vertex_array(self.vertex_array);
        }
    }
}
impl VertexArray {
    pub fn bind(&self, ctx: &context::Context) {
        unsafe {
            ctx.gl.bind_vertex_array(Some(self.vertex_array));
        }
    }

    pub fn unbind(ctx: &context::Context) {
        unsafe {
            ctx.gl.bind_vertex_array(None);
            // Horrible hack since C-side uses a single VAO for everything
            ctx.gl.bind_vertex_array(Some(glow::NativeVertexArray(
                std::num::NonZero::new(naevc::VaoId).unwrap(),
            )));
        }
    }
}

pub struct VertexArrayBuffer<'a> {
    pub buffer: &'a Buffer, // Buffer
    pub size: i32,          // in data_type units (1 to 4)
    pub stride: i32,        // in bytes, 0 indicates tightly packed
    pub offset: i32,        // in bytes
    pub divisor: u32,       // 0 indicates per vertex, non-zero is advance per instances
}
pub struct VertexArrayBuilder<'a> {
    data_type: u32, // glow::FLOAT and such
    normalized: bool,
    buffers: &'a [VertexArrayBuffer<'a>],
}
impl<'a> VertexArrayBuilder<'a> {
    pub fn new() -> Self {
        VertexArrayBuilder {
            data_type: glow::FLOAT,
            normalized: false,
            buffers: &[],
        }
    }

    pub fn data_type(mut self, data_type: u32) -> Self {
        self.data_type = data_type;
        self
    }

    pub fn normalized(mut self, normalized: bool) -> Self {
        self.normalized = normalized;
        self
    }

    pub fn buffers(mut self, buffers: &'a [VertexArrayBuffer]) -> Self {
        self.buffers = buffers;
        self
    }

    pub fn build(self, gl: &glow::Context) -> Result<VertexArray> {
        let vertex_array = unsafe { gl.create_vertex_array().map_err(|e| anyhow::anyhow!(e))? };
        /*
        let data_size = match self.data_type {
            glow::BYTE | glow::UNSIGNED_BYTE => 1,
            glow::SHORT | glow::UNSIGNED_SHORT => 2,
            glow::INT | glow::UNSIGNED_INT | glow::FIXED | glow::FLOAT => 4,
            glow::DOUBLE => 8,
            _ => anyhow::bail!("unsupported data type '{:#x}'", self.data_type),
        };
        */
        unsafe {
            gl.bind_vertex_array(Some(vertex_array));
            for (idx, buffer) in self.buffers.iter().enumerate() {
                if buffer.size < 1 || buffer.size > 4 {
                    warn!("invalid VertexArray size");
                    //    self.size = self.size.clamp(1, 4);
                }
                gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.buffer.buffer));
                gl.enable_vertex_attrib_array(idx as u32);
                gl.vertex_attrib_pointer_f32(
                    idx as u32,
                    buffer.size,
                    self.data_type,
                    self.normalized,
                    buffer.stride,
                    buffer.offset,
                );
                gl.vertex_attrib_divisor(idx as u32, buffer.divisor);
            }
            gl.bind_buffer(glow::ARRAY_BUFFER, None);
            gl.bind_vertex_array(None);
        }
        Ok(VertexArray { vertex_array })
    }
}
