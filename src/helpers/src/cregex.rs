use regex::Regex;
use std::ffi::{CStr, c_char, c_int};

/// Opaque handle
pub struct CRegex(Regex);

/// Creates a new regex
#[unsafe(no_mangle)]
pub extern "C" fn cregex_new(pattern: *const c_char) -> *mut CRegex {
   if pattern.is_null() {
      return std::ptr::null_mut();
   }

   let c_str = unsafe { CStr::from_ptr(pattern) };
   let pattern_str = match c_str.to_str() {
      Ok(s) => s,
      Err(_) => return std::ptr::null_mut(),
   };

   match Regex::new(pattern_str) {
      Ok(re) => Box::into_raw(Box::new(CRegex(re))),
      Err(e) => {
         nlog::warn_err!(e);
         std::ptr::null_mut()
      }
   }
}

/// Returns 1 for match, 0 for no match, or -1 on error.
#[unsafe(no_mangle)]
pub extern "C" fn cregex_is_match(handle: *const CRegex, text: *const c_char) -> c_int {
   if handle.is_null() || text.is_null() {
      return -1;
   }

   let regex = unsafe { &*handle };
   let c_str = unsafe { CStr::from_ptr(text) };
   let text_str = match c_str.to_str() {
      Ok(s) => s,
      Err(_) => return -1,
   };

   regex.0.is_match(text_str) as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn cregex_free(handle: *mut CRegex) {
   if !handle.is_null() {
      unsafe {
         drop(Box::from_raw(handle));
      }
   }
}
