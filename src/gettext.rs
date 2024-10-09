use std::ffi::{CStr, CString};
use std::os::raw::c_char;

#[allow(dead_code)]
pub fn gettext<'a>(msg_id: &'a str) -> &'static str {
    let cmsg_id = CString::new(msg_id).unwrap();
    unsafe {
        let t = naevc::gettext_rust(cmsg_id.as_ptr() as *const c_char);
        CStr::from_ptr(t).to_str().unwrap()
    }
}

#[allow(dead_code)]
pub fn ngettext<'a>(msg_id: &'a str, msg_id_plural: &'a str, n: u64) -> &'static str {
    let cmsg_id = CString::new(msg_id).unwrap();
    let cmsg_id_plural = CString::new(msg_id_plural).unwrap();
    unsafe {
        let t = naevc::gettext_ngettext(
            cmsg_id.as_ptr() as *const c_char,
            cmsg_id_plural.as_ptr() as *const c_char,
            n,
        );
        CStr::from_ptr(t).to_str().unwrap()
    }
}

#[allow(dead_code)]
pub fn pgettext<'a>(msg_context: &'a str, msg_id: &'a str) -> &'static str {
    let cmsg_context = CString::new(msg_context).unwrap();
    let cmsg_id = CString::new(msg_id).unwrap();
    unsafe {
        let t = naevc::pgettext_var(
            cmsg_context.as_ptr() as *const c_char,
            cmsg_id.as_ptr() as *const c_char,
        );
        CStr::from_ptr(t).to_str().unwrap()
    }
}
