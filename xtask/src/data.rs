//! Generation of the derived data files.
//!
//! Each step shells out to the same python and shell scripts meson drove, with
//! the same arguments, so the output is unchanged. They are expressed as rules
//! naming their inputs and outputs, which is what lets an edit rebuild only
//! what it affects.

use std::{
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};

use crate::{
   bioship, generated,
   rule::{self, Rule},
};

pub fn generate(root: &Path, out: &Path) -> Result<()> {
   let floor = rule::floor()?;

   let outfits = rule::run(outfit_rules(root, out)?, floor)?;
   if outfits > 0 {
      println!("generated {outfits} outfit file{}", plural(outfits));
   }

   // Everything below reads the outfits above, so it cannot start earlier.
   let mut rules = naevpedia_rules(root, out)?;
   rules.extend(tech_rules(root, out)?);
   rules.extend(translation_rules(root, out)?);
   rules.push(race_times_rule(root, out)?);
   rules.push(authors_rule(root, out));
   rules.push(gettext_stats_rule(root, out)?);

   let rest = rule::run(rules, floor)?;
   if rest > 0 {
      println!("generated {rest} data file{}", plural(rest));
   }
   Ok(())
}

fn plural(count: usize) -> &'static str {
   if count == 1 { "" } else { "s" }
}

/// Bioship families and derived outfits. Everything downstream reads these, so
/// they come first.
fn outfit_rules(root: &Path, out: &Path) -> Result<Vec<Rule>> {
   let bio_dir = out.join("outfits/bioship");
   let derived_dir = out.join("outfits/generated");
   fs::create_dir_all(&bio_dir).context("creating the bioship output directory")?;
   fs::create_dir_all(&derived_dir).context("creating the derived outfit directory")?;

   let script = root.join("dat/outfits/bioship/generate.py");
   let templates = root.join("dat/outfits/bioship/templates");

   let mut rules: Vec<Rule> = bioship::FAMILIES
      .iter()
      .map(|family| {
         let template = templates.join(format!("{}.xml.template", family.template));
         let outputs: Vec<PathBuf> = family.outputs.iter().map(|o| bio_dir.join(o)).collect();
         // generate.py switches to its per-family mode when it sees -o, which
         // is how meson drove it.
         let mut cmd = Command::new("python3");
         cmd.arg(&script).arg(&template).arg("-o").args(&outputs);
         Rule::new(format!("bioship family {}", family.template), cmd)
            .reads([script.clone(), template])
            .writes(outputs)
      })
      .collect();

   let derived_scripts = root.join("dat/outfits/generated");
   let outfit_src = root.join("dat/outfits");
   rules.extend(generated::DERIVED.iter().map(|derived| {
      let script = derived_scripts.join(derived.script);
      let input = outfit_src.join(derived.input);
      let output = derived_dir.join(derived.output);
      let mut cmd = Command::new("python3");
      cmd.arg(&script).arg(&input).arg(&output);
      Rule::new(derived.output, cmd)
         .reads([script, input])
         .writes([output])
   }));

   Ok(rules)
}

/// Markdown for every ship and outfit, including the generated outfits.
fn naevpedia_rules(root: &Path, out: &Path) -> Result<Vec<Rule>> {
   let mut rules = Vec::new();

   for (kind, src_dir) in [("ships", "dat/ships"), ("outfits", "dat/outfits")] {
      let script = root.join(format!("dat/naevpedia/{kind}/{kind}.py"));
      let dest = out.join(format!("naevpedia/{kind}"));
      fs::create_dir_all(&dest)
         .with_context(|| format!("creating the naevpedia {kind} directory"))?;
      for xml in find_xml(root, &root.join(src_dir))? {
         rules.push(page_rule(&script, &xml, &dest)?);
      }
   }

   // The derived outfits only exist in the output tree, so they are picked up
   // from there rather than from the source.
   let script = root.join("dat/naevpedia/outfits/outfits.py");
   let dest = out.join("naevpedia/outfits");
   for derived in generated::DERIVED {
      let xml = out.join("outfits/generated").join(derived.output);
      rules.push(page_rule(&script, &xml, &dest)?);
   }

   Ok(rules)
}

fn page_rule(script: &Path, xml: &Path, dest: &Path) -> Result<Rule> {
   let stem = xml
      .file_stem()
      .context("every input should have a file name")?;
   let mut md = dest.join(stem);
   md.set_extension("md");

   let mut cmd = Command::new("python3");
   cmd.arg(script).arg(xml).arg("-o").arg(&md);
   Ok(Rule::new(xml.display().to_string(), cmd)
      .reads([script.to_path_buf(), xml.to_path_buf()])
      .writes([md]))
}

