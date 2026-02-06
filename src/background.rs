use anyhow::Result;
use nalgebra::Vector3;
use rand::Rng;
use renderer::Context;
use renderer::buffer::Buffer;

const STAR_BUF: usize = 250;

pub struct Dust {
   data: Buffer,
   x: f32,
   y: f32,
}
impl Dust {
   pub fn new(n: usize) -> Result<Self> {
      let (view_w, view_h) = {
         let dims = CONTEXT.read().unwrap().dimensions.read().unwrap();
         (dims.view_width, dims.view_height)
      };
      let zoom_far = unsafe { naevc::conf.zoom_far };
      let size = n * (view_w * view_h + STAR_BUF * STAR_BUF / (zoom_far * zoom_far));
      let w = (view_w + 2 * STAR_BUF) / zoom_far;
      let h = (view_h + 2 * STAR_BUF) / zoom_far;

      let ndust = size / (800 * 600);

      let dust_vertex = Vec::<Vector3<f32>>::new().resize_with(|| x.random::<f32>());
   }
}
