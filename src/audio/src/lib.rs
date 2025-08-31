#![allow(dead_code, unused_imports, unused_variables)]
mod efx;
mod openal;
use crate::efx::*;
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;
use naev_core::utils::AtomicF32;

use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use nalgebra::Vector3;
use std::ffi::{CStr, CString};
use std::sync::atomic::{AtomicBool, AtomicPtr, Ordering};
use std::sync::{Arc, Mutex};

const NUM_VOICES: usize = 64;
const REFERENCE_DISTANCE: f32 = 500.;
const MAX_DISTANCE: f32 = 25_000.;
static AUDIO_ENABLED: AtomicBool = AtomicBool::new(false);
/// Master volume in log
static VOLUME_MASTER: AtomicF32 = AtomicF32::new(1.0);

#[derive(Clone)]
pub enum Message {
    DeleteAuxiliaryEffectSlot(std::num::NonZero<ALuint>),
    DeleteFilter(std::num::NonZero<ALuint>),
    DeleteEffect(std::num::NonZero<ALuint>),
}
impl Message {
    fn execute(self, sys: &AudioSystem) {
        match self {
            Self::DeleteAuxiliaryEffectSlot(id) => unsafe {
                (sys.efx.as_ref().unwrap().alDeleteAuxiliaryEffectSlots)(
                    1,
                    &id.get() as *const ALuint,
                );
            },
            Self::DeleteFilter(id) => unsafe {
                (sys.efx.as_ref().unwrap().alDeleteFilters)(1, &id.get() as *const ALuint);
            },
            Self::DeleteEffect(id) => unsafe {
                (sys.efx.as_ref().unwrap().alDeleteEffects)(1, &id.get() as *const ALuint);
            },
        }
    }
}
static MESSAGE_QUEUE: Mutex<Vec<Message>> = Mutex::new(vec![]);
pub(crate) fn message_push(msg: Message) {
    MESSAGE_QUEUE.lock().unwrap().push(msg);
}

#[inline]
pub(crate) fn check_error() {
    if let Some(e) = al::is_error() {
        warn_err!(e);
    }
}

struct LuaAudioEfx {
    name: String,
    effect: ALuint,
    slot: ALuint,
}

#[derive(Clone, PartialEq)]
enum AudioType {
    Static,
    Stream,
}

