use std::{
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

use crate::metadata::Metadata;

mod linux;

#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Kind {
   /// The Linux AppImage and its zsync file.
   Appimage,
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
   };
   let output = args
      .output
      .clone()
      .unwrap_or_else(|| target.join("staging"));
   for kind in &args.kinds {
      match kind {
         Kind::Appimage => linux::appimage(&job, &output)?,
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
