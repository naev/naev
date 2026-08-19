//! Translation catalogue maintenance.
//!
//! These rewrite files in `po/`, which are tracked, so they are run by hand
//! when strings change rather than as part of a build. Compiling the
//! catalogues for the game is a separate step; see `data`.

use std::{path::Path, process::Command};

use anyhow::{Context, Result, bail};
use rayon::prelude::*;

const PACKAGE: &str = "naev";
const BUGS_ADDRESS: &str = "https://codeberg.org/naev/naev/issues";
const COPYRIGHT_HOLDER: &str = "Naev Dev Team";

/// Functions xgettext should treat as translation calls.
const KEYWORDS: &[&str] = &["_", "N_", "n_:1,2", "p_:1c,2", "gettext.gettext_noop"];

/// Refreshes `po/POTFILES.in` and regenerates `po/naev.pot`.
///
/// `data_dir` has to hold the generated data, since some translatable strings
/// only exist in the naevpedia pages and outfits that `data` produces.
pub fn pot(root: &Path, data_dir: &Path) -> Result<()> {
   update_potfiles(root, data_dir)?;
   extract_pot(root)?;
   println!("regenerated po/naev.pot");
   Ok(())
}

/// Brings every catalogue up to date with the current template.
pub fn update_po(root: &Path, data_dir: &Path) -> Result<()> {
   pot(root, data_dir)?;

   let langs = languages(root)?;
   langs.par_iter().try_for_each(|lang| {
      let po = root.join("po").join(format!("{lang}.po"));
      let mut cmd = Command::new("msgmerge");
      cmd.arg("--quiet")
         .arg("--update")
         .arg("--backup=none")
         .arg(&po)
         .arg(root.join("po/naev.pot"));
      run(cmd, lang)
   })?;

   println!("merged {} catalogues", langs.len());
   Ok(())
}

/// The languages the game ships, in the order po/LINGUAS lists them.
pub fn languages(root: &Path) -> Result<Vec<String>> {
   let linguas = std::fs::read_to_string(root.join("po/LINGUAS")).context("reading po/LINGUAS")?;
   Ok(linguas
      .lines()
      .map(str::trim)
      .filter(|line| !line.is_empty() && !line.starts_with('#'))
      .map(str::to_owned)
      .collect())
}

/// Rebuilds the list of files holding translatable text, along with the
/// intermediate catalogues for the formats xgettext cannot read directly.
fn update_potfiles(root: &Path, data_dir: &Path) -> Result<()> {
   let mut cmd = Command::new("bash");
   cmd.arg(root.join("utils/update-po.sh"))
      .arg(root)
      .arg(data_dir);
   run(cmd, "POTFILES.in")
}

fn extract_pot(root: &Path) -> Result<()> {
   let mut cmd = Command::new("xgettext");
   // The XML formats are described by ITS rules in po/its, which xgettext
   // only finds through this variable. Without it the files are read as C.
   cmd.env("GETTEXTDATADIR", root.join("po"))
      .arg("--files-from")
      .arg(root.join("po/POTFILES.in"))
      .arg("--directory")
      .arg(root)
      .arg("--from-code=UTF-8")
      .arg("--add-comments")
      .arg("--package-name")
      .arg(PACKAGE)
      .arg("--package-version")
      .arg(env!("CARGO_PKG_VERSION"))
      .arg("--msgid-bugs-address")
      .arg(BUGS_ADDRESS)
      .arg("--copyright-holder")
      .arg(COPYRIGHT_HOLDER)
      .arg("-o")
      .arg(root.join("po/naev.pot"));
   for keyword in KEYWORDS {
      cmd.arg(format!("--keyword={keyword}"));
   }
   run(cmd, "naev.pot")
}

fn run(mut cmd: Command, what: &str) -> Result<()> {
   let output = cmd
      .output()
      .with_context(|| format!("failed to run the generator for {what}"))?;
   if !output.status.success() {
      bail!(
         "generating {what} failed with {}\n{}",
         output.status,
         String::from_utf8_lossy(&output.stderr)
      );
   }
   Ok(())
}
