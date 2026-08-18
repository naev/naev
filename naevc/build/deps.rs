//! Dependency discovery for the engine's C sources.
//!
//! Mirrors what the meson build did: pkg-config for the libraries that ship a
//! `.pc` file, and a plain link directive for the ones that do not. meson
//! could fall back to building a wrap subproject when a library was missing;
//! cargo has no equivalent, so those are now hard requirements.

use std::path::PathBuf;

/// Libraries with no pkg-config file, found by linking against them directly.
/// Their headers live in the compiler's default search path.
const LINK_ONLY: &[&str] = &["glpk", "cholmod", "amd", "colamd", "suitesparseconfig"];

/// Same, but the build carries on without them if they are absent.
const LINK_ONLY_OPTIONAL: &[&str] = &["ccolamd", "lapack", "metis"];

/// Everything the C sources need in order to compile and link.
///
/// Note that `pkg_config`'s `probe` emits the link flags itself, so anything
/// discovered that way must not also be announced by hand.
pub struct Deps {
   /// Include directories to hand to both `cc` and bindgen.
   pub include_paths: Vec<PathBuf>,
   /// True when LuaJIT was found rather than plain Lua 5.1.
   pub have_luajit: bool,
   /// True when tracy support was requested and found.
   pub have_tracy: bool,
}

pub fn probe() -> Deps {
   let mut include_paths = Vec::new();

   // Versioned requirements, matching the old meson dependency() calls.
   for (name, version) in [
      ("sdl3", "3.2.0"),
      ("libenet", "1.3"),
      ("libunibreak", "4.0"),
      ("libcmark", "0.31.0"),
   ] {
      include_paths.extend(required(name, Some(version)));
   }
   for name in ["opus", "freetype2", "openal", "vorbis", "vorbisfile", "ogg"] {
      include_paths.extend(required(name, None));
   }

   // meson prefers LuaJIT and falls back to plain Lua 5.1. Probing emits the
   // link flags as a side effect, so each candidate is probed exactly once.
   let luajit = optional("luajit");
   let have_luajit = luajit.is_some();
   let lua = luajit
      .or_else(|| {
         ["lua51", "lua5.1", "lua"]
            .iter()
            .find_map(|name| optional(name))
      })
      .unwrap_or_else(|| missing("luajit or lua5.1", "LuaJIT (preferred) or Lua 5.1"));
   include_paths.extend(lua);

   // These are sometimes present without a pkg-config file, for instance
   // libxml2 inside the macOS SDK, so a miss is not fatal on its own. If the
   // headers really are absent the C compile says so precisely.
   for name in ["libxml-2.0", "physfs"] {
      include_paths.extend(optional(name).unwrap_or_default());
   }

   for name in LINK_ONLY {
      println!("cargo:rustc-link-lib={name}");
   }
   for name in LINK_ONLY_OPTIONAL {
      // Emitting these unconditionally would fail the link where they do not
      // exist, and meson treated them as optional too.
      if has_library(name) {
         println!("cargo:rustc-link-lib={name}");
      }
   }
   link_csparse();
   link_blas();
   link_platform();

   let have_tracy = probe_tracy(&mut include_paths);

   Deps {
      include_paths,
      have_luajit,
      have_tracy,
   }
}

/// c[x]sparse is packaged under either name depending on the distribution.
fn link_csparse() {
   for name in ["cxsparse", "csparse"] {
      if has_library(name) {
         println!("cargo:rustc-link-lib={name}");
         return;
      }
   }
   missing(
      "cxsparse or csparse",
      "SuiteSparse (CXSparse or CSparse component)",
   );
}

/// BLAS implementation. meson exposes this as -Dblas; keep it configurable
/// since Accelerate, blis and plain cblas are all viable.
fn link_blas() {
   let blas = std::env::var("NAEV_BLAS").unwrap_or_else(|_| "openblas".to_string());
   println!("cargo:rerun-if-env-changed=NAEV_BLAS");

   if blas == "Accelerate" {
      println!("cargo:rustc-link-lib=framework=Accelerate");
   } else {
      if !has_library(&blas) {
         missing(&blas, "a BLAS implementation, or set NAEV_BLAS");
      }
      println!("cargo:rustc-link-lib={blas}");
   }
}

fn link_platform() {
   let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
   match target_os.as_str() {
      "macos" => {
         println!("cargo:rustc-link-lib=framework=Foundation");
      }
      "windows" => {
         // GLAD needs dlopen; dlfcn-win32 provides it on Windows.
         println!("cargo:rustc-link-lib=dl");
      }
      _ => {
         println!("cargo:rustc-link-lib=m");
         println!("cargo:rustc-link-lib=dl");
      }
   }
}

/// Tracy is opt-in via the `tracy` feature. meson could build it from a wrap;
/// here it has to be installed.
fn probe_tracy(include_paths: &mut Vec<PathBuf>) -> bool {
   if std::env::var_os("CARGO_FEATURE_TRACY").is_none() {
      return false;
   }
   match optional("tracy") {
      // probe() already emitted the link flags.
      Some(paths) => {
         include_paths.extend(paths);
         true
      }
      None => missing("tracy", "the tracy profiler"),
   }
}

fn required(name: &str, version: Option<&str>) -> Vec<PathBuf> {
   let mut cfg = pkg_config::Config::new();
   if let Some(version) = version {
      cfg.atleast_version(version);
   }
   match cfg.probe(name) {
      Ok(lib) => lib.include_paths,
      Err(_) => {
         let what = match version {
            Some(v) => format!("{name} >= {v}"),
            None => name.to_string(),
         };
         missing(name, &what)
      }
   }
}

fn optional(name: &str) -> Option<Vec<PathBuf>> {
   pkg_config::Config::new()
      .probe(name)
      .ok()
      .map(|lib| lib.include_paths)
}

/// Checks a library is linkable, standing in for meson's cc.find_library().
///
/// This drives the compiler directly because `cc::Build` only ever archives
/// objects; there is no link step to hook, and linking is the whole point of
/// the check.
fn has_library(name: &str) -> bool {
   let out_dir = PathBuf::from(std::env::var("OUT_DIR").expect("cargo sets OUT_DIR"));
   let probe_dir = out_dir.join("libprobe");
   let _ = std::fs::create_dir_all(&probe_dir);
   let src = probe_dir.join(format!("probe_{name}.c"));
   if std::fs::write(&src, "int main(void) { return 0; }\n").is_err() {
      return false;
   }

   let mut cc = cc::Build::new();
   cc.file(&src)
      .out_dir(&probe_dir)
      .warnings(false)
      .cargo_metadata(false)
      .cargo_warnings(false)
      .cargo_output(false);
   let Ok(compiler) = cc.try_get_compiler() else {
      return false;
   };

   let exe = probe_dir.join(format!("probe_{name}"));
   std::process::Command::new(compiler.path())
      .args(compiler.args())
      .arg(&src)
      .arg(format!("-l{name}"))
      .arg("-o")
      .arg(&exe)
      .output()
      .map(|out| out.status.success())
      .unwrap_or(false)
}

fn missing(name: &str, what: &str) -> ! {
   panic!(
      "could not find {what}.\n\
       Naev needs {name} installed on the system. meson used to build a \
       bundled copy when it was missing; the cargo build requires the \
       development package instead."
   )
}
