use std::{fs, path::Path, process::Command};

use anyhow::{Context, Result};

pub fn current(root: &Path) -> Result<String> {
   // xtask inherits version.workspace, so this is the workspace version.
   let base = env!("CARGO_PKG_VERSION").to_string();

   // A source tarball has no git but ships dat/VERSION.
   if git(root, &["rev-parse", "--is-inside-work-tree"]).as_deref() != Some("true") {
      return Ok(match fs::read_to_string(root.join("dat/VERSION")) {
         Ok(shipped) => shipped.trim().to_string(),
         Err(_) => format!("{base}+dev"),
      });
   }
   if tagged(root) && !dirty(root) {
      return Ok(base);
   }

   let mut version = base;
   let date = git(
      root,
      &["log", "-1", "--date=format:%y-%m-%d-%H", "--format=%ad"],
   );
   match (date, git(root, &["rev-parse", "--short", "HEAD"])) {
      (Some(date), Some(sha)) => version.push_str(&format!("+{date}.g{sha}")),
      (None, Some(sha)) => version.push_str(&format!("+g{sha}")),
      _ => (),
   }
   if dirty(root) {
      version.push_str(".dirty");
   }
   Ok(version)
}

/// The engine only treats a directory as ndata once it holds VERSION.
pub fn ensure(root: &Path, dir: &Path) -> Result<()> {
   let version = current(root)?;
   let file = dir.join("VERSION");
   if fs::read_to_string(&file).is_ok_and(|current| current.trim() == version) {
      return Ok(());
   }
   fs::create_dir_all(dir).with_context(|| format!("creating {}", dir.display()))?;
   fs::write(&file, &version).with_context(|| format!("writing {}", file.display()))
}

/// The file a source tarball records its commit's time in, for want of git history.
pub const COMMIT_TIME_FILE: &str = "SOURCE_DATE_EPOCH";

/// When the source's commit was made, in seconds since the epoch: from the file a source tarball
/// carries, or else from git. None when neither has it.
pub fn commit_time(root: &Path) -> Option<String> {
   let shipped = fs::read_to_string(root.join(COMMIT_TIME_FILE))
      .ok()
      .map(|text| text.trim().to_string())
      .filter(|text| !text.is_empty() && text.chars().all(|c| c.is_ascii_digit()));
   shipped.or_else(|| git(root, &["log", "-1", "--format=%ct"]))
}

fn git(root: &Path, args: &[&str]) -> Option<String> {
   let out = Command::new("git")
      .args(args)
      .current_dir(root)
      .output()
      .ok()?;
   if !out.status.success() {
      return None;
   }
   let text = String::from_utf8_lossy(&out.stdout)
      .trim()
      .trim_matches('"')
      .to_string();
   (!text.is_empty()).then_some(text)
}

fn tagged(root: &Path) -> bool {
   git(root, &["describe", "--exact-match", "--tags", "HEAD"]).is_some()
}

fn dirty(root: &Path) -> bool {
   git(root, &["status", "--porcelain"]).is_some()
}
