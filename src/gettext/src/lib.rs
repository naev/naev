use std::ffi::{CStr, CString};

pub fn init() {
    unsafe {
        naevc::gettext_init();
    };
}

#[allow(non_snake_case, dead_code)]
pub fn N_(s: &str) -> &str {
    s
}

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
        let ptr3 = naevc::gettext_ngettext(ptr1, ptr2, n as u64);
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

//pub fn pgettext<T, U>(msgctxt: T, msgid: U) -> String
//where
//    T: Into<String>,
//    U: Into<String>,
pub fn pgettext<'a>(msgctxt: &'a str, msg_id: &'a str) -> &'a str {
    let msgctxt = CString::new(msgctxt).expect("`msgctxt` contains an internal 0 byte");
    let msgid = CString::new(msg_id).expect("`msgid` contains an internal 0 byte");
    unsafe {
        let ptr1 = msgid.as_ptr();
        let ptr2 = naevc::pgettext_var(msgctxt.as_ptr(), ptr1);
        if ptr1 == ptr2 {
            return msg_id;
        }
        CStr::from_ptr(ptr2)
            .to_str()
            .expect("ngettext() returned invalid UTF-8")
    }
}
