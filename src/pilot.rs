#![allow(dead_code)]
use crate::array;
use nalgebra::Vector2;
use std::ptr::NonNull;

#[repr(transparent)]
#[derive(Debug)]
pub struct PilotWrapper(pub NonNull<naevc::Pilot>);
unsafe impl Send for PilotWrapper {}

impl PilotWrapper {
   pub unsafe fn as_mut(&mut self) -> &mut naevc::Pilot {
      unsafe { self.0.as_mut() }
   }

   pub fn pos(&self) -> Vector2<f64> {
      let p = unsafe { self.0.as_ref() };
      Vector2::new(p.solid.pos.x, p.solid.pos.y)
   }
}

pub fn player() -> Option<PilotWrapper> {
   let p = unsafe { naevc::player.p };
   if p.is_null() {
      None
   } else {
      Some(PilotWrapper(NonNull::new(p).unwrap()))
   }
}

pub fn get() -> &'static [PilotWrapper] {
   unsafe {
      let pilots = naevc::pilot_getAll();
      array::array_as_slice(pilots as *mut PilotWrapper)
   }
}

pub fn get_mut() -> &'static mut [PilotWrapper] {
   unsafe {
      let pilots = naevc::pilot_getAll();
      array::array_as_slice_mut(pilots as *mut PilotWrapper)
   }
}

unsafe extern "C-unwind" fn lua_pilot(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      if mlua::ffi::lua_isinteger(lua, 1) != 0 {
         let plt = mlua::ffi::lua_tointeger(lua, 1);
         naevc::lua_pushsystem(lua as *mut naevc::lua_State, plt as i32);
      } else {
         mlua::ffi::lua_pushstring(lua, c"lua_pilot recived a non-integer value!".as_ptr());
         mlua::ffi::lua_error(lua);
      }
   }
   1
}

unsafe extern "C-unwind" fn lua_pilot_index(lua: *mut mlua::ffi::lua_State) -> i32 {
   unsafe {
      let pltid = naevc::luaL_checkpilot(lua as *mut naevc::lua_State, 1);
      mlua::ffi::lua_pushinteger(lua, pltid.into());
   }
   1
}
pub fn from_lua_index(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<i64> {
   let f = match lua.named_registry_value::<mlua::Function>("lua_pilot_index") {
      Ok(f) => f,
      Err(_e) => {
         let f = unsafe { lua.create_c_function(lua_pilot_index)? };
         lua.set_named_registry_value("lua_pilot_index", f.clone())?;
         f
      }
   };
   f.call::<i64>(value)
}
pub fn from_lua(lua: &mlua::Lua, value: &mlua::Value) -> mlua::Result<PilotWrapper> {
   let id = from_lua_index(lua, value)?;
   let plt = unsafe { naevc::pilot_get(id as u32) };
   let plt = if plt.is_null() {
      return Err(mlua::Error::RuntimeError("Pilot not found".to_string()));
   } else {
      PilotWrapper(NonNull::new(plt).unwrap())
   };
   Ok(plt)
}
