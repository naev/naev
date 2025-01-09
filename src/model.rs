#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::HasContext;
use gltf::Gltf;
use nalgebra::{Matrix3, Matrix4, Vector3, Vector4};
use std::ffi::CStr;
use std::os::raw::{c_char, c_int};
use std::rc::Rc;

use crate::buffer::{Buffer, BufferBuilder, BufferTarget, BufferUsage};
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
    normal: Matrix3<f32>,
    shadow: [Matrix4<f32>; MAX_LIGHTS],
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
    metallic_texcoord: u32,
    emissive_texcoord: u32,
    normal_texcoord: u32,
    occlusion_texcoord: u32,
    has_normal: u32,
    normal_scale: f32,
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
    sun: u32,
    position: Vector3<f32>,
    colour: Vector3<f32>,
    intensity: f32,
}

#[repr(C)]
#[derive(Debug, Clone, Default, ShaderType)]
pub struct LightingUniform {
    ambient: Vector3<f32>,
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

pub struct ModelShader {
    shader: Shader,
    lighting_buffer: Buffer,
    vertex: u32,
    primitive_uniform: u32,
    material_uniform: u32,
    lighting_uniform: u32,
    tex_diffuse: glow::UniformLocation,
    tex_metallic: glow::UniformLocation,
    tex_emissive: glow::UniformLocation,
    tex_normal: glow::UniformLocation,
    tex_occlusion: glow::UniformLocation,
}
impl ModelShader {
    fn get_attrib(gl: &glow::Context, shader: &Shader, name: &str) -> Result<u32> {
        match unsafe { gl.get_attrib_location(shader.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!("Model shader does not have '{}' attrib!", name);
            }
        }
    }
    fn get_uniform_tex(
        gl: &glow::Context,
        shader: &Shader,
        name: &str,
        default: i32,
    ) -> Result<glow::UniformLocation> {
        match unsafe { gl.get_uniform_location(shader.program, name) } {
            Some(idx) => {
                unsafe {
                    shader.use_program(gl);
                    gl.uniform_1_i32(Some(&idx), default);
                    gl.use_program(None);
                }
                Ok(idx)
            }
            None => {
                anyhow::bail!("Model shader does not have '{}' uniform!", name);
            }
        }
    }
    fn get_uniform_block(gl: &glow::Context, shader: &Shader, name: &str) -> Result<u32> {
        match unsafe { gl.get_uniform_block_index(shader.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!("Model shader does not have '{}' uniform block!", name);
            }
        }
    }

