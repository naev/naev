use crate::physfs::Mode;
use crate::{FileType, physfs, stat};
use anyhow::Result;
use mlua::{UserData, UserDataMethods};
use sdl::iostream::IOStream;
use sdl3 as sdl;
use std::io::{Read, Seek, Write};
use std::path::Path;

pub struct OpenFile {
    mode: Mode,
    io: IOStream<'static>,
}

pub struct LuaFile {
    pub path: String,
    file: Option<OpenFile>,
}
unsafe impl Send for LuaFile {}

impl LuaFile {
    fn open(&mut self, mode: Mode) -> Result<()> {
        let io = physfs::iostream(&self.path, mode)?;
        self.file = Some(OpenFile { mode, io });
        Ok(())
    }

    pub fn iostream(&self) -> Option<&IOStream<'static>> {
        if let Some(file) = &self.file {
            Some(&file.io)
        } else {
            None
        }
    }
    pub fn iostream_mut(&mut self) -> Option<&mut IOStream<'static>> {
        if let &mut Some(ref mut file) = &mut self.file {
            Some(&mut file.io)
        } else {
            None
        }
    }

    pub fn into_iostream(mut self) -> Result<IOStream<'static>> {
        let file = match self.file.take() {
            Some(file) => file,
            None => {
                anyhow::bail!(format!("LuaFile '{}' not open", self.path));
            }
        };
        Ok(file.io)
    }

    pub fn try_clone(&self) -> Result<Self> {
        let file = if let Some(f) = &self.file {
            Some(OpenFile {
                io: physfs::iostream(&self.path, f.mode)?,
                mode: f.mode,
            })
        } else {
            None
        };
        Ok(Self {
            path: self.path.clone(),
            file,
        })
    }
}

macro_rules! file_not_open {
    () => {
        Err(mlua::Error::RuntimeError("File not open".to_string()))
    };
}

/*@
 * @brief Lua bindings to interact with files.
 *
 * @note The API here is designed to be compatible with that of "LÖVE".
 *
 * @luamod file
 */
impl UserData for LuaFile {
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        //methods.add_meta_method( MetaMethod::Eq, |_, this,

