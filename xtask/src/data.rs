//! Generation of the derived data files.
//!
//! Each step shells out to the same python and shell scripts meson drove, with
//! the same arguments, so the output is unchanged. Order matters: the outfit
//! generators have to finish before anything that reads their results.

use std::{fs, path::Path, process::Command};

use anyhow::{Context, Result, bail};
use rayon::prelude::*;

use crate::{bioship, generated};

pub fn generate(root: &Path, out: &Path) -> Result<()> {
   let outfits = generate_outfits(root, out)?;
   println!("generated {outfits} outfit files");

   // Everything below reads the outfits above, so it cannot start earlier.
   let pedia = generate_naevpedia(root, out)?;
   println!("generated {pedia} naevpedia pages");
   generate_tech(root, out)?;
   generate_race_times(root, out)?;
   Ok(())
}

/// Markdown for every ship and outfit, including the generated outfits.
fn generate_naevpedia(root: &Path, out: &Path) -> Result<usize> {
   let mut jobs = Vec::new();

   for (kind, src_dir) in [("ships", "dat/ships"), ("outfits", "dat/outfits")] {
      let script = root.join(format!("dat/naevpedia/{kind}/{kind}.py"));
      let dest = out.join(format!("naevpedia/{kind}"));
      fs::create_dir_all(&dest)
         .with_context(|| format!("creating the naevpedia {kind} directory"))?;
      for xml in find_xml(root, &root.join(src_dir))? {
         jobs.push((script.clone(), xml, dest.clone()));
      }
   }

   // The derived outfits only exist in the output tree, so they are picked up
   // from there rather than from the source.
   let outfit_script = root.join("dat/naevpedia/outfits/outfits.py");
   let outfit_dest = out.join("naevpedia/outfits");
   for derived in generated::DERIVED {
      jobs.push((
         outfit_script.clone(),
         out.join("outfits/generated").join(derived.output),
         outfit_dest.clone(),
      ));
   }

   jobs.par_iter().try_for_each(|(script, xml, dest)| {
      let stem = xml
         .file_stem()
         .context("every input should have a file name")?;
      let mut md = dest.join(stem);
      md.set_extension("md");
      let mut cmd = Command::new("python3");
      cmd.arg(script).arg(xml).arg("-o").arg(&md);
      run(cmd, &xml.display().to_string())
   })?;

   Ok(jobs.len())
}

/// The two tech lists, which are concatenations of everything not excluded.
fn generate_tech(root: &Path, out: &Path) -> Result<()> {
   let dest = out.join("tech");
   fs::create_dir_all(&dest).context("creating the tech directory")?;

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
      let mut inputs: Vec<_> = listed.lines().map(Into::into).collect::<Vec<String>>();
      // The tech lists cover the derived outfits too.
      if output == "all_outfits.xml" {
         for derived in generated::DERIVED {
            inputs.push(
               out.join("outfits/generated")
                  .join(derived.output)
                  .display()
                  .to_string(),
            );
         }
      }

      let mut cmd = Command::new("bash");
      cmd.arg(root.join(generate))
         .arg(dest.join(output))
         .args(inputs);
      run(cmd, output)?;
   }
   Ok(())
}

/// Race times, derived from the ships and outfits a race can use.
fn generate_race_times(root: &Path, out: &Path) -> Result<()> {
   let dest = out.join("missions/neutral/race");
   fs::create_dir_all(&dest).context("creating the race mission directory")?;
   let mut cmd = Command::new("python3");
   cmd.arg(root.join("dat/missions/neutral/race/gen_times.py"))
      .arg("-q")
      .arg(dest.join("times_qex.lua"));
   run(cmd, "times_qex.lua")
}

/// The tracked XML under a directory, as the old find_xml.sh reported it.
fn find_xml(root: &Path, dir: &Path) -> Result<Vec<std::path::PathBuf>> {
   let listed = sh(
      root.join("utils/find_xml.sh"),
      &[dir.to_path_buf()],
      "find_xml.sh",
   )?;
   Ok(listed.lines().map(|rel| dir.join(rel)).collect())
}

/// Runs a helper script and hands back its stdout.
fn sh(script: std::path::PathBuf, args: &[std::path::PathBuf], what: &str) -> Result<String> {
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

/// Bioship families and derived outfits. Everything downstream reads these, so
/// they come first.
fn generate_outfits(root: &Path, out: &Path) -> Result<usize> {
   let bio_dir = out.join("outfits/bioship");
   let derived_dir = out.join("outfits/generated");
   fs::create_dir_all(&bio_dir).context("creating the bioship output directory")?;
   fs::create_dir_all(&derived_dir).context("creating the derived outfit directory")?;

   let script = root.join("dat/outfits/bioship/generate.py");
   let templates = root.join("dat/outfits/bioship/templates");

   let bio: usize = bioship::FAMILIES
      .par_iter()
      .map(|family| {
         let template = templates.join(format!("{}.xml.template", family.template));
         let outputs: Vec<_> = family.outputs.iter().map(|o| bio_dir.join(o)).collect();
         // generate.py switches to its per-family mode when it sees -o, which
         // is how meson drove it.
         let mut cmd = Command::new("python3");
         cmd.arg(&script).arg(&template).arg("-o").args(&outputs);
         run(cmd, &format!("bioship family {}", family.template))?;
         Ok(family.outputs.len())
      })
      .collect::<Result<Vec<_>>>()?
      .iter()
      .sum();

   let derived_scripts = root.join("dat/outfits/generated");
   let outfit_src = root.join("dat/outfits");
   generated::DERIVED.par_iter().try_for_each(|derived| {
      let mut cmd = Command::new("python3");
      cmd.arg(derived_scripts.join(derived.script))
         .arg(outfit_src.join(derived.input))
         .arg(derived_dir.join(derived.output));
      run(cmd, derived.output)
   })?;

   Ok(bio + generated::DERIVED.len())
}

/// Runs a generator, turning a non-zero exit into an error that names it.
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
