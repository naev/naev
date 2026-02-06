use anyhow::Result;
use nlog::{warn, warn_err};
use sdl3 as sdl;

pub fn key_to_str(key: sdl::keyboard::Keycode) -> String {
   let name = key.name();
   if name.is_empty() {
      format!("SC-{}", key.to_ll().0)
   } else {
      name
   }
}

pub fn key_from_str(name: &str) -> Option<sdl::keyboard::Keycode> {
   if name.starts_with("SC-") {
      let name = match name.get(2..) {
         Some(n) => n,
         None => {
            return None;
         }
      };
      return match name.parse::<i32>() {
         Ok(kc) => sdl::keyboard::Keycode::from_i32(kc),
         Err(_) => None,
      };
   }
   sdl::keyboard::Keycode::from_name(name)
}

// Here be C API, yarr
use sdl::sys::keycode::SDL_Keycode;
//use sdl::sys::events::SDL_Event;
use sdl::event::Event;
use std::collections::HashMap;
use std::ffi::{CStr, CString, c_char};
use std::sync::{LazyLock, Mutex};

pub fn _input_handle(
   sdlctx: &sdl::Sdl,
   sdlvid: &sdl::VideoSubsystem,
   sdlevt: &sdl::EventSubsystem,
   event: Event,
) -> Result<()> {
   // Handle mouse motion
   if event.is_mouse() {
      //unsafe { input_mouseTimer = conf.mouse_hide; }
      sdlctx.mouse().show_cursor(true);
   };

   // Copy and Paste support
   if let Event::KeyDown { .. } = event {
      let clipboard = sdlvid.clipboard();
      if clipboard.has_clipboard_text() {
         let txtevent = Event::TextInput {
            timestamp: sdl::timer::ticks(),
            window_id: 0,
            text: clipboard.clipboard_text()?,
         };
         sdlevt.push_event(txtevent)?;
         return Ok(());
      }
   }

   Ok(())
}

#[unsafe(no_mangle)]
pub extern "C" fn input_keyToStr(key: SDL_Keycode) -> *const c_char {
   static KEYSTR: LazyLock<Mutex<HashMap<SDL_Keycode, CString>>> =
      LazyLock::new(|| Mutex::new(HashMap::new()));
   let mut keystr = KEYSTR.lock().unwrap();
   if let Some(name) = keystr.get(&key) {
      return name.as_ptr() as *const c_char;
   }

   let keycode = match sdl::keyboard::Keycode::from_i32(key.0 as i32) {
      Some(kc) => kc,
      None => {
         warn!("keycode '{}' not found!", key.0);
         return std::ptr::null();
      }
   };
   let name = key_to_str(keycode);
   keystr.insert(
      key,
      match CString::new(name) {
         Ok(name) => name,
         Err(e) => {
            warn_err!(e);
            c"Unknown".into()
         }
      },
   );
   keystr[&key].as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn input_keyFromStr(name: *const c_char) -> SDL_Keycode {
   if name.is_null() {
      return sdl::keyboard::Keycode::Unknown.to_ll();
   }
   let name = unsafe { CStr::from_ptr(name) };
   let key = match key_from_str(&name.to_string_lossy()) {
      Some(kc) => kc,
      None => sdl::keyboard::Keycode::Unknown,
   };
   key.to_ll()
}
