/* Documentation mentions global lock in settings. Should be thread-safe _except_ for opening the
 * same file and writing + reading/writing with multiple threads. */
use mlua::{FromLua, Lua, MetaMethod, UserData, UserDataMethods, Value};
use sdl::iostream::IOStream;
use sdl3 as sdl;
use std::ffi::{CStr, CString, c_int};
use std::io::{Error, Read, Result, Seek, SeekFrom, Write};
use std::os::raw::c_void;
use std::sync::atomic::{AtomicPtr, Ordering};

// Some stuff is based on the physfs-rs package.
// Modified to not use a global context and use functions from naevc
pub fn error_as_io_error(func: &str) -> Error {
    let cerrstr = unsafe {
        CStr::from_ptr(naevc::PHYSFS_getErrorByCode(
            naevc::PHYSFS_getLastErrorCode(),
        ))
    };
    Error::other(format!(
        "PhysicsFS Error with '{}': `{}`",
        func,
        cerrstr.to_str().unwrap_or("Unknown")
    ))
}

pub fn error_as_io_error_with_file(func: &str, file: &str) -> Error {
    let cerrstr = unsafe {
        CStr::from_ptr(naevc::PHYSFS_getErrorByCode(
            naevc::PHYSFS_getLastErrorCode(),
        ))
    };
    Error::other(format!(
        "PhysicsFS Error with '{}' on file '{}': `{}`",
        func,
        file,
        cerrstr.to_str().unwrap_or("Unknown")
    ))
}

pub fn set_write_dir(path: &str) -> Result<()> {
    match unsafe { naevc::PHYSFS_setWriteDir(CString::new(path)?.as_ptr()) } {
        0 => Err(error_as_io_error("PHYSFS_setWriteDir")),
        _ => Ok(()),
    }
}

pub fn get_base_dir() -> String {
    let val = unsafe { CStr::from_ptr(naevc::PHYSFS_getBaseDir()) };
    String::from(val.to_string_lossy())
}

pub fn get_write_dir() -> String {
    let val = unsafe { CStr::from_ptr(naevc::PHYSFS_getWriteDir()) };
    String::from(val.to_string_lossy())
}

pub fn get_pref_dir(org: &str, app: &str) -> Result<String> {
    let corg = CString::new(org)?;
    let capp = CString::new(app)?;
    let val = unsafe { naevc::PHYSFS_getPrefDir(corg.as_ptr(), capp.as_ptr()) };
    if val.is_null() {
        Err(error_as_io_error("PHYSFS_getPrefDir"))
    } else {
        unsafe { Ok(String::from(CStr::from_ptr(val).to_string_lossy())) }
    }
}

pub fn mount(new_dir: &str, append: bool) -> Result<()> {
    let cnew_dir = CString::new(new_dir)?;
    match unsafe { naevc::PHYSFS_mount(cnew_dir.as_ptr(), std::ptr::null(), append as c_int) } {
        0 => Err(error_as_io_error_with_file("PHYSFS_mount", new_dir)),
        _ => Ok(()),
    }
}

pub fn mount_at(new_dir: &str, mount_point: &str, append: bool) -> Result<()> {
    let cnew_dir = CString::new(new_dir)?;
    let cmount_point = CString::new(mount_point)?;
    match unsafe { naevc::PHYSFS_mount(cnew_dir.as_ptr(), cmount_point.as_ptr(), append as c_int) }
    {
        0 => Err(error_as_io_error_with_file("PHYSFS_mount", new_dir)),
        _ => Ok(()),
    }
}

/// Possible ways to open a file.
#[allow(dead_code)]
#[derive(Copy, Clone)]
pub enum Mode {
    /// Append to the end of the file.
    Append,
    /// Read from the file.
    Read,
    /// Write to the file, overwriting previous data.
    Write,
}

impl FromLua for Mode {
    fn from_lua(value: Value, _: &Lua) -> mlua::Result<Self> {
        match value {
            Value::String(s) => match s.to_string_lossy().as_str() {
                "a" => Ok(Self::Append),
                "r" => Ok(Self::Read),
                "w" => Ok(Self::Write),
                s => Err(mlua::Error::RuntimeError(format!(
                    "unable to convert \"{}\" to physfs::Mode",
                    s
                ))),
            },
            val => Err(mlua::Error::RuntimeError(format!(
                "unable to convert {} to physfs::Mode",
                val.type_name()
            ))),
        }
    }
}

