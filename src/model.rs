#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use gltf::Gltf;
use nalgebra::{Matrix3, Matrix4, Vector3, Vector4};
use std::rc::Rc;

use crate::buffer::{Buffer, BufferBuilder, BufferUsage};
use crate::ngl::{Context, CONTEXT};
use crate::shader::{Shader, ShaderBuilder};
use crate::texture;
use crate::texture::{Texture, TextureBuilder};

const MAX_LIGHTS: usize = 7;

fn tex_value(ctx: &Context, value: [u8; 3]) -> Result<Rc<Texture>> {
    let mut img = image::RgbImage::new(1, 1);
    img.put_pixel(0, 0, image::Rgb(value));
    Ok(Rc::new(
        TextureBuilder::new()
            .width(1)
            .height(1)
            .image(&img.into())
            .build(ctx)?,
    ))
}
fn tex_zeros(ctx: &Context) -> Result<Rc<Texture>> {
    tex_value(ctx, [0, 0, 0])
}
fn tex_ones(ctx: &Context) -> Result<Rc<Texture>> {
    tex_value(ctx, [255, 255, 255])
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
    norm: Matrix3<f32>,
}

impl PrimitiveUniform {
    pub fn new() -> Self {
        PrimitiveUniform {
            ..Default::default()
        }
    }
    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct MaterialUniform {
    diffuse_factor: Vector4<f32>,
    emissive_factor: Vector3<f32>,
    metallic_factor: f32,
    roughness_factor: f32,
    blend: i32,
    diffuse_texcoord: u32,
    normal_texcoord: u32,
    has_normal: u32,
}

impl MaterialUniform {
    pub fn new() -> Self {
        MaterialUniform {
            ..Default::default()
        }
    }
    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default, ShaderType)]
pub struct LightUniform {
    position: Vector3<f32>,
    colour: Vector3<f32>,
    intensity: f32,
    sun: u32,
}

#[repr(C)]
#[derive(Debug, Clone, Default, ShaderType)]
pub struct LightingUniform {
    ambient: Vector3<f32>,
    intensity: f32,
    nlights: u32,
    //#[size(runtime)]
    //lights: Vec<LightUniform>,
    lights: [LightUniform; MAX_LIGHTS],
}

impl LightingUniform {
    pub fn new() -> Self {
        LightingUniform {
            ..Default::default()
        }
    }

    pub fn default() -> Self {
        LightingUniform {
            ambient: Vector3::new(0.0, 0.0, 0.0),
            intensity: 1.0,
            nlights: 2,
            lights: [
                // Key Light
                LightUniform {
                    position: Vector3::new(-3.0, 2.75, -3.0),
                    colour: Vector3::new(1.0, 1.0, 1.0),
                    intensity: 80.0,
                    sun: 0,
                },
                // Fill Light
                LightUniform {
                    position: Vector3::new(10.0, 11.5, 7.0),
                    colour: Vector3::new(1.0, 1.0, 1.0),
                    intensity: 1.0,
                    sun: 1,
                },
                LightUniform::default(),
                LightUniform::default(),
                LightUniform::default(),
                LightUniform::default(),
                LightUniform::default(),
            ],
        }
    }

    pub fn buffer(&self) -> Result<encase::UniformBuffer<Vec<u8>>> {
        let mut buffer =
            encase::UniformBuffer::new(Vec::<u8>::with_capacity(Self::SHADER_SIZE.get() as usize));
        //let mut buffer =
        //    encase::UniformBuffer::new(Vec::<u8>::with_capacity(self.size().get() as usize));
        buffer.write(self)?;
        Ok(buffer)
    }
}

pub struct GltfShader {
    shader: Shader,
}
impl GltfShader {
    pub fn new(ctx: &Context) -> Result<Self> {
        let shader = ShaderBuilder::new(Some("PBR Shader"))
            .vert_file("gltf.vert")
            .frag_file("gltf_pbr.frag")
            .build(ctx)?;

        Ok(GltfShader { shader })
    }
}

pub struct Material {
    data: MaterialUniform,
    //shader: Rc<Shader>,
    diffuse: Rc<Texture>,
}

