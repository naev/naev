fn main() {
   println!("cargo::rustc-link-lib=glpk");
   bindgen::Builder::default()
      .header_contents("glpk.h", "#include <glpk.h>")
      .allowlist_function("glp_.*")
      .allowlist_type("glp_.*")
      .allowlist_var("GLP_.*")
      .generate()
      .expect("glpk headers should parse")
      .write_to_file(
         std::path::PathBuf::from(std::env::var("OUT_DIR").expect("cargo sets OUT_DIR"))
            .join("glpk.rs"),
      )
      .expect("writing the glpk bindings");
}
