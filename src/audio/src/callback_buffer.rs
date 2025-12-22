//! AL_SOFT_callback_buffer

#![allow(dead_code)]
use crate::openal::al_types::*;
use crate::openal::*;
use anyhow::{Context, Result};
use std::sync::OnceLock;

pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_BUFFER_CALLBACK_FUNCTION_SOFT: ALenum = 0x19A0;
    pub const AL_BUFFER_CALLBACK_USER_PARAM_SOFT: ALenum = 0x19A1;
}

#[allow(clippy::upper_case_acronyms)]
pub type ALBUFFERCALLBACKTYPESOFT = unsafe extern "C" fn(
    userptr: *mut ALvoid,
    sampledata: *mut ALvoid,
    numbytes: ALsizei,
) -> ALsizei;

#[allow(clippy::upper_case_acronyms)]
pub type ALBUFFERCALLBACKSOFT = unsafe extern "C" fn(
    buffer: ALuint,
    format: ALenum,
    freq: ALsizei,
    callback: ALBUFFERCALLBACKTYPESOFT,
    userptr: *mut ALvoid,
);

#[allow(clippy::upper_case_acronyms)]
pub type ALGETBUFFERPTRSOFT =
    unsafe extern "C" fn(buffer: ALuint, param: ALenum, ptr: *mut *mut ALvoid);

#[allow(clippy::upper_case_acronyms)]
pub type ALGETBUFFER3PTRSOFT = unsafe extern "C" fn(
    buffer: ALuint,
    param: ALenum,
    ptr0: *mut *mut ALvoid,
    ptr1: *mut *mut ALvoid,
    ptr2: *mut *mut ALvoid,
);

#[allow(non_snake_case)]
pub struct CallbackBuffer {
    pub alBufferCallbackSOFT: ALBUFFERCALLBACKSOFT,
    pub alGetBufferPtrSOFT: ALGETBUFFERPTRSOFT,
    pub alGetBuffer3PtrSOFT: ALGETBUFFER3PTRSOFT,
    pub alGetBufferPtrvSOFT: ALGETBUFFERPTRSOFT,
}

#[allow(non_snake_case)]
pub fn init() -> Result<()> {
    if !supported() {
        anyhow::bail!("AL_SOFT_callback_buffer is unsupported");
    }

    let alBufferCallbackSOFT = proc_address!(c"alBufferCallbackSOFT", ALBUFFERCALLBACKSOFT);
    let alGetBufferPtrSOFT = proc_address!(c"alGetBufferPtrSOFT", ALGETBUFFERPTRSOFT);
    let alGetBuffer3PtrSOFT = proc_address!(c"alGetBuffer3PtrSOFT", ALGETBUFFER3PTRSOFT);
    let alGetBufferPtrvSOFT = proc_address!(c"alGetBufferPtrvSOFT", ALGETBUFFERPTRSOFT);

    match CALLBACKBUFFER.set(CallbackBuffer {
        alBufferCallbackSOFT,
        alGetBufferPtrSOFT,
        alGetBuffer3PtrSOFT,
        alGetBufferPtrvSOFT,
    }) {
        Ok(()) => Ok(()),
        Err(_) => anyhow::bail!("failed to set CALLBACKBUFFER"),
    }
}

unsafe extern "C" fn callback_c<F>(
    userptr: *mut ALvoid,
    sampledata: *mut ALvoid,
    numbytes: ALsizei,
) -> ALsizei
where
    F: FnMut(&mut [u8]) -> usize + Send,
{
    let sampledata = sampledata as *mut u8;
    let ptr = userptr.cast::<F>();
    unsafe {
        let data = std::slice::from_raw_parts_mut(sampledata, numbytes as usize);
        (*ptr)(data) as ALsizei
    }
}

pub fn callback_buffer<F>(buffer: Buffer, format: ALenum, freq: usize, callback: F) -> Result<()>
where
    F: FnMut(&mut [u8]) -> usize + Send,
{
    let cb = CALLBACKBUFFER
        .get()
        .context("AL_SOFT_callback_buffer not initialized!")?;
    let ptr = Box::into_raw(Box::new(callback));
    unsafe {
        (cb.alBufferCallbackSOFT)(
            buffer.raw(),
            format,
            freq as ALsizei,
            callback_c::<F>,
            ptr.cast(),
        );
    }
    Ok(())
}

static CALLBACKBUFFER: OnceLock<CallbackBuffer> = OnceLock::new();

pub fn supported() -> bool {
    is_extension_present(c"AL_SOFT_callback_buffer")
}
