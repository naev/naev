use anyhow::Result;
use nalgebra::Vector2;
use physics::angle_diff;
use physics::vec2::Vec2;
use std::os::raw::{c_double, c_int, c_uint};
use std::sync::atomic::Ordering;
use std::sync::{LazyLock, RwLock};
use utils::atomicfloat::AtomicF64;

// Sound is currently in "screen" coordinates, and doesn't react to ship turning
// Would probably have to be relative to heading for accessibility support (when enabled)
const SOUND_DIR: f64 = std::f64::consts::FRAC_PI_2;

// Converts y coordinates based on viewing angles
static GAME_TO_SCREEN: AtomicF64 = AtomicF64::new(1.);
static SCREEN_TO_GAME: AtomicF64 = AtomicF64::new(1.);

/// Represents tho global camera
#[derive(Default, Clone)]
pub struct Camera {
    /// Current location
    pos: Vector2<f64>,
    /// Fixed camera offset
    offset: Vector2<f64>,
    /// Location of previous frame
    old: Vector2<f64>,
    /// Target location it is trying to go to
    target: Vector2<f64>,
    /// Movement from last frame
    der: Vector2<f64>,
    /// Current velocity
    vel: Vector2<f64>,
    /// Whether or not it is transitioning over to a target
    fly: bool,
    /// Speed at which it should move to the target
    fly_speed: f64,
    /// Current zoom level of the camera
    pub zoom: f64,
    /// Target zoom level
    zoom_target: f64,
    /// Speed at which it moves to a zoom level
    zoom_speed: f64,
    /// Whether or not the zoom is overriden
    zoom_override: bool,
    /// Pilot the camera is following
    follow_pilot: Option<c_uint>,
}

pub static CAMERA: LazyLock<RwLock<Camera>> = LazyLock::new(|| {
    let angle_sin = (naev_core::constants::CTS.camera_angle as f64).sin();
    GAME_TO_SCREEN.store(angle_sin, Ordering::Relaxed);
    SCREEN_TO_GAME.store(1. / angle_sin, Ordering::Relaxed);
    RwLock::new(Camera {
        zoom: 1.0,
        zoom_speed: unsafe { naevc::conf.zoom_speed },
        ..Default::default()
    })
});

impl Camera {
    pub fn pos(&self) -> Vector2<f64> {
        self.pos + self.offset
    }

    /// Handles updating the camera at every frame
    pub fn update(&mut self, dt: f64) {
        let der = self.pos;
        let old = self.old;
        let mut p: *mut naevc::Pilot = std::ptr::null_mut();

        if self.fly {
            match self.follow_pilot {
                Some(plt) => {
                    p = unsafe { naevc::pilot_get(plt) };
                    if p.is_null() {
                        self.follow_pilot = None;
                        self.fly = false;
                    } else {
                        let pos =
                            unsafe { Vector2::<f64>::new((*p).solid.pos.x, (*p).solid.pos.y) };
                        self.update_fly(pos, dt);
                        self.update_pilot_zoom(p, std::ptr::null(), dt);
                    }
                }
                None => {
                    self.update_fly(self.target, dt);
                }
            }
        } else if let Some(plt) = self.follow_pilot {
            p = unsafe { naevc::pilot_get(plt) };
            if p.is_null() {
                self.follow_pilot = None;
            } else {
                self.update_pilot(p, dt);
            }
        }

        let zoom_manual = unsafe { naevc::conf.zoom_manual != 0 };
        if zoom_manual || self.zoom_override {
            self.update_manual_zoom(dt);
        }

        unsafe {
            if p.is_null() {
                let dx = dt * (old.x - self.pos.x);
                let dy = dt * (old.y - self.pos.y);
                naevc::sound_updateListener(SOUND_DIR, self.pos.x, self.pos.y, dx, dy);
            } else {
                naevc::sound_updateListener(
                    SOUND_DIR,
                    (*p).solid.pos.x,
                    (*p).solid.pos.y,
                    (*p).solid.vel.x,
                    (*p).solid.vel.y,
                );
            }
        }

        self.der.x = self.pos.x - der.x;
        self.der.y = self.pos.y - der.y;

        if dt > naevc::DOUBLE_TOL {
            self.vel.x = self.der.x / dt;
            self.vel.y = self.der.y / dt;
        }
    }

