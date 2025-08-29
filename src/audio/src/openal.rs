//! OpenAL (Soft) bindings
#![allow(non_snake_case, dead_code)]
use std::ffi::{CStr, CString};
use std::sync::atomic::{AtomicPtr, Ordering};

// === alc.h ===

#[allow(dead_code)]
pub mod alc_types {
    use std::ffi;

    // TODO: If Rust ever stabilises a good way to do opaque types, use that
    // instead of a typedef of void.
    /// Opaque type.
    pub type ALCdevice = ffi::c_void;
    /// Opaque type.
    pub type ALCcontext = ffi::c_void;

    pub type ALCboolean = ffi::c_char;
    pub type ALCchar = ffi::c_char;
    pub type ALCbyte = ffi::c_schar;
    pub type ALCubyte = ffi::c_uchar;
    pub type ALCshort = ffi::c_short;
    pub type ALCushort = ffi::c_ushort;
    pub type ALCint = ffi::c_int;
    pub type ALCuint = ffi::c_uint;
    pub type ALCsizei = ffi::c_int;
    pub type ALCenum = ffi::c_int;
    pub type ALCfloat = ffi::c_float;
    pub type ALCdouble = ffi::c_double;
    pub type ALCvoid = ffi::c_void;
}
use alc_types::*;

pub const ALC_FALSE: ALCboolean = 0;
pub const ALC_TRUE: ALCboolean = 1;

pub const ALC_DEVICE_SPECIFIER: ALCenum = 0x1005;

// Context attributes
pub const ALC_FREQUENCY: ALCint = 0x1007;
pub const ALC_MONO_SOURCES: ALCint = 0x1010;
pub const ALC_STEREO_SOURCES: ALCint = 0x1011;

unsafe extern "C" {
    pub fn alcOpenDevice(devicename: *const ALCchar) -> *mut ALCdevice;
    pub fn alcCloseDevice(device: *mut ALCdevice) -> ALCboolean;

    pub fn alcCreateContext(device: *mut ALCdevice, attrlist: *const ALCint) -> *mut ALCcontext;
    pub fn alcDestroyContext(context: *mut ALCcontext);

    pub fn alcProcessContext(context: *mut ALCcontext);
    pub fn alcSuspendContext(context: *mut ALCcontext);

    pub fn alcMakeContextCurrent(context: *mut ALCcontext) -> ALCboolean;
    pub fn alcGetCurrentContext() -> *mut ALCcontext;
    pub fn alcGetContextsDevice(context: *mut ALCcontext) -> *mut ALCdevice;

    pub fn alcGetError(device: *mut ALCdevice) -> ALCenum;

    pub fn alcGetString(device: *mut ALCdevice, param: ALCenum) -> *const ALCchar;
    pub fn alcGetIntegerv(
        device: *mut ALCdevice,
        param: ALCenum,
        size: ALCsizei,
        data: *mut ALCint,
    );

    pub fn alcIsExtensionPresent(device: *mut ALCdevice, extname: *const ALchar) -> ALCboolean;
}

// === al.h ===

#[allow(dead_code)]
pub mod al_types {
    use std::ffi;

    pub type ALboolean = ffi::c_char;
    pub type ALchar = ffi::c_char;
    pub type ALbyte = ffi::c_schar;
    pub type ALubyte = ffi::c_uchar;
    pub type ALshort = ffi::c_short;
    pub type ALushort = ffi::c_ushort;
    pub type ALint = ffi::c_int;
    pub type ALuint = ffi::c_uint;
    pub type ALsizei = ffi::c_int;
    pub type ALenum = ffi::c_int;
    pub type ALfloat = ffi::c_float;
    pub type ALdouble = ffi::c_double;
    pub type ALvoid = ffi::c_void;
}
use al_types::*;

pub const AL_NO_ERROR: ALenum = 0;

pub const AL_VENDOR: ALenum = 0xb001;
pub const AL_VERSION: ALenum = 0xb002;
pub const AL_RENDERER: ALenum = 0xb003;
pub const AL_EXTENSIONS: ALenum = 0xb004;

