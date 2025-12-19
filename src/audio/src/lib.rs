//#![allow(dead_code, unused_imports)]
mod debug;
mod efx;
#[macro_use]
pub mod openal;
mod buffer_length_query;
mod callback_buffer;
mod direct_channels_remix;
mod events;
mod output_limiter;
mod source_spatialize;
use crate::buffer_length_query::consts::*;
use crate::direct_channels_remix::consts::*;
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
use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use mlua::{Either, MetaMethod, UserData, UserDataMethods, UserDataRef};
use nalgebra::{Vector2, Vector3};
use std::fmt::{Debug, Formatter};
use std::mem::ManuallyDrop;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, AtomicPtr, Ordering};
use std::sync::{Arc, LazyLock, Weak};
#[cfg(not(debug_assertions))]
use std::sync::{Mutex, RwLock};
use symphonia::core::audio::{Channels, Signal};
use symphonia::core::conv::{FromSample, IntoSample};
use symphonia::core::{
    codecs::Decoder, formats::FormatReader, io::MediaSourceStream, sample::Sample,
};
use thunderdome::Arena;
#[cfg(debug_assertions)]
use tracing_mutex::stdsync::{Mutex, RwLock};
use utils::atomicfloat::AtomicF32;
use utils::{binary_search_by_key_ref, sort_by_key_ref};

/// OpenAL soft doesn't distinguish between mono and stereo
const MAX_SOURCES: usize = 512;
/// Priority sources
const PRIORITY_SOURCES: usize = 8;
/// Reference distance for sounds
const REFERENCE_DISTANCE: f32 = 500.;
/// Max distance for sounds to still play at
const MAX_DISTANCE: f32 = 25_000.;
/// Number of frames we want to grab per buffer when streaming.
const STREAMING_BUFFER_LENGTH: usize = 32 * 1024;
/// Amount we sleep per frame when streaming, should be at least enough time to load a single
/// buffer. Should be at least greater than 2 * STREAMING_BUFFER_LENGTH / 48000.
const STREAMING_SLEEP_DELAY: std::time::Duration = std::time::Duration::from_millis(30);

struct LuaAudioEfx {
    name: String,
    effect: Effect,
    slot: AuxiliaryEffectSlot,
}
impl LuaAudioEfx {
    pub fn new(name: &str) -> Result<Self> {
        let effect = Effect::new()?;
        debug::object_label(debug::consts::AL_EFFECT_EXT, effect.raw(), name);
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
    LuaStatic,
    LuaStream,
}
impl AudioType {
    pub fn priority_threshold(&self) -> usize {
        match self {
            AudioType::Static | AudioType::LuaStatic => MAX_SOURCES - PRIORITY_SOURCES,
            AudioType::LuaStream => MAX_SOURCES - 1,
        }
    }
}

/// Small wrapper for Mono/Stereo frames
enum Frame<T> {
    Mono(T),
    Stereo(T, T),
}
impl<T> Frame<T> {
    /// Loads a Vec of Frame<f32> from an AudioBufferRef
    fn load_frames_from_buffer_ref(
        buffer: &symphonia::core::audio::AudioBufferRef,
    ) -> Result<Vec<Frame<f32>>> {
        fn load_frames_from_buffer<S: Sample>(
            buffer: &symphonia::core::audio::AudioBuffer<S>,
        ) -> Result<Vec<Frame<f32>>>
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
        use symphonia::core::audio::AudioBufferRef;
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

    /// Converts a vector of frames to a raw interleaved data vector usable by OpenAL
    fn vec_to_data(f: Vec<Frame<T>>, stereo: bool) -> Vec<T>
    where
        T: Copy,
    {
        match stereo {
            true => {
                use std::iter::once;
                f.iter()
                    .flat_map(|x| match x {
                        Frame::Mono(x) => once(*x).chain(once(*x)),
                        Frame::Stereo(l, r) => once(*l).chain(once(*r)),
                    })
                    .collect()
            }
            false => f
                .iter()
                .map(|x| match x {
                    Frame::Mono(x) => *x,
                    Frame::Stereo(l, _) => *l,
                })
                .collect(),
        }
    }
}

/// Handles loading and filtering replaygain based on symphonia
#[derive(Debug, PartialEq, Copy, Clone)]
pub struct ReplayGain {
    /// Scale factor computed from Track Gain
    scale_factor: f32,
    /// Max scale computed from Track Peak
    max_scale: f32,
}
impl ReplayGain {
    /// Extracts the replaygain values from a FormatReader
    fn from_formatreader(format: &mut Box<dyn FormatReader>) -> Result<Option<Self>> {
        use symphonia::core::meta::{StandardTagKey, Tag, Value};
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
        if let Some(track_gain_db) = track_gain_db {
            let scale_factor = 10.0_f32.powf(track_gain_db / 20.0);
            let max_scale = 1.0 / track_peak.unwrap_or(1.0);
            Ok(Some(Self {
                scale_factor,
                max_scale,
            }))
        } else {
            Ok(None)
        }
    }

    /// Filters an audio stream. Ported from vgfilter.c implementation.
    fn filter(&self, data: &mut [f32]) {
        if self.scale_factor > self.max_scale {
            for d in data {
                let mut cur_sample = *d * self.scale_factor;
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
            for d in data {
                *d *= self.scale_factor;
            }
        }
    }
}

pub struct BufferData {
    pub name: PathBuf,
    pub stereo: bool,
    pub replay_gain: Option<ReplayGain>,
    pub sample_rate: u32,
    pub data: Vec<f32>,
}
impl BufferData {
    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let path = path.as_ref();
        // If no extension try to autodetect.
        let ext = path.extension().and_then(|s| s.to_str());
        let path = Buffer::get_valid_path(path)
            .context(format!("No audio file matching '{}' found", path.display()))?;
        let src = ndata::open(&path)?;

        // Load it up
        let codecs = &CODECS;
        let probe = symphonia::default::get_probe();
        let mss = MediaSourceStream::new(Box::new(src), Default::default());
        let mut hint = symphonia::core::probe::Hint::new();
        if let Some(ext) = ext {
            hint.with_extension(ext);
        }
        // Enable gapless so encoded padding (e.g. Opus pre-skip/end trim) is removed.
        let mut format = probe
            .format(
                &hint,
                mss,
                &symphonia::core::formats::FormatOptions {
                    enable_gapless: true,
                    ..Default::default()
                },
                &Default::default(),
            )?
            .format;

        // Get replaygain information
        let replay_gain = ReplayGain::from_formatreader(&mut format)?;

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
        let delay = codec_params.delay;
        let padding = codec_params.padding;

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
                frames.append(&mut Frame::<f32>::load_frames_from_buffer_ref(&buffer)?);
            }
        }

        // I'm not sure exactly what's going on here, but the usage of padding+delay seems to be the
        // opposite to what is mentioned in the Symphonia docs at least as of 0.5.5. As this seems
        // to give correct results (for now), I am leaving it as is.
        if let Some(padding) = delay {
            frames.truncate(frames.len() - padding as usize);
        }
        if let Some(delay) = padding {
            let delay = delay as usize;
            if delay >= frames.len() {
                anyhow::bail!(format!(
                    "Audio '{}' is draining all frames!",
                    path.display()
                ));
            }
            frames.drain(..delay);
        }

        // Squish the frames together
        let data: Vec<f32> = Frame::vec_to_data(frames, stereo);

        Ok(Self {
            name: path,
            stereo,
            replay_gain,
            sample_rate,
            data,
        })
    }
}

#[derive(PartialEq, Debug)]
pub struct Buffer {
    name: PathBuf,
    stereo: bool,
    buffer: al::Buffer,
}
impl Buffer {
    pub fn get_valid_path<P: AsRef<Path>>(path: P) -> Option<PathBuf> {
        let path = path.as_ref();
        let ext = path.extension().and_then(|s| s.to_str());
        match ext {
            Some(_) => Some(path.to_path_buf()),
            None => {
                let mut path = path.to_path_buf();
                for ext in &["opus", "ogg", "flac", "wav"] {
                    path.set_extension(ext);
                    if ndata::exists(&path) {
                        return Some(path);
                    }
                }
                None
            }
        }
    }

    fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let (name, stereo, replaygain, mut data, sample_rate) = {
            let data = BufferData::from_path(path)?;
            (
                data.name,
                data.stereo,
                data.replay_gain,
                data.data,
                data.sample_rate,
            )
        };

        // Filter function for decoded Ogg Vorbis streams taken from "vgfilter.c"
        if let Some(replaygain) = replaygain {
            replaygain.filter(&mut data);
        }

