project('glpk', 'c', version: '5.0')
subdir('src')
glpk = declare_dependency(link_with: libglpk, include_directories: 'src', version: meson.project_version())