    fn update_fly(&mut self, pos: Vector2<f64>, dt: f64) {
        let max = self.fly_speed * dt;
        let k = 25. * dt;
        let mut der = (pos - self.pos) * k;
        if der.x * der.x + der.y * der.y > max * max {
            let a = der.y.atan2(der.x);
            let r = max;
            der = Vector2::<f64>::new(r * a.cos(), r * a.sin());
        }
        self.pos += der;

        /* Stop within 100 pixels. */
        if (self.pos - pos).norm_squared() < 100.0 * 100.0 {
            self.old = self.pos;
            self.fly = false;
        }

        unsafe {
            naevc::background_moveDust(-der.x as c_double, -der.y as c_double);
        }
    }

    fn update_manual_zoom(&mut self, dt: f64) {
        unsafe {
            if naevc::player.p.is_null() {
                return;
            }
        }

        let (dt_mod, zoom_far, zoom_near) =
            unsafe { (naevc::dt_mod, naevc::conf.zoom_far, naevc::conf.zoom_near) };

        /* Gradually zoom in/out. */
        let mut dz = (self.zoom_target - self.zoom).clamp(-self.zoom_speed, self.zoom_speed);
        dz *= dt / dt_mod; /* Remove dt dependence. */
        if dz < 0. {
            /* Speed up if needed. */
            dz *= 2.;
        }
        self.zoom += dz;
        self.zoom = self.zoom.clamp(zoom_far, zoom_near);
    }

    fn update_pilot_zoom(
        &mut self,
        follow: *const naevc::Pilot,
        target: *const naevc::Pilot,
        dt: f64,
    ) {
        let zoom_manual = unsafe { naevc::conf.zoom_manual != 0 };
        if zoom_manual || self.zoom_override {
            return;
        }

        let follow_vel =
            unsafe { Vector2::<f64>::new((*follow).solid.vel.x, (*follow).solid.vel.y) };

        let (screen_w, screen_h, zoom_far, zoom_near, nebu_density, dt_mod) = unsafe {
            (
                naevc::gl_screen.w,
                naevc::gl_screen.h,
                naevc::conf.zoom_far,
                naevc::conf.zoom_near,
                (*naevc::cur_system).nebu_density,
                naevc::dt_mod,
            )
        };

        /* Minimum depends on velocity normal.
         *
         * w*h = A, cte    area constant
         * w/h = K, cte    proportion constant
         * d^2 = A, cte    geometric longitud
         *
         * A_v = A*(1+v/d)   area of view is based on speed
         * A_v / A = (1 + v/d)
         *
         * z = A / A_v = 1. / (1 + v/d)
         */
        let d = {
            let wh: f64 = (screen_w * screen_h).into();
            wh.sqrt()
        };

        let zfar = if nebu_density > 0. {
            let c: f64 = screen_w.min(screen_h).into();
            let sight: f64 = unsafe { naevc::nebu_getSightRadius() };
            (c * 0.5 / sight).clamp(zoom_far, zoom_near)
        } else {
            zoom_far
        };
        let znear = zoom_near.min(1. / (0.8 + follow_vel.norm() / d)).max(zfar);

        /* Set zoom to pilot. */
        let z = self.zoom;
        let tz = {
            let stealth = unsafe { (*follow).flags[naevc::PILOT_STEALTH as usize] != 0 };
            if stealth {
                zfar
            } else if target.is_null() {
                znear
            } else {
                let mut pos = Vector2::<f64>::new(0.0, 0.0);
                let target_pos =
                    unsafe { Vector2::<f64>::new((*target).solid.pos.x, (*target).solid.pos.y) };
                let follow_pos =
                    unsafe { Vector2::<f64>::new((*follow).solid.pos.x, (*follow).solid.pos.y) };
                pos += target_pos - follow_pos;

                /* Get distance ratio. */
                let size: f64 = unsafe { (*(*target).ship).size };
                let w: f64 = screen_w.into();
                let h: f64 = screen_h.into();
                let dx = (w * 0.5) / (pos.x.abs() + 2. * size);
                let dy = (h * 0.5) / (pos.y.abs() + 2. * size);
                dx.min(dy)
            }
        };

        let mut dz = (tz - z).clamp(-self.zoom_speed, self.zoom_speed);
        dz *= dt / dt_mod;
        if dz < 0. {
            dz *= 2.;
        }
        self.zoom += dz;
        self.zoom = self.zoom.clamp(zfar, znear);
    }

