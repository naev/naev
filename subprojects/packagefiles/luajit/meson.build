project('luajit', 'c', version : '2.0.5', license : 'mit')

# TODO: This is not ideal. We should translate the enough of the makefile to have Meson handle the build.
run_command(
    'make', '-C', meson.current_source_dir(), 'install',
    check: true,
    env: ['DESTDIR="' + (meson.current_source_dir() / 'out') + '"'])

cc = meson.get_compiler('c')
libs = cc.find_library(
    'luajit-5.1',
    dirs: meson.current_source_dir() / 'out/usr/local/lib',
    static: true)

luajit_dep = declare_dependency(
    dependencies: libs,
    include_directories: include_directories('out/usr/local/include/luajit-2.0'),
    version: meson.project_version())
