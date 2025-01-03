use anyhow::Result;
use glow::*;

use crate::ngl::{Context, CONTEXT};

pub struct Buffer {
    buffer: glow::Buffer,
    datalen: usize, // in u8
    usage: BufferUsage,
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

pub struct BufferBuilder<'a> {
    usage: BufferUsage,
    data: &'a [u8],
}
impl<'a> BufferBuilder<'a> {
    pub fn new() -> Self {
        BufferBuilder {
            usage: BufferUsage::Stream,
            data: Default::default(),
        }
    }

    pub fn usage(mut self, usage: BufferUsage) -> Self {
        self.usage = usage;
        self
    }

    pub fn data(mut self, data: &'a [u8]) -> Self {
        self.data = data;
        self
    }

    pub fn data_f32(mut self, data: &'a [f32]) -> Self {
        self.data(unsafe {
            std::slice::from_raw_parts(
                data.as_ptr() as *const u8,
                data.len() * std::mem::size_of::<f32>(),
            )
        })
    }

    pub fn build(self, ctx: &Context) -> Result<Buffer> {
        let gl = &ctx.gl;

        if self.data.is_empty() {
            anyhow::bail!("BufferBuilder has no data");
        }

        let buffer = unsafe { gl.create_buffer().map_err(|e| anyhow::anyhow!(e))? };

        unsafe {
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer));
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, self.data, self.usage.to_gl());
            gl.bind_buffer(glow::ARRAY_BUFFER, None);
        }

        Ok(Buffer {
            buffer,
            datalen: self.data.len(),
            usage: self.usage,
        })
    }
}
