#![allow(dead_code)]
use nalgebra::Vector2;
use std::ffi::c_void;

pub struct PilotWrapper(pub &'static mut naevc::Pilot);
unsafe impl Send for PilotWrapper {}

impl PilotWrapper {
   pub fn pos(&self) -> Vector2<f64> {
      Vector2::new(self.0.solid.pos.x, self.0.solid.pos.y)
   }
}

pub fn player() -> Option<PilotWrapper> {
   let p = unsafe { naevc::player.p };
   if p.is_null() {
      None
   } else {
      Some(PilotWrapper(unsafe { &mut *p }))
   }
}

pub fn get() -> &'static [PilotWrapper] {
   unsafe {
      let pilots = naevc::ship_getAll();
      let n = naevc::array_size_rust(pilots as *const c_void) as usize;
      std::slice::from_raw_parts(pilots as *const PilotWrapper, n)
   }
}

pub fn get_mut() -> &'static mut [PilotWrapper] {
   unsafe {
      let pilots = naevc::ship_getAll();
      let n = naevc::array_size_rust(pilots as *const c_void) as usize;
      std::slice::from_raw_parts_mut(pilots as *mut PilotWrapper, n)
   }
}
