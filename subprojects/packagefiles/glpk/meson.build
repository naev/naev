project('glpk', 'c', version: '5.0')

cc = meson.get_compiler('c')
foreach arg: [
              '-Wno-clobbered', '-Wno-implicit-fallthrough', '-Wno-misleading-indentation', '-Wno-macro-redefined',
	      '-Wno-parentheses', '-Wno-return-type', '-Wno-self-assign', '-Wno-sign-compare', '-Wno-sometimes-uninitialized',
	      '-Wno-strict-aliasing',  # No way that will come back to bite us!
	      '-Wno-uninitialized', '-Wno-unused-but-set-variable', '-Wno-unused-function', '-Wno-unused-parameter',
	      '-Wno-unused-value', '-Wno-unused-variable',
             ]
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

subdir('src')

glpk_dep = declare_dependency(link_with: libglpk, include_directories: 'src', version: meson.project_version())
