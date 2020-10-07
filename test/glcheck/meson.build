
build_gl_version = cc.run(files('getglversion.c'), dependencies: sdl).stdout()

if build_gl_version.version_compare('<3.1')
    if build_machine.system() in ['cygwin', 'windows']
        run_command('ln', '-f', '-s', '/mingw64/bin/opengl32.dll', meson.build_root())
    endif
endif