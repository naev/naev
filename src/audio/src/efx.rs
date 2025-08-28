use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;

use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use std::ffi::CStr;

pub const ALC_EXT_EFX_NAME: &CStr = c"ALC_EXT_EFX";

pub const ALC_EFX_MAJOR_VERSION: ALenum = 0x20001;
pub const ALC_EFX_MINOR_VERSION: ALenum = 0x20002;
pub const ALC_MAX_AUXILIARY_SENDS: ALenum = 0x20003;
// Listener Properties
pub const AL_METERS_PER_UNIT: ALenum = 0x20004;
// Source Properties
pub const AL_DIRECT_FILTER: ALenum = 0x20005;
pub const AL_AUXILIARY_SEND_FILTER: ALenum = 0x20006;
pub const AL_AIR_ABSORPTION_FACTOR: ALenum = 0x20007;
pub const AL_ROOM_ROLLOFF_FACTOR: ALenum = 0x20008;
pub const AL_CONE_OUTER_GAINHF: ALenum = 0x20009;
pub const AL_DIRECT_FILTER_GAINHF_AUTO: ALenum = 0x2000A;
pub const AL_AUXILIARY_SEND_FILTER_GAIN_AUTO: ALenum = 0x2000B;
pub const AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO: ALenum = 0x2000C;

pub const AL_EFFECT_TYPE: ALenum = 0x8001;
// Effect Types
pub const AL_EFFECT_NULL: ALenum = 0x0000;
pub const AL_EFFECT_REVERB: ALenum = 0x0001;
pub const AL_EFFECT_CHORUS: ALenum = 0x0002;
pub const AL_EFFECT_DISTORTION: ALenum = 0x0003;
pub const AL_EFFECT_ECHO: ALenum = 0x0004;
pub const AL_EFFECT_FLANGER: ALenum = 0x0005;
pub const AL_EFFECT_FREQUENCY_SHIFTER: ALenum = 0x0006;
pub const AL_EFFECT_VOCAL_MORPHER: ALenum = 0x0007;
pub const AL_EFFECT_PITCH_SHIFTER: ALenum = 0x0008;
pub const AL_EFFECT_RING_MODULATOR: ALenum = 0x0009;
pub const AL_EFFECT_AUTOWAH: ALenum = 0x000A;
pub const AL_EFFECT_COMPRESSOR: ALenum = 0x000B;
pub const AL_EFFECT_EQUALIZER: ALenum = 0x000C;
pub const AL_EFFECT_EAXREVERB: ALenum = 0x8000;
// Effect Slot Properties
pub const AL_EFFECTSLOT_EFFECT: ALenum = 0x0001;
pub const AL_EFFECTSLOT_GAIN: ALenum = 0x0002;
pub const AL_EFFECTSLOT_AUXILIARY_SEND_AUTO: ALenum = 0x0003;
// Echo Effect Parameters
pub const AL_ECHO_DELAY: ALenum = 0x0001;
pub const AL_ECHO_LRDELAY: ALenum = 0x0002;
pub const AL_ECHO_DAMPING: ALenum = 0x0003;
pub const AL_ECHO_FEEDBACK: ALenum = 0x0004;
pub const AL_ECHO_SPREAD: ALenum = 0x0005;
// Filter Types, used with the AL_FILTER_TYPE
pub const AL_FILTER_NULL: ALenum = 0x0000;
pub const AL_FILTER_LOWPASS: ALenum = 0x0001;
pub const AL_FILTER_HIGHPASS: ALenum = 0x0002;
pub const AL_FILTER_BANDPASS: ALenum = 0x0003;

#[allow(non_snake_case)]
pub struct Efx {
    pub version: (i32, i32),
    pub direct_slot: AuxiliaryEffectSlot,
    pub reverb: Effect,
    pub echo: Effect,

