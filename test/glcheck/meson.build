build_gl_version = 'Unknown'
test_renderer = 'System'

gl_result = cc.run(files('getglversion.c'), dependencies: sdl)
if not gl_result.compiled()
    warning('GL Version test failed to compile')
elif gl_result.returncode() != 0
    warning('GL Version test failed to run')
else
    build_gl_version = gl_result.stdout()
endif

summary('System OpenGL Version', build_gl_version, section: 'Tests')

if build_gl_version.version_compare('<3.1')
    message('Could not detect OpenGL >= 3.1. Attempting to find software renderer for tests.')
    if build_machine.system() in ['cygwin', 'windows']
        run_command('ln', '-f', '-s', '/mingw64/bin/opengl32.dll', meson.build_root())
        test_renderer = 'Mesa'
    else
        xvfb = find_program('xvfb-run', required: false)
        if xvfb.found()
            add_test_setup(
                'xvfb',
                exe_wrapper: xvfb,
                is_default: true
            )
            test_renderer = 'XVFB'
        endif
    endif
endif

summary('Renderer', test_renderer, section: 'Tests')