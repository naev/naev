use std::ffi::{CStr, CString};
use std::os::raw::c_ulong;

pub fn init() {
    unsafe {
        naevc::gettext_init();
    };
}

/*
pub fn gettext<'a>(msgid: &'a str) -> &'static str {
    let msgid = CString::new(msgid).unwrap();
    unsafe {
        let t = naevc::gettext_rust(msgid.as_ptr() as *const c_char);
        CStr::from_ptr(t).to_str().unwrap()
    }
}
*/
//pub fn gettext<T: Into<String>>(msgid: T) -> String {
pub fn gettext(msg_id: &str) -> &str {
    let msgid = CString::new(msg_id).expect("`msgid` contains an internal 0 byte");
    unsafe {
        let ptr1 = msgid.as_ptr();
        let ptr2 = naevc::gettext_rust(ptr1);
        // If it's the original language, the pointer will remain unchanged, and CStr::from_ptr
        // will crash
        if ptr1 == ptr2 {
            return msg_id;
        }
        CStr::from_ptr(ptr2)
            .to_str()
            .expect("gettext() returned invalid UTF-8")
    }
}

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
//pub fn ngettext(msg_id: &str, msg_id_plural: &str, n: i32) -> &str {
//where
//    T: Into<String>,
//    U: Into<String>,
pub fn ngettext<'a>(msg_id: &'a str, msg_id_plural: &'a str, n: i32) -> &'a str {
    let msgid = CString::new(msg_id).expect("`msgid` contains an internal 0 byte");
    let msgid_plural =
        CString::new(msg_id_plural).expect("`msgid_plural` contains an internal 0 byte");
    unsafe {
        let ptr1 = msgid.as_ptr();
        let ptr2 = msgid_plural.as_ptr();
        let ptr3 = naevc::gettext_ngettext(ptr1, ptr2, n as c_ulong);
        if ptr1 == ptr3 {
            return msg_id;
        } else if ptr2 == ptr3 {
            return msg_id_plural;
        }
        CStr::from_ptr(ptr3)
            .to_str()
            .expect("ngettext() returned invalid UTF-8")
    }
}

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
//pub fn pgettext<T, U>(msgctxt: T, msgid: U) -> String
//where
//    T: Into<String>,
//    U: Into<String>,
pub fn pgettext<'a>(msgctxt: &'a str, msg_id: &'a str) -> &'a str {
    let msgctxt = CString::new(msgctxt).expect("`msgctxt` contains an internal 0 byte");
    let msgid = CString::new(msg_id).expect("`msgid` contains an internal 0 byte");
    unsafe {
        let ptr1 = msgid.as_ptr();
        let ptr2 = naevc::pgettext_var(msgctxt.as_ptr(), msgid.as_ptr());
        if ptr1 == ptr2 {
            return msg_id;
        }
        CStr::from_ptr(ptr2)
            .to_str()
            .expect("ngettext() returned invalid UTF-8")
    }
}