    pub fn new(ctx: &Context) -> Result<Self> {
        let gl = &ctx.gl;
        let shader = ShaderBuilder::new(Some("PBR Shader"))
            .vert_file("material_pbr.vert")
            .frag_file("material_pbr.frag")
            .build(ctx)?;

        let vertex = Self::get_attrib(gl, &shader, "vertex")?;

        let primitive_uniform = Self::get_uniform_block(gl, &shader, "Primitive")?;
        let material_uniform = Self::get_uniform_block(gl, &shader, "Material")?;
        let lighting_uniform = Self::get_uniform_block(gl, &shader, "Lighting")?;

        let tex_diffuse = Self::get_uniform_tex(gl, &shader, "baseColour_tex", 0)?;
        let tex_metallic = Self::get_uniform_tex(gl, &shader, "metallic_tex", 1)?;
        let tex_emissive = Self::get_uniform_tex(gl, &shader, "emissive_tex", 2)?;
        let tex_normal = Self::get_uniform_tex(gl, &shader, "normal_tex", 3)?;
        let tex_occlusion = Self::get_uniform_tex(gl, &shader, "occlusion_tex", 4)?;

        let lighting_data = LightingUniform::default();
        let lighting_buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(lighting_data.buffer()?.into_inner().as_slice())
            .build(ctx)?;

        Ok(ModelShader {
            shader,
            lighting_buffer,
            vertex,
            primitive_uniform,
            material_uniform,
            lighting_uniform,
            tex_diffuse,
            tex_metallic,
            tex_emissive,
            tex_normal,
            tex_occlusion,
        })
    }
}

pub struct Material {
    uniform_data: MaterialUniform,
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
        ctx: &Context,
        mat: &gltf::Material,
        textures: &[Rc<Texture>],
    ) -> Result<Self> {
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
        let metallic = match pbr.metallic_roughness_texture() {
            Some(info) => {
                data.metallic_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones(ctx)?,
        };
        let emissive = match mat.emissive_texture() {
            Some(info) => {
                data.emissive_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_zeros(ctx)?,
        };
        let normalmap = match mat.normal_texture() {
            Some(info) => {
                data.normal_texcoord = info.tex_coord();
                data.normal_scale = info.scale();
                textures[info.texture().index()].clone()
            }
            None => tex_zeros(ctx)?,
        };
        let ambientocclusion = match mat.occlusion_texture() {
            Some(info) => {
                // TODO strength?
                data.occlusion_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones(ctx)?,
        };

        let uniform_buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Static)
            .data(data.buffer()?.into_inner().as_slice())
            .build(ctx)?;

        Ok(Material {
            uniform_data: data,
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
    vertices: Buffer,
    indices: Buffer,
    num_indices: i32,
    element_type: u32,
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
        let uniform_data = PrimitiveUniform::default();
        let uniform_buffer = BufferBuilder::new()
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(uniform_data.buffer()?.into_inner().as_slice())
            .build(ctx)?;

        Ok(Primitive {
            uniform_data,
            uniform_buffer,
            topology,
            vertices,
            indices,
            element_type: glow::FLOAT,
            num_indices: index_data.len() as i32,
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

    pub fn render(
        &self,
        shader: &ModelShader,
        transform: &Matrix4<f32>,
        ctx: &Context,
    ) -> Result<()> {
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

        let gl = &ctx.gl;
        for p in &self.primitives {
            // Update the primitive uniform
            let new_transform = VIEW * transform;
            let mut data = p.uniform_data;
            data.view = OPENGL_TO_WGPU * new_transform;
            data.normal = new_transform
                .fixed_resize::<3, 3>(0.0)
                .try_inverse()
                .unwrap()
                .transpose();
            p.uniform_buffer
                .write(ctx, data.buffer()?.into_inner().as_slice())?;

            let m = &p.material;

            // Render
            unsafe {
                gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(p.indices.buffer));
                gl.bind_buffer(glow::ARRAY_BUFFER, Some(p.vertices.buffer));
                gl.enable_vertex_attrib_array(shader.vertex);

                // Uniforms
                p.uniform_buffer.bind(gl);
                gl.bind_buffer_base(
                    glow::UNIFORM_BUFFER,
                    shader.primitive_uniform,
                    Some(p.uniform_buffer.buffer),
                );
                m.uniform_buffer.bind(gl);
                gl.bind_buffer_base(
                    glow::UNIFORM_BUFFER,
                    shader.material_uniform,
                    Some(p.uniform_buffer.buffer),
                );

                // Textures
                gl.active_texture(glow::TEXTURE1);
                m.metallic.bind(gl, 1);
                gl.active_texture(glow::TEXTURE2);
                m.emissive.bind(gl, 2);
                gl.active_texture(glow::TEXTURE3);
                m.normalmap.bind(gl, 3);
                gl.active_texture(glow::TEXTURE4);
                m.ambientocclusion.bind(gl, 4);
                gl.active_texture(glow::TEXTURE0); // Have to end on TEXTURE0
                m.diffuse.bind(gl, 0);

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

    pub fn render(
        &mut self,
        shader: &ModelShader,
        transform: &Matrix4<f32>,
        ctx: &Context,
    ) -> Result<()> {
        let new_transform = self.transform * transform;
        if let Some(mesh) = &self.mesh {
            mesh.render(shader, &new_transform, ctx)?;
        }
        for child in &mut self.children {
            child.render(shader, &new_transform, ctx)?;
        }
        Ok(())
    }
}

pub struct Scene {
    name: Option<String>,
    nodes: Vec<Node>,
}

impl Scene {
    pub fn from_gltf(scene: &gltf::Scene, meshes: &[Rc<Mesh>]) -> Result<Self> {
        let nodes: Vec<Node> = scene
            .nodes()
            .map(|node| Node::from_gltf(&node, meshes))
            .collect::<Result<Vec<_>, _>>()?;
        let name = scene.name().map(|s| s.to_owned());
        Ok(Scene { nodes, name })
    }

    pub fn render(
        &mut self,
        shader: &ModelShader,
        transform: &Matrix4<f32>,
        lighting: &LightingUniform,
        ctx: &Context,
    ) -> Result<()> {
        let gl = &ctx.gl;

        // Update lighting
        shader
            .lighting_buffer
            .write(ctx, lighting.buffer()?.into_inner().as_slice())?;
        shader.lighting_buffer.bind(&ctx.gl);
        unsafe {
            gl.bind_buffer_base(
                glow::UNIFORM_BUFFER,
                shader.lighting_uniform,
                Some(shader.lighting_buffer.buffer),
            );
        };

        // Set up
        shader.shader.use_program(gl);

        // TODO shadow pass

        // Mesh pass
        for node in &mut self.nodes {
            node.render(shader, transform, ctx)?;
        }

        // Clean up
        unsafe {
            gl.disable_vertex_attrib_array(shader.vertex);
            gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, None);
            gl.bind_buffer(glow::ARRAY_BUFFER, None);
            gl.use_program(None);
        }

        Ok(())
    }
}

pub struct Model {
    scenes: Vec<Scene>,
    shader: Arc<ModelShader>,
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

fn load_gltf_texture(
    ctx: &Context,
    node: &gltf::texture::Texture,
    base: &std::path::Path,
) -> Result<Texture> {
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

use std::sync::{Arc, OnceLock};
static SHADER: OnceLock<Arc<ModelShader>> = OnceLock::new();

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

        //let shader = Rc::new(ModelShader::new(ctx)?);
        let shader = SHADER.get_or_init(|| Arc::new(ModelShader::new(ctx).unwrap()));

        Ok(Model {
            scenes,
            shader: shader.clone(),
        })
    }

    pub fn render(
        &mut self,
        ctx: &Context,
        lighting: &LightingUniform,
        transform: &Matrix4<f32>,
    ) -> Result<()> {
        if let Some(scene) = self.scenes.first_mut() {
            scene.render(&self.shader, transform, lighting, ctx)?;
        }

        Ok(())
    }
}

#[no_mangle]
pub extern "C" fn gltf_loadFromFile_(cpath: *const c_char) -> *const Model {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    let model = Model::from_path(&ctx, path.to_str().unwrap()).unwrap();
    Box::into_raw(Box::new(model))
}

#[no_mangle]
pub extern "C" fn gltf_free_(model: *mut Model) {
    let _ = unsafe { Box::from_raw(model) }; // should drop
}

#[no_mangle]
pub extern "C" fn gltf_render_(
    fb: naevc::GLuint,
    model: *mut Model,
    transform: *const Matrix4<f32>,
    time: f32,
    size: f64,
) {
    gltf_renderScene_(fb, model, 0, transform, time, size)
}

#[no_mangle]
pub extern "C" fn gltf_renderScene_(
    fb: naevc::GLuint,
    model: *mut Model,
    scene: c_int,
    transform: *const Matrix4<f32>,
    time: f32,
    size: f64,
) {
    let model = unsafe { &mut *model };
    let transform = unsafe { &*transform };
    let ctx = CONTEXT.get().unwrap(); /* Lock early. */
    //let lighting = LightingUniform::default();
    if let Some(scene) = model.scenes.get_mut(scene as usize) {
        let _ = scene.render(&model.shader, transform, ctx);
    }
}
