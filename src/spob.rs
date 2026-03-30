#![allow(dead_code)]

unsafe extern "C-unwind" fn lua_spob(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      if mlua::ffi::lua_isinteger(lua, 1) != 0 {
         let spb = mlua::ffi::lua_tointeger(lua, 1);
         naevc::lua_pushspob(lua as *mut naevc::lua_State, spb as i32);
      } else {
         mlua::ffi::lua_pushstring(lua, c"lua_spob recived a non-integer value!".as_ptr());
         mlua::ffi::lua_error(lua);
      }
   }
   1
}

pub fn to_lua(lua: &mlua::Lua, spb: *const naevc::Spob) -> mlua::Result<mlua::Value> {
   let f = match lua.named_registry_value::<mlua::Function>("lua_spob") {
      Ok(f) => f,
      Err(_e) => {
         let f = unsafe { lua.create_c_function(lua_spob)? };
         lua.set_named_registry_value("lua_spob", f.clone())?;
         f
      }
   };
   let id = unsafe { naevc::spob_index(spb) };
   f.call::<mlua::Value>(id)
}

unsafe extern "C-unwind" fn lua_spob_index(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      let spbid = naevc::luaL_checkspob(lua as *mut naevc::lua_State, 1);
      mlua::ffi::lua_pushinteger(lua, spbid.into());
   }
   1
}
pub fn from_lua_index(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<i64> {
   let f = match lua.named_registry_value::<mlua::Function>("lua_spob_index") {
      Ok(f) => f,
      Err(_e) => {
         let f = unsafe { lua.create_c_function(lua_spob_index)? };
         lua.set_named_registry_value("lua_spob_index", f.clone())?;
         f
      }
   };
   f.call::<i64>(value)
}
pub fn from_lua(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<&'static naevc::Spob> {
   let id = from_lua_index(lua, value)?;
   let spb = unsafe { naevc::spob_getIndex(id as i32) };
   let spb = if spb.is_null() {
      return Err(mlua::Error::RuntimeError("Spob not found".to_string()));
   } else {
      unsafe { &*spb }
   };
   Ok(spb)
}
