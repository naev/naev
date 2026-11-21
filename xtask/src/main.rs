use std::path::{Path, PathBuf};

use anyhow::{Context, Result, bail};
use clap::{Parser, Subcommand};

mod bioship;
mod data;
mod docs;
mod generated;
mod i18n;
// The root build script fills templates with the rest, which xtask doesn't need yet.
#[allow(dead_code)]
#[path = "../../extras/metadata.rs"]
mod metadata;
mod progress;
mod rule;
mod run;
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
   /// Refresh POTFILES.in and the translation template.
   Pot,
   /// Bring every translation catalogue up to date with the template.
   UpdatePo,
   /// Build and run the game out of the source tree.
   Run(run::RunArgs),
   /// Build and run the game under valgrind.
   Valgrind(run::ValgrindArgs),
   /// Build the documentation.
   #[command(after_help = docs::defaults_help())]
   Docs {
      /// Which sets to build. Defaults to lua and manual, skipping any whose tool is missing.
      #[arg(value_enum)]
      kinds: Vec<docs::Kind>,

      /// Where the built documentation is written.
      #[arg(long)]
      output: Option<PathBuf>,
   },
}

fn main() -> Result<()> {
   let cli = Cli::parse();
   let root = repo_root()?;
   let target = cargo_metadata(&root)?.target_directory;
   let naev = metadata::Metadata::read(&root).map_err(anyhow::Error::msg)?;
   let data_dir = cli.data_dir.unwrap_or_else(|| target.join("dat"));

   match cli.command {
      Command::Data => data::generate(&root, &data_dir),
      Command::Pot => i18n::pot(&root, &data_dir, &naev),
      Command::UpdatePo => i18n::update_po(&root, &data_dir, &naev),
      Command::Run(args) => run::run(&root, &target, &data_dir, &args),
      Command::Valgrind(args) => run::valgrind(&root, &target, &data_dir, &args),
      Command::Docs { kinds, output } => docs::build(&root, &target, &kinds, output),
   }
}

fn repo_root() -> Result<PathBuf> {
   let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
   manifest
      .parent()
      .map(Path::to_path_buf)
      .context("xtask should live one level below the repo root")
}

#[derive(serde::Deserialize)]
struct CargoMetadata {
   target_directory: PathBuf,
}

// Nothing in the environment carries cargo's target directory, so cargo has to be asked.
fn cargo_metadata(root: &Path) -> Result<CargoMetadata> {
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

   serde_json::from_slice(&output.stdout).context("parsing the output of cargo metadata")
}