        /*@
         * @brief Create a file from a string buffer.
         *
         *    @luatparam String str String to use as the memory data.
         *    @luatparam[opt="memory buffer"] String name Optional name to give the
         * buffer.
         *    @luatreturn File The new file wrapping the string.
         * @luafunc from_string
         */
        methods.add_function(
            "from_string",
            |_, (data, name): (String, Option<String>)| -> mlua::Result<LuaFile> {
                use std::sync::atomic::{AtomicU32, Ordering};
                static COUNTER: AtomicU32 = AtomicU32::new(0);

                let buffername = name.unwrap_or_else(|| {
                    let c = COUNTER.fetch_add(1, Ordering::Relaxed);
                    format!("memory buffer {}", c)
                });
                let data = data.into_bytes();
                let file = OpenFile {
                    mode: Mode::Read,
                    io: match IOStream::from_vec(data) {
                        Ok(io) => io,
                        Err(e) => return Err(mlua::Error::RuntimeError(e.to_string())),
                    },
                };
                Ok(LuaFile {
                    path: buffername,
                    file: Some(file),
                })
            },
        );
        /*@
         * @brief Opens a new file.
         *
         *    @luatparam string path Path to open.
         *    @luatreturn File New file object.
         * @luafunc new
         */
        methods.add_function("new", |_, path: String| -> mlua::Result<LuaFile> {
            Ok(LuaFile { path, file: None })
        });
        /*@
         * @brief Opens a new file.
         *
         *    @luatparam File File object to open.
         *    @luatparam[opt="r"] mode Mode to open the file in (should be 'r', 'w', or
         * 'a').
         *    @luatreturn boolean true on success, false and an error string on failure.
         * @luafunc open
         */
        methods.add_method_mut(
            "open",
            |_, this, mode: Mode| -> mlua::Result<(bool, Option<String>)> {
                match this.open(mode) {
                    Ok(io) => io,
                    Err(e) => {
                        return Ok((false, Some(e.to_string())));
                    }
                };
                Ok((true, None))
            },
        );
        /*@
         * @brief Closes a file.
         *
         *    @luatparam File file File to close.
         *    @luatreturn boolean true on success.
         * @luafunc close
         */
        methods.add_method_mut("close", |_, this, ()| -> mlua::Result<bool> {
            this.file = None;
            Ok(true)
        });
        /*@
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
                if let Some(file) = &mut this.file {
                    let (out, read) = if let Some(bytes) = bytes {
                        let mut out: Vec<u8> = vec![0; bytes];
                        let read = file.io.read(&mut out)?;
                        out.truncate(read);
                        (out, read)
                    } else {
                        let mut out: Vec<u8> = Vec::new();
                        let read = file.io.read_to_end(&mut out)?;
                        (out, read)
                    };
                    // Todo something better than this I guess
                    let out = unsafe { String::from_utf8_unchecked(out) };
                    Ok((out, read))
                } else {
                    file_not_open!()
                }
            },
        );
        /*@
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
                if let Some(file) = &mut this.file {
                    let mut bytes = data.into_bytes();
                    if let Some(len) = len {
                        bytes.truncate(len);
                    }
                    Ok(file.io.write(&bytes)?)
                } else {
                    file_not_open!()
                }
            },
        );
        /*@
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
                if let Some(file) = &mut this.file {
                    match file.io.seek(std::io::SeekFrom::Start(pos)) {
                        Ok(res) => Ok((pos == res, None)),
                        Err(e) => Ok((false, Some(e.to_string()))),
                    }
                } else {
                    file_not_open!()
                }
            },
        );
        /*@
         * @brief Gets the name of a file object.
         *
         *    @luatparam File file File object to get name of.
         *    @luatreturn string Name of the file object.
         * @luafunc getFilename
         */
        methods.add_method("getFilename", |_, this, ()| -> mlua::Result<String> {
            Ok(this.path.clone())
        });
        /*@
         * @brief Gets the mode a file is currently in.
         *
         *    @luatparam File file File to get mode of.
         *    @luatreturn string Mode of the file (either 'w', 'r', or 'a')
         * @luafunc getMode
         */
        methods.add_method("getMode", |_, this, ()| -> mlua::Result<String> {
            if let Some(file) = &this.file {
                Ok(match file.mode {
                    Mode::Append => 'a',
                    Mode::Read => 'r',
                    Mode::Write => 'w',
                }
                .to_string())
            } else {
                Ok('c'.to_string())
            }
        });
        /*@
         * @brief Gets the size of a file (must be open).
         *
         *    @luatparam File file File to get the size of.
         *    @luatreturn number Size of the file.
         * @luafunc getSize
         */
        methods.add_method("getSize", |_, this, ()| -> mlua::Result<Option<usize>> {
            if let Some(file) = &this.file {
                Ok(file.io.len())
            } else {
                file_not_open!()
            }
        });
        /*@
         * @brief Checks to see if a file is open.
         *
         *    @luatparam File file File to check to see if is open.
         *    @luatreturn boolean true if the file is open, false otherwise.
         * @luafunc isOpen
         */
        methods.add_method("isOpen", |_, this, ()| -> mlua::Result<bool> {
            Ok(this.file.is_some())
        });
        /*@
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
        /*@
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
        /*@
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
                    t.raw_push(Path::new(&f).file_name())?;
                }
                Ok(t)
            },
        );
        /*@
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

pub fn open_file(lua: &mlua::Lua) -> anyhow::Result<mlua::AnyUserData> {
    Ok(lua.create_proxy::<LuaFile>()?)
}

/*
#[test]
fn test_mlua_file() {
    let lua = mlua::Lua::new();
    let globals = lua.globals();
    globals.set("file", open_file(&lua).unwrap()).unwrap();
    lua.load(
        r#"
        local s = "Hello World!"
        local f = file.from_string(s)
        local d = f:read()
        assert( s==d )
        local _,n = f:read()
        assert( n==0 )
        local r = f:seek(0)
        assert( r )
        assert( s:len()==f:getSize() )

        -- Should exist
        local f = file.new('AUTHORS')
        local r = f:open()
        assert( r )
        local _,n = f:read()
        assert( n>0 )

        -- Shouldn't exist
        local f = file.new('DOESNT_EXIST_WILL_ERROR')
        assert( not f:open() )
        "#,
    )
    .set_name("mlua file test")
    .exec()
    .unwrap();
}
*/
