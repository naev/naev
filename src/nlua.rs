use crate::gettext;
use crate::ndata;
use constcat::concat;
use mlua::{FromLua, IntoLua};

const NLUA_LOAD_TABLE: &str = "_LOADED"; // Table to use to store the status of required libraries.
const LUA_INCLUDE_PATH: &str = "scripts/"; // Path for Lua includes.
const LUA_COMMON_PATH: &str = "common.lua"; // Common Lua functions.

pub struct LuaEnv<'a> {
    lua: &'a mlua::Lua,
    table: mlua::Table,
    rk: mlua::RegistryKey,
}

#[allow(dead_code)]
pub struct NLua<'a> {
    pub lua: mlua::Lua,
    common: mlua::Function,
    envs: Vec<LuaEnv<'a>>,
}

/// Opens the gettext library
fn open_gettext(lua: &mlua::Lua) -> mlua::Result<()> {
    let globals = lua.globals();
    let gettext_table = lua.create_table()?;

    let gettext = lua.create_function(|_lua, msg: String| -> mlua::Result<String> {
        Ok(gettext::gettext(msg))
    })?;
    globals.set("_", gettext.clone())?;
    gettext_table.set("gettext", gettext)?;
    let gettext_noop =
        lua.create_function(|_lua, msg: String| -> mlua::Result<String> { Ok(msg) })?;
    globals.set("N_", gettext_noop.clone())?;
    gettext_table.set("gettext_noop", gettext_noop)?;
    let ngettext = lua.create_function(
        |_lua, (msg1, msg2, n): (String, String, i32)| -> mlua::Result<String> {
            Ok(gettext::ngettext(msg1, msg2, n))
        },
    )?;
    globals.set("n_", ngettext.clone())?;
    gettext_table.set("ngettext", ngettext)?;
    let pgettext = lua.create_function(
        |_lua, (msg1, msg2): (String, String)| -> mlua::Result<String> {
            Ok(gettext::pgettext(msg1, msg2))
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

impl NLua<'_> {
    pub fn new() -> NLua<'static> {
        let lua = unsafe {
            // TODO get rid of the lua_init() and move entirely to mlua.
            naevc::lua_init();
            mlua::Lua::init_from_ptr(naevc::naevL as *mut mlua::lua_State)
        };

        // Load base libraries
        //lua.load_std_libs( mlua::StdLib::ALL_SAFE ).unwrap();

        // Minor sandboxing
        sandbox(&lua).unwrap();

        // Set up gettext stuff
        open_gettext(&lua).unwrap();

        // Load common chunk
        let data = ndata::read(String::from(LUA_COMMON_PATH)).unwrap();
        let common = std::str::from_utf8(&data).unwrap();
        let chunk = lua.load(common).into_function().unwrap();

        // Return it
        NLua {
            lua,
            envs: Vec::new(),
            common: chunk,
        }
    }

    pub fn new_env(&mut self, name: &str) -> mlua::Result<LuaEnv> {
        let lua = &self.lua;
        let t = lua.create_table()?;

        t.set("__name", name)?;

        // Metatable
        let m = lua.create_table()?;
        m.set("__index", lua.globals())?;
        t.set_metatable(Some(m));

        // Replace require.
        t.set("require", lua.create_function(require)?)?;

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
        // TODO reimplement in rust...
        unsafe {
            type CFunctionNaev = unsafe extern "C-unwind" fn(*mut naevc::lua_State) -> i32;
            type CFunctionMLua = unsafe extern "C-unwind" fn(*mut mlua::lua_State) -> i32;
            loaders.push(
                lua.create_c_function(std::mem::transmute::<CFunctionNaev, CFunctionMLua>(
                    naevc::nlua_package_preload,
                ))?,
            )?;
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
            loaders.push(lua.create_c_function(std::mem::transmute::<
                CFunctionNaev,
                CFunctionMLua,
            >(
                naevc::nlua_package_loader_croot
            ))?)?;
        }

        // Global state table _G should refer back to the environment.
        t.set("_G", t.clone())?;

        // Push if Naev is doing debugging
        #[cfg(debug_assertions)]
        t.set("__debugging", true)?;

        // Set up naev namespace. */
        t.set("naev", lua.create_table()?)?;

        // Load common script
        self.common.set_environment(t.clone())?;
        self.common.call::<()>(())?;

        Ok(LuaEnv {
            lua,
            table: t.clone(),
            rk: lua.create_registry_value(t)?,
        })
    }
}

#[allow(dead_code)]
impl LuaEnv<'_> {
    fn get<V: FromLua>(self, key: impl IntoLua) -> mlua::Result<V> {
        //let t: mlua::Table = self.rk.into_lua( self.lua )?.into();
        self.table.get(key)
    }
    fn set(self, key: impl IntoLua, value: impl IntoLua) -> mlua::Result<()> {
        self.table.set(key, value)
    }
    fn id(self) -> std::os::raw::c_int {
        self.rk.id()
    }
}
