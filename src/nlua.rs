//use mlua::prelude::*;
use anyhow::Result;
use constcat::concat;
use mlua::{FromLua, FromLuaMulti, IntoLua, IntoLuaMulti};
use std::sync::LazyLock;

use crate::lua::ryaml;
use gettext::{gettext, ngettext, pgettext};
use log::{info, warn, warn_err, warnx};

const NLUA_LOAD_TABLE: &str = "_LOADED"; // Table to use to store the status of required libraries.
const LUA_INCLUDE_PATH: &str = "scripts/"; // Path for Lua includes.
const LUA_COMMON_PATH: &str = "common.lua"; // Common Lua functions.

/// Variable we use for Lua environments
const ENV: &str = "_ENV";

// For black magic
type CFunctionNaev = unsafe extern "C-unwind" fn(*mut naevc::lua_State) -> i32;
type CFunctionMLua = unsafe extern "C-unwind" fn(*mut mlua::lua_State) -> i32;

#[derive(Debug)]
pub struct LuaEnv {
    pub table: mlua::Table,
    rk: mlua::RegistryKey, // Needed for C API, remove later
}
impl Drop for LuaEnv {
    // TODO something more robust here perhaps
    fn drop(&mut self) {
        let lua = &NLUA;

        // Try to run the __gc function if available
        match self.get("__gc") {
            Ok(mlua::Nil) => (),
            Ok(mlua::Value::Function(func)) => match self.call(lua, &func, ()) {
                Ok(()) => (),
                Err(e) => {
                    warn!("Error calling Lua enviroment __gc function: {e}");
                }
            },
            Err(e) => {
                warn!("Error getting Lua environment __gc function: {e}");
            }
            _ => {
                let name = match self.get::<String>("__name") {
                    Ok(s) => s,
                    Err(e) => {
                        warn!("environment __name not set: {e}");
                        String::from("unknown")
                    }
                };
                warn!("__gc is not a function or nil for environment '{name}'");
            }
        }

        let _ = lua.envs.set(self.rk.id(), mlua::Value::Nil);
    }
}

#[derive(Debug)]
pub struct NLua {
    /// The true Lua environment (to rule them all)
    pub lua: mlua::Lua,
    /// Our globals, these can be masked but not removed
    #[allow(dead_code)]
    globals: mlua::Table,
    /// The metatable for environments, just defaults to our globals
    env_mt: mlua::Table,
    // TODO remove below when we can
    envs: mlua::Table,
}

/// Opens the gettext library
fn open_gettext(lua: &mlua::Lua) -> mlua::Result<()> {
    let globals = lua.globals();
    let gettext_table = lua.create_table()?;

    let gettext = lua.create_function(|_lua, msg: String| -> mlua::Result<String> {
        Ok(gettext(msg.as_str()).to_owned())
    })?;
    globals.set("_", gettext.clone())?;
    gettext_table.set("gettext", gettext)?;
    let gettext_noop =
        lua.create_function(|_lua, msg: String| -> mlua::Result<String> { Ok(msg) })?;
    globals.set("N_", gettext_noop.clone())?;
    gettext_table.set("gettext_noop", gettext_noop)?;
    let ngettext = lua.create_function(
        |_lua, (msg1, msg2, n): (String, String, u64)| -> mlua::Result<String> {
            Ok(ngettext(msg1.as_str(), msg2.as_str(), n).to_owned())
        },
    )?;
    globals.set("n_", ngettext.clone())?;
    gettext_table.set("ngettext", ngettext)?;
    let pgettext = lua.create_function(
        |_lua, (msg1, msg2): (String, String)| -> mlua::Result<String> {
            Ok(pgettext(msg1.as_str(), msg2.as_str()).to_owned())
        },
    )?;
    globals.set("p_", pgettext.clone())?;
    gettext_table.set("pgettext", pgettext)?;
    globals.set("gettext", gettext_table)?;

    Ok(())
}

fn require(lua: &mlua::Lua, filename: mlua::String) -> mlua::Result<mlua::Value> {
    let globals = lua.globals();

    // Try to see if already loaded in the environment
    let loaded: mlua::Table = globals.get(NLUA_LOAD_TABLE)?;
    match loaded.get::<mlua::Value>(&filename)? {
        mlua::Nil => (),
        module => return Ok(module),
    }

    // Try the loaders
    let package: mlua::Table = globals.get("package")?;
    let loaders: mlua::Table = package.get("loaders")?;
    let mut errmsg = String::new();
    for f in loaders.sequence_values::<mlua::Function>() {
        match f?.call::<mlua::Value>(&filename)? {
            mlua::Value::Function(mf) => {
                let mut ret = mf.call::<mlua::Value>(&filename)?;
                if ret == mlua::Value::Nil {
                    ret = mlua::Value::Boolean(true);
                }
                loaded.set(filename, &ret)?;
                return Ok(ret);
            }
            mlua::Value::String(s) => errmsg.push_str(&s.to_str()?),
            _ => (),
        }
    }

    // Failed to load...
    Err(mlua::Error::RuntimeError(format!(
        "module '{filename}'  not found:{errmsg}",
        filename = filename.to_str()?,
        errmsg = errmsg
    )))
}

