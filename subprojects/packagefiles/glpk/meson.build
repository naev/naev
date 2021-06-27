project('glpk', 'c', version: '5.0')

cc = meson.get_compiler('c')
foreach arg: [
              '-Wno-implicit-fallthrough', '-Wno-misleading-indentation', '-Wno-parentheses',
	      '-Wno-return-type', '-Wno-sign-compare',
              '-Wno-unused-but-set-variable', '-Wno-unused-function', '-Wno-unused-parameter',
	      '-Wno-unused-value', '-Wno-unused-variable',
             ]
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

subdir('src')
glpk = declare_dependency(link_with: libglpk, include_directories: 'src', version: meson.project_version())