    fn update_pilot(&mut self, follow: *mut naevc::Pilot, dt: f64) {
        let hyperspace = unsafe { (*follow).flags[naevc::PILOT_HYPERSPACE as usize] != 0 };
        let target = if !hyperspace {
            unsafe { naevc::pilot_getTarget(follow) }
        } else {
            std::ptr::null()
        };

        /* Real diagonal might be a bit too harsh since it can cut out the ship,
         * we'll just use the largest of the two. */
        /*diag2 = pow2(SCREEN_W) + pow2(SCREEN_H);*/
        /*diag2 = pow2( MIN(SCREEN_W, SCREEN_H) );*/
        let diag2: f64 = 100. * 100.;
        let pos = unsafe { Vector2::<f64>::new((*follow).solid.pos.x, (*follow).solid.pos.y) };

        /* Compensate player movement. */
        let mov = pos - self.old;
        self.pos += mov;

        /* Set old position. */
        self.old = pos;

        /* Compute bias. */
        let mut bias = if !target.is_null() {
            let target_pos =
                unsafe { Vector2::<f64>::new((*target).solid.pos.x, (*target).solid.pos.y) };
            target_pos - pos
        } else {
            Default::default()
        };

        /* Bias towards velocity and facing. */
        let mut vel = unsafe { Vector2::<f64>::new((*follow).solid.vel.x, (*follow).solid.vel.y) };
        let fdir = unsafe { (*follow).solid.dir };
        let mut dir: f64 = angle_diff(vel.y.atan2(vel.x), fdir);
        dir = (std::f64::consts::PI - dir.abs()) / std::f64::consts::PI;
        vel *= dir;
        bias += vel;

        /* Limit bias. */
        if bias.norm_squared() > diag2 * 0.5 {
            let a = bias.y.atan2(bias.x);
            let r = diag2.sqrt() * 0.5;
            bias.x = r * a.cos();
            bias.y = r * a.sin();
        }

        /* Set up the target. */
        let targ = pos + bias;

        /* Head towards target. */
        let dt_mod = unsafe { naevc::dt_mod };
        let k = 0.5 * dt / dt_mod;
        let der = (targ - self.pos) * k;

        /* Update camera. */
        self.pos += der;

        self.update_pilot_zoom(follow, target, dt);

        unsafe {
            naevc::background_moveDust(-(mov.x + der.x), -(mov.y + der.y));
        }
    }

    /// Converts from in-game coordinates to screen coordinates
    pub fn game_to_screen_coords(&self, pos: Vector2<f64>) -> Vector2<f64> {
        let view_width = crate::VIEW_WIDTH.load(Ordering::Relaxed);
        let view_height = crate::VIEW_HEIGHT.load(Ordering::Relaxed);
        let view = Vector2::new(view_width as f64, view_height as f64);
        let mut screen = (pos - self.pos()) * self.zoom;
        screen.y *= GAME_TO_SCREEN.load(Ordering::Relaxed);
        screen + view * 0.5
    }

