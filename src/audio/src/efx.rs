#![allow(clippy::upper_case_acronyms)]
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::*;

use anyhow::Result;
use log::warn;
use std::ffi::CStr;
use std::sync::OnceLock;

pub const ALC_EXT_EFX_NAME: &CStr = c"ALC_EXT_EFX";

pub const AL_EFFECTSLOT_NULL: ALenum = 0x0000;

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
// Reverb effect parameters
pub const AL_REVERB_DENSITY: ALenum = 0x0001;
pub const AL_REVERB_DIFFUSION: ALenum = 0x0002;
pub const AL_REVERB_GAIN: ALenum = 0x0003;
pub const AL_REVERB_GAINHF: ALenum = 0x0004;
pub const AL_REVERB_DECAY_TIME: ALenum = 0x0005;
pub const AL_REVERB_DECAY_HFRATIO: ALenum = 0x0006;
pub const AL_REVERB_REFLECTIONS_GAIN: ALenum = 0x0007;
pub const AL_REVERB_REFLECTIONS_DELAY: ALenum = 0x0008;
pub const AL_REVERB_LATE_REVERB_GAIN: ALenum = 0x0009;
pub const AL_REVERB_LATE_REVERB_DELAY: ALenum = 0x000A;
pub const AL_REVERB_AIR_ABSORPTION_GAINHF: ALenum = 0x000B;
pub const AL_REVERB_ROOM_ROLLOFF_FACTOR: ALenum = 0x000C;
pub const AL_REVERB_DECAY_HFLIMIT: ALenum = 0x000D;
// EAX Reverb effect parameters
pub const AL_EAXREVERB_DENSITY: ALenum = 0x0001;
pub const AL_EAXREVERB_DIFFUSION: ALenum = 0x0002;
pub const AL_EAXREVERB_GAIN: ALenum = 0x0003;
pub const AL_EAXREVERB_GAINHF: ALenum = 0x0004;
pub const AL_EAXREVERB_GAINLF: ALenum = 0x0005;
pub const AL_EAXREVERB_DECAY_TIME: ALenum = 0x0006;
pub const AL_EAXREVERB_DECAY_HFRATIO: ALenum = 0x0007;
pub const AL_EAXREVERB_DECAY_LFRATIO: ALenum = 0x0008;
pub const AL_EAXREVERB_REFLECTIONS_GAIN: ALenum = 0x0009;
pub const AL_EAXREVERB_REFLECTIONS_DELAY: ALenum = 0x000A;
pub const AL_EAXREVERB_REFLECTIONS_PAN: ALenum = 0x000B;
pub const AL_EAXREVERB_LATE_REVERB_GAIN: ALenum = 0x000C;
pub const AL_EAXREVERB_LATE_REVERB_DELAY: ALenum = 0x000D;
pub const AL_EAXREVERB_LATE_REVERB_PAN: ALenum = 0x000E;
pub const AL_EAXREVERB_ECHO_TIME: ALenum = 0x000F;
pub const AL_EAXREVERB_ECHO_DEPTH: ALenum = 0x0010;
pub const AL_EAXREVERB_MODULATION_TIME: ALenum = 0x0011;
pub const AL_EAXREVERB_MODULATION_DEPTH: ALenum = 0x0012;
pub const AL_EAXREVERB_AIR_ABSORPTION_GAINHF: ALenum = 0x0013;
pub const AL_EAXREVERB_HFREFERENCE: ALenum = 0x0014;
pub const AL_EAXREVERB_LFREFERENCE: ALenum = 0x0015;
pub const AL_EAXREVERB_ROOM_ROLLOFF_FACTOR: ALenum = 0x0016;
pub const AL_EAXREVERB_DECAY_HFLIMIT: ALenum = 0x0017;
// Chorus effect parameters
pub const AL_CHORUS_WAVEFORM: ALenum = 0x0001;
pub const AL_CHORUS_PHASE: ALenum = 0x0002;
pub const AL_CHORUS_RATE: ALenum = 0x0003;
pub const AL_CHORUS_DEPTH: ALenum = 0x0004;
pub const AL_CHORUS_FEEDBACK: ALenum = 0x0005;
pub const AL_CHORUS_DELAY: ALenum = 0x0006;
// Distortion effect parameters
pub const AL_DISTORTION_EDGE: ALenum = 0x0001;
pub const AL_DISTORTION_GAIN: ALenum = 0x0002;
pub const AL_DISTORTION_LOWPASS_CUTOFF: ALenum = 0x0003;
pub const AL_DISTORTION_EQCENTER: ALenum = 0x0004;
pub const AL_DISTORTION_EQBANDWIDTH: ALenum = 0x0005;
// Echo effect parameters
pub const AL_ECHO_DELAY: ALenum = 0x0001;
pub const AL_ECHO_LRDELAY: ALenum = 0x0002;
pub const AL_ECHO_DAMPING: ALenum = 0x0003;
pub const AL_ECHO_FEEDBACK: ALenum = 0x0004;
pub const AL_ECHO_SPREAD: ALenum = 0x0005;
// Flanger effect parameters
pub const AL_FLANGER_WAVEFORM: ALenum = 0x0001;
pub const AL_FLANGER_PHASE: ALenum = 0x0002;
pub const AL_FLANGER_RATE: ALenum = 0x0003;
pub const AL_FLANGER_DEPTH: ALenum = 0x0004;
pub const AL_FLANGER_FEEDBACK: ALenum = 0x0005;
pub const AL_FLANGER_DELAY: ALenum = 0x0006;
// Frequency shifter effect parameters
pub const AL_FREQUENCY_SHIFTER_FREQUENCY: ALenum = 0x0001;
pub const AL_FREQUENCY_SHIFTER_LEFT_DIRECTION: ALenum = 0x0002;
pub const AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION: ALenum = 0x0003;
// Vocal morpher effect parameters
pub const AL_VOCAL_MORPHER_PHONEMEA: ALenum = 0x0001;
pub const AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING: ALenum = 0x0002;
pub const AL_VOCAL_MORPHER_PHONEMEB: ALenum = 0x0003;
pub const AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING: ALenum = 0x0004;
pub const AL_VOCAL_MORPHER_WAVEFORM: ALenum = 0x0005;
pub const AL_VOCAL_MORPHER_RATE: ALenum = 0x0006;
// Pitchshifter effect parameters
pub const AL_PITCH_SHIFTER_COARSE_TUNE: ALenum = 0x0001;
pub const AL_PITCH_SHIFTER_FINE_TUNE: ALenum = 0x0002;
// Ringmodulator effect parameters
pub const AL_RING_MODULATOR_FREQUENCY: ALenum = 0x0001;
pub const AL_RING_MODULATOR_HIGHPASS_CUTOFF: ALenum = 0x0002;
pub const AL_RING_MODULATOR_WAVEFORM: ALenum = 0x0003;
// Autowah effect parameters
pub const AL_AUTOWAH_ATTACK_TIME: ALenum = 0x0001;
pub const AL_AUTOWAH_RELEASE_TIME: ALenum = 0x0002;
pub const AL_AUTOWAH_RESONANCE: ALenum = 0x0003;
pub const AL_AUTOWAH_PEAK_GAIN: ALenum = 0x0004;
// Compressor effect parameters
pub const AL_COMPRESSOR_ONOFF: ALenum = 0x0001;
// Equalizer effect parameters
pub const AL_EQUALIZER_LOW_GAIN: ALenum = 0x0001;
pub const AL_EQUALIZER_LOW_CUTOFF: ALenum = 0x0002;
pub const AL_EQUALIZER_MID1_GAIN: ALenum = 0x0003;
pub const AL_EQUALIZER_MID1_CENTER: ALenum = 0x0004;
pub const AL_EQUALIZER_MID1_WIDTH: ALenum = 0x0005;
pub const AL_EQUALIZER_MID2_GAIN: ALenum = 0x0006;
pub const AL_EQUALIZER_MID2_CENTER: ALenum = 0x0007;
pub const AL_EQUALIZER_MID2_WIDTH: ALenum = 0x0008;
pub const AL_EQUALIZER_HIGH_GAIN: ALenum = 0x0009;
pub const AL_EQUALIZER_HIGH_CUTOFF: ALenum = 0x000A;
// Filter Types, used with the AL_FILTER_TYPE
pub const AL_FILTER_NULL: ALenum = 0x0000;
pub const AL_FILTER_LOWPASS: ALenum = 0x0001;
pub const AL_FILTER_HIGHPASS: ALenum = 0x0002;
pub const AL_FILTER_BANDPASS: ALenum = 0x0003;

