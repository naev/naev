#![allow(dead_code, unused_imports, unused_variables)]
mod openal;
use crate::openal as al;
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;

use anyhow::Result;
use log::warn_err;
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use std::sync::Arc;

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
        use symphonia::core::codecs::{CodecParameters, Decoder, DecoderOptions, CODEC_TYPE_NULL};
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
