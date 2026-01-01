use mlua::{Either, FromLua, Lua, MetaMethod, UserData, UserDataMethods, UserDataRef, Value};
use nalgebra::{Matrix3, Point2, Vector2};
use std::os::raw::c_void;

#[derive(Copy, Clone, derive_more::From, derive_more::Into)]
pub struct Transform2(Matrix3<f32>);
impl Default for Transform2 {
    fn default() -> Self {
        Self::new()
    }
}
impl Transform2 {
    #[rustfmt::skip]
    pub const fn new() -> Self {
        Transform2(Matrix3::new(
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0, ))
    }
    #[rustfmt::skip]
    pub const fn new_from(data: &[f32; 6]) -> Self {
        Transform2(Matrix3::new(
            data[0], data[1], data[2],
            data[3], data[4], data[5],
              0.0,     0.0,     1.0, ))
    }
}

impl FromLua for Transform2 {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            Value::Table(tbl) => {
                let x: f32 = tbl.get(1)?;
                let y: f32 = tbl.get(2)?;
                let z: f32 = tbl.get(3)?;
                let i: f32 = tbl.get(4)?;
                let j: f32 = tbl.get(5)?;
                let k: f32 = tbl.get(6)?;
                Ok(Self::new_from(&[x, y, z, i, j, k]))
            }
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to Transform2",
                val.type_name()
            ))),
        }
    }
}

/*@
 * @brief Represents a 2D transformation matrix in Lua.
 *
 * @luamod transform
 */
impl UserData for Transform2 {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Creates a new identity transform.Gets a transform.
         *
         *    @luatparam Transform|Table Either a transform to copy or a 3x2 table with elements of the
         *    transform matrix.
         *    @luatreturn Transform A new transform corresponding to an identity matrix.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_, data: Option<Either<UserDataRef<Self>, mlua::Table>>| -> mlua::Result<Self> {
                match data {
                    Some(Either::Left(data)) => Ok(*data),
                    Some(Either::Right(data)) => {
                        let r1: mlua::Table = data.get(1)?;
                        let x: f32 = r1.get(1)?;
                        let y: f32 = r1.get(2)?;
                        let z: f32 = r1.get(3)?;
                        let r2: mlua::Table = data.get(2)?;
                        let i: f32 = r2.get(1)?;
                        let j: f32 = r2.get(2)?;
                        let k: f32 = r2.get(3)?;
                        Ok(Transform2::new_from(&[x, y, z, i, j, k]))
                    }
                    None => Ok(Transform2::new()),
                }
            },
        );
        /*@
         * @brief Gets a string representing the transform.
         *
         *    @luatparam Transform t Transform to get string of.
         *    @luatreturn string String corresponding to the transform.
         * @luafunc __tostring
         */
        methods.add_meta_function(MetaMethod::ToString, |_, this: Self| {
            Ok(format!(
                "transform2(\n   {}, {}, {},\n   {}, {}, {}\n)",
                this.0[(0, 0)],
                this.0[(0, 1)],
                this.0[(0, 2)],
                this.0[(1, 0)],
                this.0[(1, 1)],
                this.0[(1, 2)],
            ))
        });
        /*@
         * @brief Multiplies two transforms (A*B).
         *
         *    @luatparam Transform A First element to multiply.
         *    @luatparam Transform B Second element to multiply.
         *    @luatreturn Transform Result of multiplication.
         * @luafunc __mul
         */
        methods.add_meta_function(MetaMethod::Mul, |_, (this, val): (Self, Self)| {
            Ok(Transform2(this.0 * val.0))
        });
        methods.add_method_mut("mul", |_, this, val: Self| {
            this.0 *= val.0;
            Ok(*this)
        });
        /*@
         * @brief Gets all the values of the transform.
         *
         * Note, this returns in column-major.
         *
         *    @luatparam Transform T Transform to get parameters of.
         *    @luatreturn table 2D table containing all the values of the transform.
         * @luafunc get
         */
        methods.add_method("get", |lua, this, ()| -> mlua::Result<mlua::Table> {
            let t = lua.create_table()?;
            for r in this.0.row_iter() {
                let tr = lua.create_table()?;
                for v in r {
                    let _ = tr.push(*v);
                }
                let _ = t.push(tr);
            }
            Ok(t)
        });
        /*@
         * @brief Sets an element of a transform.
         *
         *    @luatparam Transform T Transform to set element of.
         *    @luatparam number i Column to set.
         *    @luatparam number j Row to set.
         *    @luatparam number v Value to set to.
         * @luafunc set
         */
        methods.add_method_mut(
            "set",
            |_, this, (i, j, v): (usize, usize, f32)| -> mlua::Result<()> {
                match this.0.get_mut((i, j)) {
                    Some(data) => {
                        *data = v;
                        Ok(())
                    }
                    None => Err(mlua::Error::RuntimeError(format!(
                        "index ({}, {}) is out of bounds",
                        i, j
                    ))),
                }
            },
        );
        /*@
         * @brief Applies scaling to a transform.
         *
         *    @luatparam Transform T Transform to apply scaling to.
         *    @luatparam number x X-axis scaling.
         *    @luatparam number y Y-axis scaling.
         *    @luatreturn Transform A new transformation.
         * @luafunc scale
         */
        methods.add_method(
            "scale",
            |_, this, (x, y): (f32, f32)| -> mlua::Result<Self> {
                Ok(Self(this.0.prepend_nonuniform_scaling(&Vector2::new(x, y))))
            },
        );
        /*@
         * @brief Applies translation to a transform.
         *
         *    @luatparam Transform T Transform to apply translation to.
         *    @luatparam number x X-axis translation.
         *    @luatparam number y Y-axis translation.
         *    @luatreturn Transform A new transformation.
         * @luafunc translate
         */
        methods.add_method(
            "translate",
            |_, this, (x, y): (f32, f32)| -> mlua::Result<Self> {
                Ok(Self(this.0.prepend_translation(&Vector2::new(x, y))))
            },
        );
        /*@
         * @brief Applies a 2D rotation (along Z-axis) to a transform.
         *
         *    @luatparam Transform T Transform to apply rotation to.
         *    @luatparam number angle Angle to rotate (radians).
         *    @luatreturn Transform A new transformation.
         * @luafunc rotate2d
         */
        methods.add_method("rotate2d", |_, this, angle: f32| -> mlua::Result<Self> {
            let rot = nalgebra::Rotation2::new(angle);
            Ok(Self(this.0 * rot.to_homogeneous()))
        });
        /*@
         * @brief Creates an orthogonal matrix.
         *
         * As it is 2D, it assumes near = -1 and far = +1.
         *
         *    @luatparam number left Left value.
         *    @luatparam number right Right value.
         *    @luatparam number bottom Bottom value.
         *    @luatparam number top Top value.
         *    @luatreturn Transform A new transformation.
         * @luafunc ortho
         */
        methods.add_function(
            "ortho",
            |_, (left, right, bottom, top): (f32, f32, f32, f32)| -> mlua::Result<Self> {
                Ok(Self(Matrix3::new(
                    2.0 / (right - left),
                    0.0,
                    -(right + left) / (right - left),
                    0.0,
                    2.0 / (top - bottom),
                    -(top + bottom) / (top - bottom),
                    0.0,
                    0.0,
                    1.0,
                )))
            },
        );
        /*@
         * @brief Applies a transformation to a point.
         *
         *    @luatparam Transform T Transform to apply.
         *    @luatparam number x Point X-coordinate.
         *    @luatparam number y Point Y-coordinate.
         *    @luatreturn number New X coordinate.
         *    @luatreturn number New Y coordinate.
         * @luafunc applyPoint
         */
        methods.add_method(
            "applyPoint",
            |_, this, (x, y): (f32, f32)| -> mlua::Result<(f32, f32)> {
                let vec = this.0.transform_point(&Point2::new(x, y));
                Ok((vec.x, vec.y))
            },
        );
        /*@
         * @brief Applies a transformation to a dimension.
         *
         * @note This is similar to `Transform.applyPoint`, except the translation is
         * not applied.
         *
         *    @luatparam Transform T Transform to apply.
         *    @luatparam number x Dimension X-coordinate.
         *    @luatparam number y Dimension Y-coordinate.
         *    @luatreturn number New X coordinate.
         *    @luatreturn number New Y coordinate.
         * @luafunc applyDim
         */
        methods.add_method(
            "applyDim",
            |_, this, (x, y): (f32, f32)| -> mlua::Result<(f32, f32)> {
                let vec = this.0.transform_vector(&Vector2::new(x, y));
                Ok((vec.x, vec.y))
            },
        );
    }
}

