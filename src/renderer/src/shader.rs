use anyhow::Result;
use glow::*;
use std::ffi::CStr;
use std::os::raw::c_char;

use crate::Context;
use log::{warn, warn_err};

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
    pub name: Option<String>,
    pub program: glow::Program,
}
impl Drop for Shader {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteProgram(self.program));
    }
}
impl Shader {
    pub fn use_program(&self, gl: &glow::Context) {
        unsafe {
            gl.use_program(Some(self.program));
        }
    }

    pub fn get_attrib(&self, gl: &glow::Context, name: &str) -> Result<u32> {
        match unsafe { gl.get_attrib_location(self.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!(
                    "Shader '{}' does not have '{}' attrib!",
                    self.name.as_deref().unwrap_or("UNKNOWN"),
                    name
                );
            }
        }
    }

    pub fn get_uniform_block(&self, gl: &glow::Context, name: &str) -> Result<u32> {
        match unsafe { gl.get_uniform_block_index(self.program, name) } {
            Some(idx) => Ok(idx),
            None => {
                anyhow::bail!(
                    "Shader '{}' does not have '{}' uniform block!",
                    self.name.as_deref().unwrap_or("UNKOWN"),
                    name
                );
            }
        }
    }
}

#[derive(Debug)]
enum Binding {
    Sampler(String, i32),
    //Uniform( String, i32 ),
    UniformBlock(String, u32),
}
impl Binding {
    fn apply(&self, gl: &glow::Context, program: glow::Program, name: &str) {
        match self {
            Self::Sampler(samplername, idx) => unsafe {
                match gl.get_uniform_location(program, &samplername) {
                    Some(uniformid) => {
                        gl.uniform_1_i32(Some(&uniformid), *idx);
                    }
                    None => {
                        warn!("shader '{}' does not have sampler '{}'", name, samplername);
                    }
                }
            },
            Self::UniformBlock(uniformname, idx) => unsafe {
                match gl.get_uniform_block_index(program, &uniformname) {
                    Some(uniformid) => {
                        gl.uniform_block_binding(program, uniformid, *idx);
                    }
                    None => {
                        warn!(
                            "shader '{}' does not have uniform block '{}'",
                            name, uniformname
                        );
                    }
                }
            },
        }
    }

    fn name(&self) -> &str {
        match self {
            Self::Sampler(s, _) => &s,
            Self::UniformBlock(s, _) => &s,
        }
    }
}

#[derive(PartialEq)]
enum ShaderSource {
    Path(String),
    Data(String),
}
impl ShaderSource {
    const INCLUDE_INSTRUCTION: &str = "#include";
    const GLSL_PATH: &str = "shaders/";

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
        }
    }

    pub fn name(&self) -> String {
        match self {
            Self::Path(path) => path.clone(),
            Self::Data(_) => String::from("DATA"),
        }
    }
}

enum ProgramSource {
    Glsl(ShaderSource, ShaderSource),
    GlslSingle(ShaderSource),
    Wgsl(ShaderSource),
}
impl ProgramSource {
    fn shader_source_lines(source: &str) -> String {
        let mut buf = String::new();
        for (i, line) in source.lines().enumerate() {
            buf.push_str(&format!("{i:04}: {line}\n"));
        }
        buf
    }

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
            let buf = Self::shader_source_lines(&source);
            let slog = unsafe { gl.get_shader_info_log(shader) };
            warn!("{name}:\n{buf}\nFailed to compile shader '{name}': [[\n{slog}\n]]");
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

    fn build_glsl(
        gl: &glow::Context,
        name: Option<&str>,
        prepend: Option<&str>,
        vert: &ShaderSource,
        frag: &ShaderSource,
        bindings: &[Binding],
    ) -> Result<glow::Program> {
        let mut vertdata = ShaderSource::to_string(&vert)?;
        let mut fragdata = if vert == frag {
            vertdata.clone()
        } else {
            ShaderSource::to_string(&frag)?
        };

        let version = {
            let v = gl.version();
            v.major * 100 + v.minor * 10
        };

        if let Some(prepend) = prepend {
            vertdata.insert_str(0, &prepend);
            fragdata.insert_str(0, &prepend);
        }
        vertdata.insert_str(0, "#define VERT 1\n");
        fragdata.insert_str(0, "#define FRAG 1\n");
        let version = format!(
            "#version {version}\n\n#define GLSL_VERSION {version}\n#define HAS_GL_ARB_shader_subroutine 1\n"
        );
        vertdata.insert_str(0, &version);
        fragdata.insert_str(0, &version);

        let vertname = vert.name();
        let fragname = frag.name();

        let vertshader = Self::compile(gl, ShaderType::Vertex, &vertname, &vertdata)?;
        let fragshader = Self::compile(gl, ShaderType::Fragment, &fragname, &fragdata)?;
        let name = match name {
            Some(name) => name,
            None => &format!("{}-{}", &vertname, &fragname),
        };
        let program = Self::link(gl, &name, vertshader, fragshader)?;

        unsafe {
            gl.use_program(Some(program));
            for b in bindings {
                b.apply(gl, program, name);
            }
            gl.use_program(None);
        }

        Ok(program)
    }

