use rayon::prelude::*;
use std::ffi::c_void;

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
    get_mut().par_iter_mut().for_each(|ptr| {
        let s = &mut ptr.0;
        if s.flags & naevc::SHIP_NEEDSGFX > 0 {
            // TODO oad graphics here
            s.flags &= !naevc::SHIP_NEEDSGFX;
        }
    });
}
