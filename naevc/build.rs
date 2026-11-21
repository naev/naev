use std::{
   env, fs,
   path::{Path, PathBuf},
   process::Command,
};

#[path = "build/configure.rs"]
mod configure;
#[path = "build/deps.rs"]
mod deps;
#[path = "build/headers.rs"]
mod headers;
#[path = "build/sources.rs"]
mod sources;

const INCLUDE_DIRS: &[&str] = &["src", "src/tk", "src/tk/widget"];

// Handle issues with bindgen and the math.h header.
const BLOCKLIST: &[&str] = &[
   "__mingw_ldbl_type_t",
   "FP_INT_.*",
   "FP_SUBNORMAL*",
   "FP_NORMAL*",
   "FP_NAN*",
   "FP_INFINITE*",
   "FP_ZERO*",
   // clang 22's amxintrin.h marks the AMX tile registers as both packed and
   // 64-byte aligned, which bindgen has no way to express together.
   ".*tile1024i.*",
];

// These can longjmp out through Lua.
const UNWIND_FNS: &[&str] = &[
   "nlua_package_.*",
   "cli_print",
   "cli_printRaw",
   "cli_warn",
   "luaopen_utf8",
   "luaopen_cmark",
   "luaopen_enet",
];

fn main() {
   let manifest_dir =
      PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("cargo always sets CARGO_MANIFEST_DIR"));
   let root = manifest_dir
      .parent()
      .expect("the naevc crate always lives one level below the repo root")
      .to_path_buf();
   let out_dir =
      PathBuf::from(env::var("OUT_DIR").expect("cargo always sets OUT_DIR for build scripts"));

   let deps = deps::probe();
   let cfg = configure::detect();
   let config_h = configure::write_config_h(&out_dir, &cfg, &deps);
   configure::write_version_header(&out_dir, &cfg);
   configure::write_config_rs(&out_dir, &cfg);

   generate_tables(&root, &out_dir);
   compile_engine(&root, &out_dir, &deps, &config_h);
   generate_bindings(&root, &out_dir, &deps, &config_h);
}

// The headers include the generated tables, so this runs before bindgen.
fn generate_tables(root: &Path, out_dir: &Path) {
   for script in ["src/shaders_c_gen.py", "src/colours_c_gen.py"] {
      let path = root.join(script);
      let status = Command::new("python3")
         .arg(&path)
         .current_dir(out_dir)
         .status()
         .unwrap_or_else(|e| panic!("failed to run python3 for {script}: {e}"));
      assert!(status.success(), "{script} failed with {status}");
      println!("cargo:rerun-if-changed={}", path.display());
   }
}

fn compile_engine(root: &Path, out_dir: &Path, deps: &deps::Deps, config_h: &Path) {
   let mut build = cc::Build::new();
   configure_cc(&mut build, root, out_dir, deps, config_h);
   build.files(sources::SOURCES.iter().map(|s| root.join(s)));
   build.file(out_dir.join("shaders.gen.c"));
   build.file(out_dir.join("colours.gen.c"));
   if env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("macos") {
      build.files(sources::MACOS_SOURCES.iter().map(|s| root.join(s)));
   }
   build.compile("naev");

   // Debug builds of the SDF code make the intro stutter under LANGUAGE=ja.
   let mut sdf = cc::Build::new();
   configure_cc(&mut sdf, root, out_dir, deps, config_h);
   sdf.opt_level(3)
      .files(sources::SDF_SOURCES.iter().map(|s| root.join(s)));
   sdf.compile("naevsdf");

   for source in sources::SOURCES
      .iter()
      .chain(sources::SDF_SOURCES)
      .chain(sources::MACOS_SOURCES)
   {
      println!("cargo:rerun-if-changed={}", root.join(source).display());
   }
}

fn configure_cc(
   build: &mut cc::Build,
   root: &Path,
   out_dir: &Path,
   deps: &deps::Deps,
   config_h: &Path,
) {
   build
      .std("c11")
      .includes(INCLUDE_DIRS.iter().map(|d| root.join(d)))
      .include(out_dir)
      .includes(&deps.include_paths)
      // No source includes config.h itself, so it's forced in.
      .flag("-include")
      .flag(config_h.to_string_lossy().as_ref())
      .flag_if_supported("-Wno-pedantic")
      .flag_if_supported("-Wshadow")
      .flag_if_supported("-fno-signed-zeros");

   match env::var("CARGO_CFG_TARGET_OS").as_deref() {
      Ok("linux") => {
         build.define("_XOPEN_SOURCE", "700");
      }
      Ok("windows") => {
         // SDL_DISABLE_ALLOCA works around
         // https://github.com/libsdl-org/SDL/issues/13358
         build
            .define("_USE_MATH_DEFINES", None)
            .define("SDL_DISABLE_ALLOCA", None)
            // Windows 7 is the oldest supported.
            .define("WINVER", "0x0601")
            .define("_WIN32_WINNT", "0x0601");
      }
      Ok("macos") => {
         build
            .define("_DARWIN_C_SOURCE", None)
            .define("_POSIX_C_SOURCE", "200809L");
      }
      _ => {}
   }
}

fn generate_bindings(root: &Path, out_dir: &Path, deps: &deps::Deps, config_h: &Path) {
   // bindgen wants a single translation unit.
   let merged: String = headers::HEADERS
      .iter()
      .map(|h| format!("#include \"{}\"\n", root.join(h).display()))
      .collect();
   let merged_path = out_dir.join("naevc.h");
   fs::write(&merged_path, merged).expect("OUT_DIR is writable");

   let mut builder = bindgen::Builder::default()
      .header(merged_path.to_string_lossy())
      // The C compile gets the same config.h, so both agree on what is declared.
      .clang_args(["-include", config_h.to_string_lossy().as_ref()])
      // https://github.com/rust-lang/rust-bindgen/issues/3147
      .wrap_unsafe_ops(true);

   for dir in INCLUDE_DIRS {
      builder = builder.clang_arg(format!("-I{}", root.join(dir).display()));
   }
   // shaders.gen.h and colours.gen.h land here.
   builder = builder.clang_arg(format!("-I{}", out_dir.display()));
   for dir in &deps.include_paths {
      builder = builder.clang_arg(format!("-I{}", dir.display()));
   }
   for &item in BLOCKLIST {
      builder = builder.blocklist_item(item);
   }
   for &func in UNWIND_FNS {
      builder = builder.override_abi(bindgen::Abi::CUnwind, func);
   }

   builder
      .generate()
      .expect("bindgen failed to parse the Naev headers")
      .write_to_file(out_dir.join("naevc.rs"))
      .expect("OUT_DIR is writable");

   for header in headers::HEADERS.iter().chain(headers::UNPARSED_HEADERS) {
      println!("cargo:rerun-if-changed={}", root.join(header).display());
   }
}
