//! AL_EXT_debug extension
#![allow(clippy::upper_case_acronyms, dead_code)]
use crate::openal::al_types::*;
use crate::openal::*;
use anyhow::Result;
use log::{debug, warn, warn_err};
use std::ffi::CString;
use std::sync::OnceLock;

pub mod consts {
    use crate::openal::al_types::*;
    // Accepted as an attribute to alcCreateContext:
    pub const ALC_CONTEXT_FLAGS_EXT: ALenum = 0x19CF;

    // Accepted as a bitwise-or'd value for the ALC_CONTEXT_FLAGS context creation attribute value:
    pub const ALC_CONTEXT_DEBUG_BIT_EXT: ALenum = 0x0001;

    // Accepted as the <pname> parameter of alGetInteger[v]:
    pub const AL_CONTEXT_FLAGS_EXT: ALenum = 0x19CF;

    //Returned by alGetInteger[v] when <pname> is AL_CONTEXT_FLAGS:
    pub const AL_CONTEXT_DEBUG_BIT_EXT: ALenum = 0x0001;

    // Accepted as the <target> parameter of alEnable, alDisable, and alIsEnabled:
    pub const AL_DEBUG_OUTPUT_EXT: ALenum = 0x19B2;

    // Accepted as the <pname> parameter of alGetPointerEXT and alGetPointervEXT:
    pub const AL_DEBUG_CALLBACK_FUNCTION_EXT: ALenum = 0x19B3;
    pub const AL_DEBUG_CALLBACK_USER_PARAM_EXT: ALenum = 0x19B4;

    // Accepted or provided by the <source> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <sources> parameter of alGetDebugMessageLogEXT:
    pub const AL_DEBUG_SOURCE_API_EXT: ALenum = 0x19B5;
    pub const AL_DEBUG_SOURCE_AUDIO_SYSTEM_EXT: ALenum = 0x19B6;
    pub const AL_DEBUG_SOURCE_THIRD_PARTY_EXT: ALenum = 0x19B7;
    pub const AL_DEBUG_SOURCE_APPLICATION_EXT: ALenum = 0x19B8;
    pub const AL_DEBUG_SOURCE_OTHER_EXT: ALenum = 0x19B9;

    // Accepted or provided by the <type> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <types> parameter of alGetDebugMessageLogEXT:
    pub const AL_DEBUG_TYPE_ERROR_EXT: ALenum = 0x19BA;
    pub const AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_EXT: ALenum = 0x19BB;
    pub const AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_EXT: ALenum = 0x19BC;
    pub const AL_DEBUG_TYPE_PORTABILITY_EXT: ALenum = 0x19BD;
    pub const AL_DEBUG_TYPE_PERFORMANCE_EXT: ALenum = 0x19BE;
    pub const AL_DEBUG_TYPE_MARKER_EXT: ALenum = 0x19BF;
    pub const AL_DEBUG_TYPE_OTHER_EXT: ALenum = 0x19C2;

    // Accepted or provided by the <type> parameter of alDebugMessageControlEXT and ALDEBUGPROCEXT, and returned by the <types> parameter of alGetDebugMessageLogEXT:
    pub const AL_DEBUG_TYPE_PUSH_GROUP_EXT: ALenum = 0x19C0;
    pub const AL_DEBUG_TYPE_POP_GROUP_EXT: ALenum = 0x19C1;

    // Accepted or provided by the <severity> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <severities> parameter of alGetDebugMessageLogEXT:
    pub const AL_DEBUG_SEVERITY_HIGH_EXT: ALenum = 0x19C3;
    pub const AL_DEBUG_SEVERITY_MEDIUM_EXT: ALenum = 0x19C4;
    pub const AL_DEBUG_SEVERITY_LOW_EXT: ALenum = 0x19C5;
    pub const AL_DEBUG_SEVERITY_NOTIFICATION_EXT: ALenum = 0x19C6;

