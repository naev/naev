use std::{
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use clap::{Args, ValueEnum};

#[derive(Clone, Copy, Debug, PartialEq, Eq, ValueEnum)]
pub enum Debugger {
   /// gdb if installed, else lldb, else neither.
   Auto,
   Gdb,
   Lldb,
   /// Run the game without a debugger.
   None,
}

/// A debugger found on PATH.
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

#[derive(Args, Debug)]
pub struct BuildArgs {
   /// Build the release profile, which drops the debug checks. The default is release-with-debug.
   #[arg(long, conflicts_with = "debug_rust")]
   release: bool,

   /// Build the dev profile, which has no optimization.
   #[arg(long)]
   debug_rust: bool,

   /// Cargo features to build with.
   #[arg(long, value_delimiter = ',')]
   features: Vec<String>,
}

impl BuildArgs {
   // Mirrors meson's debug_rust and debug options, since unoptimized Rust runs slowly.
   fn profile(&self) -> &'static str {
      if self.release {
         "release"
      } else if self.debug_rust {
         "dev"
      } else {
         "release-with-debug"
      }
   }

   fn cargo_args(&self, cmd: &mut Command) {
      cmd.args(["--profile", self.profile()]);
      if !self.features.is_empty() {
         cmd.arg("--features").arg(self.features.join(","));
      }
   }

   fn binary(&self, target: &Path) -> PathBuf {
      // cargo names the dev profile's directory debug.
      let dir = match self.profile() {
         "dev" => "debug",
         profile => profile,
      };
      target
         .join(dir)
         .join(format!("naev{}", std::env::consts::EXE_SUFFIX))
   }
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

   /// Arguments passed through to the game.
   #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
   args: Vec<String>,
}

#[derive(Args, Debug)]
pub struct ValgrindArgs {
   #[command(flatten)]
   build: BuildArgs,

   /// Wait for a debugger to attach over vgdb.
   #[arg(long)]
   server: bool,

   /// Suppression files to use on top of utils/build/naev.supp.
   #[arg(long)]
   suppressions: Vec<PathBuf>,

   /// Where valgrind writes its log. Defaults to the target directory.
   #[arg(long)]
   log_file: Option<PathBuf>,

   /// Follow the processes the game spawns.
   #[arg(long)]
   trace_children: bool,

   /// An extra argument for valgrind itself.
   #[arg(long = "valgrind-arg", allow_hyphen_values = true)]
   valgrind_args: Vec<String>,

   /// Arguments passed through to the game.
   #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
   args: Vec<String>,
}

pub fn run(root: &Path, target: &Path, data_dir: &Path, args: &RunArgs) -> Result<()> {
   crate::data::generate(root, data_dir)?;
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

pub fn valgrind(root: &Path, target: &Path, data_dir: &Path, args: &ValgrindArgs) -> Result<()> {
   crate::data::generate(root, data_dir)?;
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
      // Works around valgrind aborting with "VEX temporary storage exhausted".
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
   cmd.args(&args.valgrind_args);
   let log = args
      .log_file
      .clone()
      .unwrap_or_else(|| target.join("naev_valgrind.log"));
   cmd.arg(format!("--log-file={}", log.display())).arg(&game);

   finish(&mut cmd, root, data_dir, &args.build, &args.args);
   exec(cmd)
}

/// Appends the game's mounts, arguments and environment after the binary.
fn finish(cmd: &mut Command, root: &Path, data_dir: &Path, build: &BuildArgs, args: &[String]) {
   // physfs searches mounts in order, so the generated tree comes first to override tracked data.
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

fn build(root: &Path, target: &Path, args: &BuildArgs) -> Result<PathBuf> {
   let cargo = std::env::var_os("CARGO").unwrap_or_else(|| "cargo".into());
   let mut cmd = Command::new(cargo);
   cmd.current_dir(root).args(["build", "--package", "naev"]);
   args.cargo_args(&mut cmd);

   let status = cmd.status().context("failed to run cargo build")?;
   if !status.success() {
      bail!("building the game failed with {status}");
   }

   let game = args.binary(target);
   if !game.is_file() {
      bail!("cargo left no binary at {}", game.display());
   }
   Ok(game)
}

fn resolve(choice: Debugger) -> Result<Option<Tool>> {
   let found = |tool: Tool| on_path(tool.program()).then_some(tool);
   match choice {
      Debugger::None => Ok(None),
      Debugger::Gdb => Ok(Some(found(Tool::Gdb).context("gdb is not on PATH")?)),
      Debugger::Lldb => Ok(Some(found(Tool::Lldb).context("lldb is not on PATH")?)),
      Debugger::Auto => Ok(found(Tool::Gdb).or_else(|| found(Tool::Lldb))),
   }
}

fn debugger_config(root: &Path, target: &Path, tool: Tool) -> Result<PathBuf> {
   let (template, name) = match tool {
      Tool::Gdb => ("utils/build/gdbinit.in", ".gdbinit"),
      Tool::Lldb => ("utils/build/lldbinit.py.in", "lldbinit.py"),
   };
   // Forward slashes, since the lldb template puts the path in a Python string.
   let slashed = |path: &Path| path.display().to_string().replace('\\', "/");
   let text = fs::read_to_string(root.join(template))
      .with_context(|| format!("reading {template}"))?
      .replace("@source_root@", &slashed(root))
      .replace("@build_root@", &slashed(target));

   let dest = target.join(name);
   fs::write(&dest, text).with_context(|| format!("writing {}", dest.display()))?;
   Ok(dest)
}

// Kept in the target dir so vgdb can't attach to an unrelated valgrind session.
fn vgdb_prefix(target: &Path) -> PathBuf {
   target.join(".vgdb-pipe")
}

pub fn on_path(program: &str) -> bool {
   let name = format!("{program}{}", std::env::consts::EXE_SUFFIX);
   std::env::var_os("PATH")
      .is_some_and(|paths| std::env::split_paths(&paths).any(|dir| dir.join(&name).is_file()))
}

/// Replaces this process so the game owns the terminal, signals and exit status.
#[cfg(unix)]
fn exec(mut cmd: Command) -> Result<()> {
   use std::os::unix::process::CommandExt;

   Err(cmd.exec()).context("failed to start the game")
}

#[cfg(not(unix))]
fn exec(mut cmd: Command) -> Result<()> {
   let status = cmd.status().context("failed to start the game")?;
   std::process::exit(status.code().unwrap_or(1));
}
