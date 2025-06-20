use rayon::prelude::*;
use std::ffi::c_void;

struct ShipWrapper(naevc::Ship);
//unsafe impl Sync for ShipWrapper {}
unsafe impl Send for ShipWrapper {}

#[unsafe(no_mangle)]
pub extern "C" fn ship_gfxLoadNeeded_() {
    let (ships, n) = unsafe {
        let ss = naevc::ship_getAll();
        (ss, naevc::array_size_rust(ss as *const c_void) as usize)
    };

    let ss: &mut [ShipWrapper] =
        unsafe { std::slice::from_raw_parts_mut(ships as *mut ShipWrapper, n) };
    ss.par_iter_mut().for_each(|ptr| {
        let s = &mut ptr.0;
        if s.flags & naevc::SHIP_NEEDSGFX > 0 {
            // TODO oad graphics here
            s.flags &= !naevc::SHIP_NEEDSGFX;
        }
    });
}