    fn bind_wgsl(
        gl: &glow::Context,
        module: &naga::ir::Module,
        vertrefl: &naga::back::glsl::ReflectionInfo,
        fragrefl: &naga::back::glsl::ReflectionInfo,
        program: glow::Program,
        name: &str,
        bindings: &[Binding],
    ) -> Result<()> {
        unsafe {
            gl.use_program(Some(program));
        }
        for b in bindings {
            let mut matched = false;
            match b {
                Binding::UniformBlock(bname, bidx) => {
                    for v in vertrefl.uniforms.iter().chain(fragrefl.uniforms.iter()) {
                        let gv = &module.global_variables[*v.0];
                        if let Some(varname) = &gv.name {
                            if bname == varname {
                                Binding::UniformBlock(v.1.clone(), *bidx).apply(gl, program, name);
                                matched = true;
                            }
                        }
                    }
                }
                Binding::Sampler(bname, bidx) => {
                    // Assume vertex shader doesn't do texture mapping
                    for v in fragrefl.texture_mapping.iter() {
                        if let Some(sampler) = v.1.sampler {
                            let gv = &module.global_variables[sampler];
                            if let Some(varname) = &gv.name {
                                if bname == varname {
                                    Binding::Sampler(v.0.clone(), *bidx).apply(gl, program, name);
                                    matched = true;
                                }
                            }
                        }
                    }
                }
            }
            if !matched {
                warn!("unmatched binding '{}'", b.name());
            }
        }
        unsafe {
            gl.use_program(None);
        }

        Ok(())
    }

    fn build_wgsl(
        gl: &glow::Context,
        name: Option<&str>,
        prepend: Option<&str>,
        source: &ShaderSource,
        bindings: &[Binding],
    ) -> Result<glow::Program> {
        let mut data = ShaderSource::to_string(&source)?;
        if let Some(prepend) = prepend {
            data.insert_str(0, &prepend);
        };
        let name = match name {
            Some(n) => n,
            None => "UNKNOWN",
        };

        use naga::back::glsl;
        let options = glsl::Options {
            version: naga::back::glsl::Version::Desktop(330),
            writer_flags: glsl::WriterFlags::empty(),
            ..Default::default()
        };

        let module: naga::Module = match naga::front::wgsl::parse_str(&data) {
            Ok(m) => m,
            Err(e) => {
                let buf = Self::shader_source_lines(&data);
                warn!("{name}:\n{buf}\nFailed to parse WGSL shader '{name}': [[\n{e}\n]]");
                return Err(e.into());
            }
        };
        let module_info: naga::valid::ModuleInfo = match naga::valid::Validator::new(
            naga::valid::ValidationFlags::all(),
            naga::valid::Capabilities::all(),
        )
        .subgroup_stages(naga::valid::ShaderStages::all())
        .subgroup_operations(naga::valid::SubgroupOperationSet::all())
        .validate(&module)
        {
            Ok(m) => m,
            Err(e) => {
                let buf = Self::shader_source_lines(&data);
                warn!("{name}:\n{buf}\nFailed to validate WGSL shader '{name}': [[\n{e}\n]]");
                return Err(e.into());
            }
        };

        let mut vertdata = String::new();
        let vertrefl = glsl::Writer::new(
            &mut vertdata,
            &module,
            &module_info,
            &options,
            &glsl::PipelineOptions {
                entry_point: "main_vs".into(),
                shader_stage: naga::ShaderStage::Vertex,
                multiview: None,
            },
            naga::proc::BoundsCheckPolicies::default(),
        )?
        .write()?;
        let vertshader = Self::compile(
            gl,
            ShaderType::Vertex,
            &format!("{} - Vertex", name),
            &vertdata,
        )?;

        let mut fragdata = String::new();
        let fragrefl = &glsl::Writer::new(
            &mut fragdata,
            &module,
            &module_info,
            &options,
            &glsl::PipelineOptions {
                entry_point: "main_fs".into(),
                shader_stage: naga::ShaderStage::Fragment,
                multiview: None,
            },
            naga::proc::BoundsCheckPolicies::default(),
        )?
        .write()?;
        let fragshader = Self::compile(
            gl,
            ShaderType::Fragment,
            &format!("{} - Fragment", name),
            &fragdata,
        )?;
        let program = Self::link(gl, &name, vertshader, fragshader)?;

        Self::bind_wgsl(gl, &module, &vertrefl, &fragrefl, program, name, bindings)?;

        Ok(program)
    }