pub const AL_MAX_GAIN: ALenum = 0x100E;

pub const AL_SOURCE_STATE: ALenum = 0x1010;

pub const AL_INITIAL: ALenum = 0x1011;
pub const AL_PLAYING: ALenum = 0x1012;
pub const AL_PAUSED: ALenum = 0x1013;
pub const AL_STOPPED: ALenum = 0x1014;

pub const AL_BUFFERS_QUEUED: ALenum = 0x1015;
pub const AL_BUFFERS_PROCESSED: ALenum = 0x1016;

pub const AL_REFERENCE_DISTANCE: ALenum = 0x1020;
pub const AL_ROLLOFF_FACTOR: ALenum = 0x1021;
pub const AL_CONE_OUTER_GAIN: ALenum = 0x1022;
pub const AL_MAX_DISTANCE: ALenum = 0x1023;

pub const AL_SEC_OFFSET: ALenum = 0x1024;
pub const AL_SAMPLE_OFFSET: ALenum = 0x1025;
pub const AL_BYTE_OFFSET: ALenum = 0x1026;

pub const AL_SOURCE_TYPE: ALenum = 0x1027;
pub const AL_STATIC: ALenum = 0x1028;
pub const AL_STREAMING: ALenum = 0x1029;
pub const AL_UNDETERMINED: ALenum = 0x1030;

pub const AL_FORMAT_MONO8: ALenum = 0x1100;
pub const AL_FORMAT_MONO16: ALenum = 0x1101;
pub const AL_FORMAT_STEREO8: ALenum = 0x1102;
pub const AL_FORMAT_STEREO16: ALenum = 0x1103;
pub const AL_FORMAT_MONO_FLOAT32: ALenum = 0x10010;
pub const AL_FORMAT_STEREO_FLOAT32: ALenum = 0x10011;

pub const AL_DOPPLER_FACTOR: ALenum = 0xC000;
pub const AL_DOPPLER_VELOCITY: ALenum = 0xC001;
pub const AL_SPEED_OF_SOUND: ALenum = 0xC003;

pub const AL_DISABLE_MODEL: ALenum = 0xD000;
pub const AL_INVERSE_DISTANCE: ALenum = 0xD001;
pub const AL_INVERSE_DISTANCE_CLAMPED: ALenum = 0xD001;
pub const AL_LINEAR_DISTANCE: ALenum = 0xD003;
pub const AL_LINEAR_DISTANCE_CLAMPED: ALenum = 0xD003;
pub const AL_EXPONENT_DISTANCE: ALenum = 0xD005;
pub const AL_EXPONENT_DISTANCE_CLAMPED: ALenum = 0xD005;

pub const AL_INVALID_NAME: ALenum = 0xa001;
pub const AL_INVALID_ENUM: ALenum = 0xa002;
pub const AL_INVALID_VALUE: ALenum = 0xa003;
pub const AL_INVALID_OPERATION: ALenum = 0xa004;
pub const AL_OUT_OF_MEMORY: ALenum = 0xa005;

