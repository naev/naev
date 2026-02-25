#![allow(dead_code)]
use crate::shader::{ProgramBuilder, Shader};
use crate::texture::Texture;
use glow::*;
use mlua::{BorrowedStr, MetaMethod, UserData, UserDataMethods, UserDataRef};
use nalgebra::{Matrix2, Matrix3, Vector4};
use trie_rs::map::{Trie, TrieBuilder};

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

pub struct TextureSlot {
   pub active: u32,
   pub texid: u32,
   pub uniform: glow::NativeUniformLocation,
   pub value: i32,
}

pub struct LuaShader {
   pub shader: Shader,
   uniforms: Trie<u8, Uniform>,
   pub textures: Vec<TextureSlot>,
   pp_id: Option<u32>,
}
impl Drop for LuaShader {
   fn drop(&mut self) {
      if let Some(pp_id) = self.pp_id {
         unsafe { naevc::render_postprocessRm(pp_id) };
         self.pp_id = None;
      }
   }
}
impl LuaShader {
   const UNIFORM_BLOCK: u32 = 0;

   fn send_uniform(
      &mut self,
      name: &str,
      args: mlua::MultiValue,
      ignore_missing: bool,
   ) -> mlua::Result<()> {
      // Look up the uniform in the trie.
      let uniform = self.uniforms.exact_match(name.as_bytes());
      let Some(u) = uniform else {
         if ignore_missing {
            return Ok(());
         }
         return Err(mlua::Error::RuntimeError(format!(
            "Shader does not have uniform '{name}'!"
         )));
      };

      // Remaining arguments are the value(s).  Collect into a small Vec so we
      // can handle both the table form and the multi-argument form uniformly.
      let rest: Vec<mlua::Value> = args.into_iter().collect();

      let ctx = crate::Context::get();
      let gl = &ctx.gl;

      unsafe {
         gl.use_program(Some(self.shader.program));
      }

      match u.utype {
         // ── floats ──────────────────────────────────────────────────────────
         glow::FLOAT => {
            let v = parse_floats(&rest, 1)?;
            unsafe { gl.uniform_1_f32(Some(&u.location), v[0]) };
         }
         glow::FLOAT_VEC2 => {
            let v = parse_floats(&rest, 2)?;
            unsafe { gl.uniform_2_f32(Some(&u.location), v[0], v[1]) };
         }
         glow::FLOAT_VEC3 => {
            let v = parse_floats(&rest, 3)?;
            unsafe { gl.uniform_3_f32(Some(&u.location), v[0], v[1], v[2]) };
         }
         glow::FLOAT_VEC4 => {
            let v = parse_floats(&rest, 4)?;
            unsafe { gl.uniform_4_f32(Some(&u.location), v[0], v[1], v[2], v[3]) };
         }

         // ── ints ─────────────────────────────────────────────────────────────
         glow::INT | glow::BOOL => {
            let v = parse_ints(&rest, 1)?;
            unsafe { gl.uniform_1_i32(Some(&u.location), v[0]) };
         }
         glow::INT_VEC2 | glow::BOOL_VEC2 => {
            let v = parse_ints(&rest, 2)?;
            unsafe { gl.uniform_2_i32(Some(&u.location), v[0], v[1]) };
         }
         glow::INT_VEC3 | glow::BOOL_VEC3 => {
            let v = parse_ints(&rest, 3)?;
            unsafe { gl.uniform_3_i32(Some(&u.location), v[0], v[1], v[2]) };
         }
         glow::INT_VEC4 | glow::BOOL_VEC4 => {
            let v = parse_ints(&rest, 4)?;
            unsafe { gl.uniform_4_i32(Some(&u.location), v[0], v[1], v[2], v[3]) };
         }

         // ── sampler ──────────────────────────────────────────────────────────
         glow::SAMPLER_2D => {
            let slot_idx = u.tex.ok_or_else(|| {
               mlua::Error::RuntimeError(format!("sampler '{name}' has no texture slot"))
            })?;
            // Expect a Texture userdata value.
            let tex = match rest.first() {
               Some(mlua::Value::UserData(ud)) => ud.borrow::<Texture>().map_err(|_| {
                  mlua::Error::RuntimeError(format!("expected Texture for sampler '{name}'"))
               })?,
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "expected Texture for sampler '{name}'"
                  )));
               }
            };
            // Store the GL texture id for later binding (matches C's ls->tex[u->tex].texid = ...).
            self.textures[slot_idx as usize].texid = tex.texture.texture.0.into();
         }

         other => {
            nlog::warn!(
               "Unsupported shader uniform type '{}' for uniform '{}'. Ignoring.",
               other,
               u.name
            );
         }
      }

      unsafe {
         gl.use_program(None);
      }
      Ok(())
   }
}