        /*
        // Crossfade overlap: Blend the end of the buffer into the start, then truncate.
        // This effectively moves the loop seam to be seamless.
        fn perform_crossfade_overlap(data: &mut Vec<f32>, channels: usize, sample_rate: u32) {
            let blend_ms = 5.0; // Increased to 15ms for better handling of engine rumble
            let frames_to_blend =
                ((sample_rate as f32 * blend_ms / 1000.0).round() as usize).max(1);
            let needed_samples = frames_to_blend * channels;

            // Need enough data: Loop body + Overlap
            if data.len() < needed_samples * 2 {
                return;
            }

            let len = data.len();
            let overlap_start_idx = len - needed_samples;

            // Blend the tail (end) into the head (start)
            // Head fades IN (0 -> 1), Tail fades OUT (1 -> 0)
            // But wait, standard crossfade for loop:
            // We want the END of the loop to transition to the START.
            // Current State: [Start ... End]
            // We want to make [Start ... NewEnd] such that NewEnd matches Start.
            // Actually, usually we take [Start ... End ... Overlap] and blend Overlap into Start.
            // Here we assume the file *is* the loop, so [Start ... End].
            // We take the last N samples (End) and blend them into Start.
            // So Start' = Start * t + End * (1-t) ? No.
            // Flow: ... -> End -> Start -> ...
            // We want End -> Start to be smooth.
            // If we modify Start to looks like End at the beginning:
            // Start[0] = End[0].
            // Then End[Last] -> Start[0] is End[Last] -> End[0]. (Smooth-ish?)
            // Better:
            // Standard "Crossfade Loop" tool (like in DAWs) usually crossfades the material.
            // Algorithm:
            // 1. Take last N frames.
            // 2. Mix them into first N frames using Linear Crossfade.
            //    NewFrame[i] = OldFrame[i] * (i / N) + OldFrame[Len - N + i] * (1 - i / N)
            //    At i=0: 0 * Head + 1 * Tail = Tail.
            //    At i=N: 1 * Head + 0 * Tail = Head.
            // 3. Truncate the last N frames.

            for i in 0..frames_to_blend {
                let t = i as f32 / frames_to_blend as f32;
                let fade_in = t; // 0 -> 1
                let fade_out = 1.0 - t; // 1 -> 0

                for ch in 0..channels {
                    let head_idx = i * channels + ch;
                    let tail_idx = overlap_start_idx + head_idx;

                    let original_head = data[head_idx];
                    let original_tail = data[tail_idx];

                    // Linear blend
                    data[head_idx] = original_head * fade_in + original_tail * fade_out;
                }
            }

            // Truncate the buffer
            data.truncate(overlap_start_idx);
        }

        // Apply crossfade to fix popping (non seamless looping)
        let channels = if stereo { 2 } else { 1 };
        perform_crossfade_overlap(&mut data, channels, sample_rate);
        */