/// A file handle.
pub struct File {
    raw: AtomicPtr<naevc::PHYSFS_File>,
    //mode: Mode,
    _marker: std::marker::PhantomData<*mut ()>,
}

impl File {
    /// Opens a file with a specific mode.
    pub fn open(filename: &str, mode: Mode) -> Result<File> {
        let c_filename = CString::new(filename)?;
        let raw = unsafe {
            match mode {
                Mode::Append => naevc::PHYSFS_openAppend(c_filename.as_ptr()),
                Mode::Read => naevc::PHYSFS_openRead(c_filename.as_ptr()),
                Mode::Write => naevc::PHYSFS_openWrite(c_filename.as_ptr()),
            }
        };

        if raw.is_null() {
            Err(error_as_io_error_with_file("PHYSFS_open", filename))
        } else {
            Ok(File {
                raw: AtomicPtr::new(raw),
                //mode,
                _marker: std::marker::PhantomData,
            })
        }
    }

    /// Closes a file handle.
    fn close(&self) -> Result<()> {
        match unsafe { naevc::PHYSFS_close(self.raw.load(Ordering::Relaxed)) } {
            0 => Err(error_as_io_error("PHYSFS_close")),
            _ => Ok(()),
        }
    }

    /// Checks whether eof is reached or not.
    #[allow(dead_code)]
    pub fn eof(&self) -> bool {
        let ret = unsafe { naevc::PHYSFS_eof(self.raw.load(Ordering::Relaxed)) };
        ret != 0
    }

    /// Determine length of file, if possible
    pub fn len(&self) -> Result<u64> {
        let len = unsafe { naevc::PHYSFS_fileLength(self.raw.load(Ordering::Relaxed)) };
        if len >= 0 {
            Ok(len as u64)
        } else {
            Err(error_as_io_error("PHYSFS_fileLength"))
        }
    }

    /// Determine if file is empty
    pub fn is_empty(&self) -> bool {
        match self.len() {
            Ok(len) => len > 0,
            Err(_) => true,
        }
    }

    /// Determines current position within a file
    pub fn tell(&self) -> Result<u64> {
        let ret = unsafe { naevc::PHYSFS_tell(self.raw.load(Ordering::Relaxed)) };
        match ret {
            -1 => Err(error_as_io_error("PHYSFS_tell")),
            _ => Ok(ret as u64),
        }
    }
}

impl Read for File {
    /// Reads from a file
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        let ret = unsafe {
            naevc::PHYSFS_readBytes(
                self.raw.load(Ordering::Relaxed),
                buf.as_ptr() as *mut c_void,
                buf.len() as naevc::PHYSFS_uint64,
            )
        };
        match ret {
            -1 => Err(error_as_io_error("PHYSFS_readBytes")),
            _ => Ok(ret as usize),
        }
    }
}

impl Write for File {
    /// Writes to a file.
    /// This code performs no safety checks to ensure
    /// that the buffer is the correct length.
    fn write(&mut self, buf: &[u8]) -> Result<usize> {
        let ret = unsafe {
            naevc::PHYSFS_writeBytes(
                self.raw.load(Ordering::Relaxed),
                buf.as_ptr() as *const c_void,
                buf.len() as naevc::PHYSFS_uint64,
            )
        };

        match ret {
            -1 => Err(error_as_io_error("PHYSFS_writeBytes")),
            _ => Ok(ret as usize),
        }
    }

    /// Flushes a file if buffered; no-op if unbuffered.
    fn flush(&mut self) -> Result<()> {
        let ret = unsafe { naevc::PHYSFS_flush(self.raw.load(Ordering::Relaxed)) };
        match ret {
            0 => Err(error_as_io_error("PHYSFS_flush")),
            _ => Ok(()),
        }
    }
}

impl Seek for File {
    /// Seek to a new position within a file
    fn seek(&mut self, pos: SeekFrom) -> Result<u64> {
        let seek_pos = match pos {
            SeekFrom::Start(n) => n as i64,
            SeekFrom::End(n) => {
                let len = self.len()?;
                n + len as i64
            }
            SeekFrom::Current(n) => {
                let curr_pos = self.tell()?;
                n + curr_pos as i64
            }
        };
        let result = unsafe {
            naevc::PHYSFS_seek(
                self.raw.load(Ordering::Relaxed),
                seek_pos as naevc::PHYSFS_uint64,
            )
        };
        if result == -1 {
            return Err(error_as_io_error("PHYSFS_seek"));
        }
        self.tell()
    }
}

