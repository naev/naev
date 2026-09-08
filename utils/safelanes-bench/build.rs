fn main() {
   for lib in ["cholmod", "amd", "colamd", "suitesparseconfig"] {
      println!("cargo::rustc-link-lib={lib}");
   }
   bindgen::Builder::default()
      .header_contents("cholmod.h", "#include <suitesparse/cholmod.h>")
      .allowlist_function("cholmod_.*")
      .allowlist_type("cholmod_.*")
      .allowlist_var("CHOLMOD_.*")
      .generate()
      .expect("cholmod headers should parse")
      .write_to_file(
         std::path::PathBuf::from(std::env::var("OUT_DIR").expect("cargo sets OUT_DIR"))
            .join("cholmod.rs"),
      )
      .expect("writing the cholmod bindings");
}
