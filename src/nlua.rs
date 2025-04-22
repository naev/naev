//use mlua::prelude::*;
use crate::gettext::{gettext, ngettext, pgettext};
use crate::ndata;
use anyhow::Result;
use constcat::concat;
use mlua::{FromLua, FromLuaMulti, IntoLua, IntoLuaMulti};

#[allow(dead_code)]
const NLUA_LOAD_TABLE: &str = "_LOADED"; // Table to use to store the status of required libraries.
#[allow(dead_code)]
const LUA_INCLUDE_PATH: &str = "scripts/"; // Path for Lua includes.
const LUA_COMMON_PATH: &str = "common.lua"; // Common Lua functions.

/// Variable we use for Lua environments
const ENV: &str = "_ENV";

// For black magic
type CFunctionNaev = unsafe extern "C-unwind" fn(*mut naevc::lua_State) -> i32;
type CFunctionMLua = unsafe extern "C-unwind" fn(*mut mlua::lua_State) -> i32;

#[allow(dead_code)]
pub struct LuaEnv {
    table: mlua::Table,
    rk: mlua::RegistryKey,
}

#[allow(dead_code)]
pub struct NLua {
    /// The true Lua environment (to rule them all)
    pub lua: mlua::Lua,
    /// Our globals, these can be masked but not removed
    globals: mlua::Table,
    /// The metatable for environments, just defaults to our globals
    env_mt: mlua::Table,
    clua: *mut mlua::lua_State, // TODO remove when we can
    envs: Vec<LuaEnv>,
}
// Got to remove this when we can...
unsafe impl Sync for NLua {}
unsafe impl Send for NLua {}

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
        |_lua, (msg1, msg2, n): (String, String, i32)| -> mlua::Result<String> {
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

fn sandbox(lua: &mlua::Lua) -> mlua::Result<()> {
    let globals = lua.globals();

    /* Override built-ins to use Naev for I/O. */
    /*
    lua_register( L, "print", cli_print );
    lua_register( L, "printRaw", cli_printRaw );
    lua_register( L, "warn", cli_warn );
    */

    // move [un]pack to table.[un]pack as in Lua5.2.
    let table: mlua::Table = globals.get("table")?;
    let unpack: mlua::Value = globals.get("unpack")?;
    table.set("unpack", unpack)?;
    globals.set("unpack", mlua::Nil)?;

    // Some overrides.
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

    //

    Ok(())
}

#[allow(dead_code)]
fn require(lua: &mlua::Lua, filename: mlua::String) -> mlua::Result<mlua::Value> {
    let globals = lua.globals();

    // Try to see if already loaded in the environment
    let loaded: mlua::Table = globals.get(NLUA_LOAD_TABLE)?;
    match loaded.get::<mlua::Value>(filename.clone())? {
        mlua::Nil => (),
        module => return Ok(module),
    }

    // Try the loaders
    let package: mlua::Table = globals.get("package")?;
    let loaders: mlua::Table = package.get("loaders")?;
    let mut errmsg = String::new();
    for f in loaders.sequence_values::<mlua::Function>() {
        match f?.call::<mlua::Value>(filename.clone())? {
            mlua::Value::Function(mf) => {
                let mut ret = mf.call::<mlua::Value>(filename)?;
                if ret == mlua::Value::Nil {
                    ret = mlua::Value::Boolean(true);
                }
                loaded.set("filename", mf.clone())?;
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
            mlua::Lua::init_from_ptr(naevc::naevL as *mut mlua::lua_State)
        };

        // Load base libraries NOT SUFFICIENT
        //lua.load_std_libs( mlua::StdLib::ALL_SAFE ).unwrap();

        // Minor sandboxing
        sandbox(&lua)?;

        // Set up gettext stuff
        open_gettext(&lua)?;

        // Add some mor functions.
        let globals = lua.globals();
        //globals.set("require", lua.create_function(require)?)?;
        unsafe {
            globals.set(
                "require",
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::nlua_require,
                ))?,
            )?;
            globals.set(
                "print",
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::cli_print,
                ))?,
            )?;
            globals.set(
                "printRaw",
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::cli_printRaw,
                ))?,
            )?;
            globals.set(
                "warn",
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::cli_warn,
                ))?,
            )?;
        }

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
        globals.set_metatable(Some(globals_mt));

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
        wrapped.set_metatable(Some(wrapped_mt));

        let env_mt = lua.create_table()?;
        env_mt.set("__index", globals.clone())?;

        // Return it
        Ok(NLua {
            lua,
            globals,
            env_mt,
            envs: Vec::new(),
            clua: unsafe { naevc::naevL as *mut mlua::lua_State },
        })
    }

    #[allow(dead_code)]
    pub fn environment_new(&mut self, name: &str) -> mlua::Result<LuaEnv> {
        let lua = &self.lua;
        let t = lua.create_table()?;

        t.set("__name", name)?;

        // Metatable
        t.set_metatable(Some(self.env_mt.clone()));

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
                        lua.create_string(format!("\n\tno field package.preload['{}']", name))?,
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
            loaders.push(
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::nlua_package_loader_c,
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

        Ok(LuaEnv {
            table: t.clone(),
            rk: lua.create_registry_value(t)?,
        })
    }

    #[allow(dead_code)]
    /// Calls a function with the environment
    pub fn environment_call<R: FromLuaMulti>(
        &self,
        env: &LuaEnv,
        func: &mlua::Function,
        args: impl IntoLuaMulti,
    ) -> mlua::Result<R> {
        self.lua.globals().raw_set(ENV, env.table.clone())?;
        func.call(args)
    }
}

