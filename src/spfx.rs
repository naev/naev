#![allow(dead_code)]
use crate::nlua::LuaEnv;
use mlua::{Either, Function, UserData, UserDataMethods, UserDataRefMut};
use physics::vec2::Vec2;
use std::sync::Mutex;
use thunderdome::Arena;

struct LuaSpfx {
    /// Sound ignores pitch changes.
    global: bool,
    /// Needs cleaning up
    cleanup: bool,
    ttl: f64,
    pos: Option<Vec2>,
    vel: Option<Vec2>,
    radius: Option<f64>,
    sfx: Option<UserDataRefMut<audio::Audio>>,
    /// Inherited Lua environment
    env: LuaEnv,
    data: mlua::Table,
    render_bg: Option<Function>,
    render_mg: Option<Function>,
    render_fg: Option<Function>,
    update: Option<Function>,
    remove: Option<Function>,
}

#[derive(Copy, Clone, derive_more::From, derive_more::Into)]
struct LuaSpfxRef(thunderdome::Index);

static LUASPFX: Mutex<Arena<LuaSpfx>> = Mutex::new(Arena::new());

impl UserData for LuaSpfxRef {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        methods.add_function(
            "new",
            |lua,
             (ttl, update, render_bg, render_mg, render_fg, pos, vel, sfx, radius, remove): (
                f64,
                Option<Function>,
                Option<Function>,
                Option<Function>,
                Option<Function>,
                Option<Either<Vec2, bool>>,
                Option<Vec2>,
                Option<UserDataRefMut<audio::Audio>>,
                Option<f64>,
                Option<Function>,
            )|
             -> mlua::Result<Self> {
                let (pos, global) = if let Some(pos) = pos {
                    match pos {
                        Either::Left(v) => (Some(v), false),
                        Either::Right(g) => (None, g),
                    }
                } else {
                    (None, true)
                };
                let env = match LuaEnv::current(&lua) {
                    Some(env) => env,
                    None => {
                        return Err(mlua::Error::RuntimeError(
                            "No current Lua environment!".to_string(),
                        ));
                    }
                };
                let data = lua.create_table()?;
                let spfx = LuaSpfx {
                    global,
                    ttl,
                    update,
                    cleanup: false,
                    render_bg,
                    render_mg,
                    render_fg,
                    remove,
                    pos,
                    vel,
                    sfx,
                    radius,
                    data,
                    env,
                };
                Ok(LUASPFX.lock().unwrap().insert(spfx).into())
            },
        );
    }
}

pub fn open_spfx(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<LuaSpfxRef>()?)
}
