#![allow(dead_code, unused_variables)]
use crate::shader::{ProgramBuilder, Shader};
use glow::*;
use mlua::{BorrowedStr, UserData, UserDataMethods};
use nalgebra::{Matrix2, Matrix3, Vector4};
use trie_rs::map::{Trie, TrieBuilder};

enum Type {
   Float,
   Vec2,
   Vec3,
   Vec4,
   Int,
   IVec2,
   IVec3,
   IVec4,
   Sampler2D,
}

struct UniformBlock {
   view_space_from_local: Matrix3<f32>,
   clip_space_from_view: Matrix3<f32>,
   clip_space_from_local: Matrix3<f32>,
   view_normal_from_local: Matrix2<f32>,
   love_screensize: Vector4<f32>,
   constant_colour: Vector4<f32>,
}

struct Uniform {
   location: glow::NativeUniformLocation,
   size: i32,
   utype: u32,
   tex: Option<u32>,
   name: String,
}

pub struct LuaShader {
   shader: Shader,
   uniforms: Trie<u8, Uniform>,
   textures: Vec<Option<crate::Texture>>,
   pp_id: Option<u32>,
}
impl LuaShader {
   const UNIFORM_BLOCK: u32 = 0;
   const VERTEX_POSITION: u32 = 0;
   const VERTEX_TEX_COROD: u32 = 1;
   const VERTEX_COLOUR: u32 = 2;
}

/*@
 * @brief Lua bindings to interact with shaders.
 *
 * @lua_mod shader
 */
impl UserData for LuaShader {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Creates a new shader.
       *
       *    @luatparam string vertex Script containing the vertex shader.
       *    @luatparam string fragment String containing the fragment shader.
       *    @luatreturn Shader A newly created shader.
       * @luafunc new
       */
      methods.add_function(
         "new",
         |_, (vertex, fragment): (BorrowedStr, BorrowedStr)| -> mlua::Result<Self> {
            let ctx = crate::Context::get();
            let shader = ProgramBuilder::new(None)
               .vert_frag_data(&vertex, &fragment)
               .uniform_buffer("LoveUniform", Self::UNIFORM_BLOCK)
               .build(&ctx.gl)?;

            let n = unsafe {
               ctx.gl
                  .get_program_parameter_i32(shader.program, glow::ACTIVE_UNIFORMS)
            };

            let mut ntex = 0;
            let mut builder = TrieBuilder::new();
            for i in 0..n {
               if let Some(uniform) = unsafe { ctx.gl.get_active_uniform(shader.program, i as u32) }
                  && let Some(loc) =
                     unsafe { ctx.gl.get_uniform_location(shader.program, &uniform.name) }
               {
                  let tex = if uniform.utype == glow::SAMPLER_2D {
                     ntex = ntex + 1;
                     Some(ntex - 1)
                  } else {
                     None
                  };
                  builder.push(
                     uniform.name.clone(),
                     Uniform {
                        location: loc,
                        size: uniform.size,
                        utype: uniform.utype,
                        tex,
                        name: uniform.name,
                     },
                  );
               }
            }
            let uniforms: Trie<u8, Uniform> = builder.build();
            let textures: Vec<_> = (0..ntex).map(|_| None).collect();

            Ok(LuaShader {
               shader,
               uniforms,
               textures,
               pp_id: None,
            })
         },
      );
      /*@
       * @brief Allows setting values of uniforms for a shader. Errors out if the
       * uniform is unknown or unused (as in optimized out by the compiler).
       *
       *    @luatparam Shader shader Shader to send uniform to.
       *    @luatparam string name Name of the uniform.
       * @luafunc send
       */
      methods.add_method(
         "send",
         |_,
          this,
          (name, data0, data1, data2, data3): (
            BorrowedStr,
            mlua::Value,
            Option<mlua::Value>,
            Option<mlua::Value>,
            Option<mlua::Value>,
         )|
          -> mlua::Result<()> { Ok(()) },
      );
      /*@
       * @brief Allows setting values of uniforms for a shader, while ignoring unknown
       * (or unused) uniforms.
       *
       *    @luatparam Shader shader Shader to send uniform to.
       *    @luatparam string name Name of the uniform.
       * @luafunc sendRaw
       */
      methods.add_method(
         "sendRaw",
         |_,
          this,
          (name, data0, data1, data2, data3): (
            BorrowedStr,
            mlua::Value,
            Option<mlua::Value>,
            Option<mlua::Value>,
            Option<mlua::Value>,
         )|
          -> mlua::Result<()> { Ok(()) },
      );
      /*@
       * @brief Checks to see if a shader has a uniform.
       *
       *    @luatparam Shader shader Shader to send uniform to.
       *    @luatparam string name Name of the uniform to check.
       *    @luatreturn boolean true if the shader has the uniform.
       * @luafunc hasUniform
       */
      methods.add_method(
         "hasUniform",
         |_, this, name: BorrowedStr| -> mlua::Result<bool> {
            Ok(this.uniforms.exact_match(&name).is_some())
         },
      );
      /*@
       * @brief Sets a shader as a post-processing shader.
       *
       *    @luatparam Shader shader Shader to set as a post-processing shader.
       *    @luatparam[opt="final"] string layer Layer to add the shader to.
       *    @luatparam[opt=0] number priority Priority of the shader to set. Higher
       * values mean it is run later.
       *    @luatreturn boolean true on success.
       * @luafunc addPPShader
       */
      methods.add_method_mut(
         "addPPShader",
         |_, _this, (layer, priority): (Option<BorrowedStr>, Option<i32>)| -> mlua::Result<bool> {
            let _priority = priority.unwrap_or(0);
            let _layer = match layer.as_deref() {
               None | Some("final") => naevc::PP_LAYER_FINAL,
               Some("game") => naevc::PP_LAYER_GAME,
               Some("gui") => naevc::PP_LAYER_GUI,
               Some("core") => naevc::PP_LAYER_CORE,
               Some(s) => {
                  return Err(mlua::Error::RuntimeError(format!("unknown layer '{}'", s)));
               }
            };
            Ok(false)
         },
      );
      /*@
       * @brief Removes a shader as a post-processing shader.
       *
       *    @luatparam Shader shader Shader to disable as post-processing shader.
       *    @luatreturn boolean True on success.
       * @luafunc rmPPShader
       */
      methods.add_method_mut("rmPPShader", |_, this, ()| -> mlua::Result<bool> {
         this.pp_id = None;
         Ok(true)
      });
   }
}
