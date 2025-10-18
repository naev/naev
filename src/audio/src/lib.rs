#![allow(dead_code, unused_imports)]
mod debug;
mod efx;
#[macro_use]
mod openal;
mod buffer_length_query;
mod events;
mod output_limiter;
mod source_spatialize;
use crate::buffer_length_query::consts::*;
use crate::efx::consts::*;
use crate::efx::*;
use crate::events::consts::*;
use crate::events::*;
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::*;
use crate::output_limiter::consts::*;
use crate::source_spatialize::consts::*;
use crate::source_spatialize::*;
use anyhow::Context;
use naev_core::utils::{binary_search_by_key_ref, sort_by_key_ref};
use std::sync::atomic::{AtomicU32, Ordering};
use thunderdome::Arena;

use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use mlua::{MetaMethod, UserData, UserDataMethods};
use nalgebra::Vector3;
use std::sync::{Arc, LazyLock, Weak};
#[cfg(not(debug_assertions))]
use std::sync::{Mutex, RwLock};
#[cfg(debug_assertions)]
use tracing_mutex::stdsync::{Mutex, RwLock};

const NUM_VOICES: usize = 64;
const REFERENCE_DISTANCE: f32 = 500.;
const MAX_DISTANCE: f32 = 25_000.;

struct LuaAudioEfx {
    name: String,
    effect: Effect,
    slot: AuxiliaryEffectSlot,
}
impl LuaAudioEfx {
    pub fn new(name: &str) -> Result<Self> {
        let effect = Effect::new()?;
        debug::object_label(debug::consts::AL_FILTER_EXT, effect.raw(), name);
        let slot = AuxiliaryEffectSlot::new()?;
        debug::object_label(
            debug::consts::AL_AUXILIARY_EFFECT_SLOT_EXT,
            slot.raw(),
            name,
        );
        Ok(Self {
            name: String::from(name),
            effect,
            slot,
        })
    }
}
static EFX_LIST: Mutex<Vec<LuaAudioEfx>> = Mutex::new(Vec::new());

#[derive(Clone, PartialEq, Copy, Eq, Debug)]
pub enum AudioType {
    Static,
    Stream,
}

#[derive(PartialEq, Debug)]
pub struct AudioBuffer {
    name: String,
    track_gain_db: Option<f32>,
    track_peak: Option<f32>,
    buffer: al::Buffer,
}
impl AudioBuffer {
    fn get_valid_path(path: &str) -> Option<String> {
        let ext = std::path::Path::new(path)
            .extension()
            .and_then(|s| s.to_str());
        match ext {
            Some(_) => Some(path.to_string()),
            None => {
                let mut npath = None;
                for ext in &["ogg", "flac", "wav"] {
                    let tpath = format!("{}.{}", path, ext);
                    if ndata::exists(&tpath) {
                        npath = Some(tpath);
                        break;
                    }
                }
                npath
            }
        }
    }

