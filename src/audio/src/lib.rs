#![allow(dead_code, unused_imports, unused_variables)]
mod openal;
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
    volume: f32,
    buffer: Option<AudioBuffer>,
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

        {
            let src = ndata::open(path)?;
            let mss = MediaSourceStream::new(Box::new(src), Default::default());

            let mut hint = Hint::new();
            hint.with_extension("mp3");

            // Use the default options for metadata and format readers.
            let meta_opts: MetadataOptions = Default::default();
            let fmt_opts: FormatOptions = Default::default();

            // Probe the media source.
            let probed =
                symphonia::default::get_probe().format(&hint, mss, &fmt_opts, &meta_opts)?;

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

            // Use the default options for the decoder.
            let dec_opts: DecoderOptions = Default::default();

            // Create a decoder for the track.
            let mut decoder =
                symphonia::default::get_codecs().make(&track.codec_params, &dec_opts)?;

            // The decode loop.
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

                // Consume any new metadata that has been read since the last packet.
                while !format.metadata().is_latest() {
                    // Pop the old head of the metadata queue.
                    format.metadata().pop();

                    // Consume the new metadata at the head of the metadata queue.
                }

                // If the packet does not belong to the selected track, skip over it.
                if packet.track_id() != track_id {
                    continue;
                }

                // Decode the packet into audio samples.
                match decoder.decode(&packet) {
                    Ok(_decoded) => {
                        // Consume the decoded audio samples (see below).
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
        }

        Ok(Self {
            name,
            ok: true,
            atype,
            nocleanup: false,
            source: 0,
            slot: 0,
            volume: 1.0,
            buffer: None,
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
