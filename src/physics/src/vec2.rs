use crate::collide;
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::{Vector2, Vector3};
use std::os::raw::c_void;

#[derive(
    Copy,
    Clone,
    derive_more::From,
    derive_more::Into,
    derive_more::Mul,
    derive_more::Add,
    derive_more::AddAssign,
    derive_more::MulAssign,
)]
pub struct Vec2(Vector2<f64>);

impl Vec2 {
    /// Creates a new Vec2
    pub const fn new(x: f64, y: f64) -> Self {
        Vec2(Vector2::new(x, y))
    }

    pub fn into_vector2(self) -> Vector2<f64> {
        self.0
    }

    pub fn into_vector3(self) -> Vector3<f64> {
        Vector3::new(self.0.x, self.0.y, 0.)
    }
}

impl FromLua for Vec2 {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            Value::Integer(num) => Ok(Self::new(num as f64, num as f64)),
            Value::Number(num) => Ok(Self::new(num, num)),
            Value::Table(tbl) => {
                let x: f64 = tbl.get(1)?;
                let y: f64 = tbl.get(2)?;
                Ok(Self::new(x, y))
            }
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to Vec2",
                val.type_name()
            ))),
        }
    }
}

/*
 * @brief Represents a 2D vector in Lua.
 *
 * This module allows you to manipulate 2D vectors. Usage is generally as
 * follows:
 *
 * @code
 * my_vec = vec2.new( 3, 2 ) -- my_vec is now (3,2)
 * my_vec:add( 5, 3 ) -- in-place addition, my_vec is now (8,5)
 * my_vec = my_vec * 3 -- my_vec is now (24,15)
 * your_vec = vec2.new( 5, 2 ) -- your_vec is now (5,2)
 * my_vec = my_vec - your_vec -- my_vec is now (19,13)
 * @endcode
 *
 * To call members of the metatable always use:
 * @code
 * vector:function( param )
 * @endcode
 *
 * @luamod vec2
 */
