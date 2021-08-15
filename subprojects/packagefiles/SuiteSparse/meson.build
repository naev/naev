project('SuiteSparse')

cc = meson.get_compiler('c')
foreach arg: ['-Wno-implicit-fallthrough', '-Wno-sign-compare',
              '-Wno-unused-but-set-variable', '-Wno-unused-const-variable', '-Wno-unused-variable', '-Wno-unused-parameter']
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

subdir('SuiteSparse_config')
subdir('AMD')
subdir('COLAMD')
subdir('CSparse')
subdir('CHOLMOD')

SuiteSparse_dep = [SuiteSparse_config_dep, AMD_dep, COLAMD_dep, CSparse_dep, CHOLMOD_dep]
