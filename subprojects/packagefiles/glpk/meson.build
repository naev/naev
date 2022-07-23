project('glpk', 'c', version: '5.0')

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments([
   '-Wno-array-compare', '-Wno-clobbered', '-Wno-implicit-fallthrough', '-Wno-misleading-indentation', '-Wno-macro-redefined',
   '-Wno-parentheses', '-Wno-return-type', '-Wno-self-assign', '-Wno-sign-compare', '-Wno-sometimes-uninitialized',
   '-Wno-strict-aliasing', '-Wno-stringop-overflow',  # No way these will come back to bite us!
   '-Wno-uninitialized', '-Wno-unused-but-set-variable', '-Wno-unused-function', '-Wno-unused-parameter', '-Wno-unused-value', '-Wno-unused-variable',
])
add_project_arguments(c_args, language: 'c')

subdir('src')

glpk_dep = declare_dependency(link_with: libglpk, include_directories: 'src', version: meson.project_version())
