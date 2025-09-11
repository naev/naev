#![allow(dead_code, unused_variables)]
mod debug;
mod efx;
mod openal;
use crate::efx::*;
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::*;
use naev_core::utils::{binary_search_by_key_ref, sort_by_key_ref};

use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use mlua::{MetaMethod, UserData, UserDataMethods};
use nalgebra::Vector3;
use std::sync::{Arc, LazyLock, Mutex, RwLock};

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
        debug::object_label(debug::AL_FILTER, effect.raw(), name);
        let slot = AuxiliaryEffectSlot::new()?;
        debug::object_label(debug::AL_AUXILIARY_EFFECT_SLOT, slot.raw(), name);
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
    track_gain_db: f32,
    track_peak: f32,
    buffer: al::Buffer,
}
impl AudioBuffer {
    fn from_path(path: &str) -> Result<Self> {
        use symphonia::core::audio::{AudioBuffer, Channels, Signal};
        use symphonia::core::codecs::{CODEC_TYPE_NULL, CodecParameters, DecoderOptions};
        use symphonia::core::errors::Error;
        use symphonia::core::formats::{FormatOptions, FormatReader};
        use symphonia::core::io::MediaSourceStream;
        use symphonia::core::meta::{MetadataOptions, StandardTagKey, Tag, Value};
        use symphonia::core::probe::Hint;
        use symphonia::core::sample::{Sample, SampleFormat};

        let src = ndata::open(path)?;
        let mss = MediaSourceStream::new(Box::new(src), Default::default());

        let mut hint = Hint::new();
        if let Some(ext) = std::path::Path::new(path)
            .extension()
            .and_then(|s| s.to_str())
        {
            hint.with_extension(ext);
        }

        // Probe the media source.
        let meta_opts: MetadataOptions = Default::default();
        let fmt_opts: FormatOptions = Default::default();
        let probed = symphonia::default::get_probe().format(&hint, mss, &fmt_opts, &meta_opts)?;

        // Get the instantiated format reader.
        let mut format = probed.format;

        let (track_gain_db, track_peak) = {
            let mut track_gain_db = 0.;
            let mut track_peak = 1.;
            if let Some(md) = format.metadata().current() {
                for t in md.tags() {
                    fn tag_to_f32(t: &Tag) -> Result<f32> {
                        match t.value {
                            Value::Float(val) => Ok(val as f32),
                            _ => anyhow::bail!("tag is not a float"),
                        }
                    }
                    if let Some(key) = t.std_key {
                        match key {
                            StandardTagKey::ReplayGainTrackGain => match tag_to_f32(t) {
                                Ok(f) => {
                                    track_gain_db = f;
                                }
                                Err(e) => {
                                    warn_err!(e);
                                }
                            },
                            StandardTagKey::ReplayGainTrackPeak => match tag_to_f32(t) {
                                Ok(f) => {
                                    track_peak = f;
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
            (10.0_f32.powf(track_gain_db / 20.0), 1.0 / track_peak)
        };

        // Find the first audio track with a known (decodeable) codec.
        let track = format
            .tracks()
            .iter()
            .find(|t| t.codec_params.codec != CODEC_TYPE_NULL)
            .ok_or(anyhow::anyhow!("unsupported codec"))?;

        // Templated function to load samples of different types
        fn parse_data<T: Sample>(
            format: &mut Box<dyn FormatReader>,
            codec_params: &CodecParameters,
            track_id: u32,
            stereo: bool,
        ) -> Result<Vec<T>> {
            let dec_opts: DecoderOptions = Default::default();
            let mut decoder = symphonia::default::get_codecs().make(codec_params, &dec_opts)?;

            let mut buffers: Vec<AudioBuffer<T>> = vec![];
            loop {
                // Get the next packet from the media format.
                let packet = match format.next_packet() {
                    Ok(packet) => packet,
                    Err(Error::ResetRequired) => {
                        unimplemented!();
                    }
                    Err(err) => {
                        anyhow::bail!(err);
                    }
                };

                // If the packet does not belong to the selected track, skip over it.
                if packet.track_id() != track_id {
                    continue;
                }

                // Decode the packet into audio samples.
                match decoder.decode(&packet) {
                    Ok(decoded) => {
                        let spec = decoded.spec();
                        buffers.push(decoded.make_equivalent());
                    }
                    Err(Error::IoError(e)) => {
                        if e.kind() == std::io::ErrorKind::UnexpectedEof
                            && e.to_string() == "end of stream"
                        {
                            // End of File
                            break;
                        } else {
                            anyhow::bail!(e);
                        }
                    }
                    Err(e) => {
                        anyhow::bail!(e);
                    }
                }
            }
            let left = buffers.iter().flat_map(|b| b.chan(0).to_vec());
            let data: Vec<_> = match stereo {
                true => {
                    let right = buffers.iter().flat_map(|b| b.chan(1).to_vec());
                    left.chain(right).collect()
                }
                false => left.collect(),
            };
            Ok(data)
        }

        let codec = track.codec_params.clone();
        let rate = codec
            .sample_rate
            .ok_or(anyhow::anyhow!("no sampling rate"))?;
        let channels = codec.channels.ok_or(anyhow::anyhow!("no channels"))?;
        if !channels.contains(Channels::FRONT_LEFT) {
            anyhow::bail!("no mono channel");
        }
        let stereo = channels.contains(Channels::FRONT_RIGHT);
        let track_id = track.id;
        let buffer = match codec.sample_format {
            Some(fmt) => {
                let buffer = al::Buffer::new()?;
                match fmt {
                    SampleFormat::U8 | SampleFormat::S8 => {
                        let data = parse_data::<u8>(&mut format, &codec, track_id, stereo)?;
                        unsafe {
                            alBufferData(
                                buffer.raw(),
                                match stereo {
                                    true => AL_FORMAT_STEREO8,
                                    false => AL_FORMAT_MONO8,
                                },
                                data.as_ptr() as *const ALvoid,
                                (data.len() * std::mem::size_of::<u8>()) as i32,
                                rate as ALsizei,
                            );
                        }
                    }
                    // Just make F64 be F32
                    SampleFormat::F32 | SampleFormat::F64 => {
                        let data = parse_data::<f32>(&mut format, &codec, track_id, stereo)?;
                        unsafe {
                            alBufferData(
                                buffer.raw(),
                                match stereo {
                                    true => AL_FORMAT_STEREO_FLOAT32,
                                    false => AL_FORMAT_MONO_FLOAT32,
                                },
                                data.as_ptr() as *const ALvoid,
                                (data.len() * std::mem::size_of::<f32>()) as i32,
                                rate as ALsizei,
                            );
                        }
                    }
                    //SampleFormat::U16 | SampleFormat::S16 => {
                    _ => {
                        let data = parse_data::<i16>(&mut format, &codec, track_id, stereo)?;
                        unsafe {
                            alBufferData(
                                buffer.raw(),
                                match stereo {
                                    true => AL_FORMAT_STEREO16,
                                    false => AL_FORMAT_MONO16,
                                },
                                data.as_ptr() as *const ALvoid,
                                (data.len() * std::mem::size_of::<i16>()) as i32,
                                rate as ALsizei,
                            );
                        }
                    }
                }
                buffer
            }
            None => anyhow::bail!("no format!"),
        };

        // Set debug stuff
        debug::object_label(debug::AL_BUFFER, buffer.raw(), path);

        Ok(Self {
            name: String::from(path),
            buffer,
            track_gain_db,
            track_peak,
        })
    }

    pub fn duration(&self, unit: AudioSeek) -> f32 {
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
    }
}

#[derive(Clone, PartialEq, Copy, Eq, Debug)]
pub enum AudioSeek {
    Seconds,
    Samples,
}

#[derive(PartialEq, Debug)]
pub struct Audio {
    atype: AudioType,
    source: al::Source,
    slot: ALuint,
    volume: f32,
    buffer: Arc<AudioBuffer>,
}
impl Audio {
    pub fn new(buffer: Arc<AudioBuffer>) -> Result<Self> {
        let source = al::Source::new()?;
        debug::object_label(debug::AL_SOURCE, source.raw(), &buffer.name);
        Ok(Self {
            atype: AudioType::Static,
            source,
            slot: 0,
            volume: 1.0,
            buffer: buffer.clone(),
        })
    }

    pub fn from_path(path: &str, atype: AudioType) -> Result<Self> {
        // TODO streaming
        let buffer = Arc::new(AudioBuffer::from_path(path)?);
        Self::new(buffer)
    }

    /// Sets the sound to be in-game as opposed to a GUI or music track.
    pub fn ingame(&self) {
        let v = &self.source;

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

    fn is_state(&self, state: ALenum) -> bool {
        self.source.get_parameter_i32(AL_SOURCE_STATE) == state
    }

    pub fn play(&self) {
        unsafe {
            alSourcePlay(self.source.raw());
        }
    }

    pub fn is_playing(&self) -> bool {
        self.is_state(AL_PLAYING)
    }

    pub fn pause(&self) {
        unsafe {
            alSourcePause(self.source.raw());
        }
    }

    pub fn is_paused(&self) -> bool {
        self.is_state(AL_PAUSED)
    }

    pub fn stop(&self) {
        unsafe {
            alSourceStop(self.source.raw());
        }
    }

    pub fn is_stopped(&self) -> bool {
        self.is_state(AL_STOPPED)
    }

    pub fn rewind(&self) {
        unsafe {
            alSourceRewind(self.source.raw());
        }
    }

    pub fn seek(&self, offset: f32, unit: AudioSeek) {
        match unit {
            AudioSeek::Seconds => self.source.parameter_f32(AL_SEC_OFFSET, offset),
            AudioSeek::Samples => self.source.parameter_f32(AL_SAMPLE_OFFSET, offset),
        }
    }

    pub fn tell(&self, unit: AudioSeek) -> f32 {
        match unit {
            AudioSeek::Seconds => self.source.get_parameter_f32(AL_SEC_OFFSET),
            AudioSeek::Samples => self.source.get_parameter_f32(AL_SAMPLE_OFFSET),
        }
    }

    pub fn set_volume(&mut self, vol: f32) {
        let master = AUDIO.volume.read().unwrap().volume;
        self.source.parameter_f32(AL_GAIN, master * vol);
        self.volume = vol;
    }

    pub fn set_volume_raw(&mut self, vol: f32) {
        self.source.parameter_f32(AL_GAIN, vol);
        self.volume = vol;
    }

    pub fn volume(&self) -> f32 {
        self.volume
    }

    pub fn set_relative(&self, relative: bool) {
        self.source
            .parameter_i32(AL_SOURCE_RELATIVE, relative as i32);
    }

    pub fn relative(&self) -> bool {
        self.source.get_parameter_i32(AL_SOURCE_RELATIVE) != 0
    }

    pub fn set_position(&self, pos: Vector3<f32>) {
        self.source
            .parameter_3_f32(AL_POSITION, pos.x, pos.y, pos.z);
    }

    pub fn position(&self) -> Vector3<f32> {
        Vector3::from(self.source.get_parameter_3_f32(AL_POSITION))
    }

    pub fn set_velocity(&self, pos: Vector3<f32>) {
        self.source
            .parameter_3_f32(AL_VELOCITY, pos.x, pos.y, pos.z);
    }

    pub fn velocity(&self) -> Vector3<f32> {
        Vector3::from(self.source.get_parameter_3_f32(AL_VELOCITY))
    }

    pub fn set_looping(&self, looping: bool) {
        self.source.parameter_i32(AL_LOOPING, looping as i32);
    }

    pub fn looping(&self) -> bool {
        self.source.get_parameter_i32(AL_LOOPING) != 0
    }

    pub fn set_pitch(&self, pitch: f32) {
        self.source.parameter_f32(AL_PITCH, pitch);
    }

    pub fn pitch(&self) -> f32 {
        self.source.get_parameter_f32(AL_PITCH)
    }

    pub fn set_attenuation_distances(&self, reference: f32, max: f32) {
        self.source.parameter_f32(AL_REFERENCE_DISTANCE, reference);
        self.source.parameter_f32(AL_MAX_DISTANCE, max);
    }

    pub fn attenuation_distances(&self) -> (f32, f32) {
        (
            self.source.get_parameter_f32(AL_REFERENCE_DISTANCE),
            self.source.get_parameter_f32(AL_MAX_DISTANCE),
        )
    }

    pub fn set_rolloff(&self, rolloff: f32) {
        self.source.parameter_f32(AL_ROLLOFF_FACTOR, rolloff);
    }

    pub fn rolloff(&self) -> f32 {
        self.source.get_parameter_f32(AL_ROLLOFF_FACTOR)
    }
}

#[derive(Clone, PartialEq, Copy, Debug)]
pub struct AudioVolume {
    volume: f32,
    volume_lin: f32,
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

pub struct AudioSystem {
    device: al::Device,
    context: al::Context,

    freq: i32,
    output_limiter: bool,

    volume: RwLock<AudioVolume>,
}
impl AudioSystem {
    pub fn new() -> Result<Self> {
        let device = al::Device::new(None)?;

        let mut attribs: Vec<ALint> = vec![ALC_MONO_SOURCES, 512, ALC_STEREO_SOURCES, 32];
        let mut has_debug = if cfg!(debug_assertions) {
            let debug = device.is_extension_present(debug::ALC_EXT_DEBUG_NAME);
            if debug {
                attribs.push(debug::ALC_CONTEXT_FLAGS);
                attribs.push(debug::ALC_CONTEXT_DEBUG_BIT);
            } else {
                warn("ALC_EXT_debug not supported on device");
            }
            debug
        } else {
            false
        };

        let has_efx = match unsafe { naevc::conf.al_efx } {
            0 => false,
            _ => match device.is_extension_present(ALC_EXT_EFX_NAME) {
                true => {
                    attribs.push(ALC_MAX_AUXILIARY_SENDS);
                    attribs.push(4);
                    true
                }
                false => false,
            },
        };
        let output_limiter = device.is_extension_present(ALC_OUTPUT_LIMITER_SOFT_NAME);
        if output_limiter {
            attribs.push(ALC_OUTPUT_LIMITER_SOFT);
            attribs.push(ALC_TRUE as i32);
        }
        attribs.push(0); // Has to be NULL terminated

        let context = al::Context::new(&device, &attribs)?;
        context.set_current()?;

        // Check to see if output limiter is working
        if output_limiter && device.get_parameter_i32(ALC_OUTPUT_LIMITER_SOFT) != ALC_TRUE as i32 {
            warn!("failed to set ALC_OUTPUT_LIMITER_SOFT");
        }
        // Check to see if debugging was enabled
        if has_debug {
            match debug::Debug::init(&device) {
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

        unsafe {
            alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        }

        debugx!(gettext("OpenAL started: {} Hz"), freq);
        let al_renderer = al::get_parameter_str(AL_RENDERER)?;
        debugx!(gettext("Renderer: {}"), &al_renderer);
        let al_vendor = al::get_parameter_str(AL_VENDOR)?;
        let al_version = al::get_parameter_str(AL_VERSION)?;
        debugx!(gettext("Version: {}"), &al_version);
        if has_debug {
            debugx!(gettext("   with DEBUG"));
        }
        if let Some(efx) = EFX.get() {
            debugx!(gettext("   with EFX {}.{}"), efx.version.0, efx.version.1);
        }
        debug!("");

        Ok(AudioSystem {
            device,
            context,

            volume: RwLock::new(AudioVolume::new()),

            freq,
            output_limiter,
        })
    }
}
static AUDIO: LazyLock<AudioSystem> = LazyLock::new(|| AudioSystem::new().unwrap());

pub fn init() -> Result<()> {
    let _ = &*AUDIO;
    Ok(())
}

/*
impl FromLua for Audio {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::UserData(ud) => Ok(*ud.borrow::<Self>()?),
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to Audio",
                val.type_name()
            ))),
        }
    }
}
*/

/*
 * @brief Lua bindings to interact with audio.
 *
 * @luamod audio
 */
impl UserData for Audio {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        fields.add_field_method_get("name", |_, this| Ok(this.buffer.name.clone()));
    }
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
         * @brief Gets a string representation of an audio file.
         *
         *    @luatparam Audio audio Audio to get string representation of.
         *    @luatreturn string String representation of the audio.
         * @luafunc __tostring
         */
        methods.add_meta_method(MetaMethod::ToString, |_, audio: &Self, ()| {
            Ok(format!("audio( {} )", &audio.buffer.name))
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
        methods.add_method("clone", |_, audio: &Self, ()| -> mlua::Result<()> {
            todo!()
        });
        /*
         * @brief Plays a source.
         *
         *    @luatparam Audio source Source to play.
         * @luafunc play
         */
        methods.add_method("play", |_, audio: &Self, ()| -> mlua::Result<()> {
            audio.play();
            Ok(())
        });
        /*
         * @brief Checks to see if a source is playing.
         *
         *    @luatparam Audio source Source to check to see if is playing.
         *    @luatreturn boolean Whether or not the source is playing.
         * @luafunc isPlaying
         */
        methods.add_method("isPlaying", |_, audio: &Self, ()| -> mlua::Result<bool> {
            Ok(audio.is_playing())
        });
        /*
         * @brief Pauses a source.
         *
         *    @luatparam Audio source Source to pause.
         * @luafunc pause
         */
        methods.add_method("pause", |_, audio: &Self, ()| -> mlua::Result<()> {
            audio.pause();
            Ok(())
        });
        /*
         * @brief Checks to see if a source is paused.
         *
         *    @luatparam Audio source Source to check to see if is paused.
         *    @luatreturn boolean Whether or not the source is paused.
         * @luafunc isPaused
         */
        methods.add_method("isPaused", |_, audio: &Self, ()| -> mlua::Result<bool> {
            Ok(audio.is_paused())
        });
        /*
         * @brief Stops a source.
         *
         *    @luatparam Audio source Source to stop.
         * @luafunc stop
         */
        methods.add_method("stop", |_, audio: &Self, ()| -> mlua::Result<()> {
            audio.stop();
            Ok(())
        });
        /*
         * @brief Checks to see if a source is stopped.
         *
         *    @luatparam Audio source Source to check to see if is stopped.
         *    @luatreturn boolean Whether or not the source is stopped.
         * @luafunc isStopped
         */
        methods.add_method("isStopped", |_, audio: &Self, ()| -> mlua::Result<bool> {
            Ok(audio.is_stopped())
        });
        /*
         * @brief Rewinds a source.
         *
         *    @luatparam Audio source Source to rewind.
         * @luafunc rewind
         */
        methods.add_method("rewind", |_, audio: &Self, ()| -> mlua::Result<()> {
            audio.rewind();
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
            |_, audio: &Self, (offset, samples): (f32, bool)| -> mlua::Result<()> {
                audio.seek(
                    offset,
                    match samples {
                        true => AudioSeek::Samples,
                        false => AudioSeek::Seconds,
                    },
                );
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
        methods.add_method(
            "tell",
            |_, audio: &Self, samples: bool| -> mlua::Result<f32> {
                Ok(audio.tell(match samples {
                    true => AudioSeek::Samples,
                    false => AudioSeek::Seconds,
                }))
            },
        );
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
            |_, audio: &Self, samples: bool| -> mlua::Result<f32> {
                Ok(audio.buffer.duration(match samples {
                    true => AudioSeek::Samples,
                    false => AudioSeek::Seconds,
                }))
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
        methods.add_method_mut(
            "setVolume",
            |_, audio: &mut Self, (volume, ignoremaster): (f32, bool)| -> mlua::Result<()> {
                if ignoremaster {
                    audio.set_volume_raw(volume)
                } else {
                    audio.set_volume(volume)
                }
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
        methods.add_method("getVolume", |_, audio: &Self, ()| -> mlua::Result<f32> {
            Ok(audio.volume())
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
            |_, audio: &Self, relative: bool| -> mlua::Result<()> {
                audio.set_relative(relative);
                Ok(())
            },
        );
        /*
         * @brief Gets whether a source is relative or not.
         *
         *    @luatreturn boolean relative Whether or not to the source is relative.
         * @luafunc isRelative
         */
        methods.add_method("isRelative", |_, audio: &Self, ()| -> mlua::Result<bool> {
            Ok(audio.relative())
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
            |_, audio: &Self, (x, y, z): (f32, f32, f32)| -> mlua::Result<()> {
                let vec: Vector3<f32> = Vector3::new(x, y, z);
                audio.set_position(vec);
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
            |_, audio: &Self, ()| -> mlua::Result<(f32, f32, f32)> {
                let pos = audio.position();
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
            |_, audio: &Self, (x, y, z): (f32, f32, f32)| -> mlua::Result<()> {
                let vec: Vector3<f32> = Vector3::new(x, y, z);
                audio.set_velocity(vec);
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
            |_, audio: &Self, ()| -> mlua::Result<(f32, f32, f32)> {
                let vel = audio.velocity();
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
        methods.add_method(
            "setLooping",
            |_, audio: &Self, looping: bool| -> mlua::Result<()> {
                audio.set_looping(looping);
                Ok(())
            },
        );
        /*
         * @brief Gets the looping state of a source.
         *
         *    @luatparam Audio source Source to get looping state of.
         *    @luatreturn boolean Whether or not the source is looping.
         * @luafunc isLooping
         */
        methods.add_method("isLooping", |_, audio: &Self, ()| -> mlua::Result<bool> {
            Ok(audio.looping())
        });
        /*
         * @brief Sets the pitch of a source.
         *
         *    @luatparam Audio source Source to set pitch of.
         *    @luatparam number pitch Pitch to set the source to.
         * @luafunc setPitch
         */
        methods.add_method(
            "setPitch",
            |_, audio: &Self, pitch: f32| -> mlua::Result<()> {
                audio.set_pitch(pitch);
                Ok(())
            },
        );
        /*
         * @brief Gets the pitch of a source.
         *
         *    @luatparam Audio source Source to get pitch of.
         *    @luatreturn number Pitch of the source.
         * @luafunc getPitch
         */
        methods.add_method("getPitch", |_, audio: &Self, ()| -> mlua::Result<f32> {
            Ok(audio.pitch())
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
            |_, audio: &Self, (reference, max): (f32, f32)| -> mlua::Result<()> {
                audio.set_attenuation_distances(reference, max);
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
            |_, audio: &Self, ()| -> mlua::Result<(f32, f32)> { Ok(audio.attenuation_distances()) },
        );
        /*
         * @brief Sets the rolloff factor.
         *
         *    @luatparam number rolloff New rolloff factor.
         * @luafunc setRolloff
         */
        methods.add_method(
            "setRolloff",
            |_, audio: &Self, rolloff: f32| -> mlua::Result<()> {
                audio.set_rolloff(rolloff);
                Ok(())
            },
        );
        /*
         * @brief Gets the rolloff factor.
         *
         *    @luatreturn number Rolloff factor or 0. if sound is disabled.
         * @luafunc getRolloff
         */
        methods.add_method("getRolloff", |_, audio: &Self, ()| -> mlua::Result<f32> {
            Ok(audio.rolloff())
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
            |_, audio: &Self, (name, enable): (String, bool)| -> mlua::Result<bool> {
                let slot = if enable {
                    let lock = EFX_LIST.lock().unwrap();
                    let efxid =
                        match binary_search_by_key_ref(&lock, &name, |e: &LuaAudioEfx| &e.name) {
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
                audio
                    .source
                    .parameter_3_i32(AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
                Ok(true)
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
                        efx.effect.parameter_f32($field, param.get::<f32>($name)?);
                    }};
                }
                macro_rules! efx_set_i32 {
                    ($name: literal, $field: ident) => {{
                        efx.effect.parameter_i32($field, param.get::<i32>($name)?);
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
    Ok(lua.create_proxy::<Audio>()?)
}
