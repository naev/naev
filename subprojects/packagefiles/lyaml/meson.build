project(
    'lyaml',
    'c',
    version: '6.2.8',
    default_options: ['warning_level=3'],
    meson_version: '>=0.53.0',
    license: 'BSD-2-Clause',
)

cc = meson.get_compiler('c')
add_project_arguments('-DPACKAGE="lyaml"', language: 'c')
add_project_arguments('-DVERSION="6.2.8"', language: 'c')
if not get_option('debug')
  add_project_arguments('-DNDEBUG=1', language: 'c')
  add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')
endif
# Ideally we would want this fixed upstream
add_project_arguments('-Wno-unused-parameter', language: 'c')
add_project_arguments('-Wno-unused-variable', language: 'c')

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
deps += dependency('yaml-0.1', required: true)

useLuaJIT = get_option('luajit')
if useLuaJIT.disabled() == false
    lua = dependency('luajit', fallback: ['luajit'], required: useLuaJIT, static: get_option('steamruntime'), default_options: ['luajit=false'])
endif
summary('LuaJIT', lua.found(), section: 'Features', bool_yn: true)
if not lua.found()
    lua = dependency('lua51', fallback: ['lua'], required: true)
endif
deps += lua

includes = include_directories('.', 'ext/yaml')

lyaml = static_library(
    'lyaml',
    lyaml_src,
    include_directories: includes,
    dependencies: deps,
)

lyaml_dep = declare_dependency(
    link_with: lyaml,
    include_directories: includes,
)
