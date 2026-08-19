//! Build tasks that sit outside what cargo itself builds: generating the data
//! files that ship in ndata, and eventually translations, docs and packaging.
//!
//! Run through the alias in `.cargo/config.toml`, for example `cargo xtask
//! data`.

use std::path::{Path, PathBuf};

use anyhow::{Context, Result, bail};
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

   /// The generated data tree, defaulting to dat/ under cargo's target
   /// directory. Packagers staging elsewhere point this at their own tree.
   #[arg(long, global = true)]
   data_dir: Option<PathBuf>,
}

#[derive(Subcommand)]
enum Command {
   /// Generate the derived data files that ship alongside dat/.
   Data,
   /// Refresh POTFILES.in and the translation template.
   Pot,
   /// Bring every translation catalogue up to date with the template.
   UpdatePo,
}

fn main() -> Result<()> {
   let cli = Cli::parse();
   let root = repo_root()?;
   let data_dir = match cli.data_dir {
      Some(dir) => dir,
      None => target_dir(&root)?.join("dat"),
   };

   match cli.command {
      Command::Data => data::generate(&root, &data_dir),
      Command::Pot => i18n::pot(&root, &data_dir),
      Command::UpdatePo => i18n::update_po(&root, &data_dir),
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

/// Where cargo puts its artifacts.
///
/// Nothing in the environment carries this, so cargo has to be asked.
fn target_dir(root: &Path) -> Result<PathBuf> {
   #[derive(serde::Deserialize)]
   struct Metadata {
      target_directory: PathBuf,
   }

   // Reuse the cargo that invoked us. Under a toolchain override it is not
   // the one on PATH.
   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let output = std::process::Command::new(cargo)
      .args(["metadata", "--no-deps", "--format-version", "1"])
      .arg("--manifest-path")
      .arg(root.join("Cargo.toml"))
      .output()
      .context("failed to run cargo metadata")?;
   if !output.status.success() {
      bail!(
         "cargo metadata failed with {}\n{}",
         output.status,
         String::from_utf8_lossy(&output.stderr)
      );
   }

   let metadata: Metadata =
      serde_json::from_slice(&output.stdout).context("parsing the output of cargo metadata")?;
   Ok(metadata.target_directory)
}
