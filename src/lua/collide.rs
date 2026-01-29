use anyhow::Result;
use mlua::{UserData, UserDataMethods};
use physics::vec2::Vec2;

#[derive(Copy, Clone)]
pub struct Collide;

/*@
 * @luamod collide
 */
impl UserData for Collide {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Sees if two line segments collide.
         *
         *    @luatparam Vec2 s1 Start point of the first segment.
         *    @luatparam Vec2 e1 End point of the first segment.
         *    @luatparam Vec2 s2 Start point of the second segment.
         *    @luatparam Vec2 e2 End point of the second segment.
         *    @luatreturn Vec2 Collision point if they collide.
         * @luafunc line_line
         */
        methods.add_function(
            "line_line",
            |_, (s1, e1, s2, e2): (Vec2, Vec2, Vec2, Vec2)| -> mlua::Result<Option<Vec2>> {
                let hit = collide::line_line(s1.into(), e1.into(), s2.into(), e2.into());
                if let Some(h) = hit.first() {
                    Ok(Some((*h).into()))
                } else {
                    Ok(None)
                }
            },
        );

        /*@
         * @brief Computes the intersection of a line segment and a circle.
         *
         *    @luatparam Vector centre Centre of the circle.
         *    @luatparam number radius Radius of the circle.
         *    @luatparam Vector p1 First point of the line segment.
         *    @luatparam Vector p2 Second point of the line segment.
         *    @luatreturn Vector|nil First point of collision or nil if no collision.
         *    @luatreturn Vector|nil Second point of collision or nil if single-point
         * collision.
         * @luafunc circle_line
         */
        methods.add_function(
            "circle_line",
            |_,
             (centre, radius, p1, p2): (Vec2, f64, Vec2, Vec2)|
             -> mlua::Result<(Option<Vec2>, Option<Vec2>)> {
                let hit = collide::line_circle(p1.into(), p2.into(), centre.into(), radius);
                if let Some(h1) = hit.first() {
                    if let Some(h2) = hit.get(1) {
                        Ok((Some((*h1).into()), Some((*h2).into())))
                    } else {
                        Ok((Some((*h1).into()), None))
                    }
                } else {
                    Ok((None, None))
                }
            },
        );
    }
}

pub fn open_collide(lua: &mlua::Lua) -> Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<Collide>()?)
}

#[test]
fn test_mlua_collide() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals
        .set("vec2", physics::vec2::open_vec2(&lua).unwrap())
        .unwrap();
    globals.set("collide", open_collide(&lua).unwrap()).unwrap();
    lua.load(include_str!("collide_test.lua"))
        .set_name("mlua Collide test")
        .exec()
        .unwrap();
}