impl NLua {
    pub fn new() -> Result<NLua> {
        let lua = unsafe {
            // TODO get rid of the lua_init() and move entirely to mlua.
            naevc::lua_init();
            let state = naevc::naevL as *mut mlua::lua_State;
            mlua::ffi::luaopen_base(state); // Needed for ipairs and friends
            mlua::Lua::get_or_init_from_ptr(state)
        };

        // Load base libraries NOT SUFFICIENT
        lua.load_std_libs(mlua::StdLib::ALL_SAFE)?;
        #[cfg(debug_assertions)]
        lua.load_std_libs(mlua::StdLib::DEBUG)?;

        // Set up gettext stuff
        open_gettext(lua)?;

        // Add some more functions.
        let globals = lua.globals();
        globals.set("require", lua.create_function(require)?)?;

        // move [un]pack to table.[un]pack as in Lua5.2.
        let table: mlua::Table = globals.get("table")?;
        let unpack: mlua::Value = globals.get("unpack")?;
        table.set("unpack", unpack)?;
        globals.set("unpack", mlua::Nil)?;

        // Some unsafe overrides.
        globals.set("dofile", mlua::Nil)?;
        globals.set("getfenv", mlua::Nil)?;
        globals.set("load", mlua::Nil)?;
        globals.set("loadfile", mlua::Nil)?;

        // Sandbox "io" and "os".
        globals.set("io", mlua::Nil)?;
        let os_table = lua.create_table()?;
        os_table.set(
            "getenv",
            lua.create_function(|_lua, _s: String| -> mlua::Result<String> {
                // Fake a $HOME
                Ok(String::from("lua_home"))
            })?,
        )?;
        globals.set("os", os_table)?;

        // Special math.
        let math_table: mlua::Table = globals.get("math")?;
        math_table.set("mod", mlua::Nil)?; /* Get rid of math.mod */

        fn escape_colours(msg: &str) -> String {
            use colored::Color;
            fn colour(col: Color) -> String {
                format!("\x1B[{}m", col.to_fg_str())
            }
            let re = regex::Regex::new(r"#(?<c>.)").unwrap();
            let mut m = re
                .replace_all(msg, "\0#$c\0")
                .split('\0')
                .map(|x| match x {
                    "##" => "#".to_string(),
                    "#0" => "\x1B[0m".to_string(),
                    "#b" => colour(Color::Blue),
                    "#g" => colour(Color::Green),
                    "#n" => colour(Color::BrightWhite),
                    "#o" => colour(Color::BrightRed),
                    "#p" => colour(Color::Magenta),
                    "#r" => colour(Color::Red),
                    "#w" => colour(Color::White),
                    "#y" => colour(Color::Yellow),
                    x => x.to_string(),
                })
                .collect::<Vec<_>>()
                .join("");
            if m.contains("\x1B[") {
                m.push_str("\x1B[0m");
            }
            m
        }

        globals.set(
            "print",
            lua.create_function(
                |_lua, args: mlua::Variadic<mlua::Value>| -> mlua::Result<()> {
                    let mut msg = String::new();
                    for a in args {
                        msg.push_str(&a.to_string()?);
                        msg.push('\t');
                    }
                    info!("{}", escape_colours(&msg));
                    let cmsg = std::ffi::CString::new(msg).unwrap();
                    unsafe {
                        naevc::cli_printCoreString(cmsg.as_ptr(), 0);
                    }
                    Ok(())
                },
            )?,
        )?;
        globals.set(
            "warn",
            lua.create_function(
                move |lua, args: mlua::Variadic<mlua::Value>| -> mlua::Result<()> {
                    let mut amsg = String::new();
                    for a in args {
                        amsg.push_str(&a.to_string()?);
                        amsg.push('\t');
                    }
                    let tstr = lua.traceback(None, 1)?;
                    let tmsg = tstr.to_str()?;
                    let emsg = {
                        let mut m = escape_colours(&amsg);
                        m.push('\n');
                        m.push_str(&tmsg);
                        m
                    };
                    warn!("{}", emsg);
                    let cmsg = std::ffi::CString::new([amsg.as_str(), &tmsg].join("")).unwrap();
                    unsafe {
                        naevc::cli_printCoreString(cmsg.as_ptr(), 1);
                    }
                    Ok(())
                },
            )?,
        )?;

        // Load common chunk
        let common_data = ndata::read(LUA_COMMON_PATH)?;
        lua.load(std::str::from_utf8(&common_data)?).exec()?;

        // Swap the global table with an empty one, returning the global one
        let globals: mlua::Table = unsafe {
            lua.exec_raw((), |state| {
                mlua::ffi::lua_pushvalue(state, mlua::ffi::LUA_GLOBALSINDEX);
                mlua::ffi::lua_newtable(state);
                mlua::ffi::lua_replace(state, mlua::ffi::LUA_GLOBALSINDEX);
            })
        }?;

        // Set globals to be truly read only (should only be possible if the user is being bad)
        let globals_mt = lua.create_table()?;
        globals_mt.set(
            "__newindex",
            lua.create_function(|_, ()| -> mlua::Result<()> {
                Err(mlua::Error::RuntimeError(String::from(
                    "globals are read only",
                )))
            })?,
        )?;
        globals.set_metatable(Some(globals_mt))?;

        // Our new globals should be usable now
        let wrapped = lua.globals();
        let wrapped_mt = lua.create_table()?;
        wrapped_mt.set(
            "__index",
            lua.create_function(
                |lua, (t, k): (mlua::Table, mlua::Value)| -> mlua::Result<mlua::Value> {
                    let env_str = lua.create_string(ENV)?;
                    match k == mlua::Value::String(env_str) {
                        true => t.raw_get(k),
                        false => {
                            let e: mlua::Table = t.raw_get(ENV)?;
                            e.get(k)
                        }
                    }
                },
            )?,
        )?;
        wrapped_mt.set(
            "__newindex",
            lua.create_function(
                |_, (t, k, v): (mlua::Table, mlua::Value, mlua::Value)| -> mlua::Result<()> {
                    let e: mlua::Table = t.raw_get(ENV)?;
                    e.set(k, v)
                },
            )?,
        )?;
        wrapped.set_metatable(Some(wrapped_mt))?;

        let env_mt = lua.create_table()?;
        env_mt.set("__index", globals.clone())?;

        let envs = lua.create_table()?;

        // Return it
        Ok(NLua {
            globals,
            env_mt,
            envs: envs.clone(),
            lua: lua.clone(),
        })
    }

