# TODO can we just use the config_data from the root meson? :?
config_data = configuration_data()

ndata_path = get_option('ndata_path')
if ndata_path == ''
   ndata_path = get_option('datadir') / 'naev'
endif

config_data.set_quoted('PACKAGE', meson.project_name())
config_data.set_quoted('PACKAGE_NAME', meson.project_name())
config_data.set_quoted('PACKAGE_VERSION', meson.project_version())
config_data.set_quoted('PKGDATADIR', get_option('prefix') / ndata_path)
config_data.set_quoted('HOST', host_machine.system() + '-' + host_machine.cpu_family())

config_data.set('DEBUG', get_option('debug') ? 1 : false)
#config_data.set10('HAVE_LUAJIT', lua.found())

configrs = configure_file( input: 'config.rs.in', output: 'config.rs', configuration: config_data )
naevc_crate_cargo = configure_file(input: 'Cargo.toml.in', output: 'Cargo.toml', configuration: config_data )