pub fn open_transform2(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    let proxy = lua.create_proxy::<Transform2>()?;

    // Only add stuff as necessary
    if let mlua::Value::Nil = lua.named_registry_value("push_transform")? {
        let push_transform =
            lua.create_function(|lua, (x, y, z, i, j, k): (f32, f32, f32, f32, f32, f32)| {
                let transform = Transform2::new_from(&[x, y, z, i, j, k]);
                lua.create_any_userdata(transform)
            })?;
        lua.set_named_registry_value("push_transform", push_transform)?;

        let get_transform =
            lua.create_function(|_, mut ud: mlua::UserDataRefMut<Transform2>| {
                let transform: *mut Transform2 = &mut *ud;
                Ok(Value::LightUserData(mlua::LightUserData(
                    transform as *mut c_void,
                )))
            })?;
        lua.set_named_registry_value("get_transform", get_transform)?;
    }

    Ok(proxy)
}

use mlua::ffi;
use std::os::raw::{c_char, c_int};
static_assertions::assert_eq_size!(Transform2, naevc::mat3);

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn luaL_checktransform(L: *mut mlua::lua_State, idx: c_int) -> *mut Transform2 {
    unsafe {
        let transform = lua_totransform(L, idx);
        if transform.is_null() {
            ffi::luaL_typerror(L, idx, c"transform".as_ptr() as *const c_char);
        }
        transform
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_istransform(L: *mut mlua::lua_State, idx: c_int) -> c_int {
    !lua_totransform(L, idx).is_null() as c_int
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_pushtransform(L: *mut mlua::lua_State, m: naevc::mat4) {
    unsafe {
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"push_transform".as_ptr());
        for i in 0..6 {
            ffi::lua_pushnumber(L, m.__bindgen_anon_1.ptr[i] as f64);
        }
        ffi::lua_call(L, 6, 1);
    }
}

#[allow(non_snake_case)]
#[unsafe(no_mangle)]
pub extern "C" fn lua_totransform(L: *mut mlua::lua_State, idx: c_int) -> *mut Transform2 {
    unsafe {
        let idx = ffi::lua_absindex(L, idx);
        ffi::lua_getfield(L, ffi::LUA_REGISTRYINDEX, c"get_transform".as_ptr());
        ffi::lua_pushvalue(L, idx);
        let transform = match ffi::lua_pcall(L, 1, 1, 0) {
            ffi::LUA_OK => ffi::lua_touserdata(L, -1) as *mut Transform2,
            _ => std::ptr::null_mut(),
        };
        ffi::lua_pop(L, 1);
        transform
    }
}

#[test]
fn test_mlua_transform2() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals
        .set("transform", open_transform2(&lua).unwrap())
        .unwrap();
    lua.load(include_str!("transform2_test.lua"))
        .set_name("mlua Transform2 test")
        .exec()
        .unwrap();
}