    pub fn environment_new(&self, name: &str) -> Result<LuaEnv> {
        let lua = &self.lua;
        let t = lua.create_table()?;

        t.set("__name", name)?;

        // Metatable
        t.set_metatable(Some(self.env_mt.clone()))?;

        // Set up paths.
        // "package.path" to look in the data.
        // "package.cpath" unset
        let package: mlua::Table = t.get("package")?;
        let pt = lua.create_table()?;
        t.set(NLUA_LOAD_TABLE, pt.clone())?;
        package.set("loaded", pt)?;
        package.set("preload", lua.create_table()?)?;
        package.set("path", concat!("?.lua;", LUA_INCLUDE_PATH, "?.lua"))?;
        package.set("cpath", "")?;
        let loaders: mlua::Table = package.get("loaders")?;
        loaders.push(
            // Pure Rust Optional libraries
            lua.create_function(|lua, name: String| -> mlua::Result<mlua::Value> {
                match name.as_str() {
                    "ryaml" => Ok(mlua::Value::Function(lua.create_function(
                        |lua, ()| -> mlua::Result<mlua::Table> { ryaml::ryaml_safe(lua) },
                    )?)),
                    "utf8" => unsafe {
                        Ok(mlua::Value::Function(lua.create_c_function(
                            std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                                naevc::luaopen_utf8,
                            ),
                        )?))
                    },
                    "cmark" => unsafe {
                        Ok(mlua::Value::Function(lua.create_c_function(
                            std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                                naevc::luaopen_cmark,
                            ),
                        )?))
                    },
                    "enet" => match unsafe { naevc::conf.lua_enet } {
                        0 => Ok(mlua::Value::Nil),
                        _ => unsafe {
                            Ok(mlua::Value::Function(lua.create_c_function(
                                std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                                    naevc::luaopen_enet,
                                ),
                            )?))
                        },
                    },
                    _ => Ok(mlua::Value::Nil),
                }
            })?,
        )?;
        loaders.push(
            lua.create_function(|lua, name: String| -> mlua::Result<mlua::Value> {
                let globals = lua.globals();
                let package_val: mlua::Value = globals.get("package")?;
                let package: mlua::Table = match package_val {
                    mlua::Value::Table(t) => t,
                    _ => {
                        return Ok(mlua::Value::String(
                            lua.create_string(gettext(" package not found."))?,
                        ));
                    }
                };
                let preload: mlua::Table = package.get("preload")?;
                let lib: mlua::Value = preload.get(name.clone())?;
                match lib {
                    mlua::Value::Nil => Ok(mlua::Value::String(
                        lua.create_string(format!("\n\tno field package.preload['{name}']"))?,
                    )),
                    v => Ok(v),
                }
            })?,
        )?;
        // TODO reimplement in rust...
        unsafe {
            loaders.push(
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::nlua_package_loader_lua,
                ))?,
            )?;
        }

        // Global state table _G should refer back to the environment.
        t.set("_G", t.clone())?;

        // Push if Naev is doing debugging
        #[cfg(debug_assertions)]
        t.set("__debugging", true)?;

        // Set up naev namespace. */
        t.set("naev", lua.create_table()?)?;

        // Store based on ID
        let rk = lua.create_registry_value(t.clone())?;
        self.envs.raw_set(rk.id(), t.clone())?;

        // Environment is all set
        Ok(LuaEnv { rk, table: t })
    }

    /// Calls a function with the environment
    pub fn environment_call<R: FromLuaMulti>(
        &self,
        env: mlua::Table,
        func: &mlua::Function,
        args: impl IntoLuaMulti,
    ) -> Result<R> {
        let globals = self.lua.globals();
        let prev_env: Option<mlua::Table> = globals.raw_get(ENV)?;
        globals.raw_set(ENV, env)?;
        let ret = func.call(args);
        if let Some(prev_env) = prev_env {
            globals.raw_set(ENV, prev_env)?;
        }
        Ok(ret?)
    }

    /// Handles resizing
    pub fn resize(&self, width: i32, height: i32) -> Result<()> {
        for pair in self.envs.pairs::<i32, mlua::Table>() {
            let (_key, value) = pair?;
            let resize: mlua::Value = value.get("__resize")?;
            match resize {
                mlua::Value::Nil => Ok(()),
                mlua::Value::Function(mf) => {
                    self.environment_call::<()>(value, &mf, (width, height))
                }
                _ => {
                    let name = value.get::<String>("__name")?;
                    warnx!(
                        gettext("__resize is not a function or nil for environment '{}'"),
                        name
                    );
                    Ok(())
                }
            }?;
        }
        Ok(())
    }
}

