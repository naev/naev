//! Documentation builds.
//!
//! Each set needs its own tools installed, so asking for everything builds
//! what it can and says what it skipped. Naming a set explicitly makes a
//! missing tool an error instead.

use std::{
   fmt, fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::ValueEnum;

use crate::run::on_path;

// The same C sources the build script compiles.
#[path = "../../naevc/build/sources.rs"]
mod sources;

/// A documentation set.
#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Kind {
   /// The engine reference, through doxygen.
   C,
   /// The Lua API reference, through docmaker and ldoc.
   Lua,
   /// The developer manual, through mdbook.
   Manual,
   /// The lore website, through loremaster.
   Lore,
}

impl Kind {
   /// Whether a request that names nothing builds this. doxygen sweeps the
   /// whole engine and the lore website wants a ruby bundle, so both wait to
   /// be asked for.
   fn by_default(self) -> bool {
      match self {
         Kind::Lua | Kind::Manual => true,
         Kind::C | Kind::Lore => false,
      }
   }

   /// Where to get the tools, for the ones that are not packaged everywhere.
   fn install_hint(self) -> Option<&'static str> {
      match self {
         Kind::Manual => Some("cargo install mdbook"),
         Kind::C | Kind::Lua | Kind::Lore => None,
      }
   }

   /// The programs that have to be installed to build this.
   fn tools(self) -> &'static [&'static str] {
      match self {
         Kind::C => &["doxygen"],
         Kind::Lua => &["ldoc"],
         Kind::Manual => &["mdbook"],
         Kind::Lore => &["bundle", "tidy", "yq", "gm"],
      }
   }
}

impl fmt::Display for Kind {
   fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
      f.write_str(match self {
         Kind::C => "C",
         Kind::Lua => "Lua",
         Kind::Manual => "manual",
         Kind::Lore => "lore",
      })
   }
}

/// The sets an empty request builds, named for the help output.
pub fn defaults_help() -> String {
   let (on, off): (Vec<_>, Vec<_>) = Kind::value_variants()
      .iter()
      .partition(|kind| kind.by_default());
   // The value names, so the footer names what a caller would type.
   let names = |kinds: Vec<&Kind>| {
      kinds
         .iter()
         .filter_map(|kind| kind.to_possible_value())
         .map(|value| value.get_name().to_owned())
         .collect::<Vec<_>>()
         .join(", ")
   };
   format!(
      "Built by default: {}\nOn request only:  {}",
      names(on),
      names(off)
   )
}

/// Builds the requested documentation, or everything with its tools present.
pub fn build(root: &Path, target: &Path, kinds: &[Kind], output: Option<PathBuf>) -> Result<()> {
   let out = output.unwrap_or_else(|| target.join("doc"));
   let named = !kinds.is_empty();
   let defaults: Vec<Kind> = Kind::value_variants()
      .iter()
      .copied()
      .filter(|kind| kind.by_default())
      .collect();
   let wanted = if named { kinds } else { &defaults };

   for &kind in wanted {
      if let Some(tool) = kind.tools().iter().find(|tool| !on_path(tool)) {
         let hint = kind
            .install_hint()
            .map(|hint| format!(" ({hint})"))
            .unwrap_or_default();
         if named {
            bail!("the {kind} documentation needs {tool}, which is not on PATH{hint}");
         }
         println!("skipping the {kind} documentation: {tool} is not installed{hint}");
         continue;
      }

      match kind {
         Kind::C => cdoc(root, target, &out)?,
         Kind::Lua => ldoc(root, target, &out)?,
         Kind::Manual => manual(root, &out)?,
         Kind::Lore => lore(root, target)?,
      }
      println!("built the {kind} documentation");
   }
   Ok(())
}

/// The engine reference, covering the sources cargo compiles and every header
/// under src/.
fn cdoc(root: &Path, target: &Path, out: &Path) -> Result<()> {
   // doxygen will not create a nested output directory for itself.
   let html = out.join("c");
   fs::create_dir_all(&html).with_context(|| format!("creating {}", html.display()))?;

   let mut input: Vec<String> = sources::SOURCES
      .iter()
      .chain(sources::SDF_SOURCES)
      .chain(sources::MACOS_SOURCES)
      .map(|source| path(root.join(source)))
      .collect();
   input.extend(
      collect(&root.join("src"), &|file| {
         Ok(file.extension().is_some_and(|ext| ext == "h"))
      })?
      .into_iter()
      .map(path),
   );
   input.sort();

   let doxyfile = fs::read_to_string(root.join("docs/c/Doxyfile.in"))
      .context("reading docs/c/Doxyfile.in")?
      .replace("@PROJECT_NAME@", "naev")
      .replace("@PROJECT_NUMBER@", env!("CARGO_PKG_VERSION"))
      .replace(
         "@PROJECT_LOGO@",
         &path(root.join("extras/logos/logo32.png")),
      )
      .replace("@OUTPUT_DIRECTORY@", &path(html))
      .replace("@INPUT@", &input.join(" "))
      .replace("@HAVE_DOT@", if on_path("dot") { "YES" } else { "NO" });

   let dest = workdir(target)?.join("Doxyfile");
   fs::write(&dest, doxyfile).with_context(|| format!("writing {}", dest.display()))?;

   // STRIP_FROM_PATH is empty, so doxygen trims the directory it was run from
   // off every path it prints.
   let mut cmd = Command::new("doxygen");
   cmd.arg(&dest).current_dir(root);
   run(cmd, Kind::C)
}