impl Material {
    pub fn from_gltf(
        ctx: &Context,
        mat: &gltf::Material,
        textures: &[Rc<Texture>],
    ) -> Result<Self> {
        //let shader = GltfShader::new(ctx);
        let mut data = MaterialUniform::new();

        let pbr = mat.pbr_metallic_roughness();
        data.metallic_factor = pbr.metallic_factor();
        data.roughness_factor = pbr.roughness_factor();
        data.diffuse_factor = pbr.base_color_factor().into();
        data.blend = match mat.alpha_mode() {
            gltf::material::AlphaMode::Opaque => 0,
            gltf::material::AlphaMode::Mask => todo!(),
            gltf::material::AlphaMode::Blend => 1,
        };
        data.emissive_factor = mat.emissive_factor().into();

        let diffuse = match pbr.base_color_texture() {
            Some(info) => {
                data.diffuse_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones(ctx)?,
        };

        let data_raw = data.buffer()?;

        Ok(Material {
            data,
            //shader,
            diffuse,
        })
    }
}

pub struct Primitive {
    uniform: PrimitiveUniform,
    vertices: Buffer,
    indices: Buffer,
    num_indices: u32,
    material: Rc<Material>,
}

impl Primitive {
    pub fn from_gltf(
        ctx: &Context,
        prim: &gltf::Primitive,
        buffer_data: &Vec<Vec<u8>>,
        materials: &[Rc<Material>],
    ) -> Result<Self> {
        let mut vertex_data: Vec<Vertex> = vec![];
        let reader = prim.reader(|buf| Some(&buffer_data[buf.index()]));

        if let Some(pos) = reader.read_positions() {
            pos.for_each(|p| {
                vertex_data.push(Vertex {
                    pos: p,
                    ..Default::default()
                })
            })
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
            None => todo!(),
        };

        //let buffer_data = uniform.buffer()?;

        let topology = match prim.mode() {
            gltf::mesh::Mode::Points => glow::POINTS,
            gltf::mesh::Mode::Lines => glow::LINES,
            gltf::mesh::Mode::LineLoop => glow::LINE_LOOP,
            gltf::mesh::Mode::LineStrip => glow::LINE_STRIP,
            gltf::mesh::Mode::Triangles => glow::TRIANGLES,
            gltf::mesh::Mode::TriangleStrip => glow::TRIANGLE_STRIP,
            gltf::mesh::Mode::TriangleFan => glow::TRIANGLE_FAN,
        };

        let vertices = BufferBuilder::new()
            .usage(BufferUsage::Static)
            .data(bytemuck::cast_slice(&vertex_data))
            .build(ctx)?;
        let indices = BufferBuilder::new()
            .usage(BufferUsage::Static)
            .data(bytemuck::cast_slice(&index_data))
            .build(ctx)?;

        Ok(Primitive {
            uniform: Default::default(),
            topology,
            vertices,
            indices,
            num_indices: index_data.len() as u32,
            material,
        })
    }
}

pub struct Mesh {
    primitives: Vec<Primitive>,
}

impl Mesh {
    pub fn new(_ctx: &Context, primitives: Vec<Primitive>) -> Self {
        Mesh { primitives }
    }

    pub fn render(&self, transform: &Matrix4<f32>, _ctx: &Context) {
        #[rustfmt::skip]
        const OPENGL_TO_WGPU: Matrix4<f32> = Matrix4::new(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.5,
            0.0, 0.0, 0.0, 1.0, );
        #[rustfmt::skip]
        #[allow(non_snake_case)]
        let VIEW: Matrix4<f32> = {
            const VIEW_ANGLE: f32 = -std::f32::consts::FRAC_PI_4;
            // TODO const cos/sin are not stabilized yet
            let VIEWCOS: f32 = VIEW_ANGLE.cos();
            let VIEWSIN: f32 = VIEW_ANGLE.sin();
            Matrix4::new(
                1.0,    0.0,     0.0,   0.0,
                0.0,  VIEWCOS, VIEWSIN, 0.0,
                0.0, -VIEWSIN, VIEWCOS, 0.0,
                0.0,    0.0,     0.0,   1.0 )
        };

        for p in &self.primitives {
            let new_transform = VIEW * transform;
            let mut data = p.uniform;
            data.view = OPENGL_TO_WGPU * new_transform;
            data.norm = new_transform
                .fixed_resize::<3, 3>(0.0)
                .try_inverse()
                .unwrap()
                .transpose();

            /*
            match data.buffer() {
                Ok(buffer) => {
                    renderer
                        .queue
                        .write_buffer(&p.buffer, 0, buffer.into_inner().as_slice());
                }
                Err(_) => (),
            }

            render_pass.set_pipeline(&p.render_pipeline);
            render_pass.set_bind_group(0, lighting_bind, &[]);
            render_pass.set_bind_group(1, &p.material.bind, &[]);
            render_pass.set_bind_group(2, &p.bind, &[]);
            render_pass.set_vertex_buffer(0, p.vertices.slice(..));
            render_pass.set_index_buffer(p.indices.slice(..), wgpu::IndexFormat::Uint32);
            render_pass.draw_indexed(0..p.num_indices, 0, 0..1);
            */
        }
    }
}

pub struct Node {
    transform: Matrix4<f32>,
    mesh: Option<Rc<Mesh>>,
    children: Vec<Node>,
}

impl Node {
    pub fn from_gltf(node: &gltf::Node, meshes: &[Rc<Mesh>]) -> Result<Self> {
        let transform: Matrix4<f32> = node.transform().matrix().into();

        let mesh = match node.mesh() {
            Some(mesh) => Some(meshes[mesh.index()].clone()),
            None => None,
        };

        let children: Vec<Node> = node
            .children()
            .map(|child| Node::from_gltf(&child, meshes))
            .collect::<Result<Vec<_>, _>>()?;

        Ok(Node {
            transform,
            mesh,
            children,
        })
    }

