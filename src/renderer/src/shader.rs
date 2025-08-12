use anyhow::Result;
use glow::*;
use std::ffi::CStr;
use std::os::raw::c_char;

use crate::Context;
use log::warn;

pub enum ShaderType {
    Fragment,
    Vertex,
}
impl ShaderType {
    pub fn to_gl(&self) -> u32 {
        match self {
            ShaderType::Fragment => glow::FRAGMENT_SHADER,
            ShaderType::Vertex => glow::VERTEX_SHADER,
        }
    }
}

pub struct Shader {
    pub name: String,
    pub vertname: String,
    pub fragname: String,
    pub program: glow::Program,
}
impl Drop for Shader {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteProgram(self.program));
    }
}
impl Shader {
    fn compile(
        gl: &glow::Context,
        shadertype: ShaderType,
        name: &str,
        source: &str,
    ) -> Result<glow::Shader> {
        let shader = unsafe {
            gl.create_shader(shadertype.to_gl())
                .map_err(|e| anyhow::anyhow!(e))?
        };
        unsafe {
            gl.shader_source(shader, source);
            gl.compile_shader(shader);
            if gl.supports_debug() {
                gl.object_label(glow::SHADER, shader.0.into(), Some(name));
            }
        }
        if unsafe { !gl.get_shader_compile_status(shader) } {
            let mut buf = String::new();
            for (i, line) in source.lines().enumerate() {
                buf.push_str(&format!("{i:04}: {line}"));
            }
            let slog = unsafe { gl.get_shader_info_log(shader) };
            warn!("{buf}\nFailed to compile shader '{name}': [[\n{slog}\n]]");
            return Err(anyhow::anyhow!("failed to compile shader program"));
        }
        Ok(shader)
    }

    fn link(
        gl: &glow::Context,
        name: &str,
        vertshader: glow::Shader,
        fragshader: glow::Shader,
    ) -> Result<glow::Program> {
        let program = unsafe { gl.create_program().map_err(|e| anyhow::anyhow!(e))? };
        unsafe {
            gl.attach_shader(program, vertshader);
            gl.attach_shader(program, fragshader);
            gl.link_program(program);
            gl.delete_shader(vertshader);
            gl.delete_shader(fragshader);
            if gl.supports_debug() {
                gl.object_label(glow::PROGRAM, program.0.into(), Some(name));
            }
        }
        if unsafe { !gl.get_program_link_status(program) } {
            let slog = unsafe { gl.get_program_info_log(program) };
            warn!("Failed to link shader: [[\n{slog}\n]]");
            return Err(anyhow::anyhow!("failed to link shader program"));
        }
        Ok(program)
    }

    pub fn use_program(&self, gl: &glow::Context) {
        unsafe {
            gl.use_program(Some(self.program));
        }
    }

    pub fn get_attrib(&self, gl: &glow::Context, name: &str) -> Result<u32> {
        match unsafe { gl.get_attrib_location(self.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!("Shader '{}' does not have '{}' attrib!", self.name, name);
            }
        }
    }

    pub fn get_uniform_block(&self, gl: &glow::Context, name: &str) -> Result<u32> {
        match unsafe { gl.get_uniform_block_index(self.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!(
                    "Shader '{}' does not have '{}' uniform block!",
                    self.name,
                    name
                );
            }
        }
    }
}

#[derive(PartialEq)]
enum ShaderSource {
    Path(String),
    Data(String),
    None,
}
impl ShaderSource {
    const INCLUDE_INSTRUCTION: &str = "#include";
    const GLSL_PATH: &str = "glsl/";

    /// Really simple preprocessor
    fn preprocess(data: &str) -> Result<String> {
        let mut module_string = String::new();
        for line in data.lines() {
            let line = line.trim();
            if line.starts_with(Self::INCLUDE_INSTRUCTION) {
                match line.split("\"").nth(1) {
                    Some(include) => {
                        let include_string = Self::load_file(include)?;
                        module_string.push_str(&include_string);
                        module_string.push('\n');
                    }
                    None => {
                        return Err(anyhow::anyhow!("#include syntax error"));
                    }
                }
            } else {
                module_string.push_str(line);
                module_string.push('\n');
            }
        }
        Ok(module_string)
    }

    fn load_file(path: &str) -> Result<String> {
        let fullpath = format!("{}{}", Self::GLSL_PATH, path);
        let rawdata = ndata::read(&fullpath)?;
        let data = std::str::from_utf8(&rawdata)?;
        Self::preprocess(data)
    }

    pub fn to_string(&self) -> Result<String> {
        match self {
            Self::Path(path) => Self::load_file(path),
            Self::Data(data) => Self::preprocess(data),
            Self::None => Err(anyhow::anyhow!("no shader source defined!")),
        }
    }

    pub fn name(&self) -> String {
        match self {
            Self::Path(path) => path.clone(),
            Self::Data(_) => String::from("DATA"),
            Self::None => String::from("NONE"),
        }
    }
}

pub struct ShaderBuilder {
    name: Option<String>,
    vert: ShaderSource,
    frag: ShaderSource,
    prepend: String,
    samplers: Vec<(String, i32)>,
    uniform_buffers: Vec<(String, u32)>,
}
impl ShaderBuilder {
    pub fn new(name: Option<&str>) -> Self {
        ShaderBuilder {
            name: name.map(String::from),
            vert: ShaderSource::None,
            frag: ShaderSource::None,
            prepend: Default::default(),
            samplers: Vec::new(),
            uniform_buffers: Vec::new(),
        }
    }