        let buffer = al::Buffer::new()?;
        buffer.data_f32(&data, stereo, sample_rate as ALsizei);
        debug::object_label(
            debug::consts::AL_BUFFER_EXT,
            buffer.raw(),
            &format!("{}", name.display()),
        );
        Ok(Self {
            name,
            stereo,
            buffer,
        })
    }

    pub fn duration(&self, unit: AudioSeek) -> f32 {
        match unit {
            AudioSeek::Seconds => self.buffer.get_parameter_f32(AL_SEC_LENGTH_SOFT),
            AudioSeek::Samples => self.buffer.get_parameter_i32(AL_SAMPLE_LENGTH_SOFT) as f32,
        }
    }

    pub fn channels(&self) -> u8 {
        self.buffer.get_parameter_i32(AL_CHANNELS) as u8
    }

    pub fn frequency(&self) -> u32 {
        self.buffer.get_parameter_i32(AL_FREQUENCY) as u32
    }

    pub fn sample_count(&self) -> f32 {
        let bytes = self.buffer.get_parameter_i32(AL_SIZE);
        let channels = self.buffer.get_parameter_i32(AL_CHANNELS);
        let bits = self.buffer.get_parameter_i32(AL_CHANNELS);
        (bytes * 8 / (channels * bits)) as f32
    }

    pub fn get(name: &str) -> Option<Arc<Self>> {
        let name = match Buffer::get_valid_path(name) {
            Some(name) => name,
            None => {
                return None;
            }
        };

        let buffers = AUDIO_BUFFER.lock().unwrap();
        for buf in buffers.iter() {
            if let Some(b) = buf.upgrade()
                && b.name == name
            {
                return Some(b);
            }
        }
        None
    }

    pub fn get_or_try_load<P: AsRef<Path>>(name: P) -> Result<Arc<Self>> {
        if AUDIO.disabled {
            return Ok(Arc::new(Buffer {
                name: PathBuf::new(),
                stereo: false,
                buffer: al::Buffer(0),
            }));
        }
        let name = Buffer::get_valid_path(&name).context(format!(
            "No audio file matching '{}' found",
            name.as_ref().display()
        ))?;

        let mut buffers = AUDIO_BUFFER.lock().unwrap();
        for buf in buffers.iter() {
            if let Some(b) = buf.upgrade()
                && b.name == name
            {
                return Ok(b);
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
pub(crate) static AUDIO_BUFFER: LazyLock<Mutex<Vec<Weak<Buffer>>>> =
    LazyLock::new(|| Mutex::new(Default::default()));
/// Counter for how many buffers were destroyed
//pub(crate) static GC_COUNTER: AtomicU32 = AtomicU32::new(0);
/// Number of destroyed buffers to start garbage collecting the cache
//pub(crate) const GC_THRESHOLD: u32 = 128;

#[derive(Clone, PartialEq, Copy, Eq, Debug)]
pub enum AudioSeek {
    Seconds,
    Samples,
}

#[derive(Debug, PartialEq, Clone)]
pub enum AudioData {
    Buffer(Arc<Buffer>),
}

#[derive(Debug)]
pub enum Audio {
    Static(AudioStatic),
    LuaStatic(AudioStatic),
    LuaStream(AudioStream),
}

#[derive(Debug, PartialEq)]
pub struct Source {
    inner: al::Source,
    volume: f32,
    pitch: f32,
    // Group values
    g_volume: f32,
    // Pitch as None means we don't want it to change, while having other values indicates it
    // should change.
    g_pitch: Option<f32>,
}
impl Source {
    fn new(source: al::Source) -> Self {
        Self {
            inner: source,
            volume: 1.,
            pitch: 1.,
            g_volume: 1.,
            g_pitch: Some(1.),
        }
    }
}

#[derive(PartialEq, Debug)]
pub struct AudioStatic {
    source: Source,
    slot: Option<AuxiliaryEffectSlot>,
    data: Option<AudioData>,
    groupid: Option<GroupRef>,
    ingame: bool,
    stereo: bool,
}
impl AudioStatic {
    fn new(data: &Option<AudioData>, source: al::Source) -> Result<Self> {
        match data {
            Some(AudioData::Buffer(buffer)) => Self::new_buffer(buffer, source),
            None => Ok(AudioStatic {
                source: Source::new(source),
                slot: None,
                data: None,
                groupid: None,
                ingame: false,
                stereo: false,
            }),
        }
    }

    fn new_buffer(buffer: &Arc<Buffer>, source: al::Source) -> Result<Self> {
        let source = Source::new(source);
        source.inner.parameter_i32(AL_BUFFER, buffer.raw() as ALint);
        debug::object_label(
            debug::consts::AL_SOURCE_EXT,
            source.inner.raw(),
            &format!("{}", &buffer.name.display()),
        );
        Ok(AudioStatic {
            source,
            slot: None,
            data: Some(AudioData::Buffer(buffer.clone())),
            groupid: None,
            ingame: false,
            stereo: buffer.stereo,
        })
    }

    pub fn try_clone(&self, source: al::Source) -> Result<Self> {
        let mut audio = Self::new(&self.data, source)?;
        audio.slot = None;
        audio.source.volume = self.source.volume;
        audio.source.pitch = self.source.pitch;
        // TODO copy some other properties over and set volume
        Ok(audio)
    }
}

/// Structure to help stream data
struct StreamData {
    source: ManuallyDrop<al::Source>,
    buffers: [al::Buffer; 2],
    format: Box<dyn FormatReader>,
    track_id: u32,
    decoder: Box<dyn Decoder>,
    replaygain: Option<ReplayGain>,
    stereo: bool,
    sample_rate: u32,
    active: usize,
}
impl Debug for StreamData {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), std::fmt::Error> {
        write!(f, "StreamData {{ }}")
    }
}
impl StreamData {
    fn from_file(source: &al::Source, file: ndata::physfs::File) -> Result<Self> {
        let codecs = &CODECS;
        let probe = symphonia::default::get_probe();
        let mss = MediaSourceStream::new(Box::new(file), Default::default());
        let mut format = probe
            .format(
                &Default::default(),
                mss,
                // For long streams (music), enable gapless to honor encoder padding.
                &symphonia::core::formats::FormatOptions {
                    enable_gapless: true,
                    ..Default::default()
                },
                &Default::default(),
            )?
            .format;

        // Replaygain
        let replaygain = ReplayGain::from_formatreader(&mut format)?;

        let track = format.default_track().context("No default track")?;
        let track_id = track.id;

        // Set up buffers
        let buffers: [al::Buffer; 2] = [al::Buffer::new()?, al::Buffer::new()?];

        let codec_params = &track.codec_params;
        let sample_rate = codec_params.sample_rate.context("Unknown sample rate")?;
        let decoder = codecs.make(codec_params, &Default::default())?;

        let channels = track.codec_params.channels.context("no channels")?;
        if !channels.contains(Channels::FRONT_LEFT) {
            anyhow::bail!("no mono channel");
        }
        let stereo = channels.contains(Channels::FRONT_RIGHT);

        Ok(Self {
            // We use the raw value so it doesn't get dropped here and double free the source
            source: ManuallyDrop::new(al::Source(source.0)),
            buffers,
            format,
            track_id,
            decoder,
            replaygain,
            stereo,
            sample_rate,
            active: 0,
        })
    }

    fn queue_next_buffer(&mut self) -> Result<bool> {
        let mut rewind = false;
        let mut frames: Vec<Frame<f32>> = vec![];
        loop {
            // Get the next packet from the media format.
            let packet = match self.format.next_packet() {
                Ok(packet) => packet,
                Err(e) => match e {
                    symphonia::core::errors::Error::IoError(e) => {
                        if e.kind() == std::io::ErrorKind::UnexpectedEof
                            && e.to_string() == "end of stream"
                        {
                            if !rewind && self.source.get_parameter_i32(AL_LOOPING) != 0 {
                                rewind = true; // Only rewind once.
                                self.format.seek(
                                    symphonia::core::formats::SeekMode::Coarse,
                                    symphonia::core::formats::SeekTo::Time {
                                        time: symphonia::core::units::Time::new(0, 0.),
                                        track_id: Some(self.track_id),
                                    },
                                )?;
                                continue;
                            }
                            break;
                        }
                        return Err(symphonia::core::errors::Error::IoError(e).into());
                    }
                    e => anyhow::bail!(e),
                },
            };

            // If the packet does not belong to the selected track, skip over it.
            if packet.track_id() == self.track_id {
                // Decode the packet into audio samples.
                let buffer = self.decoder.decode(&packet)?;
                frames.append(&mut Frame::<f32>::load_frames_from_buffer_ref(&buffer)?);
                if frames.len() > STREAMING_BUFFER_LENGTH {
                    break;
                }
            }
        }

        if !frames.is_empty() {
            let mut data: Vec<f32> = Frame::vec_to_data(frames, self.stereo);
            if let Some(replaygain) = self.replaygain {
                replaygain.filter(&mut data);
            }
            self.buffers[self.active].data_f32(&data, self.stereo, self.sample_rate as ALsizei);
            self.source.queue_buffer(&self.buffers[self.active]);
            self.active = 1 - self.active;
            Ok(true)
        } else {
            Ok(false)
        }
    }
}

#[derive(Debug)]
#[allow(dead_code)]
pub struct AudioStream {
    path: PathBuf,
    source: Source,
    finish: Arc<Mutex<bool>>,
    thread: Option<std::thread::JoinHandle<Result<()>>>,
    data: Option<StreamData>,
    stereo: bool,
}
impl Drop for AudioStream {
    fn drop(&mut self) {
        self.source.inner.stop();
        *self.finish.lock().unwrap() = true;
    }
}
impl AudioStream {
    fn thread(finish: Arc<Mutex<bool>>, mut data: StreamData) -> Result<()> {
        loop {
            {
                // We have to lock here so it won't call something on a source when it has been
                // invalidated
                let lock = finish.lock().unwrap();
                if *lock {
                    return Ok(());
                }

                if data.source.get_parameter_i32(AL_SOURCE_STATE) == AL_PLAYING {
                    let processed = data.source.get_parameter_i32(AL_BUFFERS_PROCESSED);
                    if processed > 0 {
                        data.source.unqueue_buffer();
                        data.queue_next_buffer()?;
                    }
                }
            }

            std::thread::sleep(STREAMING_SLEEP_DELAY);
        }
    }

    pub fn from_path<P: AsRef<Path>>(path: P, source: al::Source) -> Result<Self> {
        let path = Buffer::get_valid_path(&path).context(format!(
            "No audio file matching '{}' found",
            path.as_ref().display()
        ))?;
        let src = ndata::open(&path)?;

        let finish = Arc::new(Mutex::new(false));
        let mut source = Source::new(source);
        source.g_pitch = None;

        let thdata = StreamData::from_file(&source.inner, src)?;
        let stereo = thdata.stereo;

        Ok(AudioStream {
            path,
            source,
            finish,
            thread: None,
            data: Some(thdata),
            stereo,
        })
    }

    fn play(&mut self) -> Result<()> {
        if self.thread.is_none()
            && let Some(mut data) = self.data.take()
        {
            for _ in 0..2 {
                data.queue_next_buffer()?;
            }
            let finish = self.finish.clone();
            self.thread = Some(std::thread::spawn(move || {
                AudioStream::thread(finish, data)
            }));
        }
        self.source.inner.play();
        Ok(())
    }

    pub fn try_clone(&self, source: al::Source) -> Result<Self> {
        let mut audio = AudioStream::from_path(&self.path, source)?;
        audio.source.volume = self.source.volume;
        audio.source.pitch = self.source.pitch;
        // TODO copy some other properties
        Ok(audio)
    }
}

macro_rules! check_audio {
    ($self: ident) => {{
        match $self {
            Audio::Static(this) | Audio::LuaStatic(this) => {
                if this.data == None {
                    return Default::default();
                }
            }
            _ => (),
        }
    }};
}
impl Audio {
    pub fn audio_type(&self) -> AudioType {
        match self {
            Self::Static(_) => AudioType::Static,
            Self::LuaStatic(_) => AudioType::LuaStatic,
            Self::LuaStream(_) => AudioType::LuaStream,
        }
    }

    fn try_clone(&self, source: al::Source) -> Result<Self> {
        match self {
            Self::Static(this) | Self::LuaStatic(this) => {
                Ok(Audio::Static(this.try_clone(source)?))
            }
            Self::LuaStream(this) => Ok(Audio::LuaStream(this.try_clone(source)?)),
        }
    }

    /// Sets the sound to be in-game as opposed to a GUI or music track.
    pub fn set_ingame(&mut self) {
        check_audio!(self);

        match self {
            Self::Static(this) | Self::LuaStatic(this) => {
                let v = &this.source.inner;
                if this.ingame {
                    return;
                }
                if HAS_AL_SOFT_SOURCE_SPATIALIZE.load(Ordering::Relaxed) {
                    v.parameter_i32(AL_SOURCE_SPATIALIZE_SOFT, AL_AUTO_SOFT);
                }
                v.parameter_f32(AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
                v.parameter_f32(AL_MAX_DISTANCE, MAX_DISTANCE);
                v.parameter_f32(AL_ROLLOFF_FACTOR, 1.);
                v.parameter_i32(AL_DIRECT_CHANNELS_SOFT, AL_FALSE.into());

                if let Some(efx) = EFX.get() {
                    v.parameter_3_i32(
                        AL_AUXILIARY_SEND_FILTER,
                        efx.direct_slot.raw() as i32,
                        0,
                        AL_FILTER_NULL,
                    );
                }
                this.ingame = true;
            }
            Self::LuaStream(_) => (),
        }
    }

    pub fn ingame(&self) -> bool {
        check_audio!(self);
        match self {
            Self::Static(this) | Self::LuaStatic(this) => this.ingame,
            Self::LuaStream(_) => false,
        }
    }

    pub fn stereo(&self) -> bool {
        check_audio!(self);
        match self {
            Self::Static(this) | Self::LuaStatic(this) => this.stereo,
            Self::LuaStream(this) => this.stereo,
        }
    }

    fn source(&self) -> &Source {
        match self {
            Self::Static(this) | Self::LuaStatic(this) => &this.source,
            Self::LuaStream(this) => &this.source,
        }
    }

    fn source_mut(&mut self) -> &mut Source {
        match self {
            Self::Static(this) | Self::LuaStatic(this) => &mut this.source,
            Self::LuaStream(this) => &mut this.source,
        }
    }

    #[inline]
    fn al_source(&self) -> &al::Source {
        &self.source().inner
    }

    fn groupid(&self) -> Option<GroupRef> {
        match self {
            Self::Static(this) => this.groupid,
            _ => None,
        }
    }

    fn is_state(&self, state: ALenum) -> bool {
        check_audio!(self);
        self.al_source().get_parameter_i32(AL_SOURCE_STATE) == state
    }

    pub fn play(&mut self) {
        check_audio!(self);
        match self {
            Self::Static(this) | Self::LuaStatic(this) => this.source.inner.play(),
            Self::LuaStream(this) => {
                if let Err(e) = this.play() {
                    warn_err!(e);
                }
            }
        }
    }

    pub fn is_playing(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_PLAYING)
    }

    pub fn pause(&self) {
        check_audio!(self);
        self.al_source().pause();
    }

    pub fn is_paused(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_PAUSED)
    }

    pub fn stop(&self) {
        check_audio!(self);
        self.al_source().stop();
    }

    pub fn is_stopped(&self) -> bool {
        check_audio!(self);
        self.is_state(AL_STOPPED)
    }

    pub fn rewind(&self) {
        check_audio!(self);
        self.al_source().rewind();
    }

    pub fn seek(&self, offset: f32, unit: AudioSeek) {
        check_audio!(self);
        match unit {
            AudioSeek::Seconds => self.al_source().parameter_f32(AL_SEC_OFFSET, offset),
            AudioSeek::Samples => self.al_source().parameter_f32(AL_SAMPLE_OFFSET, offset),
        }
    }

    pub fn tell(&self, unit: AudioSeek) -> f32 {
        check_audio!(self);
        match unit {
            AudioSeek::Seconds => self.al_source().get_parameter_f32(AL_SEC_OFFSET),
            AudioSeek::Samples => self.al_source().get_parameter_f32(AL_SAMPLE_OFFSET),
        }
    }

    pub fn set_volume(&mut self, vol: f32) {
        check_audio!(self);
        let master = AUDIO.volume.read().unwrap().volume;
        let ingame = self.ingame();
        let src = self.source_mut();
        src.volume = vol;
        let c = if ingame {
            1.0 - AUDIO.compression_gain.load(Ordering::Relaxed)
        } else {
            1.0
        };
        src.inner
            .parameter_f32(AL_GAIN, c * master * src.volume * src.g_volume);
    }

    pub fn set_volume_raw(&mut self, vol: f32) {
        check_audio!(self);
        let ingame = self.ingame();
        let src = self.source_mut();
        src.volume = vol;
        let c = if ingame {
            1.0 - AUDIO.compression_gain.load(Ordering::Relaxed)
        } else {
            1.0
        };
        src.inner
            .parameter_f32(AL_GAIN, c * src.volume * src.g_volume);
    }

    pub fn set_gain(&self, vol: f32) {
        check_audio!(self);
        self.al_source().parameter_f32(AL_GAIN, vol);
    }

    pub fn volume(&self) -> f32 {
        check_audio!(self);
        self.source().volume
    }

    pub fn set_relative(&self, relative: bool) {
        check_audio!(self);
        self.al_source()
            .parameter_i32(AL_SOURCE_RELATIVE, relative as i32);
    }

    pub fn relative(&self) -> bool {
        check_audio!(self);
        self.al_source().get_parameter_i32(AL_SOURCE_RELATIVE) != 0
    }

    pub fn set_position(&self, pos: Vector2<f32>) {
        check_audio!(self);
        self.al_source()
            .parameter_3_f32(AL_POSITION, pos.x, pos.y, 0.);
    }

    pub fn position(&self) -> Vector2<f32> {
        check_audio!(self);
        let v3 = Vector3::from(self.al_source().get_parameter_3_f32(AL_POSITION));
        Vector2::new(v3.x, v3.y)
    }

    pub fn set_position_3d(&self, pos: Vector3<f32>) {
        check_audio!(self);
        self.al_source()
            .parameter_3_f32(AL_POSITION, pos.x, pos.y, pos.z);
    }

    pub fn position_3d(&self) -> Vector3<f32> {
        check_audio!(self);
        Vector3::from(self.al_source().get_parameter_3_f32(AL_POSITION))
    }

    pub fn set_velocity(&self, pos: Vector2<f32>) {
        check_audio!(self);
        self.al_source()
            .parameter_3_f32(AL_VELOCITY, pos.x, pos.y, 0.);
    }

    pub fn velocity(&self) -> Vector2<f32> {
        check_audio!(self);
        let v3 = Vector3::from(self.al_source().get_parameter_3_f32(AL_VELOCITY));
        Vector2::new(v3.x, v3.y)
    }

    pub fn set_velocity_3d(&self, pos: Vector3<f32>) {
        check_audio!(self);
        self.al_source()
            .parameter_3_f32(AL_VELOCITY, pos.x, pos.y, pos.z);
    }

    pub fn velocity_3d(&self) -> Vector3<f32> {
        check_audio!(self);
        Vector3::from(self.al_source().get_parameter_3_f32(AL_VELOCITY))
    }

    pub fn set_looping(&self, looping: bool) {
        check_audio!(self);
        self.al_source().parameter_i32(AL_LOOPING, looping as i32);
    }

    pub fn looping(&self) -> bool {
        check_audio!(self);
        self.al_source().get_parameter_i32(AL_LOOPING) != 0
    }

    pub fn set_pitch(&mut self, pitch: f32) {
        check_audio!(self);
        let src = self.source_mut();
        src.pitch = pitch;
        if let Some(pitch) = src.g_pitch {
            let speed = AUDIO.speed.load(Ordering::Relaxed);
            src.inner.parameter_f32(AL_PITCH, src.pitch * pitch * speed);
        }
    }

    pub fn pitch(&self) -> f32 {
        check_audio!(self);
        self.source().pitch
    }

    pub fn set_attenuation_distances(&self, reference: f32, max: f32) {
        check_audio!(self);
        let src = self.al_source();
        src.parameter_f32(AL_REFERENCE_DISTANCE, reference);
        src.parameter_f32(AL_MAX_DISTANCE, max);
    }

    pub fn attenuation_distances(&self) -> (f32, f32) {
        check_audio!(self);
        let src = self.al_source();
        (
            src.get_parameter_f32(AL_REFERENCE_DISTANCE),
            src.get_parameter_f32(AL_MAX_DISTANCE),
        )
    }

    pub fn set_rolloff(&self, rolloff: f32) {
        check_audio!(self);
        self.al_source().parameter_f32(AL_ROLLOFF_FACTOR, rolloff);
    }

    pub fn rolloff(&self) -> f32 {
        check_audio!(self);
        self.al_source().get_parameter_f32(AL_ROLLOFF_FACTOR)
    }

    pub fn set_air_absorption_factor(&self, value: f32) {
        check_audio!(self);
        self.al_source()
            .parameter_f32(AL_AIR_ABSORPTION_FACTOR, value);
    }
}
pub struct AudioBuilder {
    pos: Option<Vector2<f32>>,
    vel: Option<Vector2<f32>>,
    volume: f32,
    data: Option<AudioData>,
    path: Option<String>,
    ingame: bool,
    play: bool,
    looping: bool,
    groupid: Option<GroupRef>,
    group_volume: f32,
    group_pitch: Option<f32>,
    atype: AudioType,
}
impl AudioBuilder {
    pub const fn new(atype: AudioType) -> Self {
        AudioBuilder {
            pos: None,
            vel: None,
            volume: 1.,
            data: None,
            path: None,
            play: false,
            looping: false,
            ingame: false,
            groupid: None,
            group_volume: 1.0,
            group_pitch: Some(1.0),
            atype,
        }
    }

    pub fn position(mut self, pos: Option<Vector2<f32>>) -> Self {
        self.pos = pos;
        self
    }

    pub fn velocity(mut self, vel: Option<Vector2<f32>>) -> Self {
        self.vel = vel;
        self
    }

    pub fn path(mut self, path: Option<&str>) -> Self {
        self.path = path.map(|s| s.to_string());
        self.data = None;
        self
    }

    pub fn data(mut self, data: Option<AudioData>) -> Self {
        self.data = data;
        self.path = None;
        self
    }

    pub fn buffer(mut self, buffer: Arc<Buffer>) -> Self {
        self.data = Some(AudioData::Buffer(buffer));
        self.path = None;
        self
    }

    pub fn play(mut self, play: bool) -> Self {
        self.play = play;
        self
    }

    pub fn looping(mut self, looping: bool) -> Self {
        self.looping = looping;
        self
    }

    pub fn ingame(mut self, ingame: bool) -> Self {
        self.ingame = ingame;
        self
    }

    fn group_id(mut self, id: Option<GroupRef>) -> Self {
        self.groupid = id;
        self
    }

    fn group_volume(mut self, volume: f32) -> Self {
        self.group_volume = volume;
        self
    }

    fn group_pitch(mut self, pitch: Option<f32>) -> Self {
        self.group_pitch = pitch;
        self
    }

    fn build_static(&self, source: al::Source) -> Result<AudioStatic> {
        let audio = if let Some(path) = &self.path {
            let buf = Buffer::get_or_try_load(path)?;
            AudioStatic::new_buffer(&buf, source)?
        } else {
            AudioStatic::new(&self.data, source)?
        };
        Ok(audio)
    }

    fn build_stream(&self, source: al::Source) -> Result<AudioStream> {
        if let Some(path) = &self.path {
            AudioStream::from_path(path, source)
        } else {
            anyhow::bail!("Can only create AudioStream from paths");
        }
    }

    pub fn build(self) -> Result<AudioRef> {
        if AUDIO.disabled || SILENT.load(Ordering::Relaxed) {
            return Ok(thunderdome::Index::DANGLING.into());
        }
        let mut voices = AUDIO.voices.lock().unwrap();
        if voices.len() >= self.atype.priority_threshold() {
            return Ok(thunderdome::Index::DANGLING.into());
        }
        let source = al::Source::new()?;

        let looping = self.looping;
        let play = self.play;
        let mut audio = match self.atype {
            AudioType::Static => {
                let mut audio = self.build_static(source)?;
                audio.groupid = self.groupid;
                if self.groupid.is_some() {
                    audio.source.g_volume = self.group_volume;
                    audio.source.g_pitch = self.group_pitch;
                }
                Audio::Static(audio)
            }
            AudioType::LuaStatic => Audio::LuaStatic(self.build_static(source)?),
            AudioType::LuaStream => Audio::LuaStream(self.build_stream(source)?),
        };
        let stereo = audio.stereo();
        if let Some(pos) = self.pos {
            audio.set_ingame();
            audio.set_position(pos);
            if let Some(vel) = self.vel {
                audio.set_velocity(vel);
            }
            audio.set_air_absorption_factor(AUDIO.air_absorption.load(Ordering::Relaxed));
        } else if stereo {
            audio
                .al_source()
                .parameter_i32(AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
        } else {
            //if !stereo {
            audio.set_relative(true);
        }
        if self.ingame {
            audio.set_ingame();
        }
        audio.set_pitch(1.0);
        audio.set_volume(self.volume);
        if looping {
            audio.set_looping(true);
        }
        if play {
            audio.play();
        }
        let groupid = audio.groupid();
        let id: AudioRef = voices.insert(audio).into();
        drop(voices);
        if let Some(groupid) = groupid {
            match AUDIO.groups.lock().unwrap().get_mut(groupid.0) {
                Some(group) => group.voices.push(id),
                None => warn!("Group not found"),
            }
        }
        Ok(id)
    }
}

#[derive(Debug, Default)]
struct Group {
    max: usize,
    volume: f32,
    pitch: f32,
    speed_affects: bool,
    ingame: bool,
    voices: Vec<AudioRef>,
}
impl Group {
    fn new(max: usize) -> Self {
        Group {
            max,
            volume: 1.0,
            pitch: 1.0,
            speed_affects: true,
            ingame: false,
            voices: Vec::new(),
        }
    }
}
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct GroupRef(thunderdome::Index);
impl GroupRef {
    pub fn new(max: usize) -> Self {
        GroupRef(AUDIO.groups.lock().unwrap().insert(Group::new(max)))
    }

    pub fn play_buffer(&self, buf: &Arc<Buffer>, looping: bool) -> Option<AudioRef> {
        let (g_volume, g_pitch, ingame) = {
            let groups = AUDIO.groups.lock().unwrap();
            let group = match groups.get(self.0) {
                Some(group) => group,
                None => {
                    warn!("Group not found!");
                    return None;
                }
            };

            if group.voices.len() >= group.max {
                return None;
            }
            (
                group.volume,
                group.speed_affects.then_some(group.pitch),
                group.ingame,
            )
        };

        match AudioBuilder::new(AudioType::Static)
            .buffer(buf.clone())
            .play(true)
            .ingame(ingame)
            .looping(looping)
            .group_id(Some(*self))
            .group_volume(g_volume)
            .group_pitch(g_pitch)
            .build()
        {
            Ok(v) => Some(v),
            Err(e) => {
                warn_err!(e);
                None
            }
        }
    }

    pub fn stop(&self) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        let voices = AUDIO.voices.lock().unwrap();
        for v in group.voices.drain(..) {
            if let Some(voice) = voices.get(v.0) {
                // Will get removed from voices too automatically
                // TODO fade out
                voice.stop();
            }
        }
        Ok(())
    }

    pub fn pause(&self) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        let voices = AUDIO.voices.lock().unwrap();
        for v in group.voices.iter() {
            if let Some(voice) = voices.get(v.0)
                && voice.is_playing()
            {
                voice.pause();
            }
        }
        if let Some(sfx) = &AUDIO.compression
            && sfx.source.inner.get_parameter_i32(AL_SOURCE_STATE) == AL_PLAYING
        {
            sfx.source.inner.play();
        }
        Ok(())
    }

    pub fn resume(&self) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        let mut voices = AUDIO.voices.lock().unwrap();
        for v in group.voices.iter() {
            if let Some(voice) = voices.get_mut(v.0)
                && voice.is_paused()
            {
                voice.play();
            }
        }
        if let Some(sfx) = &AUDIO.compression
            && sfx.source.inner.get_parameter_i32(AL_SOURCE_STATE) == AL_PAUSED
        {
            sfx.source.inner.play();
        }
        Ok(())
    }

    pub fn set_ingame(&self) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        group.ingame = true;
        let mut voices = AUDIO.voices.lock().unwrap();
        for v in group.voices.iter() {
            if let Some(voice) = voices.get_mut(v.0) {
                voice.set_ingame();
            }
        }
        Ok(())
    }

    fn reset_speed(group: &Group) {
        let speed = AUDIO.speed.load(Ordering::Relaxed);
        let mut voices = AUDIO.voices.lock().unwrap();
        let pitch = group.speed_affects.then_some(group.pitch);
        for v in group.voices.iter() {
            if let Some(voice) = voices.get_mut(v.0) {
                let src = voice.source_mut();
                src.g_pitch = pitch;
                if let Some(pitch) = src.g_pitch {
                    src.inner.parameter_f32(AL_PITCH, src.pitch * pitch * speed);
                } else {
                    src.inner.parameter_f32(AL_PITCH, 1.);
                }
            }
        }
    }

    pub fn set_speed_affects(&self, enable: bool) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        group.speed_affects = enable;
        Self::reset_speed(group);
        Ok(())
    }

    pub fn set_volume(&self, volume: f32) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        group.volume = volume;

        let master = AUDIO.volume.read().unwrap().volume;
        let mut voices = AUDIO.voices.lock().unwrap();
        let cvol = 1.0 - AUDIO.compression_gain.load(Ordering::Relaxed);
        for v in group.voices.iter() {
            if let Some(voice) = voices.get_mut(v.0) {
                let c = if voice.ingame() { cvol } else { 1.0 };
                let src = voice.source_mut();
                src.g_volume = group.volume;
                src.inner
                    .parameter_f32(AL_GAIN, c * master * src.volume * src.g_volume);
            }
        }
        Ok(())
    }

    pub fn set_pitch(&self, pitch: f32) -> Result<()> {
        let mut groups = AUDIO.groups.lock().unwrap();
        let group = match groups.get_mut(self.0) {
            Some(group) => group,
            None => anyhow::bail!("Group not found"),
        };
        group.pitch = pitch;
        Self::reset_speed(group);
        Ok(())
    }

    fn into_ptr(self) -> *const c_void {
        unsafe { std::mem::transmute::<thunderdome::Index, *const c_void>(self.0) }
    }

    fn from_ptr(ptr: *const c_void) -> Self {
        unsafe {
            Self(std::mem::transmute::<*const c_void, thunderdome::Index>(
                ptr,
            ))
        }
    }
}

