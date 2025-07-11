build_gl_version = 'Unknown'
test_renderer = 'System'
note = 'Could not find a software renderer.'

if sdl.type_name() == 'internal'
    warning('Skipping GL Version test due to SDL being an internal dependency')
else
  gl_result = cc.run(files('getglversion.c'), dependencies: sdl)
  if not gl_result.compiled()
      warning('GL Version test failed to compile')
  elif gl_result.returncode() != 0
      warning('GL Version test failed to run')
  else
      build_gl_version = gl_result.stdout()
  endif
endif

summary('System OpenGL Version', build_gl_version, section: 'Tests')

# glVertexAttribDivisor requires 3.3
# Sampler Objects require 3.3

if build_gl_version.version_compare('<3.3')
    message('Could not detect OpenGL >= 3.3. Attempting to find software renderer for tests.')
    if build_machine.system() in ['cygwin', 'windows']
        opengl32 = run_command('which', 'opengl32.dll', check: true).stdout()
        run_command('ln', '-f', '-s', opengl32, meson.project_build_root(), check: true)
        test_renderer = 'Mesa'
        note = 'Found Mesa available as a software renderer.'
    elif build_machine.system() != 'darwin'
        xvfb = find_program('xvfb-run', required: false)
        if xvfb.found()
            add_test_setup(
                'xvfb',
                exe_wrapper: xvfb,
            )
            test_renderer = 'XVFB'
            note = 'Found XVFB available as a software renderer, you\'ll need to run \'meson test --setup=xvfb *testname*\' to use it.'
        endif
    else
        note = 'Could not find a software renderer.'
    endif
    message(note)
else
    note = 'OpenGL >= 3.3 support is available with your current driver/device.'
endif

summary('Renderer', test_renderer, section: 'Tests')
summary('Note', note, section: 'Tests')
