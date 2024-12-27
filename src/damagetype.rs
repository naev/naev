use anyhow::Result;
use rayon::prelude::*;
use std::ffi::{CStr, CString};
use std::io::{Error, ErrorKind};
use std::os::raw::{c_char, c_int};

use crate::gettext::gettext;
use crate::ndata;
use crate::{formatx, warn};
use crate::{nxml, nxml_err_attr_missing, nxml_err_node_unknown};

#[no_mangle]
pub extern "C" fn dtype_get(name: *const c_char) -> c_int {
    unsafe {
        let ptr = CStr::from_ptr(name);
        let name = CString::new(ptr.to_str().unwrap()).unwrap();
        let query = DamageType {
            name,
            ..DamageType::default()
        };
        match DAMAGE_TYPES.binary_search(&query) {
            Ok(i) => (i + 1) as c_int,
            Err(_) => 0,
        }
    }
}

// Assume static here, because it doesn't really change after loading
#[no_mangle]
pub fn get_c(dt: c_int) -> Option<&'static DamageType> {
    DAMAGE_TYPES.get(dt as usize)
}

#[no_mangle]
pub extern "C" fn dtype_damageTypeToStr(dtid: c_int) -> *const c_char {
    match get_c(dtid) {
        Some(dt) => match &dt.display {
            Some(d) => d.as_ptr(),
            None => dt.name.as_ptr(),
        },
        None => std::ptr::null(),
    }
}

#[no_mangle]
pub extern "C" fn dtype_raw(
    dtid: c_int,
    shield: *mut f64,
    armour: *mut f64,
    knockback: *mut f64,
) -> c_int {
    match get_c(dtid) {
        Some(dt) => {
            if shield != std::ptr::null_mut() {
                unsafe {
                    *shield = dt.shield_mod;
                }
            }
            if armour != std::ptr::null_mut() {
                unsafe {
                    *armour = dt.armour_mod;
                }
            }
            if knockback != std::ptr::null_mut() {
                unsafe {
                    *knockback = dt.knockback;
                }
            }
            0
        }
        None => -1,
    }
}

#[no_mangle]
pub extern "C" fn dtype_calcDamage(
    dshield: *mut f64,
    darmour: *mut f64,
    absorb: f64,
    knockback: *mut f64,
    dmg: *const naevc::Damage,
    _s: *const naevc::ShipStats,
) {
    /*
    if ( dshield != NULL ) {
       if ( ( dtype->soffset == 0 ) || ( s == NULL ) )
          *dshield = dtype->sdam * dmg->damage * absorb;
       else {
          ptr = (char *)s;
          memcpy( &multiplier, &ptr[dtype->soffset], sizeof( double ) );
          multiplier = MAX( 0., 1. - multiplier );
          *dshield   = dtype->sdam * dmg->damage * absorb * multiplier;
       }
    }
    if ( darmour != NULL ) {
       if ( ( dtype->aoffset ) == 0 || ( s == NULL ) )
          *darmour = dtype->adam * dmg->damage * absorb;
       else {
          ptr = (char *)s;
          memcpy( &multiplier, &ptr[dtype->aoffset], sizeof( double ) );
          multiplier = MAX( 0., 1. - multiplier );
          *darmour   = dtype->adam * dmg->damage * absorb * multiplier;
       }
    }
    if ( knockback != NULL )
       *knockback = dtype->knock;
     */
    match get_c(unsafe { (*dmg).type_ }) {
        Some(dt) => {
            if dshield != std::ptr::null_mut() {
                unsafe { *dshield = dt.shield_mod * (*dmg).damage * absorb }
            }
            if darmour != std::ptr::null_mut() {
                unsafe { *dshield = dt.armour_mod * (*dmg).damage * absorb }
            }
            if knockback != std::ptr::null_mut() {
                unsafe {
                    *knockback = dt.knockback;
                }
            }
        }
        None => (),
    }
}

#[derive(Debug, Clone)]
pub struct DamageType {
    name: CString,
    display: Option<CString>,
    shield_mod: f64,
    armour_mod: f64,
    knockback: f64,
    // TODO ship stat modifiers
}

impl DamageType {
    fn load(filename: &str) -> Result<Self> {
        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        let name = CString::new(match root.attribute("name") {
            Some(n) => n,
            None => {
                return nxml_err_attr_missing!("Damage Type", "name");
            }
        })?;
        let display = match root.attribute("display") {
            Some(n) => Some(CString::new(n)?),
            None => None,
        };
        let mut dt = DamageType {
            name,
            display,
            ..DamageType::default()
        };
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "shield" => dt.shield_mod = nxml::node_f64(node)?,
                "armour" => dt.armour_mod = nxml::node_f64(node)?,
                "knockback" => dt.knockback = nxml::node_f64(node)?,
                tag => {
                    return nxml_err_node_unknown!("Damage Type", dt.name.to_str()?, tag);
                }
            }
        }
        Ok(dt)
    }
}

impl Default for DamageType {
    fn default() -> Self {
        DamageType {
            name: CString::default(),
            display: None,
            shield_mod: 1.0,
            armour_mod: 1.0,
            knockback: 0.0,
        }
    }
}
impl Ord for DamageType {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.name.cmp(&other.name)
    }
}
impl PartialOrd for DamageType {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl PartialEq for DamageType {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}
impl Eq for DamageType {}

use std::sync::LazyLock;
static DAMAGE_TYPES: LazyLock<Vec<DamageType>> = LazyLock::new(|| load().unwrap());

pub fn load() -> Result<Vec<DamageType>> {
    let files = ndata::read_dir("damagetype/")?;
    let mut dt_data: Vec<DamageType> = files
        .par_iter()
        .filter_map(|filename| match DamageType::load(filename.as_str()) {
            Ok(dt) => Some(dt),
            _ => {
                warn!("Unable to load Damage Type '{}'!", filename);
                None
            }
        })
        .collect();
    dt_data.sort();
    Ok(dt_data)
}