pub type ALGENAUXILIARYEFFECTSLOTS =
    unsafe extern "C" fn(n: ALsizei, auxiliaryeffectslots: *mut ALuint) -> *mut ALvoid;
pub type ALDELETEAUXILIARYEFFECTSLOTS =
    unsafe extern "C" fn(n: ALsizei, auxiliaryeffectslots: *const ALuint);
pub type ALISAUXILIARYEFFECTSLOT = unsafe extern "C" fn(auxiliaryeffectslot: ALuint);
pub type ALAUXILIARYEFFECTSLOTI =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: ALint);
pub type ALAUXILIARYEFFECTSLOTIV =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALint);
pub type ALAUXILIARYEFFECTSLOTF =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: ALfloat);
pub type ALAUXILIARYEFFECTSLOTFV =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *mut ALfloat);
pub type ALGETAUXILIARYEFFECTSLOTI =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *mut ALint);
pub type ALGETAUXILIARYEFFECTSLOTIV =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *mut ALint);
pub type ALGETAUXILIARYEFFECTSLOTF =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALfloat);
pub type ALGETAUXILIARYEFFECTSLOTFV =
    unsafe extern "C" fn(auxiliaryeffectslot: ALuint, param: ALenum, value: *const ALfloat);
pub type ALGENFILTERS = unsafe extern "C" fn(n: ALsizei, filters: *mut ALuint);
pub type ALDELETEFILTERS = unsafe extern "C" fn(n: ALsizei, filters: *const ALuint);
pub type ALFILTERI = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALint);
pub type ALFILTERIV = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALint);
pub type ALFILTERF = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALfloat);
pub type ALFILTERFV = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALfloat);
pub type ALGENEFFECTS = unsafe extern "C" fn(n: ALsizei, effects: *mut ALuint);
pub type ALDELETEEFFECTS = unsafe extern "C" fn(n: ALsizei, effects: *const ALuint);
pub type ALEFFECTI = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALint);
pub type ALEFFECTIV = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALint);
pub type ALEFFECTF = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: ALfloat);
pub type ALEFFECTFV = unsafe extern "C" fn(filter: ALuint, param: ALenum, value: *const ALfloat);

