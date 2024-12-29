use crate::physics::angle_diff;
use nalgebra::{Point2, Vector2};
use std::os::raw::{c_double, c_int, c_uint};
use std::sync::{LazyLock, Mutex};

#[derive(Default, Clone)]
pub struct Camera {
    pos: Point2<f64>,
    old: Point2<f64>,
    target: Point2<f64>,
    der: Vector2<f64>,
    vel: Vector2<f64>,

    fly: bool,
    fly_speed: f64,
    zoom: f64,
    zoom_target: f64,
    zoom_speed: f64,
    zoom_override: bool,
    follow_pilot: c_uint,
}

static CAMERA: LazyLock<Mutex<Camera>> = LazyLock::new(Default::default);

impl Camera {
    pub fn update(&mut self, dt: f64) {
        let der = self.pos;
        let old = self.old;
        let mut p: *mut naevc::Pilot = std::ptr::null_mut();

        if self.fly {
            if self.follow_pilot != 0 {
                p = unsafe { naevc::pilot_get(self.follow_pilot) };
                if p == std::ptr::null_mut() {
                    self.follow_pilot = 0;
                    self.fly = false;
                } else {
                    let pos = unsafe {
                        Point2::<f64>::new((*p).solid.pos.x.into(), (*p).solid.pos.y.into())
                    };
                    self.update_fly(pos, dt);
                    self.update_pilot_zoom(p, std::ptr::null(), dt);
                }
            } else {
                self.update_fly(self.target, dt);
            }
        } else {
            if self.follow_pilot != 0 {
                p = unsafe { naevc::pilot_get(self.follow_pilot) };
                if p == std::ptr::null_mut() {
                    self.follow_pilot = 0;
                } else {
                    self.update_pilot(p, dt);
                }
            }
        }

        let zoom_manual = unsafe { naevc::conf.zoom_manual != 0 };
        if zoom_manual || self.zoom_override {
            self.update_manual_zoom(dt);
        }

        unsafe {
            if p == std::ptr::null_mut() {
                let dx = dt * (old.x - self.pos.x);
                let dy = dt * (old.y - self.pos.y);
                naevc::sound_updateListener(
                    std::f64::consts::FRAC_PI_2,
                    self.pos.x,
                    self.pos.y,
                    dx,
                    dy,
                );
            } else {
                naevc::sound_updateListener(
                    std::f64::consts::FRAC_PI_2,
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

    fn update_fly(&mut self, pos: Point2<f64>, dt: f64) {
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
        if nalgebra::distance_squared(&self.pos, &pos) < 100. * 100. {
            self.old = self.pos;
            self.fly = false;
        }

        unsafe {
            naevc::background_moveDust(-der.x as c_double, -der.y as c_double);
        }
    }

    fn update_manual_zoom(&mut self, dt: f64) {
        unsafe {
            if naevc::player.p == std::ptr::null_mut() {
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
            } else {
                if target == std::ptr::null_mut() {
                    znear
                } else {
                    let mut pos = {
                        let mut x = Box::<c_double>::new(0.);
                        let mut y = Box::<c_double>::new(0.);
                        unsafe {
                            naevc::gui_getOffset(x.as_mut(), y.as_mut());
                        };
                        Vector2::<f64>::new(*x, *y)
                    };
                    let target_pos = unsafe {
                        Point2::<f64>::new(
                            (*target).solid.pos.x.into(),
                            (*target).solid.pos.y.into(),
                        )
                    };
                    let follow_pos = unsafe {
                        Point2::<f64>::new(
                            (*follow).solid.pos.x.into(),
                            (*follow).solid.pos.y.into(),
                        )
                    };
                    pos += target_pos - follow_pos;

                    /* Get distance ratio. */
                    let size: f64 = unsafe { (*(*target).ship).size }.into();
                    let w: f64 = screen_w.into();
                    let h: f64 = screen_h.into();
                    let dx = (w * 0.5) / (pos.x.abs() + 2. * size);
                    let dy = (h * 0.5) / (pos.y.abs() + 2. * size);
                    dx.min(dy)
                }
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
        let pos = unsafe {
            Point2::<f64>::new((*follow).solid.pos.x.into(), (*follow).solid.pos.y.into())
        };

        /* Compensate player movement. */
        let mov = pos - self.old;
        self.pos += mov;

        /* Set old position. */
        self.old = pos;

        /* Compute bias. */
        let mut bias = if target != std::ptr::null() {
            let target_pos = unsafe {
                Point2::<f64>::new((*target).solid.pos.x.into(), (*target).solid.pos.y.into())
            };
            target_pos - pos
        } else {
            Default::default()
        };

        /* Bias towards velocity and facing. */
        let mut vel = unsafe {
            Vector2::<f64>::new((*follow).solid.vel.x.into(), (*follow).solid.vel.y.into())
        };
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
}

#[no_mangle]
pub unsafe extern "C" fn cam_zoomOverride(enable: c_int) {
    let mut cam = CAMERA.lock().unwrap();
    cam.zoom_override = enable != 0;
}

#[no_mangle]
pub unsafe extern "C" fn cam_setZoom(zoom: c_double) {
    let mut cam = CAMERA.lock().unwrap();
    cam.zoom = zoom
        .clamp(naevc::conf.zoom_far, naevc::conf.zoom_near)
        .into();
}

#[no_mangle]
pub unsafe extern "C" fn cam_setZoomTarget(zoom: c_double, speed: c_double) {
    let mut cam = CAMERA.lock().unwrap();
    cam.zoom_target = zoom
        .clamp(naevc::conf.zoom_far, naevc::conf.zoom_near)
        .into();
    cam.zoom_speed = speed.into();
}

#[no_mangle]
pub unsafe extern "C" fn cam_getZoom() -> c_double {
    let cam = CAMERA.lock().unwrap();
    cam.zoom as c_double
}

#[no_mangle]
pub unsafe extern "C" fn cam_getZoomTarget() -> c_double {
    let cam = CAMERA.lock().unwrap();
    cam.zoom_target as c_double
}

#[no_mangle]
pub unsafe extern "C" fn cam_getPos(x: *mut c_double, y: *mut c_double) {
    let cam = CAMERA.lock().unwrap();
    *x = cam.pos.x as c_double;
    *y = cam.pos.y as c_double;
}

#[no_mangle]
pub unsafe extern "C" fn cam_getDPos(dx: *mut c_double, dy: *mut c_double) {
    let cam = CAMERA.lock().unwrap();
    *dx = cam.der.x as c_double;
    *dy = cam.der.y as c_double;
}

#[no_mangle]
pub unsafe extern "C" fn cam_getVel(vx: *mut c_double, vy: *mut c_double) {
    let cam = CAMERA.lock().unwrap();
    *vx = cam.vel.x as c_double;
    *vy = cam.vel.y as c_double;
}

#[no_mangle]
pub unsafe extern "C" fn cam_vel(vx: c_double, vy: c_double) {
    let mut cam = CAMERA.lock().unwrap();
    cam.vel.x = vx.into();
    cam.vel.y = vy.into();
}

#[no_mangle]
pub unsafe extern "C" fn cam_setTargetPilot(follow: c_uint, soft_over: c_int) {
    let mut cam = CAMERA.lock().unwrap();
    cam.follow_pilot = follow;

    if soft_over == 0 {
        if follow != 0 {
            let p = naevc::pilot_get(follow);
            let x = (*p).solid.pos.x;
            let y = (*p).solid.pos.y;
            cam.pos.x = x.into();
            cam.pos.y = y.into();
            cam.old.x = x.into();
            cam.old.y = y.into();
        }
        cam.fly = false;
    } else {
        cam.old.x = cam.pos.x;
        cam.old.y = cam.pos.y;
        cam.fly = true;
        cam.fly_speed = soft_over.into();
    }

    naevc::sound_updateListener(std::f64::consts::FRAC_PI_2, cam.pos.x, cam.pos.y, 0., 0.);
}

#[no_mangle]
pub unsafe extern "C" fn cam_setTargetPos(x: c_double, y: c_double, soft_over: c_int) {
    let mut cam = CAMERA.lock().unwrap();
    cam.follow_pilot = 0;
    if soft_over == 0 {
        cam.pos.x = x.into();
        cam.pos.y = y.into();
        cam.old.x = x.into();
        cam.old.y = y.into();
        cam.fly = false;
    } else {
        cam.target.x = x.into();
        cam.target.y = y.into();
        cam.old.x = cam.pos.x;
        cam.old.y = cam.old.y;
        cam.fly = true;
        cam.fly_speed = soft_over.into()
    };
}

#[no_mangle]
pub unsafe extern "C" fn cam_getTarget() -> c_uint {
    let cam = CAMERA.lock().unwrap();
    cam.follow_pilot
}

#[no_mangle]
pub unsafe extern "C" fn cam_update(dt: c_double) {
    let mut cam = CAMERA.lock().unwrap();
    cam.update(dt);
}
