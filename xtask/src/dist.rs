use std::{
   fs,
   path::{Path, PathBuf},
   process::{Command, Stdio},
};

use anyhow::{Context, Result, bail};

pub fn dist(root: &Path, target: &Path, output: Option<PathBuf>) -> Result<()> {
   let version = crate::version::current(root)?;
   let name = format!("naev-{version}");
   let out_dir = output.unwrap_or_else(|| target.join("dist"));
   fs::create_dir_all(&out_dir).with_context(|| format!("creating {}", out_dir.display()))?;

   // Staged on disk so the submodules can land at their own paths.
   let stage = target.join("dist-staging");
   if stage.exists() {
      fs::remove_dir_all(&stage).with_context(|| format!("clearing {}", stage.display()))?;
   }
   let tree = stage.join(&name);
   fs::create_dir_all(&tree).with_context(|| format!("creating {}", tree.display()))?;

   let progress = crate::progress::spinner("archiving");
   archive_tree(root, &tree)?;

   // dat/VERSION is ignored by git, so the tarball carries its own copy.
   crate::version::ensure(root, &tree.join("dat"))?;

   let tarball = out_dir.join(format!("{name}.tar.xz"));
   progress.set_message("compressing");
   crate::archive::tar_xz(&tarball, &stage, &[&name])?;
   progress.finish_and_clear();
   fs::remove_dir_all(&stage).with_context(|| format!("clearing {}", stage.display()))?;
   crate::archive::checksum(&tarball)?;

   println!("wrote {}", tarball.display());
   Ok(())
}

fn submodules(root: &Path) -> Result<Vec<PathBuf>> {
   let out = Command::new("git")
      .args([
         "config",
         "--file",
         ".gitmodules",
         "--get-regexp",
         r"^submodule\..*\.path$",
      ])
      .current_dir(root)
      .output()
      .context("failed to read .gitmodules")?;
   // No submodules at all is a valid tree, and git reports it as a failure.
   if !out.status.success() {
      return Ok(Vec::new());
   }
   Ok(String::from_utf8_lossy(&out.stdout)
      .lines()
      .filter_map(|line| line.split_once(' '))
      .map(|(_, path)| PathBuf::from(path))
      .collect())
}

/// Submodules nest, as utils/shlinters carries its own.
fn archive_tree(repo: &Path, into: &Path) -> Result<()> {
   archive_into(repo, into)?;
   for path in submodules(repo)? {
      let sub = repo.join(&path);
      // Some submodules, like utils/shlinters/smd, are normally not checked out.
      if !sub.join(".git").exists() {
         eprintln!("dist: skipping {}, not checked out", sub.display());
         continue;
      }
      archive_tree(&sub, &into.join(&path))?;
   }
   Ok(())
}

fn archive_into(repo: &Path, into: &Path) -> Result<()> {
   if !repo.join(".git").exists() {
      bail!(
         "{} is not checked out; run git submodule update --init",
         repo.display()
      );
   }
   fs::create_dir_all(into).with_context(|| format!("creating {}", into.display()))?;

   // LFS pointers only resolve when their objects are present locally, as the CI checkout arranges.
   let mut archive = Command::new("git")
      .args(["archive", "--format=tar", "HEAD"])
      .current_dir(repo)
      .stdout(Stdio::piped())
      .spawn()
      .with_context(|| format!("failed to run git archive in {}", repo.display()))?;
   let tar = archive
      .stdout
      .take()
      .context("git archive closed its output")?;
   crate::archive::unpack(tar, into)?;
   let archived = archive.wait().context("waiting for git archive")?;
   if !archived.success() {
      bail!("git archive failed in {} with {archived}", repo.display());
   }
   Ok(())
}
