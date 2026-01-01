use crate::{AudioData, AudioSeek, Buffer};
use mlua::{MetaMethod, UserData, UserDataMethods};

/*@
 * @brief Lua bindings to interact with audio.
 *
 * @luamod audiodata
 */
impl UserData for AudioData {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*@
         * @brief Gets a string representation of an `AudioData`.
         *
         *    @luatparam AudioData data `AudioData` to get string representation of.
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
         *    @luatreturn number Number of samples of the data.
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

pub fn open_audiodata(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<AudioData>()?)
}