impl UserData for Vec2 {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        fields.add_field_method_get("x", |_, this| Ok(this.0.x));
        fields.add_field_method_get("y", |_, this| Ok(this.0.y));
    }
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Creates a new vector.
         *
         * @usage vec2.new( 5, 3 ) -- creates a vector at (5,3)
         * @usage vec2.new() -- creates a vector at (0,0)
         *
         *    @luatparam[opt=0] number x If set, the X value for the new vector.
         *    @luatparam[opt=x] number y If set, the Y value for the new vector.
         *    @luatreturn Vec2 The new vector.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_, (x, y): (Option<f64>, Option<f64>)| -> mlua::Result<Self> {
                let v = match x {
                    Some(x) => match y {
                        Some(y) => Vec2::new(x, y),
                        None => Vec2::new(x, x),
                    },
                    None => Vec2::new(0.0, 0.0),
                };
                Ok(v)
            },
        );

        /*
         * @brief Creates a new vector using polar coordinates.
         *
         * @usage vec2.newP( 1000, math.pi/2 ) -- creates a vector at (0,1000)
         * @usage vec2.newP() -- creates a vector at (0,0)
         *
         *    @luatparam[opt=0] number m If set, the modulus for the new vector.
         *    @luatparam[opt=0] number a If set, the angle for the new vector, in
         * radians.
         *    @luatreturn Vec2 The new vector.
         * @luafunc newP
         */
        methods.add_function("newP", |_, (m, a): (f64, f64)| -> mlua::Result<Self> {
            Ok(Vec2::new(m * a.cos(), m * a.sin()))
        });

        /*
         * @brief Creates a clone of a vector.
         *
         *    @luatparam Vec2 v Vector to clone.
         *    @luatreturn Vec2 A clone of v.
         * @luafunc clone
         */
        methods.add_method("clone", |_, vec, ()| -> mlua::Result<Self> { Ok(*vec) });

        /*
         * @brief Converts a vector to a string.
         *
         *    @luatparam Vector v Vector to convert to as string.
         *    @luatreturn string String version of v.
         * @luafunc __tostring
         */
        methods.add_meta_function(MetaMethod::ToString, |_, vec: Self| {
            Ok(format!("vec2( {}, {} )", vec.0.x, vec.0.y))
        });

        /*
         * @brief Adds two vectors or a vector and some cartesian coordinates.
         *
         * If x is a vector it adds both vectors, otherwise it adds cartesian
         * coordinates to the vector.
         *
         * @usage my_vec = my_vec + your_vec
         * @usage my_vec:add( your_vec )
         * @usage my_vec:add( 5, 3 )
         *
         *    @luatparam Vector v Vector getting stuff added to.
         *    @luatparam number|Vec2 x X coordinate or vector to add to.
         *    @luatparam number|nil y Y coordinate or nil to add to.
         *    @luatreturn Vec2 The result of the vector operation.
         * @luafunc add
         */
        methods.add_meta_function(MetaMethod::Add, |_, (vec, val): (Self, Self)| {
            Ok(Vec2(vec.0 + val.0))
        });
        methods.add_method_mut("add", |_, vec, val: Self| {
            vec.0 += val.0;
            Ok(*vec)
        });

        /*
         * @brief Subtracts two vectors or a vector and some cartesian coordinates.
         *
         * If x is a vector it subtracts both vectors, otherwise it subtracts cartesian
         * coordinates to the vector.
         *
         * @usage my_vec = my_vec - your_vec
         * @usage my_vec:sub( your_vec )
         * @usage my_vec:sub( 5, 3 )
         *
         *    @luatparam Vec2 v Vector getting stuff subtracted from.
         *    @luatparam number|Vec2 x X coordinate or vector to subtract.
         *    @luatparam number|nil y Y coordinate or nil to subtract.
         *    @luatreturn Vec2 The result of the vector operation.
         * @luafunc sub
         */
        methods.add_meta_function(MetaMethod::Sub, |_, (vec, val): (Self, Self)| {
            Ok(Vec2(vec.0 - val.0))
        });
        methods.add_method_mut("sub", |_, vec, val: Self| {
            vec.0 -= val.0;
            Ok(*vec)
        });

        /*
         * @brief Multiplies a vector by a number.
         *
         * @usage my_vec = my_vec * 3
         * @usage my_vec:mul( 3 )
         *
         *    @luatparam Vec2 v Vector to multiply.
         *    @luatparam number mod Amount to multiply by.
         *    @luatreturn Vec2 The result of the vector operation.
         * @luafunc mul
         */
        methods.add_meta_function(MetaMethod::Mul, |_, (vec, val): (Self, Self)| {
            Ok(Vec2(vec.0.component_mul(&val.0)))
        });
        methods.add_method_mut("mul", |_, vec, val: Self| {
            vec.0.component_mul_assign(&val.0);
            Ok(*vec)
        });

        /*
         * @brief Divides a vector by a number.
         *
         * @usage my_vec = my_vec / 3
         * @usage my_vec:div(3)
         *
         *    @luatparam Vec2 v Vector to divide.
         *    @luatparam number mod Amount to divide by.
         *    @luatreturn Vec2 The result of the vector operation.
         * @luafunc div
         */
        methods.add_meta_function(MetaMethod::Div, |_, (vec, val): (Self, Self)| {
            Ok(Vec2(vec.0.component_div(&val.0)))
        });
        methods.add_method_mut("div", |_, vec, val: Self| {
            vec.0.component_div_assign(&val.0);
            Ok(*vec)
        });

        methods.add_meta_function(MetaMethod::Unm, |_, vec: Self| {
            Ok(Vec2::new(-vec.0.x, -vec.0.y))
        });

        /*
         * @brief Dot product of two vectors.
         *
         *    @luatparam Vec2 a First vector.
         *    @luatparam Vec2 b Second vector.
         *    @luatreturn number The dot product.
         * @luafunc dot
         */
        methods.add_method("dot", |_, vec, val: Self| -> mlua::Result<f64> {
            Ok(vec.0.dot(&val.0))
        });

        /*
         * @brief Cross product of two vectors.
         *
         *    @luatparam Vec2 a First vector.
         *    @luatparam Vec2 b Second vector.
         *    @luatreturn number The cross product.
         * @luafunc cross
         */
        methods.add_method("cross", |_, vec, val: Self| -> mlua::Result<Self> {
            Ok(Vec2(vec.0.cross(&val.0)))
        });

        /*
         * @brief Gets the cartesian positions of the vector.
         *
         * @usage x,y = my_vec:get()
         *
         *    @luatparam Vec2 v Vector to get position of.
         *    @luatreturn number X position of the vector.
         *    @luatreturn number Y position of the vector.
         * @luafunc get
         */
        methods.add_method("get", |_, vec, ()| -> mlua::Result<(f64, f64)> {
            Ok((vec.0.x, vec.0.y))
        });

        /*
         * @brief Gets polar coordinates of a vector.
         *
         * The angle is in radians.
         *
         * @usage modulus, angle = my_vec:polar()
         *
         *    @luatparam Vec2 v Vector to get polar coordinates of.
         *    @luatreturn number The modulus of the vector.
         *    @luatreturn number The angle of the vector.
         * @luafunc polar
         */
        methods.add_method("polar", |_, vec, ()| -> mlua::Result<(f64, f64)> {
            let (x, y) = (vec.0.x, vec.0.y);
            Ok((x.hypot(y), y.atan2(x)))
        });

        /*
         * @brief Sets the vector by cartesian coordinates.
         *
         * @usage my_vec:set(5, 3) -- my_vec is now (5,3)
         *
         *    @luatparam Vec2 v Vector to set coordinates of.
         *    @luatparam number x X coordinate to set.
         *    @luatparam number y Y coordinate to set.
         * @luafunc set
         */
        methods.add_method_mut("set", |_, vec: &mut Self, (x, y): (f64, f64)| {
            vec.0.x = x;
            vec.0.y = y;
            Ok(())
        });

        /*
         * @brief Sets the vector by polar coordinates.
         *
         * @usage my_vec:setP( 1, 90 ) -- my_vec is now (0,1)
         *
         *    @luatparam Vec2 v Vector to set coordinates of.
         *    @luatparam number m Modulus to set.
         *    @luatparam number a Angle to set, in radians.
         * @luafunc setP
         */
        methods.add_method_mut("setP", |_, vec: &mut Self, (m, a): (f64, f64)| {
            vec.0.x = m * a.cos();
            vec.0.y = m * a.sin();
            Ok(())
        });

        /*
         * @brief Gets the distance from the Vec2.
         *
         * @usage my_vec:dist() -- Gets length of the vector (distance from origin).
         * @usage my_vec:dist( your_vec ) -- Gets distance from both vectors (your_vec -
         * my_vec).
         *
         *    @luatparam Vec2 v Vector to act as origin.
         *    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get distance from, uses
         * origin (0,0) if not set.
         *    @luatreturn number The distance calculated.
         * @luafunc dist
         */
        methods.add_method("dist", |_, vec, val: Option<Vec2>| -> mlua::Result<f64> {
            let d = match val {
                Some(vec2) => (vec.0.x - vec2.0.x).hypot(vec.0.y - vec2.0.y),
                None => vec.0.x.hypot(vec.0.y),
            };
            Ok(d)
        });

        /*
         * @brief Gets the squared distance from the Vec2 (saves a sqrt())
         *
         * @usage my_vec:dist2() -- Gets squared length of the vector (distance squared
         * from origin).
         * @usage my_vec:dist2( your_vec ) -- Gets squared distance from both vectors
         * (your_vec - my_vec)^2.
         *
         *    @luatparam Vec2 v Vector to act as origin.
         *    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get squared distance from,
         * uses origin (0,0) if not set.
         *    @luatreturn number The distance calculated.
         * @luafunc dist2
         */
        methods.add_method("dist2", |_, vec, val: Option<Vec2>| -> mlua::Result<f64> {
            let d = match val {
                Some(vec2) => (vec.0.x - vec2.0.x).powf(2.0) + (vec.0.y - vec2.0.y).powf(2.0),
                None => vec.0.x.powf(2.0) + vec.0.y.powf(2.0),
            };
            Ok(d)
        });

        /*
         * @brief Gets the modulus of the vector.
         *    @luatparam Vec2 v Vector to get modulus of.
         *    @luatreturn number The modulus of the vector.
         * @luafunc mod
         */
        methods.add_method("mod", |_, vec, ()| -> mlua::Result<f64> {
            Ok(vec.0.x.hypot(vec.0.y))
        });

        /*
         * @brief Gets the angle of the vector.
         *    @luatparam Vec2 v Vector to get angle of.
         *    @luatreturn number The angle of the vector.
         * @luafunc angle
         */
        methods.add_method("angle", |_, vec, ()| -> mlua::Result<f64> {
            Ok(vec.0.y.atan2(vec.0.x))
        });

        /*
         * @brief Normalizes a vector.
         *    @luatparam Vec2 v Vector to normalize.
         *    @luatparam[opt=1] number n Length to normalize the vector to.
         *    @luatreturn Vec2 Normalized vector.
         * @luafunc normalize
         */
        methods.add_method_mut(
            "normalize",
            |_, vec, n: Option<f64>| -> mlua::Result<Self> {
                let n = n.unwrap_or(1.);
                let m = n / (vec.0.x.hypot(vec.0.y)).max(1e-6);
                vec.0.x *= m;
                vec.0.y *= m;
                Ok(Vec2(vec.0))
            },
        );

        /*
         * @brief Sees if two line segments collide.
         *
         *    @luatparam Vec2 s1 Start point of the first segment.
         *    @luatparam Vec2 e1 End point of the first segment.
         *    @luatparam Vec2 s2 Start point of the second segment.
         *    @luatparam Vec2 e2 End point of the second segment.
         *    @luatreturn Vec2 Collision point if they collide.
         * @luafunc collideLineLine
         */
        methods.add_method(
            "collideLineLine",
            |_, s1, (e1, s2, e2): (Self, Self, Self)| -> mlua::Result<Option<Vec2>> {
                match collide::line_line((*s1).into(), e1.into(), s2.into(), e2.into()) {
                    Some(collide::Collision::Single(crash)) => Ok(Some(crash.into())),
                    _ => Ok(None),
                }
            },
        );

        /*
         * @brief Computes the intersection of a line segment and a circle.
         *
         *    @luatparam Vector center Center of the circle.
         *    @luatparam number radius Radius of the circle.
         *    @luatparam Vector p1 First point of the line segment.
         *    @luatparam Vector p2 Second point of the line segment.
         *    @luatreturn Vector|nil First point of collision or nil if no collision.
         *    @luatreturn Vector|nil Second point of collision or nil if single-point
         * collision.
         * @luafunc collideCircleLine
         */
        methods.add_method(
            "collideCircleLine",
            |_,
             center,
             (radius, p1, p2): (f64, Self, Self)|
             -> mlua::Result<(Option<Vec2>, Option<Vec2>)> {
                match collide::line_circle(p1.into(), p2.into(), (*center).into(), radius) {
                    Some(collide::Collision::Single(c)) => Ok((Some(c.into()), None)),
                    Some(collide::Collision::Double(c1, c2)) => {
                        Ok((Some(c1.into()), Some(c2.into())))
                    }
                    _ => Ok((None, None)),
                }
            },
        );
    }
}

