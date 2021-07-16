source = files([
   'SuiteSparse_config.c',
])

include = include_directories(['.'])

cc = meson.get_compiler('c')
system_deps = [
   cc.find_library('m', required : false)
]

libsuitesparseconfig = static_library(
   'suitesparseconfig',
   source,
   dependencies: system_deps,
   include_directories: include
)

SuiteSparse_config_dep = declare_dependency(
   link_with: libsuitesparseconfig,
   include_directories: include,
   dependencies: system_deps
)
