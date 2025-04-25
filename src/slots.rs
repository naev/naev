use anyhow::Result;
use rayon::prelude::*;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use crate::array::ArrayCString;
use crate::context::{Context, SafeContext};
use crate::gettext::gettext;
use crate::utils::{binary_search_by_key_ref, sort_by_key_ref};
use crate::warn;
use crate::{ndata, texture};
use crate::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};

#[derive(Default)]
pub struct SlotProperty {
    pub name: String,
    pub display: String,
    cdisplay: CString,
    pub description: String,
    cdescription: CString,
    pub required: bool,
    pub exclusive: bool,
    pub locked: bool,
    pub visible: bool,
    pub icon: Option<texture::Texture>,
    pub tags: Vec<String>,
    ctags: ArrayCString,
}
impl SlotProperty {
    fn load(ctx: &SafeContext, filename: &str) -> Result<Self> {
        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        let name = String::from(match root.attribute("name") {
            Some(n) => n,
            None => {
                return nxml_err_attr_missing!("Slot Property", "name");
            }
        });
        let mut sp = SlotProperty {
            name,
            ..SlotProperty::default()
        };
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "display" => {
                    sp.display = nxml::node_string(node)?;
                    sp.cdisplay = nxml::node_cstring(node)?;
                }
                "description" => {
                    sp.description = nxml::node_string(node)?;
                    sp.cdescription = nxml::node_cstring(node)?;
                }
                "required" => sp.required = true,
                "exclusive" => sp.exclusive = true,
                "locked" => sp.locked = true,
                "visible" => sp.visible = true,
                "icon" => {
                    let gfxname = nxml::node_texturepath(node, "gfx/slots/")?;
                    let nctx = ctx.lock();
                    sp.icon = Some(
                        texture::TextureBuilder::new()
                            .path(&gfxname)
                            .sdf(true)
                            .build(&nctx)?,
                    );
                }
                "tags" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        sp.tags.push(node.tag_name().name().to_lowercase());
                    }
                    // Remove when not needed for C interface
                    sp.ctags = ArrayCString::new(&sp.tags)?;
                }
                tag => nxml_warn_node_unknown!("Slot Property", &sp.name, tag),
            }
        }
        Ok(sp)
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

#[allow(dead_code)]
pub fn get(name: &str) -> Result<&'static SlotProperty> {
    match binary_search_by_key_ref(&SLOT_PROPERTIES, name, |sp: &SlotProperty| &sp.name) {
        Ok(i) => Ok(SLOT_PROPERTIES.get(i).expect("")),
        Err(_) => anyhow::bail!("Slot Property '{name}' not found .", name = &name,),
    }
}

pub fn load() -> Result<Vec<SlotProperty>> {
    let ctx = SafeContext::new(Context::get().unwrap());
    let files = ndata::read_dir("slots/")?;
    let mut sp_data: Vec<SlotProperty> = files
        .par_iter()
        .filter_map(|filename| {
            if !filename.ends_with(".xml") {
                return None;
            }
            match SlotProperty::load(&ctx, filename.as_str()) {
                Ok(sp) => Some(sp),
                _ => {
                    warn!("Unable to load Slot Property '{}'!", filename);
                    None
                }
            }
        })
        .collect();
    sort_by_key_ref(&mut sp_data, |sp: &SlotProperty| &sp.name);
    Ok(sp_data)
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_get(name: *const c_char) -> c_int {
    unsafe {
        let ptr = CStr::from_ptr(name);
        let name = ptr.to_str().unwrap();
        match binary_search_by_key_ref(&SLOT_PROPERTIES, name, |sp: &SlotProperty| &sp.name) {
            Ok(i) => (i + 1) as c_int,
            Err(_) => {
                warn!("slot property '{}' not found", name);
                0
            }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_display(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Some(prop) => prop.cdisplay.as_ptr(),
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_description(sp: c_int) -> *const c_char {
    match get_c(sp) {
        Some(prop) => prop.cdescription.as_ptr(),
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_visible(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.visible as c_int,
        None => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_required(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.required as c_int,
        None => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_exclusive(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.exclusive as c_int,
        None => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_icon(sp: c_int) -> *const naevc::glTexture {
    match get_c(sp) {
        Some(prop) => match &prop.icon {
            Some(icon) => icon as *const texture::Texture as *const naevc::glTexture,
            None => std::ptr::null(),
        },
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_locked(sp: c_int) -> c_int {
    match get_c(sp) {
        Some(prop) => prop.locked as c_int,
        None => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sp_tags(sp: c_int) -> *mut *const c_char {
    match get_c(sp) {
        // TODO implement
        Some(prop) => prop.ctags.as_ptr(),
        None => std::ptr::null_mut(),
    }
}

// Assume static here, because it doesn't really change after loading
fn get_c(sp: c_int) -> Option<&'static SlotProperty> {
    SLOT_PROPERTIES.get((sp - 1) as usize)
}