    // Accepted as the <source>, <type>, and <severity> parameters of alDebugMessageControlEXT:
    pub const AL_DONT_CARE_EXT: ALenum = 0x0002;

    // Accepted as the <pname> parameter of alGetBoolean[v], alGetInteger[v], alGetFloat[v], and alGetDouble[v]:
    pub const AL_DEBUG_LOGGED_MESSAGES_EXT: ALenum = 0x19C7;
    pub const AL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_EXT: ALenum = 0x19C8;
    pub const AL_MAX_DEBUG_MESSAGE_LENGTH_EXT: ALenum = 0x19C9;
    pub const AL_MAX_DEBUG_LOGGED_MESSAGES_EXT: ALenum = 0x19CA;
    pub const AL_MAX_DEBUG_GROUP_STACK_DEPTH_EXT: ALenum = 0x19CB;
    pub const AL_MAX_LABEL_LENGTH_EXT: ALenum = 0x19CC;

    // Returned by alGetError:
    pub const AL_STACK_OVERFLOW_EXT: ALenum = 0x19CD;
    pub const AL_STACK_UNDERFLOW_EXT: ALenum = 0x19CE;

    pub const AL_BUFFER_EXT: ALenum = 0x1009;
    pub const AL_SOURCE_EXT: ALenum = 0x19D0;

    pub const AL_FILTER_EXT: ALenum = 0x19D1;
    pub const AL_EFFECT_EXT: ALenum = 0x19D2;
    pub const AL_AUXILIARY_EFFECT_SLOT_EXT: ALenum = 0x19D3;
}
use consts::*;

pub type ALDEBUGPROCEXT = unsafe extern "C" fn(
    source: ALenum,
    etype: ALenum,
    id: ALuint,
    severity: ALenum,
    length: ALsizei,
    message: *const ALchar,
    user_param: *const ALvoid,
);

pub type ALDEBUGMESSAGECALLBACKEXT =
    unsafe extern "C" fn(callback: ALDEBUGPROCEXT, user_param: *const ALvoid);
pub type ALOBJECTLABELEXT =
    unsafe extern "C" fn(identifier: ALenum, name: ALuint, length: ALsizei, label: *const ALchar);
pub type ALDEBUGMESSAGECONTROLEXT = unsafe extern "C" fn(
    source: ALenum,
    atype: ALenum,
    severity: ALenum,
    count: ALsizei,
    ids: *const ALuint,
    enable: ALboolean,
);

#[allow(non_snake_case)]
pub struct Debug {
    // Debug C API
    pub alDebugMessageCallbackEXT: ALDEBUGMESSAGECALLBACKEXT,
    pub alObjectLabelEXT: ALOBJECTLABELEXT,
    pub alDebugMessageControlEXT: ALDEBUGMESSAGECONTROLEXT,
}

unsafe extern "C" fn debug_callback(
    source: ALenum,
    msg_type: ALenum,
    id: ALuint,
    severity: ALenum,
    length: ALsizei,
    message: *const ALchar,
    _user_param: *const ALvoid,
) {
    let s_source = match source {
        AL_DEBUG_SOURCE_API_EXT => "api",
        AL_DEBUG_SOURCE_AUDIO_SYSTEM_EXT => "audio_system",
        AL_DEBUG_SOURCE_THIRD_PARTY_EXT => "third_party",
        AL_DEBUG_SOURCE_APPLICATION_EXT => "application",
        AL_DEBUG_SOURCE_OTHER_EXT => "other",
        _ => &format!("{source}"),
    };
    let s_type = match msg_type {
        AL_DEBUG_TYPE_ERROR_EXT => "error",
        AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_EXT => "deprecated_behavior",
        AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_EXT => "undefined_behavior",
        AL_DEBUG_TYPE_PORTABILITY_EXT => "portability",
        AL_DEBUG_TYPE_PERFORMANCE_EXT => "performance",
        AL_DEBUG_TYPE_MARKER_EXT => "marker",
        AL_DEBUG_TYPE_OTHER_EXT => "other",
        _ => &format!("{msg_type}"),
    };
    let s_id = format!("{id}");
    let s_severity = match severity {
        AL_DEBUG_SEVERITY_LOW_EXT => "low",
        AL_DEBUG_SEVERITY_MEDIUM_EXT => "medium",
        AL_DEBUG_SEVERITY_HIGH_EXT => "high",
        AL_DEBUG_SEVERITY_NOTIFICATION_EXT => "notification",
        _ => &format!("{severity}"),
    };
    let msg = match str::from_utf8(unsafe {
        std::slice::from_raw_parts(message as *const u8, length as usize)
    }) {
        Ok(msg) => msg,
        Err(err) => {
            warn_err!(err);
            "unknown message"
        }
    };

    if severity == AL_DEBUG_SEVERITY_LOW_EXT {
        debug!(
            "OpenAL debug( source={s_source}, type={s_type}, id={s_id}, severity={s_severity} ): {msg}"
        );
    } else {
        warn!(
            "OpenAL debug( source={s_source}, type={s_type}, id={s_id}, severity={s_severity} ): {msg}"
        );
    }
}