/// The name a Lua module gives itself, which is what ldoc keys its output on.
fn declared_module(text: &str) -> Option<&str> {
   text
      .lines()
      .find_map(|line| line.split_once("@module"))
      .map(|(_, name)| name.trim())
      .filter(|name| !name.is_empty())
}

/// The files below a directory that a predicate accepts. Sorted, since
/// read_dir order would otherwise reach the generated output.
fn collect(dir: &Path, keep: &dyn Fn(&Path) -> Result<bool>) -> Result<Vec<PathBuf>> {
   let mut found = Vec::new();
   walk(dir, keep, &mut found)?;
   found.sort();
   Ok(found)
}

fn walk(dir: &Path, keep: &dyn Fn(&Path) -> Result<bool>, found: &mut Vec<PathBuf>) -> Result<()> {
   for entry in fs::read_dir(dir).with_context(|| format!("reading {}", dir.display()))? {
      let entry = entry
         .with_context(|| format!("walking {}", dir.display()))?
         .path();
      if entry.is_dir() {
         walk(&entry, keep, found)?;
      } else if keep(&entry)? {
         found.push(entry);
      }
   }
   Ok(())
}

/// The Lua API reference. docmaker turns the bindings' comment blocks into
/// files ldoc reads, staged next to the Lua modules: ldoc only ever looks at
/// its first positional argument.
fn ldoc(root: &Path, target: &Path, out: &Path) -> Result<()> {
   let staged = workdir(target)?.join("ldoc");
   if staged.exists() {
      fs::remove_dir_all(&staged).context("clearing the staged ldoc inputs")?;
   }
   fs::create_dir_all(&staged).with_context(|| format!("creating {}", staged.display()))?;

   // The bindings mark themselves with @luamod wherever they live.
   let bindings = crate::data::sh(
      root.join("utils/find_nlua.sh"),
      &[root.to_path_buf()],
      "find_nlua.sh",
   )?;

   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let mut cmd = Command::new(cargo);
   cmd.arg("run")
      .arg("--quiet")
      .arg("--manifest-path")
      .arg(root.join("utils/docmaker/Cargo.toml"))
      .arg("--")
      .arg("ldoc")
      .arg("--output-dir")
      .arg(&staged)
      .args(bindings.lines().map(|found| root.join(found)));
   run(cmd, Kind::Lua)?;

   // Modules mark themselves with @module, as the bindings do with @luamod.
   // The declared name is unique where the file name is not, and it survives
   // the flattening ldoc needs.
   for module in collect(&root.join("dat/scripts"), &|file| {
      Ok(file.extension().is_some_and(|ext| ext == "lua"))
   })? {
      let text =
         fs::read_to_string(&module).with_context(|| format!("reading {}", module.display()))?;
      let Some(declared) = declared_module(&text) else {
         continue;
      };
      let dest = staged.join(format!("{declared}.lua"));
      fs::copy(&module, &dest).with_context(|| format!("copying {}", module.display()))?;
   }

   let mut cmd = Command::new("ldoc");
   cmd.arg("-c")
      .arg(root.join("docs/lua/config.ld"))
      .arg("-d")
      .arg(out.join("lua"))
      .arg("-l")
      .arg(root.join("docs/lua"))
      .arg("-s")
      .arg(root.join("docs/lua"))
      .arg(&staged);
   run(cmd, Kind::Lua)
}

/// The developer manual.
fn manual(root: &Path, out: &Path) -> Result<()> {
   let mut cmd = Command::new("mdbook");
   cmd.arg("build")
      .arg("--dest-dir")
      .arg(out.join("manual"))
      .arg(root.join("docs/manual"));
   run(cmd, Kind::Manual)
}

/// The lore website, which stages its own tree before building the site.
fn lore(root: &Path, target: &Path) -> Result<()> {
   let mut cmd = Command::new("python3");
   cmd.arg(root.join("docs/lore/loremaster.py"))
      .arg("--source-dir")
      .arg(root)
      .arg("--build-dir")
      .arg(target.join("lore"));
   run(cmd, Kind::Lore)
}

/// Where the intermediate files each tool needs are staged.
fn workdir(target: &Path) -> Result<PathBuf> {
   let dir = target.join("doc-intermediate");
   fs::create_dir_all(&dir).with_context(|| format!("creating {}", dir.display()))?;
   Ok(dir)
}

fn path(of: PathBuf) -> String {
   of.display().to_string()
}

/// Runs a documentation tool, leaving its output alone: these take a while and
/// report as they go.
fn run(mut cmd: Command, kind: Kind) -> Result<()> {
   let status = cmd
      .status()
      .with_context(|| format!("failed to run the {kind} documentation build"))?;
   if !status.success() {
      bail!("building the {kind} documentation failed with {status}");
   }
   Ok(())
}
