use anyhow::Result;
use rayon::prelude::*;
use serde::{Deserialize, Deserializer, de};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use log::{warn, warn_err};
use naev_core::utils::{binary_search_by_key_ref, sort_by_key_ref};
use naev_core::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};

#[derive(Debug, Clone, Copy)]
struct Stat(usize);
impl<'de> Deserialize<'de> for Stat {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        //let name = String::deserialize(deserializer)?;
        //let cname = CString::new(&*name).unwrap();
        let cname = CString::deserialize(deserializer)?;
        let sstype = unsafe { naevc::ss_typeFromName(cname.as_ptr()) };
        if sstype == naevc::ShipStatsType_SS_TYPE_NIL {
            return Err(de::Error::custom(format!(
                "ship stat '{}' not found",
                cname.to_string_lossy()
            )));
        }
        let o = unsafe { naevc::ss_offsetFromType(sstype) };
        Ok(Stat(o))
    }
}

#[derive(Debug, Clone, Deserialize)]
pub struct DamageType {
    name: String,
    display: Option<String>,
    #[serde(rename = "shield")]
    shield_mod: f64,
    #[serde(rename = "armour")]
    armour_mod: f64,
    knockback: f64,
    // Hack for C to be rustified when shipstats get ported
    shield_stat: Option<Stat>,
    armour_stat: Option<Stat>,
    #[serde(skip, default)]
    cname: CString,
    #[serde(skip, default)]
    cdisplay: Option<CString>,
}

impl DamageType {
    fn load_xml(filename: &str) -> Result<Self> {
        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        let name = String::from(match root.attribute("name") {
            Some(n) => n,
            None => {
                return nxml_err_attr_missing!("Damage Type", "name");
            }
        });
        let cname = CString::new(name.clone())?;
        let display = match root.attribute("display") {
            Some(n) => Some(n.to_string()),
            None => None,
        };
        let mut dt = DamageType {
            name,
            cname,
            display,
            ..DamageType::default()
        };
        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            fn stat_offset(name: &str) -> Option<Stat> {
                let cname = CString::new(name).unwrap();
                let sstype = unsafe { naevc::ss_typeFromName(cname.as_ptr()) };
                if sstype == naevc::ShipStatsType_SS_TYPE_NIL {
                    return None;
                }
                let o = unsafe { naevc::ss_offsetFromType(sstype) };
                Some(Stat(o))
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "shield" => {
                    dt.shield_stat = node.attribute("stat").and_then(stat_offset);
                    dt.shield_mod = nxml::node_f64(node)?
                }
                "armour" => {
                    dt.armour_stat = node.attribute("stat").and_then(stat_offset);
                    dt.armour_mod = nxml::node_f64(node)?
                }
                "knockback" => dt.knockback = nxml::node_f64(node)?,
                tag => nxml_warn_node_unknown!("Damage Type", &dt.name, tag),
            }
        }
        Ok(dt)
    }
}

impl Default for DamageType {
    fn default() -> Self {
        DamageType {
            name: String::default(),
            cname: CString::default(),
            display: None,
            cdisplay: None,
            shield_mod: 1.0,
            armour_mod: 1.0,
            knockback: 0.0,
            shield_stat: None,
            armour_stat: None,
        }
    }
}

use std::sync::LazyLock;
static DAMAGE_TYPES: LazyLock<Vec<DamageType>> = LazyLock::new(|| load().unwrap());

