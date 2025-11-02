#![allow(dead_code)]
use anyhow::Result;
use log::warn_err;
use std::sync::LazyLock;

pub struct Constants {
    physics_speed_damp: f32,
    stealth_min_dist: f32,
    ship_min_mass: f32,
    ew_jump_bonus_range: f32,
    ew_asteroid_dist: f32,
    ew_jump_detect_dist: f32,
    ew_spob_detect_dist: f32,
    pilot_shield_down_time: f32,
    pilot_disabled_armour: f32,
}
impl Constants {
    fn load() -> Result<Self> {
        let lua = mlua::Lua::new_with(mlua::StdLib::ALL_SAFE, Default::default())?;
        let globals = lua.globals();
        globals.set("file", ndata::lua::open_file(&lua)?)?;

        let chunk = lua.load(ndata::read("constants.lua")?);
        let tbl: mlua::Table = chunk.call(())?;

        fn get_f32(tbl: &mlua::Table, name: &str, def: f32) -> f32 {
            let v: Option<f32> = match tbl.get(name) {
                Ok(v) => v,
                Err(e) => {
                    warn_err!(e);
                    None
                }
            };
            v.unwrap_or(def)
        }

        let physics_speed_damp = get_f32(&tbl, "PHYSICS_SPEED_DAMP", 3.);
        let stealth_min_dist = get_f32(&tbl, "STEALTH_MIN_DIST", 1000.);
        let ship_min_mass = get_f32(&tbl, "SHIP_MIN_MASS", 0.5);
        let ew_jump_bonus_range = get_f32(&tbl, "EW_JUMP_BONUS_RANGE", 2500.);
        let ew_asteroid_dist = get_f32(&tbl, "EW_ASTEROID_DIST", 7.5e3);
        let ew_jump_detect_dist = get_f32(&tbl, "EW_JUMPDETECT_DIST", 7.5e3);
        let ew_spob_detect_dist = get_f32(&tbl, "EW_SPOBDETECT_DIST", 20e3);
        let pilot_shield_down_time = get_f32(&tbl, "PILOT_SHIELD_DOWN_TIME", 5.);
        let pilot_disabled_armour = get_f32(&tbl, "PILOT_DISABLED_ARMOUR", 0.1);

        Ok(Self {
            physics_speed_damp,
            stealth_min_dist,
            ship_min_mass,
            ew_jump_bonus_range,
            ew_asteroid_dist,
            ew_jump_detect_dist,
            ew_spob_detect_dist,
            pilot_shield_down_time,
            pilot_disabled_armour,
        })
    }

    fn load_or_default() -> Self {
        match Self::load() {
            Ok(cts) => cts,
            Err(e) => {
                warn_err!(e);
                Self::default()
            }
        }
    }

    fn default() -> Self {
        Self {
            physics_speed_damp: 3.,
            stealth_min_dist: 1000.,
            ship_min_mass: 0.5,
            ew_jump_bonus_range: 2500.,
            ew_asteroid_dist: 7.5e3,
            ew_jump_detect_dist: 7.5e3,
            ew_spob_detect_dist: 20e3,
            pilot_shield_down_time: 5.,
            pilot_disabled_armour: 0.1,
        }
    }
}

pub static CTS: LazyLock<Constants> = LazyLock::new(|| Constants::load_or_default());
