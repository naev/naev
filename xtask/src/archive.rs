// Archives are built in Rust, so packaging doesn't depend on which tar and xz the host has.

use std::{
   fs::{self, File},
   io::{self, BufWriter, Read, Write},
   num::NonZeroU64,
   path::{Path, PathBuf},
};

use anyhow::{Context, Result};
use lzma_rust2::{XzOptions, XzWriterMt};
use sha2::{Digest, Sha256};

/// Extracts a tar stream into a directory.
pub fn unpack(stream: impl Read, into: &Path) -> Result<()> {
   tar::Archive::new(stream)
      .unpack(into)
      .with_context(|| format!("extracting into {}", into.display()))
}

/// Writes an xz-compressed tarball of the entries, each a file or directory relative to base.
pub fn tar_xz(out: &Path, base: &Path, entries: &[impl AsRef<Path>]) -> Result<()> {
   let file = File::create(out).with_context(|| format!("creating {}", out.display()))?;
   // xz's default preset, with the blocks of three dictionaries its threaded mode uses.
   let mut options = XzOptions::with_preset(6);
   options.set_block_size(NonZeroU64::new(
      u64::from(options.lzma_options.dict_size) * 3,
   ));
   let workers = std::thread::available_parallelism()
      .map_or(1, |count| u32::try_from(count.get()).unwrap_or(u32::MAX));
   let xz = XzWriterMt::new(BufWriter::new(file), options, workers)
      .with_context(|| format!("compressing {}", out.display()))?;

   let mut tar = tar::Builder::new(xz);
   tar.follow_symlinks(false);
   for entry in entries {
      let rel = entry.as_ref();
      let path = base.join(rel);
      let added = if path.is_dir() {
         tar.append_dir_all(rel, &path)
      } else {
         tar.append_path_with_name(&path, rel)
      };
      added.with_context(|| format!("archiving {}", path.display()))?;
   }
   let xz = tar
      .into_inner()
      .with_context(|| format!("finishing {}", out.display()))?;
   xz.finish()
      .and_then(|mut file| file.flush())
      .with_context(|| format!("finishing {}", out.display()))
}

/// Writes a deflated zip of the entries, each a file or directory relative to base, keeping their
/// Unix modes and symlinks. Directories are walked in name order, so the zip is reproducible.
pub fn zip(out: &Path, base: &Path, entries: &[impl AsRef<Path>]) -> Result<()> {
   let file = File::create(out).with_context(|| format!("creating {}", out.display()))?;
   let mut zip = zip::ZipWriter::new(BufWriter::new(file));
   let mut pending: Vec<PathBuf> = entries
      .iter()
      .rev()
      .map(|entry| entry.as_ref().to_path_buf())
      .collect();
   while let Some(rel) = pending.pop() {
      let path = base.join(&rel);
      let meta =
         fs::symlink_metadata(&path).with_context(|| format!("reading {}", path.display()))?;
      // Zip names always use forward slashes.
      let name = rel
         .components()
         .map(|part| part.as_os_str().to_string_lossy())
         .collect::<Vec<_>>()
         .join("/");
      let options = zip::write::SimpleFileOptions::default()
         .compression_method(zip::CompressionMethod::Deflated)
         .unix_permissions(mode(&meta))
         .large_file(meta.len() > u64::from(u32::MAX));
      if meta.file_type().is_symlink() {
         let target =
            fs::read_link(&path).with_context(|| format!("reading {}", path.display()))?;
         zip.add_symlink(name, target.to_string_lossy(), options)
      } else if meta.is_dir() {
         let mut children: Vec<PathBuf> = fs::read_dir(&path)
            .with_context(|| format!("reading {}", path.display()))?
            .map(|child| child.map(|child| rel.join(child.file_name())))
            .collect::<Result<_, _>>()
            .with_context(|| format!("reading {}", path.display()))?;
         children.sort_unstable_by(|a, b| b.cmp(a));
         pending.extend(children);
         zip.add_directory(name, options)
      } else {
         zip.start_file(name, options).and_then(|()| {
            let mut from = File::open(&path)?;
            io::copy(&mut from, &mut zip)?;
            Ok(())
         })
      }
      .with_context(|| format!("zipping {}", path.display()))?;
   }
   zip.finish()
      .context("finishing the zip")?
      .flush()
      .with_context(|| format!("writing {}", out.display()))
}

#[cfg(unix)]
fn mode(meta: &fs::Metadata) -> u32 {
   use std::os::unix::fs::PermissionsExt;

   meta.permissions().mode()
}

