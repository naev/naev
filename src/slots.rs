use std::ffi::{CStr, CString};
use std::io::Result;
use std::os::raw::{c_char, c_int};

use crate::array;
use crate::ndata;

#[derive(Debug, Clone)]
pub struct SlotProperty {
    pub name: CString,
    pub display: CString,
    pub description: CString,
    pub required: bool,
    pub exclusive: bool,
    pub locked: bool,
    pub visible: bool,
    pub icon: *mut naevc::glTexture,
}
impl Default for SlotProperty {
    fn default() -> Self {
        SlotProperty {
            name: CString::default(),
            display: CString::default(),
            description: CString::default(),
            required: false,
            exclusive: false,
            locked: false,
            visible: false,
            icon: std::ptr::null_mut(),
        }
    }
}
impl Ord for SlotProperty {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        (&self.name).cmp(&(&other.name))
    }
}
impl PartialOrd for SlotProperty {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl PartialEq for SlotProperty {
    fn eq(&self, other: &Self) -> bool {
        &self.name == &other.name
    }
}
impl Eq for SlotProperty {}

static mut SLOT_PROPERTIES: Vec<SlotProperty> = Vec::new();

#[no_mangle]
pub extern "C" fn sp_load() -> c_int {
    match load() {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn sp_cleanup() {}

#[no_mangle]
pub extern "C" fn sp_get(name: *const c_char) -> c_int {
    unsafe {
        let ptr = CStr::from_ptr(name);
        let sname = CString::new(ptr.to_str().unwrap()).unwrap();
        let query = SlotProperty {
            name: sname,
            ..SlotProperty::default()
        };
        match SLOT_PROPERTIES.binary_search(&query) {
            Ok(i) => (i + 1) as c_int,
            Err(_) => 0,
        }
    }
}

#[no_mangle]
pub extern "C" fn sp_display(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Ok(prop) => prop.display.as_ptr() as *const c_char,
        Err(_) => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_description(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Ok(prop) => prop.description.as_ptr() as *const c_char,
        Err(_) => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_visible(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.visible as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_required(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.required as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_exclusive(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.exclusive as c_int,
        Err(_) => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_icon(sp: c_int) -> *const naevc::glTexture {
    match get_c(sp) {
        Ok(prop) => prop.icon,
        Err(_) => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_locked(sp: c_int) -> c_int {
    match get_c(sp) {
        Ok(prop) => prop.locked as c_int,
        Err(_) => 0,
    }
}

// Assume static here, because it doesn't really change after loading
pub fn get_c(sp: c_int) -> Result<&'static SlotProperty> {
    unsafe {
        match SLOT_PROPERTIES.get((sp - 1) as usize) {
            Some(sp) => Ok(sp),
            None => Err(std::io::Error::new(std::io::ErrorKind::Other, "")),
        }
    }
}

#[allow(dead_code)]
pub fn get(name: CString) -> Result<&'static SlotProperty> {
    unsafe {
        let query = SlotProperty {
            name: name,
            ..SlotProperty::default()
        };
        match SLOT_PROPERTIES.binary_search(&query) {
            Ok(i) => Ok(&SLOT_PROPERTIES[i]),
            Err(_) => Err(std::io::Error::new(std::io::ErrorKind::Other, "")),
        }
    }
    // WARN( _( "Slot property '%s' not found in array." ), name );
}

pub fn load() -> Result<()> {
    let sp_files = unsafe { naevc::ndata_listRecursive(naevc::SP_DATA_PATH.as_ptr().cast()) };
    let sp_array = array::to_vec(sp_files)?;
    let mut files: Vec<String> = Vec::new();
    for sp in sp_array {
        files.push(unsafe { CStr::from_ptr(sp).to_str().unwrap().to_string() });
    }

    for filename in files {
        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data).unwrap()).unwrap();
        let root = doc.root_element();
        let name = CString::new(root.attribute("name").unwrap())?;
        let mut sp = SlotProperty {
            name: name,
            ..SlotProperty::default()
        };
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "display" => sp.display = CString::new(node.text().unwrap())?,
                "description" => sp.description = CString::new(node.text().unwrap())?,
                "required" => sp.required = true,
                "exclusive" => sp.exclusive = true,
                "locked" => sp.locked = true,
                "visible" => sp.visible = true,
                "icon" => unsafe {
                    let gfxname = CString::new(format!("gfx/slots/{}", node.text().unwrap()))?;
                    sp.icon = naevc::gl_newImage(gfxname.as_ptr() as *const c_char, 0)
                },
                i => todo!("not found: {}", i),
            }
        }
        //println!("{:?}", sp);
        unsafe { SLOT_PROPERTIES.push(sp) };
    }
    unsafe { SLOT_PROPERTIES.sort() };
    Ok(())
}
