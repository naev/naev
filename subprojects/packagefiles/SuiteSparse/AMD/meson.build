source = [
   configure_file(input: 'Config/amd.h.in', output: 'amd.h', configuration: config),
   'Source/amd_1.c',          # 'Source/amd_l1.c',
   'Source/amd_2.c',          # 'Source/amd_l2.c',
   'Source/amd_aat.c',        # 'Source/amd_l_aat.c',
   'Source/amd_control.c',    # 'Source/amd_l_control.c',
   'Source/amd_defaults.c',   # 'Source/amd_l_defaults.c',
   'Source/amd_dump.c',       # 'Source/amd_l_dump.c',
   'Source/amd_info.c',       # 'Source/amd_l_info.c',
   'Source/amd_order.c',      # 'Source/amd_l_order.c',
   'Source/amd_postorder.c',  # 'Source/amd_l_postorder.c',
   'Source/amd_post_tree.c',  # 'Source/amd_l_post_tree.c',
   'Source/amd_preprocess.c', # 'Source/amd_l_preprocess.c',
   'Source/amd_valid.c',      # 'Source/amd_l_valid.c',
]

include = include_directories([
   'Include',
   '../SuiteSparse_config',
])

cc = meson.get_compiler('c')
system_deps = [
   cc.find_library('m', required : false)
]

libamd = static_library(
   'amd',
   source,
   dependencies: system_deps,
   include_directories: include
)

AMD_dep = declare_dependency(
   link_with: libamd,
   include_directories: include,
   dependencies: system_deps
)
