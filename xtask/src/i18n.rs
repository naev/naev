use std::{path::Path, process::Command};

use anyhow::{Context, Result, bail};
use rayon::prelude::*;

const PACKAGE: &str = "naev";

const KEYWORDS: &[&str] = &["_", "N_", "n_:1,2", "p_:1c,2", "gettext.gettext_noop"];

pub fn pot(root: &Path, data_dir: &Path, metadata: &crate::metadata::Metadata) -> Result<()> {
   // Some strings only exist in the generated naevpedia and outfits.
   crate::data::generate(root, data_dir)?;
   update_potfiles(root, data_dir)?;
   extract_pot(root, metadata)?;
   println!("regenerated po/naev.pot");
   Ok(())
}

pub fn update_po(root: &Path, data_dir: &Path, metadata: &crate::metadata::Metadata) -> Result<()> {
   pot(root, data_dir, metadata)?;

   let langs = languages(root)?;
   let progress = crate::progress::bar(langs.len(), "merging");
   langs.par_iter().try_for_each(|lang| {
      let po = root.join("po").join(format!("{lang}.po"));
      let mut cmd = Command::new("msgmerge");
      cmd.arg("--quiet")
         .arg("--update")
         .arg("--backup=none")
         .arg(&po)
         .arg(root.join("po/naev.pot"));
      let result = run(cmd, lang);
      progress.inc(1);
      result
   })?;
   progress.finish_and_clear();

   println!("merged {} catalogues", langs.len());
   Ok(())
}

pub fn languages(root: &Path) -> Result<Vec<String>> {
   let linguas = std::fs::read_to_string(root.join("po/LINGUAS")).context("reading po/LINGUAS")?;
   Ok(linguas
      .lines()
      .map(str::trim)
      .filter(|line| !line.is_empty() && !line.starts_with('#'))
      .map(str::to_owned)
      .collect())
}

/// update-po.sh also writes intermediate catalogues for the formats xgettext can't read.
fn update_potfiles(root: &Path, data_dir: &Path) -> Result<()> {
   let mut cmd = Command::new("bash");
   cmd.arg(root.join("utils/update-po.sh"))
      .arg(root)
      .arg(data_dir);
   run(cmd, "POTFILES.in")
}

fn extract_pot(root: &Path, metadata: &crate::metadata::Metadata) -> Result<()> {
   let mut cmd = Command::new("xgettext");
   // GETTEXTDATADIRS adds po/its to the ITS rules xgettext searches.
   cmd.env("GETTEXTDATADIRS", root.join("po"))
      .arg("--files-from")
      .arg(root.join("po/POTFILES.in"))
      .arg("--directory")
      .arg(root)
      .arg("--from-code=UTF-8")
      .arg("--add-comments")
      // Line numbers would churn every catalogue on each edit, and they made up most of po/.
      .arg("--add-location=file")
      .arg("--package-name")
      .arg(PACKAGE)
      .arg("--package-version")
      .arg(env!("CARGO_PKG_VERSION"))
      .arg("--msgid-bugs-address")
      .arg(format!("{}/issues", metadata.repository))
      .arg("--copyright-holder")
      .arg(&metadata.copyright_holder)
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

#[cfg(test)]
mod tests {
   use std::{fs, path::Path};

   // gettext can't carry format macros like PRIu64 through translation, so strings use %.0f instead.
   #[test]
   fn template_has_no_sysdep_strings() {
      let pot = Path::new(env!("CARGO_MANIFEST_DIR")).join("../po/naev.pot");
      let template = fs::read_to_string(&pot).expect("po/naev.pot is in the repository");
      let found: Vec<&str> = template
         .lines()
         .filter(|line| line.contains("%<PRI"))
         .collect();
      assert!(
         found.is_empty(),
         "translatable strings use PRI macros: {found:?}"
      );
   }
}
