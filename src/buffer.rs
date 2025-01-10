#![allow(dead_code)]
use anyhow::Result;
use glow::*;

use crate::ngl;
use crate::ngl::{Context, CONTEXT};
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
    pub fn bind(&self, ctx: &ngl::Context) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, Some(self.buffer));
        }
    }
    pub fn unbind(&self, ctx: &ngl::Context) {
        let gl = &ctx.gl;
        unsafe {
            gl.bind_buffer(self.target, None);
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

    pub fn build(self, ctx: &Context) -> Result<Buffer> {
        let gl = &ctx.gl;

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
            target: target,
            usage: usage,
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

pub struct VertexArrayBuilder {
    index: u32,
    size: i32,      // in data_type units (1 to 4)
    data_type: u32, // glow::FLOAT and such
    normalized: bool,
    stride: i32, // in bytes
    offset: i32, //
}
impl VertexArrayBuilder {
    pub fn new(self) -> Self {
        VertexArrayBuilder {
            index: 0,
            size: 4,
            data_type: glow::FLOAT,
            normalized: false,
            stride: 0,
            offset: 0,
        }
    }

    pub fn index(mut self, index: u32) -> Self {
        self.index = index;
        self
    }

    pub fn size(mut self, size: i32) -> Self {
        self.size = size;
        self
    }

    pub fn data_type(mut self, data_type: u32) -> Self {
        self.data_type = data_type;
        self
    }

    pub fn normalized(mut self, normalized: bool) -> Self {
        self.normalized = normalized;
        self
    }

    pub fn stride(mut self, stride: i32) -> Self {
        self.stride = stride;
        self
    }

    pub fn build(mut self, ctx: &Context) -> Result<VertexArray> {
        let gl = &ctx.gl;
        if self.size < 1 || self.size > 4 {
            warn!("invalid VertexArray size");
            self.size = self.size.clamp(1, 4);
        }
        let vertex_array = unsafe { gl.create_vertex_array().map_err(|e| anyhow::anyhow!(e))? };
        unsafe {
            gl.vertex_attrib_pointer_f32(
                self.index,
                self.size,
                self.data_type,
                self.normalized,
                self.stride,
                self.offset,
            );
        }

        Ok(VertexArray { vertex_array })
    }
}
