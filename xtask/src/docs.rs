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
   // doxygen sweeps the whole engine and lore needs a ruby bundle, so both are opt-in.
   fn by_default(self) -> bool {
      match self {
         Kind::Lua | Kind::Manual => true,
         Kind::C | Kind::Lore => false,
      }
   }

   fn install_hint(self) -> Option<&'static str> {
      match self {
         Kind::Manual => Some("cargo install mdbook"),
         Kind::C | Kind::Lua | Kind::Lore => None,
      }
   }

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

pub fn defaults_help() -> String {
   let (on, off): (Vec<_>, Vec<_>) = Kind::value_variants()
      .iter()
      .partition(|kind| kind.by_default());
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

pub fn build(root: &Path, target: &Path, kinds: &[Kind], output: Option<PathBuf>) -> Result<()> {
   // doxygen runs from the repo root, so a relative path is resolved against the caller first.
   let out = std::path::absolute(output.unwrap_or_else(|| target.join("doc")))
      .context("resolving the output directory")?;
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

      // doxygen and mdbook are quiet for a long time on a cold run.
      println!("building the {kind} documentation");
      match kind {
         Kind::C => cdoc(root, target, &out),
         Kind::Lua => ldoc(root, target, &out),
         Kind::Manual => manual(root, &out),
         Kind::Lore => lore(root, target, &out),
      }?;
      println!("built the {kind} documentation");
   }
   Ok(())
}

fn cdoc(root: &Path, target: &Path, out: &Path) -> Result<()> {
   // doxygen won't create a nested output directory itself.
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

   // With STRIP_FROM_PATH empty, doxygen strips its working directory from paths.
   let mut cmd = Command::new("doxygen");
   cmd.arg(&dest).current_dir(root);
   run(cmd, Kind::C)
}

fn declared_module(text: &str) -> Option<&str> {
   text
      .lines()
      .find_map(|line| line.split_once("@module"))
      .map(|(_, name)| name.trim())
      .filter(|name| !name.is_empty())
}

// Sorted so read_dir order can't leak into the generated output.
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

// ldoc only reads its first positional argument, so all its inputs are staged together.
fn ldoc(root: &Path, target: &Path, out: &Path) -> Result<()> {
   let staged = workdir(target)?.join("ldoc");
   if staged.exists() {
      fs::remove_dir_all(&staged).context("clearing the staged ldoc inputs")?;
   }
   fs::create_dir_all(&staged).with_context(|| format!("creating {}", staged.display()))?;

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

   // Staged under the @module name, which is unique where file names aren't.
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

fn manual(root: &Path, out: &Path) -> Result<()> {
   let mut cmd = Command::new("mdbook");
   cmd.arg("build")
      .arg("--dest-dir")
      .arg(out.join("manual"))
      .arg(root.join("docs/manual"));
   run(cmd, Kind::Manual)
}

fn lore(root: &Path, target: &Path, out: &Path) -> Result<()> {
   let work = workdir(target)?;
   let mut cmd = Command::new("python3");
   cmd.arg(root.join("docs/lore/loremaster.py"))
      .arg("--source-dir")
      .arg(root)
      .arg("--build-dir")
      .arg(&work);
   run(cmd, Kind::Lore)?;

   // loremaster always writes the site to docs/lore under its build dir.
   let site = work.join("docs/lore");
   let dest = out.join("lore");
   if dest.exists() {
      fs::remove_dir_all(&dest).with_context(|| format!("clearing {}", dest.display()))?;
   }
   for file in collect(&site, &|_| Ok(true))? {
      let to = dest.join(file.strip_prefix(&site)?);
      let parent = to.parent().context("every copied file has a directory")?;
      fs::create_dir_all(parent).with_context(|| format!("creating {}", parent.display()))?;
      fs::copy(&file, &to).with_context(|| format!("copying {}", file.display()))?;
   }
   Ok(())
}

fn workdir(target: &Path) -> Result<PathBuf> {
   let dir = target.join("doc-intermediate");
   fs::create_dir_all(&dir).with_context(|| format!("creating {}", dir.display()))?;
   Ok(dir)
}

fn path(of: PathBuf) -> String {
   of.display().to_string()
}

fn run(mut cmd: Command, kind: Kind) -> Result<()> {
   let status = cmd
      .status()
      .with_context(|| format!("failed to run the {kind} documentation build"))?;
   if !status.success() {
      bail!("building the {kind} documentation failed with {status}");
   }
   Ok(())
}