    pub fn build_gl(
        self,
        gl: &glow::Context,
        name: Option<&str>,
        prepend: Option<&str>,
        bindings: &[Binding],
    ) -> Result<glow::Program> {
        match self {
            Self::Glsl(vert, frag) => Self::build_glsl(gl, name, prepend, &vert, &frag, bindings),
            Self::GlslSingle(src) => Self::build_glsl(gl, name, prepend, &src, &src, bindings),
            Self::Wgsl(src) => Self::build_wgsl(gl, name, prepend, &src, bindings),
        }
    }
}

pub struct ProgramBuilder {
    name: Option<String>,
    source: Option<ProgramSource>,
    prepend: Option<String>,
    bindings: Vec<Binding>,
}
impl ProgramBuilder {
    pub fn new(name: Option<&str>) -> Self {
        ProgramBuilder {
            name: name.map(String::from),
            source: None,
            prepend: None,
            bindings: Vec::new(),
        }
    }

    pub fn vert_frag_file(mut self, vertpath: &str, fragpath: &str) -> Self {
        self.source = Some(ProgramSource::Glsl(
            ShaderSource::Path(String::from(vertpath)),
            ShaderSource::Path(String::from(fragpath)),
        ));
        self
    }

    pub fn vert_frag_file_single(mut self, path: &str) -> Self {
        self.source = Some(ProgramSource::GlslSingle(ShaderSource::Path(String::from(
            path,
        ))));
        self
    }

    pub fn vert_frag_data(mut self, vertdata: &str, fragdata: &str) -> Self {
        self.source = Some(ProgramSource::Glsl(
            ShaderSource::Data(String::from(vertdata)),
            ShaderSource::Data(String::from(fragdata)),
        ));
        self
    }

    pub fn wgsl_file(mut self, path: &str) -> Self {
        self.source = Some(ProgramSource::Wgsl(ShaderSource::Path(String::from(path))));
        self
    }

    pub fn prepend(mut self, data: &str) -> Self {
        self.prepend = Some(String::from(data));
        self
    }

    pub fn sampler(mut self, name: &str, idx: i32) -> Self {
        self.bindings.push(Binding::Sampler(name.to_string(), idx));
        self
    }

    pub fn uniform_buffer(mut self, name: &str, idx: u32) -> Self {
        self.bindings
            .push(Binding::UniformBlock(name.to_string(), idx));
        self
    }

    pub fn build(self, gl: &glow::Context) -> Result<Shader> {
        let program = match self.source {
            Some(src) => src.build_gl(
                gl,
                self.name.as_deref(),
                self.prepend.as_deref(),
                &self.bindings,
            ),
            None => anyhow::bail!("source not specified for shader!"),
        }?;

        Ok(Shader {
            name: self.name,
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
    let mut sb =
        ProgramBuilder::new(None).vert_frag_file(&vert.to_string_lossy(), &frag.to_string_lossy());

    if !cprepend.is_null() {
        let prepend = unsafe { CStr::from_ptr(cprepend) };
        sb = sb.prepend(&prepend.to_string_lossy());
    }

    let shader = ManuallyDrop::new(match sb.build(&ctx.gl) {
        Ok(s) => s,
        Err(e) => {
            warn_err!(e);
            return 0;
        }
    });

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
        match ProgramBuilder::new(None)
            .vert_frag_data(vertdata, fragdata)
            .build(&ctx.gl)
        {
            Ok(s) => s,
            Err(e) => {
                warn_err!(e);
                return 0;
            }
        },
    );

    shader.program.0.into()
}
