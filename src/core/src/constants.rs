#![allow(dead_code)]
use crate::ntime;
use anyhow::Result;
use nlog::warn_err;
use std::sync::LazyLock;

pub struct Constants {
    pub physics_speed_damp: f32,
    pub hyperspace_enter_max: f32,
    pub hyperspace_enter_min: f32,
    pub stealth_min_dist: f32,
    pub ship_min_mass: f32,
    pub audio_ref_distance: f32,
    pub audio_max_distance: f32,
    pub ew_jump_bonus_range: f32,
    pub ew_asteroid_dist: f32,
    pub ew_jump_detect_dist: f32,
    pub ew_spob_detect_dist: f32,
    pub timedate_minor_in_major: i64,
    pub timedate_increment_in_minor: i64,
    pub timedate_increments_per_second: i64,
    pub timedate_hyperspace_increments: i64,
    pub timedate_land_increments: i64,
    pub pilot_shield_down_time: f32,
    pub pilot_stress_recovery_time: f32,
    pub pilot_disabled_armour: f32,
    pub camera_angle: f32,
    pub warn_buy_intrinsics: bool,
}
impl Constants {
    fn load() -> Result<Self> {
        let lua = mlua::Lua::new_with(mlua::StdLib::ALL_SAFE, Default::default())?;
        let globals = lua.globals();
        globals.set("file", ndata::luafile::open_file(&lua)?)?;

        // TODO fix up this loader stuff
        let package: mlua::Table = globals.get("package")?;
        let loaders: mlua::Table = package.get("loaders")?;
        // TODO reimplement in rust...
        type CFunctionNaev = unsafe extern "C-unwind" fn(*mut naevc::lua_State) -> i32;
        type CFunctionMLua = unsafe extern "C-unwind" fn(*mut mlua::lua_State) -> i32;
        unsafe {
            loaders.push(
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::nlua_package_loader_lua,
                ))?,
            )?;
        }
        const LUA_COMMON_PATH: &str = "common.lua"; // Common Lua functions.
        let common_data = ndata::read(LUA_COMMON_PATH)?;
        lua.load(std::str::from_utf8(&common_data)?).exec()?;

        let chunk = lua.load(ndata::read("constants.lua")?);
        let tbl: mlua::Table = chunk.call(())?;

        fn get<T: mlua::FromLua>(tbl: &mlua::Table, name: &str, def: T) -> T {
            let v: Option<T> = match tbl.get(name) {
                Ok(v) => v,
                Err(e) => {
                    warn_err!(e);
                    None
                }
            };
            v.unwrap_or(def)
        }
        let get_f32 = get::<f32>;
        let get_f64 = get::<f64>;
        let get_bool = get::<bool>;
        let get_u64 = get::<u32>;

        let physics_speed_damp = get_f32(&tbl, "PHYSICS_SPEED_DAMP", 3.);
        let hyperspace_enter_max = get_f32(&tbl, "HYPERSPACE_ENTER_MAX", 0.4);
        let hyperspace_enter_min = get_f32(&tbl, "HYPERSPACE_ENTER_MIN", 0.3);
        let stealth_min_dist = get_f32(&tbl, "STEALTH_MIN_DIST", 1000.);
        let ship_min_mass = get_f32(&tbl, "SHIP_MIN_MASS", 0.5);
        let audio_ref_distance = get_f32(&tbl, "AUDIO_REF_DISTANCE", 1e3);
        let audio_max_distance = get_f32(&tbl, "AUDIO_MAX_DISTANCE", 10e3);
        let ew_jump_bonus_range = get_f32(&tbl, "EW_JUMP_BONUS_RANGE", 2500.);
        let ew_asteroid_dist = get_f32(&tbl, "EW_ASTEROID_DIST", 7.5e3);
        let ew_jump_detect_dist = get_f32(&tbl, "EW_JUMPDETECT_DIST", 7.5e3);
        let ew_spob_detect_dist = get_f32(&tbl, "EW_SPOBDETECT_DIST", 20e3);
        let timedate_minor_in_major = get_u64(&tbl, "TIMEDATE_MINOR_IN_MAJOR", 5_000) as i64;
        let timedate_increment_in_minor =
            get_u64(&tbl, "TIMEDATE_INCREMENT_IN_MINOR", 10_000) as i64;
        let timedate_increments_per_second = (get_f64(&tbl, "TIMEDATE_INCREMENTS_PER_SECOND", 30.0)
            * ntime::MULTIPLIER_F)
            .round() as i64;
        let timedate_hyperspace_increments = (get_f64(
            &tbl,
            "TIMEDATE_HYPERSPACE_INCREMENT",
            10_000.0,
        ) * ntime::MULTIPLIER_F)
            .round() as i64;
        let timedate_land_increments = (get_f64(&tbl, "TIMEDATE_LAND_INCREMENT", 10_000.0)
            * ntime::MULTIPLIER_F)
            .round() as i64;
        let pilot_shield_down_time = get_f32(&tbl, "PILOT_SHIELD_DOWN_TIME", 5.);
        let pilot_stress_recovery_time = get_f32(&tbl, "PILOT_STRESS_RECOVERY_TIME", 5.);
        let pilot_disabled_armour = get_f32(&tbl, "PILOT_DISABLED_ARMOUR", 0.1);
        let camera_angle = get_f32(&tbl, "CAMERA_ANGLE", std::f32::consts::FRAC_PI_4);
        let warn_buy_intrinsics = get_bool(&tbl, "WARN_BUY_INTRINSICS", true);

        // TODO remove this
        unsafe {
            naevc::CTS.PHYSICS_SPEED_DAMP = physics_speed_damp as f64;
            naevc::CTS.HYPERSPACE_ENTER_MAX = hyperspace_enter_max as f64;
            naevc::CTS.HYPERSPACE_ENTER_MIN = hyperspace_enter_min as f64;
            naevc::CTS.STEALTH_MIN_DIST = stealth_min_dist as f64;
            naevc::CTS.SHIP_MIN_MASS = ship_min_mass as f64;
            naevc::CTS.EW_JUMP_BONUS_RANGE = ew_jump_bonus_range as f64;
            naevc::CTS.EW_ASTEROID_DIST = ew_asteroid_dist as f64;
            naevc::CTS.EW_JUMPDETECT_DIST = ew_jump_detect_dist as f64;
            naevc::CTS.EW_SPOBDETECT_DIST = ew_spob_detect_dist as f64;
            naevc::CTS.PILOT_SHIELD_DOWN_TIME = pilot_shield_down_time as f64;
            naevc::CTS.PILOT_STRESS_RECOVERY_TIME = pilot_stress_recovery_time as f64;
            naevc::CTS.PILOT_DISABLED_ARMOUR = pilot_disabled_armour as f64;
            naevc::CTS.TIMEDATE_HYPERSPACE_INCREMENTS =
                timedate_hyperspace_increments as f64 / ntime::MULTIPLIER_F;
            naevc::CTS.TIMEDATE_LAND_INCREMENTS =
                timedate_land_increments as f64 / ntime::MULTIPLIER_F;
            naevc::CTS.CAMERA_ANGLE = camera_angle as f64;
            naevc::CTS.CAMERA_VIEW = naevc::CTS.CAMERA_ANGLE.sin();
            naevc::CTS.CAMERA_VIEW_INV = 1.0 / naevc::CTS.CAMERA_VIEW;
            naevc::CTS.WARN_BUY_INTRINSICS = if warn_buy_intrinsics { 1 } else { 0 };
        }

        Ok(Self {
            physics_speed_damp,
            hyperspace_enter_max,
            hyperspace_enter_min,
            stealth_min_dist,
            ship_min_mass,
            audio_ref_distance,
            audio_max_distance,
            ew_jump_bonus_range,
            ew_asteroid_dist,
            ew_jump_detect_dist,
            ew_spob_detect_dist,
            timedate_minor_in_major,
            timedate_increment_in_minor,
            timedate_increments_per_second,
            timedate_hyperspace_increments,
            timedate_land_increments,
            pilot_shield_down_time,
            pilot_stress_recovery_time,
            pilot_disabled_armour,
            camera_angle,
            warn_buy_intrinsics,
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
            hyperspace_enter_max: 0.4,
            hyperspace_enter_min: 0.3,
            stealth_min_dist: 1000.,
            ship_min_mass: 0.5,
            audio_ref_distance: 1e3,
            audio_max_distance: 10e3,
            ew_jump_bonus_range: 2500.,
            ew_asteroid_dist: 7.5e3,
            ew_jump_detect_dist: 7.5e3,
            ew_spob_detect_dist: 20e3,
            timedate_minor_in_major: 5_000,
            timedate_increment_in_minor: 10_000,
            timedate_increments_per_second: 30_000,
            timedate_hyperspace_increments: 10_000_000,
            timedate_land_increments: 10_000_000,
            pilot_shield_down_time: 5.,
            pilot_stress_recovery_time: 5.,
            pilot_disabled_armour: 0.1,
            camera_angle: std::f32::consts::FRAC_PI_4,
            warn_buy_intrinsics: true,
        }
    }
}

pub static CTS: LazyLock<Constants> = LazyLock::new(Constants::load_or_default);
