use crate::model::Model;
use anyhow::Result;
use log::warn;
use rayon::prelude::*;
use renderer::{Context, ContextWrapper};
use std::ffi::{c_void, CStr};
use std::sync::Mutex;

struct ShipWrapper(naevc::Ship);
//unsafe impl Sync for ShipWrapper {}
unsafe impl Send for ShipWrapper {}

impl ShipWrapper {
    pub fn name(&self) -> String {
        let s = unsafe { CStr::from_ptr(self.0.name).to_str().unwrap() };
        s.to_string()
    }

    fn load_gfx_3d(&mut self, path: &str, ctx: &ContextWrapper) -> Result<()> {
        let m = Model::from_path(ctx, &path)?;
        self.0.gfx_3d = m.into_ptr() as *mut naevc::GltfObject;
        unsafe {
            naevc::ship_gfxLoadPost3D(&mut self.0 as *mut naevc::Ship);
        }
        Ok(())
    }

    fn load_gfx_2d(&mut self) -> Result<()> {
        unsafe { naevc::ship_gfxLoad2D(&mut self.0 as *mut naevc::Ship) };
        Ok(())
    }
}

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
        let path = format!("gfx/ship3d/{cpath}");
        if match ndata::is_file(&path) {
            true => ptr.load_gfx_3d(&path, &ctx),
            false => {
                let path = format!("gfx/ship3d/{base_path}/{cpath}.gltf");
                ptr.load_gfx_3d(&path, &ctx)
            }
        }
        .is_err()
        {
            needs2d.lock().unwrap().push(ptr);
        }
    });
    drop(ctx); // Need to drop

    // 2D doesn't use the safe context system yet, so it can't be threaded with 3D
    needs2d
        .lock()
        .unwrap()
        .iter_mut()
        .for_each(|ptr| match ptr.load_gfx_2d() {
            Ok(_) => (),
            Err(e) => {
                warn!("Unable to load graphics for ship '{}': {}", ptr.name(), e);
            }
        });
}
