#![allow(dead_code, unused_variables, unused_imports)]

use anyhow::Result;
use nalgebra::Vector3;
use rayon::prelude::*;
use std::ffi::{CStr, CString};
use std::io::{Error, ErrorKind};

use crate::array::ArrayCString;
use crate::context::{Context, SafeContext};
use crate::gettext::gettext;
use crate::nlua::LuaEnv;
use crate::{formatx, warn};
use crate::{ndata, texture};
use crate::{nxml, nxml_err_attr_missing, nxml_err_node_unknown};

enum Grid {
    NONE,
    ENEMIES,
    ALLIES,
    NEUTRAL,
}

#[derive(Debug)]
pub struct Generator {
    /// Generator ID
    id: i32,
    /// Weight modifier
    weight: f32,
}

#[derive(Debug)]
pub struct FactionLua {
    lua_env: LuaEnv,
    lua_hit: mlua::Function,
    lua_hit_test: mlua::Function,
    lua_text_rank: mlua::Function,
    lua_text_broad: mlua::Function,
}

#[derive(Debug, Default)]
pub struct Faction {
    id: u32,
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
    enemies: Vec<i32>,
    allies: Vec<i32>,
    neutrals: Vec<i32>,

    // Player stuff
    pub player_def: f32,
    pub player: f32,
    pub player_override: f32,

    // Scheduler
    sched_env: Option<LuaEnv>,

    // Behaviour
    friendly_at: f32,
    lua: Option<FactionLua>,

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
    pub f_repoverride: bool,

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
unsafe impl Sync for Faction {}
unsafe impl Send for Faction {}

impl Faction {
    /// Loads the elementary faction stuff, does not fill out information dependent on other
    /// factions
    fn load(ctx: &SafeContext, filename: &str) -> Result<Self> {
        let mut fct = Faction::default();
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
                tag => {
                    return nxml_err_node_unknown!("Faction", &fct.name, tag);
                }
            }
        }

        Ok(fct)
    }
}

use std::sync::OnceLock;
pub static FACTIONS: OnceLock<Vec<Faction>> = OnceLock::new();
pub static GENERATORS: OnceLock<Vec<Generator>> = OnceLock::new();

pub fn load() -> Result<()> {
    let ctx = SafeContext::new(Context::get().unwrap());
    let files = ndata::read_dir("factions/")?;
    let mut factions: Vec<Faction> = files
        .par_iter()
        .filter_map(|filename| {
            if !filename.ends_with(".xml") {
                return None;
            }
            match Faction::load(&ctx, filename.as_str()) {
                Ok(sp) => Some(sp),
                _ => {
                    warn!("Unable to load Faction '{}'!", filename);
                    None
                }
            }
        })
        .collect();

    FACTIONS.set(factions).unwrap();
    GENERATORS.set(vec![]).unwrap();
    Ok(())
}

pub fn get(name: &str) -> Option<&'static Faction> {
    None
}

// Here be C API
/*
use std::os::raw::{c_int};

#[unsafe(no_mangle)]
pub extern "C" fn faction_isFaction( f: c_int ) -> c_int {
    match f < 0 || (f >= FACTIONS.get().unwrap().len() as c_int ){
        true => 0,
        false => 1,
    }
}
*/
