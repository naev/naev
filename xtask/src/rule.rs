use std::{
   collections::{HashMap, HashSet},
   fs,
   path::{Path, PathBuf},
   process::Command,
   time::SystemTime,
};

use anyhow::{Context, Result, bail};
use rayon::prelude::*;
use serde::{Deserialize, Serialize};

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

   // The generator tables compiled into xtask surface in the command, inputs and outputs.
   fn signature(&self) -> String {
      format!("{:?} {:?} {:?}", self.command, self.inputs, self.outputs)
   }

   fn stale(&self, state: &State) -> Result<bool> {
      if state
         .read(self)?
         .is_none_or(|record| record.signature != self.signature())
      {
         return Ok(true);
      }

      let mut newest = SystemTime::UNIX_EPOCH;
      for input in &self.inputs {
         match modified(input)? {
            // Run it so the generator reports the missing input.
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

   fn execute(mut self, state: &State) -> Result<()> {
      // A run that fails or gets interrupted leaves no record, so it's redone.
      state.forget(&self)?;
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
      state.remember(&self)
   }
}

/// What a rule last succeeded with.
#[derive(Serialize, Deserialize)]
struct Record {
   signature: String,
   outputs: Vec<PathBuf>,
}

/// Records kept beside the data dir, so they're neither installed nor mounted.
pub struct State {
   data_dir: PathBuf,
   dir: PathBuf,
}

impl State {
   pub fn beside(data_dir: &Path) -> Result<Self> {
      let mut name = data_dir
         .file_name()
         .context("the data dir needs a name of its own")?
         .to_os_string();
      name.push(".state");
      let dir = data_dir.with_file_name(name);
      fs::create_dir_all(&dir).with_context(|| format!("creating {}", dir.display()))?;
      Ok(Self {
         data_dir: data_dir.to_path_buf(),
         dir,
      })
   }

   fn path(&self, rule: &Rule) -> Result<PathBuf> {
      let output = rule
         .outputs
         .first()
         .with_context(|| format!("{} writes nothing", rule.what))?;
      let rel = output.strip_prefix(&self.data_dir).unwrap_or(output);
      let name = rel
         .to_string_lossy()
         .replace('%', "%25")
         .replace(['/', '\\'], "%2F");
      Ok(self.dir.join(name))
   }

   fn read(&self, rule: &Rule) -> Result<Option<Record>> {
      read_record(&self.path(rule)?)
   }

   fn forget(&self, rule: &Rule) -> Result<()> {
      remove(&self.path(rule)?)
   }

   fn remember(&self, rule: &Rule) -> Result<()> {
      let record = Record {
         signature: rule.signature(),
         outputs: rule.outputs.clone(),
      };
      let path = self.path(rule)?;
      let text = serde_json::to_string(&record).context("encoding a rule record")?;
      fs::write(&path, text).with_context(|| format!("writing {}", path.display()))
   }

   /// Removes what earlier runs wrote that no current rule writes.
   fn prune(&self, rules: &[Rule]) -> Result<()> {
      let current: HashSet<PathBuf> = rules
         .iter()
         .map(|rule| self.path(rule))
         .collect::<Result<_>>()?;
      let written: HashSet<&Path> = rules
         .iter()
         .flat_map(|rule| rule.outputs.iter().map(PathBuf::as_path))
         .collect();

      for entry in
         fs::read_dir(&self.dir).with_context(|| format!("reading {}", self.dir.display()))?
      {
         let path = entry
            .with_context(|| format!("walking {}", self.dir.display()))?
            .path();
         if let Some(record) = read_record(&path)? {
            for output in record
               .outputs
               .iter()
               .filter(|output| !written.contains(output.as_path()))
            {
               remove(output)?;
            }
         }
         if !current.contains(&path) {
            remove(&path)?;
         }
      }
      Ok(())
   }
}

// A record that doesn't parse counts as missing, so its rule runs again.
fn read_record(path: &Path) -> Result<Option<Record>> {
   match fs::read_to_string(path) {
      Ok(text) => Ok(serde_json::from_str(&text).ok()),
      Err(err) if err.kind() == std::io::ErrorKind::NotFound => Ok(None),
      Err(err) => Err(err).with_context(|| format!("reading {}", path.display())),
   }
}

fn remove(path: &Path) -> Result<()> {
   match fs::remove_file(path) {
      Ok(()) => Ok(()),
      Err(err) if err.kind() == std::io::ErrorKind::NotFound => Ok(()),
      Err(err) => Err(err).with_context(|| format!("removing {}", path.display())),
   }
}

/// Runs the stale rules and returns how many files they wrote.
pub fn run(rules: Vec<Rule>, state: &State) -> Result<usize> {
   state.prune(&rules)?;
   let waves = schedule(&rules)?;
   let mut rules: Vec<Option<Rule>> = rules.into_iter().map(Some).collect();
   let mut written = 0;

   for wave in waves {
      let ready: Vec<Rule> = wave
         .into_iter()
         .map(|index| {
            rules[index]
               .take()
               .context("every rule is scheduled exactly once")
         })
         .collect::<Result<_>>()?;

      // Checked per wave, since earlier waves rewrite the inputs of later ones.
      let stale = ready
         .par_iter()
         .map(|rule| rule.stale(state))
         .collect::<Result<Vec<_>>>()?;

      let due: Vec<Rule> = ready
         .into_iter()
         .zip(stale)
         .filter_map(|(rule, stale)| stale.then_some(rule))
         .collect();

      written += due.iter().map(|rule| rule.outputs.len()).sum::<usize>();
      let progress = crate::progress::bar(due.len(), "generating");
      due.into_par_iter().try_for_each(|rule| {
         let result = rule.execute(state);
         progress.inc(1);
         result
      })?;
      progress.finish_and_clear();
   }
   Ok(written)
}

/// Groups rule indices into parallel waves, each depending only on earlier ones.
fn schedule(rules: &[Rule]) -> Result<Vec<Vec<usize>>> {
   let mut producer: HashMap<&Path, usize> = HashMap::new();
   for (index, rule) in rules.iter().enumerate() {
      for output in &rule.outputs {
         if let Some(first) = producer.insert(output, index) {
            bail!(
               "{} is written by both {} and {}",
               output.display(),
               rules[first].what,
               rule.what
            );
         }
      }
   }

   let mut blocking = vec![0usize; rules.len()];
   let mut dependents = vec![Vec::new(); rules.len()];
   for (index, rule) in rules.iter().enumerate() {
      for input in &rule.inputs {
         // Inputs nothing generates are tracked files.
         if let Some(&from) = producer.get(input.as_path())
            && from != index
         {
            dependents[from].push(index);
            blocking[index] += 1;
         }
      }
   }

   let mut waves = Vec::new();
   let mut ready: Vec<usize> = (0..rules.len()).filter(|&i| blocking[i] == 0).collect();
   let mut scheduled = 0;
   while !ready.is_empty() {
      scheduled += ready.len();
      let mut next = Vec::new();
      for &index in &ready {
         for &dependent in &dependents[index] {
            blocking[dependent] -= 1;
            if blocking[dependent] == 0 {
               next.push(dependent);
            }
         }
      }
      waves.push(std::mem::replace(&mut ready, next));
   }

   if scheduled != rules.len() {
      bail!("the generators depend on each other in a cycle");
   }
   Ok(waves)
}

fn modified(path: &Path) -> Result<Option<SystemTime>> {
   match std::fs::metadata(path) {
      Ok(meta) => Ok(Some(meta.modified().with_context(|| {
         format!("reading the timestamp of {}", path.display())
      })?)),
      Err(err) if err.kind() == std::io::ErrorKind::NotFound => Ok(None),
      Err(err) => Err(err).with_context(|| format!("reading {}", path.display())),
   }
}
