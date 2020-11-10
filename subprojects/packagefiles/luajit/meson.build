project('luajit', 'c', version : '2.0.5', license : 'mit')

# TODO: This is not ideal. We should translate the enough of the makefile to have Meson handle the build.
cc = meson.get_compiler('c')

env = environment()
if meson.is_cross_build()
    # Construct the compiler prefix, e.g. 'x86_64-w64-mingw32-'.
    # FIXME: This is absolutely disgusting. I'm not sure whether to loathe myself or blame the Meson team.
    cc_name_parts = cc.cmd_array()[0].split('/')[-1].split('\\')[-1].split('-')
    cc_name_hyphen = ''
    foreach cc_name_part: cc_name_parts
        cc_name_prefix = cc_name_hyphen
        cc_name_hyphen += cc_name_part + '-'
    endforeach

    env.set('CROSS', cc_name_prefix)
    if host_machine.system() == 'darwin'
        env.set('TARGET_SYS', 'Darwin')
    elif host_machine.system() == 'linux'
        env.set('TARGET_SYS', 'Linux')
    elif host_machine.system() == 'windows'
        env.set('TARGET_SYS', 'Windows')
    endif
endif

run_command(
    'make', '-C', meson.current_source_dir(), 'amalg',
    check: true,
    env: env)

if host_machine.system() == 'windows'
    libs = cc.find_library(
        'lua51',
        dirs: meson.current_source_dir() / 'src',
	static: false)
else
    libs = cc.find_library(
        'luajit',
        dirs: meson.current_source_dir() / 'src',
        static: true)
endif

luajit_dep = declare_dependency(
    dependencies: libs,
    include_directories: include_directories('src'),
    version: meson.project_version())
