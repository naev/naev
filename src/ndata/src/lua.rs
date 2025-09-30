use crate::physfs::Mode;
use crate::{FileType, physfs, stat};
use mlua::{UserData, UserDataMethods};
use sdl::iostream::IOStream;
use sdl3 as sdl;
use std::io::{Read, Seek, Write};

pub struct LuaFile {
    path: String,
    mode: Mode,
    io: IOStream<'static>,
}
unsafe impl Send for LuaFile {}

/*
 * @brief Lua bindings to interact with files.
 *
 * @note The API here is designed to be compatible with that of "LÖVE".
 *
 * @luamod file
 */
#[allow(unused_doc_comments)]
impl UserData for LuaFile {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        //methods.add_meta_method( MetaMethod::Eq, |_, this,

        /*
         * @brief Opens a new file.
         *
         *    @luatparam string path Path to open.
         *    @luatparam[opt="r"] mode Mode to open the file in (should be 'r', 'w', or
         *    @luatreturn File New file object.
         * @luafunc open
         */
        methods.add_function(
            "open",
            |_, (path, mode): (String, Mode)| -> mlua::Result<(Option<LuaFile>, Option<String>)> {
                let io = match physfs::iostream(&path, mode) {
                    Ok(io) => io,
                    Err(e) => {
                        return Ok((None, Some(e.to_string())));
                    }
                };
                Ok((Some(LuaFile { path, mode, io }), None))
            },
        );

        /*
         * @brief Reads from an open file.
         *
         *    @luatparam File file File to read from.
         *    @luatparam[opt] number bytes Number of bytes to read or all if omitted.
         *    @luatreturn string Read data.
         *    @luatreturn number Number of bytes actually read.
         * @luafunc read
         */
        methods.add_method_mut(
            "read",
            |_, this, bytes: Option<usize>| -> mlua::Result<(String, usize)> {
                let (out, read) = if let Some(bytes) = bytes {
                    let mut out: Vec<u8> = vec![0; bytes];
                    let read = this.io.read(&mut out)?;
                    out.truncate(read);
                    (out, read)
                } else {
                    let mut out: Vec<u8> = Vec::new();
                    let read = this.io.read_to_end(&mut out)?;
                    (out, read)
                };
                // Todo something better than this I guess
                let out = unsafe { String::from_utf8_unchecked(out) };
                Ok((out, read))
            },
        );

        /*
         * @brief Reads from an open file.
         *
         *    @luatparam File file File to write to.
         *    @luatparam string data Data to write.
         *    @luatparam[opt] number bytes Number of bytes to write.
         * @luafunc write
         */
        methods.add_method_mut(
            "write",
            |_, this, (data, len): (String, Option<usize>)| -> mlua::Result<usize> {
                let mut bytes = data.into_bytes();
                if let Some(len) = len {
                    bytes.truncate(len);
                }
                Ok(this.io.write(&bytes)?)
            },
        );
        /*
         * @brief Seeks in an open file.
         *
         *    @luatparam File file File to seek in.
         *    @luatparam number pos Position to seek to (from start of file).
         *    @luatreturn boolean true on success.
         * @luafunc seek
         */
        methods.add_method_mut(
            "seek",
            |_, this, pos: u64| -> mlua::Result<(bool, Option<String>)> {
                match this.io.seek(std::io::SeekFrom::Start(pos)) {
                    Ok(res) => Ok((pos == res, None)),
                    Err(e) => Ok((false, Some(e.to_string()))),
                }
            },
        );
        /*
         * @brief Gets the name of a file object.
         *
         *    @luatparam File file File object to get name of.
         *    @luatreturn string Name of the file object.
         * @luafunc name
         */
        methods.add_method("name", |_, this, ()| -> mlua::Result<String> {
            Ok(this.path.clone())
        });
        /*
         * @brief Gets the mode a file is currently in.
         *
         *    @luatparam File file File to get mode of.
         *    @luatreturn string Mode of the file (either 'w', 'r', or 'a')
         * @luafunc mode
         */
        methods.add_method("mode", |_, this, ()| -> mlua::Result<String> {
            Ok(match this.mode {
                Mode::Append => 'a',
                Mode::Read => 'r',
                Mode::Write => 'w',
            }
            .to_string())
        });
        /*
         * @brief Gets the size of a file (must be open).
         *
         *    @luatparam File file File to get the size of.
         *    @luatreturn number Size of the file.
         * @luafunc size
         */
        methods.add_method("size", |_, this, ()| -> mlua::Result<Option<usize>> {
            Ok(this.io.len())
        });
        /*
         * @brief Checks to see the filetype of a path.
         *
         *    @luatparam string path Path to check to see what type it is.
         *    @luatreturn string What type of file it is or nil if doesn't exist.
         * @luafunc filetype
         */
        methods.add_function(
            "filetype",
            |_, path: String| -> mlua::Result<Option<String>> {
                match stat(&path) {
                    Ok(s) => Ok(Some(
                        match s.filetype {
                            FileType::Regular => "file",
                            FileType::Directory => "directory",
                            FileType::Symlink => "symlink",
                            FileType::Other => "other",
                        }
                        .to_string(),
                    )),
                    Err(_) => Ok(None),
                }
            },
        );
        /*
         * @brief Makes a directory.
         *
         *    @luatparam string dir Name of the directory to make.
         *    @luatreturn boolean True on success.
         * @luafunc mkdir
         */
        methods.add_function(
            "mkdir",
            |_, path: String| -> mlua::Result<(bool, Option<String>)> {
                match physfs::mkdir(&path) {
                    Ok(()) => Ok((true, None)),
                    Err(e) => Ok((false, Some(e.to_string()))),
                }
            },
        );
        /*
         * @brief Returns a list of files and subdirectories of a directory.
         *
         *    @luatparam string dir Name of the directory to check.
         *    @luatreturn table Table containing all the names (strings) of the
         * subdirectories and files in the directory.
         * @luafunc enumerate
         */
        methods.add_function(
            "enumerate",
            |lua, path: String| -> mlua::Result<mlua::Table> {
                let t = lua.create_table()?;
                for f in crate::read_dir(&path)? {
                    t.raw_push(f)?;
                }
                Ok(t)
            },
        );
        /*
         * @brief Removes a file or directory.
         *
         *    @luatparam string path Name of the path to remove.
         *    @luatreturn boolean True on success.
         * @luafunc remove
         */
        methods.add_function(
            "remove",
            |_, path: String| -> mlua::Result<(bool, Option<String>)> {
                match physfs::remove_file(&path) {
                    Ok(()) => Ok((true, None)),
                    Err(e) => Ok((false, Some(e.to_string()))),
                }
            },
        );
    }
}
