use std::{
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

// Build files and generator inputs. The generated tree has its own AUTHORS and VERSION.
const SKIP_FILES: &[&str] = &[
   "AUTHORS",
   "VERSION",
   "meson.build",
   "outfits/bioship/generate.py",
   "scripts/meson.build",
];

const SKIP_DIRS: &[&str] = &["outfits/bioship/templates"];

const ICONS: &[(&str, &str)] = &[
   ("16x16", "logo16.png"),
   ("32x32", "logo32.png"),
   ("64x64", "logo64.png"),
   ("128x128", "logo128.png"),
   ("256x256", "naev.png"),
];

/// A part of the install that a packager can ask for on its own.
#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Component {
   /// The binary, the data it loads, and the desktop entry around it.
   Game,
   /// The C and Lua references.
   Docs,
}

impl Component {
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

   /// Staging root prepended to every path, for building a package.
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

   #[command(flatten)]
   build: crate::run::BuildArgs,
}

impl InstallArgs {
   fn ndata(&self) -> PathBuf {
      self
         .ndata_path
         .clone()
         .unwrap_or_else(|| self.datadir.join("naev"))
   }

   // DESTDIR stays out, since the installed game reads from the unstaged path.
   fn pkgdatadir(&self) -> PathBuf {
      self.prefix.join(self.ndata())
   }
}

pub fn defaults_help() -> String {
   let (on, off): (Vec<_>, Vec<_>) = Component::value_variants()
      .iter()
      .partition(|component| component.by_default());
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

pub fn install(
   root: &Path,
   target: &Path,
   data_dir: &Path,
   metadata: &crate::metadata::Metadata,
   args: &InstallArgs,
) -> Result<()> {
   // xtask always builds for the host, and only the Linux and BSD layout exists so far.
   if !cfg!(any(
      target_os = "linux",
      target_os = "freebsd",
      target_os = "openbsd",
      target_os = "netbsd"
   )) {
      bail!(
         "xtask install only lays out Linux and BSD installs; use meson install on this platform"
      );
   }

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
         Component::Game => game(root, target, data_dir, metadata, args, &dest)?,
         Component::Docs => docs(root, target, args, &dest)?,
      };
   }

   println!(
      "installed {files} files to {}",
      dest(Path::new("")).display()
   );
   Ok(())
}

fn build_engine(root: &Path, args: &InstallArgs) -> Result<()> {
   // Under a toolchain override, the cargo that invoked us isn't the one on PATH.
   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let mut cmd = Command::new(cargo);
   cmd.arg("build")
      .arg("--manifest-path")
      .arg(root.join("Cargo.toml"))
      .arg("--package")
      .arg("naev")
      .env("NAEV_PKGDATADIR", args.pkgdatadir())
      // Stamped as meson stamps its builds, so the installed game reports its commit.
      .env("NAEV_BUILD_VERSION", crate::version::current(root)?);
   args.build.cargo_args(&mut cmd);

   let status = cmd.status().context("failed to run cargo build")?;
   if !status.success() {
      bail!("cargo build failed with {status}");
   }
   Ok(())
}

fn game(
   root: &Path,
   target: &Path,
   data_dir: &Path,
   metadata: &crate::metadata::Metadata,
   args: &InstallArgs,
   dest: &impl Fn(&Path) -> PathBuf,
) -> Result<usize> {
   // The engine compiles in its data path, so it's built here to match this install.
   build_engine(root, args)?;

   let binary = args.build.binary(target);
   if !binary.is_file() {
      bail!("no binary at {} after building it", binary.display());
   }

   let datadir = &args.datadir;
   let ndata = args.ndata();

   let name = binary.file_name().context("the binary has a file name")?;
   copy(&binary, &dest(&args.bindir.join(name)), 0o755)?;
   let mut files = 1;

   let dat = dest(&ndata.join("dat"));
   files += copy_tree(&root.join("dat"), &dat, SKIP_FILES, SKIP_DIRS)?;

   files += copy_tree(&root.join("assets"), &dat, &[], &[])?;

   // Generated into the usual data dir, so a reinstall reuses it.
   crate::data::generate(root, data_dir)?;
   files += copy_tree(data_dir, &dat, &[], &[])?;

   // The desktop names its entry, metainfo and icon after the app ID.
   let app_id = &metadata.app_id;
   for (name, dir) in [
      (format!("{app_id}.desktop"), datadir.join("applications")),
      (format!("{app_id}.metainfo.xml"), datadir.join("metainfo")),
   ] {
      copy(&root.join(&name), &dest(&dir.join(&name)), 0o644)?;
      files += 1;
   }

   for (size, source) in ICONS {
      let to = datadir
         .join("icons/hicolor")
         .join(size)
         .join(format!("apps/{app_id}.png"));
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

// join replaces the base with an absolute path, so the prefix's root is stripped.
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

fn copy_tree(from: &Path, to: &Path, skip_files: &[&str], skip_dirs: &[&str]) -> Result<usize> {
   let mut copied = 0;
   let mut pending = vec![PathBuf::new()];
   while let Some(rel) = pending.pop() {
      let dir = from.join(&rel);
      for entry in fs::read_dir(&dir).with_context(|| format!("reading {}", dir.display()))? {
         let entry = entry.with_context(|| format!("walking {}", dir.display()))?;
         let path = rel.join(entry.file_name());
         // A submodule's .git is a file, and a checkout's a directory.
         if entry.file_name() == ".git" {
            continue;
         }
         // Compared as paths, since Windows walks with backslashes.
         let skipped = |list: &[&str]| list.iter().any(|skip| path == Path::new(skip));

         if entry.path().is_dir() {
            // Python leaves its bytecode caches beside the generators each time they run.
            let cache = entry.file_name() == "__pycache__";
            if !cache && !skipped(skip_dirs) {
               pending.push(path);
            }
         } else if !skipped(skip_files) {
            copy(&entry.path(), &to.join(&path), 0o644)?;
            copied += 1;
         }
      }
   }
   Ok(copied)
}
