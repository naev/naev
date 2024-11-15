use crate::gettext;

pub fn init() -> mlua::Lua {
    let lua = unsafe {
        /* TODO get rid of the lua_init() and move entirely to mlua. */
        naevc::lua_init(); /* initializes lua. */
        mlua::Lua::init_from_ptr(naevc::naevL as *mut mlua::lua_State)
    };
    let globals = lua.globals();
    let gettext_table = lua.create_table().unwrap();

    // Set up gettext stuff
    let gettext = lua
        .create_function(|_lua, msg: String| -> mlua::Result<String> { Ok(gettext::gettext(msg)) })
        .unwrap();
    globals.set("_", gettext.clone()).unwrap();
    gettext_table.set("gettext", gettext).unwrap();
    let gettext_noop = lua
        .create_function(|_lua, msg: String| -> mlua::Result<String> { Ok(msg) })
        .unwrap();
    globals.set("N_", gettext_noop.clone()).unwrap();
    gettext_table.set("gettext_noop", gettext_noop).unwrap();
    let ngettext = lua
        .create_function(
            |_lua, (msg1, msg2, n): (String, String, i32)| -> mlua::Result<String> {
                Ok(gettext::ngettext(msg1, msg2, n))
            },
        )
        .unwrap();
    globals.set("n_", ngettext.clone()).unwrap();
    gettext_table.set("ngettext", ngettext).unwrap();
    let pgettext = lua
        .create_function(
            |_lua, (msg1, msg2): (String, String)| -> mlua::Result<String> {
                Ok(gettext::pgettext(msg1, msg2))
            },
        )
        .unwrap();
    globals.set("p_", pgettext.clone()).unwrap();
    gettext_table.set("pgettext", pgettext).unwrap();
    globals.set("gettext", gettext_table).unwrap();

    // Return it
    lua
}
