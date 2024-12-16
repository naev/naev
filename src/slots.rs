use anyhow::Result;
use rayon::prelude::*;
use std::ffi::{CStr, CString};
use std::io::{Error, ErrorKind};
use std::os::raw::{c_char, c_int};

use crate::gettext::gettext;
use crate::ndata;
use crate::{formatx, warn};
use crate::{nxml, nxml_err_attr_missing, nxml_err_node_unknown};

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
impl SlotProperty {
    fn load(filename: &str) -> Result<Self> {
        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        let name = CString::new(match root.attribute("name") {
            Some(n) => n,
            None => {
                return nxml_err_attr_missing!("Slot property", "name");
            }
        })?;
        let mut sp = SlotProperty {
            name,
            ..SlotProperty::default()
        };
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "display" => sp.display = nxml::node_cstring(node)?,
                "description" => sp.description = nxml::node_cstring(node)?,
                "required" => sp.required = true,
                "exclusive" => sp.exclusive = true,
                "locked" => sp.locked = true,
                "visible" => sp.visible = true,
                "icon" => unsafe {
                    let gfxname = CString::new(format!("gfx/slots/{}", nxml::node_str(node)?))?;
                    sp.icon = naevc::gl_newImage(gfxname.as_ptr() as *const c_char, 0)
                },
                tag => {
                    return nxml_err_node_unknown!("Slot property", sp.name.to_str()?, tag);
                }
            }
        }
        Ok(sp)
    }
}
// Implementation of glTexture should be fairly thread safe, so set properties
unsafe impl Sync for SlotProperty {}
unsafe impl Send for SlotProperty {}
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
        self.name.cmp(&other.name)
    }
}
impl PartialOrd for SlotProperty {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl PartialEq for SlotProperty {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}
impl Eq for SlotProperty {}

use std::sync::LazyLock;
static SLOT_PROPERTIES: LazyLock<Vec<SlotProperty>> = LazyLock::new(|| load().unwrap());

#[no_mangle]
pub extern "C" fn sp_load() -> c_int {
    let _ = SLOT_PROPERTIES; // Should trigger a load, not necessary though
    0
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
        Some(prop) => prop.display.as_ptr(),
        None => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_description(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Some(prop) => prop.description.as_ptr(),
        None => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_visible(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.visible as c_int,
        None => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_required(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.required as c_int,
        None => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_exclusive(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.exclusive as c_int,
        None => 0,
    }
}

#[no_mangle]
pub extern "C" fn sp_icon(sp: c_int) -> *const naevc::glTexture {
    match get_c(sp) {
        Some(prop) => prop.icon,
        None => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn sp_locked(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.locked as c_int,
        None => 0,
    }
}

// Assume static here, because it doesn't really change after loading
pub fn get_c(sp: c_int) -> Option<&'static SlotProperty> {
    SLOT_PROPERTIES.get((sp - 1) as usize)
}

#[allow(dead_code)]
pub fn get(name: CString) -> Result<&'static SlotProperty> {
    let query = SlotProperty {
        name,
        ..SlotProperty::default()
    };
    let props = &SLOT_PROPERTIES;
    match props.binary_search(&query) {
        Ok(i) => Ok(props.get(i).expect("")),
        Err(_) => anyhow::bail!(
            "Slot property '{name}' not found .",
            name = query.name.to_str()?
        ),
    }
}

pub fn load() -> Result<Vec<SlotProperty>> {
    let files = ndata::read_dir("slots/")?;

    let mut sp_data: Vec<SlotProperty> = files
        .par_iter()
        .filter_map(|filename| match SlotProperty::load(filename.as_str()) {
            Ok(sp) => Some(sp),
            _ => {
                warn!("Unable to load Slot Property '{}'!", filename);
                None
            }
        })
        .collect();
    sp_data.sort();

    Ok(sp_data)
}