#[derive(Clone, PartialEq, Copy, Debug)]
pub struct Volume {
    /// Logarithmic volume
    volume: f32,
    /// Linear volume
    volume_lin: f32,
}
impl Default for Volume {
    fn default() -> Self {
        Self::new()
    }
}
impl Volume {
    pub fn new() -> Self {
        Self {
            volume: 1.,
            volume_lin: 1.,
        }
    }
}

enum Message {
    Remove(AudioRef),
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

#[allow(dead_code)]
pub struct System {
    disabled: bool,
    device: al::Device,
    context: al::Context,
    freq: i32,
    speed: AtomicF32,
    volume: RwLock<Volume>,
    voices: Mutex<Arena<Audio>>,
    groups: Mutex<Arena<Group>>,
    compression: Option<AudioStatic>,
    compression_gain: AtomicF32,
    listener_pos: RwLock<Vector2<f32>>,
    air_absorption: AtomicF32,
}
impl System {
    pub fn new() -> Result<Self> {
        let nosound = unsafe { naevc::conf.nosound != 0 };
        if nosound {
            debug!("{}", gettext("Sound disabled."));
            return Ok(Self {
                disabled: true,
                device: al::Device(AtomicPtr::new(core::ptr::null_mut())),
                context: al::Context(AtomicPtr::new(core::ptr::null_mut())),
                freq: 0,
                speed: AtomicF32::new(1.0),
                volume: Default::default(),
                voices: Default::default(),
                groups: Default::default(),
                compression: None,
                compression_gain: AtomicF32::new(0.0),
                listener_pos: RwLock::new(Default::default()),
                air_absorption: AtomicF32::new(0.0),
            });
        }

        let device = al::Device::new(None)?;

        // Doesn't distinguish between mono and stereo sources
        let mut attribs: Vec<ALint> = vec![ALC_MONO_SOURCES, MAX_SOURCES as ALint];
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
        let has_direct_channels = direct_channels_remix::supported();
        let has_callback_buffer = match callback_buffer::init() {
            Ok(()) => true,
            Err(e) => {
                warn_err!(e);
                false
            }
        };

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
        if has_direct_channels {
            extensions.push("direct_channels_remix".to_string());
        }
        if has_callback_buffer {
            extensions.push("callback_buffer".to_string());
        }
        debugx!(gettext("   with {}"), extensions.join(", "));
        debug!("");

        // Create the compression sound here
        fn load_compression() -> Result<AudioStatic> {
            let data = Arc::new(Buffer::from_path("snd/sounds/compression")?);
            // LuaStatic won't get cleaned up if stopped
            let comp = AudioStatic::new(&Some(AudioData::Buffer(data)), al::Source::new()?)?;
            // Should loop infinitely
            let als = &comp.source.inner;
            als.parameter_i32(AL_LOOPING, AL_TRUE.into());
            if comp.stereo {
                als.parameter_i32(AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
            } else {
                als.parameter_i32(AL_SOURCE_RELATIVE, AL_TRUE.into());
            }
            Ok(comp)
        }
        let compression = load_compression().ok();

        // Sound is currently in "screen" coordinates, and doesn't react to ship turning
        // Would probably have to be relative to heading for accessibility support (when enabled)
        let ori = [0.0, 1.0, 0.0, 0.0, 0.0, 1.0];
        unsafe {
            alListenerfv(AL_ORIENTATION, ori.as_ptr());
        }

        Ok(System {
            disabled: false,
            device,
            context,
            volume: RwLock::new(Volume::new()),
            speed: AtomicF32::new(1.0),
            freq,
            voices: Mutex::new(Default::default()),
            groups: Mutex::new(Default::default()),
            compression,
            compression_gain: AtomicF32::new(0.0),
            listener_pos: RwLock::new(Default::default()),
            air_absorption: AtomicF32::new(0.0),
        })
    }

    pub fn set_volume(&self, volume: f32) {
        let master = {
            let mut vol = self.volume.write().unwrap();
            vol.volume_lin = volume;
            if vol.volume_lin > 0.0 {
                vol.volume = 1.0 / 2.0_f32.powf((1.0 - volume) * 8.0);
            } else {
                vol.volume = 0.0;
            }
            vol.volume
        };

        let cvol = 1.0 - self.compression_gain.load(Ordering::Relaxed);
        for (_, v) in self.voices.lock().unwrap().iter() {
            match v {
                Audio::Static(this) | Audio::LuaStatic(this) => {
                    let c = if this.ingame { cvol } else { 1.0 };
                    this.source
                        .inner
                        .parameter_f32(AL_GAIN, c * master * v.volume())
                }
                _ => (),
            }
        }
    }

    pub fn set_speed(&self, speed: f32) {
        let master = self.volume.read().unwrap().volume;

        // Handle compression
        let c = ((speed - 2.0) / 10.0).clamp(0.0, 1.0);
        if let Some(sfx) = &self.compression {
            let als = &sfx.source.inner;
            if c > 0. {
                als.parameter_f32(AL_GAIN, master * c);
                if als.get_parameter_i32(AL_SOURCE_STATE) != AL_PLAYING {
                    als.play();
                }
            } else if self.compression_gain.load(Ordering::Relaxed) > 0.0 {
                als.parameter_f32(AL_GAIN, 0.0);
                als.stop();
            }
        }
        self.compression_gain.store(c, Ordering::Relaxed);
        self.speed.store(speed, Ordering::Relaxed);

        // Update the rest of the voices
        let cvol = 1.0 - c;
        for (_, v) in self.voices.lock().unwrap().iter_mut() {
            let src = v.source();
            if let Some(pitch) = src.g_pitch {
                src.inner.parameter_f32(AL_PITCH, src.pitch * pitch * speed);
            }
            let c = if v.ingame() { cvol } else { 1.0 };
            src.inner
                .parameter_f32(AL_GAIN, c * master * src.volume * src.g_volume);
        }
    }

    pub fn set_air_absorption_factor(&self, factor: f32) {
        let voices = self.voices.lock().unwrap();
        for (_, v) in voices.iter() {
            if v.ingame() {
                v.set_air_absorption_factor(factor);
            }
        }
        self.air_absorption.store(factor, Ordering::Relaxed);
    }

    pub fn play_buffer(&self, buf: &Arc<Buffer>) -> Result<AudioRef> {
        AudioBuilder::new(AudioType::Static)
            .buffer(buf.clone())
            .play(true)
            .build()
    }

    pub fn pause(&self) {
        let voices = self.voices.lock().unwrap();
        for (_, v) in voices.iter() {
            match v {
                Audio::Static(_this) | Audio::LuaStatic(_this) => {
                    if v.is_playing() {
                        v.pause();
                    }
                }
                _ => (),
            }
        }
    }

    pub fn resume(&self) {
        for (_, v) in self.voices.lock().unwrap().iter_mut() {
            match v {
                Audio::Static(_this) | Audio::LuaStatic(_this) => {
                    if v.is_paused() {
                        v.play()
                    }
                }
                _ => (),
            }
        }
    }

    pub fn stop(&self) {
        let mut voices = AUDIO.voices.lock().unwrap();
        voices.retain(|_, v| match v {
            Audio::Static(_this) => {
                v.stop();
                // When stopped should be dropped
                false
            }
            Audio::LuaStatic(_this) => {
                v.stop();
                // LuaStatic exist until garbage collected
                true
            }
            _ => true,
        });
    }

    pub fn update_listener(&self, pos: Vector2<f32>, vel: Vector2<f32>) {
        set_listener_position(Vector3::new(pos.x, pos.y, 0.0));
        set_listener_velocity(Vector3::new(vel.x, vel.y, 0.0));
        *AUDIO.listener_pos.write().unwrap() = pos;
    }

    pub fn execute_messages(&self) {
        let mut messages = MESSAGES.lock().unwrap();
        if messages.is_empty() {
            return;
        }
        // We always lock groups first
        let mut groups = AUDIO.groups.lock().unwrap();
        let mut voices = AUDIO.voices.lock().unwrap();
        for m in messages.drain(..) {
            match m {
                Message::Remove(id) => {
                    voices.remove(id.0);
                }
                Message::SourceStopped(id) => {
                    if let Some((vid, v)) = voices.iter().find(|(_, x)| x.al_source().raw() == id) {
                        // Remove from group too if it has one
                        if let Some(gid) = v.groupid() {
                            let group = &mut groups[gid.0];
                            if let Some(gvid) = group.voices.iter().position(|x| x.0 == vid) {
                                group.voices.remove(gvid);
                            }
                        }
                        // Finally remove the voice, but only if it is actually fine to remove
                        if let Some(voice) = voices.get(vid)
                            && let Audio::Static(_) = voice
                        {
                            voices.remove(vid);
                        }
                    }
                }
            }
        }
    }
}
pub static SILENT: AtomicBool = AtomicBool::new(false);
pub static AUDIO: LazyLock<System> = LazyLock::new(|| System::new().unwrap());
pub static CODECS: LazyLock<symphonia::core::codecs::CodecRegistry> = LazyLock::new(|| {
    let mut codec_registry = symphonia::core::codecs::CodecRegistry::new();
    symphonia::default::register_enabled_codecs(&mut codec_registry);
    codec_registry.register_all::<symphonia_adapter_libopus::OpusDecoder>();
    codec_registry
});

pub fn init() -> Result<()> {
    let _ = &*AUDIO;
    Ok(())
}

#[derive(Debug, Clone, Copy, PartialEq, derive_more::From, mlua::FromLua)]
pub struct AudioRef(thunderdome::Index);
impl AudioRef {
    fn try_clone(&self) -> Result<Self> {
        if AUDIO.disabled {
            return Ok(Self(self.0));
        }
        let mut voices = AUDIO.voices.lock().unwrap();
        let audio = match voices.get(self.0) {
            Some(audio) => {
                if voices.len() >= audio.audio_type().priority_threshold() {
                    return Ok(thunderdome::Index::DANGLING.into());
                }
                let source = al::Source::new()?;
                audio.try_clone(source)?
            }
            None => anyhow::bail!("Audio not found"),
        };
        Ok(voices.insert(audio).into())
    }

