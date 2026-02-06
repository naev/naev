use anyhow::Context as anyhow_context;
use anyhow::Result;
use encase::ShaderType;
use glow::HasContext;
use gltf::Gltf;
use nalgebra::{Matrix3, Matrix4, Point3, Vector3, Vector4};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_double, c_int, c_uint};
use std::path::Path;
use std::rc::Rc;

use nlog::warn;
use renderer::buffer::{
   Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray, VertexArrayBuffer,
   VertexArrayBuilder,
};
use renderer::shader::{ProgramBuilder, Shader};
use renderer::texture;
use renderer::texture::{
   Framebuffer, FramebufferBuilder, FramebufferTarget, Texture, TextureBuilder,
};
use renderer::{Context, ContextWrapper, Uniform, look_at4, ortho4};

const MAX_LIGHTS: usize = 7;
const SHADOWMAP_SIZE_LOW: usize = 128;
const SHADOWMAP_SIZE_HIGH: usize = 512;

fn tex_value(ctx: &ContextWrapper, name: Option<&str>, value: [u8; 3]) -> Result<Rc<Texture>> {
   let mut img = image::RgbImage::new(1, 1);
   img.put_pixel(0, 0, image::Rgb(value));
   Ok(Rc::new(
      TextureBuilder::new()
         .name(name)
         .width(Some(1))
         .height(Some(1))
         .srgb(false)
         .flipv(false)
         .image(&img.into())
         .build_wrap(ctx)?,
   ))
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Default, bytemuck::Pod, bytemuck::Zeroable)]
pub struct Vertex {
   pos: [f32; 3],
   nor: [f32; 3],
   tex0: [f32; 2],
   tex1: [f32; 2],
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct PrimitiveUniform {
   view: Matrix4<f32>,
   normal: Matrix3<f32>,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct MaterialUniform {
   diffuse_factor: Vector4<f32>,
   emissive_factor: Vector3<f32>,
   metallic_factor: f32,
   roughness_factor: f32,
   blend: i32,
   diffuse_texcoord: i32,
   metallic_texcoord: i32,
   emissive_texcoord: i32,
   normal_texcoord: i32,
   occlusion_texcoord: i32,
   has_normal: i32,
   normal_scale: f32,
}

impl MaterialUniform {
   pub fn new() -> Self {
      MaterialUniform {
         ..Default::default()
      }
   }
}

#[repr(C)]
#[derive(Debug, Copy, Default, Clone, ShaderType)]
pub struct LightUniform {
   shadow: Matrix4<f32>,
   position: Vector3<f32>,
   colour: Vector3<f32>,
   sun: i32,
}
impl LightUniform {
   fn update_shadow(&mut self) {
      if self.position.magnitude() <= 1e-8 {
         self.shadow = Matrix4::identity();
         return;
      }
      const CENTER: Vector3<f32> = Vector3::new(0.0, 0.0, 0.0);
      const UP: Vector3<f32> = Vector3::new(0.0, 1.0, 0.0);
      const R: f32 = 1.0;
      self.shadow = if self.sun != 0 {
         let pos = self.position.normalize();
         let l = look_at4(&pos, &CENTER, &UP);
         let o = ortho4(-R, R, -R, R, 0.0, 2.0);
         o * l
      } else {
         let pos = self.position;
         let l = look_at4(&pos, &CENTER, &UP);
         let norm = pos.magnitude();
         let o = ortho4(-R, R, -R, R, norm - 1.0, norm + 1.0);
         o * l
      };
   }
}
impl From<&naevc::Light> for LightUniform {
   fn from(light: &naevc::Light) -> Self {
      let intensity = light.intensity as f32;
      let mut l = LightUniform {
         shadow: Matrix4::identity(),
         sun: light.sun,
         position: {
            let v = Vector3::new(
               light.pos.v[0] as f32,
               light.pos.v[1] as f32,
               light.pos.v[2] as f32,
            );
            match light.sun {
               0 => v,
               _ => v.normalize(),
            }
         },
         colour: Vector3::new(
            light.colour.v[0] as f32 * intensity,
            light.colour.v[1] as f32 * intensity,
            light.colour.v[2] as f32 * intensity,
         ),
      };
      l.update_shadow();
      l
   }
}

#[repr(C)]
#[derive(Debug, Clone, ShaderType)]
pub struct LightingUniform {
   lights: [LightUniform; MAX_LIGHTS],
   ambient: Vector3<f32>,
   intensity: f32,
   nlights: i32,
}
impl From<&naevc::Lighting> for LightingUniform {
   fn from(lighting: &naevc::Lighting) -> Self {
      let lights: [LightUniform; 7] =
         core::array::from_fn::<_, MAX_LIGHTS, _>(|i| (&lighting.lights[i]).into());
      LightingUniform {
         intensity: lighting.intensity as f32,
         ambient: Vector3::new(
            lighting.ambient_r as f32,
            lighting.ambient_g as f32,
            lighting.ambient_b as f32,
         ),
         nlights: lighting.nlights,
         lights,
      }
   }
}
impl Default for LightingUniform {
   fn default() -> Self {
      let mut uniform = LightingUniform {
         intensity: 1.0,
         ambient: Vector3::new(0.0, 0.0, 0.0),
         nlights: 2,
         lights: [
            // Key Light
            // Endless Sky (point): power: 150, pos: -12.339, 10.559, -11.787
            // Sharky (directional) power: 5, direction: 10.75, -12.272, 7.463
            LightUniform {
               shadow: Matrix4::identity(),
               position: Vector3::new(-3.0, 2.75, -3.0),
               colour: Vector3::new(80.0, 80.0, 80.0),
               sun: 0,
            },
            // Fill Light
            // Endless Sky (directional) pomer: 1.5, direction: 9.772, 11.602, 6.988
            // Sharky (point): 2000., position: -12.339, 10.559, 11.787
            LightUniform {
               shadow: Matrix4::identity(),
               position: Vector3::new(10.0, 11.5, 7.0).normalize(),
               colour: Vector3::new(1.0, 1.0, 1.0),
               sun: 1,
            },
            LightUniform::default(),
            LightUniform::default(),
            LightUniform::default(),
            LightUniform::default(),
            LightUniform::default(),
         ],
      };
      for idx in 0..uniform.nlights as usize {
         uniform.lights[idx].update_shadow();
      }
      uniform
   }
}

pub struct ModelShader {
   shader: Shader,
   lighting_buffer: Buffer,
   use_ao: bool,
}
impl ModelShader {
   const U_LIGHTING: u32 = 0;
   const U_MATERIAL: u32 = 1;
   const U_PRIMITIVE: u32 = 2;

   pub fn new(ctx: &ContextWrapper) -> Result<Self> {
      let lctx = ctx.lock();
      let gl = &lctx.gl;
      let mut shaderbuilder = ProgramBuilder::new(Some("PBR Shader"))
         .vert_frag_file("material_pbr.vert", "material_pbr.frag")
         .sampler("baseColour_tex", 0)
         .sampler("metallic_tex", 1)
         .sampler("emissive_tex", 2)
         .sampler("normal_tex", 3)
         .uniform_buffer("Lighting", Self::U_LIGHTING)
         .uniform_buffer("Material", Self::U_MATERIAL)
         .uniform_buffer("Primitive", Self::U_PRIMITIVE);
      for i in 0..MAX_LIGHTS {
         let sname = format!("shadowmap_tex[{i}]");
         shaderbuilder = shaderbuilder.sampler(&sname, (5 + i) as i32);
      }
      let low_memory = unsafe { naevc::conf.low_memory != 0 };
      if low_memory {
         shaderbuilder = shaderbuilder.prepend("#define HAS_AO 0\n");
      } else {
         shaderbuilder = shaderbuilder.sampler("occlusion_tex", 4)
      }
      let shader = shaderbuilder.build(gl)?;

      let lighting_buffer = BufferBuilder::new(Some("PBR Lighting Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&LightingUniform::default().buffer()?)
         .build(gl)?;

      Ok(ModelShader {
         shader,
         lighting_buffer,
         use_ao: !low_memory,
      })
   }
}

#[repr(C)]
#[derive(Debug, Clone, Default, ShaderType)]
struct ShadowUniform {
   transform: Matrix4<f32>,
}

struct ShadowShader {
   shader: Shader,
   buffer: Buffer,
}

impl ShadowShader {
   const U_SHADOW: u32 = 0;

   pub fn new(ctx: &ContextWrapper) -> Result<Self> {
      let lctx = ctx.lock();
      let gl = &lctx.gl;
      let shader = ProgramBuilder::new(Some("Shadow Shader"))
         .uniform_buffer("shadow", Self::U_SHADOW)
         .wgsl_file("shadow.wgsl")
         .build(gl)?;

      let buffer = BufferBuilder::new(Some("Shadow Buffer"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&ShadowUniform::default().buffer()?)
         .build(gl)?;

      Ok(ShadowShader { shader, buffer })
   }
}

pub struct Material {
   uniform_buffer: Buffer,
   diffuse: Rc<Texture>,
   metallic: Rc<Texture>,
   emissive: Rc<Texture>,
   normalmap: Rc<Texture>,
   ambientocclusion: Rc<Texture>,
   blend: bool,
   double_sided: bool,
}

impl Material {
   pub fn from_gltf(
      ctx: &ContextWrapper,
      mat: &gltf::Material,
      tex_zeros: &Rc<Texture>,
      tex_ones: &Rc<Texture>,
      textures: &[Rc<Texture>],
   ) -> Result<Self> {
      let mut data = MaterialUniform::new();

      let pbr = mat.pbr_metallic_roughness();
      data.metallic_factor = pbr.metallic_factor();
      data.roughness_factor = pbr.roughness_factor();
      data.diffuse_factor = pbr.base_color_factor().into();
      data.blend = match mat.alpha_mode() {
         gltf::material::AlphaMode::Opaque => 0,
         gltf::material::AlphaMode::Mask => {
            anyhow::bail!("mask alpha not supported for materials yet")
         }
         gltf::material::AlphaMode::Blend => 1,
      };
      data.emissive_factor = mat.emissive_factor().into();
      if let Some(strength) = mat.emissive_strength() {
         data.emissive_factor *= strength;
      }

      let diffuse = match pbr.base_color_texture() {
         Some(info) => {
            data.diffuse_texcoord = info.tex_coord() as i32;
            textures[info.texture().index()].clone()
         }
         None => tex_ones.clone(),
      };
      let metallic = match pbr.metallic_roughness_texture() {
         Some(info) => {
            data.metallic_texcoord = info.tex_coord() as i32;
            textures[info.texture().index()].clone()
         }
         None => tex_ones.clone(),
      };
      let emissive = match mat.emissive_texture() {
         Some(info) => {
            data.emissive_texcoord = info.tex_coord() as i32;
            textures[info.texture().index()].clone()
         }
         None => tex_zeros.clone(),
      };
      let normalmap = match mat.normal_texture() {
         Some(info) => {
            data.normal_texcoord = info.tex_coord() as i32;
            data.normal_scale = info.scale();
            data.has_normal = 1;
            textures[info.texture().index()].clone()
         }
         None => tex_zeros.clone(),
      };
      // Don't load Ambient Occlusion if using low memory mode
      let ambientocclusion = {
         if unsafe { naevc::conf.low_memory != 0 } {
            tex_ones.clone()
         } else {
            match mat.occlusion_texture() {
               Some(info) => {
                  // TODO strength?
                  data.occlusion_texcoord = info.tex_coord() as i32;
                  textures[info.texture().index()].clone()
               }
               None => tex_ones.clone(),
            }
         }
      };

      let uniform_buffer = {
         let lctx = ctx.lock();
         let gl = &lctx.gl;
         BufferBuilder::new(mat.name())
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Static)
            .data(&data.buffer()?)
            .build(gl)?
      };

      Ok(Material {
         //uniform_data: data,
         uniform_buffer,
         diffuse,
         metallic,
         emissive,
         normalmap,
         ambientocclusion,
         blend: data.blend != 0,
         double_sided: mat.double_sided(),
      })
   }
}

pub struct Primitive {
   uniform_data: PrimitiveUniform,
   uniform_buffer: Buffer,
   topology: u32,
   vao: VertexArray,
   num_indices: i32,
   element_type: u32,
   material: Rc<Material>,
   vertex_data: Vec<Vertex>,
   #[allow(dead_code)]
   vertices: Buffer,
   #[allow(dead_code)]
   indices: Buffer,
}

impl Primitive {
   pub fn from_gltf(
      ctx: &ContextWrapper,
      prim: &gltf::Primitive,
      buffer_data: &[Vec<u8>],
      materials: &[Rc<Material>],
      material_default: &Rc<Material>,
   ) -> Result<Self> {
      let mut vertex_data: Vec<Vertex> = vec![];
      let reader = prim.reader(|buf| Some(&buffer_data[buf.index()]));

      if let Some(pos) = reader.read_positions() {
         pos.for_each(|p| {
            vertex_data.push(Vertex {
               pos: p,
               ..Default::default()
            })
         });
      } else {
         anyhow::bail!("No Position Data!");
      }
      if let Some(nor) = reader.read_normals() {
         nor.enumerate().for_each(|(i, n)| {
            vertex_data[i].nor = n;
         })
      };
      if let Some(tex) = reader.read_tex_coords(0).map(|v| v.into_f32()) {
         tex.enumerate().for_each(|(i, t)| {
            vertex_data[i].tex0 = t;
         })
      };
      if let Some(tex) = reader.read_tex_coords(1).map(|v| v.into_f32()) {
         tex.enumerate().for_each(|(i, t)| {
            vertex_data[i].tex1 = t;
         })
      };
      let mut index_data: Vec<u32> = vec![];
      if let Some(ind) = reader.read_indices() {
         index_data.append(&mut ind.into_u32().collect::<Vec<u32>>());
      }

      let material = match prim.material().index() {
         Some(idx) => materials[idx].clone(),
         None => material_default.clone(),
      };

      let topology = match prim.mode() {
         gltf::mesh::Mode::Points => glow::POINTS,
         gltf::mesh::Mode::Lines => glow::LINES,
         gltf::mesh::Mode::LineLoop => glow::LINE_LOOP,
         gltf::mesh::Mode::LineStrip => glow::LINE_STRIP,
         gltf::mesh::Mode::Triangles => glow::TRIANGLES,
         gltf::mesh::Mode::TriangleStrip => glow::TRIANGLE_STRIP,
         gltf::mesh::Mode::TriangleFan => glow::TRIANGLE_FAN,
      };

      let lctx = ctx.lock();
      let gl = &lctx.gl;
      let vertices = BufferBuilder::new(Some("Vertices"))
         .usage(BufferUsage::Static)
         .data(bytemuck::cast_slice(&vertex_data))
         .build(gl)?;
      let indices = BufferBuilder::new(Some("Indices"))
         .usage(BufferUsage::Static)
         .data(bytemuck::cast_slice(&index_data))
         .build(gl)?;
      let vertex_size = std::mem::size_of::<Vertex>() as i32;
      let vao = VertexArrayBuilder::new(Some("Vertex Array Object"))
         .buffers(&[
            VertexArrayBuffer {
               buffer: &vertices,
               size: 3,
               stride: vertex_size,
               offset: std::mem::offset_of!(Vertex, pos) as i32,
               divisor: 0,
            },
            VertexArrayBuffer {
               buffer: &vertices,
               size: 3,
               stride: vertex_size,
               offset: std::mem::offset_of!(Vertex, nor) as i32,
               divisor: 0,
            },
            VertexArrayBuffer {
               buffer: &vertices,
               size: 2,
               stride: vertex_size,
               offset: std::mem::offset_of!(Vertex, tex0) as i32,
               divisor: 0,
            },
            VertexArrayBuffer {
               buffer: &vertices,
               size: 2,
               stride: vertex_size,
               offset: std::mem::offset_of!(Vertex, tex1) as i32,
               divisor: 0,
            },
         ])
         .indices(Some(&indices))
         .build_gl(gl)?;
      let uniform_data = PrimitiveUniform::default();
      let uniform_buffer = BufferBuilder::new(Some("PrimitiveUniform"))
         .target(BufferTarget::Uniform)
         .usage(BufferUsage::Dynamic)
         .data(&uniform_data.buffer()?)
         .build(gl)?;

      Ok(Primitive {
         uniform_data,
         uniform_buffer,
         topology,
         vao,
         vertices,
         indices,
         element_type: glow::UNSIGNED_INT,
         num_indices: index_data.len() as i32,
         material,
         vertex_data,
      })
   }

   fn radius(&self, transform: &Matrix4<f32>) -> f32 {
      let mut radius: f32 = 0.0;
      for vertex in &self.vertex_data {
         //vertex.pos
         let pos = transform.transform_point(&Point3::from(vertex.pos));
         radius = radius.max(pos.coords.magnitude());
      }
      radius
   }
}

pub struct Mesh {
   primitives: Vec<Primitive>,
}

impl Mesh {
   pub fn new(_ctx: &ContextWrapper, primitives: Vec<Primitive>) -> Self {
      Mesh { primitives }
   }

   pub fn render(
      &self,
      ctx: &Context,
      shader: &ModelShader,
      transform: &Matrix4<f32>,
   ) -> Result<()> {
      let gl = &ctx.gl;
      for p in &self.primitives {
         // Update the primitive uniform
         let new_transform = *transform;
         let mut data = p.uniform_data;
         data.view = new_transform;
         data.normal = new_transform
            .fixed_resize::<3, 3>(0.0)
            .try_inverse()
            .unwrap()
            .transpose();
         p.uniform_buffer
            .bind_write_base(ctx, &data.buffer()?, ModelShader::U_PRIMITIVE)?;

         let m = &p.material;

         // Material Uniform should be as is
         m.uniform_buffer.bind_base(ctx, ModelShader::U_MATERIAL);

         // Textures
         m.metallic.bind(ctx, 1);
         m.emissive.bind(ctx, 2);
         m.normalmap.bind(ctx, 3);
         if shader.use_ao {
            m.ambientocclusion.bind(ctx, 4);
         }
         m.diffuse.bind(ctx, 0); // Have to end on TEXTURE0

         // Attributes
         p.vao.bind(ctx);

         // Render
         unsafe {
            if m.double_sided {
               gl.disable(glow::CULL_FACE);
            }
            if m.blend {
               gl.depth_mask(false);
            }

            gl.draw_elements(p.topology, p.num_indices, p.element_type, 0);

            if m.double_sided {
               gl.enable(glow::CULL_FACE);
            }
            if m.blend {
               gl.depth_mask(true);
            }
         }
      }
      Ok(())
   }

   fn render_shadow(
      &self,
      ctx: &Context,
      shader: &ShadowShader,
      transform: &Matrix4<f32>,
      shadow_transform: &Matrix4<f32>,
   ) -> Result<()> {
      let gl = &ctx.gl;
      for p in &self.primitives {
         // Assumption transparent (or pseudo transparent) don't cast shadows
         if p.material.blend {
            continue;
         }

         let data = ShadowUniform {
            transform: shadow_transform * transform,
         };
         shader
            .buffer
            .bind_write_base(ctx, &data.buffer()?, ShadowShader::U_SHADOW)?;
         let m = &p.material;

         // Attributes
         p.vao.bind(ctx);

         // Render
         unsafe {
            if m.double_sided {
               gl.disable(glow::CULL_FACE);
            }

            gl.draw_elements(p.topology, p.num_indices, p.element_type, 0);

            if m.double_sided {
               gl.enable(glow::CULL_FACE);
            }
         }
      }
      Ok(())
   }
}

#[derive(Clone)]
pub struct Trail {
   #[allow(dead_code)]
   generator: String,
   position: Vector3<f32>,
   // TODO remove
   cstr: CString,
}
impl Trail {
   fn from_json(json: &gltf::json::Value, transform: &Matrix4<f32>) -> Result<Self> {
      let generator = match json {
         gltf::json::Value::String(str) => str.to_string(),
         _ => {
            anyhow::bail!("invalid value")
         }
      };
      let v = transform * Vector4::new(0.0, 0.0, 0.0, 1.0);
      Ok(Self {
         cstr: CString::new(generator.as_str())?,
         generator,
         position: v.fixed_resize::<3, 1>(0.0),
      })
   }
}

#[derive(Clone)]
pub struct Mount {
   id: u32,
   position: Vector3<f32>,
}
impl Mount {
   fn from_json(json: &gltf::json::Value, transform: &Matrix4<f32>) -> Result<Self> {
      let id = match json {
         gltf::json::Value::String(str) => str.parse::<u32>()?,
         gltf::json::Value::Number(num) => num.as_u64().context("unable to convert to u64")? as u32,
         _ => {
            anyhow::bail!("invalid value")
         }
      };
      let v = transform * Vector4::new(0.0, 0.0, 0.0, 1.0);
      Ok(Self {
         id,
         position: v.fixed_resize::<3, 1>(0.0),
      })
   }
}

pub struct Node {
   transform: Matrix4<f32>,
   mesh: Option<Rc<Mesh>>,
   children: Vec<Node>,
   trail: Option<Trail>,
   mount: Option<Mount>,
}

impl Node {
   pub fn from_gltf(node: &gltf::Node, meshes: &[Rc<Mesh>]) -> Result<Self> {
      let transform: Matrix4<f32> = node.transform().matrix().into();
      let mesh = node.mesh().map(|mesh| meshes[mesh.index()].clone());

      let children: Vec<Node> = node
         .children()
         .map(|child| Node::from_gltf(&child, meshes))
         .collect::<Result<Vec<_>, _>>()?;

      let mut trail = None;
      let mut mount = None;
      if let Some(extras) = node.extras() {
         let json: gltf::json::Value = gltf::json::deserialize::from_str(extras.get())?;
         if let Some(value) = json.get("NAEV_trail_generator") {
            trail = match Trail::from_json(value, &transform) {
               Ok(data) => Some(data),
               Err(e) => {
                  warn!("{e}");
                  None
               }
            };
         }
         if let Some(value) = json.get("NAEV_weapon_mount") {
            mount = match Mount::from_json(value, &transform) {
               Ok(data) => Some(data),
               Err(e) => {
                  warn!("{e}");
                  None
               }
            };
         }
      }

      Ok(Node {
         transform,
         mesh,
         children,
         trail,
         mount,
      })
   }

   fn sort(&mut self) {
      for c in &mut self.children {
         c.sort();
      }
      self.children.sort_by_key(|a| a.blend());
   }

   fn blend(&self) -> bool {
      if let Some(m) = &self.mesh {
         for p in &m.primitives {
            if p.material.blend {
               return true;
            }
         }
      }
      for c in &self.children {
         if c.blend() {
            return true;
         }
      }
      false
   }

   pub fn radius(&self, transform: Matrix4<f32>) -> f32 {
      let mut radius: f32 = 0.0;
      let transform = transform * self.transform;
      if let Some(mesh) = &self.mesh {
         for primitive in &mesh.primitives {
            let rad = primitive.radius(&transform);
            radius = radius.max(rad);
         }
      }
      for child in &self.children {
         radius = radius.max(child.radius(transform));
      }
      radius
   }

   pub fn render(
      &mut self,
      ctx: &Context,
      shader: &ModelShader,
      transform: &Matrix4<f32>,
   ) -> Result<()> {
      let new_transform = transform * self.transform;
      // If determinant is negative, we have to invert the winding
      let det = new_transform.fixed_resize::<3, 3>(0.0).determinant();
      unsafe {
         ctx.gl
            .front_face(if det < 0.0 { glow::CW } else { glow::CCW });
      }
      // Draw the meshes
      if let Some(mesh) = &self.mesh {
         mesh.render(ctx, shader, &new_transform)?;
      }
      // Draw the children
      for child in &mut self.children {
         child.render(ctx, shader, &new_transform)?;
      }
      Ok(())
   }

   fn render_shadow(
      &mut self,
      ctx: &Context,
      shader: &ShadowShader,
      transform: &Matrix4<f32>,
      shadow_transform: &Matrix4<f32>,
   ) -> Result<()> {
      let new_transform = transform * self.transform;
      if let Some(mesh) = &self.mesh {
         mesh.render_shadow(ctx, shader, &new_transform, shadow_transform)?;
      }
      for child in &mut self.children {
         child.render_shadow(ctx, shader, &new_transform, shadow_transform)?;
      }
      Ok(())
   }
}

pub struct Scene {
   name: Option<String>,
   nodes: Vec<Node>,
   radius: f32,
}

impl Scene {
   fn from_gltf(scene: &gltf::Scene, meshes: &[Rc<Mesh>]) -> Result<Self> {
      let mut nodes: Vec<Node> = scene
         .nodes()
         .map(|node| Node::from_gltf(&node, meshes))
         .collect::<Result<Vec<_>, _>>()?;
      for n in &mut nodes {
         n.sort();
      }
      nodes.sort_by_key(|a| a.blend());
      let name = scene.name().map(|s| s.to_owned());

      let mut radius: f32 = 0.0;
      for node in &nodes {
         radius = radius.max(node.radius(Matrix4::identity()));
      }

      Ok(Scene {
         nodes,
         name,
         radius,
      })
   }

   fn render(
      &mut self,
      common: &Common,
      ctx: &Context,
      target: &FramebufferTarget,
      shader: &ModelShader,
      transform: &Matrix4<f32>,
      lighting: &LightingUniform,
   ) -> Result<()> {
      let gl = &ctx.gl;
      let (fbo_w, fbo_h) = target.dimensions();

      unsafe {
         gl.enable(glow::BLEND);
         gl.blend_func_separate(
            glow::SRC_ALPHA,
            glow::ONE_MINUS_SRC_ALPHA,
            glow::ONE,
            glow::ONE_MINUS_SRC_ALPHA,
         );
         gl.enable(glow::DEPTH_TEST);
         gl.depth_func(glow::LESS);
      }

      // Shadow pass
      let shadow_shader = &common.shader_shadow;
      let data = common.data.read().unwrap();
      for i in 0..(lighting.nlights as usize) {
         let (light_fbo, shadow_size, shadow_fbo) = {
            if fbo_w.max(fbo_h) > 255 {
               (
                  &common.light_fbo_high[i],
                  SHADOWMAP_SIZE_HIGH as i32,
                  &common.shadow_fbo_high,
               )
            } else {
               (
                  &common.light_fbo_low[i],
                  SHADOWMAP_SIZE_LOW as i32,
                  &common.shadow_fbo_low,
               )
            }
         };
         shadow_shader.shader.use_program(gl);
         let shadow_transform = &data.light_uniform.lights[i].shadow;
         light_fbo.bind(ctx);
         unsafe {
            gl.enable(glow::CULL_FACE);
            gl.clear(glow::DEPTH_BUFFER_BIT);
            gl.viewport(0, 0, shadow_size, shadow_size);
         }

         // Render depth buffer
         for node in &mut self.nodes {
            node.render_shadow(ctx, shadow_shader, transform, shadow_transform)?;
         }

         // Blur pass
         unsafe {
            gl.disable(glow::CULL_FACE);
         }
         fn blur_pass(ctx: &Context, buf_in: &Framebuffer, buf_out: &Framebuffer, shader: &Shader) {
            let gl = &ctx.gl;
            shader.use_program(gl);
            ctx.vao_square.bind(ctx);
            buf_out.bind(ctx);
            unsafe {
               gl.clear(glow::DEPTH_BUFFER_BIT);
            }
            if let Some(tex) = &buf_in.depth {
               tex.bind(ctx, 0);
               unsafe {
                  gl.draw_arrays(glow::TRIANGLE_STRIP, 0, 4);
               }
            }
         }
         blur_pass(ctx, light_fbo, shadow_fbo, &common.shader_blur_x);
         blur_pass(ctx, shadow_fbo, light_fbo, &common.shader_blur_y);

         // Bind the texture
         if let Some(tex) = &light_fbo.depth {
            tex.bind(ctx, 5 + i as u32);
         }
      }
      drop(data);

      // Update lighting
      shader.shader.use_program(gl);
      shader
         .lighting_buffer
         .bind_write_base(ctx, &lighting.buffer()?, ModelShader::U_LIGHTING)?;

      // Mesh pass
      unsafe {
         gl.viewport(0, 0, fbo_w as i32, fbo_h as i32);
         gl.cull_face(glow::FRONT);
         gl.enable(glow::CULL_FACE);
      }
      target.bind(ctx);
      for node in &mut self.nodes {
         node.render(ctx, shader, transform)?;
      }

      // Clean up
      VertexArray::unbind(ctx);
      unsafe {
         for idx in (0..5 + MAX_LIGHTS).rev() {
            let idx = idx as u32;
            gl.active_texture(glow::TEXTURE0 + idx);
            gl.bind_texture(glow::TEXTURE_2D, None);
            gl.bind_sampler(idx, None);
         }
         gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, None);
         gl.bind_buffer(glow::ARRAY_BUFFER, None);
         gl.use_program(None);
         gl.disable(glow::DEPTH_TEST);
         gl.disable(glow::CULL_FACE);
         gl.viewport(0, 0, naevc::gl_screen.rw, naevc::gl_screen.rh);
         target.unbind(ctx);
      }

      Ok(())
   }
}

pub struct Model {
   scenes: Vec<Scene>,
   transform_scale: Matrix4<f32>,
   common: &'static Common,
   trails: Vec<Trail>,
   mounts: Vec<Mount>,
   body: i32,
   engine: i32,
}

fn load_buffer(buf: &gltf::buffer::Buffer, base: &str) -> Result<Vec<u8>> {
   match buf.source() {
      gltf::buffer::Source::Uri(uri) => {
         let filename = format!("{base}/{uri}");
         Ok(ndata::read(&filename)?)
      }
      // TODO implement
      gltf::buffer::Source::Bin => anyhow::bail!("binary buffer data not supported yet"),
   }
}

fn load_gltf_texture(
   ctx: &ContextWrapper,
   node: &gltf::texture::Texture,
   base: &str,
   srgb: bool,
) -> Result<Texture> {
   let sampler = node.sampler();
   let mut tb = TextureBuilder::new()
      .name(node.name())
      .flipv(false)
      .srgb(srgb);

   tb = match node.source().source() {
      gltf::image::Source::Uri { uri, .. } => {
         let filename = format!("{base}/{uri}");
         tb.path(&filename)
      }
      // TODO implement
      _ => anyhow::bail!("unsupported image source!"),
   };

   if let Some(filter) = sampler.mag_filter() {
      tb = tb.mag_filter(match filter {
         gltf::texture::MagFilter::Linear => texture::FilterMode::Linear,
         gltf::texture::MagFilter::Nearest => texture::FilterMode::Nearest,
      });
   };
   if let Some(filter) = sampler.min_filter() {
      tb = tb.min_filter(match filter {
         // TODO fix
         gltf::texture::MinFilter::LinearMipmapLinear
         | gltf::texture::MinFilter::LinearMipmapNearest
         | gltf::texture::MinFilter::NearestMipmapLinear
         | gltf::texture::MinFilter::Linear => texture::FilterMode::Linear,
         gltf::texture::MinFilter::NearestMipmapNearest | gltf::texture::MinFilter::Nearest => {
            texture::FilterMode::Nearest
         }
      });
   };
   tb = tb.address_mode_u(match sampler.wrap_s() {
      gltf::texture::WrappingMode::ClampToEdge => texture::AddressMode::ClampToEdge,
      gltf::texture::WrappingMode::MirroredRepeat => texture::AddressMode::MirrorRepeat,
      gltf::texture::WrappingMode::Repeat => texture::AddressMode::Repeat,
   });
   tb = tb.address_mode_v(match sampler.wrap_t() {
      gltf::texture::WrappingMode::ClampToEdge => texture::AddressMode::ClampToEdge,
      gltf::texture::WrappingMode::MirroredRepeat => texture::AddressMode::MirrorRepeat,
      gltf::texture::WrappingMode::Repeat => texture::AddressMode::Repeat,
   });

   let tex = tb.build_wrap(ctx)?;

   // Downscale as necessary
   if unsafe { naevc::conf.low_memory != 0 } {
      let max_tex_size = unsafe { naevc::conf.max_3d_tex_size } as usize;
      if max_tex_size == 0 || max_tex_size >= tex.texture.w.max(tex.texture.h) {
         return Ok(tex);
      }
      tex.scale_wrap(ctx, max_tex_size, max_tex_size)
   } else {
      Ok(tex)
   }
}

// This is actually bad because they technically depend on the context existing, but if we are
// calling them with a dead context, chances are we are hosed anyways...
use std::sync::{OnceLock, RwLock};
struct Common {
   shader: ModelShader,
   shader_blur_x: Shader,
   shader_blur_y: Shader,
   shader_shadow: ShadowShader,
   light_fbo_low: [Framebuffer; MAX_LIGHTS],
   light_fbo_high: [Framebuffer; MAX_LIGHTS],
   shadow_fbo_low: Framebuffer,
   shadow_fbo_high: Framebuffer,
   data: RwLock<CommonMut>,
}
struct CommonMut {
   light_uniform: LightingUniform,
}
impl Common {
   fn new(ctx: &ContextWrapper) -> Result<Self> {
      let shader = ModelShader::new(ctx)?;
      let shader_shadow = ShadowShader::new(ctx)?;

      fn make_fbo_single(ctx: &ContextWrapper, name: &str, size: usize) -> Result<Framebuffer> {
         let fb = FramebufferBuilder::new(Some(name))
            .width(size)
            .height(size)
            .texture(false)
            .depth(true)
            .address_mode(texture::AddressMode::ClampToEdge)
            .build_wrap(ctx)?;
         let lctx = ctx.lock();
         let gl = &lctx.gl;
         fb.bind_gl(gl);
         unsafe {
            if let Some(depth) = &fb.depth {
               // Can't currently use FramebufferBuilder to set the following, so we do it
               // as post-processing
               let sampler = depth.sampler;
               gl.sampler_parameter_i32(
                  sampler,
                  glow::TEXTURE_WRAP_S,
                  glow::CLAMP_TO_BORDER as i32,
               );
               gl.sampler_parameter_i32(
                  sampler,
                  glow::TEXTURE_WRAP_T,
                  glow::CLAMP_TO_BORDER as i32,
               );
               gl.sampler_parameter_f32_slice(
                  sampler,
                  glow::TEXTURE_BORDER_COLOR,
                  &[1.0, 1.0, 1.0, 1.0],
               );
            };
            gl.read_buffer(glow::NONE);
            gl.draw_buffer(glow::NONE);
         }
         Framebuffer::unbind_gl(gl);
         Ok(fb)
      }

      fn make_fbos(
         ctx: &ContextWrapper,
         name: &str,
         size: usize,
      ) -> Result<[Framebuffer; MAX_LIGHTS]> {
         let v = (0..MAX_LIGHTS)
            .map(|i| {
               let name = &format!("{name}[{i}]");
               make_fbo_single(ctx, name, size)
            })
            .collect::<Result<Vec<_>>>()?;
         let boxed_slice = v.into_boxed_slice();
         use std::convert::TryInto;
         let boxed_array: Box<[Framebuffer; MAX_LIGHTS]> = match boxed_slice.try_into() {
            Ok(ba) => ba,
            Err(o) => anyhow::bail!(
               "Expected a Vec of length {}, but it was {}",
               MAX_LIGHTS,
               o.len()
            ),
         };
         Ok(*boxed_array)
      }

      let light_fbo_low = make_fbos(ctx, "light_fbo_low", SHADOWMAP_SIZE_LOW)?;
      let light_fbo_high = make_fbos(ctx, "light_fbo_high", SHADOWMAP_SIZE_HIGH)?;
      // Used for the shadow blurring
      let shadow_fbo_low = make_fbo_single(ctx, "shadow_fbo_low", SHADOWMAP_SIZE_LOW)?;
      let shadow_fbo_high = make_fbo_single(ctx, "shadow_fbo_high", SHADOWMAP_SIZE_HIGH)?;

      let data = RwLock::new({
         CommonMut {
            light_uniform: Default::default(),
         }
      });

      // Blur Shaders
      let lctx = ctx.lock();
      let gl = &lctx.gl;
      let shader_blur_x = ProgramBuilder::new(Some("Blur X Shader"))
         .sampler("sampler", 0)
         .vert_frag_file("blur.vert", "blurX.frag")
         .build(gl)?;
      let shader_blur_y = ProgramBuilder::new(Some("Blur Y Shader"))
         .sampler("sampler", 0)
         .vert_frag_file("blur.vert", "blurY.frag")
         .build(gl)?;

      Ok(Common {
         shader,
         shader_blur_x,
         shader_blur_y,
         shader_shadow,
         light_fbo_low,
         light_fbo_high,
         shadow_fbo_low,
         shadow_fbo_high,
         data,
      })
   }
}
static COMMON: OnceLock<Common> = OnceLock::new();

struct TextureWrap<'a> {
   texture: gltf::texture::Texture<'a>,
   srgb: bool,
}

impl Model {
   pub fn from_path<P: AsRef<Path>>(ctx: &ContextWrapper, path: P) -> Result<Self> {
      let path = path.as_ref();
      let gltf = Gltf::from_reader(ndata::open(path)?)?;
      let base = match path.parent() {
         Some(val) => val.to_str().unwrap(),
         None => "",
      };

      // Helper textures
      let tex_zeros = tex_value(ctx, Some("Black"), [0, 0, 0])?;
      let tex_ones = tex_value(ctx, Some("White"), [255, 255, 255])?;
      let material_default = Rc::new(Material {
         uniform_buffer: {
            let lctx = ctx.lock();
            BufferBuilder::new(Some("Default Material"))
               .target(BufferTarget::Uniform)
               .usage(BufferUsage::Static)
               .data(&MaterialUniform::default().buffer()?)
               .build(&lctx.gl)?
         },
         diffuse: tex_ones.clone(),
         metallic: tex_ones.clone(),
         emissive: tex_zeros.clone(),
         normalmap: tex_zeros.clone(),
         ambientocclusion: tex_ones.clone(),
         blend: false,
         double_sided: false,
      });

      let buffer_data: Vec<Vec<u8>> = gltf
         .buffers()
         .map(|buf| load_buffer(&buf, base))
         .collect::<Result<Vec<_>, _>>()?;

      let textures: Vec<Rc<Texture>> = {
         // GLTF doesn't tell us if the textures are SRGB or not, so we have to look ourselves.
         let mut texture_wraps: Vec<TextureWrap> = gltf
            .textures()
            .map(|texture| TextureWrap {
               texture,
               srgb: false,
            })
            .collect::<Vec<_>>();
         for m in gltf.materials() {
            // Diffuse and emissive textures should be srgb, rest should be linear
            if let Some(info) = m.pbr_metallic_roughness().base_color_texture() {
               texture_wraps[info.texture().index()].srgb = true;
            } else if let Some(info) = m.emissive_texture() {
               texture_wraps[info.texture().index()].srgb = true;
            }
         }
         texture_wraps
            .iter()
            .map(
               |tex| match load_gltf_texture(ctx, &tex.texture, base, tex.srgb) {
                  Ok(some) => Ok(Rc::new(some)),
                  Err(e) => Err(e),
               },
            )
            .collect::<Result<Vec<_>, _>>()?
      };

      let materials: Vec<Rc<Material>> = gltf
         .materials()
         .map(
            |mat| match Material::from_gltf(ctx, &mat, &tex_zeros, &tex_ones, &textures) {
               Ok(some) => Ok(Rc::new(some)),
               Err(e) => Err(e),
            },
         )
         .collect::<Result<Vec<_>, _>>()?;

      let meshes: Vec<Rc<Mesh>> = gltf
         .meshes()
         .map(|mesh| {
            let mut primitives = mesh
               .primitives()
               .map(|prim| {
                  Primitive::from_gltf(ctx, &prim, &buffer_data, &materials, &material_default)
               })
               .collect::<Result<Vec<_>, _>>()?;
            primitives.sort_by(|a, b| a.material.blend.cmp(&b.material.blend));
            Ok(Rc::new(Mesh::new(ctx, primitives)))
         })
         .collect::<Result<Vec<_>, anyhow::Error>>()?;

      let scenes: Vec<Scene> = gltf
         .scenes()
         .map(|scene| Scene::from_gltf(&scene, &meshes))
         .collect::<Result<Vec<_>, _>>()?;

      // Common stuff
      let common = COMMON.get_or_init(|| Common::new(ctx).unwrap());

      // Get model radius and scenes
      let mut body = 0;
      let mut engine = -1;
      let mut radius: f32 = 0.;
      for (idx, scene) in scenes.iter().enumerate() {
         radius = radius.max(scene.radius);
         if let Some(name) = &scene.name {
            match name.as_str() {
               "body" => {
                  body = idx as i32;
               }
               "engine" => {
                  engine = idx as i32;
               }
               _ => (),
            }
         }
      }
      let invradius = 1.0 / radius;
      #[rustfmt::skip]
        let transform_scale = Matrix4::<f32>::new(
            invradius, 0.0, 0.0, 0.0,
            0.0, invradius, 0.0, 0.0,
            0.0, 0.0,-invradius, 0.0,
            0.0, 0.0, 0.0, 1.0 );

      // Store Trails and Mounts
      let mut trails: Vec<Trail> = vec![];
      let mut mounts: Vec<Mount> = vec![];
      for s in &scenes {
         for n in &s.nodes {
            if let Some(trail) = &n.trail {
               let mut t = trail.clone();
               t.position *= invradius;
               trails.push(t);
            }
            if let Some(mount) = &n.mount {
               let mut m = mount.clone();
               m.position *= invradius;
               mounts.push(m);
            }
         }
      }
      trails.sort_by(|a, b| a.position.y.total_cmp(&b.position.y));
      mounts.sort_by(|a, b| a.id.cmp(&b.id));

      // Have to restore the core after loading
      let lctx = ctx.lock();
      unsafe {
         lctx.gl.bind_vertex_array(Some(lctx.vao_core));
      }

      Ok(Model {
         scenes,
         transform_scale,
         common,
         trails,
         mounts,
         body,
         engine,
      })
   }

   pub fn render_scene(
      &mut self,
      ctx: &Context,
      fb: &FramebufferTarget,
      sceneid: usize,
      lighting: &LightingUniform,
      transform: &Matrix4<f32>,
   ) -> Result<()> {
      let scene_transform = *transform * self.transform_scale;
      if let Some(scene) = self.scenes.get_mut(sceneid) {
         scene.render(
            self.common,
            ctx,
            fb,
            &self.common.shader,
            &scene_transform,
            lighting,
         )?;
      }
      Ok(())
   }

   pub fn into_ptr(self) -> *mut Model {
      Box::into_raw(Box::new(self))
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_init() -> c_int {
   let _ = COMMON.get_or_init(|| {
      let ctx = Context::get().as_wrap();
      Common::new(&ctx).unwrap()
   });
   0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_exit() {}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightReset() {
   let mut data = COMMON.get().unwrap().data.write().unwrap();
   data.light_uniform = Default::default();
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightSet(idx: c_int, light: *const naevc::Light) -> c_int {
   let n: usize = (2 + idx).try_into().unwrap();
   if n >= MAX_LIGHTS {
      warn!("Trying to set more lights than MAX_LIGHTS allows!");
      return -1;
   }
   let mut data = COMMON.get().unwrap().data.write().unwrap();
   let light = unsafe { &*light };
   data.light_uniform.nlights = data.light_uniform.nlights.max((n + 1) as i32);
   data.light_uniform.lights[n] = light.into();
   0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightAmbient(r: c_double, g: c_double, b: c_double) {
   const FACTOR: f32 = 1.0 / std::f32::consts::PI;
   let mut data = COMMON.get().unwrap().data.write().unwrap();
   data.light_uniform.ambient =
      Vector3::new(r as f32 * FACTOR, g as f32 * FACTOR, b as f32 * FACTOR);
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightAmbientGet(r: *mut c_double, g: *mut c_double, b: *mut c_double) {
   let data = COMMON.get().unwrap().data.read().unwrap();
   let amb = data.light_uniform.ambient;
   const FACTOR: f64 = std::f64::consts::PI;
   unsafe {
      *r = amb.x as f64 * FACTOR;
      *g = amb.y as f64 * FACTOR;
      *b = amb.z as f64 * FACTOR;
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightIntensity(strength: c_double) {
   let mut data = COMMON.get().unwrap().data.write().unwrap();
   data.light_uniform.intensity *= strength as f32;
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightIntensityGet() -> c_double {
   let data = COMMON.get().unwrap().data.read().unwrap();
   data.light_uniform.intensity as f64
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightTransform(
   _lighting: *mut naevc::Lighting,
   transform: *const Matrix4<f32>,
) {
   let transform = unsafe { &*transform };
   let mut data = COMMON.get().unwrap().data.write().unwrap();
   let lighting = &mut data.light_uniform;
   for i in 0..lighting.nlights as usize {
      let l = &mut lighting.lights[i];
      if l.sun != 0 {
         l.position = transform.transform_vector(&l.position);
      } else {
         l.position =
            Vector3::from_homogeneous(transform.transform_point(&Point3::from(l.position)).into())
               .unwrap();
      }
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_sceneBody(obj: *mut Model) -> c_int {
   let model = unsafe { &*obj };
   model.body as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_sceneEngine(obj: *mut Model) -> c_int {
   let model = unsafe { &*obj };
   model.engine as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_numAnimations(_obj: *mut Model) -> c_uint {
   // TODO
   0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_loadFromFile(cpath: *const c_char) -> *const Model {
   let path = unsafe { CStr::from_ptr(cpath) };
   let ctx = Context::get().as_wrap();
   let model = Model::from_path(&ctx, path.to_str().unwrap()).unwrap();
   model.into_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_free(model: *mut Model) {
   if !model.is_null() {
      let _ = unsafe { Box::from_raw(model) }; // should drop
   }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_render(
   fb: naevc::GLuint,
   model: *mut Model,
   ctransform: *const Matrix4<f32>,
   _time: f32,
   size: f64,
) {
   let model = unsafe { &mut *model };
   let transform = match ctransform.is_null() {
      true => &Matrix4::identity(),
      false => unsafe { &*ctransform },
   };
   let ctx = Context::get(); /* Lock early. */
   let data = COMMON.get().unwrap().data.read().unwrap();
   let lighting = &data.light_uniform;
   let _ = model.render_scene(
      ctx,
      &FramebufferTarget::from_gl(fb, size as usize, size as usize),
      0,
      lighting,
      transform,
   );
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_renderScene(
   fb: naevc::GLuint,
   model: *mut Model,
   scene: c_int,
   ctransform: *const Matrix4<f32>,
   _time: f32,
   size: f64,
   clighting: *const naevc::Lighting,
) {
   // TODO animations
   let model = unsafe { &mut *model };
   let transform = match ctransform.is_null() {
      true => &Matrix4::identity(),
      false => unsafe { &*ctransform },
   };
   let ctx = Context::get(); /* Lock early. */
   let data = COMMON.get().unwrap().data.read().unwrap();
   let lighting = match clighting.is_null() {
      true => &data.light_uniform,
      false => &(unsafe { &*clighting }).into(),
   };
   let _ = model.render_scene(
      ctx,
      &FramebufferTarget::from_gl(fb, size as usize, size as usize),
      scene as usize,
      lighting,
      transform,
   );
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_numLights() -> c_int {
   let data = COMMON.get().unwrap().data.read().unwrap();
   data.light_uniform.nlights as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_numTrails(obj: *const Model) -> c_uint {
   let model = unsafe { &*obj };
   model.trails.len() as c_uint
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_trailName(obj: *const Model, id: c_int) -> *const c_char {
   let model = unsafe { &*obj };
   let g = &model.trails[id as usize].cstr;
   g.as_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_trailPosition(obj: *const Model, id: c_int) -> Vector3<f64> {
   let model = unsafe { &*obj };
   model.trails[id as usize].position.cast::<f64>()
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_numMounts(obj: *const Model) -> c_uint {
   let model = unsafe { &*obj };
   model.mounts.len() as c_uint
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_mountIndex(obj: *const Model, id: c_int) -> c_uint {
   let model = unsafe { &*obj };
   model.mounts[id as usize].id as c_uint
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_mountPosition(obj: *const Model, id: c_int) -> Vector3<f64> {
   let model = unsafe { &*obj };
   model.mounts[id as usize].position.cast::<f64>()
}
