use crate::gettext;

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

    /* move [un]pack to table.[un]pack as in Lua5.2 */
    let table: mlua::Table = globals.get("table")?;
    let unpack: mlua::Value = globals.get("unpack")?;
    table.set("unpack", unpack)?;
    globals.set("unpack", mlua::Nil)?;

    /* Some overrides. */
    globals.set("dofile", mlua::Nil)?;
    globals.set("getfenv", mlua::Nil)?;
    globals.set("load", mlua::Nil)?;
    globals.set("loadfile", mlua::Nil)?;

    /* Sandbox "io" and "os". */
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

    /* Special math. */
    let math_table: mlua::Table = globals.get("math")?;
    math_table.set("mod", mlua::Nil)?; /* Get rid of math.mod */

    Ok(())
}

pub fn init() -> mlua::Lua {
    let lua = unsafe {
        /* TODO get rid of the lua_init() and move entirely to mlua. */
        naevc::lua_init(); /* initializes lua. */
        mlua::Lua::init_from_ptr(naevc::naevL as *mut mlua::lua_State)
    };

    // Load base libraries
    //lua.load_std_libs( mlua::StdLib::ALL_SAFE ).unwrap();

    // Minor sandboxing
    sandbox(&lua).unwrap();

    // Set up gettext stuff
    open_gettext(&lua).unwrap();

    // Return it
    lua
}