// Windows keeps no Unix modes, so directories get the usual 755 and files 644.
#[cfg(not(unix))]
fn mode(meta: &fs::Metadata) -> u32 {
   if meta.is_dir() { 0o755 } else { 0o644 }
}

/// The file's SHA-256, in lowercase hex.
pub fn sha256(file: &Path) -> Result<String> {
   let mut hasher = Sha256::new();
   let mut reader = File::open(file).with_context(|| format!("opening {}", file.display()))?;
   // Arbitrary, just large enough that reads stay few.
   let mut buffer = vec![0; 64 * 1024];
   loop {
      let read = reader
         .read(&mut buffer)
         .with_context(|| format!("reading {}", file.display()))?;
      if read == 0 {
         break;
      }
      hasher.update(&buffer[..read]);
   }
   Ok(hasher
      .finalize()
      .iter()
      .map(|byte| format!("{byte:02x}"))
      .collect())
}

/// Writes `<file>.sha256sum` beside the file, in the format sha256sum -c reads.
pub fn checksum(file: &Path) -> Result<()> {
   let name = file
      .file_name()
      .context("a checksummed file has a name")?
      .to_string_lossy();
   let sum = file.with_file_name(format!("{name}.sha256sum"));
   std::fs::write(&sum, format!("{}  {name}\n", sha256(file)?))
      .with_context(|| format!("writing {}", sum.display()))
}

#[cfg(test)]
mod tests {
   use super::*;

   #[cfg(unix)]
   #[test]
   fn zips_keep_modes_and_symlinks() {
      use std::os::unix::fs::PermissionsExt;

      let scratch = std::env::temp_dir().join(format!("xtask-zip-{}", std::process::id()));
      let app = scratch.join("Naev.app/Contents/MacOS");
      std::fs::create_dir_all(&app).expect("scratch dir is writable");
      std::fs::write(app.join("naev"), "exe").expect("scratch dir is writable");
      std::fs::set_permissions(app.join("naev"), std::fs::Permissions::from_mode(0o755))
         .expect("scratch dir is writable");
      std::fs::write(app.join("data.txt"), "data").expect("scratch dir is writable");
      std::os::unix::fs::symlink("naev", app.join("link")).expect("scratch dir is writable");

      let out = scratch.join("out.zip");
      zip(&out, &scratch, &["Naev.app"]).expect("zipping works");
      let mut archive =
         zip::ZipArchive::new(File::open(&out).expect("zip exists")).expect("zip parses");
      let mode = |archive: &mut zip::ZipArchive<File>, name: &str| {
         archive
            .by_name(name)
            .expect("entry exists")
            .unix_mode()
            .expect("entry has a mode")
      };
      assert_eq!(
         mode(&mut archive, "Naev.app/Contents/MacOS/naev") & 0o777,
         0o755
      );
      assert_eq!(
         mode(&mut archive, "Naev.app/Contents/MacOS/data.txt") & 0o111,
         0
      );
      assert!(
         archive
            .by_name("Naev.app/Contents/MacOS/link")
            .expect("entry exists")
            .is_symlink()
      );
      std::fs::remove_dir_all(&scratch).expect("scratch dir is removable");
   }

   #[test]
   fn tarballs_unpack_to_the_same_tree() {
      let scratch = std::env::temp_dir().join(format!("xtask-archive-{}", std::process::id()));
      let tree = scratch.join("tree");
      std::fs::create_dir_all(tree.join("dat/sub")).expect("scratch dir is writable");
      std::fs::write(tree.join("dat/sub/a.txt"), "hello").expect("scratch dir is writable");
      std::fs::write(tree.join("naev.exe"), [0u8, 1, 2]).expect("scratch dir is writable");

      let tarball = scratch.join("out.tar.xz");
      tar_xz(&tarball, &tree, &["dat", "naev.exe"]).expect("archiving works");
      checksum(&tarball).expect("checksumming works");

      let back = scratch.join("back");
      let stream = lzma_rust2::XzReader::new(File::open(&tarball).expect("tarball exists"), true);
      unpack(stream, &back).expect("unpacking works");
      assert_eq!(
         std::fs::read_to_string(back.join("dat/sub/a.txt")).expect("file came back"),
         "hello"
      );
      assert_eq!(
         std::fs::read(back.join("naev.exe")).expect("file came back"),
         [0, 1, 2]
      );
      let sum = std::fs::read_to_string(scratch.join("out.tar.xz.sha256sum")).expect("sum exists");
      assert_eq!(
         sum,
         format!("{}  out.tar.xz\n", sha256(&tarball).expect("hashing works"))
      );
      std::fs::remove_dir_all(&scratch).expect("scratch dir is removable");
   }
}
