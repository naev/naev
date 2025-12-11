#![allow(dead_code)]
use crate::nlua;
use crate::nlua::LuaEnv;
use log::warn_err;
use mlua::{Either, Function, UserData, UserDataMethods, UserDataRef};
use physics::vec2::Vec2;
use renderer::camera;
use renderer::colour::Colour;
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
    sfx: Option<audio::AudioRef>,
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

pub fn clear() {
    LUASPFX.lock().unwrap().clear();
}

pub fn set_speed(s: f32) {
    for (_, spfx) in LUASPFX.lock().unwrap().iter() {
        if spfx.global || spfx.cleanup {
            continue;
        }
        if let Some(sfx) = &spfx.sfx {
            match sfx.call(|sfx| {
                sfx.set_pitch(s);
            }) {
                Ok(_) => (),
                Err(e) => {
                    warn_err!(e);
                }
            }
        }
    }
}

pub fn set_speed_volume(v: f32) {
    for (_, spfx) in LUASPFX.lock().unwrap().iter() {
        if spfx.global || spfx.cleanup {
            continue;
        }
        if let Some(sfx) = &spfx.sfx {
            match sfx.call(|sfx| {
                // Bypasses a lot of the systems we have, is it needed?
                sfx.set_gain(sfx.volume() * v);
            }) {
                Ok(_) => (),
                Err(e) => {
                    warn_err!(e);
                }
            }
        }
    }
}

pub fn update(dt: f64) {
    let lua = &nlua::NLUA;
    LUASPFX.lock().unwrap().retain(|id, spfx| {
        spfx.ttl -= dt;
        if spfx.ttl <= 0. || spfx.cleanup {
            return false;
        }
        if let Some(pos) = &mut spfx.pos
            && let Some(vel) = spfx.vel
        {
            *pos += vel * dt;
            if let Some(sfx) = &spfx.sfx {
                // TODO move Audio ownership to LuaSpfx
                let pos = pos.into_vector2();
                sfx.call(|sfx| {
                    sfx.set_position(pos.cast::<f32>());
                })
                .unwrap_or_else(|e| {
                    warn_err!(e);
                });
            }
        }
        if let Some(update) = &spfx.update {
            spfx.env
                .call::<()>(lua, update, (LuaSpfxRef(id), dt))
                .unwrap_or_else(|e| {
                    warn_err!(e);
                });
        }
        true
    });
}

enum RenderLayer {
    Background,
    Middle,
    Foreground,
}
fn render(layer: RenderLayer, dt: f64) {
    let lua = &nlua::NLUA;
    let z = camera::CAMERA.read().unwrap().zoom;
    for (id, spfx) in LUASPFX.lock().unwrap().iter() {
        if spfx.cleanup {
            continue;
        }
        let func = match layer {
            RenderLayer::Background => &spfx.render_bg,
            RenderLayer::Middle => &spfx.render_mg,
            RenderLayer::Foreground => &spfx.render_fg,
        };
        if let Some(func) = func
            && let Some(pos) = spfx.pos
            && let Some(pos) = renderer::Context::get().game_to_screen_coords_inrange(
                pos.into_vector2(),
                spfx.radius.unwrap_or(f64::INFINITY),
            )
        {
            // TODO flip y
            spfx.env
                .call::<()>(lua, func, (LuaSpfxRef(id), pos.x, pos.y, z, dt))
                .unwrap_or_else(|e| {
                    warn_err!(e);
                });
        }
    }
}

pub fn render_bg(dt: f64) {
    render(RenderLayer::Background, dt);
}
pub fn render_mg(dt: f64) {
    render(RenderLayer::Middle, dt);
}
pub fn render_fg(dt: f64) {
    render(RenderLayer::Foreground, dt);
}

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
        #[allow(clippy::type_complexity)]
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
                Option<Either<audio::AudioRef, UserDataRef<audio::AudioData>>>,
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
                let env = match LuaEnv::current(lua) {
                    Some(env) => env,
                    None => {
                        return Err(mlua::Error::RuntimeError(
                            "No current Lua environment!".to_string(),
                        ));
                    }
                };
                let data = lua.create_table()?;
                let sfx = match sfx {
                    None => None,
                    Some(Either::Left(audio)) => Some(audio),
                    Some(Either::Right(audiodata)) => Some(
                        audio::AudioBuilder::new(audio::AudioType::Static)
                            .data(Some(audiodata.clone()))
                            .build()?,
                    ),
                };
                if let Some(ref sfx) = sfx {
                    sfx.call_mut(|audio| {
                        if pos.is_some() {
                            audio.set_ingame();
                        }
                        audio.play();
                    })?;
                }
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
        /*
         * @brief Gets the sound effect of a spfx.
         *
         *    @luatparam spfx s Spfx to get sound effect of.
         *    @luatreturn audio Sound effect of the spfx.
         * @luafunc sfx
         */
        methods.add_method(
            "sfx",
            |_, this, ()| -> mlua::Result<Option<audio::AudioRef>> {
                Ok(this.call(|this| this.sfx.clone())?)
            },
        );
        /*
         * @brief Gets the data table of a spfx.
         *
         * This table is unique to each instance.
         *
         *    @luatparam spfx s Spfx to get data table of.
         *    @luatreturn table Data table of the spfx.
         * @luafunc data
         */
        methods.add_method("data", |_, this, ()| -> mlua::Result<mlua::Table> {
            Ok(this.call(|this| this.data.clone())?)
        });
        /*
         * @brief Creates a cloud of debris.
         *
         *    @luatparam number mass Mass of the cloud.
         *    @luatparam number radius Radius of the cloud.
         *    @luatparam Vec2 pos Position of the cloud.
         *    @luatparam Vec2 vel Velocity of the cloud.
         * @luafunc debris
         */
        methods.add_function(
            "debris",
            |_, (mass, radius, pos, vel): (f64, f64, Vec2, Vec2)| -> mlua::Result<()> {
                let p = pos.into_vector2();
                let v = vel.into_vector2();
                unsafe {
                    naevc::debris_add(mass, radius, p.x, p.y, v.x, v.y);
                }
                Ok(())
            },
        );
        /*
         * @brief Sets the nebula colour.
         *
         * @usage spfx.nebulaColour( 0.3, 0.5, 0.8 )
         * @usage spfx.nebulaColour( colour.new( 0.3, 0.5, 0.8 ) )
         *
         *    @luatparam Colour|number col Colour to set.
         * @luafunc nebulaColour
         */
        methods.add_function("nebulaColour", |_, col: Colour| -> mlua::Result<()> {
            let col = col.into_vector3();
            unsafe {
                naevc::spfx_setNebulaColour(col.x.into(), col.y.into(), col.z.into());
            }
            Ok(())
        });
    }
}

pub fn open_spfx(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<LuaSpfxRef>()?)
}

use std::ffi::c_double;

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_clear() {
    clear();
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_setSpeed(s: c_double) {
    set_speed(s as f32);
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_setSpeedVolume(v: c_double) {
    set_speed(v as f32);
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_update(dt: c_double) {
    update(dt);
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_renderbg(dt: c_double) {
    render_bg(dt);
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_rendermg(dt: c_double) {
    render_mg(dt);
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn spfxL_renderfg(dt: c_double) {
    render_fg(dt);
}
