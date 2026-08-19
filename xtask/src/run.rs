//! The development launchers.
//!
//! These run the game straight out of the source tree with the generated data
//! layered over the tracked data, standing in for the naev.py and
//! naev_valgrind.py that meson used to template.

use std::{
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

/// Which debugger to ask for.
#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Debugger {
   /// gdb if it is installed, else lldb, else neither.
   Auto,
   Gdb,
   Lldb,
   /// Run the game with nothing in front of it.
   None,
}

/// A debugger that is actually installed.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Tool {
   Gdb,
   Lldb,
}

impl Tool {
   fn program(self) -> &'static str {
      match self {
         Tool::Gdb => "gdb",
         Tool::Lldb => "lldb",
      }
   }
}

/// How the game gets built, shared by both launchers.
#[derive(Args, Debug)]
pub struct BuildArgs {
   /// Build with optimisations.
   #[arg(long)]
   release: bool,

   /// Cargo features to build with.
   #[arg(long, value_delimiter = ',')]
   features: Vec<String>,
}

#[derive(Args, Debug)]
pub struct RunArgs {
   #[command(flatten)]
   build: BuildArgs,

   /// Debugger to run the game under.
   #[arg(long, value_enum, default_value = "auto")]
   debugger: Debugger,

   /// Attach to the server left waiting by `xtask valgrind --server`.
   #[arg(long)]
   valgrind: bool,

   /// Everything left over is handed to the game.
   #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
   args: Vec<String>,
}

#[derive(Args, Debug)]
pub struct ValgrindArgs {
   #[command(flatten)]
   build: BuildArgs,

   /// Wait for a debugger to attach over vgdb instead of running through.
   #[arg(long)]
   server: bool,

   /// Suppression files to use on top of utils/build/naev.supp.
   #[arg(long)]
   suppressions: Vec<PathBuf>,

   /// Where valgrind writes its log, by default into the target directory.
   #[arg(long)]
   log_file: Option<PathBuf>,

   /// Follow the processes the game spawns.
   #[arg(long)]
   trace_children: bool,

   /// Everything left over is handed to the game.
   #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
   args: Vec<String>,
}

/// Builds the game and runs it, optionally under a debugger.
pub fn run(root: &Path, target: &Path, data_dir: &Path, args: &RunArgs) -> Result<()> {
   require_data(data_dir)?;
   let tool = resolve(args.debugger)?;
   if args.valgrind && tool != Some(Tool::Gdb) {
      bail!("--valgrind attaches through vgdb, which only gdb speaks");
   }
   let game = build(root, target, &args.build)?;

   let mut cmd = match tool {
      Some(Tool::Gdb) => {
         let mut cmd = Command::new("gdb");
         cmd.arg("--nx")
            .arg("-x")
            .arg(debugger_config(root, target, Tool::Gdb)?);
         if args.valgrind {
            if !on_path("vgdb") {
               bail!("vgdb is not on PATH; it ships alongside valgrind");
            }
            cmd.arg("-ex")
               .arg(format!(
                  "target remote | vgdb --vgdb-prefix={}",
                  vgdb_prefix(target).display()
               ))
               .arg("-ex")
               .arg("continue");
         } else {
            cmd.arg("-ex").arg("run");
         }
         cmd.arg("--args").arg(&game);
         cmd
      }
      Some(Tool::Lldb) => {
         let mut cmd = Command::new("lldb");
         cmd.arg("--one-line")
            .arg(format!(
               "command script import {}",
               debugger_config(root, target, Tool::Lldb)?.display()
            ))
            .arg("--")
            .arg(&game);
         cmd
      }
      None => Command::new(&game),
   };

   finish(&mut cmd, root, data_dir, &args.build, &args.args);
   exec(cmd)
}

/// Builds the game and runs it under valgrind, either straight through or
/// waiting for `xtask run --valgrind` to attach.
pub fn valgrind(root: &Path, target: &Path, data_dir: &Path, args: &ValgrindArgs) -> Result<()> {
   require_data(data_dir)?;
   if !on_path("valgrind") {
      bail!("valgrind is not on PATH");
   }
   let game = build(root, target, &args.build)?;

   let mut cmd = Command::new("valgrind");
   cmd.args([
      "--leak-check=full",
      "--show-leak-kinds=all",
      "--track-origins=yes",
      "--num-callers=100",
      "--error-limit=no",
      // Naev exhausts valgrind's per-block translation storage without this.
      "--vex-guest-max-insns=25",
   ]);

   if args.server {
      cmd.arg("--vgdb=yes")
         .arg("--vgdb-error=0")
         .arg(format!("--vgdb-prefix={}", vgdb_prefix(target).display()));
   }

   let default = root.join("utils/build/naev.supp");
   for supp in std::iter::once(&default).chain(&args.suppressions) {
      if supp.exists() {
         cmd.arg(format!("--suppressions={}", supp.display()));
      } else {
         eprintln!("skipping missing suppression file {}", supp.display());
      }
   }

   if args.trace_children {
      cmd.arg("--trace-children=yes");
   }
   let log = args
      .log_file
      .clone()
      .unwrap_or_else(|| target.join("naev_valgrind.log"));
   cmd.arg(format!("--log-file={}", log.display())).arg(&game);

   finish(&mut cmd, root, data_dir, &args.build, &args.args);
   exec(cmd)
}

