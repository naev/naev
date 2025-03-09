use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::Vector2;
use std::sync::Arc;

#[derive(Copy, Clone, derive_more::From, derive_more::Into)]
struct Vec2(Vector2<f64>);

impl Vec2 {
    pub fn new(x: f64, y: f64) -> Self {
        Vec2(Vector2::new(x, y))
    }

    fn lua_add(self, val: Value) -> mlua::Result<Self> {
        let vec = self.0;
        match val {
            Value::UserData(ud) if ud.is::<Self>() => {
                let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                Ok(Vec2::new(vec.x + vec2.x, vec.y + vec2.y))
            }
            Value::Number(n) => Ok(Vec2::new(vec.x + n, vec.y + n)),
            Value::Integer(n) => Ok(Vec2::new(vec.x + n as f64, vec.y + n as f64)),
            _ => Err(mlua::Error::BadArgument {
                to: Some(String::from("Vec2")),
                pos: 2,
                name: Some(String::from("val")),
                cause: Arc::new(mlua::Error::UserDataTypeMismatch),
            }),
        }
    }
}

impl FromLua for Vec2 {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            _ => unreachable!(),
        }
    }
}

/// @brief Represents a 2D vector in Lua.
///
/// This module allows you to manipulate 2D vectors.  Usage is generally as
/// follows:
///
/// @code
/// my_vec = vec2.new( 3, 2 ) -- my_vec is now (3,2)
/// my_vec:add( 5, 3 ) -- my_vec is now (8,5)
/// my_vec = my_vec * 3 -- my_vec is now (24,15)
/// your_vec = vec2.new( 5, 2 ) -- your_vec is now (5,2)
/// my_vec = my_vec - your_vec -- my_vec is now (19,13)
/// @endcode
///
/// To call members of the metatable always use:
/// @code
/// vector:function( param )
/// @endcode
///
/// @luamod vec2
#[allow(unused_doc_comments)]
impl UserData for Vec2 {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        fields.add_field_method_get("x", |_, this| Ok(this.0.x));
        fields.add_field_method_get("y", |_, this| Ok(this.0.y));
    }
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /// @brief Creates a new vector.
        ///
        /// @usage vec2.new( 5, 3 ) -- creates a vector at (5,3)
        /// @usage vec2.new() -- creates a vector at (0,0)
        ///
        ///    @luatparam[opt=0] number x If set, the X value for the new vector.
        ///    @luatparam[opt=x] number y If set, the Y value for the new vector.
        ///    @luatreturn Vec2 The new vector.
        /// @luafunc new
        methods.add_function("new", |_, (x, y): (f64, f64)| Ok(Vec2::new(x, y)));

        /// @brief Creates a new vector using polar coordinates.
        ///
        /// @usage vec2.newP( 1000, math.pi/2 ) -- creates a vector at (0,1000)
        /// @usage vec2.newP() -- creates a vector at (0,0)
        ///
        ///    @luatparam[opt=0] number m If set, the modulus for the new vector.
        ///    @luatparam[opt=0] number a If set, the angle for the new vector, in
        /// radians.
        ///    @luatreturn Vec2 The new vector.
        /// @luafunc newP
        methods.add_function("newP", |_, (m, a): (f64, f64)| {
            Ok(Vec2::new(m * a.cos(), m * a.sin()))
        });

        /// @brief Creates a clone of a vector.
        ///
        ///    @luatparam Vec2 v Vector to clone.
        ///    @luatreturn Vec2 A clone of v.
        /// @luafunc clone
        methods.add_method("clone", |_, vec, ()| Ok(vec.clone()));

        /// @brief Converts a vector to a string.
        ///
        ///    @luatparam Vector v Vector to convert to as string.
        ///    @luatreturn string String version of v.
        /// @luafunc __tostring
        methods.add_meta_function(MetaMethod::ToString, |_, vec: Self| {
            Ok(format!("vec2( {}, {} )", vec.0.x, vec.0.y))
        });

        /// @brief Adds two vectors or a vector and some cartesian coordinates.
        ///
        /// If x is a vector it adds both vectors, otherwise it adds cartesian
        /// coordinates to the vector.
        ///
        /// @usage my_vec = my_vec + your_vec
        /// @usage my_vec:add( your_vec )
        /// @usage my_vec:add( 5, 3 )
        ///
        ///    @luatparam Vector v Vector getting stuff added to.
        ///    @luatparam number|Vec2 x X coordinate or vector to add to.
        ///    @luatparam number|nil y Y coordinate or nil to add to.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc add
        methods.add_meta_function(MetaMethod::Add, |_, (vec, val): (Self, Value)| {
            vec.lua_add(val)
        });
        methods.add_method("add", |_, vec, val: Value| vec.lua_add(val));

        /// @brief Subtracts two vectors or a vector and some cartesian coordinates.
        ///
        /// If x is a vector it subtracts both vectors, otherwise it subtracts cartesian
        /// coordinates to the vector.
        ///
        /// @usage my_vec = my_vec - your_vec
        /// @usage my_vec:sub( your_vec )
        /// @usage my_vec:sub( 5, 3 )
        ///
        ///    @luatparam Vec2 v Vector getting stuff subtracted from.
        ///    @luatparam number|Vec2 x X coordinate or vector to subtract.
        ///    @luatparam number|nil y Y coordinate or nil to subtract.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc sub
        methods.add_meta_function(MetaMethod::Sub, |_, (vec1, vec2): (Self, Self)| {
            let (vec1, vec2) = (vec1.0, vec2.0);
            Ok(Vec2::new(vec1.x - vec2.x, vec1.y - vec2.y))
        });
        methods.add_method("sub", |_, vec1, vec2: Self| {
            let (vec1, vec2) = (vec1.0, vec2.0);
            Ok(Vec2::new(vec1.x - vec2.x, vec1.y - vec2.y))
        });

        /// @brief Multiplies a vector by a number.
        ///
        /// @usage my_vec = my_vec * 3
        /// @usage my_vec:mul( 3 )
        ///
        ///    @luatparam Vec2 v Vector to multiply.
        ///    @luatparam number mod Amount to multiply by.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc mul
        methods.add_meta_function(MetaMethod::Mul, |_, (vec1, vec2): (Self, Self)| {
            let (vec1, vec2) = (vec1.0, vec2.0);
            Ok(Vec2::new(vec1.x * vec2.x, vec1.y * vec2.y))
        });
        methods.add_method("mul", |_, vec1, vec2: Self| {
            let (vec1, vec2) = (vec1.0, vec2.0);
            Ok(Vec2::new(vec1.x * vec2.x, vec1.y * vec2.y))
        });

        /// @brief Divides a vector by a number.
        ///
        /// @usage my_vec = my_vec / 3
        /// @usage my_vec:div(3)
        ///
        ///    @luatparam Vec2 v Vector to divide.
        ///    @luatparam number mod Amount to divide by.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc div
        methods.add_method("magnitude", |_, vec, ()| {
            let vec = vec.0;
            let mag_squared = vec.x * vec.x + vec.y * vec.y;
            Ok(mag_squared.sqrt())
        });
    }
}

pub fn open_vec2(lua: &mlua::Lua) -> anyhow::Result<()> {
    let globals = lua.globals();
    globals.set("vec2", Vec2::new(0.0, 0.0))?;
    Ok(())
}