impl LuaEnv {
    pub fn current(lua: &mlua::Lua) -> Option<Self> {
        let tbl: mlua::Table = match lua.globals().raw_get(crate::nlua::ENV) {
            Ok(t) => t,
            Err(e) => {
                warn_err!(e);
                return None;
            }
        };
        match LuaEnv::from_table(lua, tbl) {
            Ok(env) => Some(env),
            Err(e) => {
                warn_err!(e);
                None
            }
        }
    }

    /// Sets up a Lua environment from a table. Assumes the table exists.
    fn from_table(lua: &mlua::Lua, table: mlua::Table) -> Result<Self> {
        let rk = lua.create_registry_value(table.clone())?;
        Ok(LuaEnv { table, rk })
    }

    /// Gets a value from the environment
    pub fn get<V: FromLua>(&self, key: impl IntoLua) -> mlua::Result<V> {
        self.table.get(key)
    }

    /// Sets a value in the environment
    pub fn set(&self, key: impl IntoLua, value: impl IntoLua) -> mlua::Result<()> {
        self.table.set(key, value)
    }

    /// Calls a function with the environment
    pub fn call<R: FromLuaMulti>(
        &self,
        lua: &NLua,
        func: &mlua::Function,
        args: impl IntoLuaMulti,
    ) -> Result<R> {
        lua.environment_call(self.table.clone(), func, args)
    }

