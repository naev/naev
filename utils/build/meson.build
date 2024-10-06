cargo_data = configuration_data()
cargo_data.set_quoted('VERSION', meson.project_version() )
naevc_crate_cargo = configure_file(input: 'Cargo.toml.in', output: 'Cargo.toml', configuration: cargo_data )
