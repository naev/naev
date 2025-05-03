#![allow(dead_code, unused_variables, unused_imports)]

use anyhow::Result;
use nalgebra::Vector3;
use rayon::prelude::*;
use std::ffi::{CStr, CString};
use std::ops::Deref;
use std::sync::Mutex;

use crate::array::ArrayCString;
use crate::context::{Context, SafeContext};
use crate::gettext::gettext;
use crate::nlua::LuaEnv;
use crate::nlua::{NLua, NLUA};
use crate::utils::{binary_search_by_key_ref, sort_by_key_ref};
use crate::{array, ndata, texture};
use crate::{formatx, warn};
use crate::{nxml, nxml_err_attr_missing, nxml_warn_node_unknown};
use thunderdome::{Arena, Index};

enum Grid {
    None,
    Enemies,
    Allies,
    Neutral,
}

/// Full faction data
pub static FACTIONS: Mutex<Arena<Faction>> = Mutex::new(Arena::new());

pub struct FactionID {
    id: Index,
}
impl FactionID {
    pub fn new(name: &str) -> Option<FactionID> {
        let factions = FACTIONS.lock().unwrap();
        for fctid in factions.iter() {
            let (id, fct) = fctid;
            if fct.data.name == name {
                return Some(FactionID { id });
            }
        }
        None
    }

    pub fn call<F, R>(&self, f: F) -> R
    where
        F: Fn(&Faction) -> R,
    {
        let factions = FACTIONS.lock().unwrap();
        let fct = factions.get(self.id).unwrap();
        f(fct)
    }

    pub fn call_mut<F, R>(&self, f: F) -> R
    where
        F: Fn(&mut Faction) -> R,
    {
        let mut factions = FACTIONS.lock().unwrap();
        let fct = factions.get_mut(self.id).unwrap();
        f(fct)
    }
}

#[derive(Debug)]
pub struct Faction {
    pub player: f32,
    pub player_override: Option<f32>,
    pub data: DataWrapper,
}
impl Faction {
    pub fn is_dynamic(&self) -> bool {
        match &self.data {
            DataWrapper::Static(_) => false,
            DataWrapper::Dynamic(_) => true,
        }
    }
}

/// Wrapper for both Dynamic and Static factions
#[derive(Debug)]
pub enum DataWrapper {
    Static(&'static FactionData),
    Dynamic(FactionData),
}
impl Deref for DataWrapper {
    type Target = FactionData;
    fn deref(&self) -> &<Self as Deref>::Target {
        match self {
            Self::Static(d) => d,
            Self::Dynamic(d) => d,
        }
    }
}

#[derive(Debug)]
pub struct Generator {
    /// Generator ID
    id: usize,
    /// Weight modifier
    weight: f32,
}
impl Generator {
    fn new(factions: &[FactionLoad], names: &[String], weights: &[f32]) -> Vec<Self> {
        let mut generator: Vec<Generator> = vec![];
        for (name, weight) in names.iter().zip(weights.iter()) {
            if let Some(id) = FactionLoad::get(factions, name) {
                generator.push(Generator {
                    id,
                    weight: *weight,
                })
            }
        }
        generator
    }
}

#[derive(Debug, Default)]
pub struct FactionData {
    id: usize,
    pub name: String,
    pub longname: Option<String>,
    pub displayname: Option<String>,
    pub mapname: Option<String>,
    ai: String,
    description: String,
    local_th: f32,

    // Scripts
    script_standing: String,
    script_spawn: String,
    script_equip: String,

    // Graphics
    pub logo: Option<texture::Texture>,
    pub colour: Vector3<f32>,

    // Relationships
    enemies: Vec<usize>,
    allies: Vec<usize>,
    neutrals: Vec<usize>,

    // Player stuff
    pub player_def: f32,

    // Scheduler
    sched_env: Option<LuaEnv>,

    // Behaviour
    friendly_at: f32,
    lua_env: Option<LuaEnv>,

    // Equipping
    equip_env: Option<LuaEnv>,

