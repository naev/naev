use std::ffi::{CStr, CString};
use std::os::raw::c_ulong;

pub fn init() {
    unsafe {
        naevc::gettext_init();
    };
}

#[allow(dead_code)]
/*
pub fn gettext<'a>(msgid: &'a str) -> &'static str {
    let msgid = CString::new(msgid).unwrap();
    unsafe {
        let t = naevc::gettext_rust(msgid.as_ptr() as *const c_char);
        CStr::from_ptr(t).to_str().unwrap()
    }
}
*/
pub fn gettext<T: Into<String>>(msgid: T) -> String {
    let msgid = CString::new(msgid.into()).expect("`msgid` contains an internal 0 byte");
    unsafe {
        CStr::from_ptr(naevc::gettext_rust(msgid.as_ptr()))
            .to_str()
            .expect("gettext() returned invalid UTF-8")
            .to_owned()
    }
}

#[allow(dead_code)]
/*
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
*/
pub fn ngettext<T, S>(msgid: T, msgid_plural: S, n: i32) -> String
where
    T: Into<String>,
    S: Into<String>,
{
    let msgid = CString::new(msgid.into()).expect("`msgid` contains an internal 0 byte");
    let msgid_plural =
        CString::new(msgid_plural.into()).expect("`msgid_plural` contains an internal 0 byte");
    unsafe {
        CStr::from_ptr(naevc::gettext_ngettext(
            msgid.as_ptr(),
            msgid_plural.as_ptr(),
            n as c_ulong,
        ))
        .to_str()
        .expect("ngettext() returned invalid UTF-8")
        .to_owned()
    }
}

#[allow(dead_code)]
/*
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
*/
pub fn pgettext<T, U>(msgctxt: T, msgid: U) -> String
where
    T: Into<String>,
    U: Into<String>,
{
    let msgctxt = CString::new(msgctxt.into()).expect("`msgctxt` contains an internal 0 byte");
    let msgid = CString::new(msgid.into()).expect("`msgid` contains an internal 0 byte");
    unsafe {
        CStr::from_ptr(naevc::pgettext_var(msgctxt.as_ptr(), msgid.as_ptr()))
            .to_str()
            .expect("ngettext() returned invalid UTF-8")
            .to_owned()
    }
}