struct AudioBuffer {
    track_gain_db: f64,
    track_peak: f64,
    buffer: al::Buffer,
}
impl AudioBuffer {
    fn from_path(path: &str) -> Result<Self> {
        use symphonia::core::audio::{AudioBuffer, Channels, SampleBuffer, Signal};
        use symphonia::core::codecs::{CODEC_TYPE_NULL, CodecParameters, Decoder, DecoderOptions};
        use symphonia::core::errors::Error;
        use symphonia::core::formats::{FormatOptions, FormatReader, Track};
        use symphonia::core::io::MediaSourceStream;
        use symphonia::core::meta::{MetadataOptions, StandardTagKey, Tag, Value};
        use symphonia::core::probe::{Hint, ProbeResult};
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
                    fn tag_to_f64(t: &Tag) -> Result<f64> {
                        match t.value {
                            Value::Float(val) => Ok(val),
                            _ => anyhow::bail!("tag is not a float"),
                        }
                    }
                    if let Some(key) = t.std_key {
                        match key {
                            StandardTagKey::ReplayGainTrackGain => match tag_to_f64(t) {
                                Ok(f) => {
                                    track_gain_db = f;
                                }
                                Err(e) => {
                                    warn_err!(e);
                                }
                            },
                            StandardTagKey::ReplayGainTrackPeak => match tag_to_f64(t) {
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
            (10.0_f64.powf(track_gain_db / 20.0), 1.0 / track_peak)
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
                check_error();
                buffer
            }
            None => anyhow::bail!("no format!"),
        };

        Ok(Self {
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

pub enum AudioSeek {
    Seconds,
    Samples,
}

pub struct Audio {
    name: String,
    ok: bool,
    atype: AudioType,
    source: al::Source,
    slot: ALuint,
    volume: f32,
    buffer: Arc<AudioBuffer>,
}
impl Audio {
    pub fn new(path: &str) -> Result<Self> {
        let name = String::from(path);

        let atype = AudioType::Static;

        let source = al::Source::new()?;
        let buffer = Arc::new(AudioBuffer::from_path(path)?);

        Ok(Self {
            name,
            ok: true,
            atype,
            source,
            slot: 0,
            volume: 1.0,
            buffer,
        })
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
        let master = VOLUME_MASTER.load(Ordering::Relaxed);
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
}

pub struct AudioSystem {
    device: al::Device,
    context: al::Context,

    volume: f32,
    volume_lin: f32,
    volume_speed: f32,

    freq: i32,
    output_limiter: bool,
    efx: Option<Efx>,

    voices: Vec<al::Source>,
}
impl AudioSystem {
    pub fn new() -> Result<Self> {
        let device = al::Device::new(None)?;

        let mut attribs: Vec<ALint> = vec![ALC_MONO_SOURCES, 512, ALC_STEREO_SOURCES, 32];
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
        if output_limiter && device.get_parameter_i32(ALC_OUTPUT_LIMITER_SOFT) != ALC_TRUE as i32 {
            warn!("failed to set ALC_OUTPUT_LIMITER_SOFT");
        }

        // Get context information
        let freq = device.get_parameter_i32(ALC_FREQUENCY);
        let nmono = device.get_parameter_i32(ALC_MONO_SOURCES);
        let nstereo = device.get_parameter_i32(ALC_STEREO_SOURCES);

        #[allow(non_snake_case)]
        let efx = if has_efx {
            Some(Efx::new(&device)?)
        } else {
            None
        };

        let voices: Vec<_> = (0..NUM_VOICES)
            .collect::<std::vec::Vec<usize>>()
            .iter()
            .flat_map(|_| al::Source::new())
            .collect();
        for v in &voices {
            v.parameter_f32(AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
            v.parameter_f32(AL_MAX_DISTANCE, MAX_DISTANCE);
            v.parameter_f32(AL_ROLLOFF_FACTOR, 1.);

            if let Some(efx) = &efx {
                v.parameter_3_i32(
                    AL_AUXILIARY_SEND_FILTER,
                    efx.direct_slot.0.get() as i32,
                    0,
                    AL_FILTER_NULL,
                );
            }
        }

        unsafe {
            alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        }

        debugx!(gettext("OpenAL started: {} Hz"), freq);
        debugx!(gettext("Renderer: %s"), al::get_parameter_str(AL_RENDERER));
        if let Some(efx) = &efx {
            debugx!(
                gettext("Version: {} with EFX {}.{}"),
                al::get_parameter_str(AL_VERSION),
                efx.version.0,
                efx.version.1
            );
        } else {
            debugx!(
                gettext("Version: {} without EFX"),
                al::get_parameter_str(AL_VERSION)
            );
        }
        debug!("");

        Ok(Self {
            device,
            context,

            volume: 1.,
            volume_lin: 1.,
            volume_speed: 1.,

            freq,
            output_limiter,
            efx,

            voices,
        })
    }
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
        fields.add_field_method_get("name", |_, this| Ok(this.name.clone()));
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
            Ok(format!("audio( {} )", &audio.name))
        });
        /*
         * @brief Creates a new audio source.
         *
         *    @luatparam string|File data Data to load the audio from.
         *    @luatparam[opt="static"] string  Either "static" to load the entire source
         * at the start, or "stream" to load it in real time.
         *    @luatreturn Audio New audio corresponding to the data.
         * @luafunc new
         */
        methods.add_function(
            "new",
            |_, (val, _streaming): (String, bool)| -> mlua::Result<Self> {
                // TODO add streaming
                Ok(Self::new(&val)?)
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
            Ok(audio.play())
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
            Ok(audio.pause())
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
            Ok(audio.stop())
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
            Ok(audio.rewind())
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
                Ok(audio.seek(
                    offset,
                    match samples {
                        true => AudioSeek::Samples,
                        false => AudioSeek::Seconds,
                    },
                ))
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
                Ok(audio.set_relative(relative))
            },
        );
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
                Ok(audio.set_position(vec))
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
                Ok(audio.set_velocity(vec))
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
            |_, audio: &Self, looping: bool| -> mlua::Result<()> { Ok(audio.set_looping(looping)) },
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
            |_, audio: &Self, pitch: f32| -> mlua::Result<()> { Ok(audio.set_pitch(pitch)) },
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
        { "setPitch", audioL_setPitch },
        { "getPitch", audioL_getPitch },
        { "setAttenuationDistances", audioL_setAttenuationDistances },
        { "getAttenuationDistances", audioL_getAttenuationDistances },
        { "setRolloff", audioL_setRolloff },
        { "getRolloff", audioL_getRolloff },
        { "setEffect", audioL_setEffect },
        { "setGlobalEffect", audioL_setGlobalEffect },
        { "setGlobalAirAbsorption", audioL_setGlobalAirAbsorption },
        { "setGlobalDopplerFactor", audioL_setGlobaDopplerFactor },
             */
    }
}