    // Safe lanes
    lane_length_per_presence: f32,
    lane_base_cost: f32,

    // Presence
    generators: Vec<Generator>,

    // Flags
    pub f_static: bool,
    pub f_invisible: bool,
    pub f_known: bool,
    pub f_dynamic: bool,
    pub f_useshiddenjumps: bool,

    // Tags
    pub tags: Vec<String>,

    // C stuff, TODO remove when unnecessary
    cname: CString,
    clongname: Option<CString>,
    cdisplayname: Option<CString>,
    cmapname: Option<CString>,
    cdescription: CString,
    ctags: ArrayCString,
}
impl FactionData {
    fn init_lua(&self, lua: &NLua) -> Result<()> {
        if let Some(env) = &self.equip_env {
            let path = format!("factions/equip/{}.lua", self.script_equip);
            let data = ndata::read(&path)?;
            let func = lua
                .lua
                .load(std::str::from_utf8(&data)?)
                .set_name(path)
                .into_function()?;
            env.call::<()>(lua, &func, ())?;
        }
        if let Some(env) = &self.sched_env {
            let path = format!("factions/spawn/{}.lua", self.script_spawn);
            let data = ndata::read(&path)?;
            let func = lua
                .lua
                .load(std::str::from_utf8(&data)?)
                .set_name(path)
                .into_function()?;
            env.call::<()>(lua, &func, ())?;
        }
        if let Some(env) = &self.lua_env {
            let path = format!("factions/standing/{}.lua", self.script_standing);
            let data = ndata::read(&path)?;
            let func = lua
                .lua
                .load(std::str::from_utf8(&data)?)
                .set_name(path)
                .into_function()?;
            env.call::<()>(lua, &func, ())?;
        }
        Ok(())
    }
}

#[derive(Default)]
struct FactionSocial {
    enemies: Vec<usize>,
    allies: Vec<usize>,
    neutrals: Vec<usize>,
}
impl FactionSocial {
    pub fn new(fct: &FactionLoad, factions: &[FactionLoad]) -> Self {
        let mut social = FactionSocial::default();
        for name in &fct.enemies {
            if let Some(f) = FactionLoad::get(factions, name) {
                social.enemies.push(f)
            };
        }
        for name in &fct.allies {
            if let Some(f) = FactionLoad::get(factions, name) {
                social.allies.push(f)
            };
        }
        for name in &fct.neutrals {
            if let Some(f) = FactionLoad::get(factions, name) {
                social.neutrals.push(f)
            };
        }
        social
    }
}

#[derive(Debug, Default)]
struct FactionLoad {
    /// Base data
    data: FactionData,

    // Generators
    generator_name: Vec<String>,
    generator_weight: Vec<f32>,

