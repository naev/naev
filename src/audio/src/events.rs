//! AL_SOFT_events
#![allow(clippy::upper_case_acronyms)]
use crate::openal::al_types::*;
use crate::openal::*;
use anyhow::Result;
use log::warn;
use std::ffi::CStr;
use std::sync::OnceLock;

pub const AL_SOFT_EVENTS_NAME: &CStr = c"AL_SOFT_events";

pub const AL_EVENT_CALLBACK_FUNCTION_SOFT: ALenum = 0x19A2;
pub const AL_EVENT_CALLBACK_USER_PARAM_SOFT: ALenum = 0x19A3;

pub const AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT: ALenum = 0x19A4;
pub const AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT: ALenum = 0x19A5;
pub const AL_EVENT_TYPE_DISCONNECTED_SOFT: ALenum = 0x19A6;

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

#[allow(non_snake_case)]
pub struct Events {
    alEventControlSOFT: ALEVENTCONTROLSOFT,
    alEventCallbackSOFT: ALEVENTCALLBACKSOFT,
    alGetPointerSOFT: ALGETPOINTERSOFT,
    alGetPointervSOFT: ALGETPOINTERVSOFT,
}
impl Events {
    #[allow(non_snake_case)]
    pub fn init() -> Result<()> {
        let alEventControlSOFT = proc_address!(c"alEventControlSOFT", ALEVENTCONTROLSOFT);
        let alEventCallbackSOFT = proc_address!(c"alEventCallbackSOFT", ALEVENTCALLBACKSOFT);
        let alGetPointerSOFT = proc_address!(c"alGetPointerSOFT", ALGETPOINTERSOFT);
        let alGetPointervSOFT = proc_address!(c"alGetPointervSOFT", ALGETPOINTERVSOFT);

        match EVENTS.set(Self {
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