    fn from_path(path: &str) -> Result<Self> {
        use symphonia::core::audio::{AudioBuffer, AudioBufferRef, Channels, Signal};
        use symphonia::core::codecs::{CODEC_TYPE_NULL, Decoder, DecoderOptions};
        use symphonia::core::conv::{FromSample, IntoSample};
        use symphonia::core::errors::Error;
        use symphonia::core::formats::{FormatOptions, FormatReader};
        use symphonia::core::io::MediaSourceStream;
        use symphonia::core::meta::{MetadataOptions, StandardTagKey, Tag, Value};
        use symphonia::core::probe::Hint;
        use symphonia::core::sample::{Sample, SampleFormat};

        // Small wrapper for Mono/Stereo frames
        enum Frame<T> {
            Mono(T),
            Stereo(T, T),
        }

        fn load_frames_from_buffer_ref(buffer: &AudioBufferRef) -> Result<Vec<Frame<f32>>> {
            match buffer {
                AudioBufferRef::U8(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::U16(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::U24(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::U32(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::S8(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::S16(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::S24(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::S32(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::F32(buffer) => load_frames_from_buffer(buffer),
                AudioBufferRef::F64(buffer) => load_frames_from_buffer(buffer),
            }
        }

        fn load_frames_from_buffer<S: Sample>(buffer: &AudioBuffer<S>) -> Result<Vec<Frame<f32>>>
        where
            f32: FromSample<S>,
        {
            match buffer.spec().channels.count() {
                1 => Ok(buffer
                    .chan(0)
                    .iter()
                    .map(|sample| Frame::Mono((*sample).into_sample()))
                    .collect()),
                2 => Ok(buffer
                    .chan(0)
                    .iter()
                    .zip(buffer.chan(1).iter())
                    .map(|(left, right)| {
                        Frame::Stereo((*left).into_sample(), (*right).into_sample())
                    })
                    .collect()),
                _ => anyhow::bail!("Unsupported channel configuration"),
            }
        }

        // If no extension try to autodetect.
        let ext = std::path::Path::new(path)
            .extension()
            .and_then(|s| s.to_str());
        let path = Self::get_valid_path(path)
            .context(format!("No audio file matching '{}' found", path))?;
        let src = ndata::open(&path)?;

        // Load it up
        let codecs = &CODECS;
        let probe = symphonia::default::get_probe();
        let mss = MediaSourceStream::new(Box::new(src), Default::default());
        let mut hint = Hint::new();
        if let Some(ext) = ext {
            hint.with_extension(ext);
        }
        let mut format = probe
            .format(&hint, mss, &Default::default(), &Default::default())?
            .format;

        // Get replaygain information
        let (track_gain_db, track_peak) = {
            let mut track_gain_db = None;
            let mut track_peak = None;
            if let Some(md) = format.metadata().current() {
                for t in md.tags() {
                    fn tag_to_f32(t: &Tag) -> Result<f32> {
                        match &t.value {
                            Value::Float(val) => Ok(*val as f32),
                            // Strings can be like "+3.14 dB" or "0.4728732849"
                            Value::String(val) => Ok(match val.split_once(" ") {
                                Some(val) => val.0,
                                None => val,
                            }
                            .parse::<f32>()?),
                            _ => anyhow::bail!("tag is not a float"),
                        }
                    }
                    if let Some(key) = t.std_key {
                        match key {
                            StandardTagKey::ReplayGainTrackGain => match tag_to_f32(t) {
                                Ok(f) => {
                                    track_gain_db = Some(f);
                                }
                                Err(e) => {
                                    warn_err!(e);
                                }
                            },
                            StandardTagKey::ReplayGainTrackPeak => match tag_to_f32(t) {
                                Ok(f) => {
                                    track_peak = Some(f);
                                }
                                Err(e) => {
                                    warn_err!(e);
                                }
                            },
                            _ => (),
                        };
                    }
                }
            }
            (track_gain_db, track_peak)
        };

        let track = format.default_track().context("No default track")?;
        let track_id = track.id;

        let codec_params = &track.codec_params;
        let sample_rate = codec_params.sample_rate.context("Unknown sample rate")?;
        let mut decoder = codecs.make(codec_params, &Default::default())?;

        let channels = track.codec_params.channels.context("no channels")?;
        if !channels.contains(Channels::FRONT_LEFT) {
            anyhow::bail!("no mono channel");
        }
        let stereo = channels.contains(Channels::FRONT_RIGHT);

        let mut frames: Vec<Frame<f32>> = vec![];
        loop {
            // Get the next packet from the media format.
            let packet = match format.next_packet() {
                Ok(packet) => packet,
                Err(e) => match e {
                    symphonia::core::errors::Error::IoError(e) => {
                        if e.kind() == std::io::ErrorKind::UnexpectedEof
                            && e.to_string() == "end of stream"
                        {
                            break;
                        }
                        return Err(symphonia::core::errors::Error::IoError(e).into());
                    }
                    e => anyhow::bail!(e),
                },
            };

            // If the packet does not belong to the selected track, skip over it.
            if packet.track_id() == track_id {
                // Decode the packet into audio samples.
                let buffer = decoder.decode(&packet)?;
                frames.append(&mut load_frames_from_buffer_ref(&buffer)?);
            }
        }
        // Squish the frames together
        let mut data: Vec<f32> = match stereo {
            true => {
                use std::iter::once;
                frames
                    .iter()
                    .flat_map(|x| match x {
                        Frame::Mono(x) => once(*x).chain(once(*x)),
                        Frame::Stereo(l, r) => once(*l).chain(once(*r)),
                    })
                    .collect()
            }
            false => frames
                .iter()
                .map(|x| match x {
                    Frame::Mono(x) => *x,
                    Frame::Stereo(l, _) => *l,
                })
                .collect(),
        };
        // Filter function for decoded Ogg Vorbis streams taken from "vgfilter.c"
        if let Some(track_gain_db) = track_gain_db {
            let scale_factor = 10.0_f32.powf(track_gain_db / 20.0);
            let max_scale = 1.0 / track_peak.unwrap_or(1.0);
            if scale_factor > max_scale {
                for d in &mut data {
                    let mut cur_sample = *d * scale_factor;
                    // This is essentially the scaled hard-limiting algorithm
                    // It looks like the soft-knee to me
                    // I haven't found a better limiting algorithm yet...
                    if cur_sample < -0.5 {
                        cur_sample = ((cur_sample + 0.5) / (1.0 - 0.5)).tanh() * (1.0 - 0.5) - 0.5;
                    } else if cur_sample > 0.5 {
                        cur_sample = ((cur_sample - 0.5) / (1.0 - 0.5)).tanh() * (1.0 - 0.5) + 0.5;
                    }
                    *d = cur_sample;
                }
            } else {
                for d in &mut data {
                    *d *= scale_factor;
                }
            }
        }

        let buffer = al::Buffer::new()?;
        unsafe {
            alBufferData(
                buffer.raw(),
                match stereo {
                    true => AL_FORMAT_STEREO_FLOAT32,
                    false => AL_FORMAT_MONO_FLOAT32,
                },
                data.as_ptr() as *const ALvoid,
                (data.len() * std::mem::size_of::<f32>()) as i32,
                sample_rate as ALsizei,
            );
        }
        debug::object_label(debug::consts::AL_BUFFER_EXT, buffer.raw(), &path);
        Ok(Self {
            name: path,
            buffer,
            track_gain_db,
            track_peak,
        })
    }

    pub fn duration(&self, unit: AudioSeek) -> f32 {
        match unit {
            AudioSeek::Seconds => self.buffer.get_parameter_f32(AL_SEC_LENGTH_SOFT),
            AudioSeek::Samples => self.buffer.get_parameter_i32(AL_SAMPLE_LENGTH_SOFT) as f32,
        }
        /*
        let bytes = self.buffer.get_parameter_i32(AL_SIZE);
        let channels = self.buffer.get_parameter_i32(AL_CHANNELS);
        let bits = self.buffer.get_parameter_i32(AL_CHANNELS);
        let samples = (bytes * 8 / (channels * bits)) as f32;
        match unit {
            AudioSeek::Seconds => {
                let freq = self.buffer.get_parameter_i32(AL_FREQUENCY);
                samples / freq as f32
            }
            AudioSeek::Samples => samples,
        }
        */
    }

    pub fn get(name: &str) -> Option<Arc<Self>> {
        let name = match AudioBuffer::get_valid_path(name) {
            Some(name) => name,
            None => {
                return None;
            }
        };

        let buffers = AUDIO_BUFFER.lock().unwrap();
        for buf in buffers.iter() {
            if let Some(b) = buf.upgrade() {
                if b.name == name {
                    return Some(b);
                }
            }
        }
        None
    }

    pub fn get_or_try_load(name: &str) -> Result<Arc<Self>> {
        let name = AudioBuffer::get_valid_path(name)
            .context(format!("No audio file matching '{}' found", name))?;

        let mut buffers = AUDIO_BUFFER.lock().unwrap();
        for buf in buffers.iter() {
            if let Some(b) = buf.upgrade() {
                if b.name == name {
                    return Ok(b);
                }
            }
        }
        let data = Arc::new(Self::from_path(&name)?);
        buffers.push(Arc::downgrade(&data));
        Ok(data)
    }

    fn raw(&self) -> ALuint {
        self.buffer.raw()
    }
}
/// All the shared Static audio data (streaming is separate)
pub(crate) static AUDIO_BUFFER: LazyLock<Mutex<Vec<Weak<AudioBuffer>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));
/// Counter for how many buffers were destroyed
pub(crate) static GC_COUNTER: AtomicU32 = AtomicU32::new(0);
/// Number of destroyed buffers to start garbage collecting the cache
pub(crate) const GC_THRESHOLD: u32 = 128;

#[derive(Clone, PartialEq, Copy, Eq, Debug)]
pub enum AudioSeek {
    Seconds,
    Samples,
}

#[derive(Debug, PartialEq, Clone)]
pub enum AudioData {
    Buffer(Arc<AudioBuffer>),
}

#[derive(Debug)]
pub enum Audio {
    Static(AudioStatic),
}

#[derive(PartialEq, Debug)]
pub struct AudioStatic {
    source: al::Source,
    slot: Option<AuxiliaryEffectSlot>,
    data: Option<AudioData>,
    volume: f32,
    groupid: Option<thunderdome::Index>,
    ingame: bool,
}
impl AudioStatic {
    fn new(data: &Option<AudioData>) -> Result<Self> {
        match data {
            Some(AudioData::Buffer(buffer)) => Self::new_buffer(buffer),
            None => {
                let source = al::Source::new()?;
                Ok(AudioStatic {
                    source,
                    slot: None,
                    volume: 1.0,
                    data: None,
                    groupid: None,
                    ingame: false,
                })
            }
        }
    }

    fn new_buffer(buffer: &Arc<AudioBuffer>) -> Result<Self> {
        let source = al::Source::new()?;
        source.parameter_i32(AL_BUFFER, buffer.raw() as ALint);
        debug::object_label(debug::consts::AL_SOURCE_EXT, source.raw(), &buffer.name);
        Ok(AudioStatic {
            source,
            slot: None,
            volume: 1.0,
            data: Some(AudioData::Buffer(buffer.clone())),
            groupid: None,
            ingame: false,
        })
    }

    pub fn try_clone(&self) -> Result<Self> {
        let mut audio = Self::new(&self.data)?;
        audio.slot = None;
        audio.volume = self.volume;
        // TODO copy some other properties over and set volume
        Ok(audio)
    }
}

macro_rules! check_audio {
    ($self: ident) => {{
        match $self {
            Audio::Static(this) => {
                if this.data == None {
                    return Default::default();
                }
            }
        }
    }};
}
impl Audio {
    fn new(data: &Option<AudioData>) -> Result<Self> {
        Ok(Self::Static(AudioStatic::new(data)?))
    }

    fn new_buffer(buffer: &Arc<AudioBuffer>) -> Result<Self> {
        Ok(Self::Static(AudioStatic::new_buffer(buffer)?))
    }

    fn from_path(path: &str, _atype: AudioType) -> Result<Self> {
        // TODO streaming
        let buffer = Arc::new(AudioBuffer::from_path(path)?);
        Self::new_buffer(&buffer)
    }

    fn try_clone(&self) -> Result<Self> {
        match self {
            Self::Static(this) => Ok(Self::Static(this.try_clone()?)),
        }
    }

    /// Sets the sound to be in-game as opposed to a GUI or music track.
    pub fn set_ingame(&mut self) {
        check_audio!(self);

        match self {
            Self::Static(this) => {
                let v = &this.source;
                this.ingame = true;

                if HAS_AL_SOFT_SOURCE_SPATIALIZE.load(Ordering::Relaxed) {
                    v.parameter_i32(AL_SOURCE_SPATIALIZE_SOFT, AL_AUTO_SOFT);
                }
                v.parameter_f32(AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
                v.parameter_f32(AL_MAX_DISTANCE, MAX_DISTANCE);
                v.parameter_f32(AL_ROLLOFF_FACTOR, 1.);

                if let Some(efx) = EFX.get() {
                    v.parameter_3_i32(
                        AL_AUXILIARY_SEND_FILTER,
                        efx.direct_slot.raw() as i32,
                        0,
                        AL_FILTER_NULL,
                    );
                }
            }
        }
    }

    fn source(&self) -> &al::Source {
        match self {
            Self::Static(this) => &this.source,
        }
    }

    fn groupid(&self) -> Option<thunderdome::Index> {
        match self {
            Self::Static(this) => this.groupid,
        }
    }

    fn is_state(&self, state: ALenum) -> bool {
        check_audio!(self);
        self.source().get_parameter_i32(AL_SOURCE_STATE) == state
    }

    pub fn play(&self) {
        check_audio!(self);
        unsafe {
            alSourcePlay(self.source().raw());
        }
    }

    pub fn is_playing(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_PLAYING)
    }

    pub fn pause(&self) {
        check_audio!(self);
        unsafe {
            alSourcePause(self.source().raw());
        }
    }

    pub fn is_paused(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_PAUSED)
    }

    pub fn stop(&self) {
        check_audio!(self);
        unsafe {
            alSourceStop(self.source().raw());
        }
    }

    pub fn is_stopped(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_STOPPED)
    }

    pub fn rewind(&self) {
        check_audio!(self);
        unsafe {
            alSourceRewind(self.source().raw());
        }
    }

    pub fn seek(&self, offset: f32, unit: AudioSeek) {
        check_audio!(self);
        match unit {
            AudioSeek::Seconds => self.source().parameter_f32(AL_SEC_OFFSET, offset),
            AudioSeek::Samples => self.source().parameter_f32(AL_SAMPLE_OFFSET, offset),
        }
    }

    pub fn tell(&self, unit: AudioSeek) -> f32 {
        check_audio!(self);
        match unit {
            AudioSeek::Seconds => self.source().get_parameter_f32(AL_SEC_OFFSET),
            AudioSeek::Samples => self.source().get_parameter_f32(AL_SAMPLE_OFFSET),
        }
    }

    pub fn set_volume(&mut self, vol: f32) {
        check_audio!(self);
        let master = AUDIO.volume.read().unwrap().volume;
        self.source().parameter_f32(AL_GAIN, master * vol);
        match self {
            Self::Static(this) => {
                this.volume = vol;
            }
        }
    }

    pub fn set_volume_raw(&mut self, vol: f32) {
        check_audio!(self);
        self.source().parameter_f32(AL_GAIN, vol);
        match self {
            Self::Static(this) => {
                this.volume = vol;
            }
        }
    }

    pub fn set_gain(&self, vol: f32) {
        check_audio!(self);
        self.source().parameter_f32(AL_GAIN, vol);
    }

    pub fn volume(&self) -> f32 {
        check_audio!(self);
        match self {
            Self::Static(this) => this.volume,
        }
    }

    pub fn set_relative(&self, relative: bool) {
        check_audio!(self);
        self.source()
            .parameter_i32(AL_SOURCE_RELATIVE, relative as i32);
    }

    pub fn relative(&self) -> bool {
        check_audio!(self);
        self.source().get_parameter_i32(AL_SOURCE_RELATIVE) != 0
    }

    pub fn set_position(&self, pos: Vector3<f32>) {
        check_audio!(self);
        self.source()
            .parameter_3_f32(AL_POSITION, pos.x, pos.y, pos.z);
    }

    pub fn position(&self) -> Vector3<f32> {
        check_audio!(self);
        Vector3::from(self.source().get_parameter_3_f32(AL_POSITION))
    }

    pub fn set_velocity(&self, pos: Vector3<f32>) {
        check_audio!(self);
        self.source()
            .parameter_3_f32(AL_VELOCITY, pos.x, pos.y, pos.z);
    }

    pub fn velocity(&self) -> Vector3<f32> {
        check_audio!(self);
        Vector3::from(self.source().get_parameter_3_f32(AL_VELOCITY))
    }

    pub fn set_looping(&self, looping: bool) {
        check_audio!(self);
        self.source().parameter_i32(AL_LOOPING, looping as i32);
    }

    pub fn looping(&self) -> bool {
        check_audio!(self);
        self.source().get_parameter_i32(AL_LOOPING) != 0
    }

    pub fn set_pitch(&self, pitch: f32) {
        check_audio!(self);
        self.source().parameter_f32(AL_PITCH, pitch);
    }

    pub fn pitch(&self) -> f32 {
        check_audio!(self);
        self.source().get_parameter_f32(AL_PITCH)
    }

    pub fn set_attenuation_distances(&self, reference: f32, max: f32) {
        check_audio!(self);
        let src = self.source();
        src.parameter_f32(AL_REFERENCE_DISTANCE, reference);
        src.parameter_f32(AL_MAX_DISTANCE, max);
    }

    pub fn attenuation_distances(&self) -> (f32, f32) {
        check_audio!(self);
        let src = self.source();
        (
            src.get_parameter_f32(AL_REFERENCE_DISTANCE),
            src.get_parameter_f32(AL_MAX_DISTANCE),
        )
    }

    pub fn set_rolloff(&self, rolloff: f32) {
        check_audio!(self);
        self.source().parameter_f32(AL_ROLLOFF_FACTOR, rolloff);
    }

    pub fn rolloff(&self) -> f32 {
        check_audio!(self);
        self.source().get_parameter_f32(AL_ROLLOFF_FACTOR)
    }

    pub fn set_air_absorption_factor(&self, value: f32) {
        check_audio!(self);
        self.source().parameter_f32(AL_AIR_ABSORPTION_FACTOR, value);
    }
}

#[derive(Debug, Default)]
pub struct AudioGroup {
    max: usize,
    volume: f32,
    pitch: f32,
    speed_affects: bool,
    voices: Vec<AudioRef>,
}

#[derive(Clone, PartialEq, Copy, Debug)]
pub struct AudioVolume {
    /// Logarithmic volume
    volume: f32,
    /// Linear volume
    volume_lin: f32,
    /// Volume speed multiplier
    volume_speed: f32,
}
impl Default for AudioVolume {
    fn default() -> Self {
        Self::new()
    }
}
impl AudioVolume {
    pub fn new() -> Self {
        Self {
            volume: 1.,
            volume_lin: 1.,
            volume_speed: 1.,
        }
    }
}

enum Message {
    RefDropped(thunderdome::Index),
    SourceStopped(ALuint),
}
static MESSAGES: Mutex<Vec<Message>> = Mutex::new(Vec::new());

fn event_callback(event_type: ALenum, object: ALuint, param: ALuint, _message: &str) {
    // Can't call OpenAL stuff here
    if event_type == AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT {
        let param = param as i32;
        // object is the source ID
        // param is the source's new state
        if param == AL_STOPPED {
            MESSAGES
                .lock()
                .unwrap()
                .push(Message::SourceStopped(object));
        }
    }
}

pub struct AudioSystem {
    device: al::Device,
    context: al::Context,
    freq: i32,
    volume: RwLock<AudioVolume>,
    voices: Mutex<Arena<Audio>>,
    groups: Mutex<Arena<AudioGroup>>,
}
impl AudioSystem {
    pub fn new() -> Result<Self> {
        let device = al::Device::new(None)?;

        let mut attribs: Vec<ALint> = vec![ALC_MONO_SOURCES, 512, ALC_STEREO_SOURCES, 32];
        let mut has_debug = if cfg!(debug_assertions) {
            let debug = debug::supported(&device);
            if debug {
                attribs.push(debug::consts::ALC_CONTEXT_FLAGS_EXT);
                attribs.push(debug::consts::ALC_CONTEXT_DEBUG_BIT_EXT);
            } else {
                warn("ALC_EXT_debug not supported on device");
            }
            debug
        } else {
            false
        };

        let has_efx = match unsafe { naevc::conf.al_efx } {
            0 => false,
            _ => match efx::supported(&device) {
                true => {
                    attribs.push(ALC_MAX_AUXILIARY_SENDS);
                    attribs.push(4);
                    true
                }
                false => false,
            },
        };
        let has_output_limiter = output_limiter::supported(&device);
        if has_output_limiter {
            attribs.push(ALC_OUTPUT_LIMITER_SOFT);
            attribs.push(ALC_TRUE as i32);
        }
        attribs.push(0); // Has to be NULL terminated

        let context = al::Context::new(&device, &attribs)?;
        context.set_current()?;

        // Has to test after context creation
        let has_events = events::supported();
        if has_events {
            Events::init(event_callback)?;
            event_control(&[AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT], true);
        }

        let has_buffer_length_query = buffer_length_query::supported();

        // Check to see if output limiter is working
        if has_output_limiter
            && device.get_parameter_i32(ALC_OUTPUT_LIMITER_SOFT) != ALC_TRUE as i32
        {
            warn!("failed to set ALC_OUTPUT_LIMITER_SOFT");
        }
        // Check to see if debugging was enabled
        if has_debug {
            match debug::Debug::init() {
                Ok(()) => (),
                Err(e) => {
                    has_debug = false;
                    warn_err!(e);
                }
            }
        }

        // Get context information
        let freq = device.get_parameter_i32(ALC_FREQUENCY);

        // Set up the Efx
        if has_efx {
            match Efx::init(&device) {
                Ok(()) => (),
                Err(e) => {
                    warn_err!(e);
                }
            }
        }

        // Default global settings
        unsafe {
            alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        }

        let has_source_spatialize = source_spatialize::supported();

        debugx!(gettext("OpenAL started: {} Hz"), freq);
        let al_renderer = al::get_parameter_str(AL_RENDERER)?;
        if al_renderer != "OpenAL Soft" {
            warn!("Not using OpenAL Soft renderer! Things may catch on fire.");
        }
        debugx!(gettext("Renderer: {}"), &al_renderer);
        let al_version = al::get_parameter_str(AL_VERSION)?;
        debugx!(gettext("Version: {}"), &al_version);
        let mut extensions: Vec<String> = Vec::new();
        if has_debug {
            extensions.push("debug".to_string());
        }
        if let Some(efx) = EFX.get() {
            extensions.push(format!("EFX {}.{}", efx.version.0, efx.version.1));
        }
        if has_source_spatialize {
            extensions.push("source_spatialize".to_string());
        }
        if has_events {
            extensions.push("events".to_string());
        }
        if has_buffer_length_query {
            extensions.push("buffer_length_query".to_string());
        }
        if has_output_limiter {
            extensions.push("output_limiter".to_string());
        }
        debugx!(gettext("   with {}"), extensions.join(", "));
        debug!("");

        Ok(AudioSystem {
            device,
            context,
            volume: RwLock::new(AudioVolume::new()),
            freq,
            voices: Mutex::new(Default::default()),
            groups: Mutex::new(Default::default()),
        })
    }

    pub fn set_volume(&self, volume: f32) {
        let mut vol = self.volume.write().unwrap();
        vol.volume_lin = volume;
        if vol.volume_lin > 0.0 {
            vol.volume = 1.0 / 2.0_f32.powf((1.0 - volume) * 8.0);
        } else {
            vol.volume = 0.0;
        }
    }

    pub fn set_volume_speed(&self, speed: f32) {
        let mut vol = self.volume.write().unwrap();
        vol.volume_speed = speed;
        drop(vol);
    }

    pub fn set_air_absorption_factor(&self, factor: f32) {
        let voices = self.voices.lock().unwrap();
        for (_, v) in voices.iter() {
            v.set_air_absorption_factor(factor);
        }
    }

    pub fn play_buffer(&self, buf: &Arc<AudioBuffer>) -> Result<AudioRef> {
        let audio = Audio::new_buffer(buf)?;
        audio.play();
        Ok(AudioRef::from_audio(audio))
    }

    pub fn execute_messages(&self) {
        for m in MESSAGES.lock().unwrap().drain(..) {
            match m {
                Message::RefDropped(id) => {
                    AUDIO.voices.lock().unwrap().remove(id);
                }
                Message::SourceStopped(id) => {
                    // We always lock groups first
                    let mut groups = AUDIO.groups.lock().unwrap();
                    let mut voices = AUDIO.voices.lock().unwrap();
                    if let Some((vid, v)) = voices.iter().find(|(_, x)| x.source().raw() == id) {
                        // Remove from group too if it has one
                        if let Some(gid) = v.groupid() {
                            let group = &mut groups[gid];
                            if let Some(gvid) = group.voices.iter().position(|x| *x.0 == vid) {
                                group.voices.remove(gvid);
                            }
                        }
                        // Finally remove the voice
                        if let Some(voice) = voices.get(vid) {
                            match voice {
                                Audio::Static(voice) => {
                                    if voice.ingame {
                                        voices.remove(vid);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
pub static AUDIO: LazyLock<AudioSystem> = LazyLock::new(|| AudioSystem::new().unwrap());
pub static CODECS: LazyLock<symphonia::core::codecs::CodecRegistry> = LazyLock::new(|| {
    use symphonia::core::codecs;
    use symphonia_adapter_libopus::OpusDecoder;
    let mut codec_registry = codecs::CodecRegistry::new();
    symphonia::default::register_enabled_codecs(&mut codec_registry);
    codec_registry.register_all::<OpusDecoder>();
    codec_registry
});

pub fn init() -> Result<()> {
    let _ = &*AUDIO;
    Ok(())
}

#[derive(Debug, Clone, PartialEq, derive_more::From, mlua::FromLua)]
pub struct AudioRef(Arc<thunderdome::Index>);
impl Drop for AudioRef {
    fn drop(&mut self) {
        MESSAGES.lock().unwrap().push(Message::RefDropped(*self.0));
    }
}
impl AudioRef {
    fn new(data: &Option<AudioData>) -> Result<Self> {
        let audio = Audio::new(data)?;
        Ok(Arc::new(AUDIO.voices.lock().unwrap().insert(audio)).into())
    }

    fn new_buffer(buffer: &Arc<AudioBuffer>) -> Result<Self> {
        let audio = Audio::new_buffer(buffer)?;
        Ok(Arc::new(AUDIO.voices.lock().unwrap().insert(audio)).into())
    }

    fn from_path(path: &str, _atype: AudioType) -> Result<Self> {
        // TODO streaming
        let buffer = Arc::new(AudioBuffer::from_path(path)?);
        Self::new_buffer(&buffer)
    }

    fn from_audio(audio: Audio) -> Self {
        Arc::new(AUDIO.voices.lock().unwrap().insert(audio)).into()
    }

    fn try_clone(&self) -> Result<Self> {
        let mut voices = AUDIO.voices.lock().unwrap();
        let audio = match voices.get(*self.0) {
            Some(audio) => audio.try_clone()?,
            None => anyhow::bail!("Audio not found"),
        };
        Ok(Arc::new(voices.insert(audio)).into())
    }

    pub fn call<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&Audio) -> R,
    {
        let audio = AUDIO.voices.lock().unwrap();
        match audio.get(*self.0) {
            Some(audio) => Ok(f(audio)),
            None => anyhow::bail!("Audio not found"),
        }
    }

    pub fn call_mut<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&mut Audio) -> R,
    {
        let mut audio = AUDIO.voices.lock().unwrap();
        match audio.get_mut(*self.0) {
            Some(audio) => Ok(f(audio)),
            None => anyhow::bail!("Audio not found"),
        }
    }
}

/*
 * @brief Lua bindings to interact with audio.
 *
 * @luamod audio
 */
impl UserData for AudioRef {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Gets a string representation of an audio file.
         *
         *    @luatparam Audio audio Audio to get string representation of.
         *    @luatreturn string String representation of the audio.
         * @luafunc __tostring
         */
        methods.add_meta_method(MetaMethod::ToString, |_, this: &Self, ()| {
            Ok(this.call(|this| match this {
                Audio::Static(this) => format!(
                    "AudioStatic( {} )",
                    match &this.data {
                        Some(AudioData::Buffer(buffer)) => &buffer.name,
                        None => "NONE",
                    }
                ),
            })?)
        });
        /*
         * @brief Creates a new audio source.
         *
         *    @luatparam string|File data Data to load the audio from.
         *    @luatparam boolean streaming True if should be a streaming source instead of loaded
         *    entirely in memory.
         *    @luatreturn Audio New audio corresponding to the data.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_, (val, streaming): (String, bool)| -> mlua::Result<Self> {
                Ok(Self::from_path(
                    &val,
                    match streaming {
                        true => AudioType::Stream,
                        false => AudioType::Static,
                    },
                )?)
            },
        );
        /*
         * @brief Clones an existing audio source.
         *
         *    @luatparam Audio source Audio source to clone.
         *    @luatreturn Audio New audio corresponding to the data.
         * @luafunc clone
         */
        methods.add_method("clone", |_, this, ()| -> mlua::Result<AudioRef> {
            Ok(this.try_clone()?)
        });
        /*
         * @brief Plays a source.
         *
         *    @luatparam Audio source Source to play.
         * @luafunc play
         */
        methods.add_method("play", |_, this, ()| -> mlua::Result<()> {
            this.call(|this| {
                this.play();
            })?;
            Ok(())
        });
        /*
         * @brief Checks to see if a source is playing.
         *
         *    @luatparam Audio source Source to check to see if is playing.
         *    @luatreturn boolean Whether or not the source is playing.
         * @luafunc isPlaying
         */
        methods.add_method("isPlaying", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_playing())?)
        });
        /*
         * @brief Pauses a source.
         *
         *    @luatparam Audio source Source to pause.
         * @luafunc pause
         */
        methods.add_method("pause", |_, this, ()| -> mlua::Result<()> {
            this.call(|this| {
                this.pause();
            })?;
            Ok(())
        });
        /*
         * @brief Checks to see if a source is paused.
         *
         *    @luatparam Audio source Source to check to see if is paused.
         *    @luatreturn boolean Whether or not the source is paused.
         * @luafunc isPaused
         */
        methods.add_method("isPaused", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_paused())?)
        });
        /*
         * @brief Stops a source.
         *
         *    @luatparam Audio source Source to stop.
         * @luafunc stop
         */
        methods.add_method("stop", |_, this, ()| -> mlua::Result<()> {
            this.call(|this| {
                this.stop();
            })?;
            Ok(())
        });
        /*
         * @brief Checks to see if a source is stopped.
         *
         *    @luatparam Audio source Source to check to see if is stopped.
         *    @luatreturn boolean Whether or not the source is stopped.
         * @luafunc isStopped
         */
        methods.add_method("isStopped", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_stopped())?)
        });
        /*
         * @brief Rewinds a source.
         *
         *    @luatparam Audio source Source to rewind.
         * @luafunc rewind
         */
        methods.add_method("rewind", |_, this, ()| -> mlua::Result<()> {
            this.call(|this| {
                this.rewind();
            })?;
            Ok(())
        });
        /*
         * @brief Seeks a source.
         *
         *    @luatparam Audio source Source to seek.
         *    @luatparam number offset Offset to seek to.
         *    @luatparam boolean samples Whether or not to use samples as a seek unit instead of seconds.
         * @luafunc seek
         */
        methods.add_method(
            "seek",
            |_, this, (offset, samples): (f32, bool)| -> mlua::Result<()> {
                this.call(|this| {
                    this.seek(
                        offset,
                        match samples {
                            true => AudioSeek::Samples,
                            false => AudioSeek::Seconds,
                        },
                    );
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets the position of a source.
         *
         *    @luatparam Audio source Source to get position of.
         *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples"
         * indicating the type to report.
         *    @luatreturn number Offset of the source or -1 on error.
         * @luafunc tell
         */
        methods.add_method("tell", |_, this, samples: bool| -> mlua::Result<f32> {
            Ok(this.call(|this| {
                this.tell(match samples {
                    true => AudioSeek::Samples,
                    false => AudioSeek::Seconds,
                })
            })?)
        });
        /*
         * @brief Gets the length of a source.
         *
         *    @luatparam Audio source Source to get duration of.
         *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples"
         * indicating the type to report.
         *    @luatreturn number Duration of the source or -1 on error.
         * @luafunc getDuration
         */
        methods.add_method(
            "getDuration",
            |_, this, samples: bool| -> mlua::Result<f32> {
                Ok(this.call(|this| match this {
                    Audio::Static(this) => match &this.data {
                        Some(AudioData::Buffer(buffer)) => buffer.duration(match samples {
                            true => AudioSeek::Samples,
                            false => AudioSeek::Seconds,
                        }),
                        None => 0.0,
                    },
                })?)
            },
        );
        /*
         * @brief Sets the volume of a source.
         *
         *    @luatparam Audio source Source to set volume of.
         *    @luatparam number vol Volume to set the source to with 0.0 being silent
         * and 1.0 being full volume.
         *    @luatparam[opt=false] boolean ignorevol Don't modify volume based on
         * master.
         * @luafunc setVolume
         */
        methods.add_method(
            "setVolume",
            |_, this, (volume, ignoremaster): (f32, bool)| -> mlua::Result<()> {
                this.call_mut(|this| {
                    if ignoremaster {
                        this.set_volume_raw(volume)
                    } else {
                        this.set_volume(volume)
                    }
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets the volume of a source.
         *
         *    @luatparam[opt] Audio source Source to get volume of.
         *    @luatreturn number Volume the source is set to.
         * @luafunc getVolume
         */
        methods.add_method("getVolume", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.volume())?)
        });
        /*
         * @brief Sets whether a source is relative or not.
         *
         *    @luatparam boolean relative Whether or not to make the source relative or
         * not.
         * @luafunc setRelative
         */
        methods.add_method(
            "setRelative",
            |_, this, relative: bool| -> mlua::Result<()> {
                this.call_mut(|this| {
                    this.set_relative(relative);
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets whether a source is relative or not.
         *
         *    @luatreturn boolean relative Whether or not to the source is relative.
         * @luafunc isRelative
         */
        methods.add_method("isRelative", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.relative())?)
        });
        /*
         * @brief Sets the position of a source.
         *
         *    @luatparam Audio source Source to set position of.
         *    @luatparam number x X position.
         *    @luatparam number y Y position.
         *    @luatparam number z Z position.
         * @luafunc setPosition
         */
        methods.add_method(
            "setPosition",
            |_, this, (x, y, z): (f32, f32, f32)| -> mlua::Result<()> {
                let vec: Vector3<f32> = Vector3::new(x, y, z);
                this.call_mut(|this| {
                    this.set_position(vec);
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets the position of a source.
         *
         *    @luatparam Audio source Source to get position of.
         *    @luatreturn number X position.
         *    @luatreturn number Y position.
         *    @luatreturn number Z position.
         * @luafunc getPosition
         */
        methods.add_method(
            "getPosition",
            |_, this, ()| -> mlua::Result<(f32, f32, f32)> {
                let pos = this.call(|this| this.position())?;
                Ok((pos.x, pos.y, pos.z))
            },
        );
        /*
         * @brief Sets the velocity of a source.
         *
         *    @luatparam Audio source Source to set velocity of.
         *    @luatparam number x X velocity.
         *    @luatparam number y Y velocity.
         *    @luatparam number z Z velocity.
         * @luafunc setVelocity
         */
        methods.add_method(
            "setVelocity",
            |_, this, (x, y, z): (f32, f32, f32)| -> mlua::Result<()> {
                let vec: Vector3<f32> = Vector3::new(x, y, z);
                this.call_mut(|this| {
                    this.set_velocity(vec);
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets the velocity of a source.
         *
         *    @luatparam Audio source Source to get velocity of.
         *    @luatreturn number X velocity.
         *    @luatreturn number Y velocity.
         *    @luatreturn number Z velocity.
         * @luafunc getVelocity
         */
        methods.add_method(
            "getVelocity",
            |_, this, ()| -> mlua::Result<(f32, f32, f32)> {
                let vel = this.call(|this| this.velocity())?;
                Ok((vel.x, vel.y, vel.z))
            },
        );
        /*
         * @brief Sets a source to be looping or not.
         *
         *    @luatparam Audio source Source to set looping state of.
         *    @luatparam boolean enable Whether or not the source should be set to
         * looping.
         * @luafunc setLooping
         */
        methods.add_method("setLooping", |_, this, looping: bool| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.set_looping(looping);
            })?;
            Ok(())
        });
        /*
         * @brief Gets the looping state of a source.
         *
         *    @luatparam Audio source Source to get looping state of.
         *    @luatreturn boolean Whether or not the source is looping.
         * @luafunc isLooping
         */
        methods.add_method("isLooping", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.looping())?)
        });
        /*
         * @brief Sets the pitch of a source.
         *
         *    @luatparam Audio source Source to set pitch of.
         *    @luatparam number pitch Pitch to set the source to.
         * @luafunc setPitch
         */
        methods.add_method("setPitch", |_, this, pitch: f32| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.set_pitch(pitch);
            })?;
            Ok(())
        });
        /*
         * @brief Gets the pitch of a source.
         *
         *    @luatparam Audio source Source to get pitch of.
         *    @luatreturn number Pitch of the source.
         * @luafunc getPitch
         */
        methods.add_method("getPitch", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.pitch())?)
        });
        /*
         * @brief Sets the attenuation distances for the audio source.
         *
         *    @luatparam number ref Reference distance.
         *    @luatparam number max Maximum distance.
         * @luafunc setAttenuationDistances
         */
        methods.add_method(
            "setAttenuationDistances",
            |_, this, (reference, max): (f32, f32)| -> mlua::Result<()> {
                this.call_mut(|this| {
                    this.set_attenuation_distances(reference, max);
                })?;
                Ok(())
            },
        );
        /*
         * @brief Gets the attenuation distances for the audio source. Set to 0. if
         * audio is disabled.
         *
         *    @luatreturn number Reference distance.
         *    @luatreturn number Maximum distance.
         * @luafunc getAttenuationDistances
         */
        methods.add_method(
            "getAttenuationDistances",
            |_, this, ()| -> mlua::Result<(f32, f32)> {
                Ok(this.call(|this| this.attenuation_distances())?)
            },
        );
        /*
         * @brief Sets the rolloff factor.
         *
         *    @luatparam number rolloff New rolloff factor.
         * @luafunc setRolloff
         */
        methods.add_method("setRolloff", |_, this, rolloff: f32| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.set_rolloff(rolloff);
            })?;
            Ok(())
        });
        /*
         * @brief Gets the rolloff factor.
         *
         *    @luatreturn number Rolloff factor or 0. if sound is disabled.
         * @luafunc getRolloff
         */
        methods.add_method("getRolloff", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.rolloff())?)
        });
        /*
         * @brief Sets effects on a source.
         *
         * @usage source:setEffect( "reverb", true )
         *
         *    @luatparam string name Name of the effect.
         *    @luatparam boolean enable Whether or not to enable it on the source.
         *    @luatreturn boolean true on success.
         * @luafunc setEffect
         */
        methods.add_method(
            "setEffect",
            |_, this, (name, enable): (String, bool)| -> mlua::Result<bool> {
                this.call(|this| {
                    let slot = if enable {
                        let lock = EFX_LIST.lock().unwrap();
                        let efxid =
                            match binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| &e.name)
                            {
                                Ok(efx) => efx,
                                Err(_) => {
                                    return Err(mlua::Error::RuntimeError(format!(
                                        "effect '{name}' not found"
                                    )));
                                }
                            };
                        let efx = &lock[efxid];
                        efx.slot.raw() as ALint
                    } else {
                        AL_EFFECTSLOT_NULL
                    };
                    this.source().parameter_3_i32(
                        AL_AUXILIARY_SEND_FILTER,
                        slot,
                        0,
                        AL_FILTER_NULL,
                    );

                    Ok(true)
                })?
            },
        );
        /*
         * @brief Sets global effects, or creates it if necessary.
         *
         * @usage audio.setEffectData( "reverb", { type="reverb" } )
         *
         *    @luatparam string name Name of the effect.
         *    @luatparam table params Parameter table of the effect to create.
         * @luafunc setEffectData
         */
        methods.add_function(
            "setEffectData",
            |_, (name, param): (String, mlua::Table)| -> mlua::Result<()> {
                let mut lock = EFX_LIST.lock().unwrap();
                let efxid = match binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| &e.name)
                {
                    Ok(efx) => efx,
                    Err(_) => {
                        let efx = LuaAudioEfx::new(&name)?;
                        lock.push(efx);
                        sort_by_key_ref(&mut lock, |e: &LuaAudioEfx| &e.name);
                        binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| &e.name).unwrap()
                    }
                };
                let efx = &lock[efxid];

                let typename: String = param.get("type")?;
                let volume: Option<f32> = param.get("volume")?;

                macro_rules! efx_set_f32 {
                    ($name: literal, $field: ident) => {{
                        match param.get::<mlua::Value>($name)? {
                            mlua::Value::Nil => (),
                            mlua::Value::Number(val) => {
                                efx.effect.parameter_f32($field, val as f32)
                            }
                            mlua::Value::Integer(val) => {
                                efx.effect.parameter_f32($field, val as f32)
                            }
                            val => {
                                return Err(mlua::Error::RuntimeError(format!(
                                    "invalid type '{}' for paremeter '{}' (expected f32)",
                                    val.type_name(),
                                    $name
                                )));
                            }
                        }
                    }};
                }
                macro_rules! efx_set_i32 {
                    ($name: literal, $field: ident) => {{
                        match param.get::<mlua::Value>($name)? {
                            mlua::Value::Nil => (),
                            mlua::Value::Number(val) => {
                                efx.effect.parameter_i32($field, val as i32)
                            }
                            mlua::Value::Integer(val) => {
                                efx.effect.parameter_i32($field, val as i32)
                            }
                            val => {
                                return Err(mlua::Error::RuntimeError(format!(
                                    "invalid type '{}' for paremeter '{}' (expected i32)",
                                    val.type_name(),
                                    $name
                                )));
                            }
                        }
                    }};
                }

                if typename == "reverb" {
                    efx.effect.parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_REVERB);
                    efx_set_f32!("density", AL_REVERB_DENSITY);
                    efx_set_f32!("density", AL_REVERB_DENSITY); // 0.0 to 1.0 (1.0)
                    efx_set_f32!("diffusion", AL_REVERB_DIFFUSION); // 0.0 to 1.0 (1.0)
                    efx_set_f32!("gain", AL_REVERB_GAIN); // 0.0 to 1.0 (0.32)
                    efx_set_f32!("highgain", AL_REVERB_GAINHF); // 0.0 to 1.0 (0.89)
                    efx_set_f32!("decaytime", AL_REVERB_DECAY_TIME); // 0.1 to 20.0 (1.49)
                    efx_set_f32!("decayhighratio", AL_REVERB_DECAY_HFRATIO); // 0.1 to 2.0 (0.83)
                    efx_set_f32!("earlygain", AL_REVERB_REFLECTIONS_GAIN); // 0.0 to 3.16 (0.05)
                    efx_set_f32!("earlydelay", AL_REVERB_REFLECTIONS_DELAY); // 0.0 to 0.3 (0.007)
                    efx_set_f32!("lategain", AL_REVERB_LATE_REVERB_GAIN); // 0.0 to 10.0 (1.26)
                    efx_set_f32!("latedelay", AL_REVERB_LATE_REVERB_DELAY); // 0.0 to 0.1 (0.011)
                    efx_set_f32!("roomrolloff", AL_REVERB_ROOM_ROLLOFF_FACTOR); // 0.0 to 10.0 (0.0)
                    efx_set_f32!("airabsorption", AL_REVERB_AIR_ABSORPTION_GAINHF); // 0.892 to 1.0 (0.994)
                    efx_set_i32!("highlimit", AL_REVERB_DECAY_HFLIMIT); // AL_FALSE or AL_TRUE (AL_TRUE)
                } else if typename == "distortion" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
                    efx_set_f32!("gain", AL_DISTORTION_GAIN); // 0.01 to 1.0 (0.2)
                    efx_set_f32!("edge", AL_DISTORTION_EDGE); // 0.0 to 1.0 (0.05)
                    efx_set_f32!("lowcut", AL_DISTORTION_LOWPASS_CUTOFF); // 80.0 to 24000.0 (8000.0)
                    efx_set_f32!("center", AL_DISTORTION_EQCENTER); // 80.0 to 24000.0 (3600.0)
                    efx_set_f32!("bandwidth", AL_DISTORTION_EQBANDWIDTH); // 80.0 to 24000.0 (3600.0)
                } else if typename == "chorus" {
                    efx.effect.parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
                    efx_set_i32!("waveform", AL_CHORUS_WAVEFORM); // 0=sin, 1=triangle (1)
                    efx_set_i32!("phase", AL_CHORUS_PHASE); // -180 to 180 (90)
                    efx_set_f32!("rate", AL_CHORUS_RATE); // 0.0 to 10.0 (1.1)
                    efx_set_f32!("depth", AL_CHORUS_DEPTH); // 0.0 to 1.0 (0.1)
                    efx_set_f32!("feedback", AL_CHORUS_FEEDBACK); // -1.0 to 1.0 (0.25)
                    efx_set_f32!("delay", AL_CHORUS_DELAY); // 0.0 to 0.016 (0.016)
                } else if typename == "compressor" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
                    efx_set_i32!("enable", AL_COMPRESSOR_ONOFF); // AL_FALSE or AL_TRUE (AL_TRUE)
                } else if typename == "echo" {
                    efx.effect.parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_ECHO);
                    efx_set_f32!("delay", AL_ECHO_DELAY); // 0.0 to 0.207 (0.1)
                    efx_set_f32!("tapdelay", AL_ECHO_LRDELAY); // 0.0 to 0.404 (0.1)
                    efx_set_f32!("damping", AL_ECHO_DAMPING); // 0.0 to 0.99 (0.5)
                    efx_set_f32!("feedback", AL_ECHO_FEEDBACK); // 0.0 to 1.0 (0.5)
                    efx_set_f32!("spread", AL_ECHO_SPREAD); // -1.0 to 1.0 (-1.0)
                } else if typename == "ringmodulator" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
                    efx_set_f32!("frequency", AL_RING_MODULATOR_FREQUENCY); // 0.0 to 8000.0 (440.0)
                    efx_set_f32!("highcut", AL_RING_MODULATOR_HIGHPASS_CUTOFF); // 0.0 to 24000.0 (800.0)
                    efx_set_i32!("waveform", AL_RING_MODULATOR_WAVEFORM); // 0 (sin), 1 (saw), 2 (square), (0 (sin))
                } else if typename == "equalizer" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
                    efx_set_f32!("lowgain", AL_EQUALIZER_LOW_GAIN); // 0.126 to 7.943 (1.0)
                    efx_set_f32!("lowcut", AL_EQUALIZER_LOW_CUTOFF); // 50.0 to 800.0 (200.0)
                    efx_set_f32!("lowmidgain", AL_EQUALIZER_MID1_GAIN); // 0.126 to 7.943 (1.0)
                    efx_set_f32!("lowmidfrequency", AL_EQUALIZER_MID1_CENTER); // 200.0 to 3000.0 (500.0)
                    efx_set_f32!("lowmidbandwidth", AL_EQUALIZER_MID1_WIDTH); // 0.01 to 1.0 (1.0)
                    efx_set_f32!("highmidgain", AL_EQUALIZER_MID2_GAIN); // 0.126 to 7.943 (1.0)
                    efx_set_f32!("highmidfrequency", AL_EQUALIZER_MID2_CENTER); // 1000.0 to 8000.0 (3000.0)
                    efx_set_f32!("highmidbandwidth", AL_EQUALIZER_MID2_WIDTH); // 0.01 to 1.0 (1.0)
                    efx_set_f32!("highgain", AL_EQUALIZER_HIGH_GAIN); // 0.126 to 7.943 (1.0)
                    efx_set_f32!("highcut", AL_EQUALIZER_HIGH_CUTOFF); // 4000.0 to 16000.0 (6000.0)
                } else if typename == "pitchshifter" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);

                    efx_set_i32!("tunecoarse", AL_PITCH_SHIFTER_COARSE_TUNE); // -12 to 12 (12)
                    efx_set_i32!("tunefine'", AL_PITCH_SHIFTER_FINE_TUNE); // -50 to 50  (0)
                } else if typename == "vocalmorpher" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
                    efx_set_i32!("phonemea", AL_VOCAL_MORPHER_PHONEMEA); // 0 to 29 (0 ("A"))
                    efx_set_i32!("phonemeb", AL_VOCAL_MORPHER_PHONEMEB); // 0 to 29 (10 ("ER"))
                    efx_set_i32!("tunecoarsea", AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING); // -24 to 24 (0)
                    efx_set_i32!("tunecoarseb", AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING); // -24 to 24 (0)
                    efx_set_i32!("waveform", AL_VOCAL_MORPHER_WAVEFORM); // 0 (sin), 1 (saw), 2 (square), (0 (sin))
                    efx_set_f32!("rate", AL_VOCAL_MORPHER_RATE); // 0.0 to 10.0 (1.41)
                } else if typename == "flanger" {
                    efx.effect.parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
                    efx_set_i32!("waveform", AL_FLANGER_WAVEFORM); //  0 (sin), 1 (triangle)  (1 (triangle))
                    efx_set_f32!("phase", AL_FLANGER_PHASE); // -180 to 180 (0)
                    efx_set_f32!("rate", AL_FLANGER_RATE); // 0.0 to 10.0 (0.27)
                    efx_set_f32!("depth", AL_FLANGER_DEPTH); // 0.0 to 1.0 (1.0)
                    efx_set_f32!("feedback", AL_FLANGER_FEEDBACK); // -1.0 to 1.0 (-0.5)
                    efx_set_f32!("delay", AL_FLANGER_DELAY); // 0.0 to 0.004 (0.002)
                } else if typename == "frequencyshifter" {
                    efx.effect
                        .parameter_i32(AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
                    efx_set_f32!("frequency", AL_FREQUENCY_SHIFTER_FREQUENCY); // 0.0 to 24000.0 (0.0)
                    efx_set_i32!("leftdirection", AL_FREQUENCY_SHIFTER_LEFT_DIRECTION); // 0 (down), 1 (up), 2 (off) (0 (down))
                    efx_set_i32!("rightdirection", AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION); // 0 (down), 1 (up), 2 (off) (0 (down))
                } else {
                    return Err(mlua::Error::RuntimeError(format!(
                        "uknown effect type '{typename}'"
                    )));
                }

                if let Some(volume) = volume {
                    efx.slot.parameter_f32(AL_EFFECTSLOT_GAIN, volume);
                }
                efx.slot.set_effect(Some(&efx.effect));
                Ok(())
            },
        );
        /*
         * @brief Sets global effects, or creates it if necessary.
         *
         * @usage audio.setGlobalEffect( "reverb" )
         *
         *    @luatparam string name Name of the effect.
         *    @luatreturn boolean true on success.
         * @luafunc setGlobalEffect
         */
        methods.add_function(
            "setGlobalEffect",
            |_, name: Option<String>| -> mlua::Result<()> {
                if let Some(efx) = EFX.get() {
                    let direct_slot = &efx.direct_slot;
                    if let Some(name) = name {
                        let lock = EFX_LIST.lock().unwrap();
                        let efxid =
                            match binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| &e.name)
                            {
                                Ok(efx) => efx,
                                Err(_) => {
                                    return Err(mlua::Error::RuntimeError(format!(
                                        "effect '{name}' not found"
                                    )));
                                }
                            };
                        let efx = &lock[efxid];
                        direct_slot.set_effect(Some(&efx.effect));
                    } else {
                        direct_slot.set_effect(None);
                    }
                }
                Ok(())
            },
        );
        /*
         * @brief Allows setting the speed of sound.
         *
         *    @luatparam[opt=3443] number speed Air speed.
         * @luafunc setSpeedOfSound
         */
        methods.add_function(
            "setSpeedOfSound",
            |_, speed: Option<f32>| -> mlua::Result<()> {
                unsafe {
                    alSpeedOfSound(speed.unwrap_or(3433.));
                }
                Ok(())
            },
        );
        /*
         * @brief Sets the Doppler effect factor.
         *
         * Defaults to 0.3 outside of the nebula and 1.0 in the nebula.
         *
         *    @luatparam number factor Factor to set Doppler effect to. Must be
         * positive.
         * @luafunc setGlobalDopplerFactor
         */
        methods.add_function("setDopplerFactor", |_, factor: f32| -> mlua::Result<()> {
            unsafe { alDopplerFactor(factor) };
            Ok(())
        });
    }
}