    // Relationships
    enemies: Vec<String>,
    allies: Vec<String>,
    neutrals: Vec<String>,
}
impl FactionLoad {
    /// Loads the elementary faction stuff, does not fill out information dependent on other
    /// factions
    fn new(ctx: &SafeContext, lua: &Mutex<NLua>, filename: &str) -> Result<Self> {
        let mut fctload = FactionLoad::default();
        let fct = &mut fctload.data;

        // TODO use default_field_values when stabilized
        // https://github.com/rust-lang/rust/issues/132162
        fct.local_th = 10.;

        let data = ndata::read(filename)?;
        let doc = roxmltree::Document::parse(std::str::from_utf8(&data)?)?;
        let root = doc.root_element();
        fct.name = String::from(match root.attribute("name") {
            Some(n) => n,
            None => {
                return nxml_err_attr_missing!("Damage Type", "name");
            }
        });
        fct.cname = CString::new(fct.name.as_str())?;

        for node in root.children() {
            if !node.is_element() {
                continue;
            }
            match node.tag_name().name().to_lowercase().as_str() {
                "player" => fct.player_def = nxml::node_f32(node)?,
                "longname" => {
                    fct.longname = Some(nxml::node_string(node)?);
                    fct.clongname = Some(nxml::node_cstring(node)?);
                }
                "display" => {
                    fct.displayname = Some(nxml::node_string(node)?);
                    fct.cdisplayname = Some(nxml::node_cstring(node)?);
                }
                "mapname" => {
                    fct.mapname = Some(nxml::node_string(node)?);
                    fct.cmapname = Some(nxml::node_cstring(node)?);
                }
                "description" => {
                    fct.description = nxml::node_string(node)?;
                    fct.cdescription = nxml::node_cstring(node)?;
                }
                "ai" => fct.ai = nxml::node_string(node)?,
                "local_th" => fct.local_th = nxml::node_f32(node)?,
                "lane_length_per_presence" => fct.lane_length_per_presence = nxml::node_f32(node)?,
                "lane_base_cost" => fct.lane_base_cost = nxml::node_f32(node)?,
                // TODO COLOUR
                "colour" => continue,
                "logo" => {
                    let gfxname = nxml::node_texturepath(node, "gfx/logo/")?;
                    let nctx = ctx.lock();
                    fct.logo = Some(texture::TextureBuilder::new().path(&gfxname).build(&nctx)?);
                }
                "known" => fct.f_known = true,
                "static" => fct.f_static = true,
                "invisible" => fct.f_invisible = true,
                "useshiddenjumps" => fct.f_useshiddenjumps = true,
                "standing" => fct.script_standing = nxml::node_string(node)?,
                "spawn" => fct.script_spawn = nxml::node_string(node)?,
                "equip" => fct.script_equip = nxml::node_string(node)?,
                "tags" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        fct.tags.push(node.tag_name().name().to_lowercase());
                    }
                    // Remove when not needed for C interface
                    fct.ctags = ArrayCString::new(&fct.tags)?;
                }
                // Temporary scaoffolding stuff
                "allies" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        fctload.allies.push(nxml::node_string(node)?);
                    }
                }
                "enemies" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        fctload.enemies.push(nxml::node_string(node)?);
                    }
                }
                "neutrals" => {
                    for node in node.children() {
                        if !node.is_element() {
                            continue;
                        }
                        fctload.neutrals.push(nxml::node_string(node)?);
                    }
                }
                "generator" => {
                    fctload.generator_name.push(nxml::node_string(node)?);
                    fctload
                        .generator_weight
                        .push(match node.attribute("weight") {
                            Some(str) => str.parse::<f32>()?,
                            None => 1.0,
                        });
                }

                // Case we missed everything
                tag => nxml_warn_node_unknown!("Faction", &fct.name, tag),
            }
        }

        // Initaialize Lua scripts
        {
            let mut lua = lua.lock().unwrap();
            if !fct.script_spawn.is_empty() {
                fct.sched_env = Some({
                    let mut env = lua.environment_new(&fct.script_spawn)?;
                    env.load_standard(&lua)?;
                    env
                });
            }
            if !fct.script_equip.is_empty() {
                fct.equip_env = Some({
                    let mut env = lua.environment_new(&fct.script_equip)?;
                    env.load_standard(&lua)?;
                    env
                });
            }
            if !fct.script_standing.is_empty() {
                fct.lua_env = Some({
                    let mut env = lua.environment_new(&fct.script_standing)?;
                    env.load_standard(&lua)?;
                    env
                });
            }
        }

        Ok(fctload)
    }

    fn apply_social(&mut self, social: FactionSocial) {
        let fct = &mut self.data;
        fct.enemies = social.enemies;
        fct.allies = social.allies;
        fct.neutrals = social.neutrals;
    }

    fn into_data(self) -> FactionData {
        self.data
    }

    fn get(factions: &[FactionLoad], name: &str) -> Option<usize> {
        match binary_search_by_key_ref(factions, name, |fctload: &FactionLoad| &fctload.data.name) {
            Ok(id) => Some(id),
            Err(_) => {
                warn!("Faction '{}' not found during loading!", name);
                None
            }
        }
    }
}

use std::sync::OnceLock;
/// Static factions that are never modified after creation
pub static FACTIONDATA: OnceLock<Vec<FactionData>> = OnceLock::new();

