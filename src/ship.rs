use crate::model::Model;
use log::warn;
use rayon::prelude::*;
use renderer::Context;
use std::ffi::{c_void, CStr};
use std::sync::Mutex;

struct ShipWrapper(naevc::Ship);
//unsafe impl Sync for ShipWrapper {}
unsafe impl Send for ShipWrapper {}

#[allow(dead_code)]
fn get() -> &'static [ShipWrapper] {
    unsafe {
        let ships = naevc::ship_getAll();
        let n = naevc::array_size_rust(ships as *const c_void) as usize;
        std::slice::from_raw_parts(ships as *const ShipWrapper, n)
    }
}

fn get_mut() -> &'static mut [ShipWrapper] {
    unsafe {
        let ships = naevc::ship_getAll();
        let n = naevc::array_size_rust(ships as *const c_void) as usize;
        std::slice::from_raw_parts_mut(ships as *mut ShipWrapper, n)
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn ship_gfxLoadNeeded() {
    let needs2d: Mutex<Vec<&mut ShipWrapper>> = Mutex::new(vec![]);
    let ctx = Context::get().unwrap().as_safe_wrap();
    get_mut().par_iter_mut().for_each(|ptr| {
        let s = &mut ptr.0;
        if s.flags & naevc::SHIP_NEEDSGFX == 0 {
            return;
        }
        s.flags &= !naevc::SHIP_NEEDSGFX;
        if !s.gfx_3d.is_null() || !s.gfx_space.is_null() {
            return;
        }

        let cpath = unsafe { CStr::from_ptr(s.gfx_path).to_str().unwrap() };
        let base_path = unsafe {
            CStr::from_ptr(match s.base_path.is_null() {
                true => s.base_type,
                false => s.base_path,
            })
            .to_str()
            .unwrap()
        };
        let path = format!("gfx/ship3d/{base_path}/{cpath}.gltf");
        match ndata::stat(&path) {
            Ok(_) => match Model::from_path(&ctx, &path) {
                Ok(m) => {
                    s.gfx_3d = m.into_ptr() as *mut naevc::GltfObject;
                    unsafe {
                        naevc::ship_gfxLoadPost3D(s as *mut naevc::Ship);
                    }
                }
                Err(e) => {
                    warn!("Failure loading 3D model '{}': {}", &path, e);
                    needs2d.lock().unwrap().push(ptr);
                }
            },
            Err(_) => {
                needs2d.lock().unwrap().push(ptr);
            }
        };
    });
    drop(ctx); // Need to drop

    // 2D doesn't use the safe context system yet, so it can't be threaded with 3D
    needs2d.lock().unwrap().iter_mut().for_each(|ptr| {
        let s = &mut ptr.0;
        unsafe {
            naevc::ship_gfxLoad2D(s as *mut naevc::Ship);
        }
    });
}
