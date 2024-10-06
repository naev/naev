use std::ffi::CStr;
use std::io;
use std::os::raw::{c_char, c_int};

use crate::ndata;

#[derive(Debug, Clone)]
pub struct SlotProperty {
    pub name: String,
    pub display: String,
    pub description: String,
    pub required: bool,
    pub exclusive: bool,
    pub locked: bool,
    pub visible: bool,
    //pub icon: glTexture,
}

static mut SPROPS: Vec<SlotProperty> = Vec::new();

#[no_mangle]
pub extern "C" fn _sp_load() -> c_int {
    match load() {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn _sp_cleanup() {}

#[no_mangle]
pub extern "C" fn _sp_get_c(name: *const c_char) -> c_int {
    unsafe {
        let ptr = CStr::from_ptr(name);
        let sname = ptr.to_str().unwrap();
        for (i, sp) in SPROPS.iter().enumerate() {
            if sp.name == sname {
                return i as c_int;
            }
        }
    }
    -1
}

#[no_mangle]
pub extern "C" fn _sp_display(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Ok(prop) => prop.display.as_ptr() as *const c_char,
        Err(_) => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn _sp_description(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Ok(prop) => prop.description.as_ptr() as *const c_char,
        Err(_) => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn _sp_visible(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.visible as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn _sp_required(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.required as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn _sp_exclusive(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.exclusive as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn _sp_locked(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.locked as c_int,
        Err(_) => 0,
    }
}

pub fn get_c(sp: c_int) -> std::io::Result<SlotProperty> {
    return unsafe { Ok(SPROPS[sp as usize].clone()) };
}

pub fn load() -> std::io::Result<()> {
    let data = ndata::read("dat/VERSION".to_string())?;
    println!("{}", String::from_utf8(data).unwrap());

    Err(io::Error::new(io::ErrorKind::Other, "Oops"))
}