    pub fn vert_file(mut self, path: &str) -> Self {
        self.vert = ShaderSource::Path(String::from(path));
        self
    }

    pub fn frag_file(mut self, path: &str) -> Self {
        self.frag = ShaderSource::Path(String::from(path));
        self
    }

    pub fn vert_frag_file(self, path: &str) -> Self {
        self.vert_file(path).frag_file(path)
    }

    pub fn vert_data(mut self, data: &str) -> Self {
        self.vert = ShaderSource::Data(String::from(data));
        self
    }

    pub fn frag_data(mut self, data: &str) -> Self {
        self.frag = ShaderSource::Data(String::from(data));
        self
    }

    pub fn prepend(mut self, data: &str) -> Self {
        self.prepend = String::from(data);
        self
    }

    pub fn sampler(mut self, name: &str, idx: i32) -> Self {
        self.samplers.push((name.to_string(), idx));
        self
    }

    pub fn uniform_buffer(mut self, name: &str, idx: u32) -> Self {
        self.uniform_buffers.push((name.to_string(), idx));
        self
    }

    pub fn build(self, gl: &glow::Context) -> Result<Shader> {
        let mut vertdata = ShaderSource::to_string(&self.vert)?;
        let mut fragdata = if self.vert == self.frag {
            vertdata.clone()
        } else {
            ShaderSource::to_string(&self.frag)?
        };

        let glsl = unsafe { naevc::gl_screen.glsl };
        let mut prepend = format!("#version {glsl}\n\n#define GLSL_VERSION {glsl}\n");
        prepend.push_str("#define HAS_GL_ARB_shader_subroutine 1\n");

        if !self.prepend.is_empty() {
            vertdata.insert_str(0, &self.prepend);
            fragdata.insert_str(0, &self.prepend);
        }
        vertdata.insert_str(0, "#define __VERT__ 1\n");
        fragdata.insert_str(0, "#define __FRAG__ 1\n");
        vertdata.insert_str(0, &prepend);
        fragdata.insert_str(0, &prepend);

        let vertname = self.vert.name();
        let fragname = self.frag.name();

        let vertshader = Shader::compile(gl, ShaderType::Vertex, &vertname, &vertdata)?;
        let fragshader = Shader::compile(gl, ShaderType::Fragment, &fragname, &fragdata)?;
        let name = match self.name {
            Some(name) => name,
            None => format!("{}-{}", &vertname, &fragname),
        };
        let program = Shader::link(gl, &name, vertshader, fragshader)?;

        unsafe {
            gl.use_program(Some(program));
            for (samplername, idx) in self.samplers {
                match gl.get_uniform_location(program, &samplername) {
                    Some(uniformid) => {
                        gl.uniform_1_i32(Some(&uniformid), idx);
                    }
                    None => {
                        warn!("shader '{}' does not have sampler '{}'", &name, samplername);
                    }
                }
            }
            for (uniformname, idx) in self.uniform_buffers {
                match gl.get_uniform_block_index(program, &uniformname) {
                    Some(uniformid) => {
                        gl.uniform_block_binding(program, uniformid, idx);
                    }
                    None => {
                        warn!(
                            "shader '{}' does not have uniform block '{}'",
                            &name, uniformname
                        );
                    }
                }
            }
            gl.use_program(None);
        }

        Ok(Shader {
            name,
            vertname,
            fragname,
            program,
        })
    }
}

use std::mem::ManuallyDrop;

#[unsafe(no_mangle)]
pub extern "C" fn gl_program_backend(
    cvert: *const c_char,
    cfrag: *const c_char,
    cprepend: *const c_char,
) -> u32 {
    let ctx = Context::get(); /* Lock early. */
    let vert = unsafe { CStr::from_ptr(cvert) };
    let frag = unsafe { CStr::from_ptr(cfrag) };
    let mut sb = ShaderBuilder::new(None)
        .vert_file(vert.to_str().unwrap())
        .frag_file(frag.to_str().unwrap());

    if !cprepend.is_null() {
        let prepend = unsafe { CStr::from_ptr(cprepend) };
        sb = sb.prepend(prepend.to_str().unwrap());
    }

    let shader = ManuallyDrop::new(sb.build(&ctx.gl).unwrap());

    shader.program.0.into()
}

#[unsafe(no_mangle)]
pub extern "C" fn gl_program_vert_frag_string(
    cvert: *const c_char,
    vert_size: usize,
    cfrag: *const c_char,
    frag_size: usize,
) -> u32 {
    let ctx = Context::get(); /* Lock early. */
    let vertdata =
        std::str::from_utf8(unsafe { std::slice::from_raw_parts(cvert as *const u8, vert_size) })
            .unwrap();
    let fragdata =
        std::str::from_utf8(unsafe { std::slice::from_raw_parts(cfrag as *const u8, frag_size) })
            .unwrap();
    let shader = ManuallyDrop::new(
        ShaderBuilder::new(None)
            .vert_data(vertdata)
            .frag_data(fragdata)
            .build(&ctx.gl)
            .unwrap(),
    );

    shader.program.0.into()
}