#[allow(non_snake_case)]
pub struct Efx {
    pub version: (i32, i32),
    pub direct_slot: AuxiliaryEffectSlot,
    pub reverb: Effect,
    pub echo: Effect,

    // Efx C API
    pub alGenAuxiliaryEffectSlots: ALGENAUXILIARYEFFECTSLOTS,
    pub alDeleteAuxiliaryEffectSlots: ALDELETEAUXILIARYEFFECTSLOTS,
    pub alIsAuxiliaryEffectSlot: ALISAUXILIARYEFFECTSLOT,
    pub alAuxiliaryEffectSloti: ALAUXILIARYEFFECTSLOTI,
    pub alAuxiliaryEffectSlotiv: ALAUXILIARYEFFECTSLOTIV,
    pub alAuxiliaryEffectSlotf: ALAUXILIARYEFFECTSLOTF,
    pub alAuxiliaryEffectSlotfv: ALAUXILIARYEFFECTSLOTFV,
    pub alGetAuxiliaryEffectSloti: ALGETAUXILIARYEFFECTSLOTI,
    pub alGetAuxiliaryEffectSlotiv: ALGETAUXILIARYEFFECTSLOTIV,
    pub alGetAuxiliaryEffectSlotf: ALGETAUXILIARYEFFECTSLOTF,
    pub alGetAuxiliaryEffectSlotfv: ALGETAUXILIARYEFFECTSLOTFV,
    pub alGenFilters: ALGENFILTERS,
    pub alDeleteFilters: ALDELETEFILTERS,
    pub alFilteri: ALFILTERI,
    pub alFilteriv: ALFILTERIV,
    pub alFilterf: ALFILTERF,
    pub alFilterfv: ALFILTERFV,
    pub alGenEffects: ALGENEFFECTS,
    pub alDeleteEffects: ALDELETEEFFECTS,
    pub alEffecti: ALEFFECTI,
    pub alEffectiv: ALEFFECTIV,
    pub alEffectf: ALEFFECTF,
    pub alEffectfv: ALEFFECTFV,
}
impl Efx {
    #[allow(non_snake_case)]
    pub fn init(device: &al::Device) -> Result<()> {
        let version = (
            device.get_parameter_i32(ALC_EFX_MAJOR_VERSION),
            device.get_parameter_i32(ALC_EFX_MINOR_VERSION),
        );

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

        let alGenAuxiliaryEffectSlots =
            proc_address!(c"alGenAuxiliaryEffectSlots", ALGENAUXILIARYEFFECTSLOTS);
        let alDeleteAuxiliaryEffectSlots = proc_address!(
            c"alDeleteAuxiliaryEffectSlots",
            ALDELETEAUXILIARYEFFECTSLOTS
        );
        let alIsAuxiliaryEffectSlot =
            proc_address!(c"alIsAuxiliaryEffectSlot", ALISAUXILIARYEFFECTSLOT);
        let alAuxiliaryEffectSloti =
            proc_address!(c"alAuxiliaryEffectSloti", ALAUXILIARYEFFECTSLOTI);
        let alAuxiliaryEffectSlotiv =
            proc_address!(c"alAuxiliaryEffectSlotiv", ALAUXILIARYEFFECTSLOTIV);
        let alAuxiliaryEffectSlotf =
            proc_address!(c"alAuxiliaryEffectSlotf", ALAUXILIARYEFFECTSLOTF);
        let alAuxiliaryEffectSlotfv =
            proc_address!(c"alAuxiliaryEffectSlotfv", ALAUXILIARYEFFECTSLOTFV);
        let alGetAuxiliaryEffectSloti =
            proc_address!(c"alGetAuxiliaryEffectSloti", ALGETAUXILIARYEFFECTSLOTI);
        let alGetAuxiliaryEffectSlotiv =
            proc_address!(c"alGetAuxiliaryEffectSlotiv", ALGETAUXILIARYEFFECTSLOTIV);
        let alGetAuxiliaryEffectSlotf =
            proc_address!(c"alGetAuxiliaryEffectSlotf", ALGETAUXILIARYEFFECTSLOTF);
        let alGetAuxiliaryEffectSlotfv =
            proc_address!(c"alGetAuxiliaryEffectSlotfv", ALGETAUXILIARYEFFECTSLOTFV);
        let alGenFilters = proc_address!(c"alGenFilters", ALGENFILTERS);
        let alDeleteFilters = proc_address!(c"alDeleteFilters", ALDELETEFILTERS);
        let alFilteri = proc_address!(c"alFilteri", ALFILTERI);
        let alFilteriv = proc_address!(c"alFilteriv", ALFILTERIV);
        let alFilterf = proc_address!(c"alFilterf", ALFILTERF);
        let alFilterfv = proc_address!(c"alFilterfv", ALFILTERFV);
        let alGenEffects = proc_address!(c"alGenEffects", ALGENEFFECTS);
        let alDeleteEffects = proc_address!(c"alDeleteEffects", ALDELETEEFFECTS);
        let alEffecti = proc_address!(c"alEffecti", ALEFFECTI);
        let alEffectiv = proc_address!(c"alEffectiv", ALEFFECTIV);
        let alEffectf = proc_address!(c"alEffectf", ALEFFECTF);
        let alEffectfv = proc_address!(c"alEffectfv", ALEFFECTFV);

        fn new_auxiliary_effect_slot(
            alGenAuxiliaryEffectSlots: ALGENAUXILIARYEFFECTSLOTS,
        ) -> Result<AuxiliaryEffectSlot> {
            let mut id: ALuint = 0;
            unsafe { alGenAuxiliaryEffectSlots(1, &mut id) };
            Ok(AuxiliaryEffectSlot(id))
        }
        let direct_slot = new_auxiliary_effect_slot(alGenAuxiliaryEffectSlots)?;

        fn new_effect(alGenEffects: ALGENEFFECTS) -> Result<Effect> {
            let mut id: ALuint = 0;
            unsafe { alGenEffects(1, &mut id) };
            Ok(Effect(id))
        }
        let reverb = new_effect(alGenEffects)?;
        let echo = new_effect(alGenEffects)?;
        unsafe {
            alEffecti(reverb.0, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            let e = echo.0;
            alEffecti(e, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
            alEffectf(e, AL_ECHO_DELAY, 0.207);
            alListenerf(AL_METERS_PER_UNIT, 5.);
        }

        match EFX.set(Efx {
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
        }) {
            Ok(()) => Ok(()),
            Err(_) => {
                anyhow::bail!("failed to set EFX");
            }
        }
    }
}

pub struct AuxiliaryEffectSlot(pub ALuint);
impl AuxiliaryEffectSlot {
    pub fn new() -> Result<Self> {
        if let Some(efx) = EFX.get() {
            let mut id: ALuint = 0;
            unsafe { (efx.alGenAuxiliaryEffectSlots)(1, &mut id) };
            Ok(AuxiliaryEffectSlot(id))
        } else {
            anyhow::bail!("EFX not available")
        }
    }

    pub fn raw(&self) -> ALuint {
        self.0
    }

    pub fn parameter_i32(&self, param: ALenum, val: ALint) {
        if let Some(efx) = EFX.get() {
            unsafe { (efx.alAuxiliaryEffectSloti)(self.raw(), param, val) };
        }
    }

    pub fn parameter_f32(&self, param: ALenum, val: ALfloat) {
        if let Some(efx) = EFX.get() {
            unsafe { (efx.alAuxiliaryEffectSlotf)(self.raw(), param, val) };
        }
    }

    pub fn set_effect(&self, effect: Option<&Effect>) {
        self.parameter_i32(
            AL_EFFECTSLOT_EFFECT,
            match effect {
                Some(effect) => effect.raw() as ALint,
                None => AL_EFFECTSLOT_NULL,
            },
        )
    }
}
impl Drop for AuxiliaryEffectSlot {
    fn drop(&mut self) {
        if let Some(efx) = EFX.get() {
            unsafe {
                (efx.alDeleteAuxiliaryEffectSlots)(1, &self.raw());
            }
        }
    }
}

pub struct Filter(pub std::num::NonZero<ALuint>);
impl Filter {
    pub fn new() -> Result<Self> {
        if let Some(efx) = EFX.get() {
            let mut id: ALuint = 0;
            unsafe { (efx.alGenFilters)(1, &mut id) };
            let id = match std::num::NonZero::new(id) {
                Some(v) => v,
                None => anyhow::bail!("failed to create Efx filter"),
            };
            Ok(Filter(id))
        } else {
            anyhow::bail!("EFX not available")
        }
    }
}
impl Drop for Filter {
    fn drop(&mut self) {
        if let Some(efx) = EFX.get() {
            unsafe {
                (efx.alDeleteFilters)(1, &self.0.get());
            }
        }
    }
}

pub struct Effect(pub ALuint);
impl Effect {
    pub fn new() -> Result<Self> {
        if let Some(efx) = EFX.get() {
            let mut id: ALuint = 0;
            unsafe { (efx.alGenEffects)(1, &mut id) };
            Ok(Self(id))
        } else {
            anyhow::bail!("EFX not available")
        }
    }

    pub fn raw(&self) -> ALuint {
        self.0
    }

    pub fn parameter_i32(&self, param: ALenum, val: ALint) {
        if let Some(efx) = EFX.get() {
            unsafe { (efx.alEffecti)(self.raw(), param, val) };
        }
    }

    pub fn parameter_f32(&self, param: ALenum, val: ALfloat) {
        if let Some(efx) = EFX.get() {
            unsafe { (efx.alEffectf)(self.raw(), param, val) };
        }
    }
}
impl Drop for Effect {
    fn drop(&mut self) {
        if let Some(efx) = EFX.get() {
            unsafe {
                (efx.alDeleteEffects)(1, &self.0);
            }
        }
    }
}

pub static EFX: OnceLock<Efx> = OnceLock::new();
