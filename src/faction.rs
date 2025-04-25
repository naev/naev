#![allow(dead_code, unused_variables, unused_imports)]

use anyhow::Result;
use nalgebra::Vector3;

use crate::nlua::LuaEnv;
use crate::{ndata, texture};

#[derive(Debug)]
pub struct Generator {
    /// Generator ID
    id: i32,
    /// Weight modifier
    weight: f32,
}

#[derive(Debug)]
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
    sched_env: LuaEnv,

    // Behaviour
    friendly_at: f32,
    lua_env: LuaEnv,
    lua_hit: mlua::Function,
    lua_hit_test: mlua::Function,
    lua_text_rank: mlua::Function,
    lua_text_broad: mlua::Function,

    // Safe lanes
    lane_length_per_presence: f32,
    lane_base_cost: f32,

    // Presence
    generators: Vec<Generator>,

    // Equipping
    equip_env: LuaEnv,

    // Flags
    pub f_static: bool,
    pub f_invisible: bool,
    pub f_known: bool,
    pub f_dynamic: bool,
    pub f_useshiddenjumps: bool,
    pub f_repoverride: bool,

    // Tags
    pub tags: Vec<String>,
}
unsafe impl Sync for Faction {}
unsafe impl Send for Faction {}

enum Grid {
    NONE,
    ENEMIES,
    ALLIES,
    NEUTRAL,
}

use std::sync::OnceLock;
pub static FACTIONS: OnceLock<Vec<Faction>> = OnceLock::new();
pub static GENERATORS: OnceLock<Vec<Generator>> = OnceLock::new();

pub fn load() -> Result<()> {
    FACTIONS.set(vec![]).unwrap();
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
