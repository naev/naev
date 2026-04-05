use crate::array;

#[repr(transparent)]
pub struct SystemWrapper(naevc::StarSystem);
unsafe impl Send for SystemWrapper {}

impl SystemWrapper {
   pub fn as_ptr_mut(&mut self) -> *mut naevc::StarSystem {
      &mut self.0 as *mut naevc::StarSystem
   }

   pub fn presence(&self) -> &[naevc::SystemPresence] {
      let presences = self.0.presence;
      unsafe { array::array_as_slice::<naevc::SystemPresence>(presences) }
   }

   pub fn presence_mut(&mut self) -> &mut [naevc::SystemPresence] {
      let presences = self.0.presence;
      unsafe { array::array_as_slice_mut(presences) }
   }

   pub fn jumps(&self) -> &[naevc::JumpPoint] {
      let jumps = self.0.jumps;
      unsafe { array::array_as_slice(jumps) }
   }
}

pub fn get() -> &'static [SystemWrapper] {
   unsafe {
      let systems = naevc::system_getAll();
      array::array_as_slice(systems as *mut SystemWrapper)
   }
}

pub fn get_mut() -> &'static mut [SystemWrapper] {
   unsafe {
      let systems = naevc::system_getAll();
      array::array_as_slice_mut(systems as *mut SystemWrapper)
   }
}

unsafe extern "C-unwind" fn lua_system(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      if mlua::ffi::lua_isinteger(lua, 1) != 0 {
         let sys = mlua::ffi::lua_tointeger(lua, 1);
         naevc::lua_pushsystem(lua as *mut naevc::lua_State, sys as i32);
      } else {
         mlua::ffi::lua_pushstring(lua, c"lua_system recived a non-integer value!".as_ptr());
         mlua::ffi::lua_error(lua);
      }
   }
   1
}

pub fn to_lua(lua: &mlua::Lua, sys: *const naevc::StarSystem) -> mlua::Result<mlua::Value> {
   let f = match lua.named_registry_value::<mlua::Function>("lua_system") {
      Ok(f) => f,
      Err(_e) => {
         let f = unsafe { lua.create_c_function(lua_system)? };
         lua.set_named_registry_value("lua_system", f.clone())?;
         f
      }
   };
   let id = unsafe { naevc::system_index(sys) };
   f.call::<mlua::Value>(id)
}

unsafe extern "C-unwind" fn lua_system_index(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      let sysid = naevc::luaL_checksystem(lua as *mut naevc::lua_State, 1);
      mlua::ffi::lua_pushinteger(lua, sysid.into());
   }
   1
}
pub fn from_lua_index(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<i64> {
   let f = match lua.named_registry_value::<mlua::Function>("lua_system_index") {
      Ok(f) => f,
      Err(_e) => {
         let f = unsafe { lua.create_c_function(lua_system_index)? };
         lua.set_named_registry_value("lua_system_index", f.clone())?;
         f
      }
   };
   f.call::<i64>(value)
}
pub fn from_lua(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<&'static naevc::StarSystem> {
   let id = from_lua_index(lua, value)?;
   let sys = unsafe { naevc::system_getIndex(id as i32) };
   let sys = if sys.is_null() {
      return Err(mlua::Error::RuntimeError(
         "StarSystem not found".to_string(),
      ));
   } else {
      unsafe { &*sys }
   };
   Ok(sys)
}
