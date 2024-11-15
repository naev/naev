pub fn init() -> mlua::Lua {
    unsafe {
        /* TODO get rid of the lua_init() and move entirely to mlua. */
        naevc::lua_init(); /* initializes lua. */
        mlua::Lua::init_from_ptr(naevc::naevL as *mut mlua::lua_State)
    }
}