pub fn load() -> Result<()> {
    let ctx = SafeContext::new(Context::get().unwrap());
    let files = ndata::read_dir("factions/")?;

    // First pass: set up factions
    let mut factionload: Vec<FactionLoad> = files
        //.par_iter()
        .iter()
        .filter_map(|filename| {
            if !filename.ends_with(".xml") {
                return None;
            }
            match FactionLoad::new(&ctx, &NLUA, filename.as_str()) {
                Ok(sp) => Some(sp),
                Err(e) => {
                    warn!("Unable to load Faction '{}': {}", filename, e);
                    None
                }
            }
        })
        .collect();
    // Add Player before sorting
    factionload.push(FactionLoad {
        data: FactionData {
            name: String::from("Player"),
            cname: CString::new("Player")?,
            f_static: true,
            f_invisible: true,
            ..Default::default()
        },
        ..Default::default()
    });
    sort_by_key_ref(&mut factionload, |fctload: &FactionLoad| &fctload.data.name);

    // Second pass: set allies/enemies and generators
    let factionsocial: Vec<FactionSocial> = factionload
        //.par_iter()
        .iter()
        .map(|fct| FactionSocial::new(fct, &factionload))
        .collect();
    for (id, social) in factionsocial.into_iter().enumerate() {
        factionload[id].apply_social(social);
    }

    // Third pass: set faction generators
    let factiongenerator: Vec<Vec<Generator>> = factionload
        //.par_iter()
        .iter()
        .map(|fct| Generator::new(&factionload, &fct.generator_name, &fct.generator_weight))
        .collect();
    for (id, generator) in factiongenerator.into_iter().enumerate() {
        factionload[id].data.generators = generator;
        factionload[id].data.id = id; // Also set the ID here
    }

    // Convert to factions
    let factions: Vec<FactionData> = factionload
        .into_iter()
        .map(|fctload| fctload.into_data())
        .collect();

    // Save the data
    FACTIONDATA.set(factions).unwrap();

    // Populate the arena with the static factions
    let mut factions = FACTIONS.lock().unwrap();
    for fct in FACTIONDATA.get().unwrap() {
        let _ = factions.insert(Faction {
            player: fct.player_def,
            player_override: None,
            data: DataWrapper::Static(fct),
        });
    }

    // Compute grid

    Ok(())
}

pub fn load_lua() -> Result<()> {
    // Last pass: initialize Lua
    let lua = NLUA.lock().unwrap();
    for fct in FACTIONDATA.get().unwrap() {
        fct.init_lua(&lua)?;
    }
    Ok(())
}

// Here be C API
use std::mem::ManuallyDrop;
use std::os::raw::{c_char, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn _faction_isFaction(f: c_int) -> c_int {
    if f < 0 {
        return 0;
    }
    match FACTIONS.lock().unwrap().contains_slot(f as u32) {
        Some(_) => 1,
        None => 0,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn _faction_exists(name: *const c_char) -> c_int {
    let ptr = unsafe { CStr::from_ptr(name) };
    let name = ptr.to_str().unwrap();
    for fct in FACTIONS.lock().unwrap().iter() {
        let (key, val) = fct;
        if name == val.data.name {
            return key.slot() as c_int;
        }
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn _faction_get(name: *const c_char) -> c_int {
    let ptr = unsafe { CStr::from_ptr(name) };
    let name = ptr.to_str().unwrap();
    for fct in FACTIONS.lock().unwrap().iter() {
        let (key, val) = fct;
        if name == val.data.name {
            return key.slot() as c_int;
        }
    }
    warn!(gettext("Faction '{}' not found in stack."), name);
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn _faction_getAll() -> *const c_int {
    let mut fcts: Vec<c_int> = vec![];
    for fct in FACTIONS.lock().unwrap().iter() {
        let (key, val) = fct;
        fcts.push(key.slot() as c_int);
    }
    let arr = ManuallyDrop::new(array::Array::new(&fcts).unwrap());
    arr.as_ptr() as *const c_int
}