/*@
 * @brief Lua bindings to interact with shaders.
 *
 * @lua_mod shader
 */
impl UserData for LuaShader {
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      methods.add_meta_function(
         MetaMethod::Eq,
         |_, (a, b): (UserDataRef<Self>, UserDataRef<Self>)| -> mlua::Result<bool> {
            Ok(a.shader.program == b.shader.program)
         },
      );

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
            let mut textures: Vec<TextureSlot> = Vec::new();
            for i in 0..n {
               if let Some(uniform) = unsafe { ctx.gl.get_active_uniform(shader.program, i as u32) }
                  && let Some(loc) =
                     unsafe { ctx.gl.get_uniform_location(shader.program, &uniform.name) }
               {
                  let tex = if uniform.utype == glow::SAMPLER_2D && uniform.name != "MainTex" {
                     ntex += 1;
                     let slot_idx = (ntex - 1) as usize;
                     textures.push(TextureSlot {
                        active: glow::TEXTURE0 + ntex as u32,
                        texid: 0,
                        uniform: loc,
                        value: ntex,
                     });
                     Some(slot_idx as u32)
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
      methods.add_method_mut(
         "send",
         |_, this, (name, args): (BorrowedStr, mlua::MultiValue)| -> mlua::Result<()> {
            this.send_uniform(&name, args, false)
         },
      );
      /*@
       * @brief Allows setting values of uniforms for a shader, while ignoring unknown
       * (or unused) uniforms.
       *
       *    @luatparam Shader shader Shader to send uniform to.
       *    @luatparam string name Name of the uniform.
       * @luafunc sendRaw
       */
      methods.add_method_mut(
         "sendRaw",
         |_, this, (name, args): (BorrowedStr, mlua::MultiValue)| -> mlua::Result<()> {
            this.send_uniform(&*name, args, true)
         },
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
            Ok(this.uniforms.exact_match(&*name).is_some())
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
         |_, this, (layer, priority): (Option<BorrowedStr>, Option<i32>)| -> mlua::Result<bool> {
            let priority = priority.unwrap_or(0);
            let layer = match layer.as_deref() {
               None | Some("final") => naevc::PP_LAYER_FINAL,
               Some("game") => naevc::PP_LAYER_GAME,
               Some("gui") => naevc::PP_LAYER_GUI,
               Some("core") => naevc::PP_LAYER_CORE,
               Some(s) => {
                  return Err(mlua::Error::RuntimeError(format!("unknown layer '{}'", s)));
               }
            };
            if this.pp_id.is_none() {
               let raw: *mut LuaShader = this as *mut LuaShader;
               let pp_id = unsafe {
                  naevc::render_postprocessAdd(
                     raw as *mut naevc::LuaShader_t,
                     layer as i32,
                     priority,
                     0,
                  )
               };
               if pp_id > 0 {
                  this.pp_id = Some(pp_id);
               }
               Ok(this.pp_id.is_some())
            } else {
               Ok(false)
            }
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
         if let Some(pp_id) = this.pp_id {
            let ret = unsafe { naevc::render_postprocessRm(pp_id) };
            this.pp_id = None;
            Ok(ret != 0)
         } else {
            Ok(false)
         }
      });
   }
}

fn parse_floats(args: &[mlua::Value], n: usize) -> mlua::Result<[f32; 4]> {
   let mut out = [0f32; 4];
   match args.first() {
      Some(mlua::Value::Table(t)) => {
         for i in 0..n {
            out[i] = t.get::<f32>((i + 1) as i64)?;
         }
      }
      _ => {
         for i in 0..n {
            out[i] = match args.get(i) {
               Some(mlua::Value::Number(f)) => *f as f32,
               Some(mlua::Value::Integer(v)) => *v as f32,
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "expected number at argument position {}",
                     i + 1
                  )));
               }
            };
         }
      }
   }
   Ok(out)
}

fn parse_ints(args: &[mlua::Value], n: usize) -> mlua::Result<[i32; 4]> {
   let mut out = [0i32; 4];
   match args.first() {
      Some(mlua::Value::Table(t)) => {
         for i in 0..n {
            out[i] = t.get::<i32>((i + 1) as i64)?;
         }
      }
      _ => {
         for i in 0..n {
            out[i] = match args.get(i) {
               Some(mlua::Value::Integer(v)) => *v as i32,
               Some(mlua::Value::Number(f)) => *f as i32,
               _ => {
                  return Err(mlua::Error::RuntimeError(format!(
                     "expected integer at argument position {}",
                     i + 1
                  )));
               }
            };
         }
      }
   }
   Ok(out)
}
