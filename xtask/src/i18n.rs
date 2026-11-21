use std::path::Path;

use anyhow::{Context, Result};

pub fn languages(root: &Path) -> Result<Vec<String>> {
   let linguas = std::fs::read_to_string(root.join("po/LINGUAS")).context("reading po/LINGUAS")?;
   Ok(linguas
      .lines()
      .map(str::trim)
      .filter(|line| !line.is_empty() && !line.starts_with('#'))
      .map(str::to_owned)
      .collect())
}
