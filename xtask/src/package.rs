use std::{
   cell::OnceCell,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

use crate::metadata::Metadata;

mod linux;
mod macos;
mod soundtrack;
mod windows;

#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Kind {
   /// The Linux AppImage and its zsync file.
   Appimage,
   /// The Linux binary, data and libraries for the Steam depots.
   Steam,
   /// The Windows binary and the DLLs it needs, for the Steam depots.
   Windows,
   /// The Windows installer.
   Installer,
   /// The universal macOS app, zipped for the Steam depot.
   Macos,
   /// The universal macOS app on a disk image.
   Dmg,
   /// The tagged soundtrack, and its MP3s for the Steam soundtrack depot.
   Soundtrack,
}

#[derive(Args, Debug)]
pub struct PackageArgs {
   /// What to package.
   #[arg(value_enum, required = true)]
   kinds: Vec<Kind>,

   /// Where the packages are written, each in the directory the release jobs hand off.
   /// Defaults to staging/ under cargo's target directory.
   #[arg(long)]
   output: Option<PathBuf>,

   /// Point the AppImage's updates at the nightly release.
   #[arg(long)]
   nightly: bool,

   /// Leave the AppImage uncompressed, which is much faster to build.
   #[arg(long)]
   no_compression: bool,

   #[command(flatten)]
   build: crate::run::BuildArgs,
}

/// Everything a packager reads: the tree, cargo's output, the project metadata and the request.
struct Job<'a> {
   root: &'a Path,
   target: &'a Path,
   data_dir: &'a Path,
   metadata: &'a Metadata,
   args: &'a PackageArgs,
   // Both Windows packages are built from one staged install.
   windows_stage: OnceCell<PathBuf>,
   // As are both macOS ones, from one universal app.
   macos_app: OnceCell<PathBuf>,
}

pub fn package(
   root: &Path,
   target: &Path,
   data_dir: &Path,
   metadata: &Metadata,
   args: &PackageArgs,
) -> Result<()> {
   let job = Job {
      root,
      target,
      data_dir,
      metadata,
      args,
      windows_stage: OnceCell::new(),
      macos_app: OnceCell::new(),
   };
   let output = args
      .output
      .clone()
      .unwrap_or_else(|| target.join("staging"));
   for kind in &args.kinds {
      match kind {
         Kind::Appimage => linux::appimage(&job, &output)?,
         Kind::Steam => linux::steam(&job, &output)?,
         Kind::Windows => windows::tarball(&job, &output)?,
         Kind::Installer => windows::installer(&job, &output)?,
         Kind::Macos => macos::zip(&job, &output)?,
         Kind::Dmg => macos::dmg(&job, &output)?,
         Kind::Soundtrack => soundtrack::soundtrack(&job, &output)?,
      }
   }
   Ok(())
}

fn run(mut cmd: Command, what: &str) -> Result<()> {
   let status = cmd
      .status()
      .with_context(|| format!("failed to run {what}"))?;
   if !status.success() {
      bail!("{what} failed with {status}");
   }
   Ok(())
}