pub fn load() -> Result<Vec<DamageType>> {
    fn load_toml(filename: &str) -> Result<DamageType> {
        let data = ndata::read(filename)?;
        let mut dt: DamageType = toml::from_slice(&data)?;
        dt.cname = CString::new(&*dt.name)?;
        dt.cdisplay = dt.display.as_ref().and_then(|s| CString::new(&**s).ok());
        Ok(dt)
    }
    let mut dt_data: Vec<DamageType> = ndata::read_dir("damagetype/")?
        .par_iter()
        .filter_map(|filename| {
            if filename.ends_with(".toml") {
                match load_toml(filename.as_str()) {
                    Ok(dt) => Some(dt),
                    Err(e) => {
                        warn_err!(e); //rr.context(format!("unable to load Damage Type '{filename}'!")));
                        None
                    }
                }
            // TODO remove XMl support at around 0.14.0 or 0.15.0
            } else if filename.ends_with(".xml") {
                match DamageType::load_xml(filename.as_str()) {
                    Ok(dt) => Some(dt),
                    Err(err) => {
                        warn_err(err.context(format!("unable to load Damage Type '{filename}'!")));
                        None
                    }
                }
            } else {
                None
            }
        })
        .collect();
    // Special type of unmodified damage
    dt_data.push(DamageType {
        name: String::from("raw"),
        cname: CString::new("raw")?,
        ..Default::default()
    });
    sort_by_key_ref(&mut dt_data, |dt: &DamageType| &dt.name);
    dt_data.windows(2).for_each(|w| {
        if w[0].name == w[1].name {
            warn!("Damage type '{}' loaded twice!", w[0].name);
        }
    });
    Ok(dt_data)
}

#[unsafe(no_mangle)]
pub extern "C" fn dtype_get(name: *const c_char) -> c_int {
    if name.is_null() {
        return 0;
    }
    let ptr = unsafe { CStr::from_ptr(name) };
    let name = ptr.to_str().unwrap();
    match binary_search_by_key_ref(&DAMAGE_TYPES, name, |dt: &DamageType| &dt.name) {
        Ok(i) => (i + 1) as c_int,
        Err(_) => {
            warn!("damage type '{name}' not found");
            0
        }
    }
}

// Assume static here, because it doesn't really change after loading
#[unsafe(no_mangle)]
pub fn get_c(dt: c_int) -> Option<&'static DamageType> {
    DAMAGE_TYPES.get((dt - 1) as usize)
}

#[unsafe(no_mangle)]
pub extern "C" fn dtype_damageTypeToStr(dtid: c_int) -> *const c_char {
    match get_c(dtid) {
        Some(dt) => match &dt.cdisplay {
            Some(d) => d.as_ptr(),
            None => dt.cname.as_ptr(),
        },
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn dtype_raw(
    dtid: c_int,
    shield: *mut f64,
    armour: *mut f64,
    knockback: *mut f64,
) -> c_int {
    match get_c(dtid) {
        Some(dt) => {
            if !shield.is_null() {
                unsafe {
                    *shield = dt.shield_mod;
                }
            }
            if !armour.is_null() {
                unsafe {
                    *armour = dt.armour_mod;
                }
            }
            if !knockback.is_null() {
                unsafe {
                    *knockback = dt.knockback;
                }
            }
            0
        }
        None => -1,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn dtype_calcDamage(
    dshield: *mut f64,
    darmour: *mut f64,
    absorb: f64,
    knockback: *mut f64,
    dmg: *const naevc::Damage,
    ss: *const naevc::ShipStats,
) {
    fn get_stat(stat: Option<Stat>, ss: *const naevc::ShipStats) -> f64 {
        match stat {
            Some(o) => {
                if ss.is_null() {
                    1.
                } else {
                    (1. - unsafe { naevc::ss_offsetStat(ss, o.0) }).max(0.)
                }
            }
            None => 1.,
        }
    }
    if let Some(dt) = get_c(unsafe { (*dmg).type_ }) {
        if !dshield.is_null() {
            let mult = get_stat(dt.shield_stat, ss);
            unsafe { *dshield = dt.shield_mod * (*dmg).damage * absorb * mult }
        }
        if !darmour.is_null() {
            let mult = get_stat(dt.armour_stat, ss);
            unsafe { *darmour = dt.armour_mod * (*dmg).damage * absorb * mult }
        }
        if !knockback.is_null() {
            unsafe {
                *knockback = dt.knockback;
            }
        }
    }
}
