// Originally from https://github.com/khvzak/lua-ryaml
// Forked to https://github.com/bobbens/lua-ryaml to use serde-yaml2 and PR opened upstream
// We use the later with minor modifications so we can embed it

use mlua::{Error, ExternalError, Function, Lua, LuaSerdeExt, Nil, Result, Table, Value};

/// Decodes input value (must be a string) that represents a yaml document to a Lua value
fn decode(lua: &Lua, s: Value) -> Result<Value> {
    let s = match s {
        Value::String(ref s) => Ok(s.to_str()?),
        _ => Err(format!("invalid input type: {}", s.type_name()).into_lua_err()),
    }?;
    let yaml_value: serde_yaml2::wrapper::YamlNodeWrapper =
        serde_yaml2::from_str(&s).map_err(|e| Error::external(e.to_string()))?;
    lua.to_value(&yaml_value)
}

/// Encodes Lua value (any supported) to a yaml document
fn encode(lua: &Lua, v: Value) -> Result<Value> {
    let s = serde_yaml2::to_string(v).map_err(|e| Error::external(e.to_string()))?;
    lua.create_string(&s).map(Value::String)
}

fn make_exports(lua: &Lua, decode: Function, encode: Function) -> Result<Table> {
    let exports = lua.create_table()?;
    exports.set("load", decode.clone())?;
    exports.set("decode", decode)?;
    exports.set("dump", encode.clone())?;
    exports.set("encode", encode)?;
    exports.set("null", lua.null())?;
    exports.set("array_mt", lua.array_metatable())?;
    Ok(exports)
}

#[allow(dead_code)]
//#[mlua::lua_module]
pub fn ryaml(lua: &Lua) -> Result<Table> {
    let decode = lua.create_function(decode)?;
    let encode = lua.create_function(encode)?;
    make_exports(lua, decode, encode)
}

//#[mlua::lua_module]
pub fn ryaml_safe(lua: &Lua) -> Result<Table> {
    let decode = lua.create_function(|lua, s| match decode(lua, s) {
        Ok(v) => Ok((v, None)),
        Err(e) => Ok((Nil, Some(e.to_string()))),
    })?;
    let encode = lua.create_function(|lua, v| match encode(lua, v) {
        Ok(s) => Ok((s, None)),
        Err(e) => Ok((Nil, Some(e.to_string()))),
    })?;
    make_exports(lua, decode, encode)
}