unsafe extern "C" {
    pub fn alGetError() -> ALenum;

    pub fn alDistanceModel(value: ALenum);

    pub fn alGetEnumValue(enumName: *const ALchar) -> ALenum;

    pub fn alIsBuffer(buffer: ALuint) -> ALboolean;
    pub fn alIsSource(source: ALuint) -> ALboolean;

    pub fn alEnable(capability: ALenum);

    pub fn alGetString(param: ALCenum) -> *const ALCchar;

    pub fn alGetBufferi(buffer: ALuint, param: ALenum, value: *const ALint);

    pub fn alListenerf(param: ALenum, value: ALfloat);
    pub fn alListener3f(param: ALenum, value1: ALfloat, value2: ALfloat, value3: ALfloat);
    pub fn alListenerfv(param: ALenum, values: *const ALfloat);
    pub fn alListeneri(param: ALenum, value: ALint);
    pub fn alListener3i(param: ALenum, value1: ALint, value2: ALint, value3: ALint);
    pub fn alListeneriv(param: ALenum, values: *const ALint);

    pub fn alGetListenerf(param: ALenum, value: *mut ALfloat);
    pub fn alGetListener3f(
        param: ALenum,
        value1: *mut ALfloat,
        value2: *mut ALfloat,
        value3: *mut ALfloat,
    );
    pub fn alGetListenerfv(param: ALenum, values: *mut ALfloat);
    pub fn alGetListeneri(param: ALenum, value: *mut ALint);
    pub fn alGetListener3i(
        param: ALenum,
        value1: *mut ALint,
        value2: *mut ALint,
        value3: *mut ALint,
    );
    pub fn alGetListeneriv(param: ALenum, values: *mut ALint);

    pub fn alGenSources(n: ALsizei, sources: *mut ALuint);
    pub fn alDeleteSources(n: ALsizei, sources: *const ALuint);

    pub fn alSourcef(source: ALuint, param: ALenum, value: ALfloat);
    pub fn alSource3f(
        source: ALuint,
        param: ALenum,
        value1: ALfloat,
        value2: ALfloat,
        value3: ALfloat,
    );
    pub fn alSourcefv(source: ALuint, param: ALenum, values: *const ALfloat);
    pub fn alSourcei(source: ALuint, param: ALenum, value: ALint);
    pub fn alSource3i(source: ALuint, param: ALenum, value1: ALint, value2: ALint, value3: ALint);
    pub fn alSourceiv(source: ALuint, param: ALenum, values: *const ALint);

    pub fn alGetSourcef(source: ALuint, param: ALenum, value: *mut ALfloat);
    pub fn alGetSource3f(
        source: ALuint,
        param: ALenum,
        value1: *mut ALfloat,
        value2: *mut ALfloat,
        value3: *mut ALfloat,
    );
    pub fn alGetSourcefv(source: ALuint, param: ALenum, values: *mut ALfloat);
    pub fn alGetSourcei(source: ALuint, param: ALenum, value: *mut ALint);
    pub fn alGetSource3i(
        source: ALuint,
        param: ALenum,
        value1: *mut ALint,
        value2: *mut ALint,
        value3: *mut ALint,
    );
    pub fn alGetSourceiv(source: ALuint, param: ALenum, values: *mut ALint);

    pub fn alSourcePlay(source: ALuint);
    pub fn alSourcePause(source: ALuint);
    pub fn alSourceStop(source: ALuint);
    pub fn alSourceRewind(source: ALuint);

    pub fn alSourceQueueBuffers(source: ALuint, nb: ALsizei, buffers: *const ALuint);
    pub fn alSourceUnqueueBuffers(source: ALuint, nb: ALsizei, buffers: *mut ALuint);

    pub fn alGenBuffers(n: ALsizei, buffers: *mut ALuint);
    pub fn alDeleteBuffers(n: ALsizei, buffers: *const ALuint);

    pub fn alBufferData(
        buffer: ALuint,
        format: ALenum,
        data: *const ALvoid,
        size: ALsizei,
        samplerate: ALsizei,
    );

    pub fn alDopplerFactor(dopplerFactor: ALfloat);
    pub fn alDopplerVelocity(dopplerVelocity: ALfloat);
    pub fn alSpeedOfSound(speed: ALfloat);

    pub fn alGetProcAddress(fname: *const ALchar) -> *mut ALvoid;
}

// === Misc Extensions ===

pub const ALC_OUTPUT_LIMITER_SOFT_NAME: &CStr = c"ALC_SOFT_output_limiter";
pub const ALC_OUTPUT_LIMITER_SOFT: ALCenum = 0x199A;

// === Rust ===

use anyhow::Result;
pub(crate) fn get_error(e: ALenum) -> &'static str {
    match e {
        AL_INVALID_NAME => "a bad name (ID) was passed to an OpenAL function",
        AL_INVALID_ENUM => "an invalid enum value was passed to an OpenAL function",
        AL_INVALID_VALUE => "an invalid value was passed to an OpenAL function",
        AL_INVALID_OPERATION => "the requested operation is not valid",
        AL_OUT_OF_MEMORY => "the requested operation resulted in OpenAL running out of memory",
        _ => "unknown error",
    }
}

