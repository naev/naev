project(
   'lyaml',
   'c',
   version: '6.2.8',
   default_options: ['c_std=c89', 'warning_level=3'],
   meson_version: '>=0.53.0',
   license: 'BSD-2-Clause',
)

cc = meson.get_compiler('c')
add_project_arguments('-DPACKAGE="lyaml"', language: 'c')
add_project_arguments('-DVERSION="6.2.8"', language: 'c')
add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c') # uses strdup which is not C89
if not get_option('debug')
   add_project_arguments('-DNDEBUG=1', language: 'c')
   add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')
endif
# Ideally we would want this fixed upstream
add_project_arguments('-Wno-unused-parameter', language: 'c')
add_project_arguments('-Wno-unused-variable', language: 'c')
add_project_arguments('-Wno-int-conversion', language: 'c')

lyaml_src = files(
   'ext/yaml/yaml.c',
   'ext/yaml/emitter.c',
   'ext/yaml/parser.c',
   'ext/yaml/scanner.c',
)

lyaml_lua = files(
   'lib/lyaml/init.lua',
   'lib/lyaml/explicit.lua',
   'lib/lyaml/functional.lua',
   'lib/lyaml/implicit.lua',
)

deps = []
deps += dependency('yaml-0.1', required: true, version: '>=0.2.5', fallback: ['libyaml', 'yaml_dep'], static: get_option('steamruntime'), default_options: ['c_std=c89'])

useLuaJIT = get_option('luajit')
if useLuaJIT.disabled() == false
   lua = dependency('luajit', fallback: ['luajit'], required: useLuaJIT, static: get_option('steamruntime'), default_options: ['c_std=gnu99', 'luajit=false'])
endif
summary('LuaJIT', lua.found(), section: 'Features', bool_yn: true)
if not lua.found()
   lua = dependency('lua51', fallback: ['lua', 'lua_dep'], required: true)
endif
deps += lua

includes = include_directories('.', 'ext/yaml')

# Only statically link if platform isn't Windows (Or Cygwin, which should cover edge cases)
if host_machine.system() == 'windows' or host_machine.system() == 'cygwin'
   lyaml = shared_library(
      'lyaml',
      lyaml_src,
      include_directories: includes,
      dependencies: deps,
   )
else
   lyaml = static_library(
      'lyaml',
      lyaml_src,
      include_directories: includes,
      dependencies: deps,
   )
endif

lyaml_dep = declare_dependency(
   link_with: lyaml,
   include_directories: includes,
)
