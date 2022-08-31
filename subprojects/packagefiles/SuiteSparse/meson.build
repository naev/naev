project('SuiteSparse', 'c', version: '5.13.0')

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments(['-Wno-implicit-const-int-float-conversion', '-Wno-implicit-fallthrough', '-Wno-maybe-uninitialized',
   '-Wno-sign-compare', '-Wno-unused-but-set-variable', '-Wno-unused-const-variable', '-Wno-unused-variable', '-Wno-unused-parameter'])
add_project_arguments(c_args, language: 'c')

subdir('SuiteSparse_config')
subdir('AMD')
subdir('COLAMD')
subdir('CSparse')
subdir('CHOLMOD')

SuiteSparse_dep = [SuiteSparse_config_dep, AMD_dep, COLAMD_dep, CSparse_dep, CHOLMOD_dep]
