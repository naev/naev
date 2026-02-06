use anyhow::Result;
use std::fs;
use std::io::Read;
use std::path::{Path, PathBuf};
use walkdir::WalkDir;
use zip::ZipArchive;

/// Trait representing a read-only mounted plugin source.
pub trait PluginMount {
   /// Lists all files contained in the mount, relative to the root.
   fn list_files(&mut self) -> Result<Vec<String>>;
   /// Reads an individual file from the mount into memory.
   fn read_file(&mut self, path: &str) -> Result<Vec<u8>>;
   /// Returns the underlying path or identifier for the mount.
   fn source_path(&self) -> &Path;
}

/// Directory-backed plugin mount.
pub struct DirMount {
   root: PathBuf,
}

impl DirMount {
   pub fn new<P: AsRef<Path>>(path: P) -> Self {
      Self {
         root: path.as_ref().to_path_buf(),
      }
   }
}

impl PluginMount for DirMount {
   fn list_files(&mut self) -> Result<Vec<String>> {
      let mut out = Vec::new();
      for entry in WalkDir::new(&self.root) {
         let entry = entry?;
         if entry.file_type().is_file() {
            out.push(entry.path().strip_prefix(&self.root)?.display().to_string());
         }
      }
      Ok(out)
   }

   fn read_file(&mut self, path: &str) -> Result<Vec<u8>> {
      Ok(fs::read(self.root.join(path))?)
   }

   fn source_path(&self) -> &Path {
      &self.root
   }
}

/// Zip archive-backed plugin mount.
pub struct ZipMount {
   path: PathBuf,
   zip: ZipArchive<fs::File>,
}

impl ZipMount {
   pub fn open<P: AsRef<Path>>(path: P) -> Result<Self> {
      let path_buf = path.as_ref().to_path_buf();
      let file = fs::File::open(&path_buf)?;
      let zip = ZipArchive::new(file)?;
      Ok(Self {
         path: path_buf,
         zip,
      })
   }
}

impl PluginMount for ZipMount {
   fn list_files(&mut self) -> Result<Vec<String>> {
      let mut out = Vec::new();
      for i in 0..self.zip.len() {
         let file = self.zip.by_index(i)?;
         if !file.name().ends_with('/') {
            out.push(file.name().to_string());
         }
      }
      Ok(out)
   }

   fn read_file(&mut self, path: &str) -> Result<Vec<u8>> {
      let mut file = self.zip.by_name(path)?;
      let mut buf = Vec::new();
      file.read_to_end(&mut buf)?;
      Ok(buf)
   }

   fn source_path(&self) -> &Path {
      &self.path
   }
}

/// General enum wrapping both types of mounts.
pub enum PluginSource {
   Dir(DirMount),
   Zip(ZipMount),
}

impl PluginSource {
   pub fn open<P: AsRef<Path>>(path: P) -> Result<Self> {
      let path_ref = path.as_ref();
      if path_ref.is_file() && path_ref.extension().and_then(|e| e.to_str()) == Some("zip") {
         Ok(Self::Zip(ZipMount::open(path_ref)?))
      } else {
         Ok(Self::Dir(DirMount::new(path_ref)))
      }
   }

   pub fn list_files(&mut self) -> Result<Vec<String>> {
      match self {
         PluginSource::Dir(m) => m.list_files(),
         PluginSource::Zip(m) => m.list_files(),
      }
   }

   pub fn read_file(&mut self, path: &str) -> Result<Vec<u8>> {
      match self {
         PluginSource::Dir(m) => m.read_file(path),
         PluginSource::Zip(m) => m.read_file(path),
      }
   }

   pub fn source_path(&self) -> &Path {
      match self {
         PluginSource::Dir(m) => m.source_path(),
         PluginSource::Zip(m) => m.source_path(),
      }
   }
}

// Usage example:
// let mut src = PluginSource::open("/path/to/plugin.zip")?;
// for f in src.list_files()? {
//     println!("{}", f);
// }
