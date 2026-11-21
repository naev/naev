use std::path::{Path, PathBuf};

use anyhow::{Context, Result, bail};
use clap::{Parser, Subcommand};

mod bioship;
mod data;
mod generated;
mod i18n;
mod progress;
mod rule;
mod version;

#[derive(Parser)]
#[command(about, long_about = None)]
struct Cli {
   #[command(subcommand)]
   command: Command,

   /// The generated data tree, defaulting to dat/ under cargo's target directory.
   #[arg(long = "generated-dir", global = true)]
   data_dir: Option<PathBuf>,
}

#[derive(Subcommand)]
enum Command {
   /// Generate the derived data files that ship alongside dat/.
   Data,
}

fn main() -> Result<()> {
   let cli = Cli::parse();
   let root = repo_root()?;
   let target = target_dir(&root)?;
   let data_dir = cli.data_dir.unwrap_or_else(|| target.join("dat"));

   match cli.command {
      Command::Data => data::generate(&root, &data_dir),
   }
}

fn repo_root() -> Result<PathBuf> {
   let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
   manifest
      .parent()
      .map(Path::to_path_buf)
      .context("xtask should live one level below the repo root")
}

/// Nothing in the environment carries cargo's target directory, so cargo has to be asked.
fn target_dir(root: &Path) -> Result<PathBuf> {
   #[derive(serde::Deserialize)]
   struct Metadata {
      target_directory: PathBuf,
   }

   // Under a toolchain override the cargo on PATH isn't the one that invoked us.
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