#[inline]
pub(crate) fn is_error() -> Option<ALenum> {
    let e = unsafe { alGetError() };
    match e {
        AL_NO_ERROR => None,
        err => Some(err),
    }
}

pub fn get_parameter_str(parameter: ALenum) -> &'static str {
    let val = unsafe { CStr::from_ptr(alGetString(parameter)) };
    val.to_str().unwrap()
}

pub struct Device(AtomicPtr<ALCdevice>);
impl Device {
    pub fn new(devicename: Option<&str>) -> Result<Self> {
        let devicename = devicename.map(|s| CString::new(s).unwrap());
        let device = unsafe {
            alcOpenDevice(match devicename {
                None => std::ptr::null(),
                Some(n) => n.as_ptr(),
            })
        };
        if device.is_null() {
            anyhow::bail!("unable to open default sound device");
        }
        Ok(Self(AtomicPtr::new(device)))
    }

    pub fn raw(&self) -> *mut ALCdevice {
        self.0.load(Ordering::Relaxed)
    }

    pub fn is_extension_present(&self, extname: &CStr) -> bool {
        matches!(
            unsafe { alcIsExtensionPresent(self.raw(), extname.as_ptr()) },
            ALC_TRUE
        )
    }

    pub fn get_parameter_str(&self, parameter: ALCenum) -> &'static str {
        let val = unsafe { CStr::from_ptr(alcGetString(self.raw(), parameter)) };
        val.to_str().unwrap()
    }

    pub fn get_parameter_i32(&self, parameter: ALCenum) -> ALCint {
        let mut val: ALCint = 0;
        unsafe {
            alcGetIntegerv(self.raw(), parameter, 1, &mut val);
        }
        val
    }
}
impl Drop for Device {
    fn drop(&mut self) {
        unsafe {
            alcCloseDevice(self.raw());
        }
    }
}

pub struct Context(AtomicPtr<ALCcontext>);
impl Context {
    pub fn new(device: &Device, attribs: &[ALCenum]) -> Result<Self> {
        let context = unsafe { alcCreateContext(device.raw(), attribs.as_ptr()) };
        if context.is_null() {
            anyhow::bail!("unable to create context");
        }
        Ok(Self(AtomicPtr::new(context)))
    }
}

pub struct Source(ALuint);
impl Source {
    pub fn new() -> Result<Self> {
        let mut src = 0;
        unsafe { alGenSources(1, &mut src) };
        match src {
            0 => {
                let e = unsafe { alGetError() };
                anyhow::bail!(get_error(e))
            }
            v => Ok(Self(v)),
        }
    }

    #[inline]
    pub fn raw(&self) -> ALuint {
        self.0
    }

    pub fn parameter_f32(&self, param: ALenum, value: ALfloat) {
        unsafe {
            alSourcef(self.raw(), param, value);
        }
    }

    pub fn parameter_3_i32(&self, param: ALenum, v1: ALint, v2: ALint, v3: ALint) {
        unsafe {
            alSource3i(self.raw(), param, v1, v2, v3);
        }
    }

    pub fn get_parameter_i32(&self, param: ALenum) -> i32 {
        let mut out: i32 = 0;
        unsafe {
            alGetSourcei(self.raw(), param, &mut out);
        }
        out
    }
}
impl Drop for Source {
    fn drop(&mut self) {
        unsafe {
            alDeleteSources(1, &self.0);
        }
    }
}

pub struct Buffer(ALuint);
impl Buffer {
    pub fn new() -> Result<Self> {
        let mut src = 0;
        unsafe { alGenBuffers(1, &mut src) };
        match src {
            0 => {
                let e = unsafe { alGetError() };
                anyhow::bail!(get_error(e))
            }
            v => Ok(Self(v)),
        }
    }

    #[inline]
    pub fn raw(&self) -> ALuint {
        self.0
    }
}
impl Drop for Buffer {
    fn drop(&mut self) {
        unsafe {
            alDeleteBuffers(1, &self.0);
        }
    }
}
