use crate::gettext::gettext;
use crate::ndata;
use crate::ntime::{NTime, NTimeC};
use crate::nxml_err_node_unknown;
use std::ffi::CString;
use std::io::{Error, ErrorKind, Result};
use std::os::raw::{c_char, c_int};

#[derive(Default, Debug)]
struct StartData {
    name: CString,
    ship: CString,
    shipname: CString,
    acquired: CString,
    gui: CString,
    system: CString,
    mission: CString,
    event: CString,
    chapter: CString,
    spob_lua_default: CString,
    dtype_default: CString,
    local_map_default: CString,
    credits: i64,
    date: NTime,
    pos_x: f64,
    pos_y: f64,
}

macro_rules! nxml_attr_str {
    ($node: expr, $name: expr) => {
        CString::new($node.attribute($name).unwrap())?
    };
}
macro_rules! nxml_attr_i32 {
    ($node: expr, $name: expr) => {
        $node.attribute($name).unwrap().parse::<i32>().unwrap()
    };
}
macro_rules! nxml_str {
    ($node: expr) => {
        CString::new($node.text().unwrap())?
    };
}
macro_rules! nxml_f64 {
    ($node: expr) => {
        $node.text().unwrap().parse::<f64>().unwrap()
    };
}

impl StartData {
    fn load() -> Result<Self> {
        let mut start: StartData = Default::default();

        let data = ndata::read(String::from("start.xml"))?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data).unwrap()).unwrap();
        let root = doc.root_element();
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "name" => {
                    start.name = nxml_str!(node);
                }
                "player" => {
                    for cnode in node.children() {
                        if !cnode.is_element() {
                            continue;
                        }
                        match cnode.tag_name().name().to_lowercase().as_str() {
                            "credits" => {
                                start.credits = cnode.text().unwrap().parse().unwrap();
                            }
                            "mission" => {
                                start.mission = nxml_str!(cnode);
                            }
                            "event" => {
                                start.event = nxml_str!(cnode);
                            }
                            "chapter" => {
                                start.chapter = nxml_str!(cnode);
                            }
                            "gui" => {
                                start.gui = nxml_str!(cnode);
                            }
                            "ship" => {
                                start.shipname = nxml_attr_str!(cnode, "name");
                                start.acquired = nxml_attr_str!(cnode, "acquired");
                                start.ship = nxml_str!(cnode);
                            }
                            "system" => {
                                for ccnode in cnode.children() {
                                    if !ccnode.is_element() {
                                        continue;
                                    }
                                    match ccnode.tag_name().name().to_lowercase().as_str() {
                                        "name" => start.system = nxml_str!(ccnode),
                                        "x" => {
                                            start.pos_x = nxml_f64!(ccnode);
                                        }
                                        "y" => {
                                            start.pos_y = nxml_f64!(ccnode);
                                        }
                                        tag => {
                                            return nxml_err_node_unknown!(
                                                "Start/player/system",
                                                start.name.to_str().unwrap(),
                                                tag
                                            );
                                        }
                                    }
                                }
                            }
                            tag => {
                                return nxml_err_node_unknown!(
                                    "Start/player",
                                    start.name.to_str().unwrap(),
                                    tag
                                );
                            }
                        };
                    }
                }
                "date" => {
                    let scu = nxml_attr_i32!(node, "scu");
                    let stp = nxml_attr_i32!(node, "stp");
                    let stu = nxml_attr_i32!(node, "stu");
                    start.date = NTime::new(scu, stp, stu);
                }
                "spob_lua_default" => {
                    start.spob_lua_default = nxml_str!(node);
                }
                "dtype_default" => {
                    start.dtype_default = nxml_str!(node);
                }
                "local_map_default" => {
                    start.local_map_default = nxml_str!(node);
                }
                tag => {
                    return nxml_err_node_unknown!("Start", start.name.to_str().unwrap(), tag);
                }
            };
        }
        Ok(start)
    }
}

use std::sync::LazyLock;
static START: LazyLock<StartData> = LazyLock::new(|| StartData::load().unwrap());

macro_rules! start_c_func_str {
    ($funcname: ident, $field: ident) => {
        #[no_mangle]
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
#[no_mangle]
pub extern "C" fn start_credits() -> i64 {
    START.credits
}
#[no_mangle]
pub extern "C" fn start_date() -> NTimeC {
    START.date.into()
}
#[no_mangle]
pub extern "C" fn start_position(x: *mut f64, y: *mut f64) {
    unsafe {
        *x = START.pos_x;
        *y = START.pos_y;
    }
}
#[no_mangle]
pub extern "C" fn start_load() -> c_int {
    dbg!("load");
    let _ = START;
    0
}
