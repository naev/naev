source = [
   configure_file(input: 'Config/SuiteSparse_config.h.in', output: 'SuiteSparse_config.h', configuration: config),
   'SuiteSparse_config.c',
]

include = include_directories(['.'])

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments(['-Wno-array-parameter', '-Wno-use-after-free'])
# -Wno-use-after-free looks shady, but that's just for cs_realloc's code to "return original p if failure".
add_project_arguments(c_args, language : 'c')
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