    /// Like call, but allows specifying a default value.
    pub fn call_or<S, R>(&self, f: S, d: R) -> anyhow::Result<R>
    where
        S: Fn(&Audio) -> R,
    {
        if AUDIO.disabled {
            return Ok(d);
        }
        let audio = AUDIO.voices.lock().unwrap();
        match audio.get(self.0) {
            Some(audio) => Ok(f(audio)),
            None => anyhow::bail!("Audio not found"),
        }
    }

    /// Tries to call a function on a voice, returning default value if not available.
    pub fn call<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&Audio) -> R,
        R: std::default::Default,
    {
        self.call_or(f, Default::default())
    }

    /// Same as call but allows mutable access.
    pub fn call_mut<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&mut Audio) -> R,
        R: std::default::Default,
    {
        if AUDIO.disabled {
            return Ok(Default::default());
        }
        let mut audio = AUDIO.voices.lock().unwrap();
        match audio.get_mut(self.0) {
            Some(audio) => Ok(f(audio)),
            None => anyhow::bail!("Audio not found"),
        }
    }

    pub fn into_ptr(&self) -> *const c_void {
        unsafe { std::mem::transmute::<Self, *const c_void>(*self) }
    }

    pub fn from_ptr(ptr: *const c_void) -> Self {
        unsafe { std::mem::transmute::<*const c_void, AudioRef>(ptr) }
    }
}

