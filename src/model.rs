#![allow(dead_code)]
use anyhow::Result;
use encase::{ShaderSize, ShaderType};
use glow::HasContext;
use gltf::Gltf;
use nalgebra::{Matrix3, Matrix4, Point3, Vector3, Vector4};
use std::ffi::CStr;
use std::os::raw::{c_char, c_double, c_int};
use std::rc::Rc;

use crate::buffer::{
    Buffer, BufferBuilder, BufferTarget, BufferUsage, VertexArray, VertexArrayBuffer,
    VertexArrayBuilder,
};
use crate::context::{Context, ContextWrapper};
use crate::ndata;
use crate::shader::{Shader, ShaderBuilder};
use crate::texture;
use crate::texture::{FramebufferTarget, Texture, TextureBuilder};
use crate::{gettext, warn};

const MAX_LIGHTS: usize = 7;

fn tex_value(ctx: &ContextWrapper, name: Option<&str>, value: [u8; 3]) -> Result<Rc<Texture>> {
    let mut img = image::RgbImage::new(1, 1);
    img.put_pixel(0, 0, image::Rgb(value));
    Ok(Rc::new(
        TextureBuilder::new()
            .name(name)
            .width(1)
            .height(1)
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
#[derive(Debug, Copy, Default, Clone, ShaderType)]
pub struct LightUniform {
    position: Vector3<f32>,
    colour: Vector3<f32>,
    sun: u32,
}
impl LightUniform {
    pub const fn default() -> Self {
        LightUniform {
            position: Vector3::new(0.0, 0.0, 0.0),
            colour: Vector3::new(0.0, 0.0, 0.0),
            sun: 0,
        }
    }
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

    pub const fn default() -> Self {
        LightingUniform {
            ambient: Vector3::new(0.0, 0.0, 0.0),
            nlights: 2,
            lights: [
                // Key Light
                LightUniform {
                    position: Vector3::new(-3.0, 2.75, -3.0),
                    colour: Vector3::new(80.0, 80.0, 80.0),
                    sun: 0,
                },
                // Fill Light
                LightUniform {
                    position: Vector3::new(10.0, 11.5, 7.0),
                    colour: Vector3::new(1.0, 1.0, 1.0),
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
    primitive_uniform: u32,
    material_uniform: u32,
    lighting_uniform: u32,
}
impl ModelShader {
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

    pub fn new(ctx: &ContextWrapper) -> Result<Self> {
        let lctx = ctx.lock();
        let gl = &lctx.gl;
        let mut shaderbuilder = ShaderBuilder::new(Some("PBR Shader"))
            .vert_file("material_pbr.vert")
            .frag_file("material_pbr.frag")
            .sampler("baseColour_tex", 0)
            .sampler("metallic_tex", 1)
            .sampler("emissive_tex", 2)
            .sampler("normal_tex", 3)
            .sampler("occlusion_tex", 4);
        for i in 0..MAX_LIGHTS {
            let sname = format!("shadowmap_tex[{}]", i);
            shaderbuilder = shaderbuilder.sampler(&sname, (5 + i) as i32);
        }
        let shader = shaderbuilder.build(gl)?;

        let primitive_uniform = shader.get_uniform_block(gl, "Primitive")?;
        let material_uniform = shader.get_uniform_block(gl, "Material")?;
        let lighting_uniform = shader.get_uniform_block(gl, "Lighting")?;

        let lighting_data = LightingUniform::default();
        let lighting_buffer = BufferBuilder::new(Some("PBR Lighting Buffer"))
            .target(BufferTarget::Uniform)
            .usage(BufferUsage::Dynamic)
            .data(lighting_data.buffer()?.into_inner().as_slice())
            .build(gl)?;

        Ok(ModelShader {
            shader,
            lighting_buffer,
            primitive_uniform,
            material_uniform,
            lighting_uniform,
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
            gltf::material::AlphaMode::Mask => todo!(),
            gltf::material::AlphaMode::Blend => 1,
        };
        data.emissive_factor = mat.emissive_factor().into();

        let diffuse = match pbr.base_color_texture() {
            Some(info) => {
                data.diffuse_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones.clone(),
        };
        let metallic = match pbr.metallic_roughness_texture() {
            Some(info) => {
                data.metallic_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones.clone(),
        };
        let emissive = match mat.emissive_texture() {
            Some(info) => {
                data.emissive_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_zeros.clone(),
        };
        let normalmap = match mat.normal_texture() {
            Some(info) => {
                data.normal_texcoord = info.tex_coord();
                data.normal_scale = info.scale();
                textures[info.texture().index()].clone()
            }
            None => tex_zeros.clone(),
        };
        let ambientocclusion = match mat.occlusion_texture() {
            Some(info) => {
                // TODO strength?
                data.occlusion_texcoord = info.tex_coord();
                textures[info.texture().index()].clone()
            }
            None => tex_ones.clone(),
        };

        let uniform_buffer = {
            let lctx = ctx.lock();
            let gl = &lctx.gl;
            BufferBuilder::new(mat.name())
                .target(BufferTarget::Uniform)
                .usage(BufferUsage::Static)
                .data(data.buffer()?.into_inner().as_slice())
                .build(gl)?
        };

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
    vao: VertexArray,
    vertices: Buffer,
    indices: Buffer,
    num_indices: i32,
    element_type: u32,
    material: Rc<Material>,
    vertex_data: Option<Vec<Vertex>>,
}

impl Primitive {
    pub fn from_gltf(
        ctx: &ContextWrapper,
        prim: &gltf::Primitive,
        buffer_data: &[Vec<u8>],
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
            .data(uniform_data.buffer()?.into_inner().as_slice())
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
            vertex_data: Some(vertex_data),
        })
    }

    fn radius(&self, transform: &Matrix4<f32>) -> f32 {
        let mut radius: f32 = 0.0;
        if let Some(vertex_data) = &self.vertex_data {
            for vertex in vertex_data {
                //vertex.pos
                let pos = transform.transform_point(&Point3::from(vertex.pos));
                radius = radius.max(pos.coords.magnitude());
            }
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
        /*
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
        */

        let gl = &ctx.gl;
        for p in &self.primitives {
            // Update the primitive uniform
            let new_transform = transform.clone();
            //let new_transform = VIEW * transform;
            let mut data = p.uniform_data;
            data.view = new_transform;
            data.normal = new_transform
                .fixed_resize::<3, 3>(0.0)
                .try_inverse()
                .unwrap()
                .transpose();
            p.uniform_buffer.bind_write_base(
                ctx,
                data.buffer()?.into_inner().as_slice(),
                shader.primitive_uniform,
            )?;

            let m = &p.material;

            // Material Uniform should be as is
            m.uniform_buffer.bind_base(ctx, shader.material_uniform);

            // Textures
            m.metallic.bind(ctx, 1);
            m.emissive.bind(ctx, 2);
            m.normalmap.bind(ctx, 3);
            m.ambientocclusion.bind(ctx, 4);
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
}

pub struct Node {
    transform: Matrix4<f32>,
    mesh: Option<Rc<Mesh>>,
    children: Vec<Node>,
}

impl Node {
    pub fn from_gltf(node: &gltf::Node, meshes: &[Rc<Mesh>]) -> Result<Self> {
        let transform: Matrix4<f32> = node.transform().matrix().into();

        let mesh = node.mesh().map(|mesh| meshes[mesh.index()].clone());

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

    pub fn radius(&self, transform: Matrix4<f32>) -> f32 {
        let mut radius: f32 = 0.0;
        let transform = self.transform * transform.clone();
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
        let new_transform = self.transform * transform;
        if let Some(mesh) = &self.mesh {
            mesh.render(ctx, shader, &new_transform)?;
        }
        for child in &mut self.children {
            child.render(ctx, shader, &new_transform)?;
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
    pub fn from_gltf(scene: &gltf::Scene, meshes: &[Rc<Mesh>]) -> Result<Self> {
        let nodes: Vec<Node> = scene
            .nodes()
            .map(|node| Node::from_gltf(&node, meshes))
            .collect::<Result<Vec<_>, _>>()?;
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

    pub fn render(
        &mut self,
        ctx: &Context,
        target: &FramebufferTarget,
        shader: &ModelShader,
        transform: &Matrix4<f32>,
        lighting: &LightingUniform,
    ) -> Result<()> {
        let gl = &ctx.gl;

        // Update lighting
        shader.lighting_buffer.bind_write_base(
            ctx,
            lighting.buffer()?.into_inner().as_slice(),
            shader.lighting_uniform,
        )?;

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

        // TODO shadow pass
        //for i in 1..lighting.nlights {
        //}

        // Set up
        shader.shader.use_program(gl);
        shader
            .lighting_buffer
            .bind_base(ctx, shader.lighting_uniform);

        // Mesh pass
        unsafe {
            let (w, h) = target.dimensions();
            gl.viewport(0, 0, w as i32, h as i32);
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
    shader: Arc<ModelShader>,
    radius: f32,
    transform_scale: Matrix4<f32>,
}

fn load_buffer(buf: &gltf::buffer::Buffer, base: &std::path::Path) -> Result<Vec<u8>> {
    match buf.source() {
        gltf::buffer::Source::Uri(uri) => {
            let filename = base.join(uri);
            //Ok(std::fs::read(filename.as_path().to_str().unwrap())?)
            Ok(ndata::read(filename.as_path().to_str().unwrap())?)
        }
        gltf::buffer::Source::Bin => todo!(),
    }
}

fn load_gltf_texture(
    ctx: &ContextWrapper,
    node: &gltf::texture::Texture,
    base: &std::path::Path,
) -> Result<Texture> {
    let sampler = node.sampler();
    let mut tb = TextureBuilder::new().name(node.name());

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

    tb.build_wrap(ctx)
}

use std::sync::{Arc, OnceLock};
static SHADER: OnceLock<Arc<ModelShader>> = OnceLock::new();

impl Model {
    pub fn from_path(ctx: &ContextWrapper, path: &str) -> Result<Self> {
        use std::path::Path;
        let gltf = Gltf::from_reader(ndata::open(path)?)?;
        let base = Path::new(path).parent().unwrap();

        // Helper textures
        let tex_zeros = tex_value(ctx, Some("Black"), [0, 0, 0])?;
        let tex_ones = tex_value(ctx, Some("White"), [255, 255, 255])?;

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

        let shader = SHADER.get_or_init(|| Arc::new(ModelShader::new(ctx).unwrap()));

        // Get model radius
        let mut radius: f32 = 0.;
        for scene in &scenes {
            radius = radius.max(scene.radius);
        }
        let invradius = 1.0 / radius;
        #[rustfmt::skip]
        let transform_scale = Matrix4::<f32>::new(
            invradius, 0.0, 0.0, 0.0,
            0.0, invradius, 0.0, 0.0,
            0.0, 0.0,-invradius, 0.0,
            0.0, 0.0, 0.0, 1.0 );

        // Have to restore the core after loading
        let lctx = ctx.lock();
        unsafe {
            lctx.gl.bind_vertex_array(Some(lctx.vao_core));
        }

        Ok(Model {
            scenes,
            shader: shader.clone(),
            radius,
            transform_scale,
        })
    }

    pub fn render(
        &mut self,
        ctx: &Context,
        fb: &FramebufferTarget,
        lighting: &LightingUniform,
        transform: &Matrix4<f32>,
    ) -> Result<()> {
        let scene_transform = transform.clone() * self.transform_scale;
        self.render_scene(ctx, fb, 0, lighting, &scene_transform)
    }

    pub fn render_scene(
        &mut self,
        ctx: &Context,
        fb: &FramebufferTarget,
        sceneid: usize,
        lighting: &LightingUniform,
        transform: &Matrix4<f32>,
    ) -> Result<()> {
        let scene_transform = transform.clone() * self.transform_scale;
        if let Some(scene) = self.scenes.get_mut(sceneid) {
            scene.render(ctx, fb, &self.shader, &scene_transform, lighting)?;
        }
        Ok(())
    }

    pub fn into_ptr(self) -> *mut Model {
        Box::into_raw(Box::new(self))
    }
}

/// Just use cglobals for C stuff and hope it doesn't catch on fire :/
static mut CLIGHTING: LightingUniform = LightingUniform::default();
static mut CAMBIENT: Vector3<f32> = Vector3::new(0.0, 0.0, 0.0);
static mut CINTENSITY: f64 = 1.0;

#[unsafe(no_mangle)]
pub extern "C" fn gltf_init() -> c_int {
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_exit() {}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightReset() {
    unsafe {
        CLIGHTING = LightingUniform::default();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightSet(idx: c_int, light: *const naevc::Light) -> c_int {
    let n: usize = (2 + idx).try_into().unwrap();
    if n >= MAX_LIGHTS {
        warn!("Trying to set more lights than MAX_LIGHTS allows!");
        return -1;
    }
    unsafe {
        let intensity = (*light).intensity as f32;
        CLIGHTING.nlights = CLIGHTING.nlights.max((n + 1) as u32);
        CLIGHTING.lights[n] = LightUniform {
            sun: (*light).sun as u32,
            position: Vector3::new(
                (*light).pos.v[0] as f32,
                (*light).pos.v[1] as f32,
                (*light).pos.v[2] as f32,
            ),
            colour: Vector3::new(
                ((*light).colour.v[0] as f32) * intensity,
                ((*light).colour.v[1] as f32) * intensity,
                ((*light).colour.v[2] as f32) * intensity,
            ),
        }
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightAmbient(r: c_double, g: c_double, b: c_double) {
    unsafe {
        CLIGHTING.ambient = Vector3::new(r as f32, g as f32, b as f32);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightAmbientGet(r: *mut c_double, g: *mut c_double, b: *mut c_double) {
    unsafe {
        *r = CLIGHTING.ambient.x as f64;
        *g = CLIGHTING.ambient.y as f64;
        *b = CLIGHTING.ambient.z as f64;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightIntensity(strength: c_double) {
    unsafe {
        CINTENSITY = strength;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightIntensityGet() -> c_double {
    unsafe { CINTENSITY }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_lightTransform(
    _lighting: *mut naevc::Lighting,
    transform: *const Matrix4<f32>,
) {
    // TODO manipulate L
    unsafe {
        let transform = &*transform;
        for i in 0..CLIGHTING.nlights as usize {
            let mut l = CLIGHTING.lights[i];
            if l.sun != 0 {
                l.position = transform.transform_vector(&l.position);
            } else {
                l.position = Vector3::from_homogeneous(
                    transform.transform_point(&Point3::from(l.position)).into(),
                )
                .unwrap();
            }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_sceneBody(_model: *mut Model) -> u32 {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_sceneEngine(_model: *mut Model) -> u32 {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_numAnimations(_model: *mut Model) -> u32 {
    // TODO
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_trails(_model: *mut Model, num: *mut c_int) -> *const naevc::GltfTrail {
    // TODO
    unsafe {
        *num = 0;
    }
    std::ptr::null()
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_mounts(_model: *mut Model, num: *mut c_int) -> *const naevc::GltfMount {
    // TODO
    unsafe {
        *num = 0;
    }
    std::ptr::null()
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_loadFromFile(cpath: *const c_char) -> *const Model {
    let path = unsafe { CStr::from_ptr(cpath) };
    let ctx = Context::get().unwrap().as_wrap();
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
    transform: *const Matrix4<f32>,
    time: f32,
    size: f64,
) {
    gltf_renderScene(fb, model, 0, transform, time, size)
}

#[unsafe(no_mangle)]
pub extern "C" fn gltf_renderScene(
    fb: naevc::GLuint,
    model: *mut Model,
    scene: c_int,
    ctransform: *const Matrix4<f32>,
    _time: f32,
    size: f64,
) {
    // TODO animations
    let model = unsafe { &mut *model };
    let transform = match ctransform.is_null() {
        true => &Matrix4::identity(),
        false => unsafe { &*ctransform },
    };
    let ctx = Context::get().unwrap(); /* Lock early. */
    #[allow(static_mut_refs)]
    let lighting = unsafe { &CLIGHTING };
    let _ = model.render_scene(
        ctx,
        &FramebufferTarget::from_gl(fb, size as usize, size as usize),
        scene as usize,
        lighting,
        &transform,
    );
}