    pub fn render(&mut self, transform: &Matrix4<f32>, ctx: &Context) {
        let new_transform = self.transform * transform;
        if let Some(mesh) = &self.mesh {
            mesh.render(&new_transform, ctx);
        }
        for child in &mut self.children {
            child.render(&new_transform, ctx);
        }
    }
}

pub struct Scene {
    nodes: Vec<Node>,
}

impl Scene {
    pub fn from_gltf(scene: &gltf::Scene, meshes: &[Rc<Mesh>]) -> Result<Self> {
        let nodes: Vec<Node> = scene
            .nodes()
            .map(|node| Node::from_gltf(&node, meshes))
            .collect::<Result<Vec<_>, _>>()?;

        Ok(Scene { nodes })
    }

    pub fn render(&mut self, transform: &Matrix4<f32>, ctx: &Context) {
        for node in &mut self.nodes {
            node.render(transform, ctx);
        }
    }
}

pub struct Model {
    scenes: Vec<Scene>,
    //lighting_buffer: Buffer,
}

fn load_buffer(buf: &gltf::buffer::Buffer, base: &std::path::Path) -> Result<Vec<u8>> {
    match buf.source() {
        gltf::buffer::Source::Uri(uri) => {
            let filename = base.join(uri);
            Ok(std::fs::read(filename.as_path().to_str().unwrap())?)
        }
        gltf::buffer::Source::Bin => todo!(),
    }
}

pub fn load_gltf_texture(
    ctx: &Context,
    node: &gltf::texture::Texture,
    base: &std::path::Path,
) -> Result<Texture> {
    let gl = &ctx.gl;
    let sampler = node.sampler();
    let mut tb = TextureBuilder::new();

    tb = match node.source().source() {
        gltf::image::Source::Uri { uri, .. } => {
            let filename = base.join(uri);
            tb.path(filename.as_path().to_str().unwrap())
        }
        _ => todo!(),
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

    tb.build(ctx)
}

impl Model {
    pub fn from_path(ctx: &Context, path: &str) -> Result<Self> {
        use std::path::Path;
        let gltf = Gltf::open(path)?;
        let base = Path::new(path).parent().unwrap();

        let buffer_data: Vec<Vec<u8>> = gltf
            .buffers()
            .map(|buf| load_buffer(&buf, base))
            .collect::<Result<Vec<_>, _>>()?;

        let textures: Vec<Rc<Texture>> = gltf
            .textures()
            .map(|tex| match load_gltf_texture(ctx, &tex, base) {
                Ok(some) => Ok(Rc::new(some)),
                Err(e) => Err(e),
            })
            .collect::<Result<Vec<_>, _>>()?;

        let materials: Vec<Rc<Material>> = gltf
            .materials()
            .map(|mat| match Material::from_gltf(ctx, &mat, &textures) {
                Ok(some) => Ok(Rc::new(some)),
                Err(e) => Err(e),
            })
            .collect::<Result<Vec<_>, _>>()?;

        let meshes: Vec<Rc<Mesh>> = gltf
            .meshes()
            .map(|mesh| {
                let primitives = mesh
                    .primitives()
                    .map(|prim| Primitive::from_gltf(ctx, &prim, &buffer_data, &materials))
                    .collect::<Result<Vec<_>, _>>();
                match primitives {
                    Ok(primitives) => Ok(Rc::new(Mesh::new(ctx, primitives))),
                    Err(e) => Err(e),
                }
            })
            .collect::<Result<Vec<_>, _>>()?;

        let scenes: Vec<Scene> = gltf
            .scenes()
            .map(|scene| Scene::from_gltf(&scene, &meshes))
            .collect::<Result<Vec<_>, _>>()?;

        Ok(Model { scenes })
    }

    pub fn render(&mut self, transform: &Matrix4<f32>, ctx: &Context) {
        for s in &mut self.scenes {
            s.render(transform, ctx);
        }
    }
}