    pub fn load_standard(&mut self, lua: &NLua) -> Result<()> {
        // Load naev library first as it gets populated with additonal entries later
        match unsafe {
            let env = self as *mut LuaEnv as *mut naevc::nlua_env;
            naevc::nlua_loadNaev(env)
        } {
            0 => (),
            _ => anyhow::bail!("unable to load standard libraries!"),
        };

        // Load true Rust dependencies
        let naev: mlua::Table = self.table.raw_get("naev")?;
        let open_lib = |name: &str,
                        open: fn(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData>|
         -> Result<()> {
            let lib = open(&lua.lua)?;
            self.set(name, &lib)?;
            naev.raw_set(name, &lib)?;
            Ok(())
        };
        open_lib("vec2", physics::vec2::open_vec2)?;
        open_lib("colour", renderer::colour::open_colour)?;
        open_lib("rnd", crate::rng::open_rnd)?;
        open_lib("spfx", crate::spfx::open_spfx)?;
        open_lib("audiodata", audio::open_audiodata)?;
        open_lib("audio", audio::open_audio)?;
        // TODO tex has lots of dependencies, and we can't implement the FFI interface so it is
        // disabled. Similarly, file doesn't have ffi set up.
        //open_lib("file", ndata::lua::open_file)?;
        //open_lib("tex", renderer::texture::open_texture)?;
        //open_lib("gfx", renderer::open_gfx)?;

        let ret = unsafe {
            let env = self as *mut LuaEnv as *mut naevc::nlua_env;
            let mut r: c_int = 0;
            r |= naevc::nlua_loadVar(env);
            r |= naevc::nlua_loadSpob(env);
            r |= naevc::nlua_loadSystem(env);
            r |= naevc::nlua_loadJump(env);
            r |= naevc::nlua_loadTime(env);
            r |= naevc::nlua_loadPlayer(env);
            r |= naevc::nlua_loadPilot(env);
            r |= naevc::nlua_loadDiff(env);
            r |= naevc::nlua_loadFaction(env);
            r |= naevc::nlua_loadOutfit(env);
            r |= naevc::nlua_loadCommodity(env);
            r |= naevc::nlua_loadNews(env);
            r |= naevc::nlua_loadShiplog(env);
            r |= naevc::nlua_loadFile(env);
            r |= naevc::nlua_loadData(env);
            r |= naevc::nlua_loadLinOpt(env);
            r |= naevc::nlua_loadSafelanes(env);
            //r |= naevc::nlua_loadSpfx(env);
            //r |= naevc::nlua_loadAudio(env);
            r
        };
        match ret {
            0 => Ok(()),
            _ => anyhow::bail!("unable to load standard libraries!"),
        }
    }
}

impl Clone for LuaEnv {
    fn clone(&self) -> Self {
        let t = &self.table;
        let lua = &&NLUA;
        LuaEnv {
            table: t.clone(),
            rk: lua.lua.create_registry_value(t).unwrap(),
        }
    }
}

pub static NLUA: LazyLock<NLua> = LazyLock::new(|| NLua::new().unwrap());
pub fn init() -> Result<()> {
    let _unused = &NLUA;
    Ok(())
}

// Re-export some newer Lua API to C
use std::os::raw::{c_char, c_int};

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn luaL_tolstring(
    L: *mut mlua::lua_State,
    idx: c_int,
    len: *mut usize,
) -> *const c_char {
    unsafe { mlua::ffi::luaL_tolstring(L, idx, len) }
}

use std::ffi::CStr;

// C API
#[unsafe(no_mangle)]
pub extern "C" fn nlua_newEnv(name: *const c_char) -> *mut LuaEnv {
    let ptr = unsafe { CStr::from_ptr(name) };
    let name = ptr.to_str().unwrap();
    let lua = &NLUA;
    match lua.environment_new(name) {
        Ok(env) => Box::into_raw(Box::new(env)),
        Err(e) => {
            warn!("unable to create Lua environment: {e}");
            std::ptr::null_mut()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_dupEnv(env: *mut LuaEnv) -> *mut LuaEnv {
    if env.is_null() {
        return env;
    }
    let env = unsafe { &*env };
    Box::into_raw(Box::new(env.clone()))
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_freeEnv(env: *mut LuaEnv) {
    if !env.is_null() {
        let _ = unsafe { Box::from_raw(env) };
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_pushenv(lua: *mut mlua::lua_State, env: *mut LuaEnv) {
    let env = unsafe { &*env };
    unsafe {
        mlua::ffi::lua_rawgeti(lua, mlua::ffi::LUA_REGISTRYINDEX, env.rk.id().into());
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_resize() {
    let lua = &NLUA;
    let (screen_w, screen_h) = unsafe { (naevc::gl_screen.w, naevc::gl_screen.h) };
    if let Err(e) = lua.resize(screen_w, screen_h) {
        warn_err!(e);
    };
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_loadStandard(env: *mut LuaEnv) -> c_int {
    if env.is_null() {
        return -1;
    }
    let nlua = &NLUA;
    let env = unsafe { &mut *env };
    match env.load_standard(nlua) {
        Ok(()) => 0,
        Err(e) => {
            warn_err!(e);
            -1
        }
    }
}
