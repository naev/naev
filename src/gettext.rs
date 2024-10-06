use std::ffi::CStr;
use std::os::raw::c_char;

#[allow(dead_code)]
pub fn gettext<'a>(msg_id: &'a str) -> &'a str {
    unsafe {
        let t = naevc::gettext_rust(msg_id.as_ptr() as *const c_char);
        CStr::from_ptr(t).to_str().unwrap()
    }
}

#[allow(dead_code)]
pub fn ngettext<'a>(msg_id: &'a str, msg_id_plural: &'a str, n: u64) -> &'a str {
    unsafe {
        let t = naevc::gettext_ngettext(
            msg_id.as_ptr() as *const c_char,
            msg_id_plural.as_ptr() as *const c_char,
            n,
        );
        CStr::from_ptr(t).to_str().unwrap()
    }
}

#[allow(dead_code)]
pub fn pgettext<'a>(msg_context: &'a str, msg_id: &'a str) -> &'a str {
    unsafe {
        let t = naevc::pgettext_var(
            msg_context.as_ptr() as *const c_char,
            msg_id.as_ptr() as *const c_char,
        );
        CStr::from_ptr(t).to_str().unwrap()
    }
}
