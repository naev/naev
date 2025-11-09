fn main() -> iced::Result {
    log::init().unwrap();
    pluginmgr_gui::open()
}

use std::ffi::c_void;
#[unsafe(no_mangle)]
pub extern "C" fn gettext_rust(ptr: *const c_void) -> *const c_void {
    ptr
}