/// Appends the data mounts, the caller's own arguments and the environment the
/// game expects. The command must already name the binary.
fn finish(cmd: &mut Command, root: &Path, data_dir: &Path, build: &BuildArgs, args: &[String]) {
   // physfs searches in the order the mounts arrive, so the generated tree has
   // to come first for the files it rebuilds to win.
   for mount in [data_dir, &root.join("dat"), &root.join("assets"), root] {
      cmd.arg("-d").arg(mount);
   }
   cmd.args(args);

   if build.features.iter().any(|f| f == "paranoid") {
      cmd.env("ALSOFT_LOGLEVEL", "3")
         .env("ALSOFT_TRAP_AL_ERROR", "1");
   } else if !build.release {
      cmd.env("ALSOFT_LOGLEVEL", "2");
   }
   cmd.env("RUST_BACKTRACE", "1")
      .env("ASAN_OPTIONS", "halt_on_error=1");
}

/// Builds the game and hands back the binary cargo produced.
fn build(root: &Path, target: &Path, args: &BuildArgs) -> Result<PathBuf> {
   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let mut cmd = Command::new(cargo);
   cmd.current_dir(root).args(["build", "--package", "naev"]);
   if args.release {
      cmd.arg("--release");
   }
   if !args.features.is_empty() {
      cmd.arg("--features").arg(args.features.join(","));
   }

   let status = cmd.status().context("failed to run cargo build")?;
   if !status.success() {
      bail!("building the game failed with {status}");
   }

   let profile = if args.release { "release" } else { "debug" };
   let game = target.join(profile).join("naev");
   if !game.is_file() {
      bail!("cargo left no binary at {}", game.display());
   }
   Ok(game)
}

/// Resolves a request against what is installed. Asking for a debugger that is
/// not there is an error, while `auto` settles for whatever it finds.
fn resolve(choice: Debugger) -> Result<Option<Tool>> {
   let found = |tool: Tool| on_path(tool.program()).then_some(tool);
   match choice {
      Debugger::None => Ok(None),
      Debugger::Gdb => Ok(Some(found(Tool::Gdb).context("gdb is not on PATH")?)),
      Debugger::Lldb => Ok(Some(found(Tool::Lldb).context("lldb is not on PATH")?)),
      Debugger::Auto => Ok(found(Tool::Gdb).or_else(|| found(Tool::Lldb))),
   }
}

/// Writes out the debugger configuration meson used to template, into the
/// target directory the debugger is then pointed at.
fn debugger_config(root: &Path, target: &Path, tool: Tool) -> Result<PathBuf> {
   let (template, name) = match tool {
      Tool::Gdb => ("utils/build/gdbinit.in", ".gdbinit"),
      Tool::Lldb => ("utils/build/lldbinit.py.in", "lldbinit.py"),
   };
   let text = fs::read_to_string(root.join(template))
      .with_context(|| format!("reading {template}"))?
      .replace("@source_root@", &root.display().to_string())
      .replace("@build_root@", &target.display().to_string());

   let dest = target.join(name);
   fs::write(&dest, text).with_context(|| format!("writing {}", dest.display()))?;
   Ok(dest)
}

/// Where the valgrind server and the debugger attaching to it agree to leave
/// their pipe.
fn vgdb_prefix(target: &Path) -> PathBuf {
   target.join(".vgdb-pipe")
}

fn require_data(data_dir: &Path) -> Result<()> {
   if !data_dir.is_dir() {
      bail!(
         "no generated data at {}; run `cargo xtask data` first",
         data_dir.display()
      );
   }
   Ok(())
}

/// Whether a program can be found on PATH.
fn on_path(program: &str) -> bool {
   std::env::var_os("PATH")
      .is_some_and(|paths| std::env::split_paths(&paths).any(|dir| dir.join(program).is_file()))
}

/// Replaces this process with the command, so the terminal, signals and exit
/// status belong to the game rather than to a wrapper standing in front of it.
#[cfg(unix)]
fn exec(mut cmd: Command) -> Result<()> {
   use std::os::unix::process::CommandExt;

   // exec only returns if it failed to replace the process.
   Err(cmd.exec()).context("failed to start the game")
}

#[cfg(not(unix))]
fn exec(mut cmd: Command) -> Result<()> {
   let status = cmd.status().context("failed to start the game")?;
   std::process::exit(status.code().unwrap_or(1));
}
