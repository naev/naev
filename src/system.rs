use crate::array;

#[repr(transparent)]
pub struct SystemWrapper(naevc::StarSystem);
unsafe impl Send for SystemWrapper {}

/// Small wrapper for systems until it gets moved to Rust
impl SystemWrapper {
   pub fn as_ptr_mut(&mut self) -> *mut naevc::StarSystem {
      &mut self.0 as *mut naevc::StarSystem
   }

   pub fn presence(&self) -> &[naevc::SystemPresence] {
      unsafe { array::array_as_slice(self.0.presence) }
   }

   pub fn presence_mut(&mut self) -> &mut [naevc::SystemPresence] {
      unsafe { array::array_as_slice_mut(self.0.presence) }
   }

   pub fn jumps(&self) -> &[naevc::JumpPoint] {
      unsafe { array::array_as_slice(self.0.jumps) }
   }

   pub fn astexclude(&self) -> &[naevc::AsteroidExclusion] {
      unsafe { array::array_as_slice(self.0.astexclude) }
   }

   pub fn asteroids(&self) -> &[naevc::AsteroidAnchor] {
      unsafe { array::array_as_slice(self.0.asteroids) }
   }

   pub fn asteroids_mut(&mut self) -> &mut [naevc::AsteroidAnchor] {
      unsafe { array::array_as_slice_mut(self.0.asteroids) }
   }
}

/// Gets the current system if applicable
pub fn cur() -> Option<&'static SystemWrapper> {
   if unsafe { naevc::cur_system.is_null() } {
      None
   } else {
      Some(unsafe { &*(naevc::cur_system as *const SystemWrapper) })
   }
}

/// Gets the current system mutably if applicable
pub fn cur_mut() -> Option<&'static mut SystemWrapper> {
   if unsafe { naevc::cur_system.is_null() } {
      None
   } else {
      Some(unsafe { &mut *(naevc::cur_system as *mut SystemWrapper) })
   }
}

/// Gets all the systems
pub fn get() -> &'static [SystemWrapper] {
   unsafe {
      let systems = naevc::system_getAll();
      array::array_as_slice(systems as *mut SystemWrapper)
   }
}

/// Gets all the systems mutably
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
