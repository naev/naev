use std::ffi::CString;

pub fn message(msg: &str) {
   let msg = CString::new(msg).unwrap();
   unsafe {
      naevc::player_message(msg.as_ptr());
   }
}
