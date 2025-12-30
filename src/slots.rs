use crate::array::ArrayCString;
use crate::warn;
use anyhow::Context as AnyhowContext;
use anyhow::Result;
use helpers::{binary_search_by_key_ref, sort_by_key_ref};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use nlog::warn_err;
use rayon::prelude::*;
use renderer::texture::TextureDeserializer;
use renderer::{Context, ContextWrapper, texture};
use serde_seeded::DeserializeSeeded;
use std::ffi::{CStr, CString, OsStr};
use std::os::raw::{c_char, c_int};
use std::path::{Path, PathBuf};

#[derive(Default, DeserializeSeeded, Debug)]
#[seeded(de(seed(TextureDeserializer<'a>),params('a)))]
pub struct SlotProperty {
    pub name: String,
    pub display: String,
    pub description: String,
    #[seeded(default)]
    pub required: bool,
    #[seeded(default)]
    pub exclusive: bool,
    #[seeded(default)]
    pub locked: bool,
    #[seeded(default)]
    pub visible: bool,
    #[seeded(default)]
    pub icon: Option<texture::Texture>,
    #[seeded(default)]
    pub tags: Vec<String>,
    // C stuff to remove in the future
    #[seeded(skip, default)]
    cdisplay: CString,
    #[seeded(skip, default)]
    cdescription: CString,
    #[seeded(skip, default)]
    ctags: ArrayCString,
}

impl SlotProperty {
    fn load<P: AsRef<Path>>(texde: &TextureDeserializer, filename: P) -> Option<Result<Self>> {
        let sp = {
            let ext = filename.as_ref().extension();
            if ext == Some(OsStr::new("toml")) {
                Self::load_toml(texde, &filename)
            } else if ext == Some(OsStr::new("xml")) {
                Self::load_xml(&texde.ctx, &filename)
            } else {
                return None;
            }
        }
        .with_context(|| {
            format!(
                "unable to load SlotProperty '{}'",
                filename.as_ref().display()
            )
        });
        Some(sp)
    }

    fn load_toml<P: AsRef<Path>>(texde: &TextureDeserializer, filename: P) -> Result<Self> {
        let data = ndata::read_to_string(filename)?;
        let mut sp: SlotProperty =
            SlotProperty::deserialize_seeded(texde, toml::de::Deserializer::parse(&data)?)?;
        // Have to post-process the C stuff for now
        sp.cdisplay = CString::new(sp.display.as_str())?;
        sp.cdescription = CString::new(sp.description.as_str())?;
        sp.ctags = ArrayCString::new(&sp.tags)?;
        Ok(sp)
    }

    fn load_xml<P: AsRef<Path>>(ctx: &ContextWrapper, filename: P) -> Result<Self> {
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
                    sp.icon = Some(
                        texture::TextureBuilder::new()
                            .path(&gfxname)
                            .sdf(true)
                            .build_wrap(ctx)?,
                    );
                }
                "tags" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        if let Some(t) = node.text() {
                            sp.tags.push(String::from(t));
                        }
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
    fn sdf_texture(ctx: &ContextWrapper, path: &str) -> Result<texture::Texture> {
        let path = match path.starts_with('/') {
            true => path,
            false => &format!("gfx/slots/{}", path),
        };
        texture::TextureBuilder::new()
            .path(path)
            .sdf(true)
            .build_wrap(ctx)
    }
    let texde = TextureDeserializer {
        ctx: Context::get().as_safe_wrap(),
        func: Box::new(sdf_texture),
    };
    let base: PathBuf = "slots/".into();
    let mut sp_data: Vec<SlotProperty> = ndata::read_dir(&base)?
        .par_iter()
        .filter_map(
            |filename| match SlotProperty::load(&texde, &base.join(filename)) {
                Some(Ok(sp)) => Some(sp),
                Some(Err(e)) => {
                    warn_err!(e);
                    None
                }
                None => None,
            },
        )
        .collect();
    sort_by_key_ref(&mut sp_data, |sp: &SlotProperty| &sp.name);
    sp_data.windows(2).for_each(|w| {
        if w[0].name == w[1].name {
            warn!("Slot property '{}' loaded twice!", w[0].name);
        }
    });
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
                warn!("slot property '{name}' not found");
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
        Some(prop) => prop.ctags.as_ptr(),
        None => std::ptr::null_mut(),
    }
}

// Assume static here, because it doesn't really change after loading
fn get_c(sp: c_int) -> Option<&'static SlotProperty> {
    SLOT_PROPERTIES.get((sp - 1) as usize)
}