    /// Converts from in-game coordinates to screen coordinates
    pub fn screen_to_game_coords(&self, pos: Vector2<f64>) -> Vector2<f64> {
        let view_width = crate::VIEW_WIDTH.load(Ordering::Relaxed);
        let view_height = crate::VIEW_HEIGHT.load(Ordering::Relaxed);
        let view = Vector2::new(view_width as f64, view_height as f64);
        let mut game = (pos - view * 0.5) / self.zoom;
        game.y *= SCREEN_TO_GAME.load(Ordering::Relaxed);
        game + self.pos()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_zoomOverride(enable: c_int) {
    let mut cam = CAMERA.write().unwrap();
    cam.zoom_override = enable != 0;
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_setZoom(zoom: c_double) {
    let mut cam = CAMERA.write().unwrap();
    unsafe {
        cam.zoom = zoom.clamp(naevc::conf.zoom_far, naevc::conf.zoom_near);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_setZoomTarget(zoom: c_double, speed: c_double) {
    let mut cam = CAMERA.write().unwrap();
    unsafe {
        cam.zoom_target = zoom.clamp(naevc::conf.zoom_far, naevc::conf.zoom_near);
    }
    cam.zoom_speed = speed;
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getZoom() -> c_double {
    let cam = CAMERA.read().unwrap();
    cam.zoom as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getZoomTarget() -> c_double {
    let cam = CAMERA.read().unwrap();
    cam.zoom_target as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getPos(x: *mut c_double, y: *mut c_double) {
    let cam = CAMERA.read().unwrap();
    unsafe {
        let pos = cam.pos();
        *x = pos.x as c_double;
        *y = pos.y as c_double;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getDPos(dx: *mut c_double, dy: *mut c_double) {
    let cam = CAMERA.read().unwrap();
    unsafe {
        *dx = cam.der.x as c_double;
        *dy = cam.der.y as c_double;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getVel(vx: *mut c_double, vy: *mut c_double) {
    let cam = CAMERA.read().unwrap();
    unsafe {
        *vx = cam.vel.x as c_double;
        *vy = cam.vel.y as c_double;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_vel(vx: c_double, vy: c_double) {
    let mut cam = CAMERA.write().unwrap();
    cam.vel.x = vx;
    cam.vel.y = vy;
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_setTargetPilot(follow: c_uint, soft_over: c_int) {
    let mut cam = CAMERA.write().unwrap();
    cam.follow_pilot = match follow {
        0 => None,
        _ => Some(follow),
    };

    if soft_over == 0 {
        if follow != 0 {
            let p = unsafe { naevc::pilot_get(follow) };
            let x = unsafe { (*p).solid.pos.x };
            let y = unsafe { (*p).solid.pos.y };
            cam.pos.x = x;
            cam.pos.y = y;
            cam.old.x = x;
            cam.old.y = y;
        }
        cam.fly = false;
    } else {
        cam.old.x = cam.pos.x;
        cam.old.y = cam.pos.y;
        cam.fly = true;
        cam.fly_speed = soft_over.into();
    }

    unsafe {
        naevc::sound_updateListener(SOUND_DIR, cam.pos.x, cam.pos.y, 0., 0.);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_setTargetPos(x: c_double, y: c_double, soft_over: c_int) {
    let mut cam = CAMERA.write().unwrap();
    cam.follow_pilot = None;
    if soft_over == 0 {
        cam.pos.x = x;
        cam.pos.y = y;
        cam.old.x = x;
        cam.old.y = y;
        cam.fly = false;
    } else {
        cam.target.x = x;
        cam.target.y = y;
        cam.old.x = cam.pos.x;
        cam.old.y = cam.pos.y;
        cam.fly = true;
        cam.fly_speed = soft_over.into()
    };
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_getTarget() -> c_uint {
    let cam = CAMERA.read().unwrap();
    cam.follow_pilot.unwrap_or_default()
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_update(dt: c_double) {
    let mut cam = CAMERA.write().unwrap();
    cam.update(dt);
}

#[unsafe(no_mangle)]
pub extern "C" fn cam_setOffset(x: c_double, y: c_double) {
    let mut cam = CAMERA.write().unwrap();
    cam.offset = Vector2::new(x, y);
}

/// @brief Lua bindings to interact with the Camera.
///
/// An example would be:
/// @code
/// @endcode
///
/// @luamod camera
#[allow(dead_code, unused_doc_comments)]
pub fn open_camera(lua: &mlua::Lua) -> Result<()> {
    let globals = lua.globals();
    let api = lua.create_table()?;
    /// @brief Sets the camera.
    ///
    /// Make sure to reset camera after using it or we'll run into trouble.
    ///
    /// @usage camera.set() -- Resets the camera to the pilot hard.
    /// @usage camera.set( a_pilot, true ) -- Flies camera over to a_pilot.
    /// @usage camera.set( vec2.new() ) -- Jumps camera to 0,0
    ///
    ///    @luatparam Pilot|Vec2|nil target It will follow pilots around. If nil, it
    /// follows the player.
    ///    @luatparam[opt=false] boolean hard_over Indicates that the camera should
    /// instantly teleport instead of fly over.
    ///    @luaparam[opt=math.min(2000,distance)] speed Speed at which to fly over if
    /// hard_over is false.
    /// @luafunc set
    api.set(
        "set",
        lua.create_function(|_lua, ()| -> mlua::Result<(f64, f64, f64)> {
            let cam = CAMERA.read().unwrap();
            Ok((cam.pos.x, cam.pos.y, 1.0 / cam.zoom))
        })?,
    )?;
    /// @brief Gets the x/y position and zoom of the camera.
    ///
    ///    @luatreturn number X position of the camera.
    ///    @luatreturn number Y position of the camera.
    ///    @luatreturn number Zoom level of the camera.
    /// @luafunc get
    api.set(
        "get",
        lua.create_function(|_lua, ()| -> mlua::Result<(f64, f64, f64)> {
            let cam = CAMERA.read().unwrap();
            Ok((cam.pos.x, cam.pos.y, 1.0 / cam.zoom))
        })?,
    )?;
    /// @brief Gets the camera position.
    ///
    ///    @luatreturn Vec2 Position of the camera.
    /// @luafunc pos
    api.set(
        "pos",
        lua.create_function(|_lua, ()| -> mlua::Result<Vec2> {
            let cam = CAMERA.read().unwrap();
            Ok(Vec2::new(cam.pos.x, cam.pos.y))
        })?,
    )?;
    /// @brief Sets the camera zoom.
    ///
    /// Make sure to reset camera the zoom after using it or we'll run into trouble.
    ///
    /// @usage camera.setZoom() -- Resets the camera zoom
    ///
    ///    @luatparam number zoom Level of zoom to use (1 would indicate 1 unit = 1
    /// pixel while 2 would be 1 unit = 2 pixels)
    ///    @luatparam[opt=false] boolean hard_over Indicates that the camera should
    /// change the zoom gradually instead of instantly.
    ///    @luatparam[opt=naev.conf().zoom_speed]  number speed Rate of change to
    /// use.
    /// @luafunc setZoom
    api.set(
        "setZoom",
        lua.create_function(|_lua, ()| -> mlua::Result<Vec2> {
            let cam = CAMERA.read().unwrap();
            Ok(Vec2::new(cam.pos.x, cam.pos.y))
        })?,
    )?;
    /// @brief Gets the camera zoom.
    ///
    ///    @luatreturn number Zoom level of the camera.
    ///    @luatreturn number Maximum zoom level of the camera (furthest).
    ///    @luatreturn number Minimum zoom level of the camera (closest).
    /// @luafunc getZoom
    api.set(
        "getZoom",
        lua.create_function(|_lua, ()| -> mlua::Result<(f64, f64, f64)> {
            let cam = CAMERA.read().unwrap();
            let (zoom_far, zoom_near) = unsafe { (naevc::conf.zoom_far, naevc::conf.zoom_near) };
            Ok((1.0 / cam.zoom, 1.0 / zoom_far, 1.0 / zoom_near))
        })?,
    )?;
    /// @brief Makes the camera shake.
    ///
    /// @usage camera.shake() -- Shakes the camera with amplitude 1.
    /// @usage camera.shake( 0.5 ) -- Shakes the camera with amplitude .5
    ///
    ///    @luatparam number amplitude Amplitude of the shaking.
    /// @luafunc shake
    api.set(
        "shake",
        lua.create_function(|_lua, amplitude: f64| -> mlua::Result<()> {
            unsafe {
                naevc::spfx_shake(amplitude);
            }
            Ok(())
        })?,
    )?;
    globals.set("camera", api)?;
    Ok(())
}
