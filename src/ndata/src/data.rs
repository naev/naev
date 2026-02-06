use anyhow::Context;
use mlua::{BorrowedStr, UserData, UserDataMethods};

#[derive(Clone, derive_more::From, derive_more::Into)]
pub struct Data(Vec<f32>);

/*@
 * @brief Lua bindings to interact with data.
 *
 * @luamod data
 */
impl UserData for Data {
   /*
   fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
       //fields.add_field_method_get("w", |_, this| Ok(this.texture.w));
       //fields.add_field_method_get("h", |_, this| Ok(this.texture.h));
   }
   */
   fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
      /*@
       * @brief Creates a new data.
       *
       *    @luatparam number size Size to allocate for data.
       *    @luatparam string type Type of the data to create (`"number"`)
       *    @luatreturn Data New data object.
       * @luafunc new
       */
      methods.add_function(
         "new",
         |_, (size, data_type): (usize, BorrowedStr)| -> mlua::Result<Self> {
            if data_type != "number" {
               return Err(mlua::Error::RuntimeError(
                  "only 'number' type data supported at the moment".to_string(),
               ));
            }
            let mut v: Vec<f32> = Vec::with_capacity(size);
            v.resize(size, 0.0);
            Ok(v.into())
         },
      );
      /*@
       * @brief Gets the value of an element.
       *
       *    @luatparam Data data Data to index.
       *    @luatparam number pos Element position.
       *    @luatreturn number The entry.
       * @luafunc get
       */
      methods.add_method("get", |_, this, idx: usize| -> mlua::Result<f32> {
         Ok(*this.0.get(idx).context("out of bounds")?)
      });
      /*@
       * @brief Sets the value of an element.
       *
       *    @luatparam Data data Data to index.
       *    @luatparam number pos Element position.
       *    @luatparam number value Value to set it to.
       * @luafunc set
       */
      methods.add_method_mut(
         "set",
         |_, this, (idx, val): (usize, f32)| -> mlua::Result<()> {
            *this.0.get_mut(idx).context("out of bounds")? = val;
            Ok(())
         },
      );
      /*@
       * @brief Gets the number of elements.
       *
       *    @luatparam Data data ...
       *    @luatreturn number Data size.
       * @luafunc getSize
       */
      methods.add_method("getSize", |_, this, ()| -> mlua::Result<usize> {
         Ok(this.0.len())
      });
   }
}

pub fn open_data(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
   let proxy = lua.create_proxy::<Data>()?;
   Ok(proxy)
}
