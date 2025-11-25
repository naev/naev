use crate::model::Model;
use anyhow::Result;
use log::{warn, warn_err};
use rayon::prelude::*;
use renderer::{Context, ContextWrapper};
use std::ffi::{CStr, CString, c_void};
use std::path::Path;

struct ShipWrapper(naevc::Ship);
//unsafe impl Sync for ShipWrapper {}
unsafe impl Send for ShipWrapper {}

impl ShipWrapper {
    pub fn name(&self) -> String {
        let s = unsafe { CStr::from_ptr(self.0.name).to_str().unwrap() };
        s.to_string()
    }

    fn load_gfx_3d<P: AsRef<Path>>(&mut self, path: P, ctx: &ContextWrapper) -> Result<()> {
        let m = Model::from_path(ctx, path)?;
        self.0.gfx_3d = m.into_ptr() as *mut naevc::GltfObject;
        unsafe {
            naevc::ship_gfxLoadPost3D(&mut self.0 as *mut naevc::Ship);
        }
        Ok(())
    }

    fn load_gfx_2d<P: AsRef<Path>>(&mut self, path: P, ext: &str) -> Result<()> {
        use std::ops::Deref;
        let cpath = CString::new(path.as_ref().as_os_str().to_string_lossy().deref()).unwrap();
        let cext = CString::new(ext).unwrap();
        unsafe {
            naevc::ship_gfxLoad2D(
                &mut self.0 as *mut naevc::Ship,
                cpath.as_ptr(),
                cext.as_ptr(),
            )
        };
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
    // Try to avoid messing with the context and just find what we have to update first
    let mut needsgfx: Vec<&mut ShipWrapper> = get_mut()
        .iter_mut()
        .filter_map(|ptr| {
            let s = &mut ptr.0;
            if s.flags & naevc::SHIP_NEEDSGFX == 0 {
                return None;
            }
            s.flags &= !naevc::SHIP_NEEDSGFX;
            if !s.gfx_3d.is_null() || !s.gfx_space.is_null() {
                return None;
            }
            Some(ptr)
        })
        .collect();
    if needsgfx.is_empty() {
        return;
    }

    // Now we can mess with the context to try to load them as necessary
    let ctx = Context::get().as_safe_wrap();
    let mut needs2d: Vec<_> = needsgfx
        .par_iter_mut()
        .filter_map(|ptr| {
            let s = &mut ptr.0;

            let cpath = unsafe { CStr::from_ptr(s.gfx_path).to_str().unwrap() };
            let base_path = unsafe {
                CStr::from_ptr(match s.base_path.is_null() {
                    true => s.base_type,
                    false => s.base_path,
                })
                .to_str()
                .unwrap()
            };
            let path = match cpath.starts_with('/') {
                true => cpath,
                false => &{
                    let path = format!("gfx/ship3d/{base_path}/{cpath}");
                    if ndata::is_file(&path) {
                        path
                    } else {
                        format!("gfx/ship3d/{base_path}/{cpath}.gltf")
                    }
                },
            };
            if !ndata::is_file(path) {
                Some(ptr)
            } else {
                if let Err(e) = ptr.load_gfx_3d(path, &ctx) {
                    warn_err!(e);
                }
                None
            }
        })
        .collect();
    drop(ctx); // Need to drop

    // 2D doesn't use the safe context system yet, so it can't be threaded with 3D
    needs2d.iter_mut().for_each(|ptr| {
        let s = ptr.0;
        let cpath = unsafe { CStr::from_ptr(s.gfx_path).to_str().unwrap() };
        let base = cpath.split('_').next().unwrap_or("");
        let ext = unsafe {
            match s.gfx_extension.is_null() {
                true => ".webp",
                false => CStr::from_ptr(s.gfx_extension).to_str().unwrap(),
            }
        };
        let path = match cpath.starts_with('/') {
            true => cpath,
            false => &format!("gfx/ship/{base}/{cpath}"),
        };
        match ptr.load_gfx_2d(path, ext) {
            Ok(_) => (),
            Err(e) => {
                warn!("Unable to load graphics for ship '{}': {}", ptr.name(), e);
            }
        }
    });
}
