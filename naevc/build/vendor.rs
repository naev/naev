use std::{
   collections::HashSet,
   fs,
   path::{Path, PathBuf},
};

#[derive(serde::Deserialize)]
struct Manifest {
   build: Build,
}

// The [build] table of a library's vendor.toml. [upstream] only records provenance.
#[derive(serde::Deserialize)]
struct Build {
   library: String,
   sources: Vec<PathBuf>,
   include: Vec<PathBuf>,
}

/// Builds every library under vendor/ and returns their include dirs.
pub fn build_all(crate_dir: &Path) -> Vec<PathBuf> {
   let vendor = crate_dir.join("vendor");
   println!("cargo:rerun-if-changed={}", vendor.display());
   let mut libraries: Vec<PathBuf> = listed(&vendor)
      .into_iter()
      .filter(|path| path.is_dir())
      .collect();
   libraries.sort();
   libraries.iter().flat_map(|dir| build(dir)).collect()
}

fn build(dir: &Path) -> Vec<PathBuf> {
   let path = dir.join("vendor.toml");
   let text =
      fs::read_to_string(&path).unwrap_or_else(|err| panic!("reading {}: {err}", path.display()));
   let manifest: Manifest =
      toml::from_str(&text).unwrap_or_else(|err| panic!("parsing {}: {err}", path.display()));
   let build = manifest.build;

   let include: Vec<PathBuf> = build.include.iter().map(|rel| dir.join(rel)).collect();
   cc::Build::new()
      .includes(&include)
      .files(compiled_sources(dir, &build.sources))
      .compile(&build.library);
   include
}

/// Every .c file in the source dirs except those another file includes, which build as part of it.
fn compiled_sources(dir: &Path, source_dirs: &[PathBuf]) -> Vec<PathBuf> {
   let files: Vec<PathBuf> = source_dirs
      .iter()
      .flat_map(|rel| listed(&dir.join(rel)))
      .filter(|path| path.is_file())
      .collect();
   let included: HashSet<String> = files.iter().flat_map(|file| included_c(file)).collect();

   let mut sources: Vec<PathBuf> = files
      .into_iter()
      .filter(|file| file.extension().is_some_and(|ext| ext == "c"))
      .filter(|file| !included.contains(&name(file)))
      .collect();
   sources.sort();
   sources
}

/// The file names of the .c files a source or header pulls in with #include "...".
fn included_c(file: &Path) -> Vec<String> {
   let bytes = fs::read(file).unwrap_or_else(|err| panic!("reading {}: {err}", file.display()));
   String::from_utf8_lossy(&bytes)
      .lines()
      .filter_map(|line| line.trim_start().strip_prefix('#'))
      .filter_map(|line| line.trim_start().strip_prefix("include"))
      .filter_map(|line| line.trim_start().strip_prefix('"'))
      .filter_map(|line| line.split('"').next())
      .filter(|target| target.ends_with(".c"))
      .map(|target| name(Path::new(target)))
      .collect()
}

fn name(path: &Path) -> String {
   path
      .file_name()
      .map(|name| name.to_string_lossy().into_owned())
      .unwrap_or_default()
}

fn listed(dir: &Path) -> Vec<PathBuf> {
   fs::read_dir(dir)
      .unwrap_or_else(|err| panic!("reading {}: {err}", dir.display()))
      .map(|entry| {
         entry
            .unwrap_or_else(|err| panic!("reading {}: {err}", dir.display()))
            .path()
      })
      .collect()
}
