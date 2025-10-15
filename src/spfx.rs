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
impl LuaSpfxRef {
    pub fn call<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&LuaSpfx) -> R,
    {
        let luaspfx = LUASPFX.lock().unwrap();
        match luaspfx.get(self.0) {
            Some(spfx) => Ok(f(spfx)),
            None => anyhow::bail!("LuaSpfx not found"),
        }
    }

    pub fn call_mut<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&mut LuaSpfx) -> R,
    {
        let mut luaspfx = LUASPFX.lock().unwrap();
        match luaspfx.get_mut(self.0) {
            Some(spfx) => Ok(f(spfx)),
            None => anyhow::bail!("LuaSpfx not found"),
        }
    }
}

static LUASPFX: Mutex<Arena<LuaSpfx>> = Mutex::new(Arena::new());

/*
 * @brief Lua bindings to interact with spfx.
 *
 *
 * @luamod spfx
 */
impl UserData for LuaSpfxRef {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Creates a new special effect.
         *
         * @usage spfx.new( 5, update, nil, nil, render, player.pos(),
         * player.pilot():vel(), sfx ) -- Play effect with update and render functions
         * at player position/velocity
         * @usage spfx.new( 10, nil, nil, nil, nil, true, nil, sfx ) -- Play an effect
         * locally (affected by time compression and autonav stuff)
         * @usage spfx.new( 10, nil, nil, nil, nil, nil, nil, sfx ) -- Play a global
         * effect (not affected by time stuff )
         *
         *    @luatparam Number ttl Time to live of the effect.
         *    @luatparam[opt] Function|nil update Update function to use if applicable.
         *    @luatparam[opt] Function|nil render_bg Background render function to use
         * if applicable (behind ships).
         *    @luatparam[opt] Function|nil render_mg Middle render function to use if
         * applicable (infront of NPC ships, behind player).
         *    @luatparam[opt] Function|nil render_fg `Foregroundrender` function to use
         * if applicable (infront of player).
         *    @luatparam[opt] vec2|boolean pos Position of the effect, or a boolean to
         * indicate whether or not the effect is local.
         *    @luatparam[opt] vec2 vel Velocity of the effect.
         *    @luatparam[opt] audio sfx Sound effect associated with the spfx.
         *    @luatparam[opt] number radius Radius to use to determine if should render.
         *    @luatparam[opt] Function|nil remove Function to run when removing the
         * outfit.
         *    @luatreturn spfx New spfx corresponding to the data.
         * @luafunc new
         */
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
        /*
         * @brief Removes a special effect.
         *
         *    @luatparam spfx s Spfx to remove.
         * @luafunc rm
         */
        methods.add_method_mut("rm", |_, this, ()| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.cleanup = true;
                this.ttl = -1.;
            })?;
            Ok(())
        });
        /*
         * @brief Gets the position of a spfx.
         *
         *    @luatparam spfx s Spfx to get position of.
         *    @luatreturn vec2 Position of the spfx.
         * @luafunc pos( s )
         */
        methods.add_method("pos", |_, this, ()| -> mlua::Result<Option<Vec2>> {
            Ok(this.call(|this| this.pos)?)
        });
        /*
         * @brief Sets the velocity of a spfx.
         *
         *    @luatparam spfx s Spfx to set the velocity of.
         *    @luatparam vec2 v Velocity to set to.
         * @luafunc setVel
         */
        methods.add_method("vel", |_, this, ()| -> mlua::Result<Option<Vec2>> {
            Ok(this.call(|this| this.vel)?)
        });
        /*
         * @brief Sets the position of a spfx.
         *
         *    @luatparam spfx s Spfx to set the position of.
         *    @luatparam vec2 p Position to set to.
         * @luafunc setPos
         */
        methods.add_method_mut("setPos", |_, this, pos: Vec2| -> mlua::Result<()> {
            this.call_mut(|this| match this.pos {
                Some(_) => {
                    this.pos = Some(pos);
                    Ok(())
                }
                None => Err(mlua::Error::RuntimeError(
                    "can't set position of a LuaSpfx with None position".to_string(),
                )),
            })?
        });
        /*
         * @brief Sets the velocity of a spfx.
         *
         *    @luatparam spfx s Spfx to set the velocity of.
         *    @luatparam vec2 v Velocity to set to.
         * @luafunc setVel
         */
        methods.add_method_mut("setVel", |_, this, vel: Vec2| -> mlua::Result<()> {
            this.call_mut(|this| match this.vel {
                Some(_) => {
                    this.vel = Some(vel);
                    Ok(())
                }
                None => Err(mlua::Error::RuntimeError(
                    "can't set position of a LuaSpfx with None velocity".to_string(),
                )),
            })?
        });
    }
}

pub fn open_spfx(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<LuaSpfxRef>()?)
}
