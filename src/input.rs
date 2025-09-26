use log::warn;
use sdl3 as sdl;

pub fn key_to_str(key: sdl::keyboard::Keycode) -> String {
    let name = key.name();
    if name.is_empty() {
        format!("SC-{}", key.to_ll())
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
            Ok(kc) => match sdl::keyboard::Keycode::from_i32(kc) {
                Some(kc) => Some(kc),
                None => {
                    return None;
                }
            },
            Err(_) => {
                return None;
            }
        };
    }
    sdl::keyboard::Keycode::from_name(name)
}

// Here be C API, yarr
use std::ffi::{CStr, c_char};

#[unsafe(no_mangle)]
pub extern "C" fn input_keyToStr(key: sdl::sys::keycode::SDL_Keycode) -> *const c_char {
    static mut KEYSTR: String = String::new();
    let keycode = match sdl::keyboard::Keycode::from_i32(key as i32) {
        Some(kc) => kc,
        None => {
            warn!("keycode {} not found!", key);
            return std::ptr::null();
        }
    };
    #[allow(static_mut_refs)]
    unsafe {
        KEYSTR = key_to_str(keycode);
        KEYSTR.as_ptr() as *const c_char
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn input_keyFromStr(name: *const c_char) -> sdl::sys::keycode::SDL_Keycode {
    let name = unsafe { CStr::from_ptr(name) };
    let key = match key_from_str(&name.to_string_lossy()) {
        Some(kc) => kc,
        None => sdl::keyboard::Keycode::Unknown,
    };
    key.to_ll()
}
