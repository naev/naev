//! Installing the game.
//!
//! The layout follows the one meson installed, and DESTDIR works as make and
//! meson use it: everything is placed under the prefix, with DESTDIR prepended
//! when staging into a package root rather than onto the running system.

use std::{
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

/// Files under dat/ that feed the generators rather than shipping. The
/// generated AUTHORS replaces the tracked preamble of the same name.
const SKIP_FILES: &[&str] = &[
   "AUTHORS",
   "outfits/meson.build",
   "outfits/bioship/generate.py",
   "outfits/bioship/meson.build",
   "scripts/meson.build",
   "outfits/tech/meson.build",
];

/// Directories under dat/ holding generator inputs.
const SKIP_DIRS: &[&str] = &["outfits/py", "outfits/bioship/templates"];

/// The icon sizes the desktop expects, and the source each is taken from.
const ICONS: &[(&str, &str)] = &[
   ("16x16", "logo16.png"),
   ("32x32", "logo32.png"),
   ("64x64", "logo64.png"),
   ("128x128", "logo128.png"),
   ("256x256", "naev.png"),
];

/// A part of the install that can be asked for on its own, so a packager
/// splitting the game from its documentation can take one without the other.
#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Component {
   /// The binary, the data it loads, and the desktop entry around it.
   Game,
   /// The C and Lua references.
   Docs,
}

impl Component {
   /// Whether a request that names nothing installs this.
   fn by_default(self) -> bool {
      match self {
         Component::Game => true,
         Component::Docs => false,
      }
   }
}

#[derive(Args, Debug)]
pub struct InstallArgs {
   /// What to install, defaulting to the game on its own.
   #[arg(value_enum)]
   components: Vec<Component>,

   /// Staging root prepended to every path, for building a package rather
   /// than installing onto the running system.
   #[arg(long, env = "DESTDIR")]
   destdir: Option<PathBuf>,

   /// Where the install is rooted.
   #[arg(long, default_value = "/usr/local")]
   prefix: PathBuf,

   /// Binary directory, relative to the prefix.
   #[arg(long, default_value = "bin")]
   bindir: PathBuf,

   /// Data directory, relative to the prefix.
   #[arg(long, default_value = "share")]
   datadir: PathBuf,

   /// Where ndata goes, relative to the prefix. Defaults to <datadir>/naev.
   #[arg(long)]
   ndata_path: Option<PathBuf>,

   /// Install the release binary rather than the debug one.
   #[arg(long)]
   release: bool,

   /// Cargo features to build the engine with.
   #[arg(long, value_delimiter = ',')]
   features: Vec<String>,
}

impl InstallArgs {
   /// Where ndata ends up once installed, relative to the prefix.
   ///
   /// meson defaulted its ndata_path option to `<datadir>/naev`, and the same
   /// value drove both the install and the compiled-in path.
   fn ndata(&self) -> PathBuf {
      self.ndata_path
         .clone()
         .unwrap_or_else(|| self.datadir.join("naev"))
   }

   /// The absolute path the engine looks in for its data.
   ///
   /// DESTDIR is deliberately not part of this. Staging moves where files are
   /// written, not where the installed game will later read them from.
   fn pkgdatadir(&self) -> PathBuf {
      self.prefix.join(self.ndata())
   }
}

/// The components an empty request installs, named for the help output.
pub fn defaults_help() -> String {
   let (on, off): (Vec<_>, Vec<_>) = Component::value_variants()
      .iter()
      .partition(|component| component.by_default());
   // The value names, so the footer names what a caller would type.
   let names = |components: Vec<&Component>| {
      components
         .iter()
         .filter_map(|component| component.to_possible_value())
         .map(|value| value.get_name().to_owned())
         .collect::<Vec<_>>()
         .join(", ")
   };
   format!(
      "Installed by default: {}\nOn request only:      {}",
      names(on),
      names(off)
   )
}

pub fn install(root: &Path, target: &Path, data_dir: &Path, args: &InstallArgs) -> Result<()> {
   let defaults: Vec<Component> = Component::value_variants()
      .iter()
      .copied()
      .filter(|component| component.by_default())
      .collect();
   let wanted = if args.components.is_empty() {
      &defaults
   } else {
      &args.components
   };

   let dest = |rel: &Path| staged(args.destdir.as_deref(), &args.prefix, rel);
   let mut files = 0;
   for component in wanted {
      files += match component {
         Component::Game => game(root, target, data_dir, args, &dest)?,
         Component::Docs => docs(root, target, args, &dest)?,
      };
   }

   println!(
      "installed {files} files to {}",
      dest(Path::new("")).display()
   );
   Ok(())
}

/// Builds the engine with the data path this install is going to use.
fn build_engine(root: &Path, args: &InstallArgs) -> Result<()> {
   // Reuse the cargo that invoked us. Under a toolchain override it is not
   // the one on PATH.
   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let mut cmd = Command::new(cargo);
   cmd.arg("build")
      .arg("--manifest-path")
      .arg(root.join("Cargo.toml"))
      .arg("--package")
      .arg("naev")
      .env("NAEV_PKGDATADIR", args.pkgdatadir());
   if args.release {
      cmd.arg("--release");
   }
   if !args.features.is_empty() {
      cmd.arg("--features").arg(args.features.join(","));
   }

   let status = cmd.status().context("failed to run cargo build")?;
   if !status.success() {
      bail!("cargo build failed with {status}");
   }
   Ok(())
}

