#![allow(dead_code, unused_imports, unused_variables)]
mod efx;
mod openal;
use crate::efx::*;
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;

use anyhow::Result;
use gettext::gettext;
use log::{debug, debugx, warn, warn_err};
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use std::ffi::{CStr, CString};
use std::sync::atomic::{AtomicBool, AtomicPtr, Ordering};
use std::sync::{Arc, Mutex};

const NUM_VOICES: usize = 64;
const REFERENCE_DISTANCE: f32 = 500.;
const MAX_DISTANCE: f32 = 25_000.;
static AUDIO_ENABLED: AtomicBool = AtomicBool::new(false);

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
    match al::is_error() {
        Some(e) => {
            warn_err!(e);
        }
        None => (),
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
            .map(|s| s.to_str())
            .flatten()
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
            let left = buffers.iter().map(|b| b.chan(0).to_vec()).flatten();
            let data: Vec<_> = match stereo {
                true => {
                    let right = buffers.iter().map(|b| b.chan(1).to_vec()).flatten();
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
}

pub struct Audio {
    name: String,
    ok: bool,
    atype: AudioType,
    nocleanup: bool,
    source: al::Source,
    slot: ALuint,
    volume: f64,
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
            nocleanup: false,
            source,
            slot: 0,
            volume: 1.0,
            buffer,
        })
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

        let voices: Vec<_> = [0..NUM_VOICES]
            .iter()
            .map(|_| al::Source::new())
            .flatten()
            .collect();
        for v in &voices {
            v.parameter_f32(AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
            v.parameter_f32(AL_MAX_DISTANCE, MAX_DISTANCE);
            v.parameter_f32(AL_ROLLOFF_FACTOR, 1.);

            if efx.is_some() {
                //device.parameter_3_i32( AL_AUXILIARY_SEND_FILTER, direct, 0, AL_FILTER_NULL );
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

#[allow(unused_doc_comments)]
impl UserData for Audio {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        fields.add_field_method_get("name", |_, this| Ok(this.name.clone()));
    }
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        //methods.add_method("foo", |_, ()| -> mlua::Result<Self> {
        //}
    }
}
