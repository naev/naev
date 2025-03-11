use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::Vector2;
use std::sync::Arc;
//use std::os::raw::c_int;

#[derive(Copy, Clone, derive_more::From, derive_more::Into)]
pub struct Vec2(Vector2<f64>);

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
                to: Some(String::from("Vec2 | number")),
                pos: 2,
                name: Some(String::from("val")),
                cause: Arc::new(mlua::Error::UserDataTypeMismatch),
            }),
        }
    }

    fn lua_sub(self, val: Value) -> mlua::Result<Self> {
        let vec = self.0;
        match val {
            Value::UserData(ud) if ud.is::<Self>() => {
                let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                Ok(Vec2::new(vec.x - vec2.x, vec.y - vec2.y))
            }
            Value::Number(n) => Ok(Vec2::new(vec.x - n, vec.y - n)),
            Value::Integer(n) => Ok(Vec2::new(vec.x - n as f64, vec.y - n as f64)),
            _ => Err(mlua::Error::BadArgument {
                to: Some(String::from("Vec2 | number")),
                pos: 2,
                name: Some(String::from("val")),
                cause: Arc::new(mlua::Error::UserDataTypeMismatch),
            }),
        }
    }

    fn lua_mul(self, val: Value) -> mlua::Result<Self> {
        let vec = self.0;
        match val {
            Value::UserData(ud) if ud.is::<Self>() => {
                let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                Ok(Vec2::new(vec.x * vec2.x, vec.y * vec2.y))
            }
            Value::Number(n) => Ok(Vec2::new(vec.x * n, vec.y * n)),
            Value::Integer(n) => Ok(Vec2::new(vec.x * n as f64, vec.y * n as f64)),
            _ => Err(mlua::Error::BadArgument {
                to: Some(String::from("Vec2 | number")),
                pos: 2,
                name: Some(String::from("val")),
                cause: Arc::new(mlua::Error::UserDataTypeMismatch),
            }),
        }
    }

    fn lua_div(self, val: Value) -> mlua::Result<Self> {
        let vec = self.0;
        match val {
            Value::UserData(ud) if ud.is::<Self>() => {
                let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                Ok(Vec2::new(vec.x / vec2.x, vec.y / vec2.y))
            }
            Value::Number(n) => Ok(Vec2::new(vec.x / n, vec.y / n)),
            Value::Integer(n) => Ok(Vec2::new(vec.x / n as f64, vec.y / n as f64)),
            _ => Err(mlua::Error::BadArgument {
                to: Some(String::from("Vec2 | number")),
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
        methods.add_function("new", |_, (x, y): (f64, f64)| -> mlua::Result<Self> {
            Ok(Vec2::new(x, y))
        });

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
        methods.add_function("newP", |_, (m, a): (f64, f64)| -> mlua::Result<Self> {
            Ok(Vec2::new(m * a.cos(), m * a.sin()))
        });

        /// @brief Creates a clone of a vector.
        ///
        ///    @luatparam Vec2 v Vector to clone.
        ///    @luatreturn Vec2 A clone of v.
        /// @luafunc clone
        methods.add_method("clone", |_, vec: &Self, ()| -> mlua::Result<Self> {
            Ok(vec.clone())
        });

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
        methods.add_meta_function(MetaMethod::Sub, |_, (vec, val): (Self, Value)| {
            vec.lua_sub(val)
        });
        methods.add_method("sub", |_, vec, val: Value| vec.lua_sub(val));

        /// @brief Multiplies a vector by a number.
        ///
        /// @usage my_vec = my_vec * 3
        /// @usage my_vec:mul( 3 )
        ///
        ///    @luatparam Vec2 v Vector to multiply.
        ///    @luatparam number mod Amount to multiply by.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc mul
        methods.add_meta_function(MetaMethod::Mul, |_, (vec, val): (Self, Value)| {
            vec.lua_mul(val)
        });
        methods.add_method("mul", |_, vec, val: Value| vec.lua_mul(val));

        /// @brief Divides a vector by a number.
        ///
        /// @usage my_vec = my_vec / 3
        /// @usage my_vec:div(3)
        ///
        ///    @luatparam Vec2 v Vector to divide.
        ///    @luatparam number mod Amount to divide by.
        ///    @luatreturn Vec2 The result of the vector operation.
        /// @luafunc div
        methods.add_meta_function(MetaMethod::Div, |_, (vec, val): (Self, Value)| {
            vec.lua_div(val)
        });
        methods.add_method("div", |_, vec, val: Value| vec.lua_div(val));

        methods.add_meta_function(MetaMethod::Unm, |_, vec: Self| {
            Ok(Vec2::new(-vec.0.x, -vec.0.y))
        });

        /// @brief Dot product of two vectors.
        ///
        ///    @luatparam Vec2 a First vector.
        ///    @luatparam Vec2 b Second vector.
        ///    @luatreturn number The dot product.
        /// @luafunc dot
        methods.add_method("dot", |_, vec: &Self, val: Self| -> mlua::Result<f64> {
            Ok(vec.0.dot(&val.0))
        });

        /// @brief Cross product of two vectors.
        ///
        ///    @luatparam Vec2 a First vector.
        ///    @luatparam Vec2 b Second vector.
        ///    @luatreturn number The cross product.
        /// @luafunc cross
        methods.add_method("cross", |_, vec: &Self, val: Self| -> mlua::Result<Self> {
            Ok(Vec2(vec.0.cross(&val.0)))
        });

        /// @brief Gets the cartesian positions of the vector.
        ///
        /// @usage x,y = my_vec:get()
        ///
        ///    @luatparam Vec2 v Vector to get position of.
        ///    @luatreturn number X position of the vector.
        ///    @luatreturn number Y position of the vector.
        /// @luafunc get
        methods.add_method("get", |_, vec: &Self, ()| -> mlua::Result<(f64, f64)> {
            Ok((vec.0.x, vec.0.y))
        });

        /// @brief Gets polar coordinates of a vector.
        ///
        /// The angle is in radians.
        ///
        /// @usage modulus, angle = my_vec:polar()
        ///
        ///    @luatparam Vec2 v Vector to get polar coordinates of.
        ///    @luatreturn number The modulus of the vector.
        ///    @luatreturn number The angle of the vector.
        /// @luafunc polar
        methods.add_method("polar", |_, vec: &Self, ()| -> mlua::Result<(f64, f64)> {
            let (x, y) = (vec.0.x, vec.0.y);
            Ok((x.hypot(y), y.atan2(x)))
        });

        /// @brief Sets the vector by cartesian coordinates.
        ///
        /// @usage my_vec:set(5, 3) -- my_vec is now (5,3)
        ///
        ///    @luatparam Vec2 v Vector to set coordinates of.
        ///    @luatparam number x X coordinate to set.
        ///    @luatparam number y Y coordinate to set.
        /// @luafunc set
        methods.add_method_mut("set", |_, vec: &mut Self, (x, y): (f64, f64)| {
            vec.0.x = x;
            vec.0.y = y;
            Ok(())
        });

        /// @brief Sets the vector by polar coordinates.
        ///
        /// @usage my_vec:setP( 1, 90 ) -- my_vec is now (0,1)
        ///
        ///    @luatparam Vec2 v Vector to set coordinates of.
        ///    @luatparam number m Modulus to set.
        ///    @luatparam number a Angle to set, in radians.
        /// @luafunc setP
        methods.add_method_mut("setP", |_, vec: &mut Self, (m, a): (f64, f64)| {
            vec.0.x = m * a.cos();
            vec.0.y = m * a.sin();
            Ok(())
        });

        /// @brief Gets the distance from the Vec2.
        ///
        /// @usage my_vec:dist() -- Gets length of the vector (distance from origin).
        /// @usage my_vec:dist( your_vec ) -- Gets distance from both vectors (your_vec -
        /// my_vec).
        ///
        ///    @luatparam Vec2 v Vector to act as origin.
        ///    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get distance from, uses
        /// origin (0,0) if not set.
        ///    @luatreturn number The distance calculated.
        /// @luafunc dist
        methods.add_method("dist", |_, vec: &Self, val: Value| -> mlua::Result<f64> {
            match val {
                Value::UserData(ud) if ud.is::<Self>() => {
                    let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                    Ok((vec.0.x - vec2.x).hypot(vec.0.y - vec2.y))
                }
                Value::Nil => Ok(vec.0.x.hypot(vec.0.y)),
                _ => Err(mlua::Error::BadArgument {
                    to: Some(String::from("Vec2 | nil")),
                    pos: 2,
                    name: Some(String::from("v2")),
                    cause: Arc::new(mlua::Error::UserDataTypeMismatch),
                }),
            }
        });

        /// @brief Gets the squared distance from the Vec2 (saves a sqrt())
        ///
        /// @usage my_vec:dist2() -- Gets squared length of the vector (distance squared
        /// from origin).
        /// @usage my_vec:dist2( your_vec ) -- Gets squared distance from both vectors
        /// (your_vec - my_vec)^2.
        ///
        ///    @luatparam Vec2 v Vector to act as origin.
        ///    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get squared distance from,
        /// uses origin (0,0) if not set.
        ///    @luatreturn number The distance calculated.
        /// @luafunc dist2
        methods.add_method("dist2", |_, vec: &Self, val: Value| -> mlua::Result<f64> {
            match val {
                Value::UserData(ud) if ud.is::<Self>() => {
                    let vec2: Vector2<f64> = ud.borrow::<Self>()?.0.into();
                    Ok((vec.0.x - vec2.x).powf(2.0) + (vec.0.y - vec2.y).powf(2.0))
                }
                Value::Nil => Ok(vec.0.x.powf(2.0) + vec.0.y.powf(2.0)),
                _ => Err(mlua::Error::BadArgument {
                    to: Some(String::from("Vec2 | nil")),
                    pos: 2,
                    name: Some(String::from("v2")),
                    cause: Arc::new(mlua::Error::UserDataTypeMismatch),
                }),
            }
        });

        /// @brief Gets the modulus of the vector.
        ///    @luatparam Vec2 v Vector to get modulus of.
        ///    @luatreturn number The modulus of the vector.
        /// @luafunc mod
        methods.add_method("mod", |_, vec: &Self, ()| -> mlua::Result<f64> {
            Ok(vec.0.x.hypot(vec.0.y))
        });

        /// @brief Gets the angle of the vector.
        ///    @luatparam Vec2 v Vector to get angle of.
        ///    @luatreturn number The angle of the vector.
        /// @luafunc angle
        methods.add_method("angle", |_, vec: &Self, ()| -> mlua::Result<f64> {
            Ok(vec.0.y.atan2(vec.0.x))
        });

        /// @brief Normalizes a vector.
        ///    @luatparam Vec2 v Vector to normalize.
        ///    @luatparam[opt=1] number n Length to normalize the vector to.
        ///    @luatreturn Vec2 Normalized vector.
        /// @luafunc normalize
        methods.add_method_mut(
            "normalize",
            |_, vec: &mut Self, val: mlua::Value| -> mlua::Result<Self> {
                let n = match val {
                    Value::Number(n) => n,
                    Value::Nil => 1.,
                    _ => {
                        return Err(mlua::Error::BadArgument {
                            to: Some(String::from("number | nil")),
                            pos: 2,
                            name: Some(String::from("n")),
                            cause: Arc::new(mlua::Error::UserDataTypeMismatch),
                        })
                    }
                };
                let m = n / (vec.0.x.hypot(vec.0.y)).max(1e-6);
                vec.0.x *= m;
                vec.0.y *= m;
                Ok(Vec2(vec.0))
            },
        );

        // TODO collision stuff
    }
}

#[allow(dead_code)]
pub fn open_vec2(lua: &mlua::Lua) -> anyhow::Result<()> {
    let globals = lua.globals();
    globals.set("foo", Vec2::new(0.0, 0.0))?;
    Ok(())
}

/*
#[unsafe(no_mangle)]
pub unsafe extern "C" fn nlua_loadVector( env: naevc::nlua_env ) {
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn lua_tovector( L: *mut mlua::lua_State, ind: c_int ) -> &'static mut Vec2 {
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn luaL_checkvector( L: *mut mlua::lua_State, ind: c_int ) -> &'static mut Vec2 {
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn lua_pushvector( L: *mut mlua::lua_State, ind: c_int ) -> &'static mut Vec2 {
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn lua_isvector( L: *mut mlua::lua_State, ind: c_int ) -> c_int {
}
*/
