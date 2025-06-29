#![allow(dead_code)]
use anyhow::Result;
use glow::*;

use crate::context;
use crate::context::Context;
use crate::{gettext, warn};

pub struct Buffer {
    pub buffer: glow::Buffer,
    datalen: usize, // in u8
    target: u32,
    usage: u32,
}
impl Buffer {
    pub fn write(&self, ctx: &Context, data: &[u8]) -> Result<()> {
        #[cfg(debug_assertions)]
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
    /// Simplification for write + binding
    pub fn bind_write_base(&self, ctx: &context::Context, data: &[u8], idx: u32) -> Result<()> {
        #[cfg(debug_assertions)]
        if data.len() != self.datalen {
            anyhow::bail!("buffer data length mismatch!");
        }
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, Some(self.buffer));
            gl.buffer_data_u8_slice(self.target, data, self.usage);
            gl.bind_buffer_base(glow::UNIFORM_BUFFER, idx, Some(self.buffer));
        }
        Ok(())
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
        context::MESSAGE_QUEUE
            .lock()
            .unwrap()
            .push(context::Message::DeleteBuffer(self.buffer));
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
    name: Option<String>,
    target: BufferTarget,
    usage: BufferUsage,
    data: &'a [u8],
}
impl<'a> BufferBuilder<'a> {
    pub fn new(name: Option<&str>) -> Self {
        BufferBuilder {
            name: name.map(String::from),
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

    pub fn name(mut self, name: Option<&str>) -> Self {
        self.name = name.map(String::from);
        self
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
            gl.object_label(glow::BUFFER, buffer.0.into(), self.name);
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

// Thin wrapper around vertex arrays, which are containers for the vertex buffer + index buffer
// state
pub struct VertexArray {
    pub vertex_array: glow::VertexArray,
}
impl Drop for VertexArray {
    fn drop(&mut self) {
        context::MESSAGE_QUEUE
            .lock()
            .unwrap()
            .push(context::Message::DeleteVertexArray(self.vertex_array));
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
            ctx.gl.bind_vertex_array(Some(ctx.vao_core));
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
    name: Option<String>,
    data_type: u32, // glow::FLOAT and such
    normalized: bool,
    buffers: &'a [VertexArrayBuffer<'a>],
    indices: Option<&'a Buffer>,
}
impl<'a> VertexArrayBuilder<'a> {
    pub fn new(name: Option<&str>) -> Self {
        VertexArrayBuilder {
            name: name.map(String::from),
            data_type: glow::FLOAT,
            normalized: false,
            buffers: &[],
            indices: None,
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

    pub fn name(mut self, name: Option<&str>) -> Self {
        self.name = name.map(String::from);
        self
    }

    pub fn indices(mut self, indices: Option<&'a Buffer>) -> Self {
        self.indices = indices;
        self
    }

    pub fn build(self, ctx: &context::Context) -> Result<VertexArray> {
        let vao = self.build_gl(&ctx.gl)?;
        unsafe {
            ctx.gl.bind_vertex_array(Some(ctx.vao_core)); // Unbind everything AFTER the VAO
        }
        Ok(vao)
    }

    pub fn build_gl(self, gl: &glow::Context) -> Result<VertexArray> {
        let vertex_array = unsafe { gl.create_vertex_array().map_err(|e| anyhow::anyhow!(e))? };
        unsafe {
            gl.bind_vertex_array(Some(vertex_array));
            gl.object_label(glow::VERTEX_ARRAY, vertex_array.0.into(), self.name);

            // Bind Vertex Buffers
            for (idx, buffer) in self.buffers.iter().enumerate() {
                let idx = idx as u32;
                if buffer.size < 1 || buffer.size > 4 {
                    warn!("invalid VertexArray size");
                    //    self.size = self.size.clamp(1, 4);
                }
                gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer.buffer.buffer));
                gl.enable_vertex_attrib_array(idx);
                gl.vertex_attrib_pointer_f32(
                    idx,
                    buffer.size,
                    self.data_type,
                    self.normalized,
                    buffer.stride,
                    buffer.offset,
                );
                gl.vertex_attrib_divisor(idx, buffer.divisor);
            }

            // Bind Index Buffer
            if let Some(indices) = self.indices {
                gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(indices.buffer));
            }

            // Clean up
            gl.bind_vertex_array(None); // Unbind everything AFTER the VAO
            gl.bind_buffer(glow::ARRAY_BUFFER, None);
            gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, None);
        }
        Ok(VertexArray { vertex_array })
    }
}
