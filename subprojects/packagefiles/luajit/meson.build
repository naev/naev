# Files in this overlay derived from https://github.com/franko/luajit
# as of commit d9791bf68eecfd3c41d669a0439577e59031a749 (2020-12-09).

project('luajit', 'c', version : '2.1.0-beta3', default_options : ['c_winlibs='])

cc = meson.get_compiler('c')

# GetProcAddress() in the Windows API requires typecasts from FARPROC to other function pointer types.
if host_machine.system() == 'windows' and cc.has_argument('-Wno-cast-function-type')
   add_project_arguments('-Wno-cast-function-type', language : 'c')
endif
# LuaJIT bootstraps its build using a minified Lua interpreter. This code is harmlessly somewhat messy.
if meson.get_compiler('c', native: true).has_argument('-Wno-implicit-fallthrough')
   add_project_arguments('-Wno-implicit-fallthrough', language : 'c', native: true)
endif
# And let's not warn about all of the casts in the code base.
if meson.get_compiler('c').has_argument('-Wno-pedantic')
   add_project_arguments('-Wno-pedantic', language : 'c')
endif

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
