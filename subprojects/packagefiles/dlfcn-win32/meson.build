project('dlfcn-win32', 'c', version: '1.4.1')

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments(['-Wno-pedantic', '-Wno-sign-compare'])
add_project_arguments(c_args, language: 'c')

dl = static_library('dl', 'src/dlfcn.c')
dl_dep = declare_dependency(link_with: dl, include_directories: 'src', version: meson.project_version())
