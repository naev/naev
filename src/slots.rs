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
    pub icon: *mut naevc::glTexture,
}

static mut SLOT_PROPERTIES: Vec<SlotProperty> = Vec::new();

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
        for (i, sp) in SLOT_PROPERTIES.iter().enumerate() {
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
pub extern "C" fn _sp_icon(sp: c_int) -> *const naevc::glTexture {
    match get_c(sp) {
        Ok(prop) => prop.icon,
        Err(_) => std::ptr::null(),
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
    return unsafe { Ok(SLOT_PROPERTIES[sp as usize].clone()) };
}

pub fn load() -> std::io::Result<()> {
    let f = ndata::File::open("dat/VERSION".to_string(), ndata::Mode::Read)?;
    let root = minidom::Element::from_reader(std::io::BufReader::new(f)).unwrap();
    let name = root.attr("name").unwrap().to_string();
    let mut sp = SlotProperty {
        name: name,
        display: "".to_string(),
        description: "".to_string(),
        required: false,
        exclusive: false,
        locked: false,
        visible: false,
        icon: std::ptr::null_mut(),
    };
    for node in root.children() {
        match node.name().to_lowercase().as_str() {
            "display" => sp.display = node.text(),
            "description" => sp.display = node.text(),
            "required" => sp.required = true,
            "exclusive" => sp.exclusive = true,
            "locked" => sp.locked = true,
            "visible" => sp.visible = true,
            "icon" => unsafe {
                sp.icon = naevc::gl_newImage(node.text().as_ptr() as *const c_char, 0)
            },
            &_ => todo!(),
        }
    }
    unsafe { SLOT_PROPERTIES.push(sp) };

    Err(io::Error::new(io::ErrorKind::Other, "Oops"))
}