pub struct LuaAudioRef {
    pub audio: AudioRef,
    pub remove_on_drop: bool,
}
impl Drop for LuaAudioRef {
    fn drop(&mut self) {
        if self.remove_on_drop {
            MESSAGES.lock().unwrap().push(Message::Remove(self.audio));
        }
    }
}
impl LuaAudioRef {
    fn try_clone(&self) -> Result<Self> {
        let audio = self.audio.try_clone()?;
        Ok(Self {
            audio,
            remove_on_drop: self.remove_on_drop,
        })
    }

    /// Like call, but allows specifying a default value.
    pub fn call_or<S, R>(&self, f: S, d: R) -> anyhow::Result<R>
    where
        S: Fn(&Audio) -> R,
    {
        self.audio.call_or(f, d)
    }

    /// Tries to call a function on a voice, returning default value if not available.
    pub fn call<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&Audio) -> R,
        R: std::default::Default,
    {
        self.audio.call(f)
    }

    /// Same as call but allows mutable access.
    pub fn call_mut<S, R>(&self, f: S) -> anyhow::Result<R>
    where
        S: Fn(&mut Audio) -> R,
        R: std::default::Default,
    {
        self.audio.call_mut(f)
    }
}

/*@
 * @brief Lua bindings to interact with audio.
 *
 * @luamod audiodata
 */
