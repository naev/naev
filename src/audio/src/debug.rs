#![allow(clippy::upper_case_acronyms)]
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;
use anyhow::Result;
use log::{debug, warn, warn_err};
use std::ffi::{CStr, CString};
use std::sync::OnceLock;

pub const ALC_EXT_DEBUG_NAME: &CStr = c"ALC_EXT_DEBUG";

// Accepted as an attribute to alcCreateContext:
pub const ALC_CONTEXT_FLAGS: ALenum = 0x19CF;

// Accepted as a bitwise-or'd value for the ALC_CONTEXT_FLAGS context creation attribute value:
pub const ALC_CONTEXT_DEBUG_BIT: ALenum = 0x0001;

// Accepted as the <pname> parameter of alGetInteger[v]:
pub const AL_CONTEXT_FLAGS: ALenum = 0x19CF;

//Returned by alGetInteger[v] when <pname> is AL_CONTEXT_FLAGS:
pub const AL_CONTEXT_DEBUG_BIT: ALenum = 0x0001;

// Accepted as the <target> parameter of alEnable, alDisable, and alIsEnabled:
pub const AL_DEBUG_OUTPUT: ALenum = 0x19B2;

// Accepted as the <pname> parameter of alGetPointerEXT and alGetPointervEXT:
pub const AL_DEBUG_CALLBACK_FUNCTION: ALenum = 0x19B3;
pub const AL_DEBUG_CALLBACK_USER_PARAM: ALenum = 0x19B4;

// Accepted or provided by the <source> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <sources> parameter of alGetDebugMessageLogEXT:
pub const AL_DEBUG_SOURCE_API: ALenum = 0x19B5;
pub const AL_DEBUG_SOURCE_AUDIO_SYSTEM: ALenum = 0x19B6;
pub const AL_DEBUG_SOURCE_THIRD_PARTY: ALenum = 0x19B7;
pub const AL_DEBUG_SOURCE_APPLICATION: ALenum = 0x19B8;
pub const AL_DEBUG_SOURCE_OTHER: ALenum = 0x19B9;

// Accepted or provided by the <type> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <types> parameter of alGetDebugMessageLogEXT:
pub const AL_DEBUG_TYPE_ERROR: ALenum = 0x19BA;
pub const AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ALenum = 0x19BB;
pub const AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ALenum = 0x19BC;
pub const AL_DEBUG_TYPE_PORTABILITY: ALenum = 0x19BD;
pub const AL_DEBUG_TYPE_PERFORMANCE: ALenum = 0x19BE;
pub const AL_DEBUG_TYPE_MARKER: ALenum = 0x19BF;
pub const AL_DEBUG_TYPE_OTHER: ALenum = 0x19C2;

// Accepted or provided by the <type> parameter of alDebugMessageControlEXT and ALDEBUGPROCEXT, and returned by the <types> parameter of alGetDebugMessageLogEXT:
pub const AL_DEBUG_TYPE_PUSH_GROUP: ALenum = 0x19C0;
pub const AL_DEBUG_TYPE_POP_GROUP: ALenum = 0x19C1;

// Accepted or provided by the <severity> parameter of alDebugMessageControlEXT, alDebugMessageInsertEXT, and ALDEBUGPROCEXT, and returned by the <severities> parameter of alGetDebugMessageLogEXT:
pub const AL_DEBUG_SEVERITY_HIGH: ALenum = 0x19C3;
pub const AL_DEBUG_SEVERITY_MEDIUM: ALenum = 0x19C4;
pub const AL_DEBUG_SEVERITY_LOW: ALenum = 0x19C5;
pub const AL_DEBUG_SEVERITY_NOTIFICATION: ALenum = 0x19C6;

// Accepted as the <source>, <type>, and <severity> parameters of alDebugMessageControlEXT:
pub const AL_DONT_CARE: ALenum = 0x0002;

// Accepted as the <pname> parameter of alGetBoolean[v], alGetInteger[v], alGetFloat[v], and alGetDouble[v]:
pub const AL_DEBUG_LOGGED_MESSAGES: ALenum = 0x19C7;
pub const AL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH: ALenum = 0x19C8;
pub const AL_MAX_DEBUG_MESSAGE_LENGTH: ALenum = 0x19C9;
pub const AL_MAX_DEBUG_LOGGED_MESSAGES: ALenum = 0x19CA;
pub const AL_MAX_DEBUG_GROUP_STACK_DEPTH: ALenum = 0x19CB;
pub const AL_MAX_LABEL_LENGTH: ALenum = 0x19CC;

