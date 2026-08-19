//! Build tasks that sit outside what cargo itself builds: generating the data
//! files that ship in ndata, and eventually translations, docs and packaging.
//!
//! Run through the alias in `.cargo/config.toml`, for example
//! `cargo xtask data --output builddir/dat`.

use std::path::{Path, PathBuf};

use anyhow::{Context, Result};
use clap::{Parser, Subcommand};

mod bioship;
mod data;
mod generated;
mod i18n;

#[derive(Parser)]
#[command(about, long_about = None)]
struct Cli {
   #[command(subcommand)]
   command: Command,
}

#[derive(Subcommand)]
enum Command {
   /// Generate the derived data files that ship alongside dat/.
   Data {
      /// Where the generated tree is written, normally <builddir>/dat.
      #[arg(long)]
      output: PathBuf,
   },
   /// Refresh POTFILES.in and the translation template.
   Pot {
      /// A build directory holding the generated data, some of which carries
      /// translatable text.
      #[arg(long)]
      build_dir: PathBuf,
   },
   /// Bring every translation catalogue up to date with the template.
   UpdatePo {
      /// A build directory holding the generated data.
      #[arg(long)]
      build_dir: PathBuf,
   },
}

fn main() -> Result<()> {
   let cli = Cli::parse();
   let root = repo_root()?;

   match cli.command {
      Command::Data { output } => data::generate(&root, &output),
      Command::Pot { build_dir } => i18n::pot(&root, &build_dir),
      Command::UpdatePo { build_dir } => i18n::update_po(&root, &build_dir),
   }
}

/// The repo root, found from this crate rather than the working directory so
/// the task can be run from anywhere.
fn repo_root() -> Result<PathBuf> {
   let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
   manifest
      .parent()
      .map(Path::to_path_buf)
      .context("xtask should live one level below the repo root")
}