impl UserData for AudioData {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Gets a string representation of an audiodata.
         *
         *    @luatparam AudioData data AudioData to get string representation of.
         *    @luatreturn string String representation of the data.
         * @luafunc __tostring
         */
        methods.add_meta_method(MetaMethod::ToString, |_, this: &Self, ()| match this {
            AudioData::Buffer(ab) => Ok(format!("AudioData( {} )", ab.name.display())),
        });
        /*@
         * @brief Creates a new audio data;
         *
         *    @luatparam string|File data Data to load the audio from.
         *    @luatreturn AudioData New audio data.
         * @luafunc new
         */
        methods.add_function("new", |_, filename: String| -> mlua::Result<Self> {
            let buf = Buffer::get_or_try_load(&filename)?;
            Ok(AudioData::Buffer(buf))
        });
        /*@
         * @brief Gets the length of the Audio data.
         *
         *    @luatparam AudioData data Data to get duration of.
         *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples"
         * indicating the type to report.
         *    @luatreturn number Duration of the source or nil on error.
         * @luafunc getDuration
         */
        methods.add_method(
            "getDuration",
            |_, this, samples: bool| -> mlua::Result<f32> {
                match this {
                    AudioData::Buffer(ab) => Ok(ab.duration(match samples {
                        true => AudioSeek::Samples,
                        false => AudioSeek::Seconds,
                    })),
                }
            },
        );
        /*@
         * @brief Gets the number of channels of the Audio data.
         *
         *    @luatparam AudioData data Data to get the number of channels of.
         *    @luatreturn number Number of channels of the data.
         * @luafunc getChannels
         */
        methods.add_method("getChannels", |_, this, ()| -> mlua::Result<u8> {
            match this {
                AudioData::Buffer(ab) => Ok(ab.channels()),
            }
        });
        /*@
         * @brief Gets the number of samples of the Audio data.
         *
         *    @luatparam AudioData data Data to get the number of samples of.
         *    @luatreturn number Number of sampless of the data.
         * @luafunc getSampleCount
         */
        methods.add_method("getSampleCount", |_, this, ()| -> mlua::Result<f32> {
            match this {
                AudioData::Buffer(ab) => Ok(ab.sample_count()),
            }
        });
        /*@
         * @brief Gets the sample rate of the Audio data.
         *
         *    @luatparam AudioData data Data to get the sample rate of.
         *    @luatreturn number The sample rate of the data.
         * @luafunc getSampleRate
         */
        methods.add_method("getSampleRate", |_, this, ()| -> mlua::Result<u32> {
            match this {
                AudioData::Buffer(ab) => Ok(ab.frequency()),
            }
        });
    }
}

/*@
 * @brief Lua bindings to interact with audio.
 *
 * @luamod audio
 */
