use crate::context::Context;
use crate::model::Model;
use crate::ndata;
use rayon::prelude::*;
use std::ffi::{c_void, CStr};

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
pub extern "C" fn ship_gfxLoadNeeded_() {
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

        let path = unsafe { CStr::from_ptr(s.gfx_path).to_str().unwrap() };
        match ndata::stat(path) {
            Ok(_) => {
                s.gfx_3d =
                    Model::from_path(&ctx, &path).unwrap().into_ptr() as *mut naevc::GltfObject;
            }
            Err(_) => {}
        };

        // TODO reimplement ship_gfxLoad here
    });
}
