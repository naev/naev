project(
    'lyaml',
    'c',
    version: '6.2.8',
    default_options: ['warning_level=3'],
    meson_version: '>=0.49.0',
    license: 'BSD-2-Clause',
)

cc = meson.get_compiler('c')
add_project_arguments('-DPACKAGE="lyaml"', language: 'c')
add_project_arguments('-DVERSION="6.2.8"', language: 'c')
add_project_arguments('-DNDEBUG=1', language: 'c')
add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')

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
deps += dependency('yaml-0.1', fallback: ['yaml_dep'], required: true )

useLuaJIT = get_option('luajit')
lua = dependency('', required: false)
if useLuaJIT.disabled() == false
    lua = dependency('luajit', fallback: ['luajit', 'luajit_dep'], required: useLuaJIT, static: get_option('steamruntime'))
endif
summary('LuaJIT', lua.found(), section: 'Features', bool_yn: true)
if not lua.found()
    lua = dependency('lua51', fallback: ['lua', 'lua_dep'], required: true)
endif
deps += lua

includes = include_directories('.', 'ext/yaml')

lyaml = library(
    'lyaml',
    lyaml_src,
    dependencies : deps,
    include_directories: includes,
)

lyaml_dep = declare_dependency(
    include_directories: includes,
    link_with: lyaml,
)
