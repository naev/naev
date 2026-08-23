//! Incremental execution of the data generators.
//!
//! Each generator states what it reads and what it writes, which is enough to
//! skip the ones whose inputs have not moved. Nothing here discovers
//! dependencies while it runs, so a rule is stale exactly when an output is
//! missing or older than an input.

use std::{
   path::{Path, PathBuf},
   process::Command,
   time::SystemTime,
};

use anyhow::{Context, Result, bail};
use rayon::prelude::*;

/// One generator invocation, with the files it reads and writes.
pub struct Rule {
   what: String,
   command: Command,
   inputs: Vec<PathBuf>,
   outputs: Vec<PathBuf>,
}

impl Rule {
   pub fn new(what: impl Into<String>, command: Command) -> Self {
      Self {
         what: what.into(),
         command,
         inputs: Vec::new(),
         outputs: Vec::new(),
      }
   }

   pub fn reads(mut self, paths: impl IntoIterator<Item = PathBuf>) -> Self {
      self.inputs.extend(paths);
      self
   }

   pub fn writes(mut self, paths: impl IntoIterator<Item = PathBuf>) -> Self {
      self.outputs.extend(paths);
      self
   }

   /// Whether an output is missing or older than an input, treating `floor` as
   /// an input every rule has.
   fn stale(&self, floor: SystemTime) -> Result<bool> {
      let mut newest = floor;
      for input in &self.inputs {
         match modified(input)? {
            // Let the generator report a missing input rather than guessing.
            None => return Ok(true),
            Some(at) => newest = newest.max(at),
         }
      }

      for output in &self.outputs {
         match modified(output)? {
            None => return Ok(true),
            Some(at) if at < newest => return Ok(true),
            Some(_) => {}
         }
      }
      Ok(false)
   }

   fn execute(mut self) -> Result<()> {
      let output = self
         .command
         .output()
         .with_context(|| format!("failed to run the generator for {}", self.what))?;
      if !output.status.success() {
         bail!(
            "generating {} failed with {}\n{}",
            self.what,
            output.status,
            String::from_utf8_lossy(&output.stderr)
         );
      }
      Ok(())
   }
}

/// Runs the rules that are out of date and reports how many files they wrote.
pub fn run(rules: Vec<Rule>, floor: SystemTime) -> Result<usize> {
   let stale = rules
      .par_iter()
      .map(|rule| rule.stale(floor))
      .collect::<Result<Vec<_>>>()?;

   let due: Vec<Rule> = rules
      .into_iter()
      .zip(stale)
      .filter_map(|(rule, stale)| stale.then_some(rule))
      .collect();

   let written = due.iter().map(|rule| rule.outputs.len()).sum();
   due.into_par_iter().try_for_each(Rule::execute)?;
   Ok(written)
}

/// The point every rule counts as an input. The tables driving the generators
/// are compiled into this binary, so rebuilding it invalidates the tree.
pub fn floor() -> Result<SystemTime> {
   let exe = std::env::current_exe().context("locating the running xtask binary")?;
   modified(&exe)?.context("the running binary should still be on disk")
}

/// The modification time of a path, or None where there is no such file.
fn modified(path: &Path) -> Result<Option<SystemTime>> {
   match std::fs::metadata(path) {
      Ok(meta) => Ok(Some(meta.modified().with_context(|| {
         format!("reading the timestamp of {}", path.display())
      })?)),
      Err(err) if err.kind() == std::io::ErrorKind::NotFound => Ok(None),
      Err(err) => Err(err).with_context(|| format!("reading {}", path.display())),
   }
}
