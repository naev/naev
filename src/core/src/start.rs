use anyhow::Result;
use gettext::gettext;
use log::{warn, warn_err};
use nalgebra::Vector2;
use serde::Deserialize;
use std::ffi::CString;
use std::os::raw::c_char;
use std::path::Path;

use crate::ntime::{NTime, NTimeC};
use crate::nxml;
use crate::nxml_warn_node_unknown;

// TODO get rid of CString and use String
#[derive(Default, Debug, Deserialize)]
pub struct StartData {
    #[serde(rename = "scenario_name")]
    pub name: CString,
    #[serde(rename = "ship_model")]
    pub ship: CString,
    #[serde(rename = "ship_name")]
    pub shipname: CString,
    #[serde(rename = "ship_acquired")]
    pub acquired: CString,
    #[serde(rename = "gui_default")]
    pub gui: CString,
    pub system: CString,
    #[serde(default)]
    pub mission: CString,
    #[serde(default)]
    pub event: CString,
    pub chapter: CString,
    pub spob_lua_default: CString,
    pub dtype_default: CString,
    pub local_map_default: CString,
    pub credits: i64,
    pub date: NTime,
    #[serde(rename = "system_position")]
    pub pos: Vector2<f64>,
}

macro_rules! nxml_attr_str {
    ($node: expr, $name: expr) => {
        match $node.attribute($name) {
            Some(val) => CString::new(val)?,
            None => {
                warn!(
                    "xml node '{}' missing attribute '{}'",
                    $node.tag_name().name(),
                    $name
                );
                CString::new(gettext("UNKNOWN"))?
            }
        }
    };
}
macro_rules! nxml_attr_i32 {
    ($node: expr, $name: expr) => {
        match $node.attribute($name) {
            Some(val) => val.parse::<i32>(),
            None => {
                warn!(
                    "xml node '{}' missing attribute '{}'",
                    $node.tag_name().name(),
                    $name
                );
                Ok(0)
            }
        }
    };
}

impl StartData {
    fn load() -> Result<Self> {
        if ndata::exists("start.xml") {
            warn("start.xml is deprecated, please use start.toml!");
            Self::load_xml("start.xml")
        } else {
            Self::load_toml("start.toml")
        }
    }

    fn load_toml<P: AsRef<Path>>(filename: P) -> Result<Self> {
        let data = ndata::read(filename)?;
        Ok(toml::from_slice(&data)?)
    }

    fn load_xml<P: AsRef<Path>>(filename: P) -> Result<Self> {
        let mut start: StartData = Default::default();

        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "name" => {
                    start.name = nxml::node_cstring(node)?;
                }
                "player" => {
                    for cnode in node.children() {
                        if !cnode.is_element() {
                            continue;
                        }
                        match cnode.tag_name().name().to_lowercase().as_str() {
                            "credits" => {
                                start.credits = nxml::node_str(cnode)?.parse()?;
                            }
                            "mission" => {
                                start.mission = nxml::node_cstring(cnode)?;
                            }
                            "event" => {
                                start.event = nxml::node_cstring(cnode)?;
                            }
                            "chapter" => {
                                start.chapter = nxml::node_cstring(cnode)?;
                            }
                            "gui" => {
                                start.gui = nxml::node_cstring(cnode)?;
                            }
                            "ship" => {
                                start.shipname = nxml_attr_str!(cnode, "name");
                                start.acquired = nxml_attr_str!(cnode, "acquired");
                                start.ship = nxml::node_cstring(cnode)?;
                            }
                            "system" => {
                                for ccnode in cnode.children() {
                                    if !ccnode.is_element() {
                                        continue;
                                    }
                                    match ccnode.tag_name().name().to_lowercase().as_str() {
                                        "name" => start.system = nxml::node_cstring(ccnode)?,
                                        "x" => {
                                            start.pos.x = nxml::node_str(ccnode)?.parse()?;
                                        }
                                        "y" => {
                                            start.pos.y = nxml::node_str(ccnode)?.parse()?;
                                        }
                                        tag => nxml_warn_node_unknown!(
                                            "Start/player/system",
                                            start.name.to_string_lossy(),
                                            tag
                                        ),
                                    }
                                }
                            }
                            tag => {
                                nxml_warn_node_unknown!(
                                    "Start/player",
                                    start.name.to_string_lossy(),
                                    tag
                                )
                            }
                        };
                    }
                }
                "date" => {
                    let scu = nxml_attr_i32!(node, "scu")?;
                    let stp = nxml_attr_i32!(node, "stp")?;
                    let stu = nxml_attr_i32!(node, "stu")?;
                    start.date = NTime::new(scu, stp, stu);
                }
                "spob_lua_default" => {
                    start.spob_lua_default = nxml::node_cstring(node)?;
                }
                "dtype_default" => {
                    start.dtype_default = nxml::node_cstring(node)?;
                }
                "local_map_default" => {
                    start.local_map_default = nxml::node_cstring(node)?;
                }
                tag => nxml_warn_node_unknown!("Start", start.name.to_string_lossy(), tag),
            };
        }
        Ok(start)
    }
}

use std::sync::LazyLock;
static START: LazyLock<StartData> = LazyLock::new(|| match StartData::load() {
    Ok(data) => data,
    Err(e) => {
        warn_err!(e);
        Default::default()
    }
});

pub fn start() -> &'static StartData {
    &START
}

macro_rules! start_c_func_str {
    ($funcname: ident, $field: ident) => {
        #[unsafe(no_mangle)]
        pub extern "C" fn $funcname() -> *const c_char {
            match START.$field.is_empty() {
                true => std::ptr::null_mut(),
                false => START.$field.as_ptr().into(),
            }
        }
    };
}
start_c_func_str!(start_name, name);
start_c_func_str!(start_ship, ship);
start_c_func_str!(start_shipname, shipname);
start_c_func_str!(start_acquired, acquired);
start_c_func_str!(start_gui, gui);
start_c_func_str!(start_system, system);
start_c_func_str!(start_mission, mission);
start_c_func_str!(start_event, event);
start_c_func_str!(start_chapter, chapter);
start_c_func_str!(start_spob_lua_default, spob_lua_default);
start_c_func_str!(start_dtype_default, dtype_default);
start_c_func_str!(start_local_map_default, local_map_default);
#[unsafe(no_mangle)]
pub extern "C" fn start_credits() -> i64 {
    START.credits
}
#[unsafe(no_mangle)]
pub extern "C" fn start_date() -> NTimeC {
    START.date.into()
}
#[unsafe(no_mangle)]
pub extern "C" fn start_position(x: *mut f64, y: *mut f64) {
    unsafe {
        *x = START.pos.x;
        *y = START.pos.y;
    }
}
