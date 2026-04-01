#![allow(dead_code)]
use nalgebra::Vector2;
use std::ffi::c_void;
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
      let pilots = naevc::ship_getAll();
      let n = naevc::array_size_rust(pilots as *const c_void) as usize;
      std::slice::from_raw_parts(pilots as *const PilotWrapper, n)
   }
}

pub fn get_mut() -> &'static mut [PilotWrapper] {
   unsafe {
      let pilots = naevc::pilot_getAll();
      let n = naevc::array_size_rust(pilots as *const c_void) as usize;
      std::slice::from_raw_parts_mut(pilots as *mut PilotWrapper, n)
   }
}
