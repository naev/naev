project('SuiteSparse')

cc = meson.get_compiler('c')
foreach arg: ['-Wno-unused-but-set-variable', '-Wno-unused-parameter']
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

subdir('CSparse')
