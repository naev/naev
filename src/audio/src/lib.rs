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

struct LuaAudioEfx {
    name: String,
    effect: ALuint,
    slot: ALuint,
}

#[derive(Clone, PartialEq)]
pub enum AudioType {
    Static,
    Stream,
}

/*
struct BufferDataType<T> {
    data: Vec<T>,
}

struct BufferData {
    MonoU8(BufferDataType<u8>),
    StereoU8((BufferDataType<u8>, BufferDataType<u8>)),
    MonoS16(BufferDataType<i16>),
    StereoS16((BufferDataType<i16>,BufferDataType<i16>)),
}
*/

#[derive(Clone)]
pub struct AudioBuffer {
    buffer: Arc<ALuint>,
}

#[derive(Clone)]
pub struct Audio {
    name: String,
    ok: bool,
    atype: AudioType,
    nocleanup: bool,
    source: ALuint,
    slot: ALuint,
    volume: f64,
    buffer: Option<AudioBuffer>,
    track_gain_db: f64,
    track_peak: f64,
}
impl Audio {
    pub fn new(path: &str) -> Result<Self> {
        let name = String::from(path);

        let atype = AudioType::Static;

        use symphonia::core::codecs::{DecoderOptions, CODEC_TYPE_NULL};
        use symphonia::core::errors::Error;
        use symphonia::core::formats::{FormatOptions, FormatReader};
        use symphonia::core::io::MediaSourceStream;
        use symphonia::core::meta::{MetadataOptions, StandardTagKey, Tag, Value};
        use symphonia::core::probe::Hint;

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
        let track_id = track.id;

        // Create a decoder for the track.
        let dec_opts: DecoderOptions = Default::default();
        let mut decoder = symphonia::default::get_codecs().make(&track.codec_params, &dec_opts)?;

        // The decode loop.
        let mut data: Vec<symphonia::core::audio::AudioBuffer<u8>> = vec![];
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
                    data.push(decoded.make_equivalent());
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

        let source = al::create_source()?;
        let buffer = al::create_buffer()?;
        /*
        unsafe {
            alBufferData(
                buffer,
                AL_FORMAT_STEREO16,
                data,
                size, // in bytes
                freq, // frequency
            )
        }
        */

        Ok(Self {
            name,
            ok: true,
            atype,
            nocleanup: false,
            source,
            slot: 0,
            volume: 1.0,
            buffer: None,
            track_gain_db,
            track_peak,
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