    // Efx C API
    pub alGenAuxiliaryEffectSlots:
        unsafe extern "C" fn(n: ALsizei, auxiliaryeffectslots: *const ALuint) -> *mut ALvoid,
    pub alDeleteAuxiliaryEffectSlots:
        unsafe extern "C" fn(n: ALsizei, auxiliaryeffectslots: *const ALuint),
    pub alIsAuxiliaryEffectSlot: unsafe extern "C" fn(auxiliaryeffectslot: ALuint),
    pub alAuxiliaryEffectSloti:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: ALint),
    pub alAuxiliaryEffectSlotiv:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALint),
    pub alAuxiliaryEffectSlotf:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: ALfloat),
    pub alAuxiliaryEffectSlotfv:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALfloat),
    pub alGetAuxiliaryEffectSloti:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALint),
    pub alGetAuxiliaryEffectSlotiv:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALint),
    pub alGetAuxiliaryEffectSlotf:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALfloat),
    pub alGetAuxiliaryEffectSlotfv:
        unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALfloat),
    pub alGenFilters: unsafe extern "C" fn(n: ALsizei, filters: *const ALuint),
    pub alDeleteFilters: unsafe extern "C" fn(n: ALsizei, filters: *const ALuint),
    pub alFilteri: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALint),
    pub alFilteriv: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALint),
    pub alFilterf: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALfloat),
    pub alFilterfv: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALfloat),
    pub alGenEffects: unsafe extern "C" fn(n: ALsizei, effects: *const ALuint),
    pub alDeleteEffects: unsafe extern "C" fn(n: ALsizei, effects: *const ALuint),
    pub alEffecti: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALint),
    pub alEffectiv: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALint),
    pub alEffectf: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALfloat),
    pub alEffectfv: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALfloat),
}
impl Efx {
    #[allow(non_snake_case)]
    pub fn new(device: &al::Device) -> Result<Efx> {
        let version = (
            device.get_parameter_i32(ALC_EFX_MAJOR_VERSION),
            device.get_parameter_i32(ALC_EFX_MINOR_VERSION),
        );

        macro_rules! proc_address {
            ($func: literal) => {{
                let val = unsafe { alGetProcAddress($func.as_ptr()) };
                if val.is_null() {
                    warn!(
                        "unable to load proc address for '{}'",
                        $func.to_string_lossy()
                    );
                }
                unsafe { std::mem::transmute(val) }
            }};
        }

        let alGenAuxiliaryEffectSlots = proc_address!(c"alGenAuxiliaryEffectSlots");
        let alDeleteAuxiliaryEffectSlots = proc_address!(c"alDeleteAuxiliaryEffectSlots");
        let alIsAuxiliaryEffectSlot = proc_address!(c"alIsAuxiliaryEffectSlot");
        let alAuxiliaryEffectSloti = proc_address!(c"alAuxiliaryEffectSloti");
        let alAuxiliaryEffectSlotiv = proc_address!(c"alAuxiliaryEffectSlotiv");
        let alAuxiliaryEffectSlotf = proc_address!(c"alAuxiliaryEffectSlotf");
        let alAuxiliaryEffectSlotfv = proc_address!(c"alAuxiliaryEffectSlotfv");
        let alGetAuxiliaryEffectSloti = proc_address!(c"alGetAuxiliaryEffectSloti");
        let alGetAuxiliaryEffectSlotiv = proc_address!(c"alGetAuxiliaryEffectSlotiv");
        let alGetAuxiliaryEffectSlotf = proc_address!(c"alGetAuxiliaryEffectSlotf");
        let alGetAuxiliaryEffectSlotfv = proc_address!(c"alGetAuxiliaryEffectSlotfv");
        let alGenFilters = proc_address!(c"alGenFilters");
        let alDeleteFilters = proc_address!(c"alDeleteFilters");
        let alFilteri = proc_address!(c"alFilteri");
        let alFilteriv = proc_address!(c"alFilteriv");
        let alFilterf = proc_address!(c"alFilterf");
        let alFilterfv = proc_address!(c"alFilterfv");
        let alGenEffects = proc_address!(c"alGenEffects");
        let alDeleteEffects = proc_address!(c"alDeleteEffects");
        //let alEffecti:  = proc_address!( c"alEffecti" );
        let alEffecti: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALint) =
            proc_address!(c"alEffecti");
        let alEffectiv = proc_address!(c"alEffectiv");
        //let alEffectf = proc_address!( c"alEffectf" );
        let alEffectf: unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALfloat) =
            proc_address!(c"alEffectf");
        let alEffectfv = proc_address!(c"alEffectfv");

        fn new_auxiliary_effect_slot(
            alGenAuxiliaryEffectSlots: unsafe extern "C" fn(
                n: ALsizei,
                auxiliaryeffectslots: *const ALuint,
            ) -> *mut ALvoid,
        ) -> Result<AuxiliaryEffectSlot> {
            let id: ALuint = 0;
            unsafe { alGenAuxiliaryEffectSlots(1, &id) };
            let id = match std::num::NonZero::new(id) {
                Some(v) => v,
                None => anyhow::bail!("failed to create Efx auxiliary effect slot"),
            };
            Ok(AuxiliaryEffectSlot(id))
        }
        let direct_slot = new_auxiliary_effect_slot(alGenAuxiliaryEffectSlots)?;

        fn new_effect(
            alGenEffects: unsafe extern "C" fn(n: ALsizei, effects: *const ALuint),
        ) -> Result<Effect> {
            let id: ALuint = 0;
            unsafe { alGenEffects(1, &id) };
            let id = match std::num::NonZero::new(id) {
                Some(v) => v,
                None => anyhow::bail!("failed to create Efx effect"),
            };
            Ok(Effect(id))
        }
        let reverb = new_effect(alGenEffects)?;
        let echo = new_effect(alGenEffects)?;
        unsafe {
            alEffecti(reverb.0.get(), AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            let e = echo.0.get();
            alEffecti(e, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
            alEffectf(e, AL_ECHO_DELAY, 0.207);
            alListenerf(AL_METERS_PER_UNIT, 5.);
        }

        Ok(Efx {
            version,
            direct_slot,
            reverb,
            echo,

            // API
            alGenAuxiliaryEffectSlots,
            alDeleteAuxiliaryEffectSlots,
            alIsAuxiliaryEffectSlot,
            alAuxiliaryEffectSloti,
            alAuxiliaryEffectSlotiv,
            alAuxiliaryEffectSlotf,
            alAuxiliaryEffectSlotfv,
            alGetAuxiliaryEffectSloti,
            alGetAuxiliaryEffectSlotiv,
            alGetAuxiliaryEffectSlotf,
            alGetAuxiliaryEffectSlotfv,
            alGenFilters,
            alDeleteFilters,
            alFilteri,
            alFilteriv,
            alFilterf,
            alFilterfv,
            alGenEffects,
            alDeleteEffects,
            alEffecti,
            alEffectiv,
            alEffectf,
            alEffectfv,
        })
    }

    pub fn new_effect(&'static self) -> Result<Effect> {
        let id: ALuint = 0;
        unsafe { (self.alGenEffects)(1, &id) };
        let id = match std::num::NonZero::new(id) {
            Some(v) => v,
            None => anyhow::bail!("failed to create Efx effect"),
        };
        Ok(Effect(id))
    }
}

pub struct AuxiliaryEffectSlot(pub std::num::NonZero<ALuint>);
impl Drop for AuxiliaryEffectSlot {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteAuxiliaryEffectSlot(self.0));
    }
}

pub struct Filter(pub std::num::NonZero<ALuint>);
impl Drop for Filter {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteFilter(self.0));
    }
}

pub struct Effect(pub std::num::NonZero<ALuint>);
impl Drop for Effect {
    fn drop(&mut self) {
        crate::message_push(crate::Message::DeleteEffect(self.0));
    }
}