impl Drop for File {
    fn drop(&mut self) {
        let _ = self.close();
    }
}

pub fn exists(path: &str) -> bool {
    let cpath = CString::new(path).unwrap();
    !matches!(unsafe { naevc::PHYSFS_exists(cpath.as_ptr()) }, 0)
}

pub fn read_dir(path: &str) -> Result<Vec<String>> {
    let c_path = CString::new(path)?;
    let mut list = unsafe { naevc::PHYSFS_enumerateFiles(c_path.as_ptr()) };
    if list.is_null() {
        return Err(error_as_io_error_with_file("PHYSFS_enumerateFiles", path));
    }
    let listptr = list;

    let mut res = vec![];
    while !unsafe { *list }.is_null() {
        unsafe {
            let filename = format!("{}/{}", path, CStr::from_ptr(*list).to_str().unwrap());
            res.push(filename);
        }
        list = ((list as usize) + std::mem::size_of_val(&list)) as _;
    }
    unsafe {
        naevc::PHYSFS_freeList(listptr as *mut c_void);
    }

    Ok(res)
}

pub fn iostream(filename: &str, mode: Mode) -> Result<IOStream<'static>> {
    let raw = unsafe {
        let c_filename = CString::new(filename)?;
        let phys = match mode {
            Mode::Append => naevc::PHYSFS_openAppend(c_filename.as_ptr()),
            Mode::Read => naevc::PHYSFS_openRead(c_filename.as_ptr()),
            Mode::Write => naevc::PHYSFS_openWrite(c_filename.as_ptr()),
        };
        if phys.is_null() {
            return Err(error_as_io_error_with_file("PHYSFS_open", filename));
        }
        naevc::SDL_PhysFS_OpenIO(phys)
    };
    if raw.is_null() {
        Err(error_as_io_error_with_file("PHYSFS_open", filename))
    } else {
        Ok(unsafe { IOStream::from_ll(raw as *mut sdl::sys::iostream::SDL_IOStream) })
    }
}

pub fn blacklisted(filename: &str) -> bool {
    let c_filename = CString::new(filename).unwrap();
    let realdir = unsafe { naevc::PHYSFS_getRealDir(c_filename.as_ptr()) };
    if realdir.is_null() {
        return false;
    }
    let realdir = unsafe { CStr::from_ptr(realdir) };
    realdir.to_str().unwrap() == "naev.BLACKLIST"
}

pub struct LuaFile<'a> {
    io: IOStream<'a>,
}
unsafe impl Send for LuaFile<'_> {}

/*
 * @brief Lua bindings to interact with files.
 *
 * @note The API here is designed to be compatible with that of "LÖVE".
 *
 * @luamod file
 */
#[allow(unused_doc_comments)]
impl UserData for LuaFile<'_> {
    fn add_fields<F: mlua::UserDataFields<Self>>(fields: &mut F) {
        //fields.add_field_method_get("x", |_, this| Ok(this.0.x));
        //fields.add_field_method_get("y", |_, this| Ok(this.0.y));
    }
    fn add_methods<M: UserDataMethods<Self>>(methods: &mut M) {
        /*
        { "__gc", fileL_gc },       { "__eq", fileL_eq },
        { "new", fileL_new },       { "open", fileL_open },
        { "close", fileL_close },   { "from_string", fileL_from_string },
        { "read", fileL_read },     { "write", fileL_write },
        { "seek", fileL_seek },     { "getFilename", fileL_name },
        { "getMode", fileL_mode },  { "getSize", fileL_size },
        { "isOpen", fileL_isopen }, { "filetype", fileL_filetype },
        { "mkdir", fileL_mkdir },   { "enumerate", fileL_enumerate },
        { "remove", fileL_remove }, { 0, 0 } }; /**< File metatable methods. */
            */
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
            |_,
             (path, mode): (String, Mode)|
             -> mlua::Result<(Option<LuaFile<'static>>, Option<String>)> {
                let io = match iostream(&path, mode) {
                    Ok(io) => io,
                    Err(e) => {
                        return Ok((None, Some(e.to_string())));
                    }
                };
                Ok((Some(LuaFile { io }), None))
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
                match this.io.seek(SeekFrom::Start(pos)) {
                    Ok(res) => Ok((pos == res, None)),
                    Err(e) => Ok((false, Some(e.to_string()))),
                }
            },
        );
    }
}