pub fn open_audio(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<AudioRef>()?)
}

// Here be C API
use std::ffi::{CStr, c_char, c_double, c_int, c_void};

// We assume that the index can be cast to a pointer for C to not complain
// This should hold on 64 bit platforms
static_assertions::assert_eq_size!(AudioRef, *const c_void);

#[unsafe(no_mangle)]
pub extern "C" fn sound_get(name: *const c_char) -> *const Arc<AudioBuffer> {
    if name.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let name = unsafe { CStr::from_ptr(name).to_string_lossy() };
    for ext in ["wav", "ogg", "flac"] {
        let path = format!("snd/sounds/{name}.{ext}");
        if let Ok(buffer) = AudioBuffer::get_or_try_load(&path) {
            return Box::into_raw(Box::new(buffer));
        };
    }
    std::ptr::null()
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_getLength(sound: *const Arc<AudioBuffer>) -> c_double {
    if sound.is_null() {
        warn!("recieved NULL");
        return 0.0;
    }
    let sound = unsafe { &*sound };
    sound.duration(AudioSeek::Seconds) as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_play(sound: *const Arc<AudioBuffer>) -> *const c_void {
    if sound.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let sound = unsafe { &*sound };
    match AudioRef::new_buffer(sound) {
        Ok(audioref) => unsafe { std::mem::transmute::<AudioRef, *const c_void>(audioref) },
        Err(e) => {
            warn_err!(e);
            std::ptr::null()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_playPos(
    sound: *const Arc<AudioBuffer>,
    px: c_double,
    py: c_double,
    vx: c_double,
    vy: c_double,
) -> *const c_void {
    if sound.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let sound = unsafe { &*sound };

    let mut audio = match Audio::new_buffer(sound) {
        Ok(audio) => audio,
        Err(e) => {
            warn_err!(e);
            return std::ptr::null();
        }
    };
    audio.set_ingame();
    audio.set_position(Vector3::from([px as f32, py as f32, 0.0]));
    audio.set_velocity(Vector3::from([vx as f32, vy as f32, 0.0]));
    let voice = AudioRef::from_audio(audio);
    unsafe { std::mem::transmute::<AudioRef, *const c_void>(voice) }
}

macro_rules! get_voice {
    ($voice: ident) => {{
        if $voice.is_null() {
            warn!("recieved NULL");
            return Default::default();
        }
        unsafe { std::mem::transmute::<*const c_void, AudioRef>($voice) }
    }};
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_stop(voice: *const c_void) {
    let index = get_voice!(voice);
    if let Err(e) = index.call(|voice| voice.stop()) {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_updatePos(
    voice: *const c_void,
    px: c_double,
    py: c_double,
    vx: c_double,
    vy: c_double,
) {
    let index = get_voice!(voice);
    if let Err(e) = index.call(|voice| {
        voice.set_position(Vector3::from([px as f32, py as f32, 0.0]));
        voice.set_velocity(Vector3::from([vx as f32, vy as f32, 0.0]));
    }) {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_updateListener(
    dir: c_double,
    px: c_double,
    py: c_double,
    vx: c_double,
    vy: c_double,
) {
    let dir = dir as f32;
    let (c, s) = (dir.cos(), dir.sin());
    let ori = [c, s, 0.0, 0.0, 0.0, 1.0];
    unsafe {
        alListenerfv(AL_ORIENTATION, ori.as_ptr());
    }
    set_listener_position(Vector3::from([px as f32, py as f32, 0.0]));
    set_listener_velocity(Vector3::from([vx as f32, vy as f32, 0.0]));
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_update(_dt: c_double) -> i32 {
    AUDIO.execute_messages();
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pause() {
    let voices = AUDIO.voices.lock().unwrap();
    for (_, v) in voices.iter() {
        v.pause();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_resume() {
    let voices = AUDIO.voices.lock().unwrap();
    for (_, v) in voices.iter() {
        v.play();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_volume(volume: c_double) {
    AUDIO.set_volume(volume as f32);
    let master = {
        let vol = AUDIO.volume.read().unwrap();
        vol.volume * vol.volume_speed
    };
    let voices = AUDIO.voices.lock().unwrap();
    for (_, v) in voices.iter() {
        v.source().parameter_f32(AL_GAIN, master * v.volume());
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_getVolume() -> c_double {
    AUDIO.volume.read().unwrap().volume_lin as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_getVolumeLog() -> c_double {
    AUDIO.volume.read().unwrap().volume as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_stopAll() {
    let mut voices = AUDIO.voices.lock().unwrap();
    voices.retain(|_, v| match v {
        Audio::Static(this) => !this.ingame,
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_setSpeed(speed: c_double) {
    AUDIO.set_volume_speed(speed as f32);
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_createGroup(size: c_int) -> *const c_void {
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = AudioGroup {
        max: size as usize,
        volume: 1.0,
        pitch: 1.0,
        speed_affects: true,
        voices: Vec::new(),
    };
    let groupid = groups.insert(group);
    unsafe { std::mem::transmute::<thunderdome::Index, *const c_void>(groupid) }
}

macro_rules! get_group {
    ($group: ident) => {{
        if $group.is_null() {
            warn!("recieved NULL");
            return Default::default();
        }
        unsafe { std::mem::transmute::<*const c_void, thunderdome::Index>($group) }
    }};
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_playGroup(
    group: *const c_void,
    sound: *const Arc<AudioBuffer>,
    once: c_int,
) -> *const c_void {
    if sound.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let sound = unsafe { &*sound };

    let groupid = get_group!(group);
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = &mut groups[groupid];
    if group.voices.len() >= group.max {
        return std::ptr::null();
    }

    let mut audio = AudioStatic::new(&Some(AudioData::Buffer(sound.clone()))).unwrap();
    audio.groupid = Some(groupid);
    let audio = Audio::Static(audio);
    if once != 0 {
        audio.set_looping(true);
    }
    let voice = AudioRef::from_audio(audio);
    unsafe { std::mem::transmute::<AudioRef, *const c_void>(voice) }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_stopGroup(group: *const c_void) {
    let groupid = get_group!(group);
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = &mut groups[groupid];
    let voices = AUDIO.voices.lock().unwrap();
    for v in group.voices.drain(..) {
        if let Some(voice) = voices.get(*v.0) {
            voice.stop();
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pauseGroup(group: *const c_void) {
    let groupid = get_group!(group);
    let groups = AUDIO.groups.lock().unwrap();
    let group = &groups[groupid];
    let voices = AUDIO.voices.lock().unwrap();
    for v in group.voices.iter() {
        if let Some(voice) = voices.get(*v.0) {
            voice.pause();
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_resumeGroup(group: *const c_void) {
    let groupid = get_group!(group);
    let groups = AUDIO.groups.lock().unwrap();
    let group = &groups[groupid];
    let voices = AUDIO.voices.lock().unwrap();
    for v in group.voices.iter() {
        if let Some(voice) = voices.get(*v.0) {
            voice.play();
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_speedGroup(group: *const c_void, enable: c_int) {
    let groupid = get_group!(group);
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = &mut groups[groupid];
    group.speed_affects = enable != 0;
    //group.update();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_volumeGroup(group: *const c_void, volume: c_double) {
    let groupid = get_group!(group);
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = &mut groups[groupid];
    group.volume = volume as f32;
    //group.update();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pitchGroup(group: *const c_void, pitch: c_double) {
    let groupid = get_group!(group);
    let mut groups = AUDIO.groups.lock().unwrap();
    let group = &mut groups[groupid];
    group.pitch = pitch as f32;
    //group.update();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_setAbsorption(value: c_double) {
    AUDIO.set_air_absorption_factor(value as f32);
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_env(env: naevc::SoundEnv_t, param: f64) {
    match env {
        naevc::SoundEnv_e_SOUND_ENV_NORMAL => {
            unsafe {
                alSpeedOfSound(3433.0);
                alDopplerFactor(0.3);
            }
            AUDIO.set_air_absorption_factor(0.0);

            if let Some(efx) = EFX.get() {
                efx.direct_slot.set_effect(None);
            }
        }
        naevc::SoundEnv_e_SOUND_ENV_NEBULA => {
            let f = param as f32 / 1000.0;
            unsafe {
                alSpeedOfSound(3433.0 / (1.0 + f * 2.0));
                alDopplerFactor(1.0);
            }
            AUDIO.set_air_absorption_factor(3.0 * f);

            if let Some(efx) = EFX.get() {
                efx.reverb.parameter_f32(AL_REVERB_DECAY_TIME, 10.0);
                efx.reverb.parameter_f32(AL_REVERB_DECAY_HFRATIO, 0.5);
                efx.direct_slot.set_effect(Some(&efx.reverb));
            }
        }
        _ => (),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_disabled() -> c_int {
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_set_disabled(_disable: c_int) {}

/*
pub fn test () {
    audio::init().unwrap();
    let buf = AudioBuffer::get_or_try_load( "snd/sounds/activate1.ogg" )?;
    dbg!( buf.duration( AudioSeek::Seconds ) );
    let audioref = AUDIO.play_buffer(&buf)?;
        audioref.call( |a| {
            let src = a.source();
            unsafe {
                al::alSourcePlay(  src.raw() );
                dbg!( al::alGetError() );
            }
        })?;
    for _ in 0..3 {
        audioref.call( |a| {
            dbg!(a.tell( AudioSeek::Seconds ) );
            dbg!(a.is_playing());
        }).unwrap();
        std::thread::sleep( std::time::Duration::from_millis(1000) );
    }
}
*/