// Returned by alGetError:
pub const AL_STACK_OVERFLOW: ALenum = 0x19CD;
pub const AL_STACK_UNDERFLOW: ALenum = 0x19CE;

pub const AL_BUFFER: ALenum = 0x1009;
pub const AL_SOURCE: ALenum = 0x19D0;

pub type ALDEBUGPROC = unsafe extern "C" fn(
    source: ALenum,
    etype: ALenum,
    id: ALuint,
    severity: ALenum,
    length: ALsizei,
    message: *const ALchar,
    user_param: *const ALvoid,
);

pub type ALDEBUGMESSAGECALLBACK =
    unsafe extern "C" fn(callback: ALDEBUGPROC, user_param: *const ALvoid);
pub type ALOBJECTLABEL =
    unsafe extern "C" fn(identifier: ALenum, name: ALuint, length: ALsizei, label: *const ALchar);

#[allow(non_snake_case)]
pub struct Debug {
    // Debug C API
    pub alDebugMessageCallback: ALDEBUGMESSAGECALLBACK,
    pub alObjectLabel: ALOBJECTLABEL,
}

unsafe extern "C" fn debug_callback(
    source: ALenum,
    msg_type: ALenum,
    id: ALuint,
    severity: ALenum,
    length: ALsizei,
    message: *const ALchar,
    user_param: *const ALvoid,
) {
    let s_source = match source {
        AL_DEBUG_SOURCE_API => "api",
        AL_DEBUG_SOURCE_AUDIO_SYSTEM => "audio_system",
        AL_DEBUG_SOURCE_THIRD_PARTY => "third_party",
        AL_DEBUG_SOURCE_APPLICATION => "application",
        AL_DEBUG_SOURCE_OTHER => "other",
        _ => &format!("{source}"),
    };
    let s_type = match msg_type {
        AL_DEBUG_TYPE_ERROR => "error",
        AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR => "deprecated_behavior",
        AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR => "undefined_behavior",
        AL_DEBUG_TYPE_PORTABILITY => "portability",
        AL_DEBUG_TYPE_PERFORMANCE => "performance",
        AL_DEBUG_TYPE_MARKER => "marker",
        AL_DEBUG_TYPE_OTHER => "other",
        _ => &format!("{msg_type}"),
    };
    let s_id = format!("{id}");
    let s_severity = match severity {
        AL_DEBUG_SEVERITY_LOW => "low",
        AL_DEBUG_SEVERITY_MEDIUM => "medium",
        AL_DEBUG_SEVERITY_HIGH => "high",
        AL_DEBUG_SEVERITY_NOTIFICATION => "notification",
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

    if severity == AL_DEBUG_SEVERITY_LOW {
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
    pub fn init(device: &al::Device) -> Result<()> {
        macro_rules! proc_address {
            ($func: literal, $type: ident) => {{
                let val = unsafe { alGetProcAddress($func.as_ptr()) };
                if val.is_null() {
                    warn!(
                        "unable to load proc address for '{}'",
                        $func.to_string_lossy()
                    );
                }
                unsafe { std::mem::transmute::<*mut ALvoid, $type>(val) }
            }};
        }

        let alDebugMessageCallback =
            proc_address!(c"alDebugMessageCallback", ALDEBUGMESSAGECALLBACK);
        let alObjectLabel = proc_address!(c"alObjectLabel", ALOBJECTLABEL);

        let ok = unsafe {
            alEnable(AL_DEBUG_OUTPUT);
            alDebugMessageCallback(debug_callback, std::ptr::null());
            alIsEnabled(AL_DEBUG_OUTPUT) != 0
        };
        if !ok {
            warn!("failed to set AL_DEBUG_OUTPUT");
        }
        let _ = DEBUG.set(Some(Self {
            alDebugMessageCallback,
            alObjectLabel,
        }));
        Ok(())
    }

    pub fn init_none() {
        let _ = DEBUG.set(None);
    }
}

pub fn object_label(identifier: ALenum, name: ALuint, label: &str) {
    #[cfg(debug_assertions)]
    if let Some(dbg) = DEBUG.get().unwrap() {
        let clabel = CString::new(label).unwrap();
        let bytes = clabel.as_bytes_with_nul();
        unsafe {
            (dbg.alObjectLabel)(
                identifier,
                name,
                bytes.len() as ALint,
                bytes.as_ptr() as *const ALchar,
            );
        }
    }
}

static DEBUG: OnceLock<Option<Debug>> = OnceLock::new();