/// The two tech lists, which are concatenations of everything not excluded.
fn tech_rules(root: &Path, out: &Path) -> Result<Vec<Rule>> {
   let dest = out.join("tech");
   fs::create_dir_all(&dest).context("creating the tech directory")?;

   let mut rules = Vec::new();
   for (discover, generate, output, dir) in [
      (
         "dat/tech/all_ships_dep.sh",
         "dat/tech/gen_all_ships_tech.sh",
         "all_ships.xml",
         "dat/ships",
      ),
      (
         "dat/tech/all_outfits_dep.sh",
         "dat/tech/gen_all_outfits_tech.sh",
         "all_outfits.xml",
         "dat/outfits",
      ),
   ] {
      let listed = sh(root.join(discover), &[root.join(dir)], discover)?;
      let mut inputs: Vec<PathBuf> = listed.lines().map(PathBuf::from).collect();
      // The tech lists cover the derived outfits too.
      if output == "all_outfits.xml" {
         inputs.extend(
            generated::DERIVED
               .iter()
               .map(|derived| out.join("outfits/generated").join(derived.output)),
         );
      }

      let script = root.join(generate);
      let target = dest.join(output);
      let mut cmd = Command::new("bash");
      cmd.arg(&script).arg(&target).args(&inputs);
      rules.push(
         Rule::new(output, cmd)
            .reads(inputs)
            .reads([script])
            .writes([target]),
      );
   }
   Ok(rules)
}

/// Compiles each translation the game ships. The catalogues themselves are
/// maintained separately; this only turns the checked-in .po files into the
/// binary form the runtime loads.
fn translation_rules(root: &Path, out: &Path) -> Result<Vec<Rule>> {
   crate::i18n::languages(root)?
      .into_iter()
      .map(|lang| {
         let dest = out.join("gettext").join(&lang).join("LC_MESSAGES");
         fs::create_dir_all(&dest)
            .with_context(|| format!("creating the message directory for {lang}"))?;

         let po = root.join("po").join(format!("{lang}.po"));
         let mo = dest.join("naev.mo");
         let mut cmd = Command::new("msgfmt");
         cmd.arg(&po).arg("-o").arg(&mo);
         Ok(Rule::new(lang, cmd).reads([po]).writes([mo]))
      })
      .collect()
}

/// Race times, derived from the ships and outfits a race can use.
fn race_times_rule(root: &Path, out: &Path) -> Result<Rule> {
   let dest = out.join("missions/neutral/race");
   fs::create_dir_all(&dest).context("creating the race mission directory")?;

   let script = root.join("dat/missions/neutral/race/gen_times.py");
   let target = dest.join("times_qex.lua");
   let mut cmd = Command::new("python3");
   cmd.arg(&script).arg("-q").arg(&target);
   Ok(Rule::new("times_qex.lua", cmd)
      .reads([script])
      .writes([target]))
}

/// The credits, merging the tracked preamble with everyone named in the asset
/// licence manifests. It has to outrank the preamble it was built from, which
/// is why the generated tree mounts ahead of dat/.
fn authors_rule(root: &Path, out: &Path) -> Rule {
   let script = root.join("utils/build/gen_authors.py");
   let preamble = root.join("dat/AUTHORS");
   let artwork = root.join("assets/gfx/ARTWORK_LICENSE.yaml");
   let sound = root.join("assets/snd/SOUND_LICENSE.yaml");
   let target = out.join("AUTHORS");

   let mut cmd = Command::new("python3");
   cmd.arg(&script)
      .arg("--output")
      .arg(&target)
      .arg("--preamble")
      .arg(&preamble)
      .arg(&artwork)
      .arg(&sound);
   Rule::new("AUTHORS", cmd)
      .reads([script, preamble, artwork, sound])
      .writes([target])
}

/// The translatable string count the credits screen reports.
fn gettext_stats_rule(root: &Path, out: &Path) -> Result<Rule> {
   let dest = out.join("gettext_stats");
   fs::create_dir_all(&dest).context("creating the gettext stats directory")?;

   let script = root.join("utils/build/gen_gettext_stats.py");
   let template = root.join("po/naev.pot");
   let target = dest.join("naev.txt");
   let mut cmd = Command::new("python3");
   cmd.arg(&script).arg("--output").arg(&target).arg(&template);
   Ok(Rule::new("gettext_stats/naev.txt", cmd)
      .reads([script, template])
      .writes([target]))
}

/// The tracked XML under a directory, as the old find_xml.sh reported it.
fn find_xml(root: &Path, dir: &Path) -> Result<Vec<PathBuf>> {
   let listed = sh(
      root.join("utils/find_xml.sh"),
      &[dir.to_path_buf()],
      "find_xml.sh",
   )?;
   Ok(listed.lines().map(|rel| dir.join(rel)).collect())
}

/// Runs a helper script and hands back its stdout.
pub fn sh(script: PathBuf, args: &[PathBuf], what: &str) -> Result<String> {
   let output = Command::new("bash")
      .arg(script)
      .args(args)
      .output()
      .with_context(|| format!("failed to run {what}"))?;
   if !output.status.success() {
      bail!(
         "{what} failed with {}\n{}",
         output.status,
         String::from_utf8_lossy(&output.stderr)
      );
   }
   String::from_utf8(output.stdout).with_context(|| format!("{what} produced invalid UTF-8"))
}
