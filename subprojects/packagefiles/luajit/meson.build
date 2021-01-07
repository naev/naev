# Files in this overlay derived from https://github.com/franko/luajit
# as of commit d9791bf68eecfd3c41d669a0439577e59031a749 (2020-12-09).

project('luajit', 'c', version : '2.1.0-beta3', default_options : ['c_winlibs='])

cc = meson.get_compiler('c')
libm = cc.find_library('m', required : false)
libdl = cc.find_library('dl', required : false)
luajit_dependencies = [libm, libdl]

luajit_source_dir = include_directories('src')
luajit_abiver = '51'

dynasm = files('dynasm/dynasm.lua')

subdir('src')

luajit_dep = declare_dependency(
    link_with: libluajit,
    include_directories: luajit_source_dir,
    dependencies: luajit_dependencies,
    version: meson.project_version())