impl Debug {
    #[allow(non_snake_case)]
    pub fn init() -> Result<()> {
        if (get_parameter_i32(AL_CONTEXT_FLAGS_EXT) & AL_CONTEXT_DEBUG_BIT_EXT) == 0 {
            anyhow::bail!("AL_CONTEXT_DEBUG_BIT not set");
        }

        use crate::proc_address;
        let alDebugMessageCallbackEXT =
            proc_address!(c"alDebugMessageCallbackEXT", ALDEBUGMESSAGECALLBACKEXT);
        let alObjectLabelEXT = proc_address!(c"alObjectLabelEXT", ALOBJECTLABELEXT);
        let alDebugMessageControlEXT =
            proc_address!(c"alDebugMessageControlEXT", ALDEBUGMESSAGECONTROLEXT);

        let ok = unsafe {
            alEnable(AL_DEBUG_OUTPUT_EXT);
            alDebugMessageControlEXT(
                AL_DONT_CARE_EXT,
                AL_DONT_CARE_EXT,
                AL_DEBUG_SEVERITY_LOW_EXT,
                0,
                std::ptr::null(),
                AL_TRUE,
            );
            /*
            let ignores: [ALuint;1] = [
                40965, // Exceeding source limit
            ];
            alDebugMessageControlEXT(
                AL_DEBUG_SOURCE_API_EXT,
                AL_DEBUG_TYPE_ERROR_EXT,
                AL_DONT_CARE_EXT,
                ignores.len() as ALsizei,
                ignores.as_ptr(),
                AL_FALSE,
            );
            */
            alDebugMessageCallbackEXT(debug_callback, std::ptr::null());
            alIsEnabled(AL_DEBUG_OUTPUT_EXT) != 0
        };
        if !ok {
            warn!("failed to set AL_DEBUG_OUTPUT");
        }
        match DEBUG.set(Self {
            alDebugMessageCallbackEXT,
            alObjectLabelEXT,
            alDebugMessageControlEXT,
        }) {
            Ok(()) => Ok(()),
            Err(_) => anyhow::bail!("failed to set DEBUG"),
        }
    }
}

pub fn object_label(identifier: ALenum, name: ALuint, label: &str) {
    #[cfg(debug_assertions)]
    if let Some(dbg) = DEBUG.get() {
        let clabel = CString::new(label).unwrap();
        let bytes = clabel.as_bytes_with_nul();
        unsafe {
            (dbg.alObjectLabelEXT)(
                identifier,
                name,
                bytes.len() as ALint,
                bytes.as_ptr() as *const ALchar,
            );
        }
    }
}

static DEBUG: OnceLock<Debug> = OnceLock::new();

pub fn supported(device: &Device) -> bool {
    device.is_extension_present(c"ALC_EXT_debug")
}
