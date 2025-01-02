#![allow(dead_code)]
use anyhow::Result;
use glow::*;

use crate::gettext::gettext;
use crate::ndata;
use crate::ngl::Context;
use crate::{formatx, warn};

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
    pub vertfile: Option<String>,
    pub fragfile: Option<String>,
    pub program: glow::Program,
}

impl Shader {
    const INCLUDE_INSTRUCTION: &str = "#include";

    fn load_file(path: &str) -> Result<String> {
        let rawdata = ndata::read(path)?;
        let data = std::str::from_utf8(&rawdata)?;

        // Really simple preprocessor
        let mut module_string = String::new();
        for line in data.lines() {
            if line.starts_with(Shader::INCLUDE_INSTRUCTION) {
                match line.trim().split("\"").nth(2) {
                    Some(include) => {
                        let include_string = Shader::load_file(include)?;
                        module_string.push_str(&include_string);
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

    fn compile(
        ctx: &Context,
        shadertype: ShaderType,
        name: &str,
        source: &str,
    ) -> Result<glow::Shader> {
        let gl = &ctx.gl;
        let shader = unsafe {
            gl.create_shader(shadertype.to_gl())
                .map_err(|e| anyhow::anyhow!(e))?
        };
        unsafe {
            gl.shader_source(shader, source);
            gl.compile_shader(shader);
        }
        if unsafe { !gl.get_shader_compile_status(shader) } {
            let slog = unsafe { gl.get_shader_info_log(shader) };
            warn!("Failed to compile shader '{}': [[\n{}\n]]", name, slog);
            return Err(anyhow::anyhow!("failed to compile shader program"));
        }
        Ok(shader)
    }

    fn link(
        ctx: &Context,
        vertshader: glow::Shader,
        fragshader: glow::Shader,
    ) -> Result<glow::Program> {
        let gl = &ctx.gl;
        let program = unsafe { gl.create_program().map_err(|e| anyhow::anyhow!(e))? };
        unsafe {
            gl.attach_shader(program, vertshader);
            gl.attach_shader(program, fragshader);
            gl.link_program(program);
            gl.delete_shader(vertshader);
            gl.delete_shader(fragshader);
        }
        if unsafe { !gl.get_program_link_status(program) } {
            let slog = unsafe { gl.get_program_info_log(program) };
            warn!("Failed to link shader: [[\n{}\n]]", slog);
            return Err(anyhow::anyhow!("failed to link shader program"));
        }
        Ok(program)
    }

    pub fn from_files(ctx: &Context, vert: &str, frag: &str) -> Result<Self> {
        let mut vertdata = Shader::load_file(vert)?;
        let mut fragdata = Shader::load_file(frag)?;

        let glsl = unsafe { naevc::gl_screen.glsl };
        let mut prepend = format!("#version {}\n\n#define GLSL_VERSION {}", glsl, glsl);
        prepend.push_str("#define HAS_GL_ARB_shader_subroutine 1\n");

        vertdata.insert_str(0, &prepend);
        fragdata.insert_str(0, &prepend);

        let vertshader = Shader::compile(ctx, ShaderType::Vertex, vert, &vertdata)?;
        let fragshader = Shader::compile(ctx, ShaderType::Fragment, frag, &fragdata)?;
        let program = Shader::link(ctx, vertshader, fragshader)?;

        Ok(Shader {
            vertfile: Some(String::from(vert)),
            fragfile: Some(String::from(frag)),
            program,
        })
    }
}