pub fn open_vec2(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    let proxy = lua.create_proxy::<Vec2>()?;

    // Only add stuff as necessary
    if let mlua::Value::Nil = lua.named_registry_value("push_vector")? {
        let push_vector = lua.create_function(|lua, (x, y): (f64, f64)| {
            let vec = Vec2::new(x, y);
            lua.create_any_userdata(vec)
        })?;
        lua.set_named_registry_value("push_vector", push_vector)?;

        let get_vector = lua.create_function(|_, mut ud: mlua::UserDataRefMut<Vec2>| {
            let vec: *mut Vec2 = &mut *ud;
            Ok(Value::LightUserData(mlua::LightUserData(
                vec as *mut c_void,
            )))
        })?;
        lua.set_named_registry_value("get_vector", get_vector)?;
    }

    Ok(proxy)
}

use mlua::ffi;
use std::os::raw::{c_char, c_int};

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_checkvector(L: *mut mlua::lua_State, idx: c_int) -> *mut Vec2 {
    unsafe {
        let vec = lua_tovector(L, idx);
        if vec.is_null() {
            ffi::luaL_typerror(L, idx, c"vec2".as_ptr() as *const c_char);
        }
        vec
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_isvector(L: *mut mlua::lua_State, idx: c_int) -> c_int {
    !lua_tovector(L, idx).is_null() as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_pushvector(L: *mut mlua::lua_State, vec: naevc::vec2) {
    unsafe {
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_vector".as_ptr());
        ffi::lua_pushnumber(L, vec.x);
        ffi::lua_pushnumber(L, vec.y);
        ffi::lua_call(L, 2, 1);
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_tovector(L: *mut mlua::lua_State, idx: c_int) -> *mut Vec2 {
    unsafe {
        let idx = ffi::lua_absindex(L, idx);
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_vector".as_ptr());
        ffi::lua_pushvalue(L, idx);
        let vec = match ffi::lua_pcall(L, 1, 1, 0) {
            ffi::LUA_OK => ffi::lua_touserdata(L, -1) as *mut Vec2,
            _ => std::ptr::null_mut(),
        };
        ffi::lua_pop(L, 1);
        vec
    }
}

#[test]
fn test_mlua_vec2() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals.set("vec2", open_vec2(&lua).unwrap()).unwrap();
    lua.load(include_str!("vec2_test.lua"))
        .set_name("mlua Vec2 test")
        .exec()
        .unwrap();
}
