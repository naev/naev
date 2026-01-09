//! AL_SOFT_events
#![allow(clippy::upper_case_acronyms, dead_code)]
use crate::openal::al_types::*;
use crate::openal::*;
use anyhow::Result;
use nlog::warn;
use std::sync::OnceLock;

pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_EVENT_CALLBACK_FUNCTION_SOFT: ALenum = 0x19A2;
    pub const AL_EVENT_CALLBACK_USER_PARAM_SOFT: ALenum = 0x19A3;

    pub const AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT: ALenum = 0x19A4;
    pub const AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT: ALenum = 0x19A5;
    pub const AL_EVENT_TYPE_DISCONNECTED_SOFT: ALenum = 0x19A6;
}

pub type Callback = fn(event_type: ALenum, object: ALuint, param: ALuint, message: &str);

pub type ALEVENTPROCSOFT = unsafe extern "C" fn(
    event_type: ALenum,
    object: ALuint,
    param: ALuint,
    length: ALsizei,
    message: *const ALchar,
    user_param: *mut ALvoid,
);

pub type ALEVENTCONTROLSOFT =
    unsafe extern "C" fn(count: ALsizei, types: *const ALenum, enable: ALboolean);
pub type ALEVENTCALLBACKSOFT =
    unsafe extern "C" fn(callback: ALEVENTPROCSOFT, user_param: *mut ALvoid);
pub type ALGETPOINTERSOFT = unsafe extern "C" fn(pname: ALenum) -> *mut ALvoid;
pub type ALGETPOINTERVSOFT = unsafe extern "C" fn(pname: ALenum, values: *mut *mut ALvoid);

unsafe extern "C" fn callback_wrapper(
    event_type: ALenum,
    object: ALuint,
    param: ALuint,
    length: ALsizei,
    message: *const ALchar,
    _user_param: *mut ALvoid,
) {
    let events = EVENTS.get().unwrap();
    let msg = unsafe {
        std::str::from_utf8_unchecked(std::slice::from_raw_parts(
            message as *const u8,
            length as usize,
        ))
    };
    (events.callback)(event_type, object, param, msg);
}

#[allow(non_snake_case)]
pub struct Events {
    callback: Callback,

    alEventControlSOFT: ALEVENTCONTROLSOFT,
    alEventCallbackSOFT: ALEVENTCALLBACKSOFT,
    alGetPointerSOFT: ALGETPOINTERSOFT,
    alGetPointervSOFT: ALGETPOINTERVSOFT,
}
impl Events {
    #[allow(non_snake_case)]
    pub fn init(callback: Callback) -> Result<()> {
        let alEventControlSOFT = proc_address!(c"alEventControlSOFT", ALEVENTCONTROLSOFT);
        let alEventCallbackSOFT = proc_address!(c"alEventCallbackSOFT", ALEVENTCALLBACKSOFT);
        let alGetPointerSOFT = proc_address!(c"alGetPointerSOFT", ALGETPOINTERSOFT);
        let alGetPointervSOFT = proc_address!(c"alGetPointervSOFT", ALGETPOINTERVSOFT);

        // Set the callback
        unsafe {
            alEventCallbackSOFT(callback_wrapper, std::ptr::null_mut());
        }

        match EVENTS.set(Self {
            callback,
            alEventControlSOFT,
            alEventCallbackSOFT,
            alGetPointerSOFT,
            alGetPointervSOFT,
        }) {
            Ok(()) => Ok(()),
            Err(_) => anyhow::bail!("failed to set EVENTS"),
        }
    }
}
static EVENTS: OnceLock<Events> = OnceLock::new();

pub fn event_control(events_data: &[ALenum], enable: bool) {
    match EVENTS.get() {
        Some(events) => unsafe {
            (events.alEventControlSOFT)(
                events_data.len() as ALint,
                events_data.as_ptr(),
                enable as ALboolean,
            );
        },
        None => warn!("Events not enabled"),
    }
}

pub fn supported() -> bool {
    is_extension_present(c"AL_SOFT_events")
}