impl UserData for LuaAudioRef {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Gets a string representation of an audio.
         *
         *    @luatparam Audio audio Audio to get string representation of.
         *    @luatreturn string String representation of the audio.
         * @luafunc __tostring
         */
        methods.add_meta_method(MetaMethod::ToString, |_, this: &Self, ()| {
            Ok(this.call(|this| match this {
                Audio::Static(this) | Audio::LuaStatic(this) => {
                    let path = match &this.data {
                        Some(AudioData::Buffer(buffer)) => &buffer.name,
                        None => &PathBuf::new(),
                    };
                    format!("AudioStatic( {} )", path.display(),)
                }
                Audio::LuaStream(this) => format!("AudioStream( {} )", this.path.display()),
            })?)
        });
        /*@
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
            |_,
             (val, streaming): (Either<String, UserDataRef<AudioData>>, bool)|
             -> mlua::Result<Self> {
                let audio = match val {
                    Either::Left(val) => AudioBuilder::new(match streaming {
                        true => AudioType::LuaStream,
                        false => AudioType::LuaStatic,
                    })
                    .path(Some(&val))
                    .build()?,
                    Either::Right(val) => {
                        if streaming {
                            return Err(mlua::Error::RuntimeError(
                                "streaming not supported for AudioData".to_string(),
                            ));
                        }
                        AudioBuilder::new(AudioType::LuaStatic)
                            .data(Some(val.clone()))
                            .build()?
                    }
                };
                Ok(Self {
                    audio,
                    remove_on_drop: true,
                })
            },
        );
        /*@
         * @brief Clones an existing audio source.
         *
         *    @luatparam Audio source Audio source to clone.
         *    @luatreturn Audio New audio corresponding to the data.
         * @luafunc clone
         */
        methods.add_method("clone", |_, this, ()| -> mlua::Result<Self> {
            Ok(this.try_clone()?)
        });
        /*@
         * @brief Plays a source.
         *
         *    @luatparam Audio source Source to play.
         * @luafunc play
         */
        methods.add_method("play", |_, this, ()| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.play();
            })?;
            Ok(())
        });
        /*@
         * @brief Checks to see if a source is playing.
         *
         *    @luatparam Audio source Source to check to see if is playing.
         *    @luatreturn boolean Whether or not the source is playing.
         * @luafunc isPlaying
         */
        methods.add_method("isPlaying", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_playing())?)
        });
        /*@
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
        /*@
         * @brief Checks to see if a source is paused.
         *
         *    @luatparam Audio source Source to check to see if is paused.
         *    @luatreturn boolean Whether or not the source is paused.
         * @luafunc isPaused
         */
        methods.add_method("isPaused", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_paused())?)
        });
        /*@
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
        /*@
         * @brief Checks to see if a source is stopped.
         *
         *    @luatparam Audio source Source to check to see if is stopped.
         *    @luatreturn boolean Whether or not the source is stopped.
         * @luafunc isStopped
         */
        methods.add_method("isStopped", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.is_stopped())?)
        });
        /*@
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
        /*@
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
        /*@
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
        /*@
         * @brief Gets the length of a source.
         *
         *    @luatparam Audio source Source to get duration of.
         *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples"
         * indicating the type to report.
         *    @luatreturn number Duration of the source or nil on error.
         * @luafunc getDuration
         */
        methods.add_method(
            "getDuration",
            |_, this, samples: bool| -> mlua::Result<Option<f32>> {
                Ok(this.call(|this| match this {
                    Audio::Static(this) | Audio::LuaStatic(this) => {
                        this.data.as_ref().map(|AudioData::Buffer(buffer)| {
                            buffer.duration(match samples {
                                true => AudioSeek::Samples,
                                false => AudioSeek::Seconds,
                            })
                        })
                    }
                    Audio::LuaStream(_this) => None,
                })?)
            },
        );
        /*@
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
        /*@
         * @brief Gets the volume of a source.
         *
         *    @luatparam[opt] Audio source Source to get volume of.
         *    @luatreturn number Volume the source is set to.
         * @luafunc getVolume
         */
        methods.add_method("getVolume", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.volume())?)
        });
        /*@
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
        /*@
         * @brief Gets whether a source is relative or not.
         *
         *    @luatreturn boolean relative Whether or not to the source is relative.
         * @luafunc isRelative
         */
        methods.add_method("isRelative", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.relative())?)
        });
        /*@
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
            |_, this, (x, y): (f32, f32)| -> mlua::Result<()> {
                let vec: Vector2<f32> = Vector2::new(x, y);
                this.call_mut(|this| {
                    this.set_position(vec);
                    this.set_ingame();
                })?;
                Ok(())
            },
        );
        /*@
         * @brief Gets the position of a source.
         *
         *    @luatparam Audio source Source to get position of.
         *    @luatreturn number X position.
         *    @luatreturn number Y position.
         *    @luatreturn number Z position.
         * @luafunc getPosition
         */
        methods.add_method("getPosition", |_, this, ()| -> mlua::Result<(f32, f32)> {
            let pos = this.call(|this| this.position())?;
            Ok((pos.x, pos.y))
        });
        /*@
         * @brief Sets the velocity of a source.
         *
         *    @luatparam Audio source Source to set velocity of.
         *    @luatparam number x X velocity.
         *    @luatparam number y Y velocity.
         * @luafunc setVelocity
         */
        methods.add_method(
            "setVelocity",
            |_, this, (x, y): (f32, f32)| -> mlua::Result<()> {
                let vec = Vector2::new(x, y);
                this.call_mut(|this| {
                    this.set_velocity(vec);
                    this.set_ingame();
                })?;
                Ok(())
            },
        );
        /*@
         * @brief Gets the velocity of a source.
         *
         *    @luatparam Audio source Source to get velocity of.
         *    @luatreturn number X velocity.
         *    @luatreturn number Y velocity.
         * @luafunc getVelocity
         */
        methods.add_method("getVelocity", |_, this, ()| -> mlua::Result<(f32, f32)> {
            let vel = this.call(|this| this.velocity())?;
            Ok((vel.x, vel.y))
        });
        /*@
         * @brief Sets a source to be "ingame" or affected by parameters like air absorption, game
         * speed, etc.
         *
         *    @luatparam Audio source Source to set looping state of.
         *    @luatparam boolean enable Whether or not the source should be set to
         * looping.
         * @luafunc setLooping
         */
        methods.add_method("setIngame", |_, this, ()| -> mlua::Result<()> {
            this.call_mut(|this| {
                this.set_ingame();
            })?;
            Ok(())
        });
        /*@
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
        /*@
         * @brief Gets the looping state of a source.
         *
         *    @luatparam Audio source Source to get looping state of.
         *    @luatreturn boolean Whether or not the source is looping.
         * @luafunc isLooping
         */
        methods.add_method("isLooping", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.call(|this| this.looping())?)
        });
        /*@
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
        /*@
         * @brief Gets the pitch of a source.
         *
         *    @luatparam Audio source Source to get pitch of.
         *    @luatreturn number Pitch of the source.
         * @luafunc getPitch
         */
        methods.add_method("getPitch", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.pitch())?)
        });
        /*@
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
        /*@
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
        /*@
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
        /*@
         * @brief Gets the rolloff factor.
         *
         *    @luatreturn number Rolloff factor or 0. if sound is disabled.
         * @luafunc getRolloff
         */
        methods.add_method("getRolloff", |_, this, ()| -> mlua::Result<f32> {
            Ok(this.call(|this| this.rolloff())?)
        });
        /*@
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
                this.call_or(
                    |this| {
                        let slot = if enable {
                            let lock = EFX_LIST.lock().unwrap();
                            let efxid =
                                match binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| {
                                    &e.name
                                }) {
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
                        this.al_source().parameter_3_i32(
                            AL_AUXILIARY_SEND_FILTER,
                            slot,
                            0,
                            AL_FILTER_NULL,
                        );

                        Ok(true)
                    },
                    Ok(true),
                )?
            },
        );
        /*@
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
        /*@
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
        /*@
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
        /*@
         * @brief Allows setting the air absorption factor.
         *
         *    @luatparam[opt=0] number absorb Air absorption factor.
         * @luafunc setAirAbsorption
         */
        methods.add_function(
            "setAirAbsorption",
            |_, absorb: Option<f32>| -> mlua::Result<()> {
                AUDIO.set_air_absorption_factor(absorb.unwrap_or(0.0));
                Ok(())
            },
        );
        /*@
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

pub fn open_audiodata(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<AudioData>()?)
}

pub fn open_audio(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<LuaAudioRef>()?)
}

// Here be C API
use std::ffi::{CStr, c_char, c_double, c_int, c_void};

// We assume that the index can be cast to a pointer for C to not complain
// This should hold on 64 bit platforms
static_assertions::assert_eq_size!(AudioRef, *const c_void);
static_assertions::assert_eq_size!(GroupRef, *const c_void);

#[unsafe(no_mangle)]
pub extern "C" fn sound_get(name: *const c_char) -> *const Buffer {
    if AUDIO.disabled {
        return std::ptr::null();
    }
    if name.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let name = unsafe { CStr::from_ptr(name).to_string_lossy() };
    match Buffer::get_or_try_load(format!("snd/sounds/{name}")) {
        Ok(buffer) => Arc::into_raw(buffer),
        Err(e) => {
            warn_err!(e);
            std::ptr::null()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_getLength(sound: *const Buffer) -> c_double {
    if AUDIO.disabled {
        return 0.0;
    }
    if sound.is_null() {
        warn!("recieved NULL");
        return 0.0;
    }
    let sound = unsafe { &*sound };
    sound.duration(AudioSeek::Seconds) as c_double
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_play(sound: *const Buffer) -> *const c_void {
    if AUDIO.disabled {
        return std::ptr::null();
    }
    if sound.is_null() {
        warn!("recieved NULL");
        return std::ptr::null();
    }
    let sound = unsafe {
        Arc::increment_strong_count(sound);
        Arc::from_raw(sound)
    };
    match AUDIO.play_buffer(&sound) {
        Ok(audioref) => audioref.into_ptr(),
        Err(e) => {
            warn_err!(e);
            std::ptr::null()
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_playPos(
    sound: *const Buffer,
    px: c_double,
    py: c_double,
    vx: c_double,
    vy: c_double,
) -> *const c_void {
    if AUDIO.disabled {
        return std::ptr::null();
    }
    if sound.is_null() {
        return std::ptr::null();
    }

    let pos = Vector2::new(px as f32, py as f32);
    if (pos - *AUDIO.listener_pos.read().unwrap()).norm_squared() > MAX_DISTANCE * MAX_DISTANCE {
        return std::ptr::null();
    }

    let sound = unsafe {
        Arc::increment_strong_count(sound);
        Arc::from_raw(sound)
    };

    let voice = match AudioBuilder::new(AudioType::Static)
        .buffer(sound.clone())
        .position(Some(pos))
        .velocity(Some(Vector2::new(vx as f32, vy as f32)))
        .play(true)
        .build()
    {
        Ok(v) => v,
        Err(e) => {
            warn_err!(e);
            return std::ptr::null();
        }
    };
    voice.into_ptr()
}

macro_rules! get_voice {
    ($voice: ident) => {{
        if $voice.is_null() || AUDIO.disabled {
            return Default::default();
        }
        AudioRef::from_ptr($voice)
    }};
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_stop(voice: *const c_void) {
    let index = get_voice!(voice);
    let _ = index.call(|voice| voice.stop());
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
    let pos = Vector2::new(px as f32, py as f32);
    *AUDIO.listener_pos.write().unwrap() = pos;
    let _ = index.call(|voice| {
        voice.set_position(pos);
        voice.set_velocity(Vector2::new(vx as f32, vy as f32));
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_updateListener(
    _dir: c_double,
    px: c_double,
    py: c_double,
    vx: c_double,
    vy: c_double,
) {
    if AUDIO.disabled {
        return;
    }
    AUDIO.update_listener(
        Vector2::new(px as f32, py as f32),
        Vector2::new(vx as f32, vy as f32),
    );
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_update(dt: c_double) -> i32 {
    AUDIO.execute_messages();
    unsafe {
        naevc::music_update(dt);
    }
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pause() {
    AUDIO.pause();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_resume() {
    AUDIO.resume();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_volume(volume: c_double) {
    AUDIO.set_volume(volume as f32);
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
    AUDIO.stop();
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_setSpeed(speed: c_double) {
    AUDIO.set_speed(speed as f32);
}

macro_rules! get_group {
    ($group: ident) => {{
        if $group.is_null() || AUDIO.disabled {
            return Default::default();
        }
        GroupRef::from_ptr($group)
    }};
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_createGroup(size: c_int) -> *const c_void {
    GroupRef::new(size as usize).into_ptr()
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_playGroup(
    group: *const c_void,
    sound: *const Buffer,
    once: c_int,
) -> *const c_void {
    if sound.is_null() {
        return std::ptr::null();
    }
    let sound = unsafe {
        Arc::increment_strong_count(sound);
        Arc::from_raw(sound)
    };

    let groupid = GroupRef::from_ptr(group);
    match groupid.play_buffer(&sound, once == 0) {
        Some(v) => v.into_ptr(),
        None => std::ptr::null(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_stopGroup(group: *const c_void) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.stop() {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pauseGroup(group: *const c_void) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.pause() {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_resumeGroup(group: *const c_void) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.resume() {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_speedGroup(group: *const c_void, enable: c_int) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.set_speed_affects(enable != 0) {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_volumeGroup(group: *const c_void, volume: c_double) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.set_volume(volume as f32) {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_pitchGroup(group: *const c_void, pitch: c_double) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.set_pitch(pitch as f32) {
        warn_err!(e);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_ingameGroup(group: *const c_void) {
    let groupid = get_group!(group);
    if let Err(e) = groupid.set_ingame() {
        warn_err!(e);
    }
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
    AUDIO.disabled as c_int
}

#[unsafe(no_mangle)]
pub extern "C" fn sound_set_disabled(disable: c_int) {
    SILENT.store(disable != 0, Ordering::Relaxed);
}

// TODO fails to link with meson, but as a test it works
/*
#[test]
pub fn test_flac_vs_opus () {
    use std::process::{Command, Stdio};

    ndata::setup().unwrap();

    // Set up audio
    let attribs: Vec<ALint> = vec![0, 0];
    let device = al::Device::new(None).unwrap();
    let context = al::Context::new(&device, &attribs).unwrap();
    context.set_current().unwrap();

    let path = Buffer::get_valid_path( "snd/sounds/nav" ).unwrap();
    let path = Path::new("../assets").join(path);

    let temp = ndata::physfs::get_write_dir();
    let outfile1 = Path::new( "naev_audio_test.flac" );
    let outfile2 = Path::new( "naev_audio_test.opus" );
    let out1 = temp.join( &outfile1 );
    let out2 = temp.join( &outfile2 );

    assert!( Command::new("ffmpeg")
        .arg("-loglevel")
        .arg("error")
        .arg("-i")
        .arg(&path)
        .arg("-y")
        .arg("-c:a")
        .arg("flac")
        .arg(&out1)
        .stdout(Stdio::null())
        .spawn().unwrap()
        .wait().unwrap().success() );

    assert!( Command::new("ffmpeg")
        .arg("-loglevel")
        .arg("error")
        .arg("-i")
        .arg(&path)
        .arg("-y")
        .arg("-c:a")
        .arg("libopus")
        .arg("-b:a")
        .arg("256k")
        .arg("-frame_duration")
        .arg("40")
        .arg("-ar")
        .arg("48000")
        .arg(&out2)
        .stdout(Stdio::null())
        .spawn().unwrap()
        .wait().unwrap().success() );

    let data1 = BufferData::from_path( &outfile1 ).unwrap();
    let data2 = BufferData::from_path( &outfile2 ).unwrap();
    assert_eq!( data1.data.len(), data2.data.len() );

    let mut err = 0.0;
    for (sa, sb) in data1.data.iter().zip( data2.data.iter() ) {
        err += (sa-sb).powf(2.0);
    }
    err /= (data1.data.len() + data2.data.len()) as f32;

    assert!( err < 1e-3 );
}
*/
