cargo_data = configuration_data()
cargo_data.set('BUILD', meson.project_build_root() )
cargo_data.set('SOURCE', meson.project_source_root() )
configure_file( input: 'Cargo.toml.in', output: 'Cargo.toml', configuration: cargo_data )
