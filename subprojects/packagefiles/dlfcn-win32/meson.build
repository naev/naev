project('dlfcn-win32', 'c', version: '1.4.0')
dl = static_library('dl', 'src/dlfcn.c')
dl_dep = declare_dependency(link_with: dl, include_directories: 'src', version: meson.project_version())