/// The binary, everything it loads, and the entries a desktop needs to show it.
fn game(
   root: &Path,
   target: &Path,
   data_dir: &Path,
   args: &InstallArgs,
   dest: &impl Fn(&Path) -> PathBuf,
) -> Result<usize> {
   // The engine has to be compiled knowing where its data will live, so the
   // build happens here rather than being left to the caller. Doing it any
   // other way lets the two disagree, and the result only fails once someone
   // runs the game.
   build_engine(root, args)?;

   let profile = if args.release { "release" } else { "debug" };
   let binary = target.join(profile).join("naev");
   if !binary.is_file() {
      bail!("no binary at {} after building it", binary.display());
   }

   let datadir = &args.datadir;
   let ndata = args.ndata();

   copy(&binary, &dest(&args.bindir.join("naev")), 0o755)?;
   let mut files = 1;

   // dat/ ships as it is, minus what only the generators read.
   let dat = dest(&ndata.join("dat"));
   files += copy_tree(&root.join("dat"), &dat, SKIP_FILES, SKIP_DIRS)?;

   // assets/ merges into the same directory rather than nesting under its own.
   files += copy_tree(&root.join("assets"), &dat, &[], &[".git"])?;

   // Built where it usually lives and copied in, so installing twice does not
   // run the generators twice.
   crate::data::generate(root, data_dir)?;
   files += copy_tree(data_dir, &dat, &[], &[])?;

   for (name, dir) in [
      ("org.naev.Naev.desktop", datadir.join("applications")),
      ("org.naev.Naev.metainfo.xml", datadir.join("metainfo")),
   ] {
      copy(&root.join(name), &dest(&dir.join(name)), 0o644)?;
      files += 1;
   }

   for (size, source) in ICONS {
      let to = datadir
         .join("icons/hicolor")
         .join(size)
         .join("apps/org.naev.Naev.png");
      copy(&root.join("extras/logos").join(source), &dest(&to), 0o644)?;
      files += 1;
   }

   copy(
      &root.join("naev.6"),
      &dest(&datadir.join("man/man6/naev.6")),
      0o644,
   )?;
   files += 1;

   for name in ["gpl.txt", "LICENSE", "Readme.md"] {
      copy(
         &root.join(name),
         &dest(&datadir.join("doc/naev").join(name)),
         0o644,
      )?;
      files += 1;
   }
   Ok(files)
}

/// The C and Lua references, built first. meson installed these straight under
/// the prefix rather than beside the licences in datadir.
fn docs(
   root: &Path,
   target: &Path,
   args: &InstallArgs,
   dest: &impl Fn(&Path) -> PathBuf,
) -> Result<usize> {
   let built = target.join("doc");
   crate::docs::build(
      root,
      target,
      &[crate::docs::Kind::C, crate::docs::Kind::Lua],
      Some(built.clone()),
   )?;

   let mut files = 0;
   for reference in ["c", "lua"] {
      let to = dest(&args.datadir.join("doc/naev").join(reference));
      files += copy_tree(&built.join(reference), &to, &[], &[])?;
   }
   Ok(files)
}

/// Places a path under the prefix, and under DESTDIR when one is given. The
/// prefix is absolute, so its leading separator is dropped when nesting.
fn staged(destdir: Option<&Path>, prefix: &Path, rel: &Path) -> PathBuf {
   let full = prefix.join(rel);
   match destdir {
      None => full,
      Some(destdir) => destdir.join(full.strip_prefix("/").unwrap_or(&full)),
   }
}

fn copy(from: &Path, to: &Path, mode: u32) -> Result<()> {
   let parent = to.parent().context("every target has a directory")?;
   fs::create_dir_all(parent).with_context(|| format!("creating {}", parent.display()))?;
   fs::copy(from, to).with_context(|| format!("copying {}", from.display()))?;
   set_mode(to, mode)
}

#[cfg(unix)]
fn set_mode(path: &Path, mode: u32) -> Result<()> {
   use std::os::unix::fs::PermissionsExt;

   fs::set_permissions(path, fs::Permissions::from_mode(mode))
      .with_context(|| format!("setting the mode on {}", path.display()))
}

#[cfg(not(unix))]
fn set_mode(_path: &Path, _mode: u32) -> Result<()> {
   Ok(())
}

/// Copies a directory across, skipping the paths named relative to its root.
fn copy_tree(from: &Path, to: &Path, skip_files: &[&str], skip_dirs: &[&str]) -> Result<usize> {
   let mut copied = 0;
   let mut pending = vec![PathBuf::new()];
   while let Some(rel) = pending.pop() {
      let dir = from.join(&rel);
      for entry in fs::read_dir(&dir).with_context(|| format!("reading {}", dir.display()))? {
         let entry = entry.with_context(|| format!("walking {}", dir.display()))?;
         let path = rel.join(entry.file_name());
         let named = path.to_string_lossy();

         if entry.path().is_dir() {
            if !skip_dirs.iter().any(|skip| *skip == named) {
               pending.push(path);
            }
         } else if !skip_files.iter().any(|skip| *skip == named) {
            copy(&entry.path(), &to.join(&path), 0o644)?;
            copied += 1;
         }
      }
   }
   Ok(copied)
}
