source = files([
   'Source/colamd.c',
])

include = include_directories([
   'Include',
   '../SuiteSparse_config',
])

cc = meson.get_compiler('c')
system_deps = [
   cc.find_library('m', required : false)
]

libcolamd = static_library(
   'colamd',
   source,
   dependencies: system_deps,
   include_directories: include
)

COLAMD_dep = declare_dependency(
   link_with: libcolamd,
   include_directories: include,
   dependencies: system_deps
)