#[allow(dead_code)]
impl LuaEnv {
    /// Gets a value from the environment
    pub fn get<V: FromLua>(&self, key: impl IntoLua) -> mlua::Result<V> {
        //let t: mlua::Table = self.rk.into_lua( self.lua )?.into();
        self.table.get(key)
    }

    /// Sets a value in the environment
    pub fn set(&self, key: impl IntoLua, value: impl IntoLua) -> mlua::Result<()> {
        self.table.set(key, value)
    }

    // Gets the ID
    fn id(&self) -> std::os::raw::c_int {
        self.rk.id()
    }
}

use std::sync::{LazyLock, Mutex};
static NLUA: LazyLock<Mutex<NLua>> = LazyLock::new(|| Mutex::new(NLua::new().unwrap()));
pub fn init() -> Result<()> {
    let _unused = NLUA.lock();
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

/*
#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn luaL_traceback(
    L: *mut mlua::lua_State,
    L1: *mut mlua::lua_State,
    msg: *const c_char,
    level: c_int,
) {
    unsafe { mlua::ffi::luaL_traceback(L, L1, msg, level) }
}
*/

/*
*/
use crate::{formatx, warn};
use std::ffi::CStr;

// C API
#[unsafe(no_mangle)]
pub extern "C" fn nlua_newEnv(name: *const c_char) -> *mut LuaEnv {
    let ptr = unsafe { CStr::from_ptr(name) };
    let name = ptr.to_str().unwrap();
    let mut lua = NLUA.lock().unwrap();
    match lua.environment_new(name) {
        Ok(env) => Box::into_raw(Box::new(env)),
        Err(e) => {
            warn!("unable to create Lua environment: {}", e);
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
    let t = &env.table;
    let lua = &NLUA.lock().unwrap();
    let newenv = LuaEnv {
        table: t.clone(),
        rk: lua.lua.create_registry_value(t).unwrap(),
    };
    Box::into_raw(Box::new(newenv))
}

#[unsafe(no_mangle)]
pub extern "C" fn nlua_freeEnv(env: *mut LuaEnv) {
    if !env.is_null() {
        let _ = unsafe { Box::from_raw(env) };
    }
}

/*
#[unsafe(no_mangle)]
pub extern "C" fn nlua_dobufenv( env: *mut LuaEnv, buf: *const c_char, sz: usize, name: *const c_char ) -> c_int {
    if env.is_null() {
        return -1;
    }
    let buf = unsafe { std::str::from_utf8_unchecked(std::slice::from_raw_parts( buf as *const u8, sz)) };
    let nameptr = unsafe { CStr::from_ptr(name) };
    let name = nameptr.to_str().unwrap();
    let env = unsafe { &*env };

    let lua = &NLUA.lock().unwrap();
    let chunk = lua.lua.load(buf)
        .set_name( name )
        .set_environment( env.table.clone() );
    match chunk.exec() {
        Ok(()) => 0,
        Err(e) => {
            warn!("{}",e);
            -1
        },
    }
}
*/

#[unsafe(no_mangle)]
pub extern "C" fn nlua_pushenv(lua: *mut mlua::lua_State, env: *mut LuaEnv) {
    let env = unsafe { &*env };
    unsafe {
        mlua::ffi::lua_rawgeti(lua, mlua::ffi::LUA_REGISTRYINDEX, env.rk.id().into());
    }
}
