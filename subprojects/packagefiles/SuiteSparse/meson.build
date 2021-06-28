project('SuiteSparse')

cc = meson.get_compiler('c')
foreach arg: ['-Wno-implicit-fallthrough', '-Wno-unused-but-set-variable', '-Wno-unused-parameter']
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

subdir('AMD')
subdir('COLAMD')
subdir('CSparse')
