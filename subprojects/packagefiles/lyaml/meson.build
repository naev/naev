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

deps = []
deps += dependency('yaml-0.1')
deps += dependency('lua')

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
