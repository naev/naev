/// Displays a deprecated message for a Lua function
pub fn deprecated(lua: &mlua::Lua, func: &str, alt: Option<&str>) -> mlua::Result<()> {
   let warn: mlua::Function = lua.globals().get("warn")?;
   let msg = match alt {
      Some(alt) => format!("Deprecated function call: {}\nUse '{}' instead", func, alt),
      None => format!("Deprecated function call: {}", func),
   };
   warn.call::<()>(msg)?;
   Ok(())
}